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
    # Try different stat formats (virtio_net uses different names)
    echo "   All interface statistics:"
    ethtool -S $IFACE 2>/dev/null | head -20
    echo "   ..."
    echo ""
    echo "   Note: virtio_net may not expose per-queue stats via ethtool"
    echo "   Use 'cat /proc/interrupts | grep virtio' to see queue activity"
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
echo "7. Network Offloads (CRITICAL FOR PERFORMANCE):"
if command -v ethtool &> /dev/null; then
    TSO=$(ethtool -k $IFACE 2>/dev/null | grep "tcp-segmentation-offload:" | awk '{print $2}')
    GSO=$(ethtool -k $IFACE 2>/dev/null | grep "generic-segmentation-offload:" | awk '{print $2}')
    GRO=$(ethtool -k $IFACE 2>/dev/null | grep "generic-receive-offload:" | awk '{print $2}')
    
    echo "   TSO (tcp-segmentation-offload): $TSO"
    echo "   GSO (generic-segmentation-offload): $GSO"
    echo "   GRO (generic-receive-offload): $GRO"
    
    if [ "$TSO" = "off" ]; then
        echo ""
        echo "   ❌ CRITICAL: TSO is OFF! This severely limits throughput!"
        echo "   → Run: sudo ethtool -K $IFACE tso on"
    else
        echo "   ✅ TSO is enabled"
    fi
fi

echo ""
echo "8. TCP Settings:"
echo "   TCP congestion control: $(sysctl -n net.ipv4.tcp_congestion_control)"
echo "   TCP rmem: $(sysctl -n net.ipv4.tcp_rmem)"
echo "   TCP wmem: $(sysctl -n net.ipv4.tcp_wmem)"

echo ""
echo "=== Quick Fixes (IN ORDER OF IMPORTANCE) ==="
echo ""
echo "# 1. CRITICAL: Enable TSO (tcp-segmentation-offload)"
echo "sudo ethtool -K $IFACE tso on"
echo "# Without TSO, CPU must segment every packet - huge overhead!"
echo ""
echo "# 2. Enable other offloads:"
echo "sudo ethtool -K $IFACE gso on gro on"
echo ""
echo "# 3. Verify queues are enabled:"
echo "ethtool -l $IFACE  # Should show Combined: 8"
echo ""
echo "# 4. Check XPS is configured (should already be done by cloud-init):"
echo "cat /sys/class/net/$IFACE/queues/tx-*/xps_cpus"
echo ""
echo "# 5. During test, monitor to confirm parallel usage:"
echo "# Watch CPU usage:"
echo "mpstat -P ALL 1"
echo "# Watch interrupts across queues:"
echo "watch -n 1 'cat /proc/interrupts | grep virtio'"
echo ""
echo "# 6. Try BBR congestion control (optional):"
echo "sudo sysctl -w net.ipv4.tcp_congestion_control=bbr"

echo ""
echo "=== Diagnosis Complete ==="

