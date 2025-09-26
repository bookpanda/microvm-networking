```bash
# kill
sudo fuser -k 5000/vsock
pkill -f "VSOCK-LISTEN:5000"


socat -v - VSOCK-CONNECT:2:5000
```