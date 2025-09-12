#!/bin/bash

FC_PID=$(pgrep -n firecracker)  # -n = newest instance; adjust if multiple VMs

if [ -z "$FC_PID" ]; then
    echo "Firecracker PID not found!"
    exit 1
fi

echo "Tracing Firecracker PID: $FC_PID"

sudo bpftrace -e "
BEGIN { printf(\"Tracing network syscalls for Firecracker PID $FC_PID... Ctrl+C to exit\n\"); }

/* Core I/O */
tracepoint:syscalls:sys_enter_read   /pid == $FC_PID/ { @interval[\"read\"]++; @total[\"read\"]++ }
tracepoint:syscalls:sys_enter_write  /pid == $FC_PID/ { @interval[\"write\"]++; @total[\"write\"]++ }

/* Sending */
tracepoint:syscalls:sys_enter_sendto    /pid == $FC_PID/ { @interval[\"sendto\"]++; @total[\"sendto\"]++ }
tracepoint:syscalls:sys_enter_sendmsg   /pid == $FC_PID/ { @interval[\"sendmsg\"]++; @total[\"sendmsg\"]++ }
tracepoint:syscalls:sys_enter_sendmmsg  /pid == $FC_PID/ { @interval[\"sendmmsg\"]++; @total[\"sendmmsg\"]++ }

/* Receiving */
tracepoint:syscalls:sys_enter_recvfrom  /pid == $FC_PID/ { @interval[\"recvfrom\"]++; @total[\"recvfrom\"]++ }
tracepoint:syscalls:sys_enter_recvmsg   /pid == $FC_PID/ { @interval[\"recvmsg\"]++; @total[\"recvmsg\"]++ }
tracepoint:syscalls:sys_enter_recvmmsg  /pid == $FC_PID/ { @interval[\"recvmmsg\"]++; @total[\"recvmmsg\"]++ }

/* Event handling */
tracepoint:syscalls:sys_enter_select    /pid == $FC_PID/ { @interval[\"select\"]++; @total[\"select\"]++ }
tracepoint:syscalls:sys_enter_poll      /pid == $FC_PID/ { @interval[\"poll\"]++; @total[\"poll\"]++ }
tracepoint:syscalls:sys_enter_epoll_wait /pid == $FC_PID/ { @interval[\"epoll_wait\"]++; @total[\"epoll_wait\"]++ }
tracepoint:syscalls:sys_enter_epoll_ctl  /pid == $FC_PID/ { @interval[\"epoll_ctl\"]++; @total[\"epoll_ctl\"]++ }

/* Socket configuration & cleanup */
tracepoint:syscalls:sys_enter_setsockopt /pid == $FC_PID/ { @interval[\"setsockopt\"]++; @total[\"setsockopt\"]++ }
tracepoint:syscalls:sys_enter_getsockopt /pid == $FC_PID/ { @interval[\"getsockopt\"]++; @total[\"getsockopt\"]++ }
tracepoint:syscalls:sys_enter_close      /pid == $FC_PID/ { @interval[\"close\"]++; @total[\"close\"]++ }

/* Print every 2 seconds and clear interval counters */
interval:s:2 {
    printf(\"\\n--- Syscall counts (last 2 sec) ---\\n\");
    print(@interval);
    clear(@interval);
}

/* Print cumulative counts on Ctrl+C */
END {
    printf(\"\\n=== Cumulative syscall counts ===\\n\");
    print(@total);
}
"
