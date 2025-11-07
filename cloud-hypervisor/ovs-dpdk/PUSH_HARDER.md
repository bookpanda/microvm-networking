# Push for 15-20 Gbps Performance

## Current: 8.89 Gbps (Good progress!)
## Target: 15-20 Gbps (out of 25 Gbps link)

## Bottlenecks Identified:

1. **VM using only 4 queues** (vhost-user1 shows only 4 active)
2. **VM boots with 4 vCPUs** (should be 8 for 8-queue utilization)
3. **PMD threads 98%+ idle** (plenty of capacity)
4. **iperf parallel streams** (need more to saturate queues)

## Action Plan:

### 1. Boot VMs with 8 vCPUs (NOT just max=8)

**Change:**
```bash
--cpus boot=4,max=8  # ❌ Only starts with 4
```

**To:**
```bash
--cpus boot=8  # ✅ Starts with 8, can use all queues
```

### 2. Increase iperf Parallel Streams

**Current:**
```bash
iperf3 -c 10.10.1.10 -t 60 -P 8
```

**Try:**
```bash
iperf3 -c 10.10.1.10 -t 60 -P 16  # More parallelism
iperf3 -c 10.10.1.10 -t 60 -P 32  # Even more
iperf3 -c 10.10.1.10 -t 60 -P 64  # Maximum
```

### 3. Increase PMD Cores (if needed)

**Current:** 4 PMD cores (cores 2-5, mask 0x3C)

**If PMD usage goes > 50%, add more cores:**
```bash
# Add 4 more PMD cores (cores 2-9, total 8 cores)
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF

# Rebalance queues
sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance
```

### 4. Enable TSO/GSO Offloads

```bash
# In VMs:
IFACE=$(ip -o link show | grep -v "lo:" | awk -F': ' '{print $2}' | head -1)
sudo ethtool -K $IFACE tso on gso on
sudo ethtool -K $IFACE gro on
```

### 5. Increase TCP Window Size

```bash
# In VMs (already in cloud-init, but verify):
sysctl net.ipv4.tcp_rmem
sysctl net.ipv4.tcp_wmem

# Should show large buffers like: 4096 87380 67108864
```

### 6. Pin VM vCPUs to Physical Cores (Optional)

```bash
# On host:
VM_PID=$(pgrep -f cloud-hypervisor)
ps -T -p $VM_PID | grep vcpu

# Pin vcpu threads to cores 8-15 (avoid PMD cores 2-5)
# Example:
sudo taskset -cp 8 <vcpu0_tid>
sudo taskset -cp 9 <vcpu1_tid>
# ... etc
```

## Testing Methodology:

### Step 1: Launch with 8 vCPUs
```bash
sudo cloud-hypervisor \
    --cpus boot=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=16,vhost_mode=server
```

### Step 2: Verify 8 Queues Active
```bash
# In VM:
nproc  # Should show 8
IFACE=$(ip -o link show | grep -v "lo:" | awk -F': ' '{print $2}' | head -1)
ethtool -l $IFACE  # Should show Combined: 8

# On host:
sudo ovs-appctl dpif-netdev/pmd-rxq-show | grep vhost-user1
# Should show 8 queues
```

### Step 3: Run Progressive Tests
```bash
# In receiver VM:
iperf3 -s

# In sender VM:
echo "=== Test 1: P=8 ==="
iperf3 -c 10.10.1.10 -t 30 -P 8

echo "=== Test 2: P=16 ==="
iperf3 -c 10.10.1.10 -t 30 -P 16

echo "=== Test 3: P=32 ==="
iperf3 -c 10.10.1.10 -t 30 -P 32

echo "=== Test 4: P=64 ==="
iperf3 -c 10.10.1.10 -t 30 -P 64
```

### Step 4: Monitor During Test
```bash
# Watch PMD usage (should increase)
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-stats-show | grep "pmd usage" | head -10'

# Watch queue distribution
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-rxq-show | grep -E "pmd thread|queue-id.*pmd usage"'

# Watch packet rates
watch -n 1 'sudo ovs-vsctl get Interface dpdk0 statistics | grep -o "rx_packets=[^,]*"'
```

## Expected Results:

| Configuration | Expected Throughput |
|---------------|---------------------|
| 4 vCPUs, P=8 | 8.89 Gbps ✅ (current) |
| 8 vCPUs, P=16 | 12-15 Gbps |
| 8 vCPUs, P=32 | 15-18 Gbps |
| 8 vCPUs, P=64, 8 PMD cores | 18-22 Gbps |

## Verification:

### Check vCPU Usage in VM During Test:
```bash
# In VM during iperf:
top -H  # Should show multiple iperf threads across CPUs
mpstat -P ALL 1  # Watch per-CPU usage
```

### Check PMD Usage on Host:
```bash
sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -A 3 "pmd thread"
# processing cycles should increase from 1% to 10-30%
```

### Check Queue Distribution:
```bash
# All 8 vhost-user queues should have packets
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep "rx_q[0-7]_good_packets"
```

## Troubleshooting:

### If stuck at ~9 Gbps with 8 vCPUs:
- Check: `ethtool -S <interface> | grep queue` in VM
- Only 1-2 TX queues active → increase iperf streams (P=32/64)

### If PMD threads hit 50%+ usage:
- Add more PMD cores: `pmd-cpu-mask=0xFF` (8 cores)
- Rebalance: `sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance`

### If one PMD thread saturated (>80%):
- Rebalance queues: `sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance`
- Or manually assign queues to different PMDs

## Quick Test Script:

```bash
#!/bin/bash
# Quick performance test

echo "1. Checking vCPUs in VM..."
ssh vm@10.10.1.10 'nproc'

echo "2. Checking queues..."
ssh vm@10.10.1.10 'ethtool -l ens4 | grep Combined'

echo "3. Running iperf with increasing parallelism..."
for P in 8 16 32 64; do
    echo "=== P=$P ==="
    ssh vm@10.10.1.20 "iperf3 -c 10.10.1.10 -t 20 -P $P" | grep sender
done

echo "4. PMD stats:"
sudo ovs-appctl dpif-netdev/pmd-stats-show | grep "processing cycles"
```

