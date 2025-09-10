```bash
cargo clean
cargo build --release --no-default-features
./target/release/firecracker --no-kvm -n --config-file path/to/config.json

```