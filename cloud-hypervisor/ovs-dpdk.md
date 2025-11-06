## Setup
```bash
sudo apt-get -y install openvswitch-switch-dpdk
sudo update-alternatives --set ovs-vswitchd /usr/lib/openvswitch-switch-dpdk/ovs-vswitchd-dpdk

#### setup OVS ####
# load the ovs kernel module
modprobe openvswitch
sudo service openvswitch-switch start
sudo ovs-vsctl init
sudo ovs-vsctl show
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true
# run on core 0-3 only
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0xf
# allocate 2G huge pages (to NUMA 0 only)
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=1024
sudo ovs-vsctl get Open_vSwitch . other_config
# run PMD (Pull Mode Driver) threads on core 0-3 only
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xf
sudo ovs-vsctl get Open_vSwitch . other_config
sudo service openvswitch-switch restart
# double check the configurations
sudo ovs-vsctl list Open_vSwitch


```