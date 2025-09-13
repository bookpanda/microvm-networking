package experiment

import (
	"bufio"
	"context"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sync"
	"time"

	"github.com/bookpanda/microvm-networking/benchmark/internal/logging"
	"github.com/bookpanda/microvm-networking/benchmark/internal/vm"
)

type SCPair struct {
	Server *vm.SimplifiedVM
	Client *vm.SimplifiedVM
}

type VMVMExperiment struct {
	SCPairs    []*SCPair
	LogDir     string
	cancelFunc context.CancelFunc
	wg         sync.WaitGroup
}

func NewVMVMExperiment(manager *vm.Manager) (*VMVMExperiment, error) {
	logDir := "./vm-experiment-logs"
	if err := logging.GetEmptyLogDir(logDir); err != nil {
		return nil, fmt.Errorf("failed to create log directory: %v", err)
	}

	experiment := &VMVMExperiment{
		SCPairs: make([]*SCPair, 0),
		LogDir:  logDir,
	}

	pair := &SCPair{}
	for i, vm := range manager.GetVMs() {
		if i%2 == 0 {
			pair.Server = vm
		} else {
			pair.Client = vm
			experiment.SCPairs = append(experiment.SCPairs, pair)
			pair = &SCPair{}
		}
	}

	return experiment, nil
}

func RunVMVMBenchmark(ctx context.Context, manager *vm.Manager) error {
	experiment, err := NewVMVMExperiment(manager)
	if err != nil {
		return fmt.Errorf("failed to create experiment: %v", err)
	}

	// Set up cancellation context for log capture
	logCtx, cancel := context.WithCancel(ctx)
	experiment.cancelFunc = cancel
	defer experiment.Cleanup()

	log.Printf("Experiment logs will be saved to: %s", experiment.LogDir)

	log.Println("Preparing servers...")
	if err := experiment.prepareServers(logCtx); err != nil {
		return fmt.Errorf("failed to prepare servers: %v", err)
	}
	log.Println("Tracking syscalls...")
	if err := experiment.trackSyscalls(); err != nil {
		return fmt.Errorf("failed to track syscalls: %v", err)
	}
	log.Println("Starting clients...")
	if err := experiment.startClients(logCtx); err != nil {
		return fmt.Errorf("failed to start clients: %v", err)
	}

	log.Println("Waiting for log capture to complete...")
	experiment.wg.Wait()

	return nil
}

func (e *VMVMExperiment) captureCommandOutput(ctx context.Context, vmIP, command, logFileName string) error {
	logPath := filepath.Join(e.LogDir, logFileName)
	logFile, err := os.Create(logPath)
	if err != nil {
		return fmt.Errorf("failed to create log file %s: %v", logPath, err)
	}

	e.wg.Add(1)
	go func() {
		defer e.wg.Done()
		defer logFile.Close()

		cmd := exec.CommandContext(ctx, "sshpass", "-p", "root", "ssh",
			"-o", "ConnectTimeout=5", "-o", "StrictHostKeyChecking=no",
			"root@"+vmIP, command)

		stdout, err := cmd.StdoutPipe()
		if err != nil {
			log.Printf("Failed to create stdout pipe for %s: %v", vmIP, err)
			return
		}

		stderr, err := cmd.StderrPipe()
		if err != nil {
			log.Printf("Failed to create stderr pipe for %s: %v", vmIP, err)
			return
		}

		if err := cmd.Start(); err != nil {
			log.Printf("Failed to start command on %s: %v", vmIP, err)
			return
		}

		go func() {
			scanner := bufio.NewScanner(stdout)
			for scanner.Scan() {
				logFile.WriteString(fmt.Sprintf("[STDOUT] %s\n", scanner.Text()))
			}
		}()

		go func() {
			scanner := bufio.NewScanner(stderr)
			for scanner.Scan() {
				logFile.WriteString(fmt.Sprintf("[STDERR] %s\n", scanner.Text()))
			}
		}()

		cmd.Wait()
		log.Printf("Command completed on %s, output saved to %s", vmIP, logPath)
	}()

	return nil
}

