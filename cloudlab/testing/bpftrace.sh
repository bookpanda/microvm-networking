#!/bin/bash

if [ -n "$1" ]; then
    PID="$1"
    echo "Tracing PID: $PID"
else
    PID=$(pgrep -n firecracker)
    echo "Tracing Firecracker PID: $PID"
fi

if [ -z "$PID" ]; then
    echo "PID not found!"
    exit 1
fi

sudo bpftrace -e "
BEGIN { printf(\"Tracing network syscalls for PID $PID... Ctrl+C to exit\n\"); }

/* Core I/O */
tracepoint:syscalls:sys_enter_read   /pid == $PID/ { @interval[comm, \"read\"]++; @total[comm, \"read\"]++ }
tracepoint:syscalls:sys_enter_write  /pid == $PID/ { @interval[comm, \"write\"]++; @total[comm, \"write\"]++ }

/* Sending */
tracepoint:syscalls:sys_enter_sendto    /pid == $PID/ { @interval[comm, \"sendto\"]++; @total[comm, \"sendto\"]++ }
tracepoint:syscalls:sys_enter_sendmsg   /pid == $PID/ { @interval[comm, \"sendmsg\"]++; @total[comm, \"sendmsg\"]++ }
tracepoint:syscalls:sys_enter_sendmmsg  /pid == $PID/ { @interval[comm, \"sendmmsg\"]++; @total[comm, \"sendmmsg\"]++ }

/* Receiving */
tracepoint:syscalls:sys_enter_recvfrom  /pid == $PID/ { @interval[comm, \"recvfrom\"]++; @total[comm, \"recvfrom\"]++ }
tracepoint:syscalls:sys_enter_recvmsg   /pid == $PID/ { @interval[comm, \"recvmsg\"]++; @total[comm, \"recvmsg\"]++ }
tracepoint:syscalls:sys_enter_recvmmsg  /pid == $PID/ { @interval[comm, \"recvmmsg\"]++; @total[comm, \"recvmmsg\"]++ }

/* Event handling */
tracepoint:syscalls:sys_enter_select    /pid == $PID/ { @interval[comm, \"select\"]++; @total[comm, \"select\"]++ }
tracepoint:syscalls:sys_enter_poll      /pid == $PID/ { @interval[comm, \"poll\"]++; @total[comm, \"poll\"]++ }
tracepoint:syscalls:sys_enter_epoll_wait /pid == $PID/ { @interval[comm, \"epoll_wait\"]++; @total[comm, \"epoll_wait\"]++ }
tracepoint:syscalls:sys_enter_epoll_ctl  /pid == $PID/ { @interval[comm, \"epoll_ctl\"]++; @total[comm, \"epoll_ctl\"]++ }

/* Socket configuration & cleanup */
tracepoint:syscalls:sys_enter_setsockopt /pid == $PID/ { @interval[comm, \"setsockopt\"]++; @total[comm, \"setsockopt\"]++ }
tracepoint:syscalls:sys_enter_getsockopt /pid == $PID/ { @interval[comm, \"getsockopt\"]++; @total[comm, \"getsockopt\"]++ }
tracepoint:syscalls:sys_enter_close      /pid == $PID/ { @interval[comm, \"close\"]++; @total[comm, \"close\"]++ }

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
