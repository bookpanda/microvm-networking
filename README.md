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
```
## Running
```bash
sudo ./a.out

# will be down, need to turn on
ip addr show tun0
sudo ip addr add 10.0.0.1/24 dev tun0 # assign addr to interface "tun0"
sudo ip link set dev tun0 up
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
read(fd, buffer) → packet now in your program
   |
   v
[Optional] process / write(fd, buffer)
   |
   v
Kernel receives reply → ping gets response
```