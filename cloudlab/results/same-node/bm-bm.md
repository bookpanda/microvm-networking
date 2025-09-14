# Throughput
```bash
# server
iperf3 -s
./bpftrace.sh $(pgrep -n iperf)
# client
iperf3 -c 192.168.100.1 -t 30 -P 4
```
```
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-30.00  sec  83.9 GBytes  24.0 Gbits/sec    0             sender
[  5]   0.00-30.00  sec  83.9 GBytes  24.0 Gbits/sec                  receiver
[  7]   0.00-30.00  sec  82.5 GBytes  23.6 Gbits/sec    0             sender
[  7]   0.00-30.00  sec  82.5 GBytes  23.6 Gbits/sec                  receiver
[  9]   0.00-30.00  sec  85.5 GBytes  24.5 Gbits/sec    0             sender
[  9]   0.00-30.00  sec  85.5 GBytes  24.5 Gbits/sec                  receiver
[ 11]   0.00-30.00  sec  84.0 GBytes  24.1 Gbits/sec    0             sender
[ 11]   0.00-30.00  sec  84.0 GBytes  24.1 Gbits/sec                  receiver
[SUM]   0.00-30.00  sec   336 GBytes  96.2 Gbits/sec    0             sender
[SUM]   0.00-30.00  sec   336 GBytes  96.2 Gbits/sec                  receiver
```
### Server-side Syscalls
```
@total[iperf3, setsockopt]: 3
@total[iperf3, close]: 15
@total[iperf3, getsockopt]: 130
@total[iperf3, write]: 209
@total[iperf3, read]: 4740585
```

# Latency
```bash
# server
sockperf server -i 192.168.100.1
./bpftrace.sh $(pgrep -n sockperf)
# client
sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```
```
sockperf: ========= Printing statistics for Server No: 0
sockperf: [Valid Duration] RunTime=29.550 sec; SentMessages=543577; ReceivedMessages=543577
sockperf: ====> avg-latency=26.535 (std-dev=5.928, mean-ad=1.793, median-ad=0.415, siqr=0.288, cv=0.223, std-error=0.008, 99.0% ci=[26.514, 26.556])
sockperf: # dropped messages = 0; # duplicated messages = 0; # out-of-order messages = 0
sockperf: Summary: Latency is 26.535 usec
sockperf: Total 543577 observations; each percentile contains 5435.77 observations
sockperf: ---> <MAX> observation =  280.380
sockperf: ---> percentile 99.999 =  267.478
sockperf: ---> percentile 99.990 =  239.500
sockperf: ---> percentile 99.900 =   71.528
sockperf: ---> percentile 99.000 =   44.974
sockperf: ---> percentile 90.000 =   27.599
sockperf: ---> percentile 75.000 =   26.336
sockperf: ---> percentile 50.000 =   26.015
sockperf: ---> percentile 25.000 =   25.759
sockperf: ---> <MIN> observation =   11.395
```
### Server-side Syscalls
```
@total[sockperf, recvfrom]: 551953
@total[sockperf, sendto]: 551953
```