# RootFS
```bash
sudo apt update
sudo apt install debootstrap

mkdir ~/debian-bullseye-rootfs
# checl the arch
sudo debootstrap --arch=amd64 bullseye ~/debian-bullseye-rootfs http://deb.debian.org/debian/
sudo chroot ~/debian-bullseye-rootfs /bin/bash

apt update
apt install build-essential cmake git sudo
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