package experiment

import (
	"bufio"
	"context"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	"github.com/bookpanda/microvm-networking/benchmark/internal/filesystem"
	"github.com/bookpanda/microvm-networking/benchmark/internal/vm"
)

type SCPair struct {
	Server *vm.SimplifiedVM
	Client *vm.SimplifiedVM
}

type VMVMExperiment struct {
	test          config.Test
	SCPairs       []*SCPair
	logDir        string
	syscallsDir   string
	cancelFunc    context.CancelFunc
	wg            sync.WaitGroup
	traceProcs    []*exec.Cmd
	traceProcsMux sync.Mutex
}

func NewVMVMExperiment(manager *vm.Manager) (*VMVMExperiment, error) {
	logDir := "./vm-experiment-logs"
	if err := filesystem.GetEmptyLogDir(logDir); err != nil {
		return nil, fmt.Errorf("failed to create log directory: %v", err)
	}

	syscallsDir := "./vm-syscalls"
	if err := filesystem.GetEmptyLogDir(syscallsDir); err != nil {
		return nil, fmt.Errorf("failed to create syscalls directory: %v", err)
	}

	experiment := &VMVMExperiment{
		test:        manager.GetConfig().Test,
		SCPairs:     make([]*SCPair, 0),
		logDir:      logDir,
		syscallsDir: syscallsDir,
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

	log.Printf("Experiment logs will be saved to: %s", experiment.logDir)

	log.Println("Preparing servers...")
	if err := experiment.prepareServers(logCtx); err != nil {
		return fmt.Errorf("failed to prepare servers: %v", err)
	}
	log.Println("Tracking syscalls...")
	if err := experiment.trackSyscalls(logCtx); err != nil {
		return fmt.Errorf("failed to track syscalls: %v", err)
	}
	time.Sleep(5 * time.Second)
	log.Println("Starting clients...")
	if err := experiment.startClients(logCtx); err != nil {
		return fmt.Errorf("failed to start clients: %v", err)
	}

	log.Println("Waiting for log capture to complete...")
	experiment.wg.Wait()

	return nil
}

func (e *VMVMExperiment) captureCommandOutput(ctx context.Context, vmIP, command, logFileName string, wait bool, isTrace bool) error {
	var logPath string
	if isTrace {
		logPath = filepath.Join(e.syscallsDir, logFileName)
	} else {
		logPath = filepath.Join(e.logDir, logFileName)
	}

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

		var cmd *exec.Cmd
		if isTrace {
			// Split the command properly for exec
			args := strings.Fields(command)
			if len(args) == 0 {
				log.Printf("Empty trace command for %s", vmIP)
				return
			}
			cmd = exec.CommandContext(ctx, args[0], args[1:]...)
		} else {
			cmd = exec.CommandContext(ctx, "sshpass", "-p", "root", "ssh",
				"-o", "ConnectTimeout=5", "-o", "StrictHostKeyChecking=no",
				"root@"+vmIP, command)
		}

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

		// Track trace processes for proper cleanup
		if isTrace {
			e.traceProcsMux.Lock()
			e.traceProcs = append(e.traceProcs, cmd)
			e.traceProcsMux.Unlock()
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

		if wait {
			cmd.Wait()
			log.Printf("Command completed on %s, output saved to %s", vmIP, logPath)
		} else {
			// for servers: stop when context is canceled
			<-ctx.Done()
			cmd.Process.Kill() // kill ONLY server process
			cmd.Wait()         // wait for stdout/stderr to be closed
			log.Printf("Server on %s stopped, logs saved to %s", vmIP, logPath)
		}
	}()

	return nil
}

func (e *VMVMExperiment) prepareServers(ctx context.Context) error {
	for i, pair := range e.SCPairs {
		serverLogFile := fmt.Sprintf("server-%s.log", pair.Server.IP)

		log.Printf("Starting server on VM %s", pair.Server.IP)
		var command string
		switch e.test {
		case config.Throughput:
			command = "mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -s"
		case config.Latency:
			command = "mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp sockperf server -i " + pair.Server.IP
		default:
			return fmt.Errorf("invalid test: %s", e.test)
		}

		if err := e.captureCommandOutput(ctx, pair.Server.IP, command, serverLogFile, false, false); err != nil {
			return fmt.Errorf("failed to start server on %s: %v", pair.Server.IP, err)
		}

		log.Printf("Started server %d on VM %s (logs: %s)", i, pair.Server.IP, serverLogFile)
	}

	log.Println("Waiting for servers to start...")
	time.Sleep(3 * time.Second)

	return nil
}

func (e *VMVMExperiment) trackSyscalls(ctx context.Context) error {
	tracePath, err := os.Getwd()
	if err != nil {
		return fmt.Errorf("failed to get working directory: %v", err)
	}
	tracePath = filepath.Join(tracePath, "trace_syscalls.sh")

	for _, pair := range e.SCPairs {
		serverPID, err := pair.Server.Machine.PID()
		if err != nil {
			return fmt.Errorf("failed to get server PID: %v", err)
		}
		clientPID, err := pair.Client.Machine.PID()
		if err != nil {
			return fmt.Errorf("failed to get client PID: %v", err)
		}

		serverCommand := fmt.Sprintf("sudo %s %d", tracePath, serverPID)
		serverLogFile := fmt.Sprintf("server-%s.log", pair.Server.IP)
		if err := e.captureCommandOutput(ctx, "", serverCommand, serverLogFile, false, true); err != nil {
			return fmt.Errorf("failed to start server on %s: %v", pair.Server.IP, err)
		}

		clientCommand := fmt.Sprintf("sudo %s %d", tracePath, clientPID)
		clientLogFile := fmt.Sprintf("client-%s.log", pair.Client.IP)
		if err := e.captureCommandOutput(ctx, "", clientCommand, clientLogFile, false, true); err != nil {
			return fmt.Errorf("failed to start client on %s: %v", pair.Client.IP, err)
		}
	}
	return nil
}

func (e *VMVMExperiment) startClients(ctx context.Context) error {
	for i, pair := range e.SCPairs {
		clientLogFile := fmt.Sprintf("client-%s-to-%s.log", pair.Client.IP, pair.Server.IP)

		var command string
		switch e.test {
		case config.Throughput:
			command = fmt.Sprintf("mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -c %s -t 30 -P 4", pair.Server.IP)
		case config.Latency:
			command = fmt.Sprintf("mount -t tmpfs -o size=128M tmpfs /tmp && HOME=/tmp sockperf ping-pong -i %s -m 64 -t 30", pair.Server.IP)
		default:
			return fmt.Errorf("invalid test: %s", e.test)
		}

		log.Printf("Starting client test from %s to %s", pair.Client.IP, pair.Server.IP)
		if err := e.captureCommandOutput(ctx, pair.Client.IP, command, clientLogFile, true, false); err != nil {
			return fmt.Errorf("failed to start client on %s: %v", pair.Client.IP, err)
		}

		log.Printf("Started client %d from %s to %s (logs: %s)", i, pair.Client.IP, pair.Server.IP, clientLogFile)
	}

	return nil
}

func (e *VMVMExperiment) Cleanup() error {
	log.Println("Cleaning up experiment...")

	// Terminate trace processes gracefully
	e.traceProcsMux.Lock()
	if len(e.traceProcs) > 0 {
		log.Printf("Terminating %d bpftrace processes gracefully...", len(e.traceProcs))
		for i, proc := range e.traceProcs {
			if proc.Process != nil {
				log.Printf("Sending SIGTERM to bpftrace process %d (PID: %d)", i, proc.Process.Pid)
				err := proc.Process.Signal(syscall.SIGTERM)
				if err != nil {
					log.Printf("Failed to send SIGTERM to process %d: %v", proc.Process.Pid, err)
				}
			}
		}
		e.traceProcsMux.Unlock()

		// Wait longer for END blocks to execute
		log.Println("Waiting 5 seconds for bpftrace END blocks to execute...")
		time.Sleep(5 * time.Second)

		// Wait for processes to finish
		e.traceProcsMux.Lock()
		for i, proc := range e.traceProcs {
			log.Printf("Waiting for bpftrace process %d to finish...", i)
			err := proc.Wait()
			if err != nil {
				log.Printf("Process %d finished with error: %v", i, err)
			} else {
				log.Printf("Process %d finished successfully", i)
			}
		}
		e.traceProcsMux.Unlock()
	} else {
		e.traceProcsMux.Unlock()
	}

	// Cancel all log capture goroutines
	if e.cancelFunc != nil {
		e.cancelFunc()
	}

	// Kill any remaining processes in the VMs
	for _, pair := range e.SCPairs {
		killCmd := exec.Command("sshpass", "-p", "root", "ssh",
			"-o", "ConnectTimeout=2", "-o", "StrictHostKeyChecking=no",
			"root@"+pair.Server.IP, "pkill iperf3 || pkill sockperf || true")
		killCmd.Run()

		killCmd = exec.Command("sshpass", "-p", "root", "ssh",
			"-o", "ConnectTimeout=2", "-o", "StrictHostKeyChecking=no",
			"root@"+pair.Client.IP, "pkill iperf3 || pkill sockperf || true")
		killCmd.Run()
	}

	log.Printf("Experiment logs saved in: %s", e.logDir)
	return nil
}
