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
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 442
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 3119
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 11627
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 522637
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 544061
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 699450

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 963020
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 4299471821
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 14326391004
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 17068550849
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 19119544714
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 110542040610
```
### Syscalls on client process
```
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 4
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 88
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 4323
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 49084
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 276885
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 544597
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 549867

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_readv]: 33181
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 342187
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 1491758313
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 12171370338
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 26952863552
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 30889658338
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 95093741782
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
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 38
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 314
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 199650
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 402168
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 598985
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 598986

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 203935
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 1204390920
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 1823267592
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 30335901535
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 39430160272
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 117812742814
```
### Syscalls on client process
```
--- total syscall counts ---
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_readv]: 60
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_write]: 456
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_futex]: 796
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_writev]: 201168
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ioctl]: 410762
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_ppoll]: 599109
@total[qemu-system-x86, tracepoint:syscalls:sys_enter_read]: 599116

--- cumulative syscall time (ns) ---
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_readv]: 202052
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_write]: 1080241
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_read]: 1173238569
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_writev]: 1897529092
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_futex]: 15464207311
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ppoll]: 29947719371
@time[qemu-system-x86, tracepoint:syscalls:sys_exit_ioctl]: 99817828478
```
