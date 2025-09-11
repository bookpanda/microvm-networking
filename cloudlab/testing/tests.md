# Throughput
```bash
# host
iperf3 -s

# vm: 30s, 4 threads
iperf3 -c 192.168.100.1 -t 30 -P 4
```

# Latency
## Setup in VM
```bash
wget https://github.com/Mellanox/sockperf/archive/refs/tags/3.10.tar.gz


```
## Testing
```bash
# host
sockperf server -i 192.168.100.2
sockperf ping-pong -i 192.168.100.2 -m 64 -t 30

# vm: 30s, 64-byte messages
sockperf ping-pong -i 192.168.100.1 -m 64 -t 30
```

## Latency old Setup
```bash
git clone https://github.com/Mellanox/sockperf.git
cd sockperf
./autogen.sh
# ./configure LDFLAGS="-static" CXXFLAGS="-static" CC=gcc CXX=g++
./configure --prefix=/usr/local \
  CC="gcc -static" \
  CXX="g++ -static -static-libstdc++ -static-libgcc" \
  LDFLAGS="-static"
make -j$(nproc)
sudo make install

# increase FS size
truncate -s +2G /tmp/hello-rootfs.ext4
sudo e2fsck -f /tmp/hello-rootfs.ext4
sudo resize2fs /tmp/hello-rootfs.ext4

# mount sockperf to FS
mkdir -p /tmp/sockperf-libs
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 /tmp/sockperf-libs/
cp /lib/x86_64-linux-gnu/libm.so.6 /tmp/sockperf-libs/
cp /lib/x86_64-linux-gnu/libgcc_s.so.1 /tmp/sockperf-libs/
cp /lib/x86_64-linux-gnu/libc.so.6 /tmp/sockperf-libs/
cp /lib64/ld-linux-x86-64.so.2 /tmp/sockperf-libs/

sudo mkdir /mnt/vmroot
sudo mount -o loop /tmp/hello-rootfs.ext4 /mnt/vmroot

sudo mkdir -p /mnt/vmroot/usr/local/lib
sudo cp /tmp/sockperf-libs/* /mnt/vmroot/usr/local/lib/

sudo cp ./sockperf /mnt/vmroot/usr/local/bin/
sudo chmod +x /mnt/vmroot/usr/local/bin/sockperf
sudo umount /mnt/vmroot
```