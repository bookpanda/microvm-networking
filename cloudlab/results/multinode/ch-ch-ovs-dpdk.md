# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
- 2vCPU, 512MB, 4 queues, 2.28 Gbits/s
- 2vCPU, 1024MB, 4 queues, 2.27 Gbit/s
- 2vCPU(max=4), 1024MB, 4 queues, 2.38 Gbit/s
- 4vCPU(max=8), 2048MB, 4 queues, 2.45 Gbit/s
- 4vCPU(max=8), 2048MB, 4 queues, 2.45 Gbit/s (iperf3 -P 8)
- 4vCPU(max=8), 4096MB, 8 queues, 2.58 Gbit/s
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
