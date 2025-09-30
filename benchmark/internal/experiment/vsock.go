package experiment

import (
	"bufio"
	"context"
	"fmt"
	"log"
	"net"
	"os"
	"path/filepath"
)

func streamCommandVsock(ctx context.Context, sockPath string, port uint32, cmd string, outputWriter func(string)) error {
	conn, err := net.Dial("unix", sockPath)
	if err != nil {
		return fmt.Errorf("failed to connect to %s: %v", sockPath, err)
	}
	defer conn.Close()

	_, err = fmt.Fprintf(conn, "CONNECT %d\n", port)
	if err != nil {
		return fmt.Errorf("failed to send CONNECT: %v", err)
	}

	// send the actual command
	_, err = fmt.Fprintf(conn, "%s\n", cmd)
	if err != nil {
		return fmt.Errorf("failed to send command: %v", err)
	}

	reader := bufio.NewReader(conn)
	for {
		select {
		case <-ctx.Done():
			return ctx.Err()
		default:
			line, err := reader.ReadString('\n')
			if len(line) > 0 {
				outputWriter(line)
			}
			if err != nil {
				// EOF or connection closed
				return nil
			}
		}
	}
}

func (e *VMVMExperiment) captureCommandOutputVsock(ctx context.Context, sockPath string, port uint32, command, logFileName string, wait bool) error {
	logPath := filepath.Join(e.logDir, logFileName)

	logFile, err := os.Create(logPath)
	if err != nil {
		return fmt.Errorf("failed to create log file %s: %v", logPath, err)
	}

	if wait {
		e.wg.Add(1)
	}

	go func() {
		if wait {
			defer e.wg.Done()
		}
		defer logFile.Close()

		// Create a writer function that writes to the log file
		outputWriter := func(line string) {
			logFile.WriteString(fmt.Sprintf("[OUTPUT] %s", line))
		}

		if wait {
			// For commands that should complete (like clients)
			err := streamCommandVsock(ctx, sockPath, port, command, outputWriter)
			if err != nil {
				logFile.WriteString(fmt.Sprintf("Error: %v\n", err))
				log.Printf("VM %s %d: command failed: %v", sockPath, port, err)
			} else {
				log.Printf("VM %s %d: command completed, output saved to %s", sockPath, port, logPath)
			}
		} else {
			// For long-running commands (like servers)
			err := streamCommandVsock(ctx, sockPath, port, command, outputWriter)
			if err != nil && err != context.Canceled {
				logFile.WriteString(fmt.Sprintf("Error: %v\n", err))
				log.Printf("VM %s %d: command failed: %v", sockPath, port, err)
			} else {
				log.Printf("VM %s %d: server stopped, logs saved to %s", sockPath, port, logPath)
			}
		}
	}()

	return nil
}
