package experiment

import (
	"context"
	"log"

	filesystemProto "github.com/bookpanda/microvm-networking/benchmark/proto/filesystem/v1"
	networkProto "github.com/bookpanda/microvm-networking/benchmark/proto/network/v1"
	nodeProto "github.com/bookpanda/microvm-networking/benchmark/proto/node/v1"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
)

func (e *Experiment) setupNode(ctx context.Context, node *Node, createVMs bool) error {
	log.Printf("[%s]: Cleaning up node...", node.conn.Target())
	_, err := node.nodeClient.Cleanup(ctx, &nodeProto.CleanupNodeRequest{})
	if err != nil {
		log.Fatalf("[%s]: Failed to cleanup VM: %v", node.conn.Target(), err)
	}

	log.Printf("[%s]: Cleaning up VM...", node.conn.Target())
	_, err = node.vmClient.Cleanup(ctx, &vmProto.CleanupVmRequest{})
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

	if !createVMs {
		return nil
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

func (e *Experiment) startNodeServer(ctx context.Context, node *Node) error {
	_, err := node.nodeClient.SendServerCommand(ctx, &nodeProto.SendServerCommandNodeRequest{
		Command: node.config.Command,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to start server VM: %v", node.conn.Target(), err)
	}

	return nil
}

func (e *Experiment) startNodeClient(ctx context.Context, node *Node) error {
	_, err := node.nodeClient.SendClientCommand(ctx, &nodeProto.SendClientCommandNodeRequest{
		Command: node.config.Command,
	})
	if err != nil {
		log.Fatalf("[%s]: Failed to start client VM: %v", node.conn.Target(), err)
	}

	return nil
}
