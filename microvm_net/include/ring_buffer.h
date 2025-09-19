/**
 * @file ring_buffer.h
 * @brief Lock-free ring buffer implementation for microvm networking
 *
 * Simplified version of jring optimized for microvm use cases.
 */

#ifndef MICROVM_NET_RING_BUFFER_H_
#define MICROVM_NET_RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MICROVM_NET_CACHE_LINE_SIZE 64

/**
 * @brief Ring buffer structure
 */
typedef struct {
  // Ring configuration
  uint32_t size;         // Size of ring (must be power of 2)
  uint32_t mask;         // Size - 1 (for fast modulo)
  uint32_t element_size; // Size of each element in bytes

  // Producer state (cache-line aligned)
  _Alignas(MICROVM_NET_CACHE_LINE_SIZE) atomic_uint_fast32_t prod_head;
  _Alignas(MICROVM_NET_CACHE_LINE_SIZE) atomic_uint_fast32_t prod_tail;

  // Consumer state (cache-line aligned)
  _Alignas(MICROVM_NET_CACHE_LINE_SIZE) atomic_uint_fast32_t cons_head;
  _Alignas(MICROVM_NET_CACHE_LINE_SIZE) atomic_uint_fast32_t cons_tail;

  // Ring data follows this structure
  _Alignas(MICROVM_NET_CACHE_LINE_SIZE) char ring[];
} microvm_net_ring_t;

/**
 * @brief Calculate memory size needed for ring buffer
 * @param element_size Size of each element
 * @param count Number of elements (must be power of 2)
 * @return Required memory size, or 0 on error
 */
size_t microvm_net_ring_get_memsize(uint32_t element_size, uint32_t count);

/**
 * @brief Initialize a ring buffer
 * @param ring Pointer to allocated memory
 * @param element_size Size of each element
 * @param count Number of elements (must be power of 2)
 * @return 0 on success, -1 on error
 */
int microvm_net_ring_init(microvm_net_ring_t *ring, uint32_t element_size,
                          uint32_t count);

/**
 * @brief Enqueue elements (single producer)
 * @param ring Ring buffer
 * @param objs Array of objects to enqueue
 * @param count Number of objects
 * @return Number of objects enqueued
 */
uint32_t microvm_net_ring_sp_enqueue_bulk(microvm_net_ring_t *ring,
                                          const void *objs, uint32_t count);

/**
 * @brief Enqueue elements (multi-producer safe)
 * @param ring Ring buffer
 * @param objs Array of objects to enqueue
 * @param count Number of objects
 * @return Number of objects enqueued
 */
uint32_t microvm_net_ring_mp_enqueue_bulk(microvm_net_ring_t *ring,
                                          const void *objs, uint32_t count);

/**
 * @brief Dequeue elements (single consumer)
 * @param ring Ring buffer
 * @param objs Array to store dequeued objects
 * @param count Maximum number of objects to dequeue
 * @return Number of objects dequeued
 */
uint32_t microvm_net_ring_sc_dequeue_bulk(microvm_net_ring_t *ring, void *objs,
                                          uint32_t count);

/**
 * @brief Dequeue elements (multi-consumer safe)
 * @param ring Ring buffer
 * @param objs Array to store dequeued objects
 * @param count Maximum number of objects to dequeue
 * @return Number of objects dequeued
 */
uint32_t microvm_net_ring_mc_dequeue_bulk(microvm_net_ring_t *ring, void *objs,
                                          uint32_t count);

/**
 * @brief Enqueue up to N elements (burst mode, single producer)
 * @param ring Ring buffer
 * @param objs Array of objects to enqueue
 * @param count Maximum number of objects to enqueue
 * @return Number of objects actually enqueued
 */
uint32_t microvm_net_ring_sp_enqueue_burst(microvm_net_ring_t *ring,
                                           const void *objs, uint32_t count);

/**
 * @brief Dequeue up to N elements (burst mode, single consumer)
 * @param ring Ring buffer
 * @param objs Array to store dequeued objects
 * @param count Maximum number of objects to dequeue
 * @return Number of objects actually dequeued
 */
uint32_t microvm_net_ring_sc_dequeue_burst(microvm_net_ring_t *ring, void *objs,
                                           uint32_t count);

/**
 * @brief Get number of elements in ring
 * @param ring Ring buffer
 * @return Number of elements currently in ring
 */
static inline uint32_t microvm_net_ring_count(const microvm_net_ring_t *ring) {
  uint32_t prod_tail =
      atomic_load_explicit(&ring->prod_tail, memory_order_acquire);
  uint32_t cons_tail =
      atomic_load_explicit(&ring->cons_tail, memory_order_acquire);
  return (prod_tail - cons_tail) & ring->mask;
}

/**
 * @brief Get number of free slots in ring
 * @param ring Ring buffer
 * @return Number of free slots
 */
static inline uint32_t
microvm_net_ring_free_count(const microvm_net_ring_t *ring) {
  return (ring->mask + 1) - microvm_net_ring_count(ring) - 1;
}

/**
 * @brief Check if ring is empty
 * @param ring Ring buffer
 * @return true if empty, false otherwise
 */
static inline bool microvm_net_ring_empty(const microvm_net_ring_t *ring) {
  uint32_t prod_tail =
      atomic_load_explicit(&ring->prod_tail, memory_order_acquire);
  uint32_t cons_tail =
      atomic_load_explicit(&ring->cons_tail, memory_order_acquire);
  return prod_tail == cons_tail;
}

/**
 * @brief Check if ring is full
 * @param ring Ring buffer
 * @return true if full, false otherwise
 */
static inline bool microvm_net_ring_full(const microvm_net_ring_t *ring) {
  return microvm_net_ring_free_count(ring) == 0;
}

#ifdef __cplusplus
}
#endif

#endif // MICROVM_NET_RING_BUFFER_H_
