#!/usr/bin/env bash
set -e # exit on error

# load the ovs kernel module
modprobe openvswitch
sudo service openvswitch-switch start
sudo ovs-vsctl init

# tells OvS to enable DPDK
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true
# PMD threads on cores 0-7
ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF
# DPDK library on cores 8-9
ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x300

# the above commands only tell OVS-DPDK which cores to use
# still need to isolate these cores from the rest of the system
# If you're just experimenting, taskset alone is enough.
# For production, high-throughput VMs, use isolcpus + taskset.
# isolcpus=0-7,8-9 nohz_full=0-9 rcu_nocbs=0-9
# taskset -c 0-7 ovs-vswitchd --dpdk

# Rx queues for single NIC port (match no. of PMD threads)
ovs-vsctl set Interface dpdk0 options:n_rxq=8