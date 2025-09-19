/**
 * @file packet_io_tap.c
 * @brief TAP-based packet I/O implementation
 */

#include "../include/packet_io.h"
#include "../include/buffer_pool.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>

struct microvm_net_pktio {
    // Configuration
    char interface[IFNAMSIZ];
    bool promiscuous;
    uint32_t buffer_size;
    
    // TAP interface
    int tap_fd;
    
    // Buffer pool
    microvm_net_buffer_pool_t *buffer_pool;
    
    // Statistics
    atomic_uint_fast64_t rx_packets;
    atomic_uint_fast64_t tx_packets;
    atomic_uint_fast64_t rx_bytes;
    atomic_uint_fast64_t tx_bytes;
    atomic_uint_fast64_t rx_dropped;
    atomic_uint_fast64_t tx_dropped;
};

static int create_tap_interface(const char *dev_name) {
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        return -1;
    }
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (dev_name) {
        strncpy(ifr.ifr_name, dev_name, IFNAMSIZ - 1);
    }
    
    if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
        close(fd);
        return -1;
    }
    
    return fd;
}

static int set_interface_flags(const char *interface, int flags) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return -1;
    }
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    
    if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(sock_fd);
        return -1;
    }
    
    ifr.ifr_flags |= flags;
    
    int ret = ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
    close(sock_fd);
    
    return ret;
}

microvm_net_pktio_t* microvm_net_pktio_create(const microvm_net_pktio_config_t *config,
                                               microvm_net_io_type_t io_type) {
    if (!config || io_type != MICROVM_NET_IO_TAP) {
        return NULL;
    }
    
    microvm_net_pktio_t *pktio = calloc(1, sizeof(*pktio));
    if (!pktio) {
        return NULL;
    }
    
    // Store configuration
    strncpy(pktio->interface, config->interface, IFNAMSIZ - 1);
    pktio->promiscuous = config->promiscuous;
    pktio->buffer_size = config->buffer_size;
    
    // Create TAP interface
    pktio->tap_fd = create_tap_interface(config->interface);
    if (pktio->tap_fd < 0) {
        free(pktio);
        return NULL;
    }
    
    // Set interface up
    if (set_interface_flags(pktio->interface, IFF_UP | IFF_RUNNING) < 0) {
        close(pktio->tap_fd);
        free(pktio);
        return NULL;
    }
    
    // Set promiscuous mode if requested
    if (config->promiscuous) {
        set_interface_flags(pktio->interface, IFF_PROMISC);
    }
    
    // Create buffer pool
    microvm_net_buffer_pool_config_t pool_config = {
        .buffer_size = config->buffer_size,
        .buffer_count = config->rx_ring_size + config->tx_ring_size,
        .use_hugepages = false,
        .cache_size = 0
    };
    
    pktio->buffer_pool = microvm_net_buffer_pool_create(&pool_config);
    if (!pktio->buffer_pool) {
        close(pktio->tap_fd);
        free(pktio);
        return NULL;
    }
    
    // Set non-blocking mode
    int flags = fcntl(pktio->tap_fd, F_GETFL, 0);
    fcntl(pktio->tap_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Initialize statistics
    atomic_store_explicit(&pktio->rx_packets, 0, memory_order_relaxed);
    atomic_store_explicit(&pktio->tx_packets, 0, memory_order_relaxed);
    atomic_store_explicit(&pktio->rx_bytes, 0, memory_order_relaxed);
    atomic_store_explicit(&pktio->tx_bytes, 0, memory_order_relaxed);
    atomic_store_explicit(&pktio->rx_dropped, 0, memory_order_relaxed);
    atomic_store_explicit(&pktio->tx_dropped, 0, memory_order_relaxed);
    
    return pktio;
}

void microvm_net_pktio_destroy(microvm_net_pktio_t *pktio) {
    if (!pktio) {
        return;
    }
    
    if (pktio->tap_fd >= 0) {
        close(pktio->tap_fd);
    }
    
    if (pktio->buffer_pool) {
        microvm_net_buffer_pool_destroy(pktio->buffer_pool);
    }
    
    free(pktio);
}

int microvm_net_pktio_set_ip(microvm_net_pktio_t *pktio,
                              const char *ip_addr,
                              const char *netmask) {
    if (!pktio || !ip_addr) {
        return -1;
    }
    
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return -1;
    }
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pktio->interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
    // Set IP address
    struct sockaddr_in *addr = (struct sockaddr_in*)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &addr->sin_addr);
    
    if (ioctl(sock_fd, SIOCSIFADDR, &ifr) < 0) {
        close(sock_fd);
        return -1;
    }
    
    // Set netmask if provided
    if (netmask) {
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, pktio->interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        
        struct sockaddr_in *mask = (struct sockaddr_in*)&ifr.ifr_netmask;
        mask->sin_family = AF_INET;
        inet_pton(AF_INET, netmask, &mask->sin_addr);
        
        ioctl(sock_fd, SIOCSIFNETMASK, &ifr);
    }
    
    close(sock_fd);
    return 0;
}

