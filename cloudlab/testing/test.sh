#!/bin/bash
SERVER_IP=192.168.100.1
DURATION=5
PARALLEL=4

# -----------------------------
# Start server suspended
# -----------------------------
iperf3 -s &
SERVER_PID=$!
sleep 0.1                  # give OS time to start
kill -STOP $SERVER_PID      # suspend the server

# attach bpftrace to server
sudo ./trace_server.sh $SERVER_PID &
BPF_SERVER_PID=$!

# resume server
kill -CONT $SERVER_PID

# -----------------------------
# Start client suspended
# -----------------------------
iperf3 -c $SERVER_IP -t $DURATION -P $PARALLEL &
CLIENT_PID=$!
sleep 0.1
kill -STOP $CLIENT_PID      # suspend client

# attach bpftrace to client
sudo bpftrace -e '
tracepoint:syscalls:sys_enter_* /pid == '$CLIENT_PID'/ {
    @interval[comm, probe]++;
    @total[comm, probe]++;
}
interval:s:2 {
    printf("\n--- Client syscall counts (last 2s) ---\n");
    print(@interval);
    clear(@interval);
}
END { 
    printf("\n=== Client cumulative syscall counts ===\n"); 
}
' > trace_client.log &
BPF_CLIENT_PID=$!

# Resume client
kill -CONT $CLIENT_PID

# wait for client to finish
wait $CLIENT_PID

# stop bpftrace safely
sudo kill -INT $BPF_CLIENT_PID
sudo kill -INT $BPF_SERVER_PID
