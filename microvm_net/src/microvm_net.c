/**
 * @file microvm_net.c
 * @brief Main implementation of the microvm networking stack
 */

#include "microvm_net.h"
#include "ring_buffer.h"
#include "packet_io.h"
#include "buffer_pool.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>
#include <unistd.h>

#define DEFAULT_RING_SIZE 1024
#define DEFAULT_BUFFER_COUNT 2048
#define DEFAULT_BUFFER_SIZE 4096

struct microvm_net_channel {
    // Configuration
    microvm_net_config_t config;
    
    // Packet I/O
    microvm_net_pktio_t *pktio;
    
    // Shared memory channel
    void *shm_base;
    size_t shm_size;
    
    // Ring buffers
    microvm_net_ring_t *tx_ring;    // App -> Network
    microvm_net_ring_t *rx_ring;    // Network -> App
    
    // Buffer pool
    microvm_net_buffer_pool_t *buffer_pool;
    
    // Flow management
    microvm_net_flow_t *flows;
    uint32_t flow_count;
    
    // Worker thread
    pthread_t worker_thread;
    volatile bool worker_running;
    
    // Statistics
    atomic_uint_fast64_t tx_packets;
    atomic_uint_fast64_t rx_packets;
    atomic_uint_fast64_t tx_bytes;
    atomic_uint_fast64_t rx_bytes;
};

// Global state
static bool g_initialized = false;
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;

// Network protocol headers
struct eth_header {
    uint8_t  dst_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;
} __attribute__((packed));

