# Throughput
```bash
# node 0
iperf3 -s
# node 1
iperf3 -c 10.10.1.1 -t 30 -P 4
```
```

```
### Syscalls
```

```

# Latency
```bash
# node 0
sockperf server -i 192.168.100.1
# node 1
./sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```
```

```
### Syscalls
```

```