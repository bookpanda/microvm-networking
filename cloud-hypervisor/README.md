# Cloud Hypervisor
```bash
wget https://cloud-images.ubuntu.com/focal/current/focal-server-cloudimg-amd64.img
qemu-img convert -p -f qcow2 -O raw focal-server-cloudimg-amd64.img focal-server-cloudimg-amd64.raw
mv focal-server-cloudimg-amd64.raw /tmp/focal-server-cloudimg-amd64.raw

# set admin capabilities so no need to sudo
sudo setcap cap_net_admin+ep /usr/bin/cloud-hypervisor

cloud-hypervisor \
   --cpus boot=2 \
   --memory size=512M \
   --kernel vmlinux \
   --cmdline "console=ttyS0 console=hvc0 root=/dev/vda1 rw" \
   --disk path=focal-server-cloudimg-amd64.raw   \
   --net mac=52:54:00:02:d9:01

```
