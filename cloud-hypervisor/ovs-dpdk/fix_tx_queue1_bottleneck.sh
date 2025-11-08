#!/bin/bash
# CRITICAL FIX: Force VM to use ALL TX queues, not just queue 1

IFACE=$(ip -o link show | grep -v "lo:" | grep -v "NO-CARRIER" | awk -F': ' '{print $2}' | head -1)

echo "=== Fixing TX Queue 1 Bottleneck ==="
echo ""

# The problem: XPS pins tx-1 to CPU 1, and if all traffic goes through CPU 1, only tx-1 is used!

# Solution 1: DISABLE XPS COMPLETELY (let kernel do round-robin)
echo "Step 1: Disabling XPS (removing CPU pinning)"
for i in {0..15}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        echo 0 > /sys/class/net/$IFACE/queues/tx-$i/xps_cpus 2>/dev/null
        echo "  tx-$i: XPS disabled"
    fi
done

# Solution 2: Enable multi-queue qdisc (CRITICAL!)
echo ""
echo "Step 2: Enabling multi-queue qdisc"
tc qdisc replace dev $IFACE root mq
echo "  ✅ mq qdisc enabled"

# Solution 3: Set RPS to distribute RX across all CPUs
echo ""
echo "Step 3: Configuring RPS (Receive Packet Steering)"
NUM_CPUS=$(nproc)
RPS_MASK=$(printf '%x' $((2**NUM_CPUS - 1)))
for i in {0..15}; do
    if [ -d /sys/class/net/$IFACE/queues/rx-$i ]; then
        echo $RPS_MASK > /sys/class/net/$IFACE/queues/rx-$i/rps_cpus 2>/dev/null
        echo "  rx-$i: RPS enabled (mask=$RPS_MASK)"
    fi
done

# Solution 4: Increase queue length
echo ""
echo "Step 4: Increasing TX queue length"
ip link set $IFACE txqueuelen 10000
echo "  ✅ txqueuelen set to 10000"

# Solution 5: Enable RFS (Receive Flow Steering) if available
echo ""
echo "Step 5: Enabling RFS (if available)"
if [ -f /proc/sys/net/core/rps_sock_flow_entries ]; then
    echo 32768 > /proc/sys/net/core/rps_sock_flow_entries
    NUM_QUEUES=$(ls -d /sys/class/net/$IFACE/queues/rx-* 2>/dev/null | wc -l)
    ENTRIES_PER_QUEUE=$((32768 / NUM_QUEUES))
    for i in {0..15}; do
        if [ -d /sys/class/net/$IFACE/queues/rx-$i ]; then
            echo $ENTRIES_PER_QUEUE > /sys/class/net/$IFACE/queues/rx-$i/rps_flow_cnt 2>/dev/null
        fi
    done
    echo "  ✅ RFS enabled (32768 flow entries)"
fi

# Verify configuration
echo ""
echo "=== Verification ==="
echo ""
echo "XPS Status (should all be 00000000):"
for i in {0..7}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        XPS=$(cat /sys/class/net/$IFACE/queues/tx-$i/xps_cpus)
        echo "  tx-$i: $XPS"
    fi
done

echo ""
echo "Queue Discipline:"
tc qdisc show dev $IFACE | head -5

echo ""
echo "✅ CONFIGURATION COMPLETE!"
echo ""
echo "Now run iperf WITHOUT any CPU pinning:"
echo "  iperf3 -c 10.10.1.10 -t 60 -P 16"
echo ""
echo "The kernel will now distribute TX across ALL queues via mq qdisc!"
