# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-30.00  sec  10.8 GBytes  3.09 Gbits/sec    1             sender
[  5]   0.00-30.05  sec  10.8 GBytes  3.08 Gbits/sec                  receiver
[  7]   0.00-30.00  sec  10.7 GBytes  3.07 Gbits/sec    1             sender
[  7]   0.00-30.05  sec  10.7 GBytes  3.06 Gbits/sec                  receiver
[  9]   0.00-30.00  sec  10.8 GBytes  3.08 Gbits/sec    0             sender
[  9]   0.00-30.05  sec  10.8 GBytes  3.08 Gbits/sec                  receiver
[ 11]   0.00-30.00  sec  10.2 GBytes  2.93 Gbits/sec   32             sender
[ 11]   0.00-30.05  sec  10.2 GBytes  2.92 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec  42.5 GBytes  12.2 Gbits/sec   34             sender
[SUM]   0.00-30.05  sec  42.5 GBytes  12.1 Gbits/sec                  receiver
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
