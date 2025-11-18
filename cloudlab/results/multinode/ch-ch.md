# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```bash
%idle cpu, used memory, throughput
# c6525-25g nodes (enp65s0f0np0)
- 2vCPU(84%), 512MB(87Mi), 12.1 Gbits/s (-P 4)
- 4vCPU(91%), 1024MB(135Mi), 13.3 Gbits/s (-P 4)
# xl170 nodes (ens1f1np1)
- 1vCPU, 512MB
    - throughput: 20.9 Gbits/s (-P 4), idleCPU: 40%, usedMem: 100Mi
    - latency (usec, -m 64), idleCPU: 93%, usedMem: 102Mi 
        - p50: 62.852
        - p90: 70.807
        - p99.9: 228.462
- 4vCPU, 1024MB
    - throughput: 15.4 Gbits/s (-P 4), idleCPU: 90% (didn't spread), usedMem: 115Mi
    - throughput: 16.1 Gbits/s (-P 1), idleCPU: 90% (didn't spread), usedMem: 115Mi
    - latency (usec, -m 64), idleCPU: 97%, usedMem: 124Mi 
        - p50: 295.514
        - p90: 401.123
        - p99.9: 530.097

```

### Syscalls on server process
```

```
### Syscalls on client process
```

```

# Latency
## VM as server responding to VM-sent small requests
```bash
# host
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
# vm
sockperf server -i 192.168.100.2
```
```

```
### Syscalls on server process
```
```
### Syscalls on client process
```
```
