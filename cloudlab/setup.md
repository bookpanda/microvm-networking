# Setup baremetal
```bash
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/github
ssh -T git@github.com

git config --global user.name "Your Name"
git config --global user.email "you@example.com"

# core
sudo apt update
sudo apt install -y build-essential libseccomp-dev pkg-config libssl-dev curl acl sockperf isc-dhcp-server sshpass

# let you run Firecracker as your user without needing full sudo
sudo setfacl -m u:${USER}:rw /dev/kvm
sudo usermod -aG kvm $USER

# c
sudo apt install -y cmake clangd clang

# go
wget https://go.dev/dl/go1.23.7.linux-amd64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.7.linux-amd64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc
go version

# rust
sudo apt install -y cargo
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
echo 'source $HOME/.cargo/env' >> ~/.bashrc
source ~/.bashrc
sudo apt install -y rustup
rustup default stable
```

# Setup Firecracker
### Option 1: Compile yourself
```bash
cargo clean
cargo build --release --no-default-features
./target/release/firecracker --no-kvm -n --config-file path/to/config.json
```
### Option 2: Download pre-compiled binary
```bash
curl -LOJ https://github.com/firecracker-microvm/firecracker/releases/download/v1.13.1/firecracker-v1.13.1-x86_64.tgz
tar -xzf firecracker-v1.13.1-x86_64.tgz
mv ./release-v1.13.1-x86_64/firecracker-v1.13.1-x86_64 firecracker
chmod +x firecracker
sudo cp firecracker /usr/bin/
rm -rf firecracker-v1.13.1-x86_64.tgz
rm -rf release-v1.13.1-x86_64
```
### Firectl
```bash
# install go first
git clone https://github.com/firecracker-microvm/firectl
cd firectl
make
sudo cp firectl /usr/bin/
firectl -h
```
### Download rootfs, kernel
- Firecracker repo: http://spec.ccfc.min.s3.amazonaws.com/
- Alpine repo: https://dl-cdn.alpinelinux.org/alpine/

```bash
# linux kernel (for testing)
curl -fsSL -o /tmp/vmlinux-5.10.223-no-acpi http://spec.ccfc.min.s3.amazonaws.com/firecracker-ci/v1.10/x86_64/vmlinux-5.10.223-no-acpi

curl -fsSL -o /tmp/debian-rootfs.ext4 http://spec.ccfc.min.s3.amazonaws.com/ci-artifacts/disks/x86_64/debian.rootfs.ext4


# hello kernel, rootfs (can barely do anything)
curl -fsSL -o /tmp/hello-vmlinux.bin https://s3.amazonaws.com/spec.ccfc.min/img/hello/kernel/hello-vmlinux.bin

curl -fsSL -o /tmp/hello-rootfs.ext4 https://s3.amazonaws.com/spec.ccfc.min/img/hello/fsfiles/hello-rootfs.ext4


# alpine kernel, rootfs (doesn't work yet)
wget https://dl-cdn.alpinelinux.org/alpine/v3.22/releases/x86_64/alpine-minirootfs-3.22.1-x86_64.tar.gz
dd if=/dev/zero of=rootfs.ext4 bs=1M count=128
mkfs.ext4 rootfs.ext4

mkdir mnt
sudo mount -o loop rootfs.ext4 mnt
sudo tar -xzf alpine-minirootfs-3.22.1-x86_64.tar.gz -C mnt
sudo umount mnt
cp rootfs.ext4 /tmp/alpine-rootfs.ext4

wget https://dl-cdn.alpinelinux.org/alpine/edge/main/x86/linux-virt-6.12.46-r0.apk
mkdir linux-virt
tar -xzf linux-virt-6.12.46-r0.apk -C linux-virt
cp linux-virt/boot/vmlinuz-virt /tmp/alpine-vmlinux

# wget https://dl-cdn.alpinelinux.org/alpine/v3.22/releases/x86_64/alpine-virt-3.22.1-x86_64.iso
```

## Prepare rootfs
```bash
sudo mkdir /mnt/debian-rootfs
sudo mount -o loop /tmp/debian-rootfs.ext4 /mnt/debian-rootfs

sudo chroot /mnt/debian-rootfs /bin/bash

useradd -m -s /bin/bash vmuser
echo "vmuser:vm" | chpasswd
mkdir -p /home/vmuser/.ssh
chown -R vmuser:vmuser /home/vmuser/.ssh

# host: add ssh key
ssh-keygen -t ed25519 -C "your_email@example.com"
cat ~/.ssh/vm.pub | sudo tee /mnt/debian-rootfs/home/vmuser/.ssh/authorized_keys

# back to vm
chown vmuser:vmuser /home/vmuser/.ssh/authorized_keys
chmod 600 /home/vmuser/.ssh/authorized_keys

# in /etc/ssh/sshd_config
# PubkeyAuthentication yes

systemctl enable ssh
exit
sudo umount /mnt/debian-rootfs
```
### Downloading
```bash
sudo mount -o loop /tmp/debian-rootfs.ext4 /mnt/debian-rootfs

sudo mount --bind /proc /mnt/debian-rootfs/proc
sudo mount --bind /sys  /mnt/debian-rootfs/sys
sudo mount --bind /dev  /mnt/debian-rootfs/dev
sudo mount --bind /dev/pts /mnt/debian-rootfs/dev/pts

sudo cp /etc/resolv.conf /mnt/debian-rootfs/etc/resolv.conf

sudo chroot /mnt/debian-rootfs /bin/bash
ping -c 2 8.8.8.8   # tests raw IP connectivity
ping -c 2 google.com  # tests DNS

# add debian stretch repo
echo "deb http://archive.debian.org/debian stretch main contrib non-free
deb http://archive.debian.org/debian stretch-updates main contrib non-free
deb http://archive.debian.org/debian-security stretch/updates main contrib non-free" > /etc/apt/sources.list
echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid-until

export APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=1
apt-get -o Acquire::AllowInsecureRepositories=true \
        -o Acquire::AllowDowngradeToInsecureRepositories=true update

apt-get -o Acquire::AllowInsecureRepositories=true \
        -o Acquire::AllowDowngradeToInsecureRepositories=true install \
        gcc build-essential cmake git autoconf libtool iperf3

cd /home/vmuser
git clone https://github.com/Mellanox/sockperf.git
cd sockperf
./autogen.sh
./configure
make
cp sockperf /usr/bin/
cd ..
rm -rf sockperf

exit
sudo umount /mnt/debian-rootfs/proc
sudo umount /mnt/debian-rootfs/sys
sudo umount /mnt/debian-rootfs/dev/pts
sudo umount /mnt/debian-rootfs/dev
sudo umount /mnt/debian-rootfs

# fix ext4 filesystem
e2fsck -f /tmp/debian-rootfs.ext4
```
### RootFS note
- we make it read-only because we want to use same rootFS for all VMs (don't want to copy n times)
- for iperf3 (requires writing files), we need to mount read-write blocks per VM