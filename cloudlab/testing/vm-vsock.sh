#!/bin/bash

PORT=5000

while true; do
    # Listen on vsock port 5000, execute sockperf when host/VM sends "RUN"
    socat -v VSOCK-LISTEN:$PORT,reuseaddr,fork SYSTEM:"bash -c '
        while read cmd; do
            if [ \"\$cmd\" = \"RUN\" ]; then
                echo \"Running sockperf\";
                sockperf ping-pong -i 127.0.0.1 -p 5001;
                echo \"DONE\";
            fi
        done'"
done
