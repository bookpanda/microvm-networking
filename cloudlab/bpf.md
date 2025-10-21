# Setup bpftrace
```bash
sudo apt install -y zip bison build-essential cmake flex git libedit-dev \
  libllvm18 llvm-18-dev libclang-18-dev python3 zlib1g-dev libelf-dev libfl-dev python3-setuptools \
  liblzma-dev libdebuginfod-dev arping netperf iperf libpolly-18-dev

# install and build bcc (dependency of bpftrace)
git clone https://github.com/iovisor/bcc.git
mkdir bcc/build; cd bcc/build
cmake ..
make
sudo make install
cmake -DPYTHON_CMD=python3 .. # build python3 binding
pushd src/python/
make
sudo make install
popd

# install and build bcc (dependency of bpftrace)
sudo apt install -y bcc libcereal-dev libgtest-dev pahole
# install and build bpftrace
git clone --recurse-submodules https://github.com/bpftrace/bpftrace
mkdir -p bpftrace/build
cd bpftrace/build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF ../
make
sudo make install
```