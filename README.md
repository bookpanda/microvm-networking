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
```