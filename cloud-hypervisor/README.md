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

sudo cloud-hypervisor \
	--kernel /tmp/vmlinux-5.10.223-no-acpi \
	--disk path=/tmp/focal-server-cloudimg-amd64.raw,readonly=on path=/tmp/ubuntu-cloudinit.img \
	--cmdline "console=hvc0 root=/dev/vda1 rw" \
	--cpus boot=4 \
	--memory size=1024M \
	--net "tap=,mac=,ip=,mask="

ps aux | grep cloud-hypervisor | grep -v grep | awk '{print $2}' | xargs kill -9


```
