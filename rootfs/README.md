## RootFS
- typically, 1 rootfs per function (the code is already in the rootfs, we only need to send inputs)
- no SSHing into VMs (it takes 30s) for experiments
- we make it read-only because we want to use same rootfs for all VMs (don't want to copy n times)
- for iperf3 (requires writing files), we need to mount read-write blocks per VM
```bash
# install docker
sudo apt update
sudo apt install -y ca-certificates curl gnupg lsb-release
# Add Docker’s official GPG key and repository
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```
```bash
sudo apt install -y rustup
rustup update stable
cargo install buildfs

echo 'export PATH="$HOME/.cargo/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
which buildfs

# generate rootfs
sudo -E ~/.cargo/bin/buildfs run -o debian-rootfs.ext4 ./build_script.toml
cp debian-rootfs.ext4 /tmp/debian-rootfs.ext4
```

## Mounting
```bash
sudo mount -o loop,rw debian-rootfs.ext4 ~/mnt
sudo cp -r ~/code/firecracker-vsock ~/mnt/root/firecracker-vsock

sudo chroot ~/mnt /bin/bash

cd /root/firecracker-vsock
cmake .
cmake --build .
cp /root/firecracker-vsock/build/server /root/server

sudo cp ~/code/microvm-networking/benchmark/user_data.sh ~/mnt/root/user_data.sh
chmod +x /root/user_data.sh
sudo cp ~/code/microvm-networking/rootfs/server.service ~/mnt/etc/systemd/system/server.service
sudo systemctl enable server.service

sudo umount ~/mnt

# check
sudo systemctl status server.service
```
