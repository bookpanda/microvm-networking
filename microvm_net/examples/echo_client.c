/**
 * @file echo_client.c
 * @brief Echo client example for microvm networking
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
    printf("  -r, --remote IP    Remote IP address (required)\n");
    printf("  -p, --port PORT    Port number (default: 8080)\n");
    printf("  -c, --count N      Number of messages to send (default: 10)\n");
    printf("  -s, --size BYTES   Message size in bytes (default: 64)\n");
    printf("  -i, --interval MS  Interval between messages in ms (default: 1000)\n");
    printf("  -v, --verbose      Verbose output\n");
    printf("  -h, --help         Show this help\n");
}

int main(int argc, char *argv[]) {
    const char *local_ip = NULL;
    const char *remote_ip = NULL;
    uint16_t port = 8080;
    int message_count = 10;
    int message_size = 64;
    int interval_ms = 1000;
    int verbose = 0;
    
    static struct option long_options[] = {
        {"local",    required_argument, 0, 'l'},
        {"remote",   required_argument, 0, 'r'},
        {"port",     required_argument, 0, 'p'},
        {"count",    required_argument, 0, 'c'},
        {"size",     required_argument, 0, 's'},
        {"interval", required_argument, 0, 'i'},
        {"verbose",  no_argument,       0, 'v'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "l:r:p:c:s:i:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                local_ip = optarg;
                break;
            case 'r':
                remote_ip = optarg;
                break;
            case 'p':
                port = (uint16_t)atoi(optarg);
                break;
            case 'c':
                message_count = atoi(optarg);
                break;
            case 's':
                message_size = atoi(optarg);
                if (message_size < 1 || message_size > 8192) {
                    fprintf(stderr, "Message size must be between 1 and 8192 bytes\n");
                    return 1;
                }
                break;
            case 'i':
                interval_ms = atoi(optarg);
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
    
    if (!local_ip || !remote_ip) {
        fprintf(stderr, "Error: Both local and remote IP addresses are required\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize networking
    if (microvm_net_init() != 0) {
        fprintf(stderr, "Failed to initialize microvm networking\n");
        return 1;
    }
    
    // Create channel
    microvm_net_config_t config = {
        .ring_size = 1024,
        .buffer_count = 2048,
        .buffer_size = 8192,
        .flags = MICROVM_NET_CONFIG_FLAG_NONE
    };
    
    microvm_net_channel_t *channel = microvm_net_channel_create(&config);
    if (!channel) {
        fprintf(stderr, "Failed to create networking channel\n");
        microvm_net_cleanup();
        return 1;
    }
    
    // Connect to server
    microvm_net_flow_t flow;
    if (microvm_net_connect(channel, local_ip, remote_ip, port, &flow) != 0) {
        fprintf(stderr, "Failed to connect to %s:%d\n", remote_ip, port);
        microvm_net_channel_destroy(channel);
        microvm_net_cleanup();
        return 1;
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Echo Client\n");
    printf("Local: %s, Remote: %s:%d\n", local_ip, remote_ip, port);
    printf("Sending %d messages of %d bytes each (interval: %dms)\n\n", 
           message_count, message_size, interval_ms);
    
    // Prepare message buffer
    char *send_buffer = malloc(message_size);
    char *recv_buffer = malloc(message_size + 1);
    if (!send_buffer || !recv_buffer) {
        fprintf(stderr, "Failed to allocate message buffers\n");
        goto cleanup;
    }
    
    // Fill send buffer with pattern
    for (int i = 0; i < message_size; i++) {
        send_buffer[i] = 'A' + (i % 26);
    }
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    int sent = 0, received = 0;
    double total_rtt = 0.0;
    
    for (int i = 0; i < message_count && running; i++) {
        struct timespec msg_start, msg_end;
        clock_gettime(CLOCK_MONOTONIC, &msg_start);
        
        // Send message
        if (microvm_net_send_buf(channel, &flow, send_buffer, message_size) != 0) {
            fprintf(stderr, "Failed to send message %d\n", i + 1);
            continue;
        }
        sent++;
        
        if (verbose) {
            printf("Sent message %d (%d bytes)\n", i + 1, message_size);
        }
        
        // Wait for echo response
        microvm_net_flow_t resp_flow;
        int timeout_count = 0;
        
        while (timeout_count < 100 && running) {  // 1 second timeout
            ssize_t recv_bytes = microvm_net_recv_buf(channel, recv_buffer, message_size, &resp_flow);
            if (recv_bytes > 0) {
                clock_gettime(CLOCK_MONOTONIC, &msg_end);
                
                double rtt_ms = (msg_end.tv_sec - msg_start.tv_sec) * 1000.0 + 
                               (msg_end.tv_nsec - msg_start.tv_nsec) / 1000000.0;
                total_rtt += rtt_ms;
                received++;
                
                if (verbose) {
                    recv_buffer[recv_bytes] = '\0';
                    printf("Received echo %d (%zd bytes, RTT: %.3f ms)\n", 
                           received, recv_bytes, rtt_ms);
                }
                
                // Verify echo
                if (recv_bytes == message_size && memcmp(send_buffer, recv_buffer, message_size) == 0) {
                    if (verbose) printf("Echo verified successfully\n");
                } else {
                    printf("Warning: Echo mismatch for message %d\n", i + 1);
                }
                
                break;
            }
            
            usleep(10000);  // 10ms
            timeout_count++;
        }
        
        if (timeout_count >= 100) {
            printf("Timeout waiting for echo response %d\n", i + 1);
        }
        
        // Wait for next message interval
        if (i < message_count - 1 && interval_ms > 0) {
            usleep(interval_ms * 1000);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    double total_time = (end_time.tv_sec - start_time.tv_sec) + 
                       (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    
    printf("\nTest Results:\n");
    printf("  Messages sent: %d\n", sent);
    printf("  Messages received: %d\n", received);
    printf("  Success rate: %.1f%%\n", received > 0 ? (double)received / sent * 100.0 : 0.0);
    printf("  Total time: %.3f seconds\n", total_time);
    
    if (received > 0) {
        printf("  Average RTT: %.3f ms\n", total_rtt / received);
        printf("  Message rate: %.1f msg/sec\n", received / total_time);
        printf("  Throughput: %.1f KB/sec\n", (received * message_size) / total_time / 1024.0);
    }
    
    // Network statistics
    uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
    if (microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes) == 0) {
        printf("\nNetwork Statistics:\n");
        printf("  TX: %lu packets, %lu bytes\n", tx_packets, tx_bytes);
        printf("  RX: %lu packets, %lu bytes\n", rx_packets, rx_bytes);
    }
    
cleanup:
    free(send_buffer);
    free(recv_buffer);
    microvm_net_channel_destroy(channel);
    microvm_net_cleanup();
    
    return (received == sent) ? 0 : 1;
}
