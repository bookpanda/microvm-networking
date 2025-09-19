/**
 * @file echo_server.c
 * @brief Echo server example for microvm networking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include "microvm_net.h"

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -l, --local IP     Local IP address (required)\n");
    printf("  -p, --port PORT    Port number (default: 8080)\n");
    printf("  -v, --verbose      Verbose output\n");
    printf("  -h, --help         Show this help\n");
}

int main(int argc, char *argv[]) {
    const char *local_ip = NULL;
    uint16_t port = 8080;
    int verbose = 0;
    
    static struct option long_options[] = {
        {"local",   required_argument, 0, 'l'},
        {"port",    required_argument, 0, 'p'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "l:p:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                local_ip = optarg;
                break;
            case 'p':
                port = (uint16_t)atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (!local_ip) {
        fprintf(stderr, "Error: Local IP address is required\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize networking
    if (microvm_net_init() != 0) {
        fprintf(stderr, "Failed to initialize microvm networking\n");
        return 1;
    }
    
    // Create channel with larger buffers for better performance
    microvm_net_config_t config = {
        .ring_size = 2048,
        .buffer_count = 4096,
        .buffer_size = 8192,
        .flags = MICROVM_NET_CONFIG_FLAG_NONE
    };
    
    microvm_net_channel_t *channel = microvm_net_channel_create(&config);
    if (!channel) {
        fprintf(stderr, "Failed to create networking channel\n");
        microvm_net_cleanup();
        return 1;
    }
    
    // Bind to local address
    if (microvm_net_bind(channel, local_ip, port) != 0) {
        fprintf(stderr, "Failed to bind to %s:%d\n", local_ip, port);
        microvm_net_channel_destroy(channel);
        microvm_net_cleanup();
        return 1;
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Echo Server started on %s:%d\n", local_ip, port);
    if (verbose) {
        printf("Configuration:\n");
        printf("  Ring size: %u\n", config.ring_size);
        printf("  Buffer count: %u\n", config.buffer_count);
        printf("  Buffer size: %u bytes\n", config.buffer_size);
    }
    printf("Press Ctrl+C to stop\n\n");
    
    uint64_t message_count = 0;
    time_t last_stats = time(NULL);
    
    while (running) {
        char buffer[8192];
        microvm_net_flow_t client_flow;
        
        ssize_t received = microvm_net_recv_buf(channel, buffer, sizeof(buffer) - 1, &client_flow);
        if (received > 0) {
            message_count++;
            
            if (verbose) {
                buffer[received] = '\0';
                printf("Received %zd bytes from client: %s\n", received, buffer);
            }
            
            // Echo the message back
            if (microvm_net_send_buf(channel, &client_flow, buffer, received) != 0) {
                fprintf(stderr, "Failed to echo message back to client\n");
            } else if (verbose) {
                printf("Echoed %zd bytes back to client\n", received);
            }
        }
        
        // Print periodic statistics
        time_t now = time(NULL);
        if (now - last_stats >= 10) {  // Every 10 seconds
            uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
            if (microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes) == 0) {
                printf("Stats: %lu messages processed, TX: %lu pkts/%lu bytes, RX: %lu pkts/%lu bytes\n",
                       message_count, tx_packets, tx_bytes, rx_packets, rx_bytes);
            }
            last_stats = now;
        }
        
        // Small sleep to avoid busy waiting
        usleep(1000);  // 1ms
    }
    
    printf("\nShutting down echo server...\n");
    
    // Final statistics
    uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
    if (microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes) == 0) {
        printf("\nFinal Statistics:\n");
        printf("  Messages processed: %lu\n", message_count);
        printf("  TX: %lu packets, %lu bytes\n", tx_packets, tx_bytes);
        printf("  RX: %lu packets, %lu bytes\n", rx_packets, rx_bytes);
        
        if (rx_packets > 0) {
            printf("  Average message size: %.1f bytes\n", (double)rx_bytes / rx_packets);
        }
    }
    
    microvm_net_channel_destroy(channel);
    microvm_net_cleanup();
    
    return 0;
}
