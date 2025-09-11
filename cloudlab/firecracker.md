```bash
firectl \
--kernel=/tmp/hello-vmlinux.bin \
--root-drive=/tmp/hello-rootfs.ext4 \
--kernel-opts="console=ttyS0 noapic reboot=k panic=1 pci=off nomodules rw"
# ttys0: tty0 for kernel messages + logs to this
# noapic: disable Advanced Programmable Interrupt Controller
# reboot=k: Kernel-specific option for how reboot works (typical for microVMs)
# panic=1: if kernel panics, reboot after 1 second.
# pci=off: disable PCI bus scanning (reduces complexity)
# nomodules: don’t load kernel modules automatically.
# rw: mount the root FS read-write

# user: root, pass: root

# stop
reboot

# microVM = process, count them
ps aux | grep firecracker
ps aux | grep firecracker | grep -v grep | wc -l

# kill all firecracker processes
ps aux | grep firecracker | grep -v grep | awk '{print $2}' | xargs kill -9


```