#!/bin/bash

FC_PID=119718

sudo bpftrace -e "
BEGIN { printf(\"Tracing network syscalls for Firecracker PID $FC_PID... Ctrl+C to exit\n\"); }

/* Core I/O */
tracepoint:syscalls:sys_enter_read   /pid == $FC_PID/ { @[comm, \"read\"]++ }
tracepoint:syscalls:sys_enter_write  /pid == $FC_PID/ { @[comm, \"write\"]++ }

/* Sending */
tracepoint:syscalls:sys_enter_sendto    /pid == $FC_PID/ { @[comm, \"sendto\"]++ }
tracepoint:syscalls:sys_enter_sendmsg   /pid == $FC_PID/ { @[comm, \"sendmsg\"]++ }
tracepoint:syscalls:sys_enter_sendmmsg  /pid == $FC_PID/ { @[comm, \"sendmmsg\"]++ }

/* Receiving */
tracepoint:syscalls:sys_enter_recvfrom  /pid == $FC_PID/ { @[comm, \"recvfrom\"]++ }
tracepoint:syscalls:sys_enter_recvmsg   /pid == $FC_PID/ { @[comm, \"recvmsg\"]++ }
tracepoint:syscalls:sys_enter_recvmmsg  /pid == $FC_PID/ { @[comm, \"recvmmsg\"]++ }

/* Event handling */
tracepoint:syscalls:sys_enter_select    /pid == $FC_PID/ { @[comm, \"select\"]++ }
tracepoint:syscalls:sys_enter_poll      /pid == $FC_PID/ { @[comm, \"poll\"]++ }
tracepoint:syscalls:sys_enter_epoll_wait /pid == $FC_PID/ { @[comm, \"epoll_wait\"]++ }
tracepoint:syscalls:sys_enter_epoll_ctl  /pid == $FC_PID/ { @[comm, \"epoll_ctl\"]++ }

/* Socket configuration & cleanup */
tracepoint:syscalls:sys_enter_setsockopt /pid == $FC_PID/ { @[comm, \"setsockopt\"]++ }
tracepoint:syscalls:sys_enter_getsockopt /pid == $FC_PID/ { @[comm, \"getsockopt\"]++ }
tracepoint:syscalls:sys_enter_close      /pid == $FC_PID/ { @[comm, \"close\"]++ }

/* Print every 2 seconds and clear counters */
interval:s:2 {
    printf(\"\\n--- Syscall counts (last 2 sec) ---\\n\");
    print(@);
    clear(@);
}
"
