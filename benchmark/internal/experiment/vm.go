package experiment

import (
	"context"
	"fmt"
	"io"
	"log"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
)

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
			log.Printf("[%s]: Server finished sending", node.conn.Target())
			break // server finished sending
		}
		if err != nil {
			log.Fatalf("[%s]: Failed to receive client VM %s: %v", node.conn.Target(), vmConfig.IP, err)
		}
		fmt.Printf("[%s]: Notification: job %s\n", node.conn.Target(), resp.Output)

	}

	return nil
}
