#!/bin/bash

# Auto-detect the newest Firecracker PID
FC_PID=$(pgrep -n firecracker)

if [ -z "$FC_PID" ]; then
    echo "Firecracker PID not found!"
    exit 1
fi

echo "Tracing Firecracker PID: $FC_PID"

sudo bpftrace -e "
BEGIN { printf(\"Tracing network syscalls for Firecracker PID $FC_PID... Ctrl+C to exit\n\"); }

/* Core I/O */
tracepoint:syscalls:sys_enter_read   /pid == $FC_PID/ { @interval[comm, \"read\"]++; @total[comm, \"read\"]++ }
tracepoint:syscalls:sys_enter_write  /pid == $FC_PID/ { @interval[comm, \"write\"]++; @total[comm, \"write\"]++ }

/* Sending */
tracepoint:syscalls:sys_enter_sendto    /pid == $FC_PID/ { @interval[comm, \"sendto\"]++; @total[comm, \"sendto\"]++ }
tracepoint:syscalls:sys_enter_sendmsg   /pid == $FC_PID/ { @interval[comm, \"sendmsg\"]++; @total[comm, \"sendmsg\"]++ }
tracepoint:syscalls:sys_enter_sendmmsg  /pid == $FC_PID/ { @interval[comm, \"sendmmsg\"]++; @total[comm, \"sendmmsg\"]++ }

/* Receiving */
tracepoint:syscalls:sys_enter_recvfrom  /pid == $FC_PID/ { @interval[comm, \"recvfrom\"]++; @total[comm, \"recvfrom\"]++ }
tracepoint:syscalls:sys_enter_recvmsg   /pid == $FC_PID/ { @interval[comm, \"recvmsg\"]++; @total[comm, \"recvmsg\"]++ }
tracepoint:syscalls:sys_enter_recvmmsg  /pid == $FC_PID/ { @interval[comm, \"recvmmsg\"]++; @total[comm, \"recvmmsg\"]++ }

/* Event handling */
tracepoint:syscalls:sys_enter_select    /pid == $FC_PID/ { @interval[comm, \"select\"]++; @total[comm, \"select\"]++ }
tracepoint:syscalls:sys_enter_poll      /pid == $FC_PID/ { @interval[comm, \"poll\"]++; @total[comm, \"poll\"]++ }
tracepoint:syscalls:sys_enter_epoll_wait /pid == $FC_PID/ { @interval[comm, \"epoll_wait\"]++; @total[comm, \"epoll_wait\"]++ }
tracepoint:syscalls:sys_enter_epoll_ctl  /pid == $FC_PID/ { @interval[comm, \"epoll_ctl\"]++; @total[comm, \"epoll_ctl\"]++ }

/* Socket configuration & cleanup */
tracepoint:syscalls:sys_enter_setsockopt /pid == $FC_PID/ { @interval[comm, \"setsockopt\"]++; @total[comm, \"setsockopt\"]++ }
tracepoint:syscalls:sys_enter_getsockopt /pid == $FC_PID/ { @interval[comm, \"getsockopt\"]++; @total[comm, \"getsockopt\"]++ }
tracepoint:syscalls:sys_enter_close      /pid == $FC_PID/ { @interval[comm, \"close\"]++; @total[comm, \"close\"]++ }

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