func (e *VMVMExperiment) prepareServers(ctx context.Context) error {
	for i, pair := range e.SCPairs {
		serverLogFile := fmt.Sprintf("server-%s.log", pair.Server.IP)

		log.Printf("Starting iperf3 server on VM %s", pair.Server.IP)
		command := "mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -s"
		if err := e.captureCommandOutput(ctx, pair.Server.IP, command, serverLogFile); err != nil {
			return fmt.Errorf("failed to start iperf3 server on %s: %v", pair.Server.IP, err)
		}

		log.Printf("Started iperf3 server %d on VM %s (logs: %s)", i, pair.Server.IP, serverLogFile)
	}

	log.Println("Waiting for servers to start...")
	time.Sleep(3 * time.Second)

	return nil
}

func (e *VMVMExperiment) trackSyscalls() error {
	for _, pair := range e.SCPairs {
		serverPID, err := pair.Server.Machine.PID()
		if err != nil {
			return fmt.Errorf("failed to get server PID: %v", err)
		}
		clientPID, err := pair.Client.Machine.PID()
		if err != nil {
			return fmt.Errorf("failed to get client PID: %v", err)
		}

		runTraceSyscallsScript(serverPID, fmt.Sprintf("/tmp/server-%s.log", pair.Server.IP))
		runTraceSyscallsScript(clientPID, fmt.Sprintf("/tmp/client-%s.log", pair.Client.IP))
	}
	return nil
}

func (e *VMVMExperiment) startClients(ctx context.Context) error {
	for i, pair := range e.SCPairs {
		clientLogFile := fmt.Sprintf("client-%s-to-%s.log", pair.Client.IP, pair.Server.IP)
		clientCommand := fmt.Sprintf("mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -c %s -t 10 -P 4", pair.Server.IP)

		log.Printf("Starting iperf3 client test from %s to %s", pair.Client.IP, pair.Server.IP)
		if err := e.captureCommandOutput(ctx, pair.Client.IP, clientCommand, clientLogFile); err != nil {
			return fmt.Errorf("failed to start iperf3 client on %s: %v", pair.Client.IP, err)
		}

		log.Printf("Started iperf3 client %d from %s to %s (logs: %s)", i, pair.Client.IP, pair.Server.IP, clientLogFile)
	}

	return nil
}

func runTraceSyscallsScript(pid int, logfile string) {
	cmd := exec.Command("/bin/bash", "./trace_syscalls.sh", fmt.Sprintf("%d", pid), logfile)
	cmd.Stdout = nil
	cmd.Stderr = nil
	err := cmd.Start()
	if err != nil {
		log.Fatalf("Failed to start script: %v", err)
	}

	log.Printf("Tracing script started with PID %d", cmd.Process.Pid)

}

func (e *VMVMExperiment) Cleanup() error {
	log.Println("Cleaning up experiment...")

	// Cancel all log capture goroutines
	if e.cancelFunc != nil {
		e.cancelFunc()
	}

	// Kill any remaining iperf3 processes in the VMs
	for _, pair := range e.SCPairs {
		killCmd := exec.Command("sshpass", "-p", "root", "ssh",
			"-o", "ConnectTimeout=2", "-o", "StrictHostKeyChecking=no",
			"root@"+pair.Server.IP, "pkill iperf3 || true")
		killCmd.Run()

		killCmd = exec.Command("sshpass", "-p", "root", "ssh",
			"-o", "ConnectTimeout=2", "-o", "StrictHostKeyChecking=no",
			"root@"+pair.Client.IP, "pkill iperf3 || true")
		killCmd.Run()
	}

	log.Printf("Experiment logs saved in: %s", e.LogDir)
	return nil
}
