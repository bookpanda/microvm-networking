# NIC
```bash
# eno33np0 / eno34np1 → likely a dual-port Intel NIC
# enp65s0f0np0 / enp65s0f1np1 → another dual-port NIC
# tap0 / br0 / virbr0 → virtual interfaces (not relevant for DPDK Rx queues)
sudo ethtool -l eno33np0
sudo ethtool -l eno34np1
sudo ethtool -l enp65s0f0np0
sudo ethtool -l enp65s0f1np1

# each port (33, 34, 65, 66) supports 32 Rx queues
# 1 PMD thread polls 1 Rx queue, and each PMD thread should have 1 dedicated CPU core.
```
## NUMA
- c6525-25g CPU: 16-core AMD 7302P at 3.00GHz
- The 7302P is single NUMA node → all cores on same NUMA node.
- 2 NICs × 2 ports = 4 ports, each 25 Gb → 100 Gb total traffic potential