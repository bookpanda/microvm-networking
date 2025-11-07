# OVS-DPDK Performance Tuning Guide

## Current Performance
- **Before DPDK fix**: 2.36 Gbits/s (wrong NIC)
- **After DPDK fix**: 3.03 Gbits/s (correct NIC, but not optimized)
- **Target**: 10+ Gbits/s (link is 25Gbps)

## Identified Bottlenecks

### 1. VM Guest Only Using 2 TX Queues (CRITICAL)
```
vhost-user1: only queue 0,1 active
tx_q0: 9.2 GB ✓
tx_q1: 0 bytes ✗  ← Barely used!
```
**Impact**: Single-queue bottleneck on TX side

### 2. PMD Threads 99.8% Idle
**Meaning**: OVS-DPDK has plenty of capacity, bottleneck is elsewhere

### 3. Link Underutilized
- Link speed: 25 Gbps
- Current: 3 Gbps (12% utilization)

## Optimization Steps

### Step 1: Increase VM vCPUs and Memory
More vCPUs = more queues can be utilized by guest

```bash
# Launch with 8 vCPUs (not just max=8, but boot=8)
sudo cloud-hypervisor \
    --cpus boot=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server
```

### Step 2: Enable Multi-Queue in Guest (Run in VM)
```bash
# Check current queue config
IFACE=$(ip -o link show | grep -v "lo:" | awk -F': ' '{print $2}' | head -1)
ethtool -l $IFACE

# Enable all 8 queues
sudo ethtool -L $IFACE combined 8

# Verify
ethtool -l $IFACE  # Should show "Current hardware settings: Combined: 8"

# Set up XPS for better TX distribution
for i in {0..7}; do
    [ -d /sys/class/net/$IFACE/queues/tx-$i ] && printf '%x' $((1 << $i)) | sudo tee /sys/class/net/$IFACE/queues/tx-$i/xps_cpus > /dev/null
done

# Set up RPS for better RX distribution
for i in {0..7}; do
    [ -d /sys/class/net/$IFACE/queues/rx-$i ] && echo ff | sudo tee /sys/class/net/$IFACE/queues/rx-$i/rps_cpus > /dev/null
done
```

### Step 3: Increase TCP Buffers (Run in VM)
```bash
# In the VM guest
sudo sysctl -w net.core.rmem_max=134217728
sudo sysctl -w net.core.wmem_max=134217728
sudo sysctl -w net.core.rmem_default=16777216
sudo sysctl -w net.core.wmem_default=16777216
sudo sysctl -w net.ipv4.tcp_rmem="4096 87380 67108864"
sudo sysctl -w net.ipv4.tcp_wmem="4096 65536 67108864"
sudo sysctl -w net.ipv4.tcp_congestion_control=bbr
```

### Step 4: Increase OVS Queue Count
```bash
# On host, increase dpdk0 queues to match
sudo ovs-vsctl set Interface dpdk0 options:n_rxq=8
sudo ovs-vsctl set Interface vhost-user1 options:n_rxq=8

# Verify
sudo ovs-appctl dpif-netdev/pmd-rxq-show
```

### Step 5: Pin VM vCPUs to Physical Cores (Optional but Recommended)
```bash
# Get VM PID
VM_PID=$(pgrep -f cloud-hypervisor)

# Get vCPU thread IDs
ps -T -p $VM_PID | grep vcpu

# Pin vcpu0-7 to cores 8-15 (assuming cores 0-7 are for system/PMD)
# Example (adjust core numbers based on your system):
sudo taskset -cp 8 <vcpu0_tid>
sudo taskset -cp 9 <vcpu1_tid>
sudo taskset -cp 10 <vcpu2_tid>
# ... and so on
```

### Step 6: Use More iperf Parallel Streams
```bash
# In receiver VM (host 0)
iperf3 -s

# In sender VM (host 1)
iperf3 -c 192.168.100.2 -t 60 -P 16 -w 4M

# Try with different stream counts
iperf3 -c 192.168.100.2 -t 60 -P 32 -w 4M
```

### Step 7: Enable Jumbo Frames (Optional)
```bash
# On both hosts
sudo ip link set ovsbr0 mtu 9000
sudo ovs-vsctl set Interface dpdk0 mtu_request=9000

# In both VMs
sudo ip link set <interface> mtu 9000
```

## Verification Commands

### On Host During Test:
```bash
# Watch PMD usage (should increase from 0.17% to higher)
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -E "pmd thread|usage|idle" | head -20'

# Watch queue distribution
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-rxq-show'

# Watch packet stats
watch -n 1 'sudo ovs-vsctl get Interface dpdk0 statistics | grep -o "tx_packets=[^,]*"; sudo ovs-vsctl get Interface vhost-user1 statistics | grep -o "rx_q[0-7]_good_packets=[^,]*"'
```

### In VM During Test:
```bash
# Check queue usage
ethtool -S <interface> | grep -E "tx_queue_[0-7]_packets|rx_queue_[0-7]_packets"

# Check interrupts spreading across cores
watch -n 1 'cat /proc/interrupts | grep virtio'
```

## Expected Results

After all optimizations:
- **8 queues active** on vhost-user1
- **More balanced packet distribution** across all queues
- **Higher throughput**: 8-12 Gbits/s or more
- **PMD usage increases** to 5-20% (still plenty of headroom)

## Troubleshooting

### If still bottlenecked at ~3 Gbps:
1. Check if VM is CPU-bound: `top` in VM should show high CPU usage on multiple cores
2. If single core at 100%: Need better load distribution or queue steering
3. If all cores low: TCP window might be limiting, increase `-w` in iperf3

### If PMD threads hit 50%+ usage:
1. Add more PMD cores: `sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF` (8 cores)
2. Rebalance queues: `sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance`

