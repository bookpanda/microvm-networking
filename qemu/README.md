# QEMU
- boots like a normal OS from disk
- image contains everything (kernel, OS, etc.)
- full device emulation (microVMs only do virtio)
## Setup
```bash
sudo apt update
sudo apt install -y qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virt-manager cloud-image-utils

# check
sudo kvm-ok
qemu-system-x86_64 -accel help

wget --progress=bar:force https://cloud-images.ubuntu.com/jammy/current/jammy-server-cloudimg-amd64.img -O /tmp/ubuntu.img

cloud-localds /tmp/my-seed.img user-data

ps aux | grep qemu | grep -v grep | awk '{print $2}' | xargs kill -9

```
## Run
```bash
# 2GB RAM, 2 vCPUs (smp), ubuntu image, simple NAT networking, forwards host port 5201 to VM port 5201
qemu-system-x86_64 \
  -enable-kvm \
  -m 2048 \
  -smp 2 \
  -cpu host \
  -hda /tmp/ubuntu.img \
  -cdrom /tmp/my-seed.img \
  -boot c \
  -nographic \
  -netdev user,id=net0,hostfwd=tcp::5201-:5201 \
  -device virtio-net-pci,netdev=net0


```