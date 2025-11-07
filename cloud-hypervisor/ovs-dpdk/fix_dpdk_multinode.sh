#!/usr/bin/env bash
set -e

echo "🔧 Fixing OVS-DPDK multinode configuration..."

# 1. Match queue counts: 2 queues everywhere
echo "1. Reconfiguring dpdk0 with 2 RX/TX queues..."
sudo ovs-vsctl set Interface dpdk0 options:n_rxq=2

echo "2. Reconfiguring vhost-user1 with 2 queues..."
sudo ovs-vsctl set Interface vhost-user1 options:n_rxq=2

# 3. Reduce PMD threads to 2 (cores 2-3)
echo "3. Reducing PMD threads: 8 → 2 (cores 2-3)..."
sudo ovs-vsctl set Open_vSwitch . other_config:pmd-cpu-mask=0xC

# 4. Set DPDK lcore to core 4
echo "4. Setting DPDK lcore to core 4..."
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-lcore-mask=0x10

# 5. Reduce memory
echo "5. Reducing OVS memory: 8GB → 2GB..."
sudo ovs-vsctl set Open_vSwitch . other_config:dpdk-socket-mem=2048

# 6. Restart OVS
echo "6. Restarting OVS..."
sudo systemctl restart ovs-vswitchd
sleep 3

echo "✅ Configuration updated!"
echo ""
sudo ovs-vsctl get Open_vSwitch . other_config
echo ""
sudo ovs-appctl dpif/show
echo ""
echo "⚠️  NOW RESTART YOUR VMs WITH:"
echo "   --net mac=52:54:00:02:d9:01,vhost_user=true,socket=/tmp/vhost-user1,num_queues=2,vhost_mode=server"
echo ""
echo "Expected throughput: 8-12 Gbps"

