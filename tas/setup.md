# Setup
```bash
# dpdk
cd ~
wget https://fast.dpdk.org/rel/dpdk-19.11.14.tar.xz
tar xf dpdk-19.11.14.tar.xz
mv dpdk-stable-19.11.14 dpdk-inst

cd ~/dpdk-inst
make config T=x86_64-native-linuxapp-gcc
make -j

# tas
cd ~/code/tas
make clean
make RTE_SDK=~/dpdk-inst/build

```