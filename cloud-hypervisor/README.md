# Cloud Hypervisor
```bash
# image
wget https://cloud-images.ubuntu.com/focal/current/focal-server-cloudimg-amd64.img
qemu-img convert -p -f qcow2 -O raw focal-server-cloudimg-amd64.img focal-server-cloudimg-amd64.raw
mv focal-server-cloudimg-amd64.raw /tmp/focal-server-cloudimg-amd64.raw

# init config in vm
./init/create-cloud-init.sh

# set admin capabilities
sudo setcap cap_net_admin+ep /usr/bin/cloud-hypervisor

sudo ip link add name br0 type bridge
sudo ip link set br0 up
sudo ip addr add 192.168.249.1/24 dev br0

# sudo ip tuntap add dev tap0 mode tap user $USER
# sudo ip link set tap0 up

# sudo ip link delete tap0

# tap will be removed after vm is stopped
sudo cloud-hypervisor \
	--kernel /tmp/vmlinux-5.10.223-no-acpi \
	--disk path=/tmp/focal-server-cloudimg-amd64.raw,readonly=on path=/tmp/ubuntu-cloudinit.img \
	--cmdline "console=hvc0 root=/dev/vda1 rw" \
	--cpus boot=4 \
	--memory size=1024M \
	--net "tap=tap0,mac=12:34:56:78:90:ab,ip=192.168.249.2,mask=255.255.255.0"

sudo ip link set tap0 master br0

ssh cloud@192.168.249.2

ps aux | grep cloud-hypervisor | grep -v grep | awk '{print $2}' | xargs kill -9


```
