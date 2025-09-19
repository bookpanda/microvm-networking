/**
 * @file benchmark.c
 * @brief Benchmark tool for microvm networking performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "microvm_net.h"

#define MAX_LATENCY_SAMPLES 100000

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

typedef struct {
    double *samples;
    int count;
    int capacity;
} latency_stats_t;

static void latency_stats_init(latency_stats_t *stats, int capacity) {
    stats->samples = malloc(capacity * sizeof(double));
    stats->count = 0;
    stats->capacity = capacity;
}

static void latency_stats_add(latency_stats_t *stats, double latency) {
    if (stats->count < stats->capacity) {
        stats->samples[stats->count++] = latency;
    }
}

static int compare_double(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    return (da > db) - (da < db);
}

static void latency_stats_print(latency_stats_t *stats) {
    if (stats->count == 0) {
        printf("No latency samples collected\n");
        return;
    }
    
    qsort(stats->samples, stats->count, sizeof(double), compare_double);
    
    double sum = 0.0;
    for (int i = 0; i < stats->count; i++) {
        sum += stats->samples[i];
    }
    
    double avg = sum / stats->count;
    double min = stats->samples[0];
    double max = stats->samples[stats->count - 1];
    
    printf("Latency Statistics (%d samples):\n", stats->count);
    printf("  Min:    %8.3f ms\n", min);
    printf("  Avg:    %8.3f ms\n", avg);
    printf("  Max:    %8.3f ms\n", max);
    printf("  P50:    %8.3f ms\n", stats->samples[stats->count * 50 / 100]);
    printf("  P95:    %8.3f ms\n", stats->samples[stats->count * 95 / 100]);
    printf("  P99:    %8.3f ms\n", stats->samples[stats->count * 99 / 100]);
    printf("  P99.9:  %8.3f ms\n", stats->samples[stats->count * 999 / 1000]);
}

static void latency_stats_cleanup(latency_stats_t *stats) {
    free(stats->samples);
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -l, --local IP     Local IP address (required)\n");
    printf("  -r, --remote IP    Remote IP address (server mode if not specified)\n");
    printf("  -p, --port PORT    Port number (default: 8080)\n");
    printf("  -c, --count N      Number of messages (default: 10000)\n");
    printf("  -s, --size BYTES   Message size in bytes (default: 64)\n");
    printf("  -t, --time SEC     Test duration in seconds (overrides count)\n");
    printf("  -w, --warmup SEC   Warmup time in seconds (default: 1)\n");
    printf("  -i, --interval US  Interval between messages in microseconds (default: 0)\n");
    printf("  --no-latency       Skip latency measurements (throughput only)\n");
    printf("  -h, --help         Show this help\n");
    printf("\nModes:\n");
    printf("  Server: %s --local IP\n", prog_name);
    printf("  Client: %s --local IP --remote IP\n", prog_name);
}

int main(int argc, char *argv[]) {
    const char *local_ip = NULL;
    const char *remote_ip = NULL;
    uint16_t port = 8080;
    int message_count = 10000;
    int message_size = 64;
    int test_duration = 0;
    int warmup_time = 1;
    int interval_us = 0;
    int measure_latency = 1;
    
    static struct option long_options[] = {
        {"local",      required_argument, 0, 'l'},
        {"remote",     required_argument, 0, 'r'},
        {"port",       required_argument, 0, 'p'},
        {"count",      required_argument, 0, 'c'},
        {"size",       required_argument, 0, 's'},
        {"time",       required_argument, 0, 't'},
        {"warmup",     required_argument, 0, 'w'},
        {"interval",   required_argument, 0, 'i'},
        {"no-latency", no_argument,       0, 1000},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "l:r:p:c:s:t:w:i:h", long_options, NULL)) != -1) {
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
                break;
            case 't':
                test_duration = atoi(optarg);
                break;
            case 'w':
                warmup_time = atoi(optarg);
                break;
            case 'i':
                interval_us = atoi(optarg);
                break;
            case 1000:
                measure_latency = 0;
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
    
    // Create high-performance channel
    microvm_net_config_t config = {
        .ring_size = 4096,
        .buffer_count = 8192,
        .buffer_size = message_size < 4096 ? 4096 : message_size + 1024,
        .flags = MICROVM_NET_CONFIG_FLAG_ZEROCOPY
    };
    
    microvm_net_channel_t *channel = microvm_net_channel_create(&config);
    if (!channel) {
        fprintf(stderr, "Failed to create networking channel\n");
        microvm_net_cleanup();
        return 1;
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("MicroVM Networking Benchmark\n");
    printf("Configuration:\n");
    printf("  Local IP: %s:%d\n", local_ip, port);
    
    if (remote_ip) {
        // Client mode
        printf("  Remote IP: %s:%d\n", remote_ip, port);
        printf("  Mode: Client\n");
        printf("  Message size: %d bytes\n", message_size);
        
        if (test_duration > 0) {
            printf("  Test duration: %d seconds\n", test_duration);
        } else {
            printf("  Message count: %d\n", message_count);
        }
        
        printf("  Warmup time: %d seconds\n", warmup_time);
        printf("  Interval: %d microseconds\n", interval_us);
        printf("  Latency measurement: %s\n", measure_latency ? "enabled" : "disabled");
        
        // Connect to server
        microvm_net_flow_t flow;
        if (microvm_net_connect(channel, local_ip, remote_ip, port, &flow) != 0) {
            fprintf(stderr, "Failed to connect to server\n");
            goto cleanup;
        }
        
        printf("\nStarting benchmark...\n");
        
        // Allocate buffers
        char *send_buffer = malloc(message_size);
        char *recv_buffer = malloc(message_size);
        if (!send_buffer || !recv_buffer) {
            fprintf(stderr, "Failed to allocate buffers\n");
            goto cleanup;
        }
        
        // Fill send buffer
        for (int i = 0; i < message_size; i++) {
            send_buffer[i] = (char)(i & 0xFF);
        }
        
        latency_stats_t latency_stats;
        if (measure_latency) {
            latency_stats_init(&latency_stats, MAX_LATENCY_SAMPLES);
        }
        
        // Warmup
        printf("Warming up for %d seconds...\n", warmup_time);
        time_t warmup_end = time(NULL) + warmup_time;
        while (time(NULL) < warmup_end && running) {
            microvm_net_send_buf(channel, &flow, send_buffer, message_size);
            microvm_net_flow_t resp_flow;
            microvm_net_recv_buf(channel, recv_buffer, message_size, &resp_flow);
            usleep(1000);  // 1ms
        }
        
        printf("Starting measurement...\n");
        
        struct timespec start_time, end_time, msg_start, msg_end;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        
        int sent = 0, received = 0;
        time_t test_end_time = test_duration > 0 ? time(NULL) + test_duration : 0;
        
        while (running) {
            if (test_duration > 0 && time(NULL) >= test_end_time) {
                break;
            }
            if (test_duration == 0 && sent >= message_count) {
                break;
            }
            
            if (measure_latency) {
                clock_gettime(CLOCK_MONOTONIC, &msg_start);
            }
            
            // Send message
            if (microvm_net_send_buf(channel, &flow, send_buffer, message_size) == 0) {
                sent++;
            }
            
            // Receive response
            microvm_net_flow_t resp_flow;
            ssize_t recv_bytes = microvm_net_recv_buf(channel, recv_buffer, message_size, &resp_flow);
            if (recv_bytes > 0) {
                received++;
                
                if (measure_latency) {
                    clock_gettime(CLOCK_MONOTONIC, &msg_end);
                    double rtt_ms = (msg_end.tv_sec - msg_start.tv_sec) * 1000.0 + 
                                   (msg_end.tv_nsec - msg_start.tv_nsec) / 1000000.0;
                    latency_stats_add(&latency_stats, rtt_ms);
                }
            }
            
            if (interval_us > 0) {
                usleep(interval_us);
            }
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        
        double total_time = (end_time.tv_sec - start_time.tv_sec) + 
                           (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
        
        printf("\nBenchmark Results:\n");
        printf("  Test duration: %.3f seconds\n", total_time);
        printf("  Messages sent: %d\n", sent);
        printf("  Messages received: %d\n", received);
        printf("  Message rate: %.1f msg/sec\n", received / total_time);
        printf("  Throughput: %.3f MB/sec\n", (received * message_size) / total_time / (1024 * 1024));
        
        if (measure_latency) {
            printf("\n");
            latency_stats_print(&latency_stats);
            latency_stats_cleanup(&latency_stats);
        }
        
        free(send_buffer);
        free(recv_buffer);
        
    } else {
        // Server mode
        printf("  Mode: Server\n");
        printf("Press Ctrl+C to stop\n\n");
        
        if (microvm_net_bind(channel, local_ip, port) != 0) {
            fprintf(stderr, "Failed to bind to address\n");
            goto cleanup;
        }
        
        char *buffer = malloc(config.buffer_size);
        if (!buffer) {
            fprintf(stderr, "Failed to allocate buffer\n");
            goto cleanup;
        }
        
        uint64_t message_count = 0;
        time_t last_stats = time(NULL);
        
        while (running) {
            microvm_net_flow_t client_flow;
            ssize_t received = microvm_net_recv_buf(channel, buffer, config.buffer_size, &client_flow);
            
            if (received > 0) {
                message_count++;
                
                // Echo back immediately
                microvm_net_send_buf(channel, &client_flow, buffer, received);
            }
            
            // Print periodic stats
            time_t now = time(NULL);
            if (now - last_stats >= 5) {
                uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
                microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes);
                
                printf("Messages: %lu, Rate: %.1f msg/sec, TX: %lu pkts, RX: %lu pkts\n",
                       message_count, (double)message_count / (now - last_stats + 1),
                       tx_packets, rx_packets);
                
                last_stats = now;
            }
        }
        
        free(buffer);
    }
    
cleanup:
    // Final network statistics
    uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
    if (microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes) == 0) {
        printf("\nNetwork Statistics:\n");
        printf("  TX: %lu packets, %lu bytes\n", tx_packets, tx_bytes);
        printf("  RX: %lu packets, %lu bytes\n", rx_packets, rx_bytes);
    }
    
    microvm_net_channel_destroy(channel);
    microvm_net_cleanup();
    
    return 0;
}
