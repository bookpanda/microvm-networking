# Performance Analysis: Why OVS-DPDK is Slower than Vanilla

## Results Comparison

| Setup | Throughput | Hardware | Network Path |
|-------|-----------|----------|--------------|
| **Vanilla** | 12.2 Gbps | 2 physical hosts | Physical NIC + kernel network stack |
| **OVS-DPDK** | 2.58 Gbps | Same host | OVS-DPDK + vhost-user |

**OVS-DPDK is 4.7x SLOWER!** This is completely wrong - it should be 2-3x FASTER.

## Root Causes

### 1. Massive CPU Contention
```
PMD threads:  Cores 0-7 (8 threads)
DPDK lcore:   Cores 8-9 (2 threads)  
VM1:          Not pinned (floating)
VM2:          Not pinned (floating)
System:       Using all cores

RESULT: All processes fighting for CPU, constant context switches
```

### 2. Too Many PMD Threads
- 8 PMD threads for 2 VMs = 4 PMD threads per VM
- Each PMD thread tries to lock the same vhost-user queue
- Lock contention + cache thrashing = SLOW

### 3. No Core Isolation
- Kernel scheduler moves VMs and PMD threads around
- Cache misses, NUMA issues, interrupt handling overhead
- PMD threads should be polling 100%, but getting preempted

### 4. Inefficient Memory Allocation
- 8GB allocated to OVS-DPDK is excessive
- More memory = more cache pressure

## Expected vs Actual Performance

### Vanilla (12.2 Gbps)
✅ Dedicated CPU per VM (no contention)
✅ Hardware NIC offloads (TSO, checksum, etc)
✅ Kernel optimized for this path
❌ Physical NIC bottleneck (10 Gbps limit)
❌ Kernel network stack overhead

### OVS-DPDK (Should be 20-30+ Gbps)
✅ No physical NIC bottleneck
✅ Zero-copy vhost-user
✅ DPDK poll mode (no interrupts)
❌ **YOUR SETUP**: Massive CPU contention
❌ **YOUR SETUP**: No CPU pinning
❌ **YOUR SETUP**: Lock contention from too many PMD threads

## The Fix: Proper CPU Isolation

### Optimal Core Allocation (32 cores available)
```
Cores 0-1:   System/OS
Cores 2-3:   OVS PMD threads (2 cores = 1 per VM)
Cores 4:     OVS DPDK lcore
Cores 5-8:   VM1 (4 vCPUs, pinned)
Cores 9-12:  VM2 (4 vCPUs, pinned)
Cores 13-31: Reserved
```

### Expected Performance After Fix
- **Throughput**: 20-40 Gbps (limited by vhost-user, not NIC)
- **Latency**: 10-30 µs (much better than physical NIC's 80-100 µs)
- **CPU efficiency**: Each core dedicated, no context switches

## Implementation

See `optimized_setup.sh` for the proper configuration.

Key changes:
1. Reduce PMD threads from 8 to 2 (mask: 0xC = cores 2-3)
2. Pin VMs to specific cores (taskset)
3. Add isolcpus to kernel boot parameters
4. Reduce OVS memory from 8GB to 2GB

