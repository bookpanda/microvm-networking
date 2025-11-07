#!/usr/bin/env bash
set -e # exit on error

BRIDGE="ovsbr0"
PORT="vhost-user1"
VHOST_PATH="/tmp/vhost-user1"
RX_QUEUES=8

# create bridge if it doesn't exist
if ! sudo ovs-vsctl br-exists "$BRIDGE"; then
    sudo ovs-vsctl add-br "$BRIDGE" -- set bridge "$BRIDGE" datapath_type=netdev
    echo "✅ OVS bridge '$BRIDGE' created"
else
    echo "ℹ️ OVS bridge '$BRIDGE' already exists"
fi

# add port if it doesn't exist
if ! sudo ovs-vsctl list-ports "$BRIDGE" | grep -qw "$PORT"; then
    sudo ovs-vsctl add-port "$BRIDGE" "$PORT" -- set Interface "$PORT" type=dpdkvhostuserclient options:vhost-server-path="$VHOST_PATH"
    echo "✅ OVS port '$PORT' added"
else
    echo "ℹ️ OVS port '$PORT' already exists"
fi

sudo ovs-vsctl set Interface "$PORT" options:n_rxq="$RX_QUEUES"
echo "✅ OVS Rx queues for '$PORT' set to $RX_QUEUES"

sudo ovs-vsctl show
