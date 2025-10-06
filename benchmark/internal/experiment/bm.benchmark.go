package experiment

import (
	"context"
	"log"
	"time"

	nodeProto "github.com/bookpanda/microvm-networking/benchmark/proto/node/v1"
)

func (e *Experiment) RunBMBenchmark(ctx context.Context) error {
	for _, node := range e.nodes {
		e.wg.Add(1)
		go func(node *Node) {
			defer e.wg.Done()
			err := e.setupNode(ctx, node, false)
			if err != nil {
				log.Fatalf("[%s]: Failed to setup node: %v", node.conn.Target(), err)
			}
		}(node)
	}
	e.wg.Wait()
	log.Printf("Nodes setup")

	// time.Sleep(3 * time.Second) // VERY IMPORTANT TO WAIT FOR MICROVM TO START
	log.Printf("Starting servers...")
	for _, node := range e.nodes {
		e.wg.Add(1)
		go func(n *Node) {
			defer e.wg.Done()
			err := e.startNodeServer(ctx, n)
			if err != nil {
				log.Fatalf("[%s]: Failed to start server VM: %v", n.conn.Target(), err)
			}
		}(node)
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
		e.wg.Add(1)
		go func(n *Node) {
			defer e.wg.Done()
			err := e.startNodeClient(ctx, n)
			if err != nil {
				log.Fatalf("[%s]: Failed to start client: %v", n.conn.Target(), err)
			}
		}(node)
	}
	e.wg.Wait()
	log.Printf("Clients finished")

	time.Sleep(5 * time.Second)
	log.Printf("Stopping syscalls tracking...")
	for _, node := range e.nodes {
		_, err := node.nodeClient.StopSyscalls(ctx, &nodeProto.StopSyscallsNodeRequest{})
		if err != nil {
			log.Fatalf("[%s]: Failed to stop syscalls: %v", node.conn.Target(), err)
		}
	}

	return nil
}
