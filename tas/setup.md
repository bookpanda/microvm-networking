# Setup
```bash
sudo apt install -y dpdk dpdk-dev libdpdk-dev

# allocate 1024 hugepages in mem (2MB each, DPDK uses for zero-copy)
echo 1024 | sudo tee /proc/sys/vm/nr_hugepages
# check current hugepages allocation
grep HugePages /proc/meminfo

# loads the VFIO driver, allows DPDK to bind NICs directly to user space
sudo modprobe vfio-pci
# check
lsmod | grep vfio

# lists all NICs and shows which driver each NIC uses
sudo dpdk-devbind.py --status

```