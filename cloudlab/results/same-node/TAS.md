# No TAS
## Echoserver test
```bash
fp-cores-max, #server thread, max_flows, max_bytes, #client thread
- x, 16, 4096, 4096, 16
total=630.09 mbps  50p=3380 us  90p=6097 us  99.9p=10264 us  99.99p=14494 us  flows=128

# client is in CH VM
- x, 1, 4096, 4096, 1
total=161.95 mbps  50p=1046 us  90p=1380 us  99.9p=2813 us  99.99p=4248 us  flows=8

- x, 4, 4096, 4096, 4
total=178.03 mbps  50p=4842 us  90p=6121 us  99.9p=12167 us  99.99p=21953 us  flows=32

- x, 16, 4096, 4096, 16
total=178.59 mbps  50p=22065 us  90p=25509 us  99.9p=41431 us  99.99p=59044 us  flows=128

- x, 4, 4096, 8192, 4
total=174.10 mbps  50p=2821 us  90p=4209 us  99.9p=7825 us  99.99p=17657 us  flows=32
```