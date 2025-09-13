```bash
# microVM = process, count them
ps aux | grep firecracker
ps aux | grep firecracker | grep -v grep | wc -l

# kill all firecracker processes
ps aux | grep firecracker | grep -v grep | awk '{print $2}' | xargs kill -9

go run cmd/main.go -vms=4

go run cmd/main.go -vms=4 -kernel=/tmp/vmlinux-5.10.223-no-acpi -rootfs=/tmp/debian-rootfs.ext4

ssh root@192.168.100.2
sshpass -p "root" ssh root@192.168.100.2
```
## Running experiments
1. start servers
2. start clients
3. track syscalls via PIDs
4. make each client start sending data to the server
- doens't matter if clients don't start at same time:
    - total throughput = sum all
    - per vm throughput = avg of each client-server pair
    - latency = avg of each client-server pair