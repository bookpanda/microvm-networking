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
## Two-Host Setup
### Host 1 (Server - 10.0.0.1)
```bash
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.1/24 --fp-cores-max=4 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

cd ~/code/tas-benchmark
# listen_port, num_threads, _, max_flows, max_bytes
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/echoserver_linux 1234 16 foo 4096 4096
```
### Host 2 (Client - 10.0.0.2)
```bash
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.2/24 --fp-cores-max=4 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

# Connect to server IP (10.0.0.1)
cd ~/code/tas-benchmark
# ip, port, num_threads, _, 
sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so ./micro_rpc/build/testclient_linux 10.0.0.1 1234 32 foo
```
# Without TAS
```bash
# same host
./micro_rpc/build/echoserver_linux 1234 16 foo 4096 4096
./micro_rpc/build/testclient_linux 127.0.0.1 1234 16 foo

# different host
# host 0
./micro_rpc/build/echoserver_linux 1234 16 foo 4096 4096
# host 1
./micro_rpc/build/testclient_linux 10.10.1.1 1234 16 foo
```
## VM Setup
- setup node networking, VM in `./cloud-hypervisor/vanilla` directory
```bash
# NODE 0 VM setup
ssh-keygen -f '/users/ipankam/.ssh/known_hosts' -R '192.168.100.2'
# first time after ssh-keygen, do manually to say YES
sshpass -p "cloud123" scp ~/.ssh/github cloud@192.168.100.2:~/.ssh/github
sshpass -p "cloud123" scp ~/code/microvm-networking/cloudlab/config cloud@192.168.100.2:~/.ssh/config
sshpass -p "cloud123" scp -r ~/code/tas/include cloud@192.168.100.2:~/tas-include
sshpass -p "cloud123" scp -r ~/code/tas/lib cloud@192.168.100.2:~/tas-lib
sshpass -p "cloud123" scp ~/code/microvm-networking/tas/vm_init.sh cloud@192.168.100.2:~/init.sh

# NODE 1 VM setup
ssh-keygen -f '/users/ipankam/.ssh/known_hosts' -R '192.168.101.2'
sshpass -p "cloud123" scp ~/.ssh/github cloud@192.168.101.2:~/.ssh/github
sshpass -p "cloud123" scp ~/code/microvm-networking/cloudlab/config cloud@192.168.101.2:~/.ssh/config
sshpass -p "cloud123" scp -r ~/code/tas/include cloud@192.168.101.2:~/tas-include
sshpass -p "cloud123" scp -r ~/code/tas/lib cloud@192.168.101.2:~/tas-lib
sshpass -p "cloud123" scp ~/code/microvm-networking/tas/vm_init.sh cloud@192.168.101.2:~/init.sh

# host-vm
./micro_rpc/build/echoserver_linux 1234 4 foo 4096 8192
./micro_rpc/build/testclient_linux 192.168.100.1 1234 4 foo

# TAS researchers likely avoided iperf3 because it depends on kernel TCP features that TAS doesn’t implement fully
# host 0
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.1/24 --fp-cores-max=4 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so iperf3 -s -p 5201

# host 1
sudo ~/code/tas/tas/tas --ip-addr=10.0.0.2/24 --fp-cores-max=4 \
  --dpdk-extra='-w' --dpdk-extra='0000:03:00.1'

sudo LD_PRELOAD=~/code/tas/lib/libtas_interpose.so iperf3 -c 10.0.0.1 -t 10 -p 5201
```