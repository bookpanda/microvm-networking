# Network Performance Comparison: Multinode Configurations

## Executive Summary

Comparison of four networking configurations:
- **BM-BM**: Baremetal to Baremetal
- **VM-VM**: MicroVM (Firecracker) to MicroVM
- **KVM-KVM**: Normal KVM to Normal KVM
- **KVM-KVM-vhost**: Normal KVM with vhost to Normal KVM with vhost

---

## 1. Throughput Comparison (iperf3)

### Aggregate Throughput Results

| Configuration | Throughput | Performance vs BM | Retransmissions |
|--------------|------------|-------------------|-----------------|
| **BM-BM** | **23.5 Gbits/sec** | Baseline (100%) | 6,408 |
| **KVM-KVM-vhost** | **13.2 Gbits/sec** | 56.2% | 3,860 |
| **VM-VM** | **10.6 Gbits/sec** | 45.1% | 0 |
| **KVM-KVM** | **9.60 Gbits/sec** | 40.9% | 158 |

### Key Findings:
- **Baremetal** provides the highest throughput at 23.5 Gbits/sec
- **KVM with vhost** achieves 56% of baremetal performance (13.2 Gbits/sec)
- **MicroVM** achieves 45% of baremetal (10.6 Gbits/sec) with zero retransmissions
- **Standard KVM** has the lowest throughput at 41% of baremetal (9.60 Gbits/sec)
- **vhost acceleration** provides ~38% improvement over standard KVM

---

## 2. Latency Comparison (sockperf ping-pong)

### Average Latency Results

| Configuration | Avg Latency | vs BM Overhead | Std Dev | Message Rate |
|--------------|-------------|----------------|---------|--------------|
| **BM-BM** | **32.867 μs** | Baseline | 1.389 μs | ~15,200/sec |
| **KVM-KVM-vhost** | **48.444 μs** | +47.4% (15.6 μs) | 3.239 μs | ~10,300/sec |
| **KVM-KVM** | **65.325 μs** | +98.8% (32.5 μs) | 4.230 μs | ~7,600/sec |
| **VM-VM** | **88.612 μs** | +169.6% (55.7 μs) | 2.427 μs | ~5,600/sec |

### Latency Percentile Comparison

| Percentile | BM-BM | VM-VM | KVM-KVM | KVM-KVM-vhost |
|-----------|-------|-------|---------|---------------|
| **Min** | 31.6 μs | 81.4 μs | 54.0 μs | 38.4 μs |
| **25th** | 32.1 μs | 87.0 μs | 62.8 μs | 46.5 μs |
| **50th (Median)** | 32.3 μs | 88.2 μs | 64.7 μs | 48.1 μs |
| **75th** | 33.2 μs | 89.8 μs | 66.9 μs | 50.1 μs |
| **90th** | 34.7 μs | 91.4 μs | 69.9 μs | 52.0 μs |
| **99th** | 37.9 μs | 96.6 μs | 77.9 μs | 57.1 μs |
| **99.9th** | 44.7 μs | 103.9 μs | 107.8 μs | 65.0 μs |
| **Max** | 143.9 μs | 186.4 μs | 235.3 μs | 576.6 μs |

### Key Findings:
- **Baremetal** provides the lowest latency at ~33 μs
- **KVM with vhost** adds ~15.6 μs overhead (47% increase)
- **Standard KVM** adds ~32.5 μs overhead (99% increase, nearly doubles latency)
- **MicroVM** has the highest latency at ~89 μs (170% increase)
- **vhost** provides 26% latency reduction vs standard KVM (65.3 → 48.4 μs)
- **MicroVM** has tighter variance (std dev 2.4) despite higher absolute latency

---

## 3. Syscall Composition Analysis

### 3.1 Throughput Test (iperf3) - Server Process

#### BM-BM Server Syscalls
```
Key syscalls (Top 5):
1. pselect6: 1,895,163  (50.0%)
2. read:     1,895,228  (50.0%)
3. getsockopt: 130
4. write: 12
5. close: 15

Total dominant: ~3.79M syscalls
```

#### VM-VM Server Syscalls
```
Key syscalls (Top 5):
1. readv:       617,340  (37.5%)
2. writev:      266,526  (16.2%)
3. ioctl:       163,632  (10.0%) [fc_vcpu 0]
4. epoll_pwait: 138,015  (8.4%)
5. write:       124,294  (7.6%)

Total dominant: ~1.31M syscalls
Additional: read: 123,565
```

#### KVM-KVM Server Syscalls
```
Key syscalls (Top 5):
1. ioctl:  724,154  (39.0%)
2. read:   606,631  (32.7%)
3. writev: 554,089  (29.9%)
4. ppoll:   14,604  (0.8%)
5. futex:   11,480  (0.6%)

Total dominant: ~1.91M syscalls
```

