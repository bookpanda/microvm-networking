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

# Remove IP from physical NIC before DPDK takes it over
sudo ip addr flush dev enp65s0f0np0 2>/dev/null || true
sudo ip link set enp65s0f0np0 up

./setup_dpdk.sh

# Clean up old bridge if it exists
sudo ip link set br0 down 2>/dev/null || true
sudo ip link delete br0 2>/dev/null || true

# Clear netplan config since enp65s0f0np0 is now managed by OVS
sudo rm -f /etc/netplan/01-netcfg.yaml
echo "✅ Removed netplan config (NIC managed by OVS now)"

BRIDGE_IP="192.168.$((100 + NODE_ID)).1"
INTER_HOST_IP="10.10.1.$((NODE_ID + 1))"  # 10.10.1.1 for node0, 10.10.1.2 for node1
OTHER_VM_NETWORK="192.168.$((100 + 1 - NODE_ID)).0/24"  # 192.168.101.0/24 for node0
OTHER_HOST_IP="10.10.1.$((2 - NODE_ID))"  # 10.10.1.2 for node0, 10.10.1.1 for node1

# Configure OVS bridge with BOTH IPs: VM network + inter-host network
sudo ip addr flush dev ovsbr0 2>/dev/null || true
sudo ip addr add ${BRIDGE_IP}/24 dev ovsbr0
sudo ip addr add ${INTER_HOST_IP}/24 dev ovsbr0
sudo ip link set ovsbr0 up

# Add route to other VM network via other host
sudo ip route add ${OTHER_VM_NETWORK} via ${OTHER_HOST_IP} dev ovsbr0 2>/dev/null || true

echo "✅ OVS bridge configured:"
echo "   - VM network: ${BRIDGE_IP}/24"
echo "   - Inter-host: ${INTER_HOST_IP}/24"
echo "   - Route to ${OTHER_VM_NETWORK} via ${OTHER_HOST_IP}"

../init/clean-disk-state.sh
../init/create-cloud-init.sh

echo "✅ Node ${NODE_ID} setup complete"
