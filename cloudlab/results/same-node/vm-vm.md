# Throughput
## VM as client sending bulk data to a host or another VM
```bash
# host
iperf3 -s
# vm
iperf3 -c 192.168.100.1 -t 30 -P 4
```
```

```
### Syscalls
```

```

# Latency
## VM as client sending small requests to a host or another VM
```bash
# host
sockperf server -i 192.168.100.1
# vm
./sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```
```

```
### Syscalls
```

```