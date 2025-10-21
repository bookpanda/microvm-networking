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

## 4. Syscall Timing Analysis & Performance Bottlenecks

### 4.1 Throughput Test (iperf3) - Server Process Timing

#### BM-BM Server Syscall Timing Analysis
```
Syscall Times (cumulative):
1. pselect6: 70,482,702,503 ns (96.9%) - 1,696,411 calls = 41.5 μs/call
2. read:      2,175,322,349 ns (3.0%)  - 1,696,405 calls = 1.3 μs/call  
3. write:       244,720 ns (0.0%)      - 12 calls      = 20.4 μs/call

Key Insight: pselect6 dominates with 97% of time but at reasonable per-call cost
```

#### VM-VM Server Syscall Timing Analysis  
```
Syscall Times (cumulative):
1. fc_vcpu 0 ioctl: 30,335,638,095 ns (48.0%) - 133,010 calls = 228.1 μs/call
2. firecracker readv: 21,199,346,769 ns (33.6%) - 617,066 calls = 34.4 μs/call
3. firecracker epoll_pwait: 3,233,888,438 ns (5.1%) - 128,282 calls = 25.2 μs/call
4. firecracker writev: 2,259,799,787 ns (3.6%) - 266,881 calls = 8.5 μs/call
5. firecracker write: 414,686,000 ns (0.7%) - 125,880 calls = 3.3 μs/call

Key Insights:
- ioctl is EXTREMELY expensive: 228.1 μs/call vs ~34 μs for readv
- VCPU ioctl calls consume 48% of total syscall time with only 10% of calls
- Vectorized I/O (readv/writev) provides good efficiency when not blocked by ioctl
```

#### KVM-KVM Server Syscall Timing Analysis
```
Syscall Times (cumulative):
1. ioctl: 110,542,040,610 ns (59.1%) - 699,450 calls = 158.1 μs/call
2. ppoll:  19,119,544,714 ns (10.2%) - 11,627 calls  = 1,644.2 μs/call
3. futex:  17,068,550,849 ns (9.1%)  - 3,119 calls   = 5,472.4 μs/call
4. read:   14,326,391,004 ns (7.7%)  - 544,061 calls = 26.3 μs/call
5. writev:  4,299,471,821 ns (2.3%)  - 522,637 calls = 8.2 μs/call

Key Insights:
- futex is MASSIVELY expensive: 5,472 μs/call (5.5ms per call!)
- ppoll also very expensive: 1,644 μs/call (1.6ms per call)
- ioctl expensive but frequent: 158 μs/call, 59% of total time
- Standard I/O (read/writev) quite efficient: 8-26 μs/call
```

#### KVM-KVM-vhost Server Syscall Timing Analysis
```
Syscall Times (cumulative):
1. ioctl: 125,809,364,363 ns (46.5%) - 80,249 calls = 1,567.6 μs/call
2. futex:  66,883,668,239 ns (24.7%) - 498 calls    = 134,304.9 μs/call (134ms!)
3. ppoll:  63,230,753,777 ns (23.4%) - 103 calls    = 613,891.0 μs/call (614ms!)
4. writev:    56,916,086 ns (0.0%)   - 13,715 calls = 4.1 μs/call
5. write:      488,593 ns (0.0%)    - 90 calls     = 5.4 μs/call

Key Insights:
- futex and ppoll are EXTREMELY expensive: 134ms and 614ms per call respectively
- These are likely blocking operations waiting for I/O or locks
- ioctl expensive but manageable: 1.6ms/call
- Data I/O (write/writev) very efficient: 4-5 μs/call when not blocked
```

### 4.2 Latency Test (sockperf) - Server Process Timing

#### BM-BM Latency Server Timing
```
Syscall Times (cumulative):
1. recvfrom: 24,424,085,030 ns (85.5%) - 679,717 calls = 35.9 μs/call
2. sendto:    4,031,180,067 ns (14.5%) - 679,717 calls = 5.9 μs/call

Key Insight: Perfect 1:1 ratio, recvfrom takes 6x longer than sendto (35.9 vs 5.9 μs)
```

#### VM-VM Latency Server Timing
```
Syscall Times (cumulative):
1. fc_vcpu 0 ioctl: 36,590,728,696 ns (70.8%) - 685,646 calls = 53.4 μs/call
2. firecracker epoll_pwait: 31,338,106,034 ns (27.7%) - 348,020 calls = 90.1 μs/call
3. firecracker write: 1,132,353,780 ns (1.0%) - 342,828 calls = 3.3 μs/call
4. firecracker writev: 1,278,204,220 ns (1.1%) - 171,416 calls = 7.5 μs/call

Key Insights:
- VCPU ioctl dominates: 71% of time, 53.4 μs/call average
- epoll_pwait expensive: 90.1 μs/call but necessary for event multiplexing
- Data writes very efficient: 3-7.5 μs/call
```

#### KVM-KVM Latency Server Timing  
```
Syscall Times (cumulative):
1. ioctl: 117,812,742,814 ns (56.3%) - 402,168 calls = 293.0 μs/call
2. ppoll:   39,430,160,272 ns (18.8%) - 598,986 calls = 65.8 μs/call
3. futex:   30,335,901,535 ns (14.5%) - 314 calls = 96,693.9 μs/call (97ms!)
4. read:     1,204,390,920 ns (0.6%)  - 598,985 calls = 2.0 μs/call
5. writev:  1,823,267,592 ns (0.9%)  - 199,650 calls = 9.1 μs/call

Key Insights:
- futex extremely expensive: 97ms per call - major bottleneck
- ioctl expensive: 293 μs/call but frequent (56% of time)
- ppoll reasonable: 65.8 μs/call for event polling
- Actual I/O efficient: 2-9 μs/call for read/writev
```

