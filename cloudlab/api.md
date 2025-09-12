## API
### Note
- firectl forces isolation (cannot talk VM-VM)
- firecracker API is 1 per VM (need to create new API for each VM)
```bash
# rust lib: fctools

sudo rm -f /tmp/firecracker.sock

./firecracker --api-sock /tmp/firecracker.sock

ls -l /tmp/firecracker.sock
ps aux | grep firecracker


curl --unix-socket /tmp/firecracker.sock -i \
     -X PUT "http://localhost/machine-config" \
     -H "Accept: application/json" \
     -H "Content-Type: application/json" \
     -d '{
           "vcpu_count": 1,
           "mem_size_mib": 256,
           "track_dirty_pages": false
         }'

curl --unix-socket /tmp/firecracker.sock -i \
    -X PUT "http://localhost/boot-source" \
    -H "accept: application/json" \
    -H "Content-Type: application/json" \
    -d '{
        "kernel_image_path": "/tmp/vmlinux-5.10.223-no-acpi",
        "boot_args": "console=ttyS0 reboot=k panic=1 pci=off"
    }'

curl --unix-socket /tmp/firecracker.sock -i \
    -X PUT "http://localhost/drives/rootfs" \
    -H "accept: application/json" \
    -H "Content-Type: application/json" \
    -d '{
        "drive_id": "rootfs",
        "path_on_host": "/tmp/debian-rootfs.ext4",
        "is_root_device": true,
        "is_read_only": false
    }'

# curl --unix-socket /tmp/firecracker.sock -i \
#   -X PUT "http://localhost/serial" \
#   -H "accept: application/json" \
#   -H "Content-Type: application/json" \
#   -d '{
#         "output_path": "/tmp/vm_console.log",
#       }'

curl --unix-socket /tmp/firecracker.sock -i \
    -X PUT "http://localhost/actions" \
    -H  "accept: application/json" \
    -H  "Content-Type: application/json" \
    -d '{
        "action_type": "InstanceStart"
     }'

curl --unix-socket /tmp/firecracker.sock -i \
    -X PUT "http://localhost/actions" \
    -H "accept: application/json" \
    -H "Content-Type: application/json" \
    -d '{
        "action_type": "SendCtrlAltDel"
     }'
```