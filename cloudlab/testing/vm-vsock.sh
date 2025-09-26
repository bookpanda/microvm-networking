#!/bin/bash
PORT=5000

while true; do
    # Listen on vsock port and fork a child per connection.
    # For each line received, feed it to `bash -s` so complex commands run.
    socat -v VSOCK-LISTEN:$PORT,reuseaddr,fork SYSTEM:"bash -c '
        while IFS= read -r cmd; do
            if [ -z \"\$cmd\" ]; then
                # ignore empty lines
                continue
            fi
            printf \"[listener] Running: %s\n\" \"\$cmd\" >&2
            # execute the received command by piping it to bash -s
            printf \"%s\n\" \"\$cmd\" | bash -s
            printf \"[listener] DONE\n\" >&2
        done
    '"
done
