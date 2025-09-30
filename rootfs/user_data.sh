#!/bin/sh

cd /root
mount -t tmpfs -o size=64M tmpfs /tmp
HOME=/tmp ./server
# HOME=/tmp ./server > /tmp/server.log 2>&1 &