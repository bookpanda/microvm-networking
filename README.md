# microvm-networking
### Infrastructure
Since macbooks don't have KVM support, need to use a baremetal server to run Firecracker's microVMs, so either:
- AWS: metal instances (lowest $4/hr)
- use [cloudlab.us](https://cloudlab.us) m510

### m510 Specs
- CPU: Eight-core Intel Xeon D-1548 at 2.0 GHz
- RAM: 64GB ECC Memory (4x 16 GB DDR4-2133 SO-DIMMs)
- Disk: 256 GB NVMe flash storage
- NIC: Dual-port Mellanox ConnectX-3 10 GB NIC (PCIe v3.0, 8 lanes (one port available for experiment use)
