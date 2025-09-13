package vm

import (
	"context"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"github.com/firecracker-microvm/firecracker-go-sdk"
	"github.com/firecracker-microvm/firecracker-go-sdk/client/models"
	"github.com/google/uuid"
)

type SimplifiedVM struct {
	Machine *firecracker.Machine
	Socket  string
	Stdout  chan string
	Stderr  chan string
	VMID    string
	TapName string
}

func (v *SimplifiedVM) Start(ctx context.Context) error {
	if err := v.Machine.Start(ctx); err != nil {
		return fmt.Errorf("failed to start machine: %v", err)
	}

	go func() {
		for {
			select {
			case line := <-v.Stdout:
				if line != "" {
					v.Stdout <- line
				}
			case line := <-v.Stderr:
				if line != "" {
					v.Stderr <- line
				}
			case <-ctx.Done():
				close(v.Stdout)
				close(v.Stderr)
				return
			}
		}
	}()

	return nil
}

func (v *SimplifiedVM) Stop(ctx context.Context) error {
	if err := v.Machine.Shutdown(ctx); err != nil {
		return fmt.Errorf("failed to stop machine: %v", err)
	}

	if err := os.Remove(v.Socket); err != nil {
		log.Printf("failed to remove socket file: %v", err)
	}
	return nil
}

func CreateVM(ctx context.Context, kernelPath, rootfsPath string, vmIndex int) (*SimplifiedVM, error) {
	vmID := uuid.New().String()
	socketPath := filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.sock", vmID))

	stdout := make(chan string, 100)
	stderr := make(chan string, 100)

	macAddr := fmt.Sprintf("AA:FC:00:00:00:%02X", vmIndex+1)
	tapName := fmt.Sprintf("tap%d", vmIndex)

	cfg := firecracker.Config{
		SocketPath:      socketPath,
		KernelImagePath: kernelPath,
		KernelArgs:      "console=ttyS0 noapic reboot=k panic=1 pci=off rw",
		Drives: []models.Drive{
			{
				DriveID:      firecracker.String("1"),
				PathOnHost:   firecracker.String(rootfsPath),
				IsRootDevice: firecracker.Bool(true),
				IsReadOnly:   firecracker.Bool(false),
			},
		},
		NetworkInterfaces: []firecracker.NetworkInterface{
			{
				StaticConfiguration: &firecracker.StaticNetworkConfiguration{
					MacAddress:  macAddr,
					HostDevName: tapName,
				},
			},
		},
		MachineCfg: models.MachineConfiguration{
			VcpuCount:  firecracker.Int64(1),
			MemSizeMib: firecracker.Int64(128),
		},
		ForwardSignals: []os.Signal{},
		LogLevel:       "Debug",
		LogPath:        filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.log", vmID)),
		MetricsPath:    filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s-metrics", vmID)),
	}

	stdoutFile, err := os.Create(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.stdout", vmID)))
	if err != nil {
		return nil, fmt.Errorf("failed to create stdout file: %v", err)
	}

	stderrFile, err := os.Create(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.stderr", vmID)))
	if err != nil {
		return nil, fmt.Errorf("failed to create stderr file: %v", err)
	}

	cmd := firecracker.VMCommandBuilder{}.
		WithBin("firecracker").
		WithSocketPath(socketPath).
		WithStdin(os.Stdin).
		WithStdout(stdoutFile).
		WithStderr(stderrFile).
		Build(ctx)

	machine, err := firecracker.NewMachine(ctx, cfg, firecracker.WithProcessRunner(cmd))
	if err != nil {
		return nil, fmt.Errorf("failed to create machine: %v", err)
	}

	return &SimplifiedVM{
		Machine: machine,
		Socket:  socketPath,
		Stdout:  stdout,
		Stderr:  stderr,
		VMID:    vmID,
		TapName: tapName,
	}, nil
}
