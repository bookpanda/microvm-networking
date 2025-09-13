package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"sync"
	"syscall"
	"time"

	"github.com/firecracker-microvm/firecracker-go-sdk"
	"github.com/firecracker-microvm/firecracker-go-sdk/client/models"
	"github.com/google/uuid"
)

type SimplifiedVM struct {
	machine *firecracker.Machine
	socket  string
	stdout  chan string
	stderr  chan string
	vmID    string
	tapName string
}

// cleanupExistingNetworking removes any existing tap interfaces and bridge
func cleanupExistingNetworking(numVMs int) error {
	log.Printf("Cleaning up any existing network interfaces...")

	// Remove existing tap interfaces (try to remove more than needed to be safe)
	for i := 0; i < numVMs+5; i++ {
		tapName := fmt.Sprintf("tap%d", i)
		cmd := exec.Command("sudo", "ip", "link", "delete", tapName)
		if err := cmd.Run(); err != nil {
			// Interface might not exist, that's okay
			log.Printf("Tap interface %s cleanup: %v (might not exist)", tapName, err)
		}
	}

	// Remove bridge
	cmd := exec.Command("sudo", "ip", "link", "delete", "br0")
	if err := cmd.Run(); err != nil {
		log.Printf("Bridge cleanup: %v (might not exist)", err)
	}

	// Wait a moment for cleanup to complete
	time.Sleep(1 * time.Second)

	log.Println("Existing network cleanup completed")
	return nil
}

// setupNetworking creates the bridge and tap interfaces needed for the VMs
func setupNetworking(numVMs int) error {
	log.Printf("Setting up networking for %d VMs...", numVMs)

	// Create bridge
	cmd := exec.Command("sudo", "ip", "link", "add", "name", "br0", "type", "bridge")
	if err := cmd.Run(); err != nil {
		// Bridge might already exist, that's okay
		log.Printf("Bridge creation: %v (might already exist)", err)
	}

	cmd = exec.Command("sudo", "ip", "link", "set", "br0", "up")
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("failed to bring up bridge: %v", err)
	}

	// Create tap interfaces for each VM
	for i := 0; i < numVMs; i++ {
		tapName := fmt.Sprintf("tap%d", i)

		// Create tap interface
		cmd = exec.Command("sudo", "ip", "tuntap", "add", "dev", tapName, "mode", "tap", "user", os.Getenv("USER"))
		if err := cmd.Run(); err != nil {
			return fmt.Errorf("failed to create tap interface %s: %v", tapName, err)
		}

		// Add tap to bridge
		cmd = exec.Command("sudo", "ip", "link", "set", tapName, "master", "br0")
		if err := cmd.Run(); err != nil {
			return fmt.Errorf("failed to add %s to bridge: %v", tapName, err)
		}

		// Bring up tap interface
		cmd = exec.Command("sudo", "ip", "link", "set", tapName, "up")
		if err := cmd.Run(); err != nil {
			return fmt.Errorf("failed to bring up %s: %v", tapName, err)
		}

		log.Printf("Created and configured tap interface: %s", tapName)
	}

	// Configure bridge IP (only if not already configured)
	cmd = exec.Command("ip", "addr", "show", "br0")
	if err := cmd.Run(); err != nil {
		// Bridge doesn't have IP, add one
		cmd = exec.Command("sudo", "ip", "addr", "add", "192.168.100.254/24", "dev", "br0")
		if err := cmd.Run(); err != nil {
			log.Printf("Failed to add IP to bridge (might already exist): %v", err)
		}
	}

	// Set up iptables rules for forwarding
	setupIptables()

	log.Println("Networking setup completed successfully")
	return nil
}

// setupIptables configures iptables rules for VM networking
func setupIptables() {
	commands := [][]string{
		{"sudo", "sh", "-c", "echo 1 > /proc/sys/net/ipv4/ip_forward"},
		{"sudo", "iptables", "-I", "INPUT", "-i", "br0", "-p", "udp", "-j", "ACCEPT"},
		{"sudo", "iptables", "-I", "INPUT", "-i", "br0", "-p", "tcp", "-j", "ACCEPT"},
		{"sudo", "iptables", "-I", "FORWARD", "-i", "br0", "-p", "udp", "-j", "ACCEPT"},
		{"sudo", "iptables", "-I", "FORWARD", "-i", "br0", "-p", "tcp", "-j", "ACCEPT"},
		{"sudo", "iptables", "-I", "FORWARD", "1", "-i", "br0", "-o", "br0", "-j", "ACCEPT"},
	}

	for _, cmd := range commands {
		if err := exec.Command(cmd[0], cmd[1:]...).Run(); err != nil {
			log.Printf("iptables command failed (might already exist): %v", err)
		}
	}
}

// cleanupNetworking removes the tap interfaces and bridge
func cleanupNetworking(numVMs int) error {
	log.Printf("Cleaning up networking for %d VMs...", numVMs)

	// Remove tap interfaces
	for i := 0; i < numVMs; i++ {
		tapName := fmt.Sprintf("tap%d", i)
		cmd := exec.Command("sudo", "ip", "link", "delete", tapName)
		if err := cmd.Run(); err != nil {
			log.Printf("Failed to delete tap interface %s: %v", tapName, err)
		}
	}

	// Remove bridge
	cmd := exec.Command("sudo", "ip", "link", "delete", "br0")
	if err := cmd.Run(); err != nil {
		log.Printf("Failed to delete bridge: %v", err)
	}

	log.Println("Networking cleanup completed")
	return nil
}

