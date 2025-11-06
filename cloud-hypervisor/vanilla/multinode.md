# Multinode
```bash
# host 0
sudo cloud-hypervisor \
    --cpus boot=2 \
    --memory size=512M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net "tap=tap0,mac=52:54:00:02:d9:01"


# host 1
sudo cloud-hypervisor \
    --cpus boot=2 \
    --memory size=512M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1.img \
    --net "tap=tap0,mac=52:54:20:11:C5:02"


# vm 0
sudo ip addr add 192.168.100.2/24 dev ens3
sudo ip link set ens3 up
sudo ip route add default via 192.168.100.1

# vm 1
sudo ip addr add 192.168.101.2/24 dev ens3
sudo ip link set ens3 up
sudo ip route add default via 192.168.101.1

ip link show
ip addr show
sudo iptables -t nat -L -v -n

```

## Testing
```bash
# vm 0
iperf3 -s


# vm 1
iperf3 -c 192.168.100.2 -t 30 -P 4
```