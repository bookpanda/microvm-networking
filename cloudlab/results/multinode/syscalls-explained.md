# Syscall Explanations: Network Performance Analysis

This document explains every syscall observed in the multinode networking benchmarks, organized by functional category.

---

## 1. Core Network I/O Syscalls

### `read()` / `write()`
**Purpose**: Basic I/O operations for reading from/writing to file descriptors (including sockets)

**Signature**: 
```c
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
```

**Context**: 
- Used extensively in **baremetal** iperf3 (1.9M reads, 646K writes)
- Single buffer per syscall (not vectorized)
- Each call crosses user→kernel boundary
- **Performance impact**: High syscall count due to non-vectorized I/O

**Example**: Baremetal server does ~1.9M reads for 82.2 GB transfer = ~43 KB per read

---

### `readv()` / `writev()`
**Purpose**: Vectorized (scatter-gather) I/O - read/write multiple buffers in a single syscall

**Signature**:
```c
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

**Context**:
- Primary I/O method for **VM-VM** (617K readv, 266K writev on server)
- Primary I/O method for **KVM-KVM** (554K writev on server)
- More efficient than read/write for handling fragmented data
- **Performance benefit**: Reduces syscall count by batching multiple buffers

**Example**: VM-VM server uses 617K readv for 37.1 GB = ~60 KB per call (aggregates multiple buffers)

---

### `recvfrom()` / `sendto()`
**Purpose**: Socket-specific I/O with address information

**Signature**:
```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

**Context**:
- **Exclusively used by sockperf** in latency tests (ping-pong)
- Baremetal: 455,731 recvfrom + 455,731 sendto = 1:1 ratio
- Can retrieve/specify source/destination address per packet
- **Ideal for**: UDP or datagram-oriented protocols

**Efficiency**: 2 syscalls per round-trip message (recv then send)

---

### `pread64()` / `pwrite64()`
**Purpose**: Positioned read/write - I/O at specific file offset without changing file position

**Signature**:
```c
ssize_t pread64(int fd, void *buf, size_t count, off64_t offset);
ssize_t pwrite64(int fd, const void *buf, size_t count, off64_t offset);
```

**Context**:
- Found in **KVM configurations** (not MicroVM or baremetal)
- Used for disk I/O operations (not network)
- Thread-safe - doesn't modify shared file offset
- **KVM-KVM server**: 79 pread64, 61 pwrite64 (disk backing operations)

**Network relevance**: Indirect - used for VM disk state, not packet I/O

---

### `preadv()` / `pwritev()`
**Purpose**: Positioned vectorized I/O - combines benefits of pread64 and readv

**Signature**:
```c
ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
```

**Context**:
- Used in **KVM configurations** for disk operations
- KVM-KVM server: 5 preadv, 5 pwritev
- More efficient than multiple pread64 calls

**Network relevance**: VM image file access, not network I/O

---

## 2. Event Notification / Multiplexing Syscalls

### `pselect6()`
**Purpose**: Monitor multiple file descriptors for I/O readiness (with signal masking)

**Signature**:
```c
int pselect6(int nfds, fd_set *readfds, fd_set *writefds,
             fd_set *exceptfds, const struct timespec *timeout,
             const sigset_t *sigmask);
```

**Context**:
- **Heavily used by baremetal** iperf3: 1,895,163 calls on server
- Traditional I/O multiplexing (older than epoll)
- Can monitor up to 1024 file descriptors
- **Performance characteristic**: 1:1 ratio with read() calls on baremetal

**Typical flow**:
```
pselect6() → wait for socket readable → read() → pselect6() → ...
```

**Why so many calls?**: Each network packet arrival triggers pselect6 wakeup

---

### `epoll_pwait()`
**Purpose**: Scalable I/O event notification (Linux-specific, better than select/poll)

**Signature**:
```c
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents,
                int timeout, const sigset_t *sigmask);
```

**Context**:
- Used by **MicroVM (Firecracker)** exclusively
- VM-VM throughput: 138K epoll_pwait (server), 191K (client)
- VM-VM latency: 343K epoll_pwait on both sides
- More scalable than pselect6 for many file descriptors

**Advantages over pselect6**:
- O(1) performance vs O(n) for select
- No 1024 fd limit
- Edge-triggered and level-triggered modes

**Pattern in VM-VM**:
```
epoll_pwait() → readv()/writev() → process → epoll_pwait() → ...
```

---

### `ppoll()`
**Purpose**: Poll multiple file descriptors with signal masking and precise timeout

**Signature**:
```c
int ppoll(struct pollfd *fds, nfds_t nfds,
          const struct timespec *tmo_p, const sigset_t *sigmask);
```

