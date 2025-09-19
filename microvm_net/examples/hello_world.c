/**
 * @file hello_world.c
 * @brief Simple hello world example for microvm networking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include "../include/microvm_net.h"

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -l, --local IP     Local IP address\n");
    printf("  -r, --remote IP    Remote IP address (client mode)\n");
    printf("  -p, --port PORT    Port number (default: 8080)\n");
    printf("  -h, --help         Show this help\n");
    printf("\nExamples:\n");
    printf("  Server: %s --local 192.168.1.10\n", prog_name);
    printf("  Client: %s --local 192.168.1.20 --remote 192.168.1.10\n", prog_name);
}

int main(int argc, char *argv[]) {
    const char *local_ip = NULL;
    const char *remote_ip = NULL;
    uint16_t port = 8080;
    
    static struct option long_options[] = {
        {"local",  required_argument, 0, 'l'},
        {"remote", required_argument, 0, 'r'},
        {"port",   required_argument, 0, 'p'},
        {"help",   no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "l:r:p:h", long_options, NULL)) != -1) {
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
    
    // Create channel
    microvm_net_channel_t *channel = microvm_net_channel_create(NULL);
    if (!channel) {
        fprintf(stderr, "Failed to create networking channel\n");
        microvm_net_cleanup();
        return 1;
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("MicroVM Networking Hello World\n");
    printf("Local IP: %s:%d\n", local_ip, port);
    
    if (remote_ip) {
        // Client mode
        printf("Remote IP: %s:%d\n", remote_ip, port);
        printf("Running in client mode...\n");
        
        microvm_net_flow_t flow;
        if (microvm_net_connect(channel, local_ip, remote_ip, port, &flow) != 0) {
            fprintf(stderr, "Failed to connect to remote endpoint\n");
            goto cleanup;
        }
        
        const char *message = "Hello from microvm client!";
        printf("Sending message: %s\n", message);
        
        if (microvm_net_send_buf(channel, &flow, message, strlen(message)) != 0) {
            fprintf(stderr, "Failed to send message\n");
            goto cleanup;
        }
        
        // Wait for response
        char response[1024];
        microvm_net_flow_t resp_flow;
        
        printf("Waiting for response...\n");
        for (int i = 0; i < 100 && running; i++) {  // Wait up to 10 seconds
            ssize_t received = microvm_net_recv_buf(channel, response, sizeof(response) - 1, &resp_flow);
            if (received > 0) {
                response[received] = '\0';
                printf("Received response: %s\n", response);
                break;
            }
            usleep(100000);  // 100ms
        }
        
    } else {
        // Server mode
        printf("Running in server mode...\n");
        
        if (microvm_net_bind(channel, local_ip, port) != 0) {
            fprintf(stderr, "Failed to bind to local address\n");
            goto cleanup;
        }
        
        printf("Listening for messages...\n");
        
        while (running) {
            char buffer[1024];
            microvm_net_flow_t client_flow;
            
            ssize_t received = microvm_net_recv_buf(channel, buffer, sizeof(buffer) - 1, &client_flow);
            if (received > 0) {
                buffer[received] = '\0';
                printf("Received message: %s\n", buffer);
                
                // Send response
                const char *response = "Hello from microvm server!";
                printf("Sending response: %s\n", response);
                
                if (microvm_net_send_buf(channel, &client_flow, response, strlen(response)) != 0) {
                    fprintf(stderr, "Failed to send response\n");
                }
            }
            
            usleep(10000);  // 10ms
        }
    }
    
    printf("Shutting down...\n");
    
cleanup:
    // Print statistics
    uint64_t tx_packets, rx_packets, tx_bytes, rx_bytes;
    if (microvm_net_get_stats(channel, &tx_packets, &rx_packets, &tx_bytes, &rx_bytes) == 0) {
        printf("\nStatistics:\n");
        printf("  TX: %lu packets, %lu bytes\n", tx_packets, tx_bytes);
        printf("  RX: %lu packets, %lu bytes\n", rx_packets, rx_bytes);
    }
    
    microvm_net_channel_destroy(channel);
    microvm_net_cleanup();
    
    return 0;
}