#### KVM-KVM-vhost Server Syscalls
```
Key syscalls (Top 5):
1. ioctl:  84,007   (85.9%)
2. writev: 13,930   (14.2%)
3. futex:     998   (1.0%)
4. write:     404   (0.4%)
5. read:      297   (0.3%)

Total: ~97,800 syscalls
```

**Analysis:**
- **BM-BM**: Uses simple `read`/`pselect6` pattern, ~3.79M syscalls
- **VM-VM**: Vectorized I/O (`readv`/`writev`) with epoll, ~1.31M syscalls (65% reduction vs BM)
- **KVM-KVM**: High `ioctl` overhead (724K), ~1.91M syscalls  
- **KVM-KVM-vhost**: Dramatically fewer syscalls (~98K, 95% reduction vs standard KVM), dominated by `ioctl` (86%)
- **vhost benefit**: 95% syscall reduction through kernel-level packet processing

---

### 3.2 Throughput Test (iperf3) - Client Process

#### BM-BM Client Syscalls
```
Key syscalls:
1. write: 646,190   (99.8%)
2. getsockopt: 116
3. pselect6: 32

Total: ~646,400 syscalls
```

#### VM-VM Client Syscalls
```
Key syscalls:
1. writev:      612,158  (46.4%)
2. readv:       393,557  (29.8%)
3. ioctl:       252,548  (19.1%) [fc_vcpu 0]
4. epoll_pwait: 191,669  (14.5%)
5. write:       171,307  (13.0%)

Total dominant: ~1.62M syscalls
Additional: read: 88,554
```

#### KVM-KVM Client Syscalls
```
Key syscalls:
1. writev: 605,786  (42.7%)
2. read:   588,143  (41.5%)
3. ioctl:  210,378  (14.8%)
4. ppoll:   60,757  (4.3%)
5. futex:    4,542  (0.3%)

Total dominant: ~1.47M syscalls
```

#### KVM-KVM-vhost Client Syscalls
```
Key syscalls:
1. ioctl:  84,795   (85.2%)
2. writev: 14,737   (14.8%)
3. futex:   1,542   (1.5%)
4. write:     512   (0.5%)
5. read:      271   (0.3%)

Total: ~99,500 syscalls
```

**Analysis:**
- **BM-BM**: Extremely efficient, ~646K `write` syscalls dominate
- **VM-VM/KVM-KVM**: Similar patterns with vectorized I/O, ~1.5M syscalls
- **KVM-vhost**: 95% syscall reduction on client side as well

---

### 3.3 Latency Test (sockperf) - Server Process

#### BM-BM Server Syscalls
```
1. recvfrom: 455,731  (50.0%)
2. sendto:   455,731  (50.0%)

Total: 911,462 syscalls
Ratio: 1:1 (perfectly balanced)
Messages: 455,731
```

#### VM-VM Server Syscalls
```
1. ioctl:       676,756  (35.0%) [fc_vcpu 0]
2. readv:       343,511  (17.8%)
3. epoll_pwait: 343,506  (17.8%)
4. write:       338,380  (17.5%)
5. read:        174,316  (9.0%)
6. writev:      169,190  (8.8%)

Total: ~1.93M syscalls
Messages: 169,184
Syscalls per message: 11.4
```

#### KVM-KVM Server Syscalls
```
1. ppoll:  688,115  (36.1%)
2. read:   688,112  (36.1%)
3. ioctl:  462,795  (24.3%)
4. writev: 229,370  (12.0%)
5. futex:      543  (0.03%)

Total: ~1.91M syscalls
Messages: 229,367
Syscalls per message: 8.3
```

#### KVM-KVM-vhost Server Syscalls
```
1. ioctl: 2,052  (100%)

Total: 2,052 syscalls
Messages: 308,991
Syscalls per message: 0.007
```

**Analysis:**
- **BM-BM**: Minimal overhead, 2 syscalls per message (recvfrom + sendto)
- **VM-VM**: 11.4 syscalls per message (5.7x overhead)
- **KVM-KVM**: 8.3 syscalls per message (4.2x overhead)
- **KVM-vhost**: Nearly zero syscall overhead (0.007 per message) - 99.7% reduction!
- **vhost advantage**: Kernel-space packet processing eliminates per-packet syscalls

---

### 3.4 Latency Test (sockperf) - Client Process

#### BM-BM Client Syscalls
```
Key syscalls:
1. recvfrom: 455,731  (50.0%)
2. sendto:   455,731  (50.0%)
Plus minimal control syscalls: ~40 total

Total: ~911,502 syscalls
```

