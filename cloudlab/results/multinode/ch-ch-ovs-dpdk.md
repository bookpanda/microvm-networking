# Throughput
## VM as server responding to bulk data from a VM
```bash
# server
iperf3 -s
# client
iperf3 -c 192.168.100.2 -t 30 -P 4
```
### Before binding NIC to DPDK
- 2vCPU, 512MB, 4 queues, 2.28 Gbits/s
- 2vCPU, 1024MB, 4 queues, 2.27 Gbit/s
- 2vCPU(max=4), 1024MB, 4 queues, 2.38 Gbit/s
- 4vCPU(max=8), 2048MB, 4 queues, 2.45 Gbit/s
- 4vCPU(max=8), 2048MB, 4 queues, 2.45 Gbit/s (iperf3 -P 8)
- 4vCPU(max=8), 4096MB, 8 queues, 2.58 Gbit/s
- 4vCPU(max=8), 4096MB, 8 queues, 2.76 Gbit/s (pin VM cores 13-16)
### After binding NIC to DPDK (vfio-pci, faulty mode)
- 4vCPU(max=8), 4096MB, 2 queues, 1.97 Gbits/s
- 4vCPU(max=8), 4096MB, 4 queues, 2.13 Gbits/s
### After binding unused enp65s0f1np1 NIC to DPDK (mlx5_core)
- 4vCPU(max=8), 1024MB, 4 queues, 2.15 Gbits/s
- 4vCPU(max=8), 2048MB, 4 queues, 2.36 Gbits/s
### After binding enp65s0f0np0 NIC to DPDK (mlx5_core)
- 4vCPU(max=8), 2048MB, 4 queues, 3.03 Gbits/s
- 4vCPU(max=8), 4096MB, 8 queues, 2.94 Gbits/s
### After removing IP from physical NIC and disabling kernel routing through it
- 4vCPU(max=8), 4096MB, 8 queues, 8.89 Gbits/s
- 8vCPU, 4096MB, 16 queues, 10.4 Gbits/s (-P 8)
- 8vCPU, 4096MB, 16 queues, 11.1 Gbits/s (-P 16)
### 1 queue (RX/TX) should map to 1vCPU (if not, only 1 TX, 7 RX)
- 8vCPU, 4096MB, 8 queues (4096 size), 9.20 Gbits/s (-P 8)
- 2vCPU, 512MB, 2 queues (1024 size), 8.41 Gbits/s (-P 8)
- 2vCPU(47%), 512MB(85Mi), 2 queues (1024 size), 8.59 Gbits/s (-P 4)
- 4vCPU(78%), 1024MB(117Mi), 2 queues (1024 size), 7.75 Gbits/s (-P 4)
- 4vCPU(78%), 1024MB(115Mi), 4 queues (1024 size), 8.82 Gbits/s (-P 4)

### Syscalls on server process
```

```
### Syscalls on client process
```

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

```
### Syscalls on server process
```
```
### Syscalls on client process
```
```
