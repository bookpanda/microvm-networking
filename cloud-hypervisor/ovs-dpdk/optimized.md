# Optimized OVS-DPDK Setup with CPU Isolation

## CPU Allocation Strategy (32 CPUs available)
```
Cores 0-1:   System/OS processes
Cores 2-5:   OVS-DPDK PMD threads (4 cores for packet processing)
Cores 6-9:   VM1 vCPUs (4 vCPUs pinned)
Cores 10-13: VM2 vCPUs (4 vCPUs pinned)
Cores 14-31: Reserved for more VMs or other workloads
```

## Setup

### 1. Isolate CPU cores from scheduler
```bash
# Edit /etc/default/grub and add to GRUB_CMDLINE_LINUX:
# isolcpus=2-13 nohz_full=2-13 rcu_nocbs=2-13

# Then update grub and reboot:
sudo update-grub
sudo reboot
```

### 2. Configure OVS-DPDK with isolated cores
```bash
# PMD threads on cores 2-5 (mask: 0x3c = binary 111100)
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0x3c

# Allocate more hugepages (4GB = 2048 pages)
sudo sysctl -w vm.nr_hugepages=2048

# Restart OVS
sudo systemctl restart openvswitch-switch
```

### 3. Launch VMs with CPU pinning
```bash
# VM1 on cores 6-9
sudo cloud-hypervisor \
    --cpus boot=4,topology=1:1:4:1 \
    --memory size=2G,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server

# Then pin the process to cores 6-9:
VM1_PID=$(pgrep -f "vhost-user1")
sudo taskset -acp 6-9 $VM1_PID

# VM2 on cores 10-13
sudo cloud-hypervisor \
    --cpus boot=4,topology=1:1:4:1 \
    --memory size=2G,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64-vm2.raw \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user2,num_queues=4,vhost_mode=server

VM2_PID=$(pgrep -f "vhost-user2")
sudo taskset -acp 10-13 $VM2_PID
```

### 4. Verify CPU isolation
```bash
# Check PMD thread placement
sudo ovs-appctl dpif-netdev/pmd-stats-show

# Check VM CPU affinity
ps -eLo pid,tid,psr,comm | grep cloud-hypervisor
```

## Expected Performance Improvements
- **Throughput**: Should approach 20+ Gbps (limited by vhost-user, not NIC)
- **Latency**: Should improve to ~10-30µs (better than multi-node!)
- **CPU usage**: More predictable, no context switching

## Why this is better than multi-node:
✅ Lower latency (no physical NIC, no kernel network stack)
✅ Higher throughput (DPDK zero-copy)
✅ Deterministic performance (isolated cores)

