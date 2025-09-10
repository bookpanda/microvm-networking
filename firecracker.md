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