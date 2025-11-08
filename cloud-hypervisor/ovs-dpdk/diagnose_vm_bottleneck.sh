#!/usr/bin/env bash
# Run this script INSIDE the VM to diagnose performance bottleneck

echo "=== VM Performance Diagnosis ==="
echo ""

echo "1. vCPU Count:"
VCPUS=$(nproc)
echo "   vCPUs available: $VCPUS"
if [ $VCPUS -lt 8 ]; then
    echo "   ⚠️  WARNING: Only $VCPUS vCPUs! Need 8 for best performance"
    echo "   → Relaunch VM with: --cpus boot=8"
else
    echo "   ✅ Good: 8 vCPUs available"
fi

echo ""
echo "2. Network Interface:"
IFACE=$(ip -o link show | grep -v "lo:" | awk -F': ' '{print $2}' | head -1)
echo "   Interface: $IFACE"

echo ""
echo "3. Queue Configuration:"
if command -v ethtool &> /dev/null; then
    ethtool -l $IFACE 2>/dev/null || echo "   ethtool not available"
    CURRENT_QUEUES=$(ethtool -l $IFACE 2>/dev/null | grep "Current hardware" -A 3 | grep "Combined:" | awk '{print $2}')
    MAX_QUEUES=$(ethtool -l $IFACE 2>/dev/null | grep "Pre-set" -A 3 | grep "Combined:" | awk '{print $2}')
    
    if [ -n "$CURRENT_QUEUES" ]; then
        echo "   Current queues: $CURRENT_QUEUES"
        echo "   Maximum queues: $MAX_QUEUES"
        
        if [ "$CURRENT_QUEUES" != "$MAX_QUEUES" ]; then
            echo "   ⚠️  Not using all queues! Run: sudo ethtool -L $IFACE combined $MAX_QUEUES"
        else
            echo "   ✅ Using all available queues"
        fi
    fi
else
    echo "   ⚠️  ethtool not installed: sudo apt install ethtool"
fi

echo ""
echo "4. Queue Statistics:"
if command -v ethtool &> /dev/null; then
    echo "   TX queue packet counts:"
    ethtool -S $IFACE 2>/dev/null | grep "tx_queue_[0-7]_packets" | head -8
    echo ""
    echo "   RX queue packet counts:"
    ethtool -S $IFACE 2>/dev/null | grep "rx_queue_[0-7]_packets" | head -8
    
    # Count how many TX queues have packets
    ACTIVE_TX=$(ethtool -S $IFACE 2>/dev/null | grep "tx_queue_[0-7]_packets" | awk '{print $2}' | awk '$1 > 0' | wc -l)
    echo ""
    echo "   Active TX queues: $ACTIVE_TX / 8"
    if [ $ACTIVE_TX -lt 4 ]; then
        echo "   ⚠️  Only $ACTIVE_TX TX queues active! This is the bottleneck!"
    fi
fi

echo ""
echo "5. XPS (Transmit Packet Steering) Configuration:"
for i in {0..7}; do
    if [ -f /sys/class/net/$IFACE/queues/tx-$i/xps_cpus ]; then
        XPS=$(cat /sys/class/net/$IFACE/queues/tx-$i/xps_cpus)
        echo "   tx-$i: $XPS"
    fi
done

echo ""
echo "6. CPU Usage (run during iperf test):"
echo "   Run this while iperf is active:"
echo "   mpstat -P ALL 1 5"
echo "   → Look for: Are all CPUs being used? Or just 1-2 at 100%?"

echo ""
echo "7. Network Offloads:"
if command -v ethtool &> /dev/null; then
    ethtool -k $IFACE 2>/dev/null | grep -E "tcp-segmentation-offload|generic-segmentation-offload|generic-receive-offload"
fi

echo ""
echo "8. TCP Settings:"
echo "   TCP congestion control: $(sysctl -n net.ipv4.tcp_congestion_control)"
echo "   TCP rmem: $(sysctl -n net.ipv4.tcp_rmem)"
echo "   TCP wmem: $(sysctl -n net.ipv4.tcp_wmem)"

echo ""
echo "=== Quick Fixes ==="
echo ""
echo "# If vCPUs < 8: Relaunch VM with --cpus boot=8"
echo ""
echo "# If queues < 8: Enable all queues"
echo "sudo ethtool -L $IFACE combined 8"
echo ""
echo "# Setup XPS for proper queue distribution:"
echo "for i in {0..7}; do printf '%x' \$((1 << \$i)) | sudo tee /sys/class/net/$IFACE/queues/tx-\$i/xps_cpus; done"
echo ""
echo "# Enable offloads:"
echo "sudo ethtool -K $IFACE tso on gso on gro on"
echo ""
echo "# During test, monitor CPU:"
echo "mpstat -P ALL 1"
echo "# Look for single core at 100% (bottleneck) vs all cores moderate (good)"

echo ""
echo "=== Diagnosis Complete ==="

