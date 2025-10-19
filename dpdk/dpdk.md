# Setup
```bash
sudo apt update
sudo apt install -y dpdk dpdk-dev cpuset

# reserve hugepages
# 1024 × 2 MB = 2 GB mem for hugepages
sudo sysctl -w vm.nr_hugepages=1024
grep Huge /proc/meminfo

# mount hugepage FS
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge
mount | grep huge

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
```

## Running
```bash
# run DPDK testpmd (example app)
# cores 21-23 are reserved for DPDK threads, 4 memory channels per NUMA node
# --vdev creates a virtual device (vhost-user backend)
# net_vhost0 is the name of the DPDK virtual device
# iface=/tmp/sock0 specifies the socket file that VM will connect to via vhost-user protocol
# --file-prefix ensures shared memory files have unique names (for multi-process)
# Everything before -- configures DPDK, after = testpmd options (-i = interactive mode)
sudo dpdk-testpmd -l 29-31 -n 4 -a 0000:41:00.1 \
  --vdev 'net_vhost0,iface=/mnt/huge/sock0' \
  --huge-dir=/mnt/huge --file-prefix=vhost -- -i

# --no-pci: don't touch NIC
sudo dpdk-testpmd -l 8-11 -n 4 --no-pci \
  --vdev 'net_vhost0,iface=/mnt/huge/sock0' \
  --huge-dir=/mnt/huge --file-prefix=vhost -- -i
```

## Test with Firecracker
```bash
sudo firecracker --no-api --config-file fc_config.json
# user: root, pass: root
```

## Troubleshoot
```bash
lscpu | grep NUMA
# cores with ssh process
ps -eLo pid,cls,rtprio,pri,psr,comm | grep ssh
# unbind NIC from DPDK
sudo dpdk-devbind.py -u 0000:01:00.1
# check cset set
sudo cset set
sudo cset shield --reset

# creates 2 cpusets
# 1. system cpuset (0-19): Linux kernel threads, SSH, and normal user processes (everything else)
# 2. user cpuset (20-31): special workload (e.g. DPDK)
sudo cset shield --cpu=28-31 --kthread=on
```
