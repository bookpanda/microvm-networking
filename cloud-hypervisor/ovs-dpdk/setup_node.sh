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

# Setup NAT/masquerading for VM internet access
VM_NETWORK="192.168.$((100 + NODE_ID)).0/24"
EXT_IFACE=$(ip route | grep default | awk '{print $5}')
# Remove any existing rule for this network
sudo iptables -t nat -D POSTROUTING -s ${VM_NETWORK} -o ${EXT_IFACE} -j MASQUERADE 2>/dev/null || true
# Add masquerading rule for VM network
sudo iptables -t nat -A POSTROUTING -s ${VM_NETWORK} -o ${EXT_IFACE} -j MASQUERADE
echo "✅ NAT configured for ${VM_NETWORK} via ${EXT_IFACE}"

# Physical NIC that will be used for DPDK (should match setup_dpdk.sh)
DPDK_NIC="enp65s0f0np0"

# Remove IP from physical NIC and disable kernel routing through it
sudo ip addr flush dev $DPDK_NIC 2>/dev/null || true
sudo ip link set $DPDK_NIC up
# Prevent kernel from using this interface for routing (set to no-arp, no-multicast)
sudo ip link set $DPDK_NIC arp off
sudo ip link set $DPDK_NIC multicast off
# Remove from routing table
sudo ip route flush dev $DPDK_NIC 2>/dev/null || true
echo "✅ Prepared $DPDK_NIC for DPDK (kernel routing disabled)"

./setup_dpdk.sh

# Clean up old bridge if it exists
sudo ip link set br0 down 2>/dev/null || true
sudo ip link delete br0 2>/dev/null || true

# Clear netplan config since enp65s0f0np0 is now managed by OVS
sudo rm -f /etc/netplan/01-netcfg.yaml
echo "✅ Removed netplan config (NIC managed by OVS now)"

# DON'T put IPs on ovsbr0! Keep it pure L2 for DPDK fast path
# The internal port causes kernel routing which bypasses DPDK entirely
sudo ip addr flush dev ovsbr0 2>/dev/null || true
sudo ip link set ovsbr0 up

echo "✅ OVS bridge configured as pure L2 switch (no IPs - DPDK fast path enabled)"
echo "   VMs should use IPs from 10.10.1.0/24 network directly"
echo "   Example: Host 0 VM: 10.10.1.10/24, Host 1 VM: 10.10.1.20/24"

../init/clean-disk-state.sh
../init/create-cloud-init.sh

echo "✅ Node ${NODE_ID} setup complete"
