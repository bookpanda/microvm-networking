# Throughput
## VM as server responding to bulk data from a VM
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
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_socket]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_rt_sigaction]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_bind]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_lseek]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_listen]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_clock_gettime]: 2
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_newfstat]: 2
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getrusage]: 2
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_setsockopt]: 3
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_unlink]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_munmap]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_madvise]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_ftruncate]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_mprotect]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_exit]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_clone3]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_set_robust_list]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_rseq]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_accept]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getpeername]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_openat]: 6
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_mmap]: 8
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getsockname]: 9
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_write]: 12
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_close]: 15
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 17
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getsockopt]: 130
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_pselect6]: 1895163
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_read]: 1895228
```
### Syscalls on client process
```
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getrusage]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_mprotect]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_exit_group]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_clock_gettime]: 1
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_newfstat]: 2
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_openat]: 2
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_madvise]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_exit]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_tgkill]: 4
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_futex]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_read]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_mmap]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_rt_sigaction]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_munmap]: 5
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getpid]: 8
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_close]: 11
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 12
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_pselect6]: 32
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_getsockopt]: 116
[STDOUT] @total[iperf3, tracepoint:syscalls:sys_enter_write]: 646190
```

# Latency
## VM as server responding to VM-sent small requests
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
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_recvfrom]: 455731
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_sendto]: 455731
```
### Syscalls on client process
```
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_clock_nanosleep]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_close]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_rt_sigreturn]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_setitimer]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_exit_group]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_rt_sigaction]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_gettid]: 1
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_mmap]: 2
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_munmap]: 4
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_write]: 23
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_recvfrom]: 455731
[STDOUT] @total[sockperf, tracepoint:syscalls:sys_enter_sendto]: 455731
```