**Context**:
- Used by **KVM (QEMU)** configurations
- KVM-KVM throughput: 14,604 ppoll (server), 60,757 (client)
- KVM-KVM latency: 688,115 ppoll (server)
- Similar to pselect6 but with different interface

**Why KVM uses ppoll**: QEMU's event loop architecture uses poll-based I/O multiplexing

**Overhead in KVM-KVM latency**: ~688K ppoll calls for 226K messages = 3 ppoll per message

---

### `epoll_ctl()`
**Purpose**: Control interface for epoll instance (add/modify/delete file descriptors)

**Signature**:
```c
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

**Context**:
- Minimal usage: VM-VM shows only 6 calls
- Used to register/unregister sockets with epoll
- Called during setup, not per-packet

**Operations**:
- `EPOLL_CTL_ADD`: Add fd to epoll instance
- `EPOLL_CTL_MOD`: Modify event flags
- `EPOLL_CTL_DEL`: Remove fd from monitoring

---

## 3. Device Control Syscalls

### `ioctl()`
**Purpose**: Device-specific I/O control operations (generic device manipulation)

**Signature**:
```c
int ioctl(int fd, unsigned long request, ...);
```

**Context**: **Critical for understanding VM performance!**

**Usage patterns**:

#### MicroVM (Firecracker):
- VM-VM throughput: **163,632 ioctl** (server), **252,548** (client)
- VM-VM latency: **676,756 ioctl** (server), **676,796** (client)
- Called by **fc_vcpu 0 thread** (vCPU thread)
- **Purpose**: KVM_RUN operations to enter/exit guest VM

#### Standard KVM:
- KVM-KVM throughput: **724,154 ioctl** (server)
- KVM-KVM latency: **462,795 ioctl** (server)
- Much higher than MicroVM in throughput tests

#### KVM with vhost:
- KVM-vhost throughput: **84,007 ioctl** (90% reduction vs standard KVM!)
- KVM-vhost latency: **2,052 ioctl** (99.6% reduction!)
- **Reason**: vhost-net handles packets in kernel, reducing VM exits

**What ioctl does in KVM context**:
```c
ioctl(vcpu_fd, KVM_RUN, NULL);  // Enter guest, exit on I/O
```

Each ioctl represents a **VM exit** (guest→host transition), which is expensive (~1000 cycles).

**Performance impact**: 
- Each packet in standard KVM may trigger multiple VM exits
- vhost-net keeps packet processing in kernel, minimizing exits

---

## 4. Socket Management Syscalls

### `socket()`
**Purpose**: Create a communication endpoint

**Signature**:
```c
int socket(int domain, int type, int protocol);
```

**Context**:
- Minimal usage (1-2 calls during setup)
- Creates the socket file descriptor used for all subsequent I/O
- Example: `socket(AF_INET, SOCK_STREAM, 0)` for TCP

---

### `bind()`
**Purpose**: Assign address to a socket

**Signature**:
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**Context**:
- Called once during server setup
- Associates socket with IP address and port
- Required before `listen()` on server sockets

---

### `listen()`
**Purpose**: Mark socket as passive (accepting connections)

**Signature**:
```c
int listen(int sockfd, int backlog);
```

**Context**:
- Called once on server socket
- Enables socket to accept incoming connections
- Sets connection queue size (backlog)

---

### `accept()` / `accept4()`
**Purpose**: Accept incoming connection on listening socket

**Signature**:
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
```

**Context**:
- Baremetal: 5 accept calls (one per iperf3 parallel stream + control)
- MicroVM: uses `accept4()` (extended version with flags)
- `accept4()` allows setting socket flags atomically (e.g., O_NONBLOCK)

**Difference**: `accept4()` adds flags parameter to avoid separate fcntl() call

---

### `setsockopt()` / `getsockopt()`
**Purpose**: Set/get socket options

**Signature**:
```c
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);
```

**Context**:
- Baremetal iperf3: 3 setsockopt, 130 getsockopt (server)
- Used to configure TCP parameters, buffer sizes, timeouts
- Common options: SO_REUSEADDR, TCP_NODELAY, SO_RCVBUF

**Example**:
```c
int flag = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
```

---

### `getsockname()` / `getpeername()`
**Purpose**: Get local/remote socket address