#### KVM-KVM-vhost Latency Server Timing
```
Syscall Times (cumulative):
1. ioctl: 122,972,830,112 ns (100%) - 1,824 calls = 67,440.0 μs/call (67.4ms!)

Key Insight: 
- Only 1,824 ioctl calls total but each takes 67.4ms
- This explains the 67.4ms average latency despite only 0.007 syscalls/message
- Each ioctl likely represents a significant VCPU operation or kernel transition
```

### 4.3 Expensive Syscall Patterns & Bottlenecks

#### Most Expensive Syscalls by Latency Impact

**Extremely Expensive (>100ms per call):**
1. **futex** in KVM-KVM-vhost latency: 134ms/call (blocking operations)
2. **futex** in KVM-KVM latency: 96.7ms/call (synchronization primitives)
3. **ppoll** in KVM-KVM-vhost throughput: 614ms/call (I/O blocking)

**Very Expensive (1-100ms per call):**
1. **ioctl** in KVM-KVM-vhost latency: 67.4ms/call (VCPU operations)
2. **ioctl** in KVM-KVM latency: 293μs/call (but frequent - major bottleneck)
3. **ppoll** in KVM-KVM latency: 1.64ms/call (event polling)

**Expensive but Manageable (100μs-1ms per call):**
1. **ioctl** in KVM-KVM-vhost throughput: 1.57ms/call 
2. **ioctl** in VM-VM scenarios: 53-228μs/call depending on load

#### Key Performance Bottleneck Insights

1. **VCPU ioctl Dominance**: VCPU-related ioctl calls are the primary performance bottleneck in virtualized environments, consuming 46-71% of syscall time across all configurations.

2. **Blocking Synchronization**: futex calls in KVM environments (particularly latency tests) show extreme blocking times (96-134ms), indicating potential synchronization or resource contention issues.

3. **Event Polling Overhead**: ppoll/epoll_pwait calls vary widely in cost:
   - Efficient in VM-VM: 25-90μs/call
   - Expensive in KVM: 654μs-1.6ms/call
   - Extremely expensive in KVM-vhost: 614ms/call

4. **vhost Paradox**: KVM-vhost shows both the best (0.007 syscalls/message) and worst (67.4ms/ioctl) characteristics - extremely efficient for data path but expensive for control operations.

5. **Baremetal Efficiency**: BM-BM shows consistent, predictable syscall timing with no extreme outliers, explaining its superior performance.

---

## 5. Key Insights & Recommendations

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
- ⚠️ High ioctl overhead (724K in throughput test, 158μs/call average)
- ❌ Extreme futex blocking (5.5ms-97ms per call) causes major latency spikes
- ❌ Should use vhost instead for production

### Recommendations

1. **For Production Network-Intensive Workloads**: 
   - Use **KVM with vhost** - best balance of performance and features
   - Achieves 56% of baremetal throughput with only 47% latency overhead
   - **Warning**: Monitor VCPU ioctl latency (67ms/call in latency tests can cause spikes)

2. **For Low-Latency Requirements** (< 50 μs target):
   - Only **baremetal** meets this consistently
   - **KVM+vhost** is the closest virtualized option at 48.4 μs

3. **For Microservices/Serverless**:
   - **MicroVM** is acceptable if latency tolerance > 80 μs
   - Fast boot times and isolation may outweigh performance gap

4. **Avoid Standard KVM**: 
   - Always enable vhost for significant performance improvement
   - 38% throughput gain and 26% latency reduction over standard KVM
   - Standard KVM shows extreme futex blocking (96ms/call) and expensive ppoll (1.6ms/call)

5. **Critical Performance Monitoring**:
   - **VCPU ioctl calls** are the #1 bottleneck in all virtualized environments (46-71% of syscall time)
   - **futex synchronization** in KVM can cause 96-134ms blocking delays - investigate contention
   - **ppoll/epoll** overhead varies dramatically: 25μs (VM) vs 614ms (KVM-vhost) per call
   - Focus optimization efforts on reducing VCPU context switches and synchronization bottlenecks

### Unexpected Findings: vhost Efficiency & Timing Paradoxes

#### vhost Syscall Efficiency
The **KVM+vhost** configuration shows a remarkable characteristic: in latency tests, it requires only **0.007 syscalls per message**, far fewer than even baremetal (2.0). This is because vhost-net handles packet processing entirely in kernel space, requiring syscalls only for control operations, not per-packet processing.

#### vhost Timing Paradox
However, **KVM-vhost shows extreme timing inconsistencies**:
- **Best case**: 4-5 μs per data I/O syscall (write/writev)
- **Worst case**: 67.4ms per ioctl call in latency tests, 614ms per ppoll call in throughput tests
- This explains why vhost has excellent average performance but can suffer from latency spikes

#### Synchronization Bottlenecks
**futex calls** show the most extreme blocking behavior across virtualized environments:
- BM-BM: No significant futex usage (311ns total)
- KVM-KVM: 96.7ms average per futex call (synchronization primitives)
- KVM-vhost: 134ms average per futex call (resource contention)

This suggests that virtualization overhead manifests primarily through synchronization bottlenecks rather than pure I/O inefficiency.

---

## Appendix: Raw Data Sources

- `bm-bm.md` - Baremetal configuration
- `vm-vm.md` - MicroVM (Firecracker) configuration  
- `kvm-kvm.md` - Standard KVM configuration
- `kvm-kvm-vhost.md` - KVM with vhost-net configuration