struct ip_header {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((packed));

struct udp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

#define ETH_TYPE_IP 0x0800
#define IP_PROTO_UDP 17

static void* worker_thread_func(void *arg);
static int process_rx_packet(microvm_net_channel_t *channel, microvm_net_packet_t *packet);
static int process_tx_messages(microvm_net_channel_t *channel);

int microvm_net_init(void) {
    pthread_mutex_lock(&g_init_mutex);
    
    if (g_initialized) {
        pthread_mutex_unlock(&g_init_mutex);
        return 0;
    }
    
    // Initialize any global state here
    g_initialized = true;
    
    pthread_mutex_unlock(&g_init_mutex);
    return 0;
}

void microvm_net_cleanup(void) {
    pthread_mutex_lock(&g_init_mutex);
    
    if (!g_initialized) {
        pthread_mutex_unlock(&g_init_mutex);
        return;
    }
    
    // Cleanup global state here
    g_initialized = false;
    
    pthread_mutex_unlock(&g_init_mutex);
}

microvm_net_channel_t* microvm_net_channel_create(const microvm_net_config_t *config) {
    if (!g_initialized) {
        if (microvm_net_init() != 0) {
            return NULL;
        }
    }
    
    microvm_net_channel_t *channel = calloc(1, sizeof(*channel));
    if (!channel) {
        return NULL;
    }
    
    // Set default configuration
    if (config) {
        channel->config = *config;
    } else {
        channel->config.ring_size = DEFAULT_RING_SIZE;
        channel->config.buffer_count = DEFAULT_BUFFER_COUNT;
        channel->config.buffer_size = DEFAULT_BUFFER_SIZE;
        channel->config.flags = MICROVM_NET_CONFIG_FLAG_NONE;
    }
    
    // Ensure ring size is power of 2
    uint32_t ring_size = channel->config.ring_size;
    if ((ring_size & (ring_size - 1)) != 0) {
        // Round up to next power of 2
        ring_size--;
        ring_size |= ring_size >> 1;
        ring_size |= ring_size >> 2;
        ring_size |= ring_size >> 4;
        ring_size |= ring_size >> 8;
        ring_size |= ring_size >> 16;
        ring_size++;
        channel->config.ring_size = ring_size;
    }
    
    // Calculate shared memory size
    size_t tx_ring_size = microvm_net_ring_get_memsize(sizeof(uint32_t), ring_size);
    size_t rx_ring_size = microvm_net_ring_get_memsize(sizeof(uint32_t), ring_size);
    size_t flows_size = MICROVM_NET_MAX_FLOWS * sizeof(microvm_net_flow_t);
    
    channel->shm_size = tx_ring_size + rx_ring_size + flows_size;
    channel->shm_size = (channel->shm_size + 4095) & ~4095;  // Page align
    
    // Allocate shared memory
    channel->shm_base = mmap(NULL, channel->shm_size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (channel->shm_base == MAP_FAILED) {
        free(channel);
        return NULL;
    }
    
    // Setup memory layout
    char *mem_ptr = (char*)channel->shm_base;
    
    channel->tx_ring = (microvm_net_ring_t*)mem_ptr;
    mem_ptr += tx_ring_size;
    
    channel->rx_ring = (microvm_net_ring_t*)mem_ptr;
    mem_ptr += rx_ring_size;
    
    channel->flows = (microvm_net_flow_t*)mem_ptr;
    
    // Initialize rings
    if (microvm_net_ring_init(channel->tx_ring, sizeof(uint32_t), ring_size) != 0 ||
        microvm_net_ring_init(channel->rx_ring, sizeof(uint32_t), ring_size) != 0) {
        munmap(channel->shm_base, channel->shm_size);
        free(channel);
        return NULL;
    }
    
    // Create buffer pool
    microvm_net_buffer_pool_config_t pool_config = {
        .buffer_size = channel->config.buffer_size,
        .buffer_count = channel->config.buffer_count,
        .use_hugepages = false,
        .cache_size = 64
    };
    
    channel->buffer_pool = microvm_net_buffer_pool_create(&pool_config);
    if (!channel->buffer_pool) {
        munmap(channel->shm_base, channel->shm_size);
        free(channel);
        return NULL;
    }
    
    // Initialize statistics
    atomic_store_explicit(&channel->tx_packets, 0, memory_order_relaxed);
    atomic_store_explicit(&channel->rx_packets, 0, memory_order_relaxed);
    atomic_store_explicit(&channel->tx_bytes, 0, memory_order_relaxed);
    atomic_store_explicit(&channel->rx_bytes, 0, memory_order_relaxed);
    
    return channel;
}

void microvm_net_channel_destroy(microvm_net_channel_t *channel) {
    if (!channel) {
        return;
    }
    
    // Stop worker thread
    if (channel->worker_running) {
        channel->worker_running = false;
        pthread_join(channel->worker_thread, NULL);
    }
    
    // Cleanup packet I/O
    if (channel->pktio) {
        microvm_net_pktio_destroy(channel->pktio);
    }
    
    // Cleanup buffer pool
    if (channel->buffer_pool) {
        microvm_net_buffer_pool_destroy(channel->buffer_pool);
    }
    
    // Cleanup shared memory
    if (channel->shm_base) {
        munmap(channel->shm_base, channel->shm_size);
    }
    
    free(channel);
}

int microvm_net_bind(microvm_net_channel_t *channel, 
                     const char *local_ip, 
                     uint16_t local_port) {
    if (!channel || !local_ip) {
        return -1;
    }
    
    // Create packet I/O if not already created
    if (!channel->pktio) {
        microvm_net_pktio_config_t pktio_config = {
            .interface = "tap0",
            .promiscuous = false,
            .rx_ring_size = channel->config.ring_size,
            .tx_ring_size = channel->config.ring_size,
            .buffer_size = channel->config.buffer_size
        };
        
        channel->pktio = microvm_net_pktio_create(&pktio_config, MICROVM_NET_IO_TAP);
        if (!channel->pktio) {
            return -1;
        }
        
        // Set IP address
        if (microvm_net_pktio_set_ip(channel->pktio, local_ip, "255.255.255.0") != 0) {
            return -1;
        }
    }
    
    // Start worker thread if not running
    if (!channel->worker_running) {
        channel->worker_running = true;
        if (pthread_create(&channel->worker_thread, NULL, worker_thread_func, channel) != 0) {
            channel->worker_running = false;
            return -1;
        }
    }
    
    return 0;
}

int microvm_net_connect(microvm_net_channel_t *channel,
                        const char *local_ip,
                        const char *remote_ip,
                        uint16_t remote_port,
                        microvm_net_flow_t *flow) {
    if (!channel || !local_ip || !remote_ip || !flow) {
        return -1;
    }
    
    // First bind to local address
    if (microvm_net_bind(channel, local_ip, 0) != 0) {
        return -1;
    }
    
    // Create flow
    flow->src_ip = ntohl(inet_addr(local_ip));
    flow->dst_ip = ntohl(inet_addr(remote_ip));
    flow->src_port = 0;  // Will be assigned dynamically
    flow->dst_port = remote_port;
    flow->protocol = IP_PROTO_UDP;
    
    // Add to flow table
    if (channel->flow_count < MICROVM_NET_MAX_FLOWS) {
        channel->flows[channel->flow_count++] = *flow;
    }
    
    return 0;
}

int microvm_net_send(microvm_net_channel_t *channel, const microvm_net_msg_t *msg) {
    if (!channel || !msg || !msg->iov || msg->iov_count == 0) {
        return -1;
    }
    
    // Allocate buffer
    microvm_net_buffer_t *buffer = microvm_net_buffer_alloc(channel->buffer_pool);
    if (!buffer) {
        return -1;
    }
    
    // Copy message data
    size_t offset = 0;
    for (size_t i = 0; i < msg->iov_count; i++) {
        if (offset + msg->iov[i].len > buffer->size) {
            microvm_net_buffer_free(channel->buffer_pool, buffer);
            return -1;
        }
        
        memcpy((char*)buffer->data + offset, msg->iov[i].base, msg->iov[i].len);
        offset += msg->iov[i].len;
    }
    
    // Enqueue buffer index for transmission
    uint32_t buffer_index = buffer->index;
    if (microvm_net_ring_sp_enqueue_bulk(channel->tx_ring, &buffer_index, 1) != 1) {
        microvm_net_buffer_free(channel->buffer_pool, buffer);
        return -1;
    }
    
    return 0;
}

int microvm_net_recv(microvm_net_channel_t *channel, microvm_net_msg_t *msg) {
    if (!channel || !msg) {
        return -1;
    }
    
    // Dequeue received buffer
    uint32_t buffer_index;
    if (microvm_net_ring_sc_dequeue_bulk(channel->rx_ring, &buffer_index, 1) != 1) {
        return 0;  // No message available
    }
    
    microvm_net_buffer_t *buffer = microvm_net_buffer_get_by_index(channel->buffer_pool, buffer_index);
    if (!buffer) {
        return -1;
    }
    
    // Copy data to user buffers
    size_t copied = 0;
    size_t buffer_offset = 0;
    
    for (size_t i = 0; i < msg->iov_count && copied < msg->total_len; i++) {
        size_t to_copy = msg->iov[i].len;
        if (copied + to_copy > msg->total_len) {
            to_copy = msg->total_len - copied;
        }
        
        memcpy(msg->iov[i].base, (char*)buffer->data + buffer_offset, to_copy);
        buffer_offset += to_copy;
        copied += to_copy;
    }
    
    msg->total_len = copied;
    
    // Free buffer
    microvm_net_buffer_free(channel->buffer_pool, buffer);
    
    return 1;
}

int microvm_net_send_buf(microvm_net_channel_t *channel,
                         const microvm_net_flow_t *flow,
                         const void *buf,
                         size_t len) {
    microvm_net_iovec_t iov = { .base = (void*)buf, .len = len };
    microvm_net_msg_t msg = {
        .flow = *flow,
        .iov = &iov,
        .iov_count = 1,
        .total_len = len,
        .flags = MICROVM_NET_MSG_FLAG_NONE
    };
    
    return microvm_net_send(channel, &msg);
}

ssize_t microvm_net_recv_buf(microvm_net_channel_t *channel,
                             void *buf,
                             size_t len,
                             microvm_net_flow_t *flow) {
    microvm_net_iovec_t iov = { .base = buf, .len = len };
    microvm_net_msg_t msg = {
        .iov = &iov,
        .iov_count = 1,
        .total_len = len,
        .flags = MICROVM_NET_MSG_FLAG_NONE
    };
    
    int ret = microvm_net_recv(channel, &msg);
    if (ret <= 0) {
        return ret;
    }
    
    if (flow) {
        *flow = msg.flow;
    }
    
    return msg.total_len;
}

int microvm_net_get_stats(microvm_net_channel_t *channel,
                          uint64_t *tx_packets,
                          uint64_t *rx_packets,
                          uint64_t *tx_bytes,
                          uint64_t *rx_bytes) {
    if (!channel) {
        return -1;
    }
    
    if (tx_packets) {
        *tx_packets = atomic_load_explicit(&channel->tx_packets, memory_order_relaxed);
    }
    
    if (rx_packets) {
        *rx_packets = atomic_load_explicit(&channel->rx_packets, memory_order_relaxed);
    }
    
    if (tx_bytes) {
        *tx_bytes = atomic_load_explicit(&channel->tx_bytes, memory_order_relaxed);
    }
    
    if (rx_bytes) {
        *rx_bytes = atomic_load_explicit(&channel->rx_bytes, memory_order_relaxed);
    }
    
    return 0;
}

// Worker thread implementation
static void* worker_thread_func(void *arg) {
    microvm_net_channel_t *channel = (microvm_net_channel_t*)arg;
    
    while (channel->worker_running) {
        // Process RX packets
        microvm_net_packet_t packets[16];
        int received = microvm_net_pktio_recv(channel->pktio, packets, 16);
        
        for (int i = 0; i < received; i++) {
            process_rx_packet(channel, &packets[i]);
            microvm_net_pktio_free_packet(channel->pktio, &packets[i]);
        }
        
        // Process TX messages
        process_tx_messages(channel);
        
        // Small sleep to avoid busy waiting
        usleep(10);
    }
    
    return NULL;
}

static int process_rx_packet(microvm_net_channel_t *channel, microvm_net_packet_t *packet) {
    // Simple UDP packet processing
    if (packet->len < sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header)) {
        return -1;
    }
    
    struct eth_header *eth = (struct eth_header*)packet->data;
    if (ntohs(eth->ethertype) != ETH_TYPE_IP) {
        return -1;
    }
    
    struct ip_header *ip = (struct ip_header*)((char*)packet->data + sizeof(struct eth_header));
    if (ip->protocol != IP_PROTO_UDP) {
        return -1;
    }
    
    // Allocate buffer and enqueue for application
    microvm_net_buffer_t *buffer = microvm_net_buffer_alloc(channel->buffer_pool);
    if (!buffer) {
        return -1;
    }
    
    // Copy payload (skip headers)
    size_t header_len = sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header);
    size_t payload_len = packet->len - header_len;
    
