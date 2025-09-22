package main

import (
	"context"
	"fmt"
	"log"
	"os"
	"sync"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	_vm "github.com/bookpanda/microvm-networking/benchmark/internal/vm"
)

func main() {
	cfg := config.ParseFlags()
	log.Printf("Starting %d VMs...", cfg.NumVMs)

	// Save PID file
	pidFile := "/tmp/firecracker.pid"
	pid := os.Getpid()
	if err := os.WriteFile(pidFile, []byte(fmt.Sprintf("%d", pid)), 0644); err != nil {
		log.Fatalf("Failed to write PID file: %v", err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	manager := _vm.NewManager(cfg)
	if err := manager.Initialize(ctx); err != nil {
		log.Fatalf("Failed to initialize VMs: %v", err)
	}

	if err := manager.Start(ctx); err != nil {
		log.Fatalf("Failed to start VMs: %v", err)
	}

	var wg sync.WaitGroup
	vms := manager.GetVMs()
	for i, vm := range vms {
		wg.Add(1)
		go func(vmIndex int, vm *_vm.SimplifiedVM) {
			defer wg.Done()
			for {
				select {
				case line := <-vm.Stdout:
					fmt.Printf("VM %d stdout: %s\n", vmIndex, line)
				case line := <-vm.Stderr:
					fmt.Printf("VM %d stderr: %s\n", vmIndex, line)
				case <-ctx.Done():
					return
				}
			}
		}(i, vm)
	}

	manager.LogNetworkingInfo()

	// time.Sleep(5 * time.Second)
	// if err := experiment.RunVMVMBenchmark(ctx, manager); err != nil {
	// 	log.Fatalf("Failed to run VM VM benchmark: %v", err)
	// }

	// log.Println("Shutting down VMs...")

	// // cleanup files
	// os.Remove(pidFile)
	// for i := 0; i < cfg.NumVMs; i++ {
	// 	socketFile := fmt.Sprintf("/tmp/firecracker-%d.sock", i)
	// 	os.Remove(socketFile)
	// }

	// // stop all VMs and cleanup
	// if err := manager.Stop(ctx); err != nil {
	// 	log.Printf("Failed to stop VMs: %v", err)
	// }

	// if err := manager.Cleanup(); err != nil {
	// 	log.Printf("Failed to cleanup networking: %v", err)
	// }

	// log.Printf("All %d VMs stopped successfully", cfg.NumVMs)
}
