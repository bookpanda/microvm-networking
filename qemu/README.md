# QEMU
- boots like a normal OS from disk
- image contains everything (kernel, OS, etc.)
- full device emulation (microVMs only do virtio)
## Setup
```bash
sudo apt update
sudo apt install -y qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virt-manager

# check
sudo kvm-ok
qemu-system-x86_64 -accel help

wget --progress=bar:force https://cloud-images.ubuntu.com/jammy/current/jammy-server-cloudimg-amd64.img -O /tmp/ubuntu.img

```
## Run
```bash
# 1024MB RAM, ubuntu image, 2 vCPUs
qemu-system-x86_64 \
  -enable-kvm \
  -m 1024 \
  -hda ubuntu.img \
  -boot c \
  -cpu host \
  -smp 2

qemu-system-x86_64 \
  -enable-kvm \
  -m 512 \
  -smp 1 \
  -kernel vmlinuz \
  -initrd initrd.img \
  -append "root=/dev/ram console=ttyS0" \
  -nographic \
  -netdev user,id=net0,hostfwd=tcp::5201-:5201 \
  -device virtio-net-pci,netdev=net0
```