# TAS
## Echoserver test
- `max_bytes` sets a fixed message size. The server waits for exactly this many bytes, processes the message, and sends back the same max_bytes. Can't go beyond 4096 bytes (throughput 0)
```bash
fp-cores-max, #server thread, max_flows, max_bytes, #client thread
- 2, 1, 8192, 1, 1
total=30.49 mbps  50p=8703 us  90p=10814 us  99.9p=19736 us  99.99p=123394 us  flows=8

- 2, 1, 4096, 1024, 1
total=766.73 mbps  50p=54 us  90p=190 us  99.9p=3475 us  99.99p=239976 us  flows=8

- 2, 4, 4096, 1024, 4
total=4,815.56 mbps  50p=177 us  90p=290 us  99.9p=1507 us  99.99p=177944 us  flows=32

- 2, 8, 4096, 1024, 8
total=5,759.68 mbps  50p=174 us  90p=305 us  99.9p=693 us  99.99p=1737 us  flows=60

- 2, 8, 128, 1024, 8
total=5,077.09 mbps  50p=215 us  90p=369 us  99.9p=1526 us  99.99p=3618 us  flows=60

- 2, 8, 4096, 4096, 8
total=5,898.04 mbps  50p=300 us  90p=443 us  99.9p=634 us  99.99p=3266 us  flows=64

- 4, 8, 4096, 4096, 8 # constant FP scale up/down ping-pong
total=853.06 mbps  50p=65 us  90p=168 us  99.9p=192017 us  99.99p=-1 us  flows=64

- 4, 16, 4096, 4096, 16 # constant FP scale up/down ping-pong
total=2,661.82 mbps  50p=41 us  90p=88 us  99.9p=1270 us  99.99p=203737 us  flows=126

```

# No TAS
## Echoserver test
```bash
fp-cores-max, #server thread, max_flows, max_bytes, #client thread
- x, 16, 4096, 4096, 16
total=700.74 mbps  50p=3002 us  90p=5519 us  99.9p=8668 us  99.99p=11902 us  flows=128
```