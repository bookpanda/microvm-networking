# Specs
- vmlinux-5.10.223-no-acpi (36MB)
- debian-rootfs.ext4 (1000MB)

# Throughput
```bash
# host
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
## Syscalls
```
@total[fc_vcpu 0, write]: 15664
@total[firecracker, read]: 27836
@total[firecracker, write]: 35355
```

# Latency
```bash
# host
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30
# vm
./sockperf server -i 192.168.100.2
```
```
sockperf: Summary: Latency is 68.964 usec
sockperf: Total 212210 observations; each percentile contains 2122.10 observations
sockperf: ---> <MAX> observation =  407.908
sockperf: ---> percentile 99.999 =  327.526
sockperf: ---> percentile 99.990 =  292.848
sockperf: ---> percentile 99.900 =  257.501
sockperf: ---> percentile 99.000 =  123.873
sockperf: ---> percentile 90.000 =   79.139
sockperf: ---> percentile 75.000 =   74.059
sockperf: ---> percentile 50.000 =   67.161
sockperf: ---> percentile 25.000 =   61.182
sockperf: ---> <MIN> observation =   39.109
```
## Syscalls
```
@total[firecracker, read]: 230037
@total[firecracker, write]: 446541
```