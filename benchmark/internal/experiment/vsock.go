package experiment

import (
	"bufio"
	"context"
	"fmt"
	"log"
	"net"
	"os"
	"path/filepath"
	"strings"
)

func sendCommandVsock(sockPath string, port uint32, cmd string) (string, error) {
	conn, err := net.Dial("unix", sockPath)
	if err != nil {
		return "", fmt.Errorf("failed to connect to %s: %v", sockPath, err)
	}
	defer conn.Close()

	_, err = fmt.Fprintf(conn, "CONNECT %d\n", port)
	if err != nil {
		return "", fmt.Errorf("failed to send CONNECT: %v", err)
	}

	// send the actual command
	_, err = fmt.Fprintf(conn, "%s\n", cmd)
	if err != nil {
		return "", fmt.Errorf("failed to send command: %v", err)
	}

	// Send command
	_, err = fmt.Fprintf(conn, "%s\n", cmd)
	if err != nil {
		return "", fmt.Errorf("failed to send command: %v", err)
	}

	reader := bufio.NewReader(conn)
	var output strings.Builder
	for {
		line, err := reader.ReadString('\n')
		if len(line) > 0 {
			output.WriteString(line)
		}
		if err != nil {
			// EOF or connection closed
			break
		}
	}

	return output.String(), nil
}

func (e *VMVMExperiment) captureCommandOutputVsock(ctx context.Context, sockPath string, port uint32, command, logFileName string, wait bool) error {
	logPath := filepath.Join(e.logDir, logFileName)

	if wait {
		e.wg.Add(1)
	}

	go func() {
		if wait {
			defer e.wg.Done()
		}

		output, err := sendCommandVsock(sockPath, port, command)
		if err != nil {
			log.Printf("VM %s %d: command failed: %v", sockPath, port, err)
			output = fmt.Sprintf("Error: %v\n", err)
		}

		if err := os.WriteFile(logPath, []byte(output), 0644); err != nil {
			log.Printf("VM %s %d: failed to write log file %s: %v", sockPath, port, logPath, err)
		} else {
			log.Printf("VM %s %d: command output saved to %s", sockPath, port, logPath)
		}
	}()

	return nil
}
