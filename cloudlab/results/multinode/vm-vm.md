# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```
[OUTPUT] [ ID] Interval           Transfer     Bitrate         Retr
[OUTPUT] [  7]   0.00-30.00  sec  9.36 GBytes  2.68 Gbits/sec    0             sender
[OUTPUT] [  7]   0.00-30.01  sec  9.35 GBytes  2.68 Gbits/sec                  receiver
[OUTPUT] [  9]   0.00-30.00  sec  9.25 GBytes  2.65 Gbits/sec    0             sender
[OUTPUT] [  9]   0.00-30.01  sec  9.25 GBytes  2.65 Gbits/sec                  receiver
[OUTPUT] [ 11]   0.00-30.00  sec  9.24 GBytes  2.65 Gbits/sec    0             sender
[OUTPUT] [ 11]   0.00-30.01  sec  9.24 GBytes  2.65 Gbits/sec                  receiver
[OUTPUT] [ 13]   0.00-30.00  sec  9.29 GBytes  2.66 Gbits/sec    0             sender
[OUTPUT] [ 13]   0.00-30.01  sec  9.29 GBytes  2.66 Gbits/sec                  receiver
[OUTPUT] [SUM]   0.00-30.00  sec  37.1 GBytes  10.6 Gbits/sec    0             sender
[OUTPUT] [SUM]   0.00-30.01  sec  37.1 GBytes  10.6 Gbits/sec                  receiver
```
### Syscalls on server process
```
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 123565
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 124294
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 138015
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 163632
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 266526
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 617340
```
### Syscalls on client process
```
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_close]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_accept4]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_ctl]: 6
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_lseek]: 86
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 88554
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 171307
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 191669
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 252548
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 393557
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 612158
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
[OUTPUT] sockperf: [Total Run] RunTime=30.000 sec; Warm up time=400 msec; SentMessages=169184; ReceivedMessages=169183
[OUTPUT] sockperf: ========= Printing statistics for Server No: 0
[OUTPUT] sockperf: [Valid Duration] RunTime=29.550 sec; SentMessages=166691; ReceivedMessages=166691
[OUTPUT] sockperf: [2;35m====> avg-latency=88.612 (std-dev=2.427)[0m
[OUTPUT] sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
[OUTPUT] sockperf: Summary: Latency is 88.612 usec
[OUTPUT] sockperf: [2;35mTotal 166691 observations[0m; each percentile contains 1666.91 observations
[OUTPUT] sockperf: ---> <MAX> observation =  186.419
[OUTPUT] sockperf: ---> percentile 99.999 =  158.151
[OUTPUT] sockperf: ---> percentile 99.990 =  130.635
[OUTPUT] sockperf: ---> percentile 99.900 =  103.914
[OUTPUT] sockperf: ---> percentile 99.000 =   96.606
[OUTPUT] sockperf: ---> percentile 90.000 =   91.351
[OUTPUT] sockperf: ---> percentile 75.000 =   89.753
[OUTPUT] sockperf: ---> percentile 50.000 =   88.170
[OUTPUT] sockperf: ---> percentile 25.000 =   87.048
[OUTPUT] sockperf: ---> <MIN> observation =   81.412
```
### Syscalls on server process
```
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 169190
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 174316
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 338380
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 343506
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 343511
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 676756
```
### Syscalls on client process
```
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_accept4]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_close]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_ctl]: 6
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_lseek]: 219
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 169190
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 174558
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 338416
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 343512
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 343531
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 676796
```
