# Packet Flow
1. from wire -> NIC
2. DPDK mode: NIC is detached from kernel stack
3. VFIO kernel driver (vfio-pci)
4. DPDK EAL (Environment Abstraction Layer)
    - Initializes NIC, hugepages, DMA access
    - Bypasses kernel networking stack 
5. Packets stored in hugepages
6. User-space DPDK App (app business logic is here)
    - Router, Firewall, etc.
    - Reads/writes NIC rings
7. Processed Packets Sent Back to NIC


## Installing DPDK
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
# moves a NIC (here PCI address 0000:03:00.0) from its default kernel driver to VFIO, so DPDK can use it directly
# Only one driver can control a NIC at a time
# don't run this command, you'll lose access to SSH
sudo dpdk-devbind.py --bind=vfio-pci 0000:03:00.0

# dpdk-testpmd: tool for testing DPDK applications
# -l 0-1: use cores 0 and 1
# -n 4: 4 memory channels (DPDK NUMA config)
# port-topology = how ports are connected inside testpmd
sudo dpdk-testpmd -l 0-1 -n 4 -- --port-topology=chained
```