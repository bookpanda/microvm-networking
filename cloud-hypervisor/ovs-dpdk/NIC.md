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
## DPDK ports
### type=dpdkvhostuserclient (for vNICs)
- Connects a VM’s vhost-user interface to OVS
- options: vhost-server-path, n_rxq

### type=dpdk (for physical NICs)
- Exposes a physical NIC bound to DPDK (VFIO/uio) to OVS
- options: dpdk-devargs

## NUMA
- c6525-25g CPU: 16-core AMD 7302P at 3.00GHz
- The 7302P is single NUMA node → all cores on same NUMA node.
- 2 NICs × 2 ports = 4 ports, each 25 Gb → 100 Gb total traffic potential

## VM specs
- 1 vCPU ≠ 1 physical core — can share cores unless pinned
- VM memory ≥ 64–128 MB per queue (depends on workload)
- vCPUs ≥ num_queues