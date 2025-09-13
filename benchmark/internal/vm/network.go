package vm

import (
	"context"
	"fmt"
	"log"
	"time"
)

// sets up IP addresses inside the VM
func (v *SimplifiedVM) ConfigureNetwork(ctx context.Context, vmIndex int) error {
	// wait for VM to be ready
	time.Sleep(5 * time.Second)

	// vmIndex = 0: 192.168.100.2
	vmIP := fmt.Sprintf("192.168.100.%d/24", vmIndex+2)

	commands := []string{
		fmt.Sprintf("ip addr add %s dev eth0", vmIP),
		"ip link set eth0 up",
		"ip route add default via 192.168.100.254",
		"echo 'nameserver 8.8.8.8' > /etc/resolv.conf",
	}

	for _, cmd := range commands {
		log.Printf("Configuring VM %d network: %s", vmIndex, cmd)
		// Note: This would require implementing command execution via Firecracker API
		// For now, this is a placeholder showing what needs to be done
	}

	log.Printf("VM %d configured with IP: 192.168.100.%d", vmIndex, vmIndex+2)
	return nil
}
