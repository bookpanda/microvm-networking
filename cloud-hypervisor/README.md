# Cloud Hypervisor
## Setup
```bash
sudo apt update
sudo apt install -y flex bison libelf-dev mtools
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
sudo ip addr add 192.168.249.2/24 dev tap0

# Clean up
sudo ip link delete tap0 2>/dev/null
sudo ip link delete br0 2>/dev/null

# tap will be removed after vm is stopped
# to prevent "A start job is running for Wait for Network to be Configured", make sure the tap0 and its ip match the --net config BEFORE running the vm
sudo cloud-hypervisor \
	--kernel /tmp/vmlinux.bin \
	--disk path=/tmp/focal-server-cloudimg-amd64.raw path=/tmp/ubuntu-cloudinit.img \
	--cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw systemd.mask=systemd-networkd-wait-online.service" \
	--cpus boot=2 \
	--memory size=512M \
	--net "tap=tap0,mac=12:34:56:78:90:ab,ip=192.168.249.2,mask=255.255.255.0"

# login: cloud/cloud123
ssh cloud@192.168.249.2
sshpass -p "cloud123" ssh cloud@192.168.249.2

ps aux | grep cloud-hypervisor | grep -v grep | awk '{print $2}' | xargs kill -9


```
