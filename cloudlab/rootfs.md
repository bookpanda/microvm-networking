# RootFS
```bash
sudo apt update
sudo apt install debootstrap

mkdir ~/minbase-bullseye-rootfs
# checl the arch
sudo debootstrap --arch=amd64 --variant=minbase bullseye ~/minbase-bullseye-rootfs http://deb.debian.org/debian/
sudo chroot ~/minbase-bullseye-rootfs /bin/bash

apt update
apt install -y build-essential cmake git sudo autoconf libtool iperf3 sockperf
g++ --version    # should be g++ 10+
cmake --version
exit

cd ~
fallocate -l 4G debian-bullseye-rootfs.ext4
mkfs.ext4 debian-bullseye-rootfs.ext4
sudo mount -o loop debian-bullseye-rootfs.ext4 ~/mnt
sudo cp -a ~/debian-bullseye-rootfs/. ~/mnt/
sudo umount ~/mnt

```
### RootFS note
- we make it read-only because we want to use same rootFS for all VMs (don't want to copy n times)
- for iperf3 (requires writing files), we need to mount read-write blocks per VM
