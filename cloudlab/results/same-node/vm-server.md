# Throughput
## VM as server responding to bulk data from a host
```bash
# host
iperf3 -c 192.168.100.2 -t 30 -P 4
# vm
iperf3 -s
```
```
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-30.00  sec  15.3 GBytes  4.38 Gbits/sec    0             sender
[  5]   0.00-30.00  sec  15.3 GBytes  4.38 Gbits/sec                  receiver
[  7]   0.00-30.00  sec  15.3 GBytes  4.37 Gbits/sec    0             sender
[  7]   0.00-30.00  sec  15.3 GBytes  4.37 Gbits/sec                  receiver
[  9]   0.00-30.00  sec  15.2 GBytes  4.35 Gbits/sec    0             sender
[  9]   0.00-30.00  sec  15.2 GBytes  4.35 Gbits/sec                  receiver
[ 11]   0.00-30.00  sec  15.2 GBytes  4.35 Gbits/sec    0             sender
[ 11]   0.00-30.00  sec  15.2 GBytes  4.35 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec  61.0 GBytes  17.5 Gbits/sec    0             sender
[SUM]   0.00-30.00  sec  61.0 GBytes  17.5 Gbits/sec                  receiver
```
### Syscalls on microVM process (server)
```
@total[fc_vcpu 0, write]: 15131
@total[firecracker, read]: 198492
@total[firecracker, write]: 198612
```

# Latency
## VM as server responding to host-sent small requests
```bash
# host
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
# vm
./sockperf server -i 192.168.100.2
```
```
sockperf: ========= Printing statistics for Server No: 0
sockperf: [Valid Duration] RunTime=29.550 sec; SentMessages=215839; ReceivedMessages=215839
sockperf: ====> avg-latency=67.798 (std-dev=17.156, mean-ad=9.381, median-ad=6.378, siqr=4.406, cv=0.253, std-error=0.037, 99.0% ci=[67.703, 67.893])
sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
sockperf: Summary: Latency is 67.798 usec
sockperf: Total 215839 observations; each percentile contains 2158.39 observations
sockperf: ---> <MAX> observation =  414.500
sockperf: ---> percentile 99.999 =  366.822
sockperf: ---> percentile 99.990 =  305.657
sockperf: ---> percentile 99.900 =  249.379
sockperf: ---> percentile 99.000 =  132.527
sockperf: ---> percentile 90.000 =   78.878
sockperf: ---> percentile 75.000 =   70.779
sockperf: ---> percentile 50.000 =   65.509
sockperf: ---> percentile 25.000 =   61.966
sockperf: ---> <MIN> observation =   40.071
```
### Syscalls on microVM process (server)
```
@total[firecracker, read]: 224897
@total[firecracker, write]: 436562
```