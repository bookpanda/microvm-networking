package experiment

import (
	"context"
	"fmt"
	"log"
	"os/exec"

	"github.com/bookpanda/microvm-networking/benchmark/internal/vm"
)

type SCPair struct {
	Server *vm.SimplifiedVM
	Client *vm.SimplifiedVM
}

type VMVMExperiment struct {
	SCPairs []*SCPair
}

func NewVMVMExperiment(manager *vm.Manager) *VMVMExperiment {
	experiment := &VMVMExperiment{
		SCPairs: make([]*SCPair, 0),
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

	return experiment
}

func RunVMVMBenchmark(ctx context.Context, manager *vm.Manager) error {
	experiment := NewVMVMExperiment(manager)
	if err := experiment.prepareServers(); err != nil {
		return fmt.Errorf("failed to prepare servers: %v", err)
	}
	if err := experiment.trackSyscalls(); err != nil {
		return fmt.Errorf("failed to track syscalls: %v", err)
	}
	if err := experiment.startClients(); err != nil {
		return fmt.Errorf("failed to start clients: %v", err)
	}

	return nil
}

func (e *VMVMExperiment) prepareServers() error {
	for _, pair := range e.SCPairs {
		cmd := exec.Command("sshpass", "-p", "root", "ssh", "root@"+pair.Server.IP, "iperf3", "-s")
		err := cmd.Run()
		if err != nil {
			return fmt.Errorf("failed to execute commands via SSH: %v", err)
		}
	}
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

		runTraceSyscallsScript(serverPID, fmt.Sprintf("server-%s.log", pair.Server.IP))
		runTraceSyscallsScript(clientPID, fmt.Sprintf("client-%s.log", pair.Client.IP))
	}
	return nil
}

func (e *VMVMExperiment) startClients() error {
	for _, pair := range e.SCPairs {
		cmd := exec.Command("sshpass", "-p", "root", "ssh", "root@"+pair.Client.IP,
			"iperf3", "-c", pair.Server.IP, "-t", "30", "-P", "4")
		err := cmd.Run()
		if err != nil {
			return fmt.Errorf("failed to execute commands via SSH: %v", err)
		}
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
	return nil
}
