# Syscall Profiling
## BPFtrace
```bash
# host
sudo apt install -y bpftrace linux-headers-$(uname -r)

# pid is second column
ps aux | grep firecracker

./bpftrace.sh
# fc_vcpu 0: a vCPU thread inside Firecracker
# firecracker: the main thread of the process
```

## perf
```bash
# host
sudo apt install -y linux-perf

ps aux | grep firecracker

sudo perf stat -e syscalls:sys_enter_sendto,syscalls:sys_enter_recvfrom -p <firecracker_pid>
```