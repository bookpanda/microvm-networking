# Setup
- [docs](https://doc.dpdk.org/guides-19.11/linux_gsg/index.html)
```bash
# meson is new Makefile/CMake
# ninja is new make
sudo apt install meson-1.5 build-essential libnuma-dev ninja-build nasm

# intel-ipsec-mb downgrade (DPDK 19.11 needs v0.54, not v1.5+)
# Remove newer incompatible version
sudo apt remove -y libipsec-mb-dev libipsec-mb1

# Build and install v0.54 from source
cd ~ && git clone https://github.com/intel/intel-ipsec-mb.git
cd ~/intel-ipsec-mb && git checkout v0.54

# Build and install shared library
sudo make -j$(nproc) && sudo make install

# Build static library (needed for DPDK)
sudo make SHARED=n

# Install to locations where DPDK expects to find it
sudo cp ~/intel-ipsec-mb/libIPSec_MB.a /usr/lib/x86_64-linux-gnu/
sudo ln -s /usr/lib/libIPSec_MB.so.0.54.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0.54.0
sudo ln -s /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0.54.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0
sudo ln -s /usr/lib/x86_64-linux-gnu/libIPSec_MB.so.0 /usr/lib/x86_64-linux-gnu/libIPSec_MB.so

# dpdk 19.11.14
cd ~
wget https://fast.dpdk.org/rel/dpdk-19.11.14.tar.xz
tar xf dpdk-19.11.14.tar.xz
mv dpdk-stable-19.11.14 dpdk-inst

cd ~/dpdk-inst
# Disable kernel modules to avoid KNI build issues on newer kernels
rm -rf build && meson build -Denable_kmods=false

cd ~/dpdk-inst/build
ninja
sudo ninja install
sudo ldconfig

# tas
cd ~/code/tas
# replace pthread_yield() with sched_yield()

# Fix: Add flag to suppress packed struct warning (newer GCC is strict)
# In Makefile, change line 8 from:
#   CFLAGS += -std=gnu99 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer
# To:
#   CFLAGS += -std=gnu99 -O3 -g -Wall -Werror -Wno-address-of-packed-member -march=native -fno-omit-frame-pointer

make clean
make RTE_SDK=~/dpdk-inst/build

```

## Running
```bash
# Remove system DPDK if installed (conflicts with custom build)
sudo apt remove -y dpdk dpdk-dev libdpdk-dev

# Setup hugepages (need enough free pages, not just allocated)
sudo mount -t hugetlbfs nodev /dev/hugepages
echo 3072 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
grep HugePages /proc/meminfo  # Check HugePages_Free is > 0

# Clean up any leftover hugepage files from previous runs
sudo rm -f /dev/hugepages/tas_memory

# Check NIC binding status
sudo ~/dpdk-inst/usertools/dpdk-devbind.py --status

# For Mellanox ConnectX NICs: Keep them on mlx5_core driver (NOT vfio-pci)
# DPDK MLX5 PMD works with the kernel driver
# If NIC is on vfio-pci, rebind it:
sudo ~/dpdk-inst/usertools/dpdk-devbind.py -b mlx5_core 0000:41:00.0

# For Intel NICs: Bind to vfio-pci
# sudo modprobe vfio-pci
# sudo ~/dpdk-inst/usertools/dpdk-devbind.py -b vfio-pci <PCI_ADDRESS>

# Run TAS (use --dpdk-extra to specify which NIC if multiple devices)
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.1/24 --fp-cores-max=2 \
  --dpdk-extra='-w' --dpdk-extra='0000:41:00.0'

# To stop: Ctrl+C

# Cleanup hugepages after stopping
sudo umount /dev/hugepages
```