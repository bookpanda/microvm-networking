# Setup baremetal
```bash
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
# you can also put "config" in ~/.ssh/config to avoid typing the email every time
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/github
ssh -T git@github.com

git config --global user.name "Your Name"
git config --global user.email "you@example.com"

git clone git@github.com:bookpanda/microvm-networking.git

# core
sudo apt update
sudo apt install -y build-essential libseccomp-dev pkg-config libssl-dev curl acl sockperf isc-dhcp-server sshpass bpftrace linux-headers-$(uname -r) socat cloud-image-utils protobuf-compiler

# let you run Firecracker as your user without needing full sudo
sudo setfacl -m u:${USER}:rw /dev/kvm
sudo usermod -aG kvm $USER
sudo chmod 666 /dev/kvm

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
### Download kernel
- Firecracker repo: http://spec.ccfc.min.s3.amazonaws.com/

```bash
# linux kernel (for testing)
curl -fsSL -o /tmp/vmlinux-5.10.223-no-acpi http://spec.ccfc.min.s3.amazonaws.com/firecracker-ci/v1.10/x86_64/vmlinux-5.10.223-no-acpi
```
