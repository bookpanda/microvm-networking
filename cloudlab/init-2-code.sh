#!/bin/bash

# run this on remote machine
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

# go
wget https://go.dev/dl/go1.23.7.linux-amd64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.7.linux-amd64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc
go version

# linux kernel
curl -fsSL -o /tmp/vmlinux-5.10.223-no-acpi http://spec.ccfc.min.s3.amazonaws.com/firecracker-ci/v1.10/x86_64/vmlinux-5.10.223-no-acpi
# rootfs
curl -fsSL -o /tmp/debian-rootfs.ext4 https://cloudlab-microvm.s3.ap-southeast-1.amazonaws.com/debian-rootfs.ext4

# code
mkdir -p ~/code
cd ~/code
git clone git@github.com:bookpanda/microvm-networking.git
git clone git@github.com:bookpanda/firecracker-runner-node.git