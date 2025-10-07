# Latency
```bash
# host: send 64-byte messages for 30s
sockperf ping-pong -i 127.0.0.1 -p 5201 -m 64 -t 30 -T tcp

# vm: act as server
sockperf server -i 127.0.0.1
```

# Throughput
```bash
# host
iperf3 -s

# vm: 30s, 4 threads
iperf3 -c 192.168.100.1 -t 30 -P 4

iperf3 -c 127.0.0.1 -p 5201 -t 30 -P 4
```
