# TAS Benchmark
```bash
git clone git@github.com:fahren-stack/tas-benchmark.git
cd ~/code/tas-benchmark

cd micro_rpc
make TAS_CODE=~/code/tas

# removes build/ 
make clean

cd ~/code/tas-benchmark
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/echoserver_linux 1234 1 foo 8192 1

# cd ~/code/tas
# sudo LD_PRELOAD=lib/libtas_interpose.so ../tas-benchmark/micro_rpc/echoserver_linux 1234 1 foo 8192 1 
```