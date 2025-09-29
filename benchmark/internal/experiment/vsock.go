package experiment

import (
	"fmt"
	"io"

	"golang.org/x/sys/unix"
)

func sendCommandVsock(cid, port uint32, cmd string) (string, error) {
	fd, err := unix.Socket(unix.AF_VSOCK, unix.SOCK_STREAM, 0)
	if err != nil {
		return "", fmt.Errorf("failed to create vsock: %v", err)
	}
	defer unix.Close(fd)

	addr := &unix.SockaddrVM{
		CID:  cid,  // VM CID
		Port: port, // vsock port server is listening on
	}

	if err := unix.Connect(fd, addr); err != nil {
		return "", fmt.Errorf("failed to connect: %v", err)
	}

	// Send command
	if _, err := unix.Write(fd, []byte(cmd+"\n")); err != nil {
		return "", fmt.Errorf("failed to send command: %v", err)
	}

	// Read output
	buf := make([]byte, 4096)
	var output []byte
	for {
		n, err := unix.Read(fd, buf)
		if n > 0 {
			output = append(output, buf[:n]...)
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			return "", fmt.Errorf("read error: %v", err)
		}
	}

	return string(output), nil
}
