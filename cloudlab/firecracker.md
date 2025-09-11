## Networking
```bash
# host
# create TAP device called "tap0" in mode tap (layer-2)
sudo ip tuntap add dev tap0 mode tap
# activate tap0 interface
sudo ip link set tap0 up
# add IP address to tap0 interface
sudo ip addr add 192.168.100.1/24 dev tap0

# iptables rules to enable packet forwarding for VM
DEVICE_NAME=eno1
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
sudo iptables -t nat -A POSTROUTING -o $DEVICE_NAME -j MASQUERADE
sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i tap0 -o $DEVICE_NAME -j ACCEPT
MAC="$(cat /sys/class/net/tap0/address)"

# check
ip link show tap0

# VM
# activate eth0 interface
ip link set eth0 up
# add IP address to eth0 interface
ip addr add 192.168.100.2/24 dev eth0
# add default gateway for vm
ip route add default via 192.168.100.1
# add DNS server to resolv.conf
echo "nameserver 8.8.8.8" > /etc/resolv.conf
# check
ip link
```
```bash
sudo setfacl -m u:${USER}:rw /dev/kvm
sudo usermod -aG kvm $USER

MAC="$(cat /sys/class/net/tap0/address)"
firectl \
--kernel=/tmp/hello-vmlinux.bin \
--root-drive=/tmp/hello-rootfs.ext4 \
--kernel-opts="console=ttyS0 noapic reboot=k panic=1 pci=off rw" \
--tap-device tap0/$MAC
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

unused
```bash
ls -l /usr/local/bin/
export PATH=$PATH:/usr/local/bin
sockperf --version
ldd /usr/local/bin/sockperf

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
/usr/local/bin/sockperf
```