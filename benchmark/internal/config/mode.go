package config

type Mode string

const (
	BM_BM Mode = "bm-bm"
	VM_VM Mode = "vm-vm"
	BM_VM Mode = "bm-vm"
)

type Test string

const (
	Throughput Test = "throughput"
	Latency    Test = "latency"
)
