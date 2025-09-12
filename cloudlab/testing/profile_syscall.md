# Syscall Profiling
## BPFtrace
```bash
# host
sudo apt install -y bpftrace linux-headers-$(uname -r)

# pid is second column
ps aux | grep firecracker

sudo bpftrace -e 'tracepoint:syscalls:sys_enter_sendto /pid == 119718 { @[comm] = count(); }'

```

## perf
```bash
# host
sudo apt install -y linux-perf

ps aux | grep firecracker

sudo perf stat -e syscalls:sys_enter_sendto,syscalls:sys_enter_recvfrom -p <firecracker_pid>
```