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
 
# user: ubuntu, password: ubuntu
cloud-localds /tmp/my-seed.img user-data

ps aux | grep qemu | grep -v grep | awk '{print $2}' | xargs kill -9

```
## Run
```bash
# 2GB RAM, 2 vCPUs (smp), ubuntu image, simple NAT networking, 
# forwards host port 5201 to VM port 5201 (TCP only, UDP requires bridged or TAP networking)
# isock 5.2Gbits/s
sudo qemu-system-x86_64 \
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

# bridged (can do UDP now)
# isock 16.2Gbits/s
sudo qemu-system-x86_64 \
  -enable-kvm \
  -m 2048 \
  -smp 2 \
  -cpu host \
  -hda /tmp/ubuntu.img \
  -cdrom /tmp/my-seed.img \
  -boot c \
  -nographic \
  -netdev bridge,id=net0,br=br0 \
  -device virtio-net-pci,netdev=net0

# inside VM
sudo apt update
sudo apt install -y iperf3 sockperf
```