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


## DPDK
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
# 1 TAS on host for microVMs (sidecar)
The IPC for a one-way shared memory crossing is measured to be around 250 nanoseconds. While this is a measurable cost, it's significantly less than the multiple microseconds (or even milliseconds at the tail) of latency added by a traditional in-kernel stack1. microVMs don't need VFIO access, use virtio-net instead
- networking go through tap devices
- TAS can accelerate TCP inside the host, routing traffic between microVMs over these virtual interfaces
## current microVM flow 
External packet → host kernel → tap → microVM kernel → microVM TCP stack → user-space app
## TAS microVM flow (bypasses microVM TCP kernel)
External network → host NIC/tap → Host TAS (user-space) → Virtio-net/tap to microVM → microVM app

## Components
### Tap/virtio interception on the host
- You don’t bind virtio/tap to DPDK like VFIO, because tap interfaces are already in userspace. Instead, you tell DPDK which interface to attach to via its PMD driver
- Each microVM is connected via a tap device.
- TAS attaches to the tap device using DPDK or AF_XDP (user-space packet I/O).
- TAS can read/write packets directly on the tap interface without passing them to the microVM kernel.

### TAS-enabled API inside microVM
- MicroVM apps must use a TAS-aware API or socket library.
- When the app sends TCP data, it goes through a special virtio interface (or via a control socket) to the host TAS.
- The TAS emulates the TCP stack in user-space, so the microVM kernel is completely bypassed.
- can't use `iperf3` or `sockperf` as they use standard sockets, not TAS-aware sockets