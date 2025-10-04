package config

import (
	"flag"
)

type Config struct {
	NumVMs     int
	KernelPath string
	RootfsPath string
	Nodes      []NodeConfig
}

type NodeConfig struct {
	IP       string
	Port     int
	BridgeIP string
	VMs      []VMConfig
}

type VMConfig struct {
	IP      string
	Type    string
	Command string
}

func ParseFlags() *Config {
	cfg := &Config{}

	flag.StringVar(&cfg.KernelPath, "kernel", "/tmp/vmlinux-5.10.223-no-acpi", "Path to kernel image")
	flag.StringVar(&cfg.RootfsPath, "rootfs", "/tmp/debian-rootfs.ext4", "Path to rootfs image")

	flag.Parse()

	return cfg
}
