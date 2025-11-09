# Setup
- [docs](https://doc.dpdk.org/guides-19.11/linux_gsg/index.html)
```bash
# meson is new Makefile/CMake
# ninja is new make
sudo apt install meson-1.5 

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