#! /bin/bash

if ! mountpoint -q ~/mnt; then
    sudo mount -o loop,rw /tmp/debian-rootfs.ext4 ~/mnt
    echo "Mounted rootfs"
else
    echo "Rootfs already mounted"
fi

sudo chroot ~/mnt /bin/bash -c "
rm -rf /root/firecracker-vsock
"

sudo cp -r ~/code/firecracker-vsock ~/mnt/root/firecracker-vsock

sudo chroot ~/mnt /bin/bash -c "
cd /root/firecracker-vsock &&
cmake . &&
cmake --build . &&
cp /root/firecracker-vsock/build/server /root/server &&
ls -l /root/
"

sudo umount ~/mnt
echo "Unmounted rootfs"