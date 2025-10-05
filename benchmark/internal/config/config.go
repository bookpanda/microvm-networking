package config

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
)

type Config struct {
	NumVMs     int
	KernelPath string
	RootfsPath string
	Nodes      []NodeConfig
}

type NodeConfig struct {
	IP       string     `json:"ip"`
	Port     int        `json:"port"`
	BridgeIP string     `json:"bridge_ip"`
	VMs      []VMConfig `json:"vms"`
}

type VMConfig struct {
	IP      string `json:"ip"`
	Type    string `json:"type"`
	Command string `json:"command"`
}

func NewConfig() *Config {
	cfg := &Config{}

	var configFile string
	flag.StringVar(&configFile, "config", "./tests/vm-latency.json", "Path to configuration JSON file")
	flag.StringVar(&cfg.KernelPath, "kernel", "/tmp/vmlinux-5.10.223-no-acpi", "Path to kernel image")
	flag.StringVar(&cfg.RootfsPath, "rootfs", "/tmp/debian-rootfs.ext4", "Path to rootfs image")

	flag.Parse()

	if configFile != "" {
		if err := loadConfigFile(configFile, cfg); err != nil {
			fmt.Fprintf(os.Stderr, "Error loading config file: %v\n", err)
			os.Exit(1)
		}
	}

	return cfg
}

func loadConfigFile(path string, cfg *Config) error {
	data, err := os.ReadFile(path)
	if err != nil {
		return fmt.Errorf("failed to read config file: %w", err)
	}

	var jsonData struct {
		Nodes []NodeConfig `json:"nodes"`
	}

	if err := json.Unmarshal(data, &jsonData); err != nil {
		return fmt.Errorf("failed to parse config file: %w", err)
	}

	cfg.Nodes = jsonData.Nodes
	return nil
}
