# TAS
## Echoserver test
- `max_bytes` sets a fixed message size. The server waits for exactly this many bytes, processes the message, and sends back the same max_bytes. Can't go beyond 4096 bytes (throughput 0)

### Host-Host

| FP Cores Max | Server Threads | Max Flows | Max Bytes | Client Threads | Total (mbps) | 50p (us) | 90p (us) | 99.9p (us) | 99.99p (us) | Flows | Notes |
|--------------|----------------|-----------|-----------|----------------|--------------|----------|----------|------------|-------------|-------|-------|
| 2 | 1 | 8192 | 1 | 1 | 30.49 | 8703 | 10814 | 19736 | 123394 | 8 | |
| 2 | 1 | 4096 | 1024 | 1 | 766.73 | 54 | 190 | 3475 | 239976 | 8 | |
| 2 | 4 | 4096 | 1024 | 4 | 4,815.56 | 177 | 290 | 1507 | 177944 | 32 | |
| 2 | 8 | 4096 | 1024 | 8 | 5,759.68 | 174 | 305 | 693 | 1737 | 60 | |
| 2 | 8 | 128 | 1024 | 8 | 5,077.09 | 215 | 369 | 1526 | 3618 | 60 | |
| 2 | 8 | 4096 | 4096 | 8 | 5,898.04 | 300 | 443 | 634 | 3266 | 64 | |
| 4 | 8 | 4096 | 4096 | 8 | 853.06 | 65 | 168 | 192017 | -1 | 64 | constant FP scale up/down ping-pong |
| 4 | 16 | 4096 | 4096 | 16 | 2,661.82 | 41 | 88 | 1270 | 203737 | 126 | constant FP scale up/down ping-pong |

# No TAS
## Echoserver test

### Host-Host

| FP Cores Max | Server Threads | Max Flows | Max Bytes | Client Threads | Total (mbps) | 50p (us) | 90p (us) | 99.9p (us) | 99.99p (us) | Flows |
|--------------|----------------|-----------|-----------|----------------|--------------|----------|----------|------------|-------------|-------|
| x | 8 | 4096 | 4096 | 8 | 514.24 | 2211 | 3701 | 5741 | 7447 | 64 |
| x | 16 | 4096 | 4096 | 16 | 700.74 | 3002 | 5519 | 8668 | 11902 | 128 |

### CH VM-VM, diff host (1vcpu, 512MB)

| FP Cores Max | Server Threads | Max Flows | Max Bytes | Client Threads | Total (mbps) | 50p (us) | 90p (us) | 99.9p (us) | 99.99p (us) | Flows | Server vcpu idle (%) | Server mem usage (MB) | Client vcpu idle (%) | Client mem usage (MB) |
|--------------|----------------|-----------|-----------|----------------|--------------|----------|----------|------------|-------------|-------|-------------------|------------------|-------------------|------------------|
| x | 8 | 4096 | 4096 | 8 | 176.50 | 10343 | 12907 | 22796 | 34364 | 64 | 44 | 246 | 0 | 124 |
| x | 16 | 4096 | 4096 | 16 | 179.87 | 4830 | 6421 | 10571 | 13137 | 32 | 38 | 375 | 0 | 125 |