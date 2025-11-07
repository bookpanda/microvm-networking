#!/usr/bin/env bash
set -e # exit on error

### config hugepages ###
# 8GB total (OVS needs ~2GB, VMs need up to 2GB each)
sudo sysctl -w vm.nr_hugepages=4096
grep Huge /proc/meminfo
echo "✅ Hugepages allocated"

# Mount hugepage filesystem
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge 2>/dev/null || true
# verify it is mounted
mount | grep huge
echo "✅ Hugepage filesystem mounted"

# load the ovs kernel module
modprobe openvswitch
sudo service openvswitch-switch start
sudo ovs-vsctl init
echo "✅ OVS initialized"

# tells OvS to enable DPDK
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true

### OPTIMIZED CONFIGURATION FOR 2 VMs ###
# PMD threads on cores 2-3 ONLY (1 core per VM) - mask: 0xC = binary 1100
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xC

# DPDK library on core 4 - mask: 0x10 = binary 10000
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x10

# allocate 2GB for OVS (enough for packet buffers)
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=2048

echo "✅ OVS DPDK configured (2 PMD threads on cores 2-3)"

# NOTE: For maximum performance, add to kernel boot params (then reboot):
# isolcpus=2-12 nohz_full=2-12 rcu_nocbs=2-12
# This isolates cores 2-12 from the scheduler
# Cores: 0-1 (system), 2-3 (PMD), 4 (dpdk), 5-8 (VM1), 9-12 (VM2)

sudo service openvswitch-switch restart
sudo ovs-vsctl get Open_vSwitch . other_config
echo "✅ OVS restarted with optimized config"

echo ""
echo "⚠️  IMPORTANT: Pin your VMs to cores after starting them:"
echo "   VM1: sudo taskset -acp 5-8 \$VM1_PID"
echo "   VM2: sudo taskset -acp 9-12 \$VM2_PID"

