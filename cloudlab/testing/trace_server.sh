#!/bin/bash

if [ -n "$1" ]; then
    PID="$1"
    echo "Tracing PID: $PID"
else
    echo "PID not found!"
    exit 1
fi

sudo bpftrace -e '
tracepoint:syscalls:sys_enter_* /pid == '$PID'/ {
    @interval[comm, probe]++;
    @total[comm, probe]++;
}

interval:s:2 {
    printf("\n--- Server syscall counts (last 2s) ---\n");
    print(@interval);
    clear(@interval);
}
END { 
    printf("\n=== Server cumulative syscall counts ===\n"); 
}
' | tee trace_server.log
