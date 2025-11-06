## Setup
```bash
sudo apt-get -y install openvswitch-switch-dpdk
sudo update-alternatives --set ovs-vswitchd /usr/lib/openvswitch-switch-dpdk/ovs-vswitchd-dpdk

#### Configure Hugepages (REQUIRED for DPDK) ####
# Allocate 1024 hugepages × 2MB = 2GB
sudo sysctl -w vm.nr_hugepages=1024
grep Huge /proc/meminfo
# Mount hugepage filesystem
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge
mount | grep huge

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

# create a bridge
sudo ovs-vsctl add-br ovsbr0 -- set bridge ovsbr0 datapath_type=netdev
# create two DPDK ports and add them to the bridge
# dpdkvhostuserclient: VS waits for the VM to create the socket, then connects to it
# dpdkvhostuser: OVS creates the socket and waits for VMs to connect
sudo ovs-vsctl add-port ovsbr0 vhost-user1 -- set Interface vhost-user1 type=dpdkvhostuserclient options:vhost-server-path=/tmp/vhost-user1
sudo ovs-vsctl add-port ovsbr0 vhost-user2 -- set Interface vhost-user2 type=dpdkvhostuserclient options:vhost-server-path=/tmp/vhost-user2
# set the number of rx queues
sudo ovs-vsctl set Interface vhost-user1 options:n_rxq=2
sudo ovs-vsctl set Interface vhost-user2 options:n_rxq=2

# show vhost
sudo ovs-vsctl show
```