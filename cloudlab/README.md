# Setup baremetal
```bash
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/github
ssh -T git@github.com

git config --global user.name "Your Name"
git config --global user.email "you@example.com"

# c
sudo apt update
sudo apt install build-essential clangd
sudo apt install -y cmake
sudo apt install -y libseccomp-dev build-essential
sudo apt install -y cmake clang pkg-config libssl-dev

# go
wget https://go.dev/dl/go1.23.7.linux-arm64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.7.linux-arm64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc
go version

# rust
sudo apt install cargo
sudo apt install -y curl build-essential
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
echo 'source $HOME/.cargo/env' >> ~/.bashrc
source ~/.bashrc
```
# Setup Firecracker

## Compile yourself
```bash
cargo clean
cargo build --release --no-default-features
./target/release/firecracker --no-kvm -n --config-file path/to/config.json
```

## Downloads
```bash
curl -LOJ https://github.com/firecracker-microvm/firecracker/releases/download/v1.13.1/firecracker-v1.13.1-aarch64.tgz
tar -xzf firecracker-v1.13.1-aarch64.tgz
mv ./release-v1.13.1-aarch64/firecracker-v1.13.1-aarch64 firecracker
chmod +x firecracker
sudo cp firecracker /usr/bin/
rm -rf firecracker-v1.13.1-aarch64.tgz
rm -rf release-v1.13.1-aarch64


curl -fsSL -o /tmp/hello-vmlinux.bin https://s3.amazonaws.com/spec.ccfc.min/img/hello/kernel/hello-vmlinux.bin

curl -fsSL -o /tmp/hello-rootfs.ext4 https://s3.amazonaws.com/spec.ccfc.min/img/hello/fsfiles/hello-rootfs.ext4

# install go first
git clone https://github.com/firecracker-microvm/firectl
cd firectl
make
sudo cp firectl /usr/bin/
firectl -h
```