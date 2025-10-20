# Shared Memory
- lives in `/dev/shm`
```bash
fallocate -l 4K /dev/shm/vhost_ring

# truncate to 0 size
truncate -s 0 /dev/shm/vhost_ring
```

## How to make host shm accessible to guest
1. shared FS: mount `virtio-fs` to mount host dir to VM
> VM sees host memory as files inside a mounted directory (FS layer overhead)
- e.g. mount host `/dev/shm` (or another shared dir) into `/mnt/shm` inside the VM
- VM: mmap("/mnt/shm/vhost_ring")
> Memory is registered directly with the host and exposed to the VM via virtqueues. 
> VM maps the exact same physical pages that the host uses
2. vhost-user / virtio device (high performance)
- Host registers the shared memory via vhost-user backend
- VM maps it via the virtio device (e.g., /dev/vhost-user0)
- This allows zero-copy, very low-latency access
