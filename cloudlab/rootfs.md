# RootFS
```bash
# S3 url for pre-built rootfs
curl -fL --progress-bar -o /tmp/minbase-bullseye-rootfs.ext4 https://cloudlab-microvm.s3.ap-southeast-1.amazonaws.com/minbase-bullseye-rootfs.ext4

sudo apt update
sudo apt install debootstrap

mkdir ~/minbase-bullseye-rootfs
# check the arch
sudo debootstrap --arch=amd64 --variant=minbase --include openssh-server,nano bullseye ~/minbase-bullseye-rootfs http://deb.debian.org/debian/

sudo cp -r ~/code/microvm-userspace-stack ~/minbase-bullseye-rootfs/root/microvm-userspace-stack

sudo chroot ~/minbase-bullseye-rootfs /bin/bash

apt update
apt install -y build-essential cmake git sudo autoconf libtool iperf3 sockperf openssh-server
g++ --version    # should be g++ 10+
cmake --version

cd /root/microvm-userspace-stack
# git clone https://github.com/bookpanda/microvm-userspace-stack.git
cmake -S . -B build
cmake --build build
cp /root/microvm-userspace-stack/build/vm_app /root/vm_app

passwd
passwd root
sed -i 's/^#  PasswordAuthentication.*/PasswordAuthentication yes/' /etc/ssh/sshd_config
# sed -i 's/^ChallengeResponseAuthentication.*/ChallengeResponseAuthentication yes/' /etc/ssh/sshd_config
# sed -i 's/^#PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config

# systemctl enable ssh
/etc/init.d/ssh start

exit

cd ~

fallocate -l 1G minbase-bullseye-rootfs.ext4
truncate -s 1G minbase-bullseye-rootfs.ext4

mkfs.ext4 minbase-bullseye-rootfs.ext4
mkdir mnt
sudo mount -o loop minbase-bullseye-rootfs.ext4 ~/mnt
sudo cp -a ~/minbase-bullseye-rootfs/. ~/mnt/
sudo umount ~/mnt
```
## Editing
```bash
sudo mount -o loop,rw minbase-bullseye-rootfs.ext4 ~/mnt
sudo chroot ~/mnt /bin/bash

# sudo cp ~/code/microvm-networking/cloudlab/rootfs/ssh_config ~/mnt/etc/ssh/ssh_config
sudo cp ~/code/microvm-networking/cloudlab/rootfs/sshd_config ~/mnt/etc/ssh/sshd_config
# /usr/sbin/sshd -D
# sudo cp ~/code/microvm-networking/cloudlab/rootfs/common-session ~/mnt/etc/pam.d/common-session

# never mess with /etc/init.d/ssh 
# sudo cp ~/code/microvm-networking/cloudlab/rootfs/ssh ~/mnt/etc/init.d/ssh

sudo umount ~/mnt

# on mac
scp -i ~/.ssh/cloudlab ipankam@amd128.utah.cloudlab.us:/tmp/minbase-bullseye-rootfs.ext4 ./minbase-bullseye-rootfs.ext4
```
### RootFS note
- we make it read-only because we want to use same rootFS for all VMs (don't want to copy n times)
- for iperf3 (requires writing files), we need to mount read-write blocks per VM
