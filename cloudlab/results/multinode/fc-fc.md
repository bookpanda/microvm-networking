# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
```bash
%idle cpu, used memory, throughput
# c6525-25g nodes (enp65s0f0np0)
- 1vCPU(), 512MB()
    - throughput: 10.6 Gbits/s (-P 4)
    - latency (usec, -m 64), idleCPU: %, usedMem: Mi 
        - p50: 88.170
        - p90: 91.351
        - p99.9: 103.914
# xl170 nodes (ens1f1np1)
- 1vCPU, 512MB
    - throughput: 14.1 Gbits/s (-P 4), idleCPU: %, usedMem: Mi
    - latency (usec, -m 64), idleCPU: %, usedMem: Mi 
        - p50: 95.752
        - p90: 416.853
        - p99.9: 550.135
- 4vCPU, 1024MB
    - throughput: 9.59 Gbits/s (-P 4), idleCPU: %, usedMem: Mi
    - latency (usec, -m 64), idleCPU: %, usedMem: Mi 
        - p50: 475.186
        - p90: 530.059
        - p99.9: 669.473
```
### Syscalls on server process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 125880
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 126130
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 128282
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 133010
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 266881
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 617066
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 266880147
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 414686000
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 2259799787
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 3233888438
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 21199346769
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 30335638095
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 82226
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 152698
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 178015
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 231868
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 377290
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 611606
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_ioctl]: 4318
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_recvfrom]: 24938
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 178715731
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 537127757
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 1481277788
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 12300902661
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 13814503411
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 30312316986
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
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 171416
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 176609
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 342828
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 348020
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 348027
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 685646
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 337514720
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 1079568044
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 1132353780
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 1278204220
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 31338106034
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 36590728696
```
### Syscalls on client process
```
[STDOUT] --- total syscall counts ---
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_ioctl]: 1
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_recvfrom]: 4
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_writev]: 171416
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_read]: 176835
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_write]: 342867
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_readv]: 348030
[STDOUT] @total[firecracker, tracepoint:syscalls:sys_enter_epoll_pwait]: 348050
[STDOUT] @total[fc_vcpu 0, tracepoint:syscalls:sys_enter_ioctl]: 685694
[STDOUT] 
[STDOUT] --- cumulative syscall time (ns) ---
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_ioctl]: 4097
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_recvfrom]: 25197
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_read]: 331147111
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_readv]: 1062012071
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_write]: 1077289072
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_writev]: 1308416804
[STDOUT] @time[firecracker, tracepoint:syscalls:sys_exit_epoll_pwait]: 27362322358
[STDOUT] @time[fc_vcpu 0, tracepoint:syscalls:sys_exit_ioctl]: 32606767107
```
