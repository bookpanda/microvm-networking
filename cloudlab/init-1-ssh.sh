#!/bin/bash
# run this on your local machine

# usage: ./copy_key.sh amd195
HOST=$1

if [ -z "$HOST" ]; then
  echo "Usage: $0 <host-prefix>"
  exit 1
fi

scp -i ~/.ssh/cloudlab ~/.ssh/cloudlab_github ipankam@${HOST}.utah.cloudlab.us:~/.ssh/github
scp -i ~/.ssh/cloudlab ~/.ssh/cloudlab_github.pub ipankam@${HOST}.utah.cloudlab.us:~/.ssh/github.pub
scp -i ~/.ssh/cloudlab ~/Code/cloudlab/microvm-networking/cloudlab/init-2-code.sh ipankam@${HOST}.utah.cloudlab.us:~/init.sh
