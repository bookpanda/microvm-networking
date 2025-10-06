package main

import (
	"context"
	"log"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	"github.com/bookpanda/microvm-networking/benchmark/internal/experiment"
)

func main() {
	cfg := config.NewConfig("vm-throughput")
	// cfg := config.NewConfig("bm-throughput")
	// cfg := config.NewConfig("bm-latency")
	experiment, err := experiment.NewExperiment(cfg)
	if err != nil {
		log.Fatalf("Failed to create experiment: %v", err)
	}

	// err = experiment.RunBMBenchmark(context.Background())
	err = experiment.RunVMBenchmark(context.Background())
	if err != nil {
		log.Fatalf("Failed to run benchmark: %v", err)
	}
}
