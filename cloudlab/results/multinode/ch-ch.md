# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
%idle cpu, %used memory, throughput
- 2vCPU(84%), 512MB(87Mi), 12.1 Gbits/s (-P 4)
- 4vCPU(91%), 1024MB(135Mi), 13.3 Gbits/s (-P 4)

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