    if (payload_len > buffer->size) {
        microvm_net_buffer_free(channel->buffer_pool, buffer);
        return -1;
    }
    
    memcpy(buffer->data, (char*)packet->data + header_len, payload_len);
    
    uint32_t buffer_index = buffer->index;
    if (microvm_net_ring_sp_enqueue_bulk(channel->rx_ring, &buffer_index, 1) != 1) {
        microvm_net_buffer_free(channel->buffer_pool, buffer);
        return -1;
    }
    
    atomic_fetch_add_explicit(&channel->rx_packets, 1, memory_order_relaxed);
    atomic_fetch_add_explicit(&channel->rx_bytes, payload_len, memory_order_relaxed);
    
    return 0;
}

static int process_tx_messages(microvm_net_channel_t *channel) {
    uint32_t buffer_indices[16];
    uint32_t dequeued = microvm_net_ring_sc_dequeue_burst(channel->tx_ring, buffer_indices, 16);
    
    if (dequeued == 0) {
        return 0;
    }
    
    microvm_net_packet_t packets[16];
    int to_send = 0;
    
    for (uint32_t i = 0; i < dequeued; i++) {
        microvm_net_buffer_t *buffer = microvm_net_buffer_get_by_index(channel->buffer_pool, buffer_indices[i]);
        if (!buffer) {
            continue;
        }
        
        // For simplicity, we'll send raw payload for now
        // In a real implementation, we'd add proper headers
        packets[to_send].data = buffer->data;
        packets[to_send].len = buffer->size;  // Should track actual data length
        packets[to_send].buf_size = buffer->size;
        packets[to_send].priv = buffer;
        
        to_send++;
    }
    
    int sent = microvm_net_pktio_send(channel->pktio, packets, to_send);
    
    // Free buffers
    for (int i = 0; i < to_send; i++) {
        microvm_net_buffer_t *buffer = (microvm_net_buffer_t*)packets[i].priv;
        microvm_net_buffer_free(channel->buffer_pool, buffer);
        
        if (i < sent) {
            atomic_fetch_add_explicit(&channel->tx_packets, 1, memory_order_relaxed);
            atomic_fetch_add_explicit(&channel->tx_bytes, packets[i].len, memory_order_relaxed);
        }
    }
    
    return sent;
}
