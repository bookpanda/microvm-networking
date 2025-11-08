#!/bin/bash
# Run this INSIDE the VM to fix TX queue distribution

IFACE=$(ip -o link show | grep -v "lo:" | grep -v "NO-CARRIER" | awk -F': ' '{print $2}' | head -1)

echo "=== Fixing TX queue distribution for $IFACE ==="

# 1. Disable XPS (it's pinning connections to specific queues)
for i in {0..7}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        echo 0 > /sys/class/net/$IFACE/queues/tx-$i/xps_cpus
        echo "Disabled XPS on tx-$i"
    fi
done

# 2. Set XDP/BPF TX hash to use all queues
# (This is what actually distributes packets across TX queues)
for i in {0..7}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        printf '%x' $((2**8 - 1)) > /sys/class/net/$IFACE/queues/tx-$i/xps_cpus
        echo "Set tx-$i XPS to all CPUs (ff)"
    fi
done

# 3. Increase TX queue length (reduce drops)
ip link set $IFACE txqueuelen 10000

# 4. Verify current queue config
echo ""
echo "Current TX queue XPS configuration:"
for i in {0..7}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        XPS=$(cat /sys/class/net/$IFACE/queues/tx-$i/xps_cpus)
        echo "  tx-$i: xps_cpus=$XPS"
    fi
done

echo ""
echo "✅ TX queue configuration updated"
echo "Now restart your iperf test"
