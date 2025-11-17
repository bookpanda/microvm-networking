#!/bin/bash

chmod 600 ~/.ssh/github

mkdir -p ~/tas/include
mv ~/tas-include/* ~/tas/include/
rm -rf ~/tas-include

mkdir -p ~/tas/lib
mv ~/tas-lib/* ~/tas/lib/
rm -rf ~/tas-lib

sudo apt update
sudo apt install -y make gcc python

git clone git@github.com:fahren-stack/tas-benchmark.git
cd ~/tas-benchmark/micro_rpc
make TAS_CODE=~/tas