#### VM-VM Client Syscalls
```
Key syscalls:
1. ioctl:       676,796  (35.0%) [fc_vcpu 0]
2. epoll_pwait: 343,531  (17.8%)
3. readv:       343,512  (17.8%)
4. write:       338,416  (17.5%)
5. read:        174,558  (9.0%)
6. writev:      169,190  (8.8%)

Total: ~1.93M syscalls
```

#### KVM-KVM Client Syscalls
```
Key syscalls:
1. ppoll:  688,267  (36.1%)
2. read:   688,273  (36.1%)
3. ioctl:  469,373  (24.6%)
4. writev: 230,887  (12.1%)
5. futex:      702  (0.04%)

Total: ~1.91M syscalls
```

#### KVM-KVM-vhost Client Syscalls
```
Key syscalls:
1. ioctl:  10,995   (87.9%)
2. writev:  1,517   (12.1%)
3. futex:     489   (3.9%)
4. write:     446   (3.6%)
5. read:      156   (1.2%)

Total: ~12,500 syscalls
```

**Analysis:**
- **BM-BM**: 2 syscalls per message, symmetric with server
- **VM-VM/KVM-KVM**: Both ~1.91M syscalls, client-side matches server overhead
- **KVM-vhost**: 99.4% syscall reduction vs standard KVM (1.91M → 12.5K)

---

## 4. Key Insights & Recommendations

### Performance Ranking

**Throughput (Best → Worst):**
1. Baremetal: 23.5 Gbits/sec
2. KVM+vhost: 13.2 Gbits/sec (56% of BM)
3. MicroVM: 10.6 Gbits/sec (45% of BM)
4. Standard KVM: 9.60 Gbits/sec (41% of BM)

**Latency (Best → Worst):**
1. Baremetal: 32.9 μs
2. KVM+vhost: 48.4 μs (+47%)
3. Standard KVM: 65.3 μs (+99%)
4. MicroVM: 88.6 μs (+170%)

**Syscall Efficiency (Latency Test):**
1. Baremetal: 2.0 syscalls/message
2. KVM+vhost: 0.007 syscalls/message (vhost kernel handling)
3. Standard KVM: 8.3 syscalls/message
4. MicroVM: 11.4 syscalls/message

### Technology Trade-offs

#### Baremetal (BM-BM)
- ✅ Best throughput and latency
- ✅ Minimal syscall overhead
- ❌ No isolation or multi-tenancy
- ❌ No live migration

#### KVM with vhost (KVM-KVM-vhost)
- ✅ Best virtualized performance (56% throughput, 47% latency overhead)
- ✅ Excellent syscall efficiency in latency workloads
- ✅ Full hardware virtualization features
- ⚠️ Requires vhost-net kernel module
- ⚠️ Higher memory footprint than MicroVM

#### MicroVM (VM-VM)
- ✅ Zero retransmissions in throughput test
- ✅ Good isolation with minimal footprint
- ✅ Fast boot times
- ❌ Higher latency than KVM+vhost (83% worse)
- ⚠️ Lower throughput than KVM+vhost (20% less)
- ⚠️ Higher syscall count per message (11.4 vs 8.3)

#### Standard KVM (KVM-KVM)
- ✅ Better latency than MicroVM
- ⚠️ Lowest throughput of virtualized options
- ⚠️ High ioctl overhead (724K in throughput test)
- ❌ Should use vhost instead for production

### Recommendations

1. **For Production Network-Intensive Workloads**: 
   - Use **KVM with vhost** - best balance of performance and features
   - Achieves 56% of baremetal throughput with only 47% latency overhead

2. **For Low-Latency Requirements** (< 50 μs target):
   - Only **baremetal** meets this consistently
   - **KVM+vhost** is the closest virtualized option at 48.4 μs

3. **For Microservices/Serverless**:
   - **MicroVM** is acceptable if latency tolerance > 80 μs
   - Fast boot times and isolation may outweigh performance gap

4. **Avoid Standard KVM**: 
   - Always enable vhost for significant performance improvement
   - 38% throughput gain and 26% latency reduction over standard KVM

### Unexpected Finding: vhost Syscall Efficiency

The **KVM+vhost** configuration shows a remarkable characteristic: in latency tests, it requires only **0.007 syscalls per message**, far fewer than even baremetal (2.0). This is because vhost-net handles packet processing entirely in kernel space, requiring syscalls only for control operations, not per-packet processing.

This makes vhost extremely efficient for low-latency, high-frequency workloads despite having higher absolute latency than baremetal.

---

## Appendix: Raw Data Sources

- `bm-bm.md` - Baremetal configuration
- `vm-vm.md` - MicroVM (Firecracker) configuration  
- `kvm-kvm.md` - Standard KVM configuration
- `kvm-kvm-vhost.md` - KVM with vhost-net configuration

