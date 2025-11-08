#!/usr/bin/env bash
set -e # exit on error

BRIDGE="ovsbr0"
PORT="vhost-user1"
VHOST_PATH="/tmp/vhost-user1"
RX_QUEUES=8

DPDK_PORT="dpdk0"
DPDK_NIC="enp65s0f0np0"  # Physical NIC to use for DPDK (MUST match setup_node.sh)

# Dynamically get MAC address of the physical NIC
if [ ! -e "/sys/class/net/$DPDK_NIC" ]; then
    echo "❌ Error: Interface $DPDK_NIC not found"
    exit 1
fi

DPDK_MAC=$(cat /sys/class/net/$DPDK_NIC/address)
echo "Using $DPDK_NIC with MAC: $DPDK_MAC"

# create bridge if it doesn't exist
if ! sudo ovs-vsctl br-exists "$BRIDGE"; then
    sudo ovs-vsctl add-br "$BRIDGE" -- set bridge "$BRIDGE" datapath_type=netdev
    echo "✅ OVS bridge '$BRIDGE' created"
else
    echo "ℹ️ OVS bridge '$BRIDGE' already exists"
fi

# Remove and recreate DPDK physical port to ensure correct configuration
if sudo ovs-vsctl list-ports "$BRIDGE" | grep -qw "$DPDK_PORT"; then
    echo "ℹ️ Removing existing '$DPDK_PORT' to recreate with correct MAC"
    sudo ovs-vsctl del-port "$BRIDGE" "$DPDK_PORT"
fi

# Add DPDK physical port with correct MAC
# Using class=eth,mac for Mellanox NICs (they work better this way than with PCI binding)
sudo ovs-vsctl add-port "$BRIDGE" "$DPDK_PORT" -- set Interface "$DPDK_PORT" type=dpdk options:dpdk-devargs="class=eth,mac=$DPDK_MAC"
echo "✅ OVS DPDK port '$DPDK_PORT' added (using MAC $DPDK_MAC)"

# add vhost-user port if it doesn't exist
if ! sudo ovs-vsctl list-ports "$BRIDGE" | grep -qw "$PORT"; then
    sudo ovs-vsctl add-port "$BRIDGE" "$PORT" -- set Interface "$PORT" type=dpdkvhostuserclient options:vhost-server-path="$VHOST_PATH"
    echo "✅ OVS port '$PORT' added"
else
    echo "ℹ️ OVS port '$PORT' already exists"
fi

sudo ovs-vsctl set Interface "$PORT" options:n_rxq="$RX_QUEUES" options:n_txq="$RX_QUEUES"
echo "✅ OVS RX/TX queues for '$PORT' set to $RX_QUEUES"

sudo ovs-vsctl set Interface "$DPDK_PORT" options:n_rxq="$RX_QUEUES" options:n_txq="$RX_QUEUES"
echo "✅ OVS RX/TX queues for '$DPDK_PORT' set to $RX_QUEUES"

sudo ovs-vsctl show
