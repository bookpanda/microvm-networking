package network

import (
	"fmt"
	"log"
	"os"
	"os/exec"
)

var dhcpProcess *exec.Cmd

func SetupDHCP(numVMs int) error {
	log.Println("Setting up DHCP server for automatic IP assignment...")

	// give IPs from .10 to .50, gateway is .254 (br0)
	// use Google DNS servers
	dhcpConfig := `# DHCP configuration for VMs
subnet 192.168.100.0 netmask 255.255.255.0 {
    range 192.168.100.10 192.168.100.50;
    option routers 192.168.100.254;
    option domain-name-servers 8.8.8.8, 8.8.4.4;
    default-lease-time 600;
    max-lease-time 7200;
    
    # Static assignments for predictable IPs based on MAC addresses
    host vm0 { hardware ethernet AA:FC:00:00:00:01; fixed-address 192.168.100.2; }
    host vm1 { hardware ethernet AA:FC:00:00:00:02; fixed-address 192.168.100.3; }
    host vm2 { hardware ethernet AA:FC:00:00:00:03; fixed-address 192.168.100.4; }
    host vm3 { hardware ethernet AA:FC:00:00:00:04; fixed-address 192.168.100.5; }
    host vm4 { hardware ethernet AA:FC:00:00:00:05; fixed-address 192.168.100.6; }
    host vm5 { hardware ethernet AA:FC:00:00:00:06; fixed-address 192.168.100.7; }
    host vm6 { hardware ethernet AA:FC:00:00:00:07; fixed-address 192.168.100.8; }
    host vm7 { hardware ethernet AA:FC:00:00:00:08; fixed-address 192.168.100.9; }
}`

	if err := os.WriteFile("/tmp/vm-dhcp.conf", []byte(dhcpConfig), 0644); err != nil {
		return fmt.Errorf("failed to write DHCP config: %v", err)
	}

	// start DHCP server
	dhcpProcess = exec.Command("sudo", "dhcpd", "-cf", "/tmp/vm-dhcp.conf", "br0")
	if err := dhcpProcess.Start(); err != nil {
		log.Printf("DHCP server not available, VMs will need manual IP configuration: %v", err)
		log.Println("To install DHCP server: sudo apt install isc-dhcp-server")
		return nil
	}

	log.Println("DHCP server started - These VMs will get predictable IPs:")
	for i := 0; i < numVMs && i < 8; i++ {
		log.Printf("  VM %d: MAC AA:FC:00:00:00:%02X → IP 192.168.100.%d", i, i+1, i+2)
	}
	return nil
}

func StopDHCP() error {
	if dhcpProcess != nil && dhcpProcess.Process != nil {
		log.Println("Stopping DHCP server...")
		if err := dhcpProcess.Process.Kill(); err != nil {
			log.Printf("Failed to stop DHCP server: %v", err)
		}
		dhcpProcess.Wait() // wait for process to exit
	}

	// cleanup DHCP config file
	if err := os.Remove("/tmp/vm-dhcp.conf"); err != nil {
		log.Printf("Failed to remove DHCP config file: %v", err)
	}

	log.Println("DHCP server stopped")
	return nil
}