**Signature**:
```c
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

**Context**:
- Baremetal: 9 getsockname, 5 getpeername
- Retrieves IP address and port information
- Used for logging and connection tracking

---

## 5. Memory Management Syscalls

### `mmap()`
**Purpose**: Map files or devices into memory

**Signature**:
```c
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
```

**Context**:
- Setup/teardown operations
- Used for shared memory regions, file mappings
- KVM: Maps guest memory into host address space
- Not per-packet, but affects overall VM performance

---

### `munmap()`
**Purpose**: Unmap memory region

**Signature**:
```c
int munmap(void *addr, size_t length);
```

**Context**:
- Cleanup operation
- Releases memory mappings created by mmap()
- Minimal impact on network performance

---

### `madvise()`
**Purpose**: Provide advice about memory usage patterns

**Signature**:
```c
int madvise(void *addr, size_t length, int advice);
```

**Context**:
- Memory optimization hint to kernel
- Common advice: MADV_DONTNEED, MADV_WILLNEED, MADV_SEQUENTIAL
- Used by VMs for memory management optimization

---

### `mprotect()`
**Purpose**: Change memory region protection

**Signature**:
```c
int mprotect(void *addr, size_t len, int prot);
```

**Context**:
- Setup operation
- Changes read/write/execute permissions on memory pages
- Used during initialization, not per-packet

---

## 6. Process/Thread Management Syscalls

### `clone3()`
**Purpose**: Create new process or thread (modern interface)

**Signature**:
```c
int clone3(struct clone_args *cl_args, size_t size);
```

**Context**:
- Creates parallel iperf3 streams (4 threads for `-P 4`)
- More flexible than older `clone()` or `fork()`
- Baremetal: 4 clone3 calls = 4 parallel connections

---

### `exit()` / `exit_group()`
**Purpose**: Terminate process or thread

**Signature**:
```c
void exit(int status);
void exit_group(int status);
```

**Context**:
- `exit()`: Single thread termination
- `exit_group()`: Entire process termination
- Called when benchmark completes

---

### `set_robust_list()` / `rseq()`
**Purpose**: Thread management infrastructure

**Signature**:
```c
int set_robust_list(struct robust_list_head *head, size_t len);
int rseq(struct rseq *rseq, uint32_t rseq_len, int flags, uint32_t sig);
```

**Context**:
- `set_robust_list()`: Handles futex cleanup on thread death
- `rseq()`: Restartable sequences for per-CPU operations
- Modern threading primitives, minimal performance impact

---

## 7. Synchronization Syscalls

### `futex()`
**Purpose**: Fast userspace mutex (wait/wake operations)

**Signature**:
```c
int futex(int *uaddr, int futex_op, int val,
          const struct timespec *timeout, int *uaddr2, int val3);
```

**Context**:
- Used for thread synchronization
- KVM configurations show significant usage:
  - KVM-KVM throughput: 11,480 futex (server)
  - KVM-vhost throughput: 998 futex (server)
- **Pattern**: I/O threads waiting/waking other threads

**Why KVM uses more**: Multi-threaded QEMU architecture with I/O threads and vCPU threads

---

### `rt_sigprocmask()` / `rt_sigaction()` / `rt_sigreturn()`
**Purpose**: Real-time signal handling

**Signatures**:
```c
int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int rt_sigaction(int signum, const struct sigaction *act, 
                 struct sigaction *oldact);
int rt_sigreturn(void);
```

**Context**:
- Signal management (e.g., handling Ctrl+C)
- Used with pselect6/ppoll for atomic signal masking
- Minimal performance impact (setup operations)

---

## 8. File System Syscalls

### `openat()`
**Purpose**: Open file relative to directory file descriptor

**Signature**:
```c
int openat(int dirfd, const char *pathname, int flags, mode_t mode);
```

**Context**:
- Opens files for reading/writing
- Used for log files, config files
- Not performance-critical for network benchmarks

---

### `close()`
**Purpose**: Close file descriptor

**Signature**:
```c
int close(int fd);
```

**Context**:
- Cleanup operation
- Closes sockets, files after use
- Baremetal: 15 close calls (cleanup after each connection)

---

### `lseek()`
**Purpose**: Reposition file offset

**Signature**:
```c
off_t lseek(int fd, off_t offset, int whence);
```

**Context**:
- VM configurations use for disk file operations
- Not used for network I/O

---

### `fdatasync()`
**Purpose**: Synchronize file data to disk

**Signature**:
```c
int fdatasync(int fd);
```

**Context**:
- KVM configurations: 4-10 calls
- Ensures VM disk state is written to storage
- Network performance impact: Minimal

---

### `ftruncate()`
**Purpose**: Truncate file to specified length

**Signature**:
```c
int ftruncate(int fd, off_t length);
```

**Context**:
- Used during file operations
- Not network-related

---

### `unlink()`
**Purpose**: Delete file name from filesystem

**Signature**:
```c
int unlink(const char *pathname);
```

**Context**:
- Cleanup operation
- Removes temporary files

---

## 9. Time and Resource Management Syscalls

### `clock_gettime()`
**Purpose**: Get high-resolution time

**Signature**:
```c
int clock_gettime(clockid_t clockid, struct timespec *tp);
```

**Context**:
- Used for performance measurements
- Minimal overhead (often VDSO-accelerated)

---

### `clock_nanosleep()`
**Purpose**: High-resolution sleep

**Signature**:
```c
int clock_nanosleep(clockid_t clockid, int flags,
                    const struct timespec *request,
                    struct timespec *remain);
