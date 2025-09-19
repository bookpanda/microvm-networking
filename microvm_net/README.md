# MicroVM Minimal Networking Stack

A lightweight, kernel-bypass networking stack designed specifically for microvms, inspired by machnet but optimized for virtualized environments.

## Architecture Overview

This minimal networking stack provides:

1. **Virtio-based packet I/O**: Uses virtio-net for efficient packet processing in microvms
2. **Shared memory channels**: Lock-free ring buffers for application communication
3. **Zero-copy packet handling**: Direct memory mapping for high performance
4. **Minimal protocol stack**: UDP-based messaging with flow control
5. **Simple API**: Socket-like interface for easy adoption

## Key Differences from Machnet

- **No DPDK dependency**: Uses virtio-net and TAP interfaces
- **Simpler buffer management**: Fixed-size buffers with basic pooling
- **Reduced complexity**: Focused on microvm use cases
- **Lightweight**: Minimal memory footprint and CPU overhead

## Components

- `microvm_net.h` - Main API header
- `ring_buffer.h` - Lock-free ring buffer implementation
- `packet_io.h` - Virtio-net packet I/O interface
- `buffer_pool.h` - Simple buffer pool management
- `flow_manager.h` - Basic flow management
- `examples/` - Usage examples and benchmarks

## Build Requirements

- Linux kernel 4.8+ (for virtio-net features)
- GCC 7+ or Clang 6+
- CMake 3.12+

## Performance Targets

- Sub-10μs latency for small messages
- 1M+ packets per second throughput
- <1MB memory footprint per application
