# Cloud Hypervisor
## Setup
```bash
sudo apt update
sudo apt install -y flex bison libelf-dev mtools libguestfs-tools
# kernel
git clone --depth 1 https://github.com/cloud-hypervisor/linux.git -b ch-6.12.8 linux-cloud-hypervisor
pushd linux-cloud-hypervisor
make ch_defconfig
KCFLAGS="-Wa,-mx86-used-note=no" make bzImage -j `nproc`
# make -j `nproc`
popd
mv ~/linux-cloud-hypervisor/arch/x86/boot/compressed/vmlinux.bin /tmp/vmlinux.bin

# image
wget https://cloud-images.ubuntu.com/focal/current/focal-server-cloudimg-amd64.img
qemu-img convert -p -f qcow2 -O raw focal-server-cloudimg-amd64.img focal-server-cloudimg-amd64.raw
mv focal-server-cloudimg-amd64.raw /tmp/focal-server-cloudimg-amd64.raw

wget https://cloud-images.ubuntu.com/noble/current/noble-server-cloudimg-amd64.img
qemu-img convert -p -f qcow2 -O raw noble-server-cloudimg-amd64.img noble-server-cloudimg-amd64.raw
mv noble-server-cloudimg-amd64.raw /tmp/noble-server-cloudimg-amd64.raw

# init config in vm
./init/create-cloud-init.sh

# set admin capabilities
sudo setcap cap_net_admin+ep /usr/bin/cloud-hypervisor
```

## Running
```bash
sudo ip link add name br0 type bridge
sudo ip link set br0 up
sudo ip addr add 192.168.249.1/24 dev br0

sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip link set tap0 master br0
sudo ip link set tap0 up

# Enable NAT for internet access (optional)
sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -s 192.168.100.0/24 -j MASQUERADE
sudo iptables -A FORWARD -i br0 -o $(ip route | grep default | awk '{print $5}') -j ACCEPT
sudo iptables -A FORWARD -i $(ip route | grep default | awk '{print $5}') -o br0 -m state --state RELATED,ESTABLISHED -j ACCEPT

# Clean up
sudo ip link delete tap0 2>/dev/null
sudo ip link delete br0 2>/dev/null
sudo iptables -t nat -D POSTROUTING -s 192.168.249.0/24 -j MASQUERADE 2>/dev/null
sudo iptables -D FORWARD -i br0 -j ACCEPT 2>/dev/null

# to prevent "A start job is running for Wait for Network to be Configured", make sure the tap0 match the --net config BEFORE running the vm and --cmdline has mask=systemd-networkd-wait-online.service
# set MAC to match the network-config
sudo cloud-hypervisor \
	--kernel /tmp/vmlinux.bin \
	--disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/ubuntu-cloudinit.img \
	--cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service systemd.mask=snapd.service systemd.mask=snapd.seeded.service systemd.mask=snapd.socket" \
	--cpus boot=2 \
	--memory size=512M \
	--net "tap=tap0,mac=12:34:56:78:90:ab" 

# login: cloud/cloud123
ssh cloud@10.10.1.10
sshpass -p "cloud123" ssh cloud@10.10.1.10

ps aux | grep cloud-hypervisor | grep -v grep | awk '{print $2}' | xargs kill -9

# To force cloud-init aka update config of vm in /init folder to rerun (choose one):
# 1. Change instance-id in init/meta-data and regenerate cloud-init.img (EASIEST)
# 2. Clean cloud-init state from disk (AFTER killing VM):
./init/clean-disk-state.sh

```
