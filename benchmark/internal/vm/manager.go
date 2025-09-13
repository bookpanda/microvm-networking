package vm

import (
	"context"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sync"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	"github.com/bookpanda/microvm-networking/benchmark/internal/network"
)

type Manager struct {
	config *config.Config
	vms    []*SimplifiedVM
}

func NewManager(cfg *config.Config) *Manager {
	return &Manager{
		config: cfg,
		vms:    make([]*SimplifiedVM, cfg.NumVMs),
	}
}

func (m *Manager) Initialize(ctx context.Context) error {
	if err := network.CleanupExisting(m.config.NumVMs); err != nil {
		return fmt.Errorf("failed to clean up existing network interfaces: %v", err)
	}

	if err := network.Setup(m.config.NumVMs); err != nil {
		return fmt.Errorf("failed to set up networking: %v", err)
	}

	for i := 0; i < m.config.NumVMs; i++ {
		vm, err := CreateVM(ctx, m.config.KernelPath, m.config.RootfsPath, i)
		if err != nil {
			return fmt.Errorf("failed to create VM %d: %v", i, err)
		}
		m.vms[i] = vm

		socketFile := fmt.Sprintf("/tmp/firecracker-%d.sock", i)
		if err := os.WriteFile(socketFile, []byte(vm.Socket), 0644); err != nil {
			return fmt.Errorf("failed to write socket file for VM %d: %v", i, err)
		}
	}

	return nil
}

func (m *Manager) Start(ctx context.Context) error {
	for i, vm := range m.vms {
		if err := vm.Start(ctx); err != nil {
			return fmt.Errorf("failed to start VM %d: %v", i, err)
		}
		log.Printf("VM %d started successfully. Socket: %s", i, vm.Socket)
	}
	return nil
}

func (m *Manager) Stop(ctx context.Context) error {
	var wg sync.WaitGroup
	for i, vm := range m.vms {
		wg.Add(1)
		go func(vmIndex int, vm *SimplifiedVM) {
			defer wg.Done()
			if err := vm.Stop(ctx); err != nil {
				log.Printf("Failed to stop VM %d: %v", vmIndex, err)
			}
		}(i, vm)
	}
	wg.Wait()
	return nil
}

func (m *Manager) GetVMs() []*SimplifiedVM {
	return m.vms
}

func (m *Manager) Cleanup() error {
	for _, vm := range m.vms {
		os.RemoveAll(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.stdout", vm.VMID)))
		os.RemoveAll(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.stderr", vm.VMID)))
		os.RemoveAll(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s.log", vm.VMID)))
		os.RemoveAll(filepath.Join(os.TempDir(), fmt.Sprintf("firecracker-%s-metrics", vm.VMID)))
	}

	return network.Cleanup(m.config.NumVMs)
}

func (m *Manager) LogNetworkingInfo() {
	log.Printf("All %d VMs started successfully", m.config.NumVMs)
	log.Println("VM networking setup:")
	log.Println("  Bridge: br0 (192.168.100.254/24)")
	for i := 0; i < m.config.NumVMs; i++ {
		log.Printf("  VM %d: tap%d, MAC: AA:FC:00:00:00:%02X", i, i, i+1)
	}
}
