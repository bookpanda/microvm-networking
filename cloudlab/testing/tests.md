# Throughput
```bash
# host
iperf3 -s

# vm: 30s, 4 threads
iperf3 -c 192.168.100.1 -t 30 -P 4
```

# Latency
## Setup
```bash
git clone https://github.com/Mellanox/sockperf.git
cd sockperf
./autogen.sh
./configure --prefix=/usr/local LDFLAGS="-static" CXXFLAGS="-static" CC=gcc CXX=g++
make -j$(nproc)
sudo make install

# increase FS size
truncate -s +2G /tmp/hello-rootfs.ext4
sudo e2fsck -f /tmp/hello-rootfs.ext4
sudo resize2fs /tmp/hello-rootfs.ext4

# mount sockperf to FS
sudo mkdir /mnt/vmroot
sudo mount -o loop /tmp/hello-rootfs.ext4 /mnt/vmroot
sudo cp ./sockperf /mnt/vmroot/usr/local/bin/
sudo chmod +x /mnt/vmroot/usr/local/bin/sockperf
sudo umount /mnt/vmroot
```
## Testing
```bash
# host
sockperf server -i 192.168.100.2
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30

# vm: 30s, 64-byte messages
sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```