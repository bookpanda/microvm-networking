# Throughput
## VM as client sending bulk data to a host
```bash
# host
./bpftrace.sh
iperf3 -s
# vm
iperf3 -c 192.168.100.1 -t 30 -P 4
```
```
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bandwidth       Retr
[  4]   0.00-30.00  sec  14.4 GBytes  4.12 Gbits/sec    0             sender
[  4]   0.00-30.00  sec  14.4 GBytes  4.12 Gbits/sec                  receiver
[  6]   0.00-30.00  sec  14.3 GBytes  4.10 Gbits/sec    0             sender
[  6]   0.00-30.00  sec  14.3 GBytes  4.10 Gbits/sec                  receiver
[  8]   0.00-30.00  sec  14.3 GBytes  4.10 Gbits/sec    0             sender
[  8]   0.00-30.00  sec  14.3 GBytes  4.09 Gbits/sec                  receiver
[ 10]   0.00-30.00  sec  14.2 GBytes  4.07 Gbits/sec    0             sender
[ 10]   0.00-30.00  sec  14.2 GBytes  4.07 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec  57.2 GBytes  16.4 Gbits/sec    0             sender
[SUM]   0.00-30.00  sec  57.2 GBytes  16.4 Gbits/sec                  receiver
```
### Syscalls
```
@total[fc_vcpu 0, write]: 15664
@total[firecracker, read]: 27836
@total[firecracker, write]: 35355
```

# Latency
## VM as client sending small requests to a host
```bash
# host
./bpftrace.sh
sockperf server -i 192.168.100.1
# vm
./sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```
```
sockperf: ========= Printing statistics for Server No: 0
sockperf: [Valid Duration] RunTime=29.549 sec; SentMessages=198352; ReceivedMessages=198352
sockperf: ====> avg-latency=74.383 (std-dev=15.718, mean-ad=8.553, median-ad=4.873, siqr=3.878, cv=0.211, std-error=0.035, 99.0% ci=[74.292, 74.474])
sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
sockperf: Summary: Latency is 74.383 usec
sockperf: Total 198352 observations; each percentile contains 1983.52 observations
sockperf: ---> <MAX> observation =  385.178
sockperf: ---> percentile 99.999 =  343.253
sockperf: ---> percentile 99.990 =  313.233
sockperf: ---> percentile 99.900 =  264.171
sockperf: ---> percentile 99.000 =  129.705
sockperf: ---> percentile 90.000 =   84.220
sockperf: ---> percentile 75.000 =   76.258
sockperf: ---> percentile 50.000 =   70.909
sockperf: ---> percentile 25.000 =   68.501
sockperf: ---> <MIN> observation =   39.925
```
### Syscalls
```
@total[fc_vcpu 0, write]: 1644
@total[firecracker, read]: 207161
@total[firecracker, write]: 402018
```