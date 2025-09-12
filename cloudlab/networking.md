# Multinode Networking
```bash
ifconfig
# look for inet ip in eno1d1
```
- A single host `write()` may correspond to many guest writes inside the VM.
- A guest `write()` may be split or coalesced by the VM before hitting the host.

### Why tracing inside the VM is unnecessary
- The guest kernel translates network syscalls into VM device operations.
- The host sees the actual system call cost, including virtualization overhead.
- Guest-level syscalls (e.g., iperf3 write() inside the VM) don’t add extra info for your goal, because you care about host-VM interaction speed, not guest process internals.