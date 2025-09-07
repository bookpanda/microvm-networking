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
sudo apt install build-essential
```