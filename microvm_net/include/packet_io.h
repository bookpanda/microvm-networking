/**
 * @file packet_io.h
 * @brief Packet I/O interface for microvm networking
 *
 * Provides virtio-net and TAP-based packet I/O for kernel bypass
 * in microvm environments.
 */

#ifndef MICROVM_NET_PACKET_IO_H_
#define MICROVM_NET_PACKET_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Packet I/O context (opaque)
 */
typedef struct microvm_net_pktio microvm_net_pktio_t;

/**
 * @brief Packet buffer
 */
typedef struct {
  void *data;       // Packet data
  size_t len;       // Packet length
  size_t buf_size;  // Total buffer size
  void *priv;       // Private data for I/O layer
} microvm_net_packet_t;

/**
 * @brief Packet I/O configuration
 */
typedef struct {
  const char *interface;  // Interface name (e.g., "tap0", "eth0")
  bool promiscuous;       // Enable promiscuous mode
  uint32_t rx_ring_size;  // RX ring size
  uint32_t tx_ring_size;  // TX ring size
  uint32_t buffer_size;   // Packet buffer size
} microvm_net_pktio_config_t;

/**
 * @brief I/O backend types
 */
typedef enum {
  MICROVM_NET_IO_TAP,     // TAP interface
  MICROVM_NET_IO_VIRTIO,  // Virtio-net (when available)
  MICROVM_NET_IO_RAW,     // Raw socket (fallback)
} microvm_net_io_type_t;

/**
 * @brief Create packet I/O context
 * @param config I/O configuration
 * @param io_type Preferred I/O backend type
 * @return I/O context on success, NULL on failure
 */
microvm_net_pktio_t *microvm_net_pktio_create(
    const microvm_net_pktio_config_t *config, microvm_net_io_type_t io_type);

/**
 * @brief Destroy packet I/O context
 * @param pktio I/O context
 */
void microvm_net_pktio_destroy(microvm_net_pktio_t *pktio);

/**
 * @brief Set interface IP address
 * @param pktio I/O context
 * @param ip_addr IP address (string format)
 * @param netmask Netmask (string format)
 * @return 0 on success, -1 on failure
 */
int microvm_net_pktio_set_ip(microvm_net_pktio_t *pktio, const char *ip_addr,
                             const char *netmask);

/**
 * @brief Get interface MAC address
 * @param pktio I/O context
 * @param mac_addr Output buffer for MAC address (6 bytes)
 * @return 0 on success, -1 on failure
 */
int microvm_net_pktio_get_mac(microvm_net_pktio_t *pktio, uint8_t mac_addr[6]);

/**
 * @brief Receive packets
 * @param pktio I/O context
 * @param packets Array to store received packets
 * @param max_packets Maximum number of packets to receive
 * @return Number of packets received, -1 on error
 */
int microvm_net_pktio_recv(microvm_net_pktio_t *pktio,
                           microvm_net_packet_t *packets, int max_packets);

/**
 * @brief Send packets
 * @param pktio I/O context
 * @param packets Array of packets to send
 * @param num_packets Number of packets to send
 * @return Number of packets sent, -1 on error
 */
int microvm_net_pktio_send(microvm_net_pktio_t *pktio,
                           const microvm_net_packet_t *packets,
                           int num_packets);

/**
 * @brief Allocate packet buffer
 * @param pktio I/O context
 * @param size Required buffer size
 * @return Allocated packet on success, NULL on failure
 */
microvm_net_packet_t *microvm_net_pktio_alloc_packet(microvm_net_pktio_t *pktio,
                                                     size_t size);

/**
 * @brief Free packet buffer
 * @param pktio I/O context
 * @param packet Packet to free
 */
void microvm_net_pktio_free_packet(microvm_net_pktio_t *pktio,
                                   microvm_net_packet_t *packet);

/**
 * @brief Get file descriptor for polling
 * @param pktio I/O context
 * @return File descriptor, -1 if not supported
 */
int microvm_net_pktio_get_fd(microvm_net_pktio_t *pktio);

/**
 * @brief Check if packets are available for reading
 * @param pktio I/O context
 * @return true if packets available, false otherwise
 */
bool microvm_net_pktio_has_packets(microvm_net_pktio_t *pktio);

/**
 * @brief Get I/O statistics
 * @param pktio I/O context
 * @param rx_packets Output: received packets
 * @param tx_packets Output: transmitted packets
 * @param rx_bytes Output: received bytes
 * @param tx_bytes Output: transmitted bytes
 * @param rx_dropped Output: dropped RX packets
 * @param tx_dropped Output: dropped TX packets
 * @return 0 on success, -1 on failure
 */
int microvm_net_pktio_get_stats(microvm_net_pktio_t *pktio,
                                uint64_t *rx_packets, uint64_t *tx_packets,
                                uint64_t *rx_bytes, uint64_t *tx_bytes,
                                uint64_t *rx_dropped, uint64_t *tx_dropped);

#ifdef __cplusplus
}
#endif

#endif  // MICROVM_NET_PACKET_IO_H_
