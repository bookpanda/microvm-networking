package config

import (
	"flag"
	"log"
)

type Config struct {
	NumVMs     int
	KernelPath string
	RootfsPath string
	Mode       Mode
	Test       Test
}

func ParseFlags() *Config {
	cfg := &Config{}

	flag.IntVar(&cfg.NumVMs, "vms", 2, "Number of VMs to create")
	flag.StringVar(&cfg.KernelPath, "kernel", "/tmp/vmlinux-5.10.223-no-acpi", "Path to kernel image")
	flag.StringVar(&cfg.RootfsPath, "rootfs", "/tmp/debian-rootfs.ext4", "Path to rootfs image")

	mode := flag.String("mode", string(VM_VM), "Mode to run the benchmark in")
	test := flag.String("test", string(Throughput), "Test to run the benchmark in")
	flag.Parse()

	cfg.Mode = Mode(*mode)
	if cfg.Mode == VM_VM && cfg.NumVMs%2 != 0 {
		log.Fatalf("Number of VMs must be even for VM_VM mode (got %d)", cfg.NumVMs)
	}

	cfg.Test = Test(*test)

	return cfg
}
