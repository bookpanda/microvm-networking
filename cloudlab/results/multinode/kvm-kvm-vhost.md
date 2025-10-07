# Throughput
## KVM as server responding to bulk data from a KVM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-30.00  sec  11.7 GBytes  3.35 Gbits/sec  1107             sender
[  5]   0.00-30.04  sec  11.7 GBytes  3.34 Gbits/sec                  receiver
[  7]   0.00-30.00  sec  11.6 GBytes  3.32 Gbits/sec  927             sender
[  7]   0.00-30.04  sec  11.6 GBytes  3.31 Gbits/sec                  receiver
[  9]   0.00-30.00  sec  11.1 GBytes  3.17 Gbits/sec  1203             sender
[  9]   0.00-30.04  sec  11.1 GBytes  3.16 Gbits/sec                  receiver
[ 11]   0.00-30.00  sec  11.8 GBytes  3.37 Gbits/sec  623             sender
[ 11]   0.00-30.04  sec  11.8 GBytes  3.37 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec  46.1 GBytes  13.2 Gbits/sec  3860             sender
[SUM]   0.00-30.04  sec  46.1 GBytes  13.2 Gbits/sec                  receiver
```
### Syscalls on server process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mprotect]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mmap]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_preadv]: 7
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_fdatasync]: 10
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwritev]: 16
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 22
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_munmap]: 26
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 34
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 61
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pread64]: 79
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 296
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 297
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 404
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 998
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 13930
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 84007
```
### Syscalls on client process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwritev]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_fdatasync]: 8
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 10
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_preadv]: 10
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mprotect]: 26
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mmap]: 26
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 47
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pread64]: 77
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 80
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 266
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 271
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 512
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 1542
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 14737
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 84795
```

# Latency
## KVM as server responding to KVM-sent small requests
```bash
# host
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
# kVM
sockperf server -i 192.168.100.2
```
```
sockperf: [Total Run] RunTime=30.000 sec; Warm up time=400 msec; SentMessages=308991; ReceivedMessages=308990
sockperf: ========= Printing statistics for Server No: 0
sockperf: [Valid Duration] RunTime=29.549 sec; SentMessages=304768; ReceivedMessages=304768
sockperf: ====> avg-latency=48.444 (std-dev=3.239)
sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
sockperf: Summary: Latency is 48.444 usec
sockperf: Total 304768 observations; each percentile contains 3047.68 observations
sockperf: ---> <MAX> observation =  576.614
sockperf: ---> percentile 99.999 =  209.822
sockperf: ---> percentile 99.990 =  100.788
sockperf: ---> percentile 99.900 =   65.016
sockperf: ---> percentile 99.000 =   57.111
sockperf: ---> percentile 90.000 =   52.032
sockperf: ---> percentile 75.000 =   50.073
sockperf: ---> percentile 50.000 =   48.075
sockperf: ---> percentile 25.000 =   46.452
sockperf: ---> <MIN> observation =   38.351
```
### Syscalls on server process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 2052
```
### Syscalls on client process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mmap]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_mprotect]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 12
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_preadv]: 13
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 60
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 149
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 156
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 446
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 489
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 1517
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 10995
```
