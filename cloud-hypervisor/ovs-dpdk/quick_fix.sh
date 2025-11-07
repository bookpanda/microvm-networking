#!/usr/bin/env bash
set -e

echo "🔧 Applying OVS-DPDK performance fixes..."

# Reduce PMD threads from 8 to 2 (cores 2-3)
echo "1. Reducing PMD threads: 8 → 2 (cores 2-3)"
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xC

# Move DPDK lcore to core 4
echo "2. Moving DPDK lcore to core 4"
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x10

# Reduce memory from 8GB to 2GB
echo "3. Reducing OVS memory: 8GB → 2GB"
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=2048

# Restart OVS
echo "4. Restarting OVS..."
sudo systemctl restart ovs-vswitchd
sleep 3

echo "✅ OVS optimized!"
echo ""
sudo ovs-vsctl get Open_vSwitch . other_config
echo ""

# Show hugepages
echo "Hugepages status:"
cat /proc/meminfo | grep -E "HugePages_Total|HugePages_Free"
echo ""

echo "⚠️  NOW PIN YOUR VMs TO SPECIFIC CORES:"
echo ""
echo "After starting VMs, run:"
echo "  VM1_PID=\$(pgrep -f 'vhost-user1')"
echo "  VM2_PID=\$(pgrep -f 'vhost-user2')"  
echo "  sudo taskset -acp 5-8 \$VM1_PID"
echo "  sudo taskset -acp 9-12 \$VM2_PID"
echo ""
echo "Then re-run iperf3. You should see 15-25+ Gbps!"

