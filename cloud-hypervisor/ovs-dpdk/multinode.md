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

# host 0 (16 queues = 8 RX + 8 TX)
sudo cloud-hypervisor \
    --cpus boot=4 \
    --memory size=1024M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0-dpdk.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server,queue_size=1024

# host 1
sudo cloud-hypervisor \
    --cpus boot=4 \
    --memory size=1024M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1-dpdk.img \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server,queue_size=1024

ip link show
ip addr show
sudo iptables -t nat -L -v -n

sudo ovs-vsctl set Interface dpdk0 options:rx-steering=rss
sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance
```

## Testing
```bash
# VM IPs (both on 10.10.1.0/24 for pure L2 switching):
# - Host 0 VM: 10.10.1.10
# - Host 1 VM: 10.10.1.20

# vm 0 (10.10.1.10)
iperf3 -s

# vm 1 (10.10.1.20) - test with increasing parallelism
iperf3 -c 10.10.1.10 -t 30 -P 4
iperf3 -c 10.10.1.10 -t 300 -P 8
iperf3 -c 10.10.1.10 -t 30 -P 8
iperf3 -c 10.10.1.10 -t 30 -P 16
iperf3 -c 10.10.1.10 -t 30 -P 32
iperf3 -c 10.10.1.10 -t 30 -P 64 

# UDP test for max throughput
iperf3 -c 10.10.1.10 -u -b 20G -t 30
```
## Monitoring
```bash
##### HOST #####
sudo ovs-appctl dpif-netdev/pmd-stats-show

# processing usage of PMD threads
sudo ovs-appctl dpif-netdev/pmd-stats-show | grep "processing cycles"

# rx/tx packets processed by dpdk0
sudo ovs-vsctl get Interface dpdk0 statistics | grep -E "rx_packets|tx_packets"

# queue distribution
sudo ovs-appctl dpif-netdev/pmd-rxq-show | grep -E "pmd thread|dpdk0|vhost-user1" | head -20

sudo ovs-appctl fdb/show ovsbr0

scp ./diagnose_tx_queue1.sh cloud@10.10.1.10:/home/cloud/diagnose_tx_queue1.sh
scp ./fix_tx_queue1_bottleneck.sh cloud@10.10.1.10:/home/cloud/fix_tx_queue1_bottleneck.sh
scp /tmp/verify_vm_config.sh cloud@10.10.1.10:/home/cloud/verify_vm_config.sh


##### VM #####
nproc # no. of vCPUs
ethtool -l ens4  # Should show "Combined: 8"
# vCPU usage
mpstat -P ALL 1
# memory usage
free -h

# check vCPU allocation for each queue
for i in {0..3}; do
    cat /sys/class/net/ens4/queues/rx-$i/rps_cpus
    cat /sys/class/net/ens4/queues/tx-$i/xps_cpus
done

# clear vCPU allocation for each queue
for i in {0..3}; do
    sudo bash -c "echo 0 > /sys/class/net/ens4/queues/rx-$i/rps_cpus"
    sudo bash -c "echo 0 > /sys/class/net/ens4/queues/tx-$i/xps_cpus"
done

# spread vCPUs across queues
for i in /sys/class/net/ens4/queues/tx-*; do
    sudo bash -c "echo ff > $i/xps_cpus"   # 8 vCPUs
done
for i in /sys/class/net/ens4/queues/rx-*; do
    sudo bash -c "echo ff > $i/rps_cpus"   # 8 vCPUs
done

# pin 1 vCPU to each queue
rx_cpus=(1 2 4 8)  # CPU0, CPU1, CPU2, CPU3
i=0
for q in /sys/class/net/ens4/queues/rx-*; do
    sudo bash -c "echo ${rx_cpus[i]} > $q/rps_cpus"
    i=$((i+1))
done
tx_cpus=(10 20 40 80)  # CPU4, CPU5, CPU6, CPU7
i=0
for q in /sys/class/net/ens4/queues/tx-*; do
    sudo bash -c "echo ${tx_cpus[i]} > $q/xps_cpus"
    i=$((i+1))
done
```
### On Host During Test:
```bash
# Watch PMD usage (should increase from 0.17% to higher)
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -E "pmd thread|usage|idle" | head -20'

# Watch queue distribution
watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-rxq-show'

# dpdk0 packets
watch -n 1 'sudo ovs-vsctl get Interface dpdk0 statistics | tr "," "\n" | grep -E "tx_q[0-7]_packets|rx_q[0-7]_packets"'

# vhost-user1 packets
watch -n 1 'sudo ovs-vsctl get Interface vhost-user1 statistics | tr "," "\n" | grep -E "tx_q[0-7]_good_packets|rx_q[0-7]_good_packets"'

```

### In VM During Test:
```bash
# Check queue usage
ethtool -S ens4 | grep -E "tx_queue_[0-7]_packets|rx_queue_[0-7]_packets"

# Check interrupts spreading across cores
watch -n 1 'cat /proc/interrupts | grep virtio'
```
