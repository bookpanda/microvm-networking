#!/bin/bash

# run this on remote machine
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
echo 'export PATH=$PATH:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin' >> ~/.bashrc

chmod 600 ~/.ssh/github
chmod 600 ~/.ssh/github.pub

NAME="$1"
EMAIL="$2"

if [ -z "$NAME" ] || [ -z "$EMAIL" ]; then
  echo "Usage: $0 \"Full Name\" email@example.com"
  exit 1
fi

git config --global user.name "$NAME"
git config --global user.email "$EMAIL"
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/github
ssh -T git@github.com

sudo chmod 666 /dev/kvm

# go
wget https://go.dev/dl/go1.23.7.linux-amd64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.7.linux-amd64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
export PATH=$PATH:/usr/local/go/bin # make it work right now
go version
rm go1.23.7.linux-amd64.tar.gz

# linux kernel
echo "Downloading linux kernel..."
curl -fL --progress-bar -o /tmp/vmlinux-5.10.223-no-acpi http://spec.ccfc.min.s3.amazonaws.com/firecracker-ci/v1.10/x86_64/vmlinux-5.10.223-no-acpi
# rootfs
echo "Downloading rootfs..."
curl -fL --progress-bar -o /tmp/debian-rootfs.ext4 https://cloudlab-microvm.s3.ap-southeast-1.amazonaws.com/debian-rootfs.ext4

# cloud hypervisor
sudo curl -fL --progress-bar -o /usr/bin/cloud-hypervisor https://github.com/cloud-hypervisor/cloud-hypervisor/releases/download/v48.0/cloud-hypervisor-static
sudo chmod +x /usr/bin/cloud-hypervisor

# code
mkdir -p ~/code
mkdir -p ~/mnt
cd ~/code
git clone git@github.com:bookpanda/microvm-networking.git
git clone git@github.com:bookpanda/firecracker-runner-node.git runner-node
git clone git@github.com:bookpanda/firecracker-vsock.git vsock
git clone git@github.com:bookpanda/userspace-stack.git
git clone git@github.com:bookpanda/virtio.git
git clone git@github.com:bookpanda/cloud-hypervisor.git
git clone git@github.com:fahren-stack/fahren.git
git clone git@github.com:fahren-stack/tas.git
git clone git@github.com:fahren-stack/firecracker-virtio-net.git

cp ~/code/microvm-networking/cloudlab/config ~/.ssh/config

# vscode extensions
code --install-extension anysphere.cpptools
code --install-extension anysphere.cursorpyright
code --install-extension llvm-vs-code-extensions.vscode-clangd
code --install-extension mesonbuild.mesonbuild