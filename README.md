# microvm-networking
### Infrastructure
Since macbooks don't have KVM support, need to use a baremetal server to run Firecracker's microVMs, so either:
- AWS: metal instances (lowest $4/hr)
- use [cloudlab.us](https://cloudlab.us) m510

### m510 (BM) Specs
- [Cloudlab specs](https://docs.cloudlab.us/hardware.html)
- CPU: Eight-core Intel Xeon D-1548 at 2.0 GHz
- RAM: 64GB ECC Memory (4x 16 GB DDR4-2133 SO-DIMMs)
- Disk: 256 GB NVMe flash storage
- NIC: Dual-port Mellanox ConnectX-3 10 GB NIC (PCIe v3.0, 8 lanes (one port available for experiment use)

### Firecracker microVM Specs
- kernel: [vmlinux-5.10.223-no-acpi](http://spec.ccfc.min.s3.amazonaws.com/firecracker-ci/v1.10/x86_64/vmlinux-5.10.223-no-acpi) (36MB)
- root FS: [debian-rootfs.ext4](http://spec.ccfc.min.s3.amazonaws.com/ci-artifacts/disks/x86_64/debian.rootfs.ext4) (1000MB)