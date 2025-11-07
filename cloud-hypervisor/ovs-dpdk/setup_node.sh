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

./setup_dpdk.sh

# Clean up old bridge if it exists
sudo ip link set br0 down 2>/dev/null || true
sudo ip link delete br0 2>/dev/null || true

# Apply netplan config for physical NIC routing
sudo rm -f /etc/netplan/01-netcfg.yaml
sudo cp ./netplan-node${NODE_ID}.yaml /etc/netplan/01-netcfg.yaml
sudo netplan apply
echo "✅ netplan applied"

BRIDGE_IP="192.168.$((100 + NODE_ID)).1"

# configure OVS bridge IP (netplan can't manage OVS bridges)
sudo ip addr flush dev ovsbr0 2>/dev/null || true
sudo ip addr add ${BRIDGE_IP}/24 dev ovsbr0
sudo ip link set ovsbr0 up

echo "✅ OVS bridge configured with IP ${BRIDGE_IP}"

../init/clean-disk-state.sh
../init/create-cloud-init.sh

echo "✅ Node ${NODE_ID} setup complete"
