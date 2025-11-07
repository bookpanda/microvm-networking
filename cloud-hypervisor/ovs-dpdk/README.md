## Setup
- [ref](https://github.com/cloud-hypervisor/cloud-hypervisor/blob/main/docs/vhost-user-net-testing.md)
```bash
sudo apt-get -y install openvswitch-switch-dpdk
sudo update-alternatives --set ovs-vswitchd /usr/lib/openvswitch-switch-dpdk/ovs-vswitchd-dpdk

#### Configure Hugepages (REQUIRED for DPDK) ####
# Allocate 1536 hugepages × 2MB = 3GB (OVS needs ~1GB, VMs need 512MB each)
sudo sysctl -w vm.nr_hugepages=1536
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
# tells OvS to enable DPDK
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-init=true
# run on core 0-3 only (0xf = 0b1111 → cores 0,1,2,3)
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0xf
# allocate 2G huge pages (to NUMA 0 only)
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=1024
# queries the current OVS global config
sudo ovs-vsctl get Open_vSwitch . other_config
# run PMD (Pull Mode Driver) threads on core 0-3 only
# PMD will dominate CPU, leaving less for DPDK maintenance (dpdk-lcore)
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

## Running
```bash
# Create second disk for second VM (each VM needs separate disk)
sudo cp /tmp/focal-server-cloudimg-amd64.raw /tmp/focal-server-cloudimg-amd64-vm2.raw

# remove sockets before starting VMs
sudo rm -f /tmp/vhost-user*

# From one terminal. We need to give the cloud-hypervisor binary the NET_ADMIN capabilities for it to set TAP interfaces up on the host.
sudo cloud-hypervisor \
    --cpus boot=2 \
    --memory size=512M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw   \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server

sudo ip addr add 172.100.0.1/24 dev ens3
sudo ip link set up dev ens3

# From another terminal. We need to give the cloud-hypervisor binary the NET_ADMIN capabilities for it to set TAP interfaces up on the host.
# Note: Each VM needs its own disk image (can't share)
sudo cloud-hypervisor \
        --cpus boot=2 \
        --memory size=512M,hugepages=on,shared=true \
        --kernel /tmp/vmlinux.bin \
        --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
        --disk path=/tmp/focal-server-cloudimg-amd64-vm2.raw   \
        --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user2,num_queues=4,vhost_mode=server

sudo ip addr add 172.100.0.2/24 dev ens3
sudo ip link set up dev ens3
```