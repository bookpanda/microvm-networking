package experiment

import (
	"context"
	"log"
	"time"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
)

func (e *Experiment) RunVMBenchmark(ctx context.Context) error {
	for _, node := range e.nodes {
		e.wg.Add(1)
		go func(node *Node) {
			defer e.wg.Done()
			err := e.setupNode(ctx, node, true)
			if err != nil {
				log.Fatalf("[%s]: Failed to setup node: %v", node.conn.Target(), err)
			}
		}(node)
	}
	e.wg.Wait()
	log.Printf("Nodes setup")

	time.Sleep(3 * time.Second) // VERY IMPORTANT TO WAIT FOR MICROVM TO START
	log.Printf("Starting servers...")
	for _, node := range e.nodes {
		for _, vmConfig := range node.config.VMs {
			if vmConfig.Type != "server" {
				continue
			}
			e.wg.Add(1)
			go func(n *Node, vm *config.VMConfig) {
				defer e.wg.Done()
				err := e.startServer(ctx, n, vm)
				if err != nil {
					log.Fatalf("[%s]: Failed to start server VM: %v", n.conn.Target(), err)
				}
			}(node, &vmConfig)
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
	for _, node := range e.nodes {
		for _, vmConfig := range node.config.VMs {
			if vmConfig.Type != "client" {
				continue
			}
			e.wg.Add(1)
			go func(n *Node, vm *config.VMConfig) {
				defer e.wg.Done()
				err := e.startClient(ctx, n, vm)
				if err != nil {
					log.Fatalf("[%s]: Failed to start client: %v", n.conn.Target(), err)
				}
			}(node, &vmConfig)
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
