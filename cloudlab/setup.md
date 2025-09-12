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
sudo apt install -y build-essential libseccomp-dev pkg-config libssl-dev curl acl sockperf

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
```bash
# hello kernel, rootfs (can barely do anything)
curl -fsSL -o /tmp/hello-vmlinux.bin https://s3.amazonaws.com/spec.ccfc.min/img/hello/kernel/hello-vmlinux.bin

curl -fsSL -o /tmp/hello-rootfs.ext4 https://s3.amazonaws.com/spec.ccfc.min/img/hello/fsfiles/hello-rootfs.ext4

# alpine kernel, rootfs (for testing)
wget https://dl-cdn.alpinelinux.org/alpine/v3.22/releases/x86_64/alpine-minirootfs-3.22.1-x86_64.tar.gz
dd if=/dev/zero of=rootfs.ext4 bs=1M count=128
mkfs.ext4 rootfs.ext4

mkdir mnt
sudo mount -o loop rootfs.ext4 mnt
sudo tar -xzf alpine-minirootfs-3.22.1-x86_64.tar.gz -C mnt
sudo umount mnt

wget https://dl-cdn.alpinelinux.org/alpine/edge/main/x86/linux-virt-6.12.46-r0.apk
mkdir linux-virt
tar -xzf linux-virt-6.12.46-r0.apk -C linux-virt
```