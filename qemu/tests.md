# Latency
```bash
# vm: act as server
sockperf server -i 192.168.100.2
# host: send 64-byte messages for 30s
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
```

# Throughput
```bash
# vm: act as server
iperf3 -s
# host: 30s, 4 threads
iperf3 -c 192.168.100.2 -t 30 -P 4
```

# Syscalls
```bash
# get pid of process with most cpu usage
ps aux | grep qemu
~/code/microvm-networking/benchmark/trace_syscalls.sh <pid>
```