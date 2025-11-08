# Why You're Stuck at 11 Gbps

## The Evidence:

```
HOST (DPDK fast path):
- PMD threads: 0.12-0.22% usage  ← 99% IDLE!
- dpdk0: 10-30% queue usage      ← Some activity
- vhost-user1: 0-4% usage        ← VM NOT PUSHING TRAFFIC!

PERFORMANCE:
- P=8: 10 Gbps
- P=64: 11 Gbps  ← Only 10% improvement from 8x streams!
```

## The Problem:

**The VM cannot generate enough traffic!** The host DPDK path is barely working because the VM guest isn't pushing packets fast enough through vhost-user.

## Most Likely Causes:

### 1. VM Only Has 4 vCPUs (NOT 8)

**Check:**
```bash
# In VM:
nproc
```

If it shows 4, your VM launched with `boot=4,max=8` which only starts 4 vCPUs.

**Fix:**
```bash
# Relaunch VM with:
--cpus boot=8  # NOT boot=4,max=8
```

### 2. Only 4 TX Queues Active in VM

**Check:**
```bash
# In VM:
ethtool -l ens4
ethtool -S ens4 | grep "tx_queue_[0-7]_packets"
```

If only queues 0-3 have packets, that's the bottleneck!

**Fix:**
```bash
# In VM:
sudo ethtool -L ens4 combined 8

# Setup XPS for proper TX distribution:
IFACE=$(ip -o link show | grep -v "lo:" | awk -F': ' '{print $2}' | head -1)
for i in {0..7}; do 
    printf '%x' $((1 << $i)) | sudo tee /sys/class/net/$IFACE/queues/tx-$i/xps_cpus
done
```

### 3. Single-Core CPU Bottleneck in VM

**Check:**
```bash
# In VM during iperf test:
mpstat -P ALL 1

# Look for:
# ❌ BAD: One CPU at 100%, others idle → bottleneck
# ✅ GOOD: All CPUs at 30-50% → parallelism working
```

**Fix:**
- If single core saturated: Need better load distribution (XPS/RPS)
- Enable multi-queue and XPS as above

### 4. iperf Not Spreading Across Queues

Even with P=64, if XPS isn't configured, all TX might go through 1-2 queues.

## Action Plan:

### Step 1: Diagnose in VM
```bash
# Copy diagnostic script to VM and run:
./diagnose_vm_bottleneck.sh
```

### Step 2: Apply Fixes in VM
```bash
# Enable all 8 queues:
sudo ethtool -L ens4 combined 8

# Setup XPS (one queue per CPU):
for i in {0..7}; do 
    printf '%x' $((1 << $i)) | sudo tee /sys/class/net/ens4/queues/tx-$i/xps_cpus
done

# Enable offloads:
sudo ethtool -K ens4 tso on gso on gro on

# Verify:
ethtool -l ens4  # Should show "Combined: 8"
```

### Step 3: Monitor During Test
```bash
# In VM during iperf:
mpstat -P ALL 1  # Watch CPU usage across all cores
ethtool -S ens4 | grep "tx_queue_[0-7]_packets"  # Watch queue distribution
```

### Step 4: Re-test
```bash
# In VM receiver:
iperf3 -s

# In VM sender:
iperf3 -c 10.10.1.10 -t 60 -P 32
```

## Expected Results After Fix:

| Before | After |
|--------|-------|
| 4 vCPUs, 4 queues active | 8 vCPUs, 8 queues active |
| vhost-user1: 0-4% usage | vhost-user1: 20-40% usage |
| Single core at 100% | All cores at 30-50% |
| **11 Gbps** | **15-20 Gbps** |

## Why This Matters:

Your DPDK path (host) is fine! Evidence:
- PMDs 99% idle (tons of capacity)
- dpdk0 working properly
- 25G link available

The bottleneck is **the VM can't push enough packets into vhost-user**. It's like having a Ferrari (DPDK) but the driver (VM) is only pressing the gas pedal 10%.

Once the VM has 8 vCPUs and 8 active TX queues with proper XPS, it can push 2x more traffic, and you'll hit 15-20 Gbps! 🚀

## Quick Diagnostic Commands:

```bash
# In VM:
nproc                          # Should be 8
ethtool -l ens4                # Should show Combined: 8
cat /sys/class/net/ens4/queues/tx-*/xps_cpus  # Should show different masks
ethtool -S ens4 | grep "tx_queue_[0-7]_packets"  # All should have packets

# During test:
mpstat -P ALL 1  # All CPUs should be active
```

