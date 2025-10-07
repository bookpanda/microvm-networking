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
[  5]   0.00-30.00  sec  8.42 GBytes  2.41 Gbits/sec   98             sender
[  5]   0.00-30.04  sec  8.42 GBytes  2.41 Gbits/sec                  receiver
[  7]   0.00-30.00  sec  8.39 GBytes  2.40 Gbits/sec    0             sender
[  7]   0.00-30.04  sec  8.39 GBytes  2.40 Gbits/sec                  receiver
[  9]   0.00-30.00  sec  8.35 GBytes  2.39 Gbits/sec    0             sender
[  9]   0.00-30.04  sec  8.35 GBytes  2.39 Gbits/sec                  receiver
[ 11]   0.00-30.00  sec  8.37 GBytes  2.40 Gbits/sec   60             sender
[ 11]   0.00-30.04  sec  8.37 GBytes  2.39 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec  33.5 GBytes  9.60 Gbits/sec  158             sender
[SUM]   0.00-30.04  sec  33.5 GBytes  9.59 Gbits/sec                  receiver
```
### Syscalls on server process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pread64]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_fdatasync]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwritev]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_preadv]: 5
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 22
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 23
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 691
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 11480
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 14604
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 554089
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 606631
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 724154
```
### Syscalls on client process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pread64]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_fdatasync]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_munmap]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwritev]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 11
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 47
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 353
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 4542
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 60757
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 210378
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 588143
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 605786
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
sockperf: [Total Run] RunTime=30.000 sec; Warm up time=400 msec; SentMessages=229367; ReceivedMessages=229366
sockperf: ========= Printing statistics for Server No: 0
sockperf: [Valid Duration] RunTime=29.549 sec; SentMessages=226052; ReceivedMessages=226052
sockperf: ====> avg-latency=65.325 (std-dev=4.230)
sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
sockperf: Summary: Latency is 65.325 usec
sockperf: Total 226052 observations; each percentile contains 2260.52 observations
sockperf: ---> <MAX> observation =  235.349
sockperf: ---> percentile 99.999 =  205.033
sockperf: ---> percentile 99.990 =  141.759
sockperf: ---> percentile 99.900 =  107.796
sockperf: ---> percentile 99.000 =   77.855
sockperf: ---> percentile 90.000 =   69.940
sockperf: ---> percentile 75.000 =   66.884
sockperf: ---> percentile 50.000 =   64.660
sockperf: ---> percentile 25.000 =   62.832
sockperf: ---> <MIN> observation =   53.955
```
### Syscalls on server process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 3
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 543
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 229370
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 462795
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 688112
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 688115
```
### Syscalls on client process
```
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_clone3]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_exit]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pread64]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwritev]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_set_robust_list]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_madvise]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rseq]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_pwrite64]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_fdatasync]: 2
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_rt_sigprocmask]: 6
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_preadv]: 12
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 60
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 453
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 702
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 230887
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 469373
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 688267
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 688273
```
