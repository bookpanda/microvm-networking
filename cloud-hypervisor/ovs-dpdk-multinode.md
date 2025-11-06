# Multinode
```bash
# host
sudo mkdir -p /etc/qemu
sudo bash -c 'echo "allow br0" > /etc/qemu/bridge.conf'
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
sudo ip link delete tap0 2>/dev/null
sudo ip link delete br0 2>/dev/null
sudo rm -f /tmp/vhost-user*


# host 0
sudo ip addr add 192.168.100.1/24 dev ovsbr0
sudo ip link set ovsbr0 up
sudo ip route add 192.168.101.0/24 via 10.10.1.2

sudo cloud-hypervisor \
    --cpus boot=2 \
    --memory size=512M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw   \
    --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server

# host 1
sudo ip addr add 192.168.101.1/24 dev ovsbr0
sudo ip link set ovsbr0 up
sudo ip route add 192.168.100.0/24 via 10.10.1.1

sudo cloud-hypervisor \
    --cpus boot=2 \
    --memory size=512M,hugepages=on,shared=true \
    --kernel /tmp/vmlinux.bin \
    --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
    --disk path=/tmp/focal-server-cloudimg-amd64.raw   \
    --net mac=52:54:20:11:C5:02,vhost_user=true,socket=/tmp/vhost-user1,num_queues=4,vhost_mode=server


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