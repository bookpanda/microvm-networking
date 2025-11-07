# Multinode
```bash
# load VFIO kernel modules
sudo modprobe vfio
sudo modprobe vfio-pci
lsmod | grep vfio
sudo dmesg | grep -e DMAR -e IOMMU

# bind NIC to DPDK
ip a # prob eno34np1 is unused 
# get info on NIC, see PCI address (0000:01:00.1)
sudo ethtool -i enp65s0f1np1
sudo dpdk-devbind.py -b vfio-pci 0000:41:00.1
sudo dpdk-devbind.py --status

# troubleshoot: can't ping the other host
# unbind
sudo dpdk-devbind.py -u 0000:41:00.1
# rebind to mlx5_core (kernel driver)
sudo dpdk-devbind.py -b mlx5_core 0000:41:00.1
# remove from OVS
sudo ovs-vsctl del-port ovsbr0 dpdk0

sudo ovs-vsctl show
sudo ovs-vsctl get Interface vhost-user1 statistics
sudo ovs-appctl dpif-netdev/pmd-rxq-show
sudo ovs-vsctl list Interface dpdk0 | grep -E "n_rxq|options"

# Bus error = not enough hugepages allocated for VMs (OvS takes all)
# hugepages=on = maps hugepages from the host into the VM’s physical address space, replacing normal 4 KB pages, so the guest OS sees them as normal RAM, but backed by 2 MB pages on the host

# host 0
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server

# host 1
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1.img \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server

ip link show
ip addr show
sudo iptables -t nat -L -v -n

```

## Testing
```bash
# vm 0
iperf3 -s

# vm 1 - various parallel stream counts
iperf3 -c 192.168.100.2 -t 60 -P 8 -w 4M
iperf3 -c 192.168.100.2 -t 60 -P 16 -w 4M
iperf3 -c 192.168.100.2 -t 60 -P 32 -w 4M

# UDP test for max throughput
iperf3 -c 192.168.100.2 -u -b 20G -t 30
```