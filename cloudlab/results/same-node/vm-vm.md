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
[OUTPUT] sockperf: [Total Run] RunTime=30.000 sec; Warm up time=400 msec; SentMessages=201170; ReceivedMessages=201169
[OUTPUT] sockperf: ========= Printing statistics for Server No: 0
[OUTPUT] sockperf: [Valid Duration] RunTime=29.550 sec; SentMessages=198378; ReceivedMessages=198378
[OUTPUT] sockperf: [2;35m====> avg-latency=74.449 (std-dev=3.660)[0m
[OUTPUT] sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
[OUTPUT] sockperf: Summary: Latency is 74.449 usec
[OUTPUT] sockperf: [2;35mTotal 198378 observations[0m; each percentile contains 1983.78 observations
[OUTPUT] sockperf: ---> <MAX> observation =  290.965
[OUTPUT] sockperf: ---> percentile 99.999 =  205.705
[OUTPUT] sockperf: ---> percentile 99.990 =  135.819
[OUTPUT] sockperf: ---> percentile 99.900 =   88.856
[OUTPUT] sockperf: ---> percentile 99.000 =   84.884
[OUTPUT] sockperf: ---> percentile 90.000 =   79.949
[OUTPUT] sockperf: ---> percentile 75.000 =   75.877
[OUTPUT] sockperf: ---> percentile 50.000 =   72.911
[OUTPUT] sockperf: ---> percentile 25.000 =   72.170
[OUTPUT] sockperf: ---> <MIN> observation =   67.120
```
### Syscalls on server process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 201175
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 207270
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 402351
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 408446
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 408455
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 804698
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 395094448
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 1001531123
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 1220983493
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 1337703890
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 26900230342
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 31812982515
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 201175
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 207525
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 402389
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 408457
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 408476
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 804742
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_ioctl]: 6973
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_recvfrom]: 22953
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 392278437
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 992534536
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 1219978548
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 1328476310
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 27062759939
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 31935105589
```
