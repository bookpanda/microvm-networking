package config

import "flag"

type Config struct {
	NumVMs     int
	KernelPath string
	RootfsPath string
}

func ParseFlags() *Config {
	cfg := &Config{}

	flag.IntVar(&cfg.NumVMs, "vms", 1, "Number of VMs to create")
	flag.StringVar(&cfg.KernelPath, "kernel", "/tmp/vmlinux-5.10.223-no-acpi", "Path to kernel image")
	flag.StringVar(&cfg.RootfsPath, "rootfs", "/tmp/debian-rootfs.ext4", "Path to rootfs image")

	flag.Parse()
	return cfg
}
