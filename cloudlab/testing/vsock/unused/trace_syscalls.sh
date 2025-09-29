#!/bin/bash

if [ -n "$1" ]; then
    PID="$1"
    echo "Tracing PID: $PID"
else
    echo "PID not found!"
    exit 1
fi

if [ -n "$2" ]; then
    LOG_FILE="$2"
    echo "Logging to: $LOG_FILE"
else
    echo "LOG_FILE not found!"
    exit 1
fi

sudo bpftrace -e '
BEGIN { printf("Tracing network syscalls for PID '$PID'...\n"); }

tracepoint:syscalls:sys_enter_* /pid == '$PID'/ {
    @interval[comm, probe]++;
    @total[comm, probe]++;
}

interval:s:2 {
    printf("\n--- syscall counts (last 2s) ---\n");
    print(@interval);
    clear(@interval);
}
END { 
    printf("\n=== cumulative syscall counts ===\n"); 
}
' | tee $LOG_FILE
