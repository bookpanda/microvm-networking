#!/usr/bin/env bash
# Optimize OVS-DPDK performance for maximum throughput

echo "=== OVS-DPDK Performance Optimization ==="
echo ""

# Check current PMD configuration
echo "1. Current PMD configuration:"
PMD_MASK=$(sudo ovs-vsctl get Open_vSwitch . other_config:pmd-cpu-mask | tr -d '"')
echo "   PMD CPU mask: $PMD_MASK"
NUM_PMDS=$(echo "obase=2; $((PMD_MASK))" | bc | tr -cd '1' | wc -c)
echo "   Number of PMD cores: $NUM_PMDS"

echo ""
echo "2. Queue distribution:"
sudo ovs-appctl dpif-netdev/pmd-rxq-show | grep -E "pmd thread|dpdk0|vhost-user1" | head -20

echo ""
echo "3. Current PMD utilization:"
sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -E "pmd thread|processing cycles" | head -12

echo ""
echo "=== Optimization Recommendations ==="
echo ""

# Check if we need more PMD cores
MAX_UTIL=$(sudo ovs-appctl dpif-netdev/pmd-stats-show | grep "processing cycles" | \
    awk '{print $4}' | tr -d '(%' | sort -n | tail -1)

if (( $(echo "$MAX_UTIL > 50" | bc -l) )); then
    echo "⚠️  PMD thread utilization > 50% - Consider adding more PMD cores"
    echo "   Current: $PMD_MASK ($NUM_PMDS cores)"
    echo "   Suggested: 0xFF (8 cores) or 0x3FC (8 cores on cores 2-9)"
    echo "   Command: sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF"
elif (( $(echo "$MAX_UTIL > 30" | bc -l) )); then
    echo "✓ PMD utilization moderate ($MAX_UTIL%) - Performance good"
    echo "  If you want more throughput, try adding PMD cores or more parallel streams"
else
    echo "✓ PMD utilization low ($MAX_UTIL%) - Bottleneck is elsewhere"
    echo "  Suggestions:"
    echo "  - Use more vCPUs in VM (boot=8 instead of boot=4,max=8)"
    echo "  - Use more parallel iperf streams (-P 32 or -P 64)"
    echo "  - Enable all queues in VM guest (ethtool -L ens4 combined 8)"
fi

echo ""
echo "4. Quick optimization commands:"
echo ""
echo "# Add more PMD cores (if needed):"
echo "sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xFF"
echo "sudo ovs-appctl dpif-netdev/pmd-rxq-rebalance"
echo ""
echo "# In VM - enable all queues and optimizations:"
echo "sudo ethtool -L ens4 combined 8"
echo "sudo ethtool -K ens4 tso on gso on gro on"
echo "sudo sysctl -w net.ipv4.tcp_congestion_control=bbr"
echo ""
echo "# Test with more parallelism:"
echo "iperf3 -c 10.10.1.10 -t 30 -P 64"
echo ""

echo "=== To monitor performance in real-time ==="
echo "watch -n 1 'sudo ovs-appctl dpif-netdev/pmd-stats-show | grep -E \"pmd thread|processing cycles\" | head -12'"

