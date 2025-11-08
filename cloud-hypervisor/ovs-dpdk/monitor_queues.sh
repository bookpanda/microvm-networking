#!/bin/bash
# Monitor queue depths and identify bottlenecks

echo "=== Queue Monitoring Tool ==="
echo ""

echo "1. vhost-user1 Queue Usage (VM connection):"
echo "   RX queues (host receiving from VM):"
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep "rx_q[0-7]_good_packets" | sort -t= -k2 -n -r
echo ""
echo "   TX queues (host sending to VM) - watch for imbalance:"
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep "tx_q[0-7]_good_packets" | sort -t= -k2 -n -r
echo ""

echo "2. Drops and Errors (THIS IS KEY):"
echo "   vhost-user1 drops:"
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep -E "drop|retry|error" | grep -v "=0"
echo ""

echo "3. Guest Notifications (Host→VM wakeups):"
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep "guest_notifications" | grep -v "=0" | head -10
echo ""

echo "4. dpdk0 Queue Usage:"
echo "   RX queues (from physical network):"
sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "rx_q[0-7]_packets" | sort -t= -k2 -n -r
echo ""
echo "   TX queues (to physical network):"
sudo ovs-vsctl get Interface dpdk0 statistics | tr ',' '\n' | grep "tx_q[0-7]_packets" | sort -t= -k2 -n -r
echo ""

echo "5. PMD Thread Queue Assignment:"
sudo ovs-appctl dpif-netdev/pmd-rxq-show | grep -E "pmd thread|vhost-user1|dpdk0"
echo ""

echo "6. Inflight Packets (Stuck in buffers):"
sudo ovs-vsctl get Interface vhost-user1 statistics | tr ',' '\n' | grep "inflight"
echo ""

echo "=== How to Interpret ==="
echo ""
echo "Bottleneck Indicators:"
echo "  - ovs_tx_failure_drops > 0     : Host can't send to VM fast enough"
echo "  - ovs_tx_retries > 0           : Host had to retry sends"
echo "  - rx_q*_inflight > 0           : Packets stuck in buffers"
echo "  - Uneven tx_q* distribution    : Poor load balancing"
echo "  - tx_q5-7 = 0                  : VM only using 5 queues (not 8)"
echo ""
echo "Run this WHILE iperf is running to see live bottlenecks!"

