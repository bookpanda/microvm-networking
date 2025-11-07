#!/usr/bin/env bash
set -e # exit on error

### config hugepages ###
# 16GB
sudo sysctl -w vm.nr_hugepages=8192
grep Huge /proc/meminfo
echo "✅ Hugepages allocated"

# Mount hugepage filesystem
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge
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
# PMD threads on cores 0-7
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF
# DPDK library on cores 8-9
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x300
# allocate 8GB for OVS
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=8192
echo "✅ OVS DPDK configured"

# the above commands only tell OVS-DPDK which cores to use
# still need to isolate these cores from the rest of the system
# If you're just experimenting, taskset alone is enough.
# For production, high-throughput VMs, use isolcpus + taskset.
# isolcpus=0-7,8-9 nohz_full=0-9 rcu_nocbs=0-9
# taskset -c 0-7 ovs-vswitchd --dpdk

sudo service openvswitch-switch restart
sudo ovs-vsctl get Open_vSwitch . other_config
echo "✅ OVS restarted"
