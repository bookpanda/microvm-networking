#!/bin/bash
PORT=5000

while true; do
    # Listen on vsock port and fork a child per connection.
    # For each line received, feed it to `bash -s` so complex commands run.
    # Merge stderr into stdout (2>&1) so the client receives both streams.
    socat -v VSOCK-LISTEN:$PORT,reuseaddr,fork SYSTEM:"bash -c '
        while IFS= read -r cmd; do
            if [ -z \"\$cmd\" ]; then
                continue
            fi

            # Debug to listener's stderr (not sent to client)
            printf \"[listener] Running: %s\n\" \"\$cmd\" >&2

            # Execute the received command; send both stdout and stderr back to the client
            printf \"%s\n\" \"\$cmd\" | bash -s 2>&1

            printf \"[listener] DONE\n\" >&2
        done
    '"
done
