package main

import (
	"context"
	"log"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	"github.com/bookpanda/microvm-networking/benchmark/internal/experiment"
)

func main() {
	cfg := config.NewConfig("bm-throughput")
	experiment, err := experiment.NewExperiment(cfg)
	if err != nil {
		log.Fatalf("Failed to create experiment: %v", err)
	}

	err = experiment.RunBMBenchmark(context.Background())
	if err != nil {
		log.Fatalf("Failed to run benchmark: %v", err)
	}

	// nodeConn, err := grpc.NewClient("10.10.1.1:50051", grpc.WithTransportCredentials(insecure.NewCredentials()))
	// if err != nil {
	// 	log.Fatalf("Failed to create gRPC client: %v", err)
	// }

	// vmClient := vmProto.NewVmServiceClient(nodeConn)
	// networkClient := networkProto.NewNetworkServiceClient(nodeConn)
	// filesystemClient := filesystemProto.NewFileSystemServiceClient(nodeConn)

	// ctx := context.Background()

	// log.Printf("Cleaning up VM...")
	// _, err = vmClient.Cleanup(ctx, &vmProto.CleanupVmRequest{})
	// if err != nil {
	// 	log.Fatalf("Failed to cleanup VM: %v", err)
	// }

	// log.Printf("Cleaning up network...")
	// _, err = networkClient.Cleanup(ctx, &networkProto.CleanupNetworkRequest{
	// 	NumVMs: 2,
	// })
	// if err != nil {
	// 	log.Fatalf("Failed to cleanup network: %v", err)
	// }

	// log.Printf("Cleaning up filesystem...")
	// _, err = filesystemClient.Cleanup(ctx, &filesystemProto.CleanupFileSystemRequest{})
	// if err != nil {
	// 	log.Fatalf("Failed to cleanup filesystem: %v", err)
	// }

	// log.Printf("Setting up network...")
	// _, err = networkClient.Setup(ctx, &networkProto.SetupNetworkRequest{
	// 	NumVMs:   2,
	// 	BridgeIP: "192.168.100.1",
	// })
	// if err != nil {
	// 	log.Fatalf("Failed to setup network: %v", err)
	// }

	// log.Printf("Starting VM...")
	// ips := []string{"192.168.100.2", "192.168.100.3"}
	// for _, ip := range ips {
	// 	vmClient.Create(ctx, &vmProto.CreateVmRequest{
	// 		Ip:         ip,
	// 		KernelPath: cfg.KernelPath,
	// 		RootfsPath: cfg.RootfsPath,
	// 		GatewayIP:  "192.168.100.1",
	// 	})
	// }

	// time.Sleep(5 * time.Second)

	// log.Printf("Starting server VM...")
	// vmClient.SendServerCommand(ctx, &vmProto.SendServerCommandVmRequest{
	// 	Ip:      "192.168.100.2",
	// 	Command: "mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -s",
	// })
	// log.Printf("Server VM started")

	// log.Printf("Starting to track syscalls...")
	// vmClient.TrackSyscalls(ctx, &vmProto.TrackSyscallsVmRequest{})
	// time.Sleep(5 * time.Second)
	// log.Printf("Syscalls being tracked")

	// log.Printf("Starting client VM...")
	// ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	// defer cancel()

	// stream, err := vmClient.SendClientCommand(ctx, &vmProto.SendClientCommandVmRequest{
	// 	Ip:      "192.168.100.3",
	// 	Command: fmt.Sprintf("mount -t tmpfs -o size=64M tmpfs /tmp && HOME=/tmp iperf3 -c %s -t 30 -P 4", ips[0]),
	// })
	// if err != nil {
	// 	log.Fatalf("could not start job: %v", err)
	// }

	// for {
	// 	resp, err := stream.Recv()
	// 	if err == io.EOF {
	// 		break // server finished sending
	// 	}
	// 	if err != nil {
	// 		log.Fatalf("error receiving: %v", err)
	// 	}
	// 	fmt.Printf("Notification: job %s\n", resp.Output)
	// }

	// time.Sleep(5 * time.Second)
	// log.Printf("Stopping syscalls tracking...")
	// vmClient.StopSyscalls(ctx, &vmProto.StopSyscallsVmRequest{})
}
