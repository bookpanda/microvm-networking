/**
 * @file microvm_net.h
 * @brief MicroVM Minimal Networking Stack - Main API
 *
 * A lightweight networking stack designed for microvms that bypasses
 * the kernel for high-performance messaging.
 */

#ifndef MICROVM_NET_H_
#define MICROVM_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define MICROVM_NET_VERSION_MAJOR 1
#define MICROVM_NET_VERSION_MINOR 0

// Maximum message size (1MB)
#define MICROVM_NET_MAX_MSG_SIZE (1024 * 1024)

// Default buffer size (4KB)
#define MICROVM_NET_DEFAULT_BUF_SIZE 4096

// Maximum number of flows per channel
#define MICROVM_NET_MAX_FLOWS 1024

/**
 * @brief Network flow identifier (5-tuple equivalent)
 */
typedef struct {
  uint32_t src_ip;
  uint32_t dst_ip;
  uint16_t src_port;
  uint16_t dst_port;
  uint8_t protocol;  // Usually UDP
} microvm_net_flow_t;

/**
 * @brief I/O vector for scatter-gather operations
 */
typedef struct {
  void *base;
  size_t len;
} microvm_net_iovec_t;

/**
 * @brief Message header for send/receive operations
 */
typedef struct {
  microvm_net_flow_t flow;
  microvm_net_iovec_t *iov;
  size_t iov_count;
  size_t total_len;
  uint32_t flags;
} microvm_net_msg_t;

/**
 * @brief Channel context (opaque handle)
 */
typedef struct microvm_net_channel microvm_net_channel_t;

/**
 * @brief Channel configuration
 */
typedef struct {
  size_t ring_size;     // Number of ring entries (power of 2)
  size_t buffer_count;  // Number of buffers in pool
  size_t buffer_size;   // Size of each buffer
  uint32_t flags;       // Configuration flags
} microvm_net_config_t;

// Message flags
#define MICROVM_NET_MSG_FLAG_NONE 0x00
#define MICROVM_NET_MSG_FLAG_NOTIFY 0x01  // Request delivery notification

// Channel configuration flags
#define MICROVM_NET_CONFIG_FLAG_NONE 0x00
#define MICROVM_NET_CONFIG_FLAG_ZEROCOPY 0x01  // Enable zero-copy mode

/**
 * @brief Initialize the microvm networking library
 * @return 0 on success, -1 on failure
 */
int microvm_net_init(void);

/**
 * @brief Cleanup the microvm networking library
 */
void microvm_net_cleanup(void);

/**
 * @brief Create a new networking channel
 * @param config Channel configuration (NULL for defaults)
 * @return Channel handle on success, NULL on failure
 */
microvm_net_channel_t *microvm_net_channel_create(
    const microvm_net_config_t *config);

/**
 * @brief Destroy a networking channel
 * @param channel Channel to destroy
 */
void microvm_net_channel_destroy(microvm_net_channel_t *channel);

/**
 * @brief Bind to a local address for listening
 * @param channel Channel handle
 * @param local_ip Local IP address (string format)
 * @param local_port Local port number
 * @return 0 on success, -1 on failure
 */
int microvm_net_bind(microvm_net_channel_t *channel, const char *local_ip,
                     uint16_t local_port);

/**
 * @brief Connect to a remote endpoint
 * @param channel Channel handle
 * @param local_ip Local IP address
 * @param remote_ip Remote IP address
 * @param remote_port Remote port
 * @param flow Output flow information
 * @return 0 on success, -1 on failure
 */
int microvm_net_connect(microvm_net_channel_t *channel, const char *local_ip,
                        const char *remote_ip, uint16_t remote_port,
                        microvm_net_flow_t *flow);

/**
 * @brief Send a message
 * @param channel Channel handle
 * @param msg Message to send
 * @return 0 on success, -1 on failure
 */
int microvm_net_send(microvm_net_channel_t *channel,
                     const microvm_net_msg_t *msg);

/**
 * @brief Receive a message (non-blocking)
 * @param channel Channel handle
 * @param msg Message buffer to fill
 * @return 1 if message received, 0 if no message available, -1 on error
 */
int microvm_net_recv(microvm_net_channel_t *channel, microvm_net_msg_t *msg);

/**
 * @brief Send a simple buffer
 * @param channel Channel handle
 * @param flow Target flow
 * @param buf Buffer to send
 * @param len Buffer length
 * @return 0 on success, -1 on failure
 */
int microvm_net_send_buf(microvm_net_channel_t *channel,
                         const microvm_net_flow_t *flow, const void *buf,
                         size_t len);

/**
 * @brief Receive into a simple buffer
 * @param channel Channel handle
 * @param buf Buffer to receive into
 * @param len Buffer length
 * @param flow Output flow information
 * @return Number of bytes received, 0 if no message, -1 on error
 */
ssize_t microvm_net_recv_buf(microvm_net_channel_t *channel, void *buf,
                             size_t len, microvm_net_flow_t *flow);

/**
 * @brief Get channel statistics
 * @param channel Channel handle
 * @param tx_packets Output: transmitted packets
 * @param rx_packets Output: received packets
 * @param tx_bytes Output: transmitted bytes
 * @param rx_bytes Output: received bytes
 * @return 0 on success, -1 on failure
 */
int microvm_net_get_stats(microvm_net_channel_t *channel, uint64_t *tx_packets,
                          uint64_t *rx_packets, uint64_t *tx_bytes,
                          uint64_t *rx_bytes);

#ifdef __cplusplus
}
#endif

#endif  // MICROVM_NET_H_
