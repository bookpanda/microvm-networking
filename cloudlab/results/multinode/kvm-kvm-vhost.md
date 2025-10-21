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
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 90
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 90
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 103
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 498
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 13715
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 80249

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 262074
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 488593
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 56916086
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 63230753777
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 66883668239
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 125809364363
```
### Syscalls on client process
```
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 47
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 314
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 320
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 554
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 1361
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 14731
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 87073

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_readv]: 148476
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 815230
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 1427737
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 61398785
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 48047837283
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 52897934603
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 112709564855
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
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 1
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 1824

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 6973
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 122972830112
```
### Syscalls on client process
```
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 60
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 153
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 159
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 448
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 486
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 1517
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 10705

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_readv]: 170476
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 340692
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 944162
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 6887013
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 2375325658
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 12210367310
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 100570917805
```
