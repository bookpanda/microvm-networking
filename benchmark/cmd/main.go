package main

import (
	"context"
	"fmt"
	"io"
	"log"
	"time"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	filesystemProto "github.com/bookpanda/microvm-networking/benchmark/proto/filesystem/v1"
	networkProto "github.com/bookpanda/microvm-networking/benchmark/proto/network/v1"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func main() {
	cfg := config.ParseFlags()

	nodeConn, err := grpc.NewClient("10.10.1.2:50051", grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		log.Fatalf("Failed to create gRPC client: %v", err)
	}

	vmClient := vmProto.NewVmServiceClient(nodeConn)
	networkClient := networkProto.NewNetworkServiceClient(nodeConn)
	filesystemClient := filesystemProto.NewFileSystemServiceClient(nodeConn)

	ctx := context.Background()

	log.Printf("Cleaning up VM...")
	_, err = vmClient.Cleanup(ctx, &vmProto.CleanupVmRequest{})
	if err != nil {
		log.Fatalf("Failed to cleanup VM: %v", err)
	}

	log.Printf("Cleaning up network...")
	_, err = networkClient.Cleanup(ctx, &networkProto.CleanupNetworkRequest{
		NumVMs: 2,
	})
	if err != nil {
		log.Fatalf("Failed to cleanup network: %v", err)
	}

	log.Printf("Cleaning up filesystem...")
	_, err = filesystemClient.Cleanup(ctx, &filesystemProto.CleanupFileSystemRequest{})
	if err != nil {
		log.Fatalf("Failed to cleanup filesystem: %v", err)
	}

	log.Printf("Setting up network...")
	_, err = networkClient.Setup(ctx, &networkProto.SetupNetworkRequest{
		NumVMs:   2,
		BridgeIP: "192.168.100.1",
	})
	if err != nil {
		log.Fatalf("Failed to setup network: %v", err)
	}

	log.Printf("Starting VM...")
	ips := []string{"192.168.100.2", "192.168.100.3"}
	for _, ip := range ips {
		vmClient.Create(ctx, &vmProto.CreateVmRequest{
			Ip:         ip,
			KernelPath: cfg.KernelPath,
			RootfsPath: cfg.RootfsPath,
		})
	}

	time.Sleep(5 * time.Second)

	log.Printf("Starting server VM...")
	vmClient.SendServerCommand(ctx, &vmProto.SendServerCommandVmRequest{
		Ip:      "192.168.100.2",
		Command: "mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -s",
	})
	log.Printf("Server VM started")

	log.Printf("Starting to track syscalls...")
	vmClient.TrackSyscalls(ctx, &vmProto.TrackSyscallsVmRequest{})
	time.Sleep(5 * time.Second)
	log.Printf("Syscalls being tracked")

	log.Printf("Starting client VM...")
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()

	stream, err := vmClient.SendClientCommand(ctx, &vmProto.SendClientCommandVmRequest{
		Ip:      "192.168.100.3",
		Command: fmt.Sprintf("mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -c %s -t 30 -P 4", ips[0]),
	})
	if err != nil {
		log.Fatalf("could not start job: %v", err)
	}

	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			break // server finished sending
		}
		if err != nil {
			log.Fatalf("error receiving: %v", err)
		}
		fmt.Printf("Notification: job %s\n", resp.Output)
	}

	time.Sleep(5 * time.Second)
	log.Printf("Stopping syscalls tracking...")
	vmClient.StopSyscalls(ctx, &vmProto.StopSyscallsVmRequest{})

	// vmClient.SendCommand(ctx, &vmProto.SendCommandVmRequest{
	// 	Ip:      "192.168.100.3",
	// 	Command: fmt.Sprintf("mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -c %s -t 30 -P 4", ips[0]),
	// })
	// log.Printf("Client VM started")
	// // Save PID file
	// pidFile := "/tmp/firecracker.pid"
	// pid := os.Getpid()
	// if err := os.WriteFile(pidFile, []byte(fmt.Sprintf("%d", pid)), 0644); err != nil {
	// 	log.Fatalf("Failed to write PID file: %v", err)
	// }

	// ctx, cancel := context.WithCancel(context.Background())
	// defer cancel()

	// manager := _vm.NewManager(cfg)
	// if err := manager.Initialize(ctx); err != nil {
	// 	log.Fatalf("Failed to initialize VMs: %v", err)
	// }

	// if err := manager.Start(ctx); err != nil {
	// 	log.Fatalf("Failed to start VMs: %v", err)
	// }

	// var wg sync.WaitGroup
	// vms := manager.GetVMs()
	// for i, vm := range vms {
	// 	wg.Add(1)
	// 	go func(vmIndex int, vm *_vm.SimplifiedVM) {
	// 		defer wg.Done()
	// 		for {
	// 			select {
	// 			case line := <-vm.Stdout:
	// 				fmt.Printf("VM %d stdout: %s\n", vmIndex, line)
	// 			case line := <-vm.Stderr:
	// 				fmt.Printf("VM %d stderr: %s\n", vmIndex, line)
	// 			case <-ctx.Done():
	// 				return
	// 			}
	// 		}
	// 	}(i, vm)
	// }

	// manager.LogNetworkingInfo()

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
