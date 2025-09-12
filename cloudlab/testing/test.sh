#!/bin/bash
SERVER_IP=192.168.100.1
DURATION=30
PARALLEL=4

sudo pkill iperf3
sleep 0.5

# -----------------------------
# Start server suspended
# -----------------------------
iperf3 -s &
SERVER_PID=$!
sleep 0.1                  # give OS time to start
kill -STOP $SERVER_PID      # suspend the server

# attach bpftrace to server
sudo ./trace_syscalls.sh $SERVER_PID trace_server.log &
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
sudo ./trace_syscalls.sh $CLIENT_PID trace_client.log &
BPF_CLIENT_PID=$!

# Resume client
kill -CONT $CLIENT_PID

# wait for client to finish
wait $CLIENT_PID

# stop bpftrace safely
sudo kill -INT $BPF_CLIENT_PID
sudo kill -INT $BPF_SERVER_PID
