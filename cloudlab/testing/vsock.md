```bash
# kill
sudo fuser -k 5000/vsock
pkill -f "VSOCK-LISTEN:5000"

cd ~/code/microvm-networking/cloudlab/testing
./vm-vsock.sh

socat -v - VSOCK-CONNECT:2:5000
```

## Example
```bash
# vm: run server
socat VSOCK-LISTEN:52,fork -

# host: client
socat - UNIX-CONNECT:./vsock-192.168.100.2.sock
CONNECT 52
# now anything sent to ./vsock-192.168.100.2.sock will be sent to the vm
```