package experiment

import (
	"context"
	"fmt"
	"io"
	"log"
	"sync"
	"time"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	filesystemProto "github.com/bookpanda/microvm-networking/benchmark/proto/filesystem/v1"
	networkProto "github.com/bookpanda/microvm-networking/benchmark/proto/network/v1"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type Experiment struct {
	config *config.Config
	nodes  []*Node
	wg     sync.WaitGroup
}

type Node struct {
	conn       *grpc.ClientConn
	vmClient   vmProto.VmServiceClient
	netwClient networkProto.NetworkServiceClient
	fsClient   filesystemProto.FileSystemServiceClient
	config     config.NodeConfig
}

func NewExperiment(config *config.Config) (*Experiment, error) {
	experiment := &Experiment{
		config: config,
		wg:     sync.WaitGroup{},
	}

	for _, nodeConfig := range config.Nodes {
		conn, err := grpc.NewClient(fmt.Sprintf("%s:%d", nodeConfig.IP, nodeConfig.Port), grpc.WithTransportCredentials(insecure.NewCredentials()))
		if err != nil {
			return nil, fmt.Errorf("failed to create gRPC client: %v", err)
		}
		experiment.nodes = append(experiment.nodes, &Node{
			conn:       conn,
			vmClient:   vmProto.NewVmServiceClient(conn),
			netwClient: networkProto.NewNetworkServiceClient(conn),
			fsClient:   filesystemProto.NewFileSystemServiceClient(conn),
			config:     nodeConfig,
		})
	}

	return experiment, nil
}

func (e *Experiment) RunBenchmark(ctx context.Context) error {
	for _, node := range e.nodes {
		e.wg.Add(1)
		go func(node *Node) {
			defer e.wg.Done()
			err := e.setupNode(ctx, node)
			if err != nil {
				log.Fatalf("[%s]: Failed to setup node: %v", node.conn.Target(), err)
			}
		}(node)
	}
	e.wg.Wait()
	log.Printf("Nodes setup")

	// Setup cross-node routing
	// log.Printf("Setting up cross-node routing...")
	// for _, node := range e.nodes {
	// 	e.wg.Add(1)
	// 	go func(node *Node) {
	// 		defer e.wg.Done()
	// 		err := e.setupCrossNodeRouting(ctx, node)
	// 		if err != nil {
	// 			log.Fatalf("[%s]: Failed to setup cross-node routing: %v", node.conn.Target(), err)
	// 		}
	// 	}(node)
	// }
	// e.wg.Wait()
	// log.Printf("Cross-node routing setup completed")
	// time.Sleep(3 * time.Second)

	log.Printf("Starting servers...")
	for _, node := range e.nodes {
		for _, vmConfig := range node.config.VMs {
			if vmConfig.Type != "server" {
				continue
			}
			e.wg.Add(1)
			go func() {
				defer e.wg.Done()
				err := e.startServer(ctx, node, &vmConfig)
				if err != nil {
					log.Fatalf("[%s]: Failed to start server VM: %v", node.conn.Target(), err)
				}
			}()
		}
	}
	e.wg.Wait()
	log.Printf("Servers started")

	log.Printf("Starting to track syscalls...")
	for _, node := range e.nodes {
		e.wg.Add(1)
		go func(node *Node) {
			defer e.wg.Done()
			err := e.trackSyscalls(ctx, node)
			if err != nil {
				log.Fatalf("[%s]: Failed to track syscalls: %v", node.conn.Target(), err)
			}
		}(node)
	}
	e.wg.Wait()
	log.Printf("Syscalls being tracked")
	time.Sleep(3 * time.Second)

	log.Printf("Starting clients...")
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()

	for _, node := range e.nodes {
		for _, vmConfig := range node.config.VMs {
			if vmConfig.Type != "client" {
				continue
			}
			e.wg.Add(1)
			go func() {
				defer e.wg.Done()
				err := e.startClient(ctx, node, &vmConfig)
				if err != nil {
					log.Fatalf("[%s]: Failed to start client: %v", node.conn.Target(), err)
				}
			}()
		}
	}
	e.wg.Wait()
	log.Printf("Clients finished")

	time.Sleep(5 * time.Second)
	log.Printf("Stopping syscalls tracking...")
	for _, node := range e.nodes {
		_, err := node.vmClient.StopSyscalls(ctx, &vmProto.StopSyscallsVmRequest{})
		if err != nil {
			log.Fatalf("[%s]: Failed to stop syscalls: %v", node.conn.Target(), err)
		}
	}

	return nil
}

func (e *Experiment) setupNode(ctx context.Context, node *Node) error {
	log.Printf("[%s]: Cleaning up VM...", node.conn.Target())
	_, err := node.vmClient.Cleanup(ctx, &vmProto.CleanupVmRequest{})
	if err != nil {
		log.Fatalf("[%s]: Failed to cleanup VM: %v", node.conn.Target(), err)
	}

	log.Printf("[%s]: Cleaning up network...", node.conn.Target())
	_, err = node.netwClient.Cleanup(ctx, &networkProto.CleanupNetworkRequest{
		NumVMs: 2,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to cleanup network: %v", node.conn.Target(), err)
	}

	log.Printf("[%s]: Cleaning up filesystem...", node.conn.Target())
	_, err = node.fsClient.Cleanup(ctx, &filesystemProto.CleanupFileSystemRequest{})
	if err != nil {
		log.Fatalf("[%s]: Failed to cleanup filesystem: %v", node.conn.Target(), err)
	}

	log.Printf("[%s]: Setting up network...", node.conn.Target())
	_, err = node.netwClient.Setup(ctx, &networkProto.SetupNetworkRequest{
		NumVMs:   int32(len(node.config.VMs)),
		BridgeIP: node.config.BridgeIP,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to setup network: %v", node.conn.Target(), err)
	}

	log.Printf("[%s]: Creating VMs...", node.conn.Target())
	for _, vmConfig := range node.config.VMs {
		_, err = node.vmClient.Create(ctx, &vmProto.CreateVmRequest{
			Ip:         vmConfig.IP,
			KernelPath: e.config.KernelPath,
			RootfsPath: e.config.RootfsPath,
			GatewayIP:  node.config.BridgeIP,
		})
		if err != nil {
			log.Fatalf("[%s]: Failed to create VM: %v", node.conn.Target(), err)
		}
	}

	return nil
}

func (e *Experiment) setupCrossNodeRouting(ctx context.Context, node *Node) error {
	log.Printf("[%s]: Setting up cross-node routing...", node.conn.Target())

	// For each other node, set up routing and NAT
	for _, otherNode := range e.nodes {
		if otherNode.config.IP == node.config.IP {
			continue // Skip self
		}

		// Get the subnet from the other node's bridge IP (e.g., "192.168.101.1" -> "192.168.101.0/24")
		remoteSubnet := getBridgeSubnet(otherNode.config.BridgeIP)

		log.Printf("[%s]: Adding route to %s via %s", node.conn.Target(), remoteSubnet, otherNode.config.IP)
		_, err := node.netwClient.SetupCrossNodeRoute(ctx, &networkProto.SetupCrossNodeRouteRequest{
			RemoteSubnet:  remoteSubnet,
			RemoteNodeIP:  otherNode.config.IP,
			LocalBridgeIP: node.config.BridgeIP,
		})
		if err != nil {
			log.Printf("[%s]: Failed to setup route to %s: %v", node.conn.Target(), remoteSubnet, err)
			return err
		}
	}

	return nil
}

func getBridgeSubnet(bridgeIP string) string {
	// Extract first 3 octets and add .0/24
	// e.g., "192.168.100.1" -> "192.168.100.0/24"
	parts := make([]byte, 0, len(bridgeIP))
	octets := 0
	for i := 0; i < len(bridgeIP) && octets < 3; i++ {
		if bridgeIP[i] == '.' {
			octets++
			parts = append(parts, '.')
		} else {
			parts = append(parts, bridgeIP[i])
		}
	}
	return string(parts) + "0/24"
}

func (e *Experiment) startServer(ctx context.Context, node *Node, vmConfig *config.VMConfig) error {
	log.Printf("[%s]: Starting server VM...", node.conn.Target())
	_, err := node.vmClient.SendServerCommand(ctx, &vmProto.SendServerCommandVmRequest{
		Ip:      vmConfig.IP,
		Command: vmConfig.Command,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to start server VM %s: %v", node.conn.Target(), vmConfig.IP, err)
	}

	return nil
}

func (e *Experiment) trackSyscalls(ctx context.Context, node *Node) error {
	log.Printf("[%s]: Starting to track syscalls...", node.conn.Target())
	_, err := node.vmClient.TrackSyscalls(ctx, &vmProto.TrackSyscallsVmRequest{})
	if err != nil {
		log.Fatalf("[%s]: Failed to track syscalls: %v", node.conn.Target(), err)
	}
	return nil
}

func (e *Experiment) startClient(ctx context.Context, node *Node, vmConfig *config.VMConfig) error {
	log.Printf("[%s]: Starting client VM...", node.conn.Target())
	stream, err := node.vmClient.SendClientCommand(ctx, &vmProto.SendClientCommandVmRequest{
		Ip:      vmConfig.IP,
		Command: vmConfig.Command,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to start client VM %s: %v", node.conn.Target(), vmConfig.IP, err)
	}

	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			break // server finished sending
		}
		if err != nil {
			log.Fatalf("[%s]: Failed to receive client VM %s: %v", node.conn.Target(), vmConfig.IP, err)
		}
		fmt.Printf("[%s]: Notification: job %s\n", node.conn.Target(), resp.Output)

	}

	return nil
}
