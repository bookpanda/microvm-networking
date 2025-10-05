# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 10.10.1.1 -t 30 -P 4
```
```

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
