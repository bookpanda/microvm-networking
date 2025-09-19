/**
 * @file buffer_pool.c
 * @brief Buffer pool implementation
 */

#include "../include/buffer_pool.h"
#include "../include/ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000  // Linux-specific flag
#endif

struct microvm_net_buffer_pool {
    // Configuration
    size_t   buffer_size;
    uint32_t buffer_count;
    bool     use_hugepages;
    
    // Memory management
    void    *memory_base;
    size_t   memory_size;
    
    // Buffer descriptors
    microvm_net_buffer_t *buffers;
    
    // Free buffer ring
    microvm_net_ring_t *free_ring;
    
    // Statistics
    atomic_uint_fast64_t total_allocs;
    atomic_uint_fast64_t total_frees;
    atomic_uint_fast64_t failed_allocs;
};

#define HUGEPAGE_SIZE (2 * 1024 * 1024)  // 2MB
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))

microvm_net_buffer_pool_t* microvm_net_buffer_pool_create(
    const microvm_net_buffer_pool_config_t *config) {
    
    if (!config || config->buffer_size == 0 || config->buffer_count == 0) {
        printf("DEBUG: Invalid buffer pool config\n");
        return NULL;
    }
    
    printf("DEBUG: Creating buffer pool - size: %zu, count: %u\n", 
           config->buffer_size, config->buffer_count);
    
    microvm_net_buffer_pool_t *pool = calloc(1, sizeof(*pool));
    if (!pool) {
        printf("DEBUG: Failed to allocate buffer pool struct\n");
        return NULL;
    }
    
    pool->buffer_size = config->buffer_size;
    pool->buffer_count = config->buffer_count;
    pool->use_hugepages = config->use_hugepages;
    
    // Calculate memory requirements
    size_t aligned_buffer_size = ALIGN_UP(config->buffer_size, 64);  // Cache-line aligned
    size_t buffers_memory = aligned_buffer_size * config->buffer_count;
    size_t descriptors_memory = sizeof(microvm_net_buffer_t) * config->buffer_count;
    
    // Ring size needs to be buffer_count + 1, rounded up to power of 2
    uint32_t ring_size = config->buffer_count + 1;
    if ((ring_size & (ring_size - 1)) != 0) {
        ring_size--;
        ring_size |= ring_size >> 1;
        ring_size |= ring_size >> 2;
        ring_size |= ring_size >> 4;
        ring_size |= ring_size >> 8;
        ring_size |= ring_size >> 16;
        ring_size++;
    }
    size_t ring_memory = microvm_net_ring_get_memsize(sizeof(uint32_t), ring_size);
    
    pool->memory_size = buffers_memory + descriptors_memory + ring_memory;
    
    // Allocate memory
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    if (pool->use_hugepages && pool->memory_size >= HUGEPAGE_SIZE) {
        flags |= MAP_HUGETLB;
        pool->memory_size = ALIGN_UP(pool->memory_size, HUGEPAGE_SIZE);
    }
    
    printf("DEBUG: Allocating %zu bytes for buffer pool memory\n", pool->memory_size);
    pool->memory_base = mmap(NULL, pool->memory_size, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (pool->memory_base == MAP_FAILED) {
        printf("DEBUG: First mmap failed, trying fallback\n");
        // Fallback to regular pages if hugepages failed
        if (flags & MAP_HUGETLB) {
            flags &= ~MAP_HUGETLB;
            pool->memory_base = mmap(NULL, pool->memory_size, PROT_READ | PROT_WRITE, flags, -1, 0);
        }
        
        if (pool->memory_base == MAP_FAILED) {
            printf("DEBUG: Buffer pool mmap failed completely\n");
            free(pool);
            return NULL;
        }
    }
    printf("DEBUG: Buffer pool memory allocated successfully\n");
    
    // Setup memory layout
    char *mem_ptr = (char*)pool->memory_base;
    
    // Buffer data
    void *buffers_data = mem_ptr;
    mem_ptr += buffers_memory;
    
    // Buffer descriptors
    pool->buffers = (microvm_net_buffer_t*)mem_ptr;
    mem_ptr += descriptors_memory;
    
    // Free ring
    pool->free_ring = (microvm_net_ring_t*)mem_ptr;
    printf("DEBUG: Initializing free ring with size %u to hold %u buffers\n", ring_size, config->buffer_count);
    if (microvm_net_ring_init(pool->free_ring, sizeof(uint32_t), ring_size) != 0) {
        printf("DEBUG: Free ring initialization failed\n");
        munmap(pool->memory_base, pool->memory_size);
        free(pool);
        return NULL;
    }
    printf("DEBUG: Free ring initialized successfully\n");
    
    // Initialize buffer descriptors
    for (uint32_t i = 0; i < config->buffer_count; i++) {
        pool->buffers[i].data = (char*)buffers_data + i * aligned_buffer_size;
        pool->buffers[i].size = config->buffer_size;
        pool->buffers[i].index = i;
        pool->buffers[i].pool_priv = pool;
    }
    
    // Initialize free ring with all buffer indices
    printf("DEBUG: Allocating indices array\n");
    uint32_t *indices = malloc(config->buffer_count * sizeof(uint32_t));
    if (!indices) {
        printf("DEBUG: Failed to allocate indices array\n");
        munmap(pool->memory_base, pool->memory_size);
        free(pool);
        return NULL;
    }
    
    for (uint32_t i = 0; i < config->buffer_count; i++) {
        indices[i] = i;
    }
    
    printf("DEBUG: Enqueueing %u buffer indices to free ring\n", config->buffer_count);
    uint32_t enqueued = microvm_net_ring_sp_enqueue_bulk(pool->free_ring, indices, config->buffer_count);
    free(indices);
    
    printf("DEBUG: Enqueued %u out of %u indices\n", enqueued, config->buffer_count);
    if (enqueued != config->buffer_count) {
        printf("DEBUG: Failed to enqueue all buffer indices\n");
        munmap(pool->memory_base, pool->memory_size);
        free(pool);
        return NULL;
    }
    
    // Initialize statistics
    atomic_store_explicit(&pool->total_allocs, 0, memory_order_relaxed);
    atomic_store_explicit(&pool->total_frees, 0, memory_order_relaxed);
    atomic_store_explicit(&pool->failed_allocs, 0, memory_order_relaxed);
    
    return pool;
}

void microvm_net_buffer_pool_destroy(microvm_net_buffer_pool_t *pool) {
    if (!pool) {
        return;
    }
    
    if (pool->memory_base) {
        munmap(pool->memory_base, pool->memory_size);
    }
    
    free(pool);
}

microvm_net_buffer_t* microvm_net_buffer_alloc(microvm_net_buffer_pool_t *pool) {
    if (!pool) {
        return NULL;
    }
    
    uint32_t index;
    uint32_t dequeued = microvm_net_ring_sc_dequeue_bulk(pool->free_ring, &index, 1);
    
    if (dequeued == 0) {
        atomic_fetch_add_explicit(&pool->failed_allocs, 1, memory_order_relaxed);
        return NULL;
    }
    
    atomic_fetch_add_explicit(&pool->total_allocs, 1, memory_order_relaxed);
    
    microvm_net_buffer_t *buffer = &pool->buffers[index];
    microvm_net_buffer_reset(buffer);
    
    return buffer;
}

uint32_t microvm_net_buffer_alloc_bulk(microvm_net_buffer_pool_t *pool,
                                        microvm_net_buffer_t **buffers,
                                        uint32_t count) {
    if (!pool || !buffers || count == 0) {
        return 0;
    }
    
    uint32_t *indices = malloc(count * sizeof(uint32_t));
    if (!indices) {
        return 0;
    }
    
    uint32_t dequeued = microvm_net_ring_sc_dequeue_bulk(pool->free_ring, indices, count);
    
    for (uint32_t i = 0; i < dequeued; i++) {
        buffers[i] = &pool->buffers[indices[i]];
        microvm_net_buffer_reset(buffers[i]);
    }
    
    free(indices);
    
    atomic_fetch_add_explicit(&pool->total_allocs, dequeued, memory_order_relaxed);
    if (dequeued < count) {
        atomic_fetch_add_explicit(&pool->failed_allocs, count - dequeued, memory_order_relaxed);
    }
    
    return dequeued;
}

void microvm_net_buffer_free(microvm_net_buffer_pool_t *pool,
                             microvm_net_buffer_t *buffer) {
    if (!pool || !buffer) {
        return;
    }
    
    uint32_t index = buffer->index;
    uint32_t enqueued = microvm_net_ring_sp_enqueue_bulk(pool->free_ring, &index, 1);
    
    if (enqueued == 1) {
        atomic_fetch_add_explicit(&pool->total_frees, 1, memory_order_relaxed);
    }
}

void microvm_net_buffer_free_bulk(microvm_net_buffer_pool_t *pool,
                                  microvm_net_buffer_t **buffers,
                                  uint32_t count) {
    if (!pool || !buffers || count == 0) {
        return;
    }
    
    uint32_t *indices = malloc(count * sizeof(uint32_t));
    if (!indices) {
        return;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        indices[i] = buffers[i]->index;
    }
    
    uint32_t enqueued = microvm_net_ring_sp_enqueue_bulk(pool->free_ring, indices, count);
    free(indices);
    
    atomic_fetch_add_explicit(&pool->total_frees, enqueued, memory_order_relaxed);
}

microvm_net_buffer_t* microvm_net_buffer_get_by_index(microvm_net_buffer_pool_t *pool,
                                                       uint32_t index) {
    if (!pool || index >= pool->buffer_count) {
        return NULL;
    }
    
    return &pool->buffers[index];
}

uint32_t microvm_net_buffer_pool_available(microvm_net_buffer_pool_t *pool) {
    if (!pool) {
        return 0;
    }
    
    return microvm_net_ring_count(pool->free_ring);
}

uint32_t microvm_net_buffer_pool_size(microvm_net_buffer_pool_t *pool) {
    if (!pool) {
        return 0;
    }
    
    return pool->buffer_count;
}

int microvm_net_buffer_pool_get_stats(microvm_net_buffer_pool_t *pool,
                                       uint64_t *total_allocs,
                                       uint64_t *total_frees,
                                       uint64_t *current_allocs,
                                       uint64_t *failed_allocs) {
    if (!pool) {
        return -1;
    }
    
    if (total_allocs) {
        *total_allocs = atomic_load_explicit(&pool->total_allocs, memory_order_relaxed);
    }
    
    if (total_frees) {
        *total_frees = atomic_load_explicit(&pool->total_frees, memory_order_relaxed);
    }
    
    if (current_allocs) {
        uint64_t allocs = atomic_load_explicit(&pool->total_allocs, memory_order_relaxed);
        uint64_t frees = atomic_load_explicit(&pool->total_frees, memory_order_relaxed);
        *current_allocs = allocs - frees;
    }
    
    if (failed_allocs) {
        *failed_allocs = atomic_load_explicit(&pool->failed_allocs, memory_order_relaxed);
    }
    
    return 0;
}
