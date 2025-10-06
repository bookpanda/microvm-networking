package experiment

import (
	"fmt"
	"sync"

	"github.com/bookpanda/microvm-networking/benchmark/internal/config"
	filesystemProto "github.com/bookpanda/microvm-networking/benchmark/proto/filesystem/v1"
	networkProto "github.com/bookpanda/microvm-networking/benchmark/proto/network/v1"
	vmProto "github.com/bookpanda/microvm-networking/benchmark/proto/vm/v1"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type Experiment struct {
	config *config.Config
	nodes  []*Node
	wg     sync.WaitGroup
}

type Node struct {
	conn       *grpc.ClientConn
	vmClient   vmProto.VmServiceClient
	netwClient networkProto.NetworkServiceClient
	fsClient   filesystemProto.FileSystemServiceClient
	config     config.NodeConfig
}

func NewExperiment(config *config.Config) (*Experiment, error) {
	experiment := &Experiment{
		config: config,
		wg:     sync.WaitGroup{},
	}

	for _, nodeConfig := range config.Nodes {
		conn, err := grpc.NewClient(fmt.Sprintf("%s:%d", nodeConfig.IP, nodeConfig.Port), grpc.WithTransportCredentials(insecure.NewCredentials()))
		if err != nil {
			return nil, fmt.Errorf("failed to create gRPC client: %v", err)
		}
		experiment.nodes = append(experiment.nodes, &Node{
			conn:       conn,
			vmClient:   vmProto.NewVmServiceClient(conn),
			netwClient: networkProto.NewNetworkServiceClient(conn),
			fsClient:   filesystemProto.NewFileSystemServiceClient(conn),
			config:     nodeConfig,
		})
	}

	return experiment, nil
}
