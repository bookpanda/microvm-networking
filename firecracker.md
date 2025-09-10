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
rm -rf firecracker-v1.13.1-aarch64.tgz
rm -rf release-v1.13.1-aarch64
```