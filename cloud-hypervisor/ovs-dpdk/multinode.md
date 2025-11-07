# Multinode
```bash
# Bus error = not enough hugepages allocated for VMs (OvS takes all)
# hugepages=on = maps hugepages from the host into the VM’s physical address space, replacing normal 4 KB pages, so the guest OS sees them as normal RAM, but backed by 2 MB pages on the host

# host 0
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm0.img \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server

# host 1
sudo cloud-hypervisor \
    --cpus boot=4,max=8 \
    --memory size=4096M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/cloudinit-vm1.img \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user1,num_queues=8,vhost_mode=server

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
iperf3 -c 192.168.100.2 -t 30 -P 8
```