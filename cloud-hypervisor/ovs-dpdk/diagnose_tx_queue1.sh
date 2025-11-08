#!/bin/bash
# Run this INSIDE the VM to diagnose why only tx_q1 is used

IFACE=$(ip -o link show | grep -v "lo:" | grep -v "NO-CARRIER" | awk -F': ' '{print $2}' | head -1)

echo "=== Why is Only TX Queue 1 Active? ==="
echo ""

echo "1. XPS Configuration (CPU → Queue Mapping):"
for i in {0..7}; do
    if [ -d /sys/class/net/$IFACE/queues/tx-$i ]; then
        XPS=$(cat /sys/class/net/$IFACE/queues/tx-$i/xps_cpus)
        echo "  tx-$i: xps_cpus=$XPS"
    fi
done
echo ""

echo "2. Queue Discipline (should be 'mq' for multi-queue):"
tc qdisc show dev $IFACE
echo ""

echo "3. Number of TX Queues Configured:"
REAL_QUEUES=$(ls -d /sys/class/net/$IFACE/queues/tx-* 2>/dev/null | wc -l)
echo "  Found $REAL_QUEUES TX queues"
ethtool -l $IFACE 2>/dev/null || echo "  ethtool -l not available"
echo ""

echo "4. Check Which CPU iperf is Running On:"
ps aux | grep iperf | grep -v grep
echo ""
pgrep iperf3 | xargs -I {} taskset -cp {}
echo ""

echo "5. IRQ Affinity (which CPUs handle network interrupts):"
cat /proc/interrupts | grep virtio | head -10
echo ""

echo "6. Network Device Queues (from driver perspective):"
cat /sys/class/net/$IFACE/device/numa_node 2>/dev/null || echo "  NUMA node: unknown"
cat /sys/class/net/$IFACE/tx_queue_len
echo ""

echo "=== LIKELY CAUSES ==="
echo ""
echo "If xps_cpus shows '00000001' or '00000002' (only CPU 0 or 1):"
echo "  → XPS is pinning all TX to queue 1!"
echo "  → FIX: Disable XPS or set to all CPUs"
echo ""
echo "If queue discipline is 'noqueue' or 'pfifo_fast':"
echo "  → Not using multi-queue properly!"
echo "  → FIX: Set to 'mq' qdisc"
echo ""
echo "If iperf is pinned to CPU 1:"
echo "  → Socket is bound to that CPU's TX queue!"
echo "  → FIX: Let iperf use all CPUs (no taskset)"
echo ""

echo "=== QUICK FIXES TO TRY ==="
echo ""
echo "# Fix 1: Completely disable XPS (let kernel round-robin)"
echo "for i in {0..7}; do echo 0 > /sys/class/net/$IFACE/queues/tx-\$i/xps_cpus 2>/dev/null; done"
echo ""
echo "# Fix 2: Enable multi-queue qdisc"
echo "tc qdisc replace dev $IFACE root mq"
echo ""
echo "# Fix 3: If iperf is CPU-pinned, restart without taskset"
echo "killall iperf3"
echo "iperf3 -c 10.10.1.10 -t 60 -P 8"
