#!/usr/bin/env bash
set -e

# Usage: ./setup-node.sh <node_id>
# node_id: 0 or 1

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <node_id>"
    echo "  node_id: 0 or 1"
    exit 1
fi

NODE_ID=$1

if [ "$NODE_ID" != "0" ] && [ "$NODE_ID" != "1" ]; then
    echo "Error: node_id must be 0 or 1"
    exit 1
fi


echo "Setting up node ${NODE_ID}"

sudo rm -f /tmp/vhost-user*

sudo mkdir -p /etc/qemu
sudo bash -c 'echo "allow br0" > /etc/qemu/bridge.conf'
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward

# Clean up OvS bridge
sudo ip link set ovsbr0 down || true
sudo ovs-vsctl del-br ovsbr0 || true

# Apply netplan config
sudo netplan apply --file ./netplan-node${NODE_ID}.yaml

# create tap0, br0
sudo ip link delete tap0 2>/dev/null
sudo ip link delete br0 2>/dev/null

sudo ip link add name br0 type bridge || true
sudo ip link set br0 up || true
sudo ip addr add 192.168.10${NODE_ID}.1/24 dev br0 || true

sudo ip tuntap add dev tap0 mode tap user $USER || true
sudo ip link set tap0 master br0 || true
sudo ip link set tap0 up || true


../init/clean-disk-state.sh
../init/create-cloud-init.sh

echo "✅ Node ${NODE_ID} setup complete"
