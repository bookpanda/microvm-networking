# Networking
```bash
# host
sudo mkdir -p /etc/qemu
sudo bash -c 'echo "allow br0" > /etc/qemu/bridge.conf'

sudo ip link add name br0 type bridge
sudo ip addr add 192.168.100.1/24 dev br0
sudo ip link set br0 up


# vm
sudo ip addr add 192.168.100.2/24 dev ens3
sudo ip link set ens3 up
sudo ip route add default via 192.168.100.1
ip link show
ip addr show

```