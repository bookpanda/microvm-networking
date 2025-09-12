#!/bin/bash
SERVER_IP=192.168.100.1
DURATION=5
PARALLEL=4

# Start bpftrace first
sudo bpftrace -e '
tracepoint:syscalls:sys_enter_* /comm == "iperf3" && argv[1] == "-c"/ {
    @interval[comm, probe]++;
    @total[comm, probe]++;
}
interval:s:2 {
    printf("\n--- Syscall counts (last 2s) ---\n");
    print(@interval);
    clear(@interval);
}
END {
    printf("\n=== Cumulative syscall counts ===\n");
}
' > trace.log &
BPF_PID=$!

# Give bpftrace time to attach
sleep 1

# Now run iperf3
iperf3 -c $SERVER_IP -t $DURATION -P $PARALLEL

# Stop bpftrace after iperf3 ends
kill -INT $BPF_PID
