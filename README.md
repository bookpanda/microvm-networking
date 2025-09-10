# tun-tap
## Setting up VM
if boot after setup fails, clear `CD/DVD`, then rerun
```bash
# ssh
sudo apt update
sudo apt install openssh-server
sudo systemctl enable ssh
sudo systemctl start ssh
ifconfig

# in mac
ssh -l <username> <ip from ifconfig>

# in VM, create a new ssh key for github, add in github settings
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/github
ssh -T git@github.com

git config --global user.name "Your Name"
git config --global user.email "you@example.com"

# gcc
sudo apt update
sudo apt install build-essential clangd
sudo apt install -y cmake
sudo apt install -y libseccomp-dev build-essential
sudo apt install -y cmake clang pkg-config libssl-dev

# go
wget https://go.dev/dl/go1.23.7.linux-arm64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.7.linux-arm64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc
go version

# rust
sudo apt install cargo
sudo apt install -y curl build-essential
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
echo 'source $HOME/.cargo/env' >> ~/.bashrc
source ~/.bashrc
```
## Running
```bash
sudo ./a.out

# will be down, need to turn on
ip addr show tun0
sudo ip addr add 10.0.0.1/24 dev tun0 # assign addr to interface "tun0"
sudo ip link set dev tun0 up
ping -c 1 10.0.0.1

# can also ping from mac
sudo route -n add -net 10.0.0.0/24 192.168.1.186 # VM ip
ping -c 1 10.0.0.1

```
``` bash
ping 10.0.0.1
   |
   v
Kernel routing → sees 10.0.0.1 on tun0
   |
   v
TUN driver queues packet → user-space (your program)
   |
   v
# copies the packet from kernel memory to user-space buffer.
read(fd, buffer) packet now in your program
   |
   v
# driver copies the packet back into kernel memory
[Optional] process / write(fd, buffer)
   |
   v
Kernel receives reply → ping gets response
```