```

**Context**:
- Used for precise timing delays
- sockperf uses for pacing

---

### `getrusage()`
**Purpose**: Get resource usage statistics

**Signature**:
```c
int getrusage(int who, struct rusage *usage);
```

**Context**:
- Retrieves CPU time, memory usage, etc.
- Called at end of benchmark for reporting

---

### `setitimer()`
**Purpose**: Set interval timer

**Signature**:
```c
int setitimer(int which, const struct itimerval *new_value,
              struct itimerval *old_value);
```

**Context**:
- Used for timeout handling
- sockperf uses for benchmark duration control

---

## 10. Miscellaneous Syscalls

### `newfstat()`
**Purpose**: Get file status (modern version)

**Signature**:
```c
int fstat(int fd, struct stat *statbuf);
```

**Context**:
- Retrieves file metadata
- Minimal usage (2 calls in baremetal)

---

### `getpid()` / `gettid()`
**Purpose**: Get process ID / thread ID

**Signature**:
```c
pid_t getpid(void);
pid_t gettid(void);
```

**Context**:
- Used for logging and debugging
- VDSO-accelerated (no actual syscall overhead)

---

### `tgkill()`
**Purpose**: Send signal to specific thread

**Signature**:
```c
int tgkill(int tgid, int tid, int sig);
```

**Context**:
- Thread-specific signal delivery
- Used for graceful shutdown

---

## Summary: Syscall Impact on Performance

### Most Performance-Critical Syscalls:

1. **`ioctl(KVM_RUN)`**: Each call = VM exit (expensive)
   - **Impact**: 724K in KVM-KVM throughput vs 84K in vhost
   - **Mitigation**: Use vhost-net for kernel-space packet processing

2. **`read()/write()` vs `readv()/writev()`**:
   - **Impact**: Vectorized I/O reduces syscall count by 50%+
   - **Example**: BM uses 1.9M reads; VMs use fewer readv calls

3. **`pselect6()` vs `epoll_pwait()` vs `ppoll()`**:
   - **Impact**: Event notification overhead
   - **pselect6**: O(n) complexity, used by baremetal
   - **epoll_pwait**: O(1), better scalability (MicroVM)
   - **ppoll**: Used by QEMU

4. **`recvfrom()/sendto()`**: Most efficient for ping-pong
   - **Impact**: 2 syscalls per round-trip (optimal)
   - **Used by**: sockperf latency tests

### Syscall Reduction Strategies:

1. **vhost-net**: Moves packet processing to kernel
   - Reduces ioctl calls by 95%+
   - Reduces overall syscalls dramatically

2. **Vectorized I/O**: Use readv/writev instead of read/write
   - Batch multiple buffers per syscall
   - Reduces context switches

3. **Efficient event loops**: epoll better than select/poll
   - Scales to thousands of connections
   - Lower CPU overhead per event

4. **Zero-copy techniques**: splice, sendfile (not observed in tests)
   - Avoid copying data through userspace
   - Direct kernel→kernel transfers

---

## Performance Correlation Analysis

### Baremetal (23.5 Gbits/sec, 32.9 μs latency):
- **Syscall pattern**: Simple read/write with pselect6
- **Total**: ~3.79M syscalls for throughput test
- **Efficiency**: Direct hardware access, no VM exits

### KVM with vhost (13.2 Gbits/sec, 48.4 μs latency):
- **Syscall pattern**: Minimal ioctl, writev-based
- **Total**: ~98K syscalls for throughput test (95% reduction!)
- **Efficiency**: Kernel-space packet processing bypasses VM exits

### MicroVM (10.6 Gbits/sec, 88.6 μs latency):
- **Syscall pattern**: Heavy epoll_pwait + readv/writev + ioctl
- **Total**: ~1.31M syscalls for throughput test
- **Challenge**: Each packet requires VM exit (ioctl)

### Standard KVM (9.60 Gbits/sec, 65.3 μs latency):
- **Syscall pattern**: Very high ioctl count (724K)
- **Total**: ~1.91M syscalls for throughput test
- **Bottleneck**: Excessive VM exits for packet processing

**Conclusion**: Syscall count and VM exit frequency directly correlate with network performance. vhost-net's kernel-space processing eliminates the per-packet syscall overhead that limits other virtualized configurations.

