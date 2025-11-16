# TAS Benchmark
```bash
cd ~/code
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
## Two-Host Setup (Recommended)
### Host 1 (Server - 10.0.0.1)
```bash
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.1/24 --fp-cores-max=2 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

cd ~/code/tas-benchmark
# listen_port, num_threads, _, max_flows, max_bytes
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/echoserver_linux 1234 1 foo 8192 1
```
### Host 2 (Client - 10.0.0.2)
```bash
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.2/24 --fp-cores-max=2 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

# Connect to server IP (10.0.0.1)
cd ~/code/tas-benchmark
# ip, port, num_threads, _, 
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/testclient_linux 10.0.0.1 1234 1 foo
```

## Without TAS
```bash
./micro_rpc/build/echoserver_linux 1234 1 foo 8192 1

./micro_rpc/build/testclient_linux 127.0.0.1 1234 1 foo

```