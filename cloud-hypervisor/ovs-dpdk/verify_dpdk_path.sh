#!/usr/bin/env bash
# Verify that VM-to-VM traffic is using DPDK fast path

echo "=== Verifying DPDK Fast Path ==="
echo ""

# Get initial packet counts
echo "1. Initial packet counts:"
DPDK0_RX_BEFORE=$(sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "rx_packets=" | cut -d= -f2)
DPDK0_TX_BEFORE=$(sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "tx_packets=" | cut -d= -f2)
echo "   dpdk0: RX=$DPDK0_RX_BEFORE, TX=$DPDK0_TX_BEFORE"

echo ""
echo "2. MAC learning table:"
sudo ovs-appctl fdb/show ovsbr0

echo ""
echo "3. Port mapping:"
sudo ovs-ofctl show ovsbr0 | grep -E "^\s+[0-9]+\(|LOCAL"

echo ""
echo "4. Waiting 5 seconds for traffic..."
echo "   (Run iperf or ping between VMs now)"
sleep 5

# Get final packet counts
DPDK0_RX_AFTER=$(sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "rx_packets=" | cut -d= -f2)
DPDK0_TX_AFTER=$(sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "tx_packets=" | cut -d= -f2)

DPDK0_RX_DIFF=$((DPDK0_RX_AFTER - DPDK0_RX_BEFORE))
DPDK0_TX_DIFF=$((DPDK0_TX_AFTER - DPDK0_TX_BEFORE))

echo ""
echo "5. Packet count changes:"
echo "   dpdk0: RX +$DPDK0_RX_DIFF, TX +$DPDK0_TX_DIFF"

echo ""
if [ $DPDK0_RX_DIFF -gt 100 ] || [ $DPDK0_TX_DIFF -gt 100 ]; then
    echo "✅ SUCCESS: Traffic is flowing through dpdk0 (DPDK fast path)!"
    echo "   This means cross-host VM-to-VM traffic is using DPDK userspace forwarding."
else
    echo "⚠️  WARNING: Little/no traffic through dpdk0"
    echo "   Possible causes:"
    echo "   - Testing VM-to-VM on same host (expected, only uses vhost-user)"
    echo "   - No active traffic during test window"
    echo "   - Traffic still going through kernel (check routing)"
fi

echo ""
echo "6. PMD thread utilization:"
sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -E "pmd thread|processing cycles" | head -12

echo ""
echo "=== Verification Complete ==="

