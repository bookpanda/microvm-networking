# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```
[STDOUT] - - - - - - - - - - - - - - - - - - - - - - - - -
[STDOUT] [ ID] Interval           Transfer     Bandwidth
[STDOUT] [  5]   0.00-30.02  sec  0.00 Bytes  0.00 bits/sec                  sender
[STDOUT] [  5]   0.00-30.02  sec  15.6 GBytes  4.45 Gbits/sec                  receiver
[STDOUT] [  7]   0.00-30.02  sec  0.00 Bytes  0.00 bits/sec                  sender
[STDOUT] [  7]   0.00-30.02  sec  15.5 GBytes  4.44 Gbits/sec                  receiver
[STDOUT] [  9]   0.00-30.02  sec  0.00 Bytes  0.00 bits/sec                  sender
[STDOUT] [  9]   0.00-30.02  sec  15.3 GBytes  4.38 Gbits/sec                  receiver
[STDOUT] [ 11]   0.00-30.02  sec  0.00 Bytes  0.00 bits/sec                  sender
[STDOUT] [ 11]   0.00-30.02  sec  15.0 GBytes  4.29 Gbits/sec                  receiver
[STDOUT] [SUM]   0.00-30.02  sec  0.00 Bytes  0.00 bits/sec                  sender
[STDOUT] [SUM]   0.00-30.02  sec  61.4 GBytes  17.6 Gbits/sec                  receiver
```
### Syscalls on server process
```
[STDOUT] --- cumulative syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_lseek]: 23
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 235509
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 240729
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 250544
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 435378
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 450500
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 1096616
```
### Syscalls on client process
```
[STDOUT] --- cumulative syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_lseek]: 125
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 54443
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 88380
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 112016
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 114938
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 519669
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 992395
```

# Latency
## VM as server responding to VM-sent small requests
```bash
# host
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
# vm
./sockperf server -i 192.168.100.2
```
```
[STDOUT] sockperf: ========= Printing statistics for Server No: 0
[STDOUT] sockperf: [Valid Duration] RunTime=29.549 sec; SentMessages=94416; ReceivedMessages=94416
[STDOUT] sockperf: [2;35m====> avg-latency=156.309 (std-dev=101.155, mean-ad=62.044, median-ad=8.582, siqr=6.420, cv=0.647, std-error=0.329, 99.0% ci=[155.461, 157.157])[0m
[STDOUT] sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
[STDOUT] sockperf: Summary: Latency is 156.309 usec
[STDOUT] sockperf: [2;35mTotal 94416 observations[0m; each percentile contains 944.16 observations
[STDOUT] sockperf: ---> <MAX> observation =  768.182
[STDOUT] sockperf: ---> percentile 99.999 =  767.523
[STDOUT] sockperf: ---> percentile 99.990 =  735.845
[STDOUT] sockperf: ---> percentile 99.900 =  581.216
[STDOUT] sockperf: ---> percentile 99.000 =  561.029
[STDOUT] sockperf: ---> percentile 90.000 =  260.583
[STDOUT] sockperf: ---> percentile 75.000 =  128.459
[STDOUT] sockperf: ---> percentile 50.000 =  120.567
[STDOUT] sockperf: ---> percentile 25.000 =  115.618
[STDOUT] sockperf: ---> <MIN> observation =   79.330
```
### Syscalls on server process
```
[STDOUT] --- cumulative syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 95245
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 98131
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 190494
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 193380
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 193385
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 380984
```
### Syscalls on client process
```
[STDOUT] --- cumulative syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_lseek]: 195
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 94638
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 97797
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 189367
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 192135
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 192231
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 378708
```
