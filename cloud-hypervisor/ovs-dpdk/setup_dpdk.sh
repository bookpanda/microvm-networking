#!/usr/bin/env bash
set -e # exit on error

### config hugepages ###
# 16GB
sudo sysctl -w vm.nr_hugepages=8192
grep Huge /proc/meminfo

# Mount hugepage filesystem
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge
# verify it is mounted
mount | grep huge

# load the ovs kernel module
modprobe openvswitch
sudo service openvswitch-switch start
sudo ovs-vsctl init

# tells OvS to enable DPDK
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true
# PMD threads on cores 0-7
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF
# DPDK library on cores 8-9
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x300
# allocate 16G huge pages
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=16384

# the above commands only tell OVS-DPDK which cores to use
# still need to isolate these cores from the rest of the system
# If you're just experimenting, taskset alone is enough.
# For production, high-throughput VMs, use isolcpus + taskset.
# isolcpus=0-7,8-9 nohz_full=0-9 rcu_nocbs=0-9
# taskset -c 0-7 ovs-vswitchd --dpdk

sudo service openvswitch-switch restart

sudo ovs-vsctl add-br ovsbr0 -- set bridge ovsbr0 datapath_type=netdev
sudo ovs-vsctl add-port ovsbr0 vhost-user1 -- set Interface vhost-user1 type=dpdkvhostuserclient options:vhost-server-path=/tmp/vhost-user1
# Rx queues for single NIC port (match no. of PMD threads)
sudo ovs-vsctl set Interface vhost-user1 options:n_rxq=8
sudo ovs-vsctl show
