# TAS Benchmark
```bash
git clone git@github.com:fahren-stack/tas-benchmark.git
cd ~/code/tas-benchmark

cd micro_rpc
make TAS_CODE=~/code/tas

# removes build/ 
make clean

cd ~/code/tas-benchmark
# listen_port, num_threads, _, max_flows, max_bytes
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/echoserver_linux 1234 1 foo 8192 1

# ip, port, num_threads, _, 
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/testclient_linux 10.0.0.1 1234 1 foo

```
## Without TAS
```bash
./micro_rpc/build/echoserver_linux 1234 1 foo 8192 1

./micro_rpc/build/testclient_linux 127.0.0.1 1234 1 foo

```