func createVM(ctx context.Context, kernelPath, rootfsPath string, vmIndex int) (*SimplifiedVM, error) {
	vmID := uuid.New().String()
	socketPath := filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.sock", vmID))

	// Create channels for console output
	stdout := make(chan string, 100)
	stderr := make(chan string, 100)

	// Create unique MAC address for each VM
	macAddr := fmt.Sprintf("AA:FC:00:00:00:%02X", vmIndex+1)
	// Create unique tap interface name for each VM
	tapName := fmt.Sprintf("tap%d", vmIndex)

	cfg := firecracker.Config{
		SocketPath:      socketPath,
		KernelImagePath: kernelPath,
		KernelArgs:      "console=ttyS0 reboot=k panic=1 init=/bin/sh root=/dev/vda rw",
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
		machine: machine,
		socket:  socketPath,
		stdout:  stdout,
		stderr:  stderr,
		vmID:    vmID,
		tapName: tapName,
	}, nil
}

func (vm *SimplifiedVM) start(ctx context.Context) error {
	if err := vm.machine.Start(ctx); err != nil {
		return fmt.Errorf("failed to start machine: %v", err)
	}

	go func() {
		for {
			select {
			case line := <-vm.stdout:
				if line != "" {
					vm.stdout <- line
				}
			case line := <-vm.stderr:
				if line != "" {
					vm.stderr <- line
				}
			case <-ctx.Done():
				close(vm.stdout)
				close(vm.stderr)
				return
			}
		}
	}()

	return nil
}

func (vm *SimplifiedVM) stop(ctx context.Context) error {
	if err := vm.machine.Shutdown(ctx); err != nil {
		return fmt.Errorf("failed to stop machine: %v", err)
	}

	if err := os.Remove(vm.socket); err != nil {
		log.Printf("failed to remove socket file: %v", err)
	}
	return nil
}

func main() {
	// Command line flags
	numVMs := flag.Int("vms", 1, "Number of VMs to create")
	kernelPath := flag.String("kernel", "/tmp/vmlinux-5.10.223-no-acpi", "Path to kernel image")
	rootfsPath := flag.String("rootfs", "/tmp/debian-rootfs.ext4", "Path to rootfs image")
	flag.Parse()

	log.Printf("Starting %d VMs...", *numVMs)

	pidFile := "/tmp/firecracker.pid"

	// Save PID
	pid := os.Getpid()
	if err := os.WriteFile(pidFile, []byte(fmt.Sprintf("%d", pid)), 0644); err != nil {
		log.Fatalf("Failed to write PID file: %v", err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// Clean up any existing network interfaces
	if err := cleanupExistingNetworking(*numVMs); err != nil {
		log.Fatalf("Failed to clean up existing network interfaces: %v", err)
	}

	// Set up networking first
	if err := setupNetworking(*numVMs); err != nil {
		log.Fatalf("Failed to set up networking: %v", err)
	}

	// Create slice to hold all VMs
	vms := make([]*SimplifiedVM, *numVMs)
	var wg sync.WaitGroup

	// Create all VMs
	for i := 0; i < *numVMs; i++ {
		vm, err := createVM(ctx, *kernelPath, *rootfsPath, i)
		if err != nil {
			log.Fatalf("Failed to create VM %d: %v", i, err)
		}
		vms[i] = vm

		// Save socket path for each VM
		socketFile := fmt.Sprintf("/tmp/firecracker-%d.sock", i)
		if err := os.WriteFile(socketFile, []byte(vm.socket), 0644); err != nil {
			log.Fatalf("Failed to write socket file for VM %d: %v", i, err)
		}
	}

	// Start all VMs
	for i, vm := range vms {
		if err := vm.start(ctx); err != nil {
			log.Fatalf("Failed to start VM %d: %v", i, err)
		}
		log.Printf("VM %d started successfully. Socket: %s", i, vm.socket)
	}

	// Set up signal handling
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Start goroutines for VM output handling
	for i, vm := range vms {
		wg.Add(1)
		go func(vmIndex int, vm *SimplifiedVM) {
			defer wg.Done()
			for {
				select {
				case line := <-vm.stdout:
					fmt.Printf("VM %d stdout: %s\n", vmIndex, line)
				case line := <-vm.stderr:
					fmt.Printf("VM %d stderr: %s\n", vmIndex, line)
				case <-ctx.Done():
					return
				}
			}
		}(i, vm)
	}

	log.Printf("All %d VMs started successfully. PID: %d", *numVMs, pid)
	log.Println("To stop the VMs, run: kill $(cat /tmp/firecracker.pid)")
	log.Println("VM networking setup:")
	log.Println("  Bridge: br0 (192.168.100.254/24)")
	for i := 0; i < *numVMs; i++ {
		log.Printf("  VM %d: tap%d, MAC: AA:FC:00:00:00:%02X", i, i, i+1)
	}

	// Wait for shutdown signal
	<-sigChan
	log.Println("Shutting down VMs...")

	// Cleanup files
	os.Remove(pidFile)
	for i := 0; i < *numVMs; i++ {
		socketFile := fmt.Sprintf("/tmp/firecracker-%d.sock", i)
		os.Remove(socketFile)
	}

	// Stop all VMs concurrently
	stopWg := sync.WaitGroup{}
	for i, vm := range vms {
		stopWg.Add(1)
		go func(vmIndex int, vm *SimplifiedVM) {
			defer stopWg.Done()
			if err := vm.stop(ctx); err != nil {
				log.Printf("Failed to stop VM %d: %v", vmIndex, err)
			}
		}(i, vm)
	}
	stopWg.Wait()

	// Clean up networking
	cleanupNetworking(*numVMs)

	log.Printf("All %d VMs stopped successfully", *numVMs)
}
