# Setup
- [docs](https://doc.dpdk.org/guides-19.11/linux_gsg/index.html)
```bash
# meson is new Makefile/CMake
# ninja is new make
sudo apt install meson-1.5 build-essential libnuma-dev ninja-build nasm

# remove intel-ipsec-mb that is too new
sudo apt remove -y libipsec-mb-dev libipsec-mb1
cd ~ && git clone https://github.com/intel/intel-ipsec-mb.git
cd ~/intel-ipsec-mb && git checkout v0.54
sudo make -j$(nproc) && sudo make install

sudo ln -s /usr/lib/libIPSec_MB.so.0.54.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0.54.0
sudo ln -s /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0.54.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0
sudo ln -s /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so

# dpdk
cd ~
wget https://fast.dpdk.org/rel/dpdk-19.11.14.tar.xz
tar xf dpdk-19.11.14.tar.xz
mv dpdk-stable-19.11.14 dpdk-inst

cd ~/dpdk-inst
meson build

cd ~/dpdk-inst/build
ninja
ninja install
ldconfig
# make config T=x86_64-native-linuxapp-gcc
# make -j$(nproc)

# tas
cd ~/code/tas
# replace pthread_yield() with sched_yield()
make clean
make RTE_SDK=~/dpdk-inst/build

```