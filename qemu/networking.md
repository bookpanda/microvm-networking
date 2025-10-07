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
sudo iptables -t nat -L -v -n

```

## Multinode
```bash
# host 0
sudo ip route add 192.168.101.0/24 via 10.10.1.2
# host 1
sudo ip route add 192.168.100.0/24 via 10.10.1.1

# Enable IPv4 forwarding
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
sudo sysctl -w net.ipv4.ip_forward=1

# Accept incoming packets from the bridge
sudo iptables -I INPUT -i br0 -p udp -j ACCEPT
sudo iptables -I INPUT -i br0 -p tcp -j ACCEPT
sudo iptables -I INPUT -i br0 -p icmp -j ACCEPT

# Accept forwarding of packets from the bridge
sudo iptables -I FORWARD -i br0 -p udp -j ACCEPT
sudo iptables -I FORWARD -i br0 -p tcp -j ACCEPT
sudo iptables -I FORWARD -i br0 -p icmp -j ACCEPT

# Accept forwarding back to the bridge
sudo iptables -I FORWARD -o br0 -p icmp -j ACCEPT
sudo iptables -I FORWARD 1 -i br0 -o br0 -j ACCEPT

# Enable NAT masquerading for outbound traffic
# host 0
sudo iptables -t nat -A POSTROUTING -s 192.168.100.0/24 ! -d 192.168.100.0/24 -j MASQUERADE
sudo iptables -A FORWARD -i br0 -o enp65s0f0np0 -s 192.168.101.0/24 -j ACCEPT
sudo iptables -A FORWARD -o br0 -i enp65s0f0np0 -d 192.168.101.0/24 -j ACCEPT

# host 1
sudo iptables -t nat -A POSTROUTING -s 192.168.101.0/24 ! -d 192.168.101.0/24 -j MASQUERADE
```