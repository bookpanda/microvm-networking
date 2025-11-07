#!/usr/bin/env bash
set -e # exit on error

# Pin each VM PID matching 'vhost-user1' to cores 13-16
pgrep -f 'vhost-user1' | while read -r PID; do
    if [ -n "$PID" ]; then
        echo "Pinning PID $PID to cores 13-16"
        sudo taskset -cp 13-16 "$PID"
    fi
done
