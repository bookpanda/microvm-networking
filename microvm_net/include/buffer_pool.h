/**
 * @file buffer_pool.h
 * @brief Simple buffer pool management for microvm networking
 *
 * Provides efficient allocation and deallocation of fixed-size buffers
 * for packet processing.
 */

#ifndef MICROVM_NET_BUFFER_POOL_H_
#define MICROVM_NET_BUFFER_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Buffer pool context (opaque)
 */
typedef struct microvm_net_buffer_pool microvm_net_buffer_pool_t;

/**
 * @brief Buffer descriptor
 */
typedef struct {
  void *data;       // Buffer data pointer
  size_t size;      // Buffer size
  uint32_t index;   // Buffer index in pool
  void *pool_priv;  // Private pool data
} microvm_net_buffer_t;

/**
 * @brief Buffer pool configuration
 */
typedef struct {
  size_t buffer_size;     // Size of each buffer
  uint32_t buffer_count;  // Number of buffers in pool
  bool use_hugepages;     // Use huge pages if available
  uint32_t cache_size;    // Per-thread cache size (0 = no cache)
} microvm_net_buffer_pool_config_t;

/**
 * @brief Create a buffer pool
 * @param config Pool configuration
 * @return Pool handle on success, NULL on failure
 */
microvm_net_buffer_pool_t *microvm_net_buffer_pool_create(
    const microvm_net_buffer_pool_config_t *config);

/**
 * @brief Destroy a buffer pool
 * @param pool Pool to destroy
 */
void microvm_net_buffer_pool_destroy(microvm_net_buffer_pool_t *pool);

/**
 * @brief Allocate a buffer
 * @param pool Buffer pool
 * @return Allocated buffer on success, NULL if pool is empty
 */
microvm_net_buffer_t *microvm_net_buffer_alloc(microvm_net_buffer_pool_t *pool);

/**
 * @brief Allocate multiple buffers
 * @param pool Buffer pool
 * @param buffers Array to store allocated buffers
 * @param count Number of buffers to allocate
 * @return Number of buffers actually allocated
 */
uint32_t microvm_net_buffer_alloc_bulk(microvm_net_buffer_pool_t *pool,
                                       microvm_net_buffer_t **buffers,
                                       uint32_t count);

/**
 * @brief Free a buffer back to the pool
 * @param pool Buffer pool
 * @param buffer Buffer to free
 */
void microvm_net_buffer_free(microvm_net_buffer_pool_t *pool,
                             microvm_net_buffer_t *buffer);

/**
 * @brief Free multiple buffers back to the pool
 * @param pool Buffer pool
 * @param buffers Array of buffers to free
 * @param count Number of buffers to free
 */
void microvm_net_buffer_free_bulk(microvm_net_buffer_pool_t *pool,
                                  microvm_net_buffer_t **buffers,
                                  uint32_t count);

/**
 * @brief Get buffer by index
 * @param pool Buffer pool
 * @param index Buffer index
 * @return Buffer pointer, NULL if invalid index
 */
microvm_net_buffer_t *microvm_net_buffer_get_by_index(
    microvm_net_buffer_pool_t *pool, uint32_t index);

/**
 * @brief Get number of available buffers
 * @param pool Buffer pool
 * @return Number of free buffers
 */
uint32_t microvm_net_buffer_pool_available(microvm_net_buffer_pool_t *pool);

/**
 * @brief Get total number of buffers in pool
 * @param pool Buffer pool
 * @return Total buffer count
 */
uint32_t microvm_net_buffer_pool_size(microvm_net_buffer_pool_t *pool);

/**
 * @brief Get pool statistics
 * @param pool Buffer pool
 * @param total_allocs Output: total allocations
 * @param total_frees Output: total frees
 * @param current_allocs Output: currently allocated buffers
 * @param failed_allocs Output: failed allocations
 * @return 0 on success, -1 on failure
 */
int microvm_net_buffer_pool_get_stats(microvm_net_buffer_pool_t *pool,
                                      uint64_t *total_allocs,
                                      uint64_t *total_frees,
                                      uint64_t *current_allocs,
                                      uint64_t *failed_allocs);

/**
 * @brief Reset buffer content and metadata
 * @param buffer Buffer to reset
 */
static inline void microvm_net_buffer_reset(microvm_net_buffer_t *buffer) {
  // Reset any buffer-specific metadata here
  // Data content is not cleared for performance
  (void)buffer;  // Currently no metadata to reset
}

/**
 * @brief Get buffer data pointer with offset
 * @param buffer Buffer
 * @param offset Offset from start of buffer
 * @return Data pointer at offset, NULL if offset is invalid
 */
static inline void *microvm_net_buffer_get_data(microvm_net_buffer_t *buffer,
                                                size_t offset) {
  if (offset >= buffer->size) {
    return NULL;
  }
  return (char *)buffer->data + offset;
}

#ifdef __cplusplus
}
#endif

#endif  // MICROVM_NET_BUFFER_POOL_H_
