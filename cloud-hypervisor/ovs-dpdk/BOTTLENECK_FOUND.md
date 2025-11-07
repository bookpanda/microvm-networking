# Bottleneck Found: Kernel Routing Bypass DPDK!

## The Problem

Your traffic was going through the **KERNEL**, completely bypassing DPDK:

```
❌ What was happening (SLOW - 3 Gbps):
VM → vhost-user1 (DPDK) → ovsbr0 internal port (KERNEL!) 
                              ↓
                        kernel routing
                              ↓
                        enp65s0f0np0 (KERNEL) → wire

✅ What should happen (FAST - 10+ Gbps):
VM → vhost-user1 (DPDK) → ovsbr0 (L2 switch, DPDK) → dpdk0 (DPDK) → wire
```

## Evidence

1. **dpdk0 had only 138 packets** (should have millions)
2. **PMD threads processing 12M packets** (only vhost-user local traffic)
3. **Physical NIC accessible to kernel** (enp65s0f0np0 was UP)
4. **IPs on ovsbr0 internal port** (192.168.100.1, 10.10.1.1) 
5. **Kernel routing table using ovsbr0** → kernel intercepts all traffic!

## Root Cause

When you put IPs on `ovsbr0` (which is an OVS internal/kernel port):
- Kernel networking stack takes over
- Traffic destined for those IPs goes to kernel
- Kernel routes it out via `enp65s0f0np0` (kernel driver)
- **DPDK datapath is completely bypassed!**

## The Fix

### What Changed:

1. **No IPs on ovsbr0** - Pure L2 switch only
2. **VMs use 10.10.1.0/24 directly**:
   - VM on host 0: `10.10.1.10/24`
   - VM on host 1: `10.10.1.20/24`
3. **Pure L2 switching** - OVS forwards packets between vhost-user1 ↔ dpdk0 in DPDK fast path

### Files Updated:

- `setup_node.sh`: Removes IPs from ovsbr0, keeps it pure L2
- `network-config-vm0`: VM gets 10.10.1.10/24
- `network-config-vm1`: VM gets 10.10.1.20/24

## How to Test

### On Both Hosts:

```bash
cd /users/ipankam/code/microvm-networking/cloud-hypervisor/ovs-dpdk

# Host 0:
./setup_node.sh 0

# Host 1:
./setup_node.sh 1
```

### Launch VMs:

```bash
# Host 0:
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server

# Host 1:
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1.img \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server
```

### Test:

```bash
# In VM on host 0:
ip addr  # Should show 10.10.1.10/24

# In VM on host 1:
ip addr  # Should show 10.10.1.20/24
ping 10.10.1.10  # Test connectivity

# Run iperf
iperf3 -s

# In VM on host 0:
iperf3 -c 10.10.1.20 -t 60 -P 8
```

### Verify DPDK Fast Path:

```bash
# On host during test:
sudo ovs-vsctl get Interface dpdk0 statistics | grep -E "rx_packets|tx_packets"
# Should show MILLIONS of packets now!

sudo ovs-appctl dpif-netdev/pmd-stats-show | grep "processing cycles"
# PMD usage should increase from 2% to 10-30%

sudo ovs-appctl fdb/show ovsbr0
# Should learn MACs on port 6 (dpdk0) and port 2 (vhost-user1)
```

## Expected Results

| Configuration | Throughput | DPDK Used? |
|---------------|------------|------------|
| Before (kernel routing) | 2.76 Gbps | ❌ No |
| After (wrong NIC) | 3.03 Gbps | ❌ No (wrong link) |
| **After (pure L2)** | **8-15 Gbps** | ✅ **Yes!** |

## Why This Matters

- **PMD threads were 97% idle** - they had capacity
- **Link is 25 Gbps** - bandwidth available  
- **Problem was architectural** - kernel was stealing traffic

With pure L2 switching, traffic flows entirely in DPDK userspace at line rate!

