# Multinode
```bash
# c6525-25g
./setup_node.sh 0 enp65s0f0np0
./setup_node.sh 1 enp65s0f0np0

# xl170
./setup_node.sh 0 eno49np0
./setup_node.sh 1 eno49np0

# host 0
sudo cloud-hypervisor \
    --cpus boot=4 \
    --memory size=1024M \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net "tap=tap0,mac=52:54:00:02:d9:01"


# host 1
sudo cloud-hypervisor \
    --cpus boot=4 \
    --memory size=1024M \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1.img \
    --net "tap=tap0,mac=52:54:20:11:C5:02"

# 8 vcpu, 4096M same results as 2 vcpu, 512M

# vm 0
# sudo ethtool -K ens4 tso on gso on gro on

# vm 1

ip link show
ip addr show
sudo iptables -t nat -L -v -n

```

## Testing
```bash
# vm 0
iperf3 -s

ssh cloud@192.168.100.2
nproc # no. of vCPUs
ethtool -l ens4
# vCPU usage
mpstat -P ALL 1
# memory usage
free -h

# vm 1
iperf3 -c 192.168.100.2 -t 30 -P 4
# more P doesn't help
iperf3 -c 192.168.100.2 -t 30 -P 8
iperf3 -c 192.168.100.2 -t 300 -P 16
```