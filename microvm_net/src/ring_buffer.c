/**
 * @file ring_buffer.c
 * @brief Lock-free ring buffer implementation
 */

#include "../include/ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define RING_ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define IS_POWER_OF_2(n) ((n) && !((n) & ((n) - 1)))

size_t microvm_net_ring_get_memsize(uint32_t element_size, uint32_t count) {
    if (element_size == 0 || count == 0 || !IS_POWER_OF_2(count)) {
        return 0;
    }
    
    size_t ring_size = sizeof(microvm_net_ring_t) + (count * element_size);
    return RING_ALIGN_UP(ring_size, MICROVM_NET_CACHE_LINE_SIZE);
}

int microvm_net_ring_init(microvm_net_ring_t *ring, uint32_t element_size, uint32_t count) {
    printf("DEBUG: Ring init - element_size: %u, count: %u, is_power_of_2: %d\n", 
           element_size, count, IS_POWER_OF_2(count));
    if (!ring || element_size == 0 || count == 0 || !IS_POWER_OF_2(count)) {
        printf("DEBUG: Ring init failed validation\n");
        return -1;
    }
    printf("DEBUG: Ring init validation passed\n");
    
    memset(ring, 0, sizeof(*ring));
    
    ring->size = count;
    ring->mask = count - 1;
    ring->element_size = element_size;
    
    atomic_store_explicit(&ring->prod_head, 0, memory_order_relaxed);
    atomic_store_explicit(&ring->prod_tail, 0, memory_order_relaxed);
    atomic_store_explicit(&ring->cons_head, 0, memory_order_relaxed);
    atomic_store_explicit(&ring->cons_tail, 0, memory_order_relaxed);
    
    return 0;
}

static inline void ring_copy_elements(void *dst, const void *src,
                                      uint32_t element_size, uint32_t count) {
    memcpy(dst, src, element_size * count);
}

static uint32_t ring_do_enqueue(microvm_net_ring_t *ring, const void *objs,
                                 uint32_t count, bool is_sp, bool exact) {
    uint_fast32_t prod_head, prod_next;
    uint32_t cons_tail;
    uint32_t free_entries;
    
    printf("DEBUG: Enqueue - requested count: %u\n", count);
    
    do {
        prod_head = atomic_load_explicit(&ring->prod_head, memory_order_relaxed);
        cons_tail = atomic_load_explicit(&ring->cons_tail, memory_order_acquire);
        
        free_entries = (ring->mask + cons_tail - prod_head);
        
        printf("DEBUG: Enqueue - prod_head: %u, cons_tail: %u, mask: %u, free_entries: %u\n", 
               (uint32_t)prod_head, cons_tail, ring->mask, free_entries);
        
        if (count > free_entries) {
            printf("DEBUG: Enqueue - count (%u) > free_entries (%u), exact: %d\n", count, free_entries, exact);
            if (exact) {
                return 0;
            }
            count = free_entries;
        }
        
        if (count == 0) {
            printf("DEBUG: Enqueue - count is 0, returning\n");
            return 0;
        }
        
        prod_next = prod_head + count;
        
        if (is_sp) {
            atomic_store_explicit(&ring->prod_head, prod_next, memory_order_relaxed);
            break;
        } else {
            if (atomic_compare_exchange_weak_explicit(&ring->prod_head, &prod_head, prod_next,
                                                      memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        }
    } while (1);
    
    // Copy elements to ring
    uint32_t idx = prod_head & ring->mask;
    uint32_t size = ring->size;
    uint32_t element_size = ring->element_size;
    
    if (idx + count <= size) {
        // Single copy
        ring_copy_elements(&ring->ring[idx * element_size], objs, element_size, count);
    } else {
        // Split copy
        uint32_t first_part = size - idx;
        ring_copy_elements(&ring->ring[idx * element_size], objs, element_size, first_part);
        ring_copy_elements(&ring->ring[0], 
                          (const char*)objs + first_part * element_size,
                          element_size, count - first_part);
    }
    
    // Wait for previous producers to finish
    if (!is_sp) {
        while (atomic_load_explicit(&ring->prod_tail, memory_order_relaxed) != prod_head) {
            // Spin wait
        }
    }
    
    atomic_store_explicit(&ring->prod_tail, prod_next, memory_order_release);
    
    return count;
}

static uint32_t ring_do_dequeue(microvm_net_ring_t *ring, void *objs,
                                 uint32_t count, bool is_sc, bool exact) {
    uint_fast32_t cons_head, cons_next;
    uint32_t prod_tail;
    uint32_t entries;
    
    do {
        cons_head = atomic_load_explicit(&ring->cons_head, memory_order_relaxed);
        prod_tail = atomic_load_explicit(&ring->prod_tail, memory_order_acquire);
        
        entries = (prod_tail - cons_head);
        
        if (count > entries) {
            if (exact) {
                return 0;
            }
            count = entries;
        }
        
        if (count == 0) {
            return 0;
        }
        
        cons_next = cons_head + count;
        
        if (is_sc) {
            atomic_store_explicit(&ring->cons_head, cons_next, memory_order_relaxed);
            break;
        } else {
            if (atomic_compare_exchange_weak_explicit(&ring->cons_head, &cons_head, cons_next,
                                                      memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        }
    } while (1);
    
    // Copy elements from ring
    uint32_t idx = cons_head & ring->mask;
    uint32_t size = ring->size;
    uint32_t element_size = ring->element_size;
    
    if (idx + count <= size) {
        // Single copy
        ring_copy_elements(objs, &ring->ring[idx * element_size], element_size, count);
    } else {
        // Split copy
        uint32_t first_part = size - idx;
        ring_copy_elements(objs, &ring->ring[idx * element_size], element_size, first_part);
        ring_copy_elements((char*)objs + first_part * element_size,
                          &ring->ring[0], element_size, count - first_part);
    }
    
    // Wait for previous consumers to finish
    if (!is_sc) {
        while (atomic_load_explicit(&ring->cons_tail, memory_order_relaxed) != cons_head) {
            // Spin wait
        }
    }
    
    atomic_store_explicit(&ring->cons_tail, cons_next, memory_order_release);
    
    return count;
}

uint32_t microvm_net_ring_sp_enqueue_bulk(microvm_net_ring_t *ring,
                                           const void *objs, uint32_t count) {
    return ring_do_enqueue(ring, objs, count, true, true);
}

uint32_t microvm_net_ring_mp_enqueue_bulk(microvm_net_ring_t *ring,
                                           const void *objs, uint32_t count) {
    return ring_do_enqueue(ring, objs, count, false, true);
}

uint32_t microvm_net_ring_sc_dequeue_bulk(microvm_net_ring_t *ring,
                                           void *objs, uint32_t count) {
    return ring_do_dequeue(ring, objs, count, true, true);
}

uint32_t microvm_net_ring_mc_dequeue_bulk(microvm_net_ring_t *ring,
                                           void *objs, uint32_t count) {
    return ring_do_dequeue(ring, objs, count, false, true);
}

uint32_t microvm_net_ring_sp_enqueue_burst(microvm_net_ring_t *ring,
                                            const void *objs, uint32_t count) {
    return ring_do_enqueue(ring, objs, count, true, false);
}

uint32_t microvm_net_ring_sc_dequeue_burst(microvm_net_ring_t *ring,
                                            void *objs, uint32_t count) {
    return ring_do_dequeue(ring, objs, count, true, false);
}
