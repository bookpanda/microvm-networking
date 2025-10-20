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
[OUTPUT] [  7]   0.00-30.00  sec  19.1 GBytes  5.46 Gbits/sec    0             sender
[OUTPUT] [  7]   0.00-30.00  sec  19.1 GBytes  5.46 Gbits/sec                  receiver
[OUTPUT] [  9]   0.00-30.00  sec  18.9 GBytes  5.41 Gbits/sec    0             sender
[OUTPUT] [  9]   0.00-30.00  sec  18.9 GBytes  5.41 Gbits/sec                  receiver
[OUTPUT] [ 11]   0.00-30.00  sec  18.9 GBytes  5.40 Gbits/sec    0             sender
[OUTPUT] [ 11]   0.00-30.00  sec  18.9 GBytes  5.40 Gbits/sec                  receiver
[OUTPUT] [ 13]   0.00-30.00  sec  18.8 GBytes  5.39 Gbits/sec    0             sender
[OUTPUT] [ 13]   0.00-30.00  sec  18.8 GBytes  5.39 Gbits/sec                  receiver
[OUTPUT] [SUM]   0.00-30.00  sec  75.6 GBytes  21.7 Gbits/sec    0             sender
[OUTPUT] [SUM]   0.00-30.00  sec  75.6 GBytes  21.7 Gbits/sec                  receiver
```
### Syscalls on server process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 200750
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 252001
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 252740
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 482191
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 562285
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 1271585
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 519155921
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 687664320
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 2335941802
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 5860559809
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 15640085321
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 30232056814
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 46283
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 59147
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 72253
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 82976
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 615445
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 1257337
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_ioctl]: 4278
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_recvfrom]: 46556
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 94715426
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 203828120
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 2014684770
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 2517544240
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 22775536117
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 30774532057
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
