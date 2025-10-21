# Throughput
## BM as server responding to bulk data from a BM
```bash
# server
iperf3 -s
# client
iperf3 -c 10.10.1.1 -t 30 -P 4
```
```
[STDOUT] [ ID] Interval           Transfer     Bitrate         Retr
[STDOUT] [  5]   0.00-30.00  sec  21.7 GBytes  6.21 Gbits/sec    0             sender
[STDOUT] [  5]   0.00-30.00  sec  21.7 GBytes  6.21 Gbits/sec                  receiver
[STDOUT] [  7]   0.00-30.00  sec  19.4 GBytes  5.56 Gbits/sec  3379             sender
[STDOUT] [  7]   0.00-30.00  sec  19.4 GBytes  5.56 Gbits/sec                  receiver
[STDOUT] [  9]   0.00-30.00  sec  19.5 GBytes  5.59 Gbits/sec  3029             sender
[STDOUT] [  9]   0.00-30.00  sec  19.5 GBytes  5.59 Gbits/sec                  receiver
[STDOUT] [ 11]   0.00-30.00  sec  21.6 GBytes  6.18 Gbits/sec    0             sender
[STDOUT] [ 11]   0.00-30.00  sec  21.6 GBytes  6.18 Gbits/sec                  receiver
[STDOUT] [SUM]   0.00-30.00  sec  82.2 GBytes  23.5 Gbits/sec  6408             sender
[STDOUT] [SUM]   0.00-30.00  sec  82.2 GBytes  23.5 Gbits/sec                  receiver
```
### Syscalls on server process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_write]: 12
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_read]: 1696405
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_pselect6]: 1696411
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_write]: 244720
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_read]: 2175322349
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_pselect6]: 70482702503
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_read]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_futex]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_pselect6]: 33
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_write]: 668367
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_read]: 34324
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_futex]: 311545
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_pselect6]: 28993222173
[STDOUT] @time[iperf3, tracepoint:syscalls:sys_exit_write]: 117289405325
```

# Latency
## BM as server responding to BM-sent small requests
```bash
# server
sockperf server -i 10.10.1.2
# client
sockperf ping-pong -i 10.10.1.2 -m 64 -t 30
```
```
[STDOUT] sockperf: [Total Run] RunTime=30.000 sec; Warm up time=400 msec; SentMessages=455731; ReceivedMessages=455730
[STDOUT] sockperf: ========= Printing statistics for Server No: 0
[STDOUT] sockperf: [Valid Duration] RunTime=29.550 sec; SentMessages=449213; ReceivedMessages=449213
[STDOUT] sockperf: [2;35m====> avg-latency=32.867 (std-dev=1.389)[0m
[STDOUT] sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
[STDOUT] sockperf: Summary: Latency is 32.867 usec
[STDOUT] sockperf: [2;35mTotal 449213 observations[0m; each percentile contains 4492.13 observations
[STDOUT] sockperf: ---> <MAX> observation =  143.884
[STDOUT] sockperf: ---> percentile 99.999 =   59.501
[STDOUT] sockperf: ---> percentile 99.990 =   49.778
[STDOUT] sockperf: ---> percentile 99.900 =   44.733
[STDOUT] sockperf: ---> percentile 99.000 =   37.930
[STDOUT] sockperf: ---> percentile 90.000 =   34.654
[STDOUT] sockperf: ---> percentile 75.000 =   33.182
[STDOUT] sockperf: ---> percentile 50.000 =   32.300
[STDOUT] sockperf: ---> percentile 25.000 =   32.054
[STDOUT] sockperf: ---> <MIN> observation =   31.554
```
### Syscalls on server process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_sendto]: 679717
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_recvfrom]: 679717
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[sockperf, tracepoint:syscalls:sys_exit_sendto]: 4031180067
[STDOUT] @time[sockperf, tracepoint:syscalls:sys_exit_recvfrom]: 24424085030
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_write]: 30
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_sendto]: 679717
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_recvfrom]: 679717
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[sockperf, tracepoint:syscalls:sys_exit_write]: 136716
[STDOUT] @time[sockperf, tracepoint:syscalls:sys_exit_sendto]: 4042726944
[STDOUT] @time[sockperf, tracepoint:syscalls:sys_exit_recvfrom]: 24407682871
```
