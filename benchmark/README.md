```bash
# microVM = process, count them
ps aux | grep firecracker
ps aux | grep firecracker | grep -v grep | wc -l

# kill all firecracker processes
ps aux | grep firecracker | grep -v grep | awk '{print $2}' | xargs kill -9

go run cmd/main.go -vms=5

go run cmd/main.go -vms=3 -kernel=/tmp/vmlinux-5.10.223-no-acpi -rootfs=/tmp/debian-rootfs.ext4

sudo dhcpd -cf /tmp/vm-dhcp.conf br0
```