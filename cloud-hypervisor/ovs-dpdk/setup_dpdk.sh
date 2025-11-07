#!/usr/bin/env bash
set -e # exit on error

BRIDGE="ovsbr0"
PORT="vhost-user1"
VHOST_PATH="/tmp/vhost-user1"
RX_QUEUES=4

DPDK_PORT="dpdk0"
DPDK_PCI="0000:41:00.1"

# create bridge if it doesn't exist
if ! sudo ovs-vsctl br-exists "$BRIDGE"; then
    sudo ovs-vsctl add-br "$BRIDGE" -- set bridge "$BRIDGE" datapath_type=netdev
    echo "✅ OVS bridge '$BRIDGE' created"
else
    echo "ℹ️ OVS bridge '$BRIDGE' already exists"
fi

# add DPDK physical port if it doesn't exist
if ! sudo ovs-vsctl list-ports "$BRIDGE" | grep -qw "$DPDK_PORT"; then
    sudo ovs-vsctl add-port "$BRIDGE" "$DPDK_PORT" -- set Interface "$DPDK_PORT" type=dpdk options:dpdk-devargs="$DPDK_PCI"
    echo "✅ OVS DPDK port '$DPDK_PORT' added"
else
    echo "ℹ️ OVS DPDK port '$DPDK_PORT' already exists"
fi

# add vhost-user port if it doesn't exist
if ! sudo ovs-vsctl list-ports "$BRIDGE" | grep -qw "$PORT"; then
    sudo ovs-vsctl add-port "$BRIDGE" "$PORT" -- set Interface "$PORT" type=dpdkvhostuserclient options:vhost-server-path="$VHOST_PATH"
    echo "✅ OVS port '$PORT' added"
else
    echo "ℹ️ OVS port '$PORT' already exists"
fi

sudo ovs-vsctl set Interface "$PORT" options:n_rxq="$RX_QUEUES"
echo "✅ OVS Rx queues for '$PORT' set to $RX_QUEUES"

sudo ovs-vsctl set Interface "$DPDK_PORT" options:n_rxq="$RX_QUEUES"
echo "✅ OVS Rx queues for '$DPDK_PORT' set to $RX_QUEUES"

sudo ovs-vsctl show
