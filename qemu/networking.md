# Networking
```bash
# host
sudo mkdir -p /etc/qemu
sudo bash -c 'echo "allow br0" > /etc/qemu/bridge.conf'
# echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward


# host 0
sudo ip link add name br0 type bridge
sudo ip addr add 192.168.100.1/24 dev br0
sudo ip link set br0 up

# host 1
sudo ip link add name br0 type bridge
sudo ip addr add 192.168.101.1/24 dev br0
sudo ip link set br0 up
# sudo iptables -t nat -A POSTROUTING -s 192.168.101.0/24 -o eth0 -j MASQUERADE


# vm 0
sudo ip addr add 192.168.100.2/24 dev ens3
sudo ip link set ens3 up
sudo ip route add default via 192.168.100.1

# vm 1
sudo ip addr add 192.168.101.2/24 dev ens3
sudo ip link set ens3 up
sudo ip route add default via 192.168.101.1
# sudo bash -c 'echo "nameserver 8.8.8.8" > /etc/resolv.conf'
# echo "ubuntu" | sudo tee /etc/hostname
# echo "127.0.1.1 ubuntu" | sudo tee -a /etc/hosts


ip link show
ip addr show

```