int microvm_net_pktio_get_mac(microvm_net_pktio_t *pktio, uint8_t mac_addr[6]) {
    if (!pktio || !mac_addr) {
        return -1;
    }
    
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return -1;
    }
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pktio->interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
    if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock_fd);
        return -1;
    }
    
    memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, 6);
    close(sock_fd);
    
    return 0;
}

int microvm_net_pktio_recv(microvm_net_pktio_t *pktio,
                           microvm_net_packet_t *packets,
                           int max_packets) {
    if (!pktio || !packets || max_packets <= 0) {
        return -1;
    }
    
    int received = 0;
    
    for (int i = 0; i < max_packets; i++) {
        microvm_net_buffer_t *buffer = microvm_net_buffer_alloc(pktio->buffer_pool);
        if (!buffer) {
            atomic_fetch_add_explicit(&pktio->rx_dropped, 1, memory_order_relaxed);
            break;
        }
        
        ssize_t bytes = read(pktio->tap_fd, buffer->data, buffer->size);
        if (bytes <= 0) {
            microvm_net_buffer_free(pktio->buffer_pool, buffer);
            if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                return -1;
            }
            break;
        }
        
        packets[i].data = buffer->data;
        packets[i].len = bytes;
        packets[i].buf_size = buffer->size;
        packets[i].priv = buffer;
        
        atomic_fetch_add_explicit(&pktio->rx_packets, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&pktio->rx_bytes, bytes, memory_order_relaxed);
        
        received++;
    }
    
    return received;
}

int microvm_net_pktio_send(microvm_net_pktio_t *pktio,
                           const microvm_net_packet_t *packets,
                           int num_packets) {
    if (!pktio || !packets || num_packets <= 0) {
        return -1;
    }
    
    int sent = 0;
    
    for (int i = 0; i < num_packets; i++) {
        ssize_t bytes = write(pktio->tap_fd, packets[i].data, packets[i].len);
        if (bytes < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                return -1;
            }
            atomic_fetch_add_explicit(&pktio->tx_dropped, 1, memory_order_relaxed);
            break;
        }
        
        atomic_fetch_add_explicit(&pktio->tx_packets, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&pktio->tx_bytes, bytes, memory_order_relaxed);
        
        sent++;
    }
    
    return sent;
}

microvm_net_packet_t* microvm_net_pktio_alloc_packet(microvm_net_pktio_t *pktio,
                                                      size_t size) {
    if (!pktio || size > pktio->buffer_size) {
        return NULL;
    }
    
    microvm_net_buffer_t *buffer = microvm_net_buffer_alloc(pktio->buffer_pool);
    if (!buffer) {
        return NULL;
    }
    
    microvm_net_packet_t *packet = malloc(sizeof(*packet));
    if (!packet) {
        microvm_net_buffer_free(pktio->buffer_pool, buffer);
        return NULL;
    }
    
    packet->data = buffer->data;
    packet->len = 0;
    packet->buf_size = buffer->size;
    packet->priv = buffer;
    
    return packet;
}

void microvm_net_pktio_free_packet(microvm_net_pktio_t *pktio,
                                   microvm_net_packet_t *packet) {
    if (!pktio || !packet) {
        return;
    }
    
    if (packet->priv) {
        microvm_net_buffer_free(pktio->buffer_pool, (microvm_net_buffer_t*)packet->priv);
    }
    
    free(packet);
}

int microvm_net_pktio_get_fd(microvm_net_pktio_t *pktio) {
    return pktio ? pktio->tap_fd : -1;
}

bool microvm_net_pktio_has_packets(microvm_net_pktio_t *pktio) {
    if (!pktio || pktio->tap_fd < 0) {
        return false;
    }
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pktio->tap_fd, &readfds);
    
    struct timeval timeout = {0, 0};  // Non-blocking
    
    int result = select(pktio->tap_fd + 1, &readfds, NULL, NULL, &timeout);
    return result > 0 && FD_ISSET(pktio->tap_fd, &readfds);
}

int microvm_net_pktio_get_stats(microvm_net_pktio_t *pktio,
                                uint64_t *rx_packets,
                                uint64_t *tx_packets,
                                uint64_t *rx_bytes,
                                uint64_t *tx_bytes,
                                uint64_t *rx_dropped,
                                uint64_t *tx_dropped) {
    if (!pktio) {
        return -1;
    }
    
    if (rx_packets) {
        *rx_packets = atomic_load_explicit(&pktio->rx_packets, memory_order_relaxed);
    }
    
    if (tx_packets) {
        *tx_packets = atomic_load_explicit(&pktio->tx_packets, memory_order_relaxed);
    }
    
    if (rx_bytes) {
        *rx_bytes = atomic_load_explicit(&pktio->rx_bytes, memory_order_relaxed);
    }
    
    if (tx_bytes) {
        *tx_bytes = atomic_load_explicit(&pktio->tx_bytes, memory_order_relaxed);
    }
    
    if (rx_dropped) {
        *rx_dropped = atomic_load_explicit(&pktio->rx_dropped, memory_order_relaxed);
    }
    
    if (tx_dropped) {
        *tx_dropped = atomic_load_explicit(&pktio->tx_dropped, memory_order_relaxed);
    }
    
    return 0;
}
