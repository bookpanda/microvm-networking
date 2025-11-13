# TAS Benchmark
```bash
git clone git@github.com:fahren-stack/tas-benchmark.git
cd ~/code/tas-benchmark


sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/echoserver_linux 1234 1 foo 8192 1

```