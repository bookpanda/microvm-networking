package network

import (
	"fmt"
	"log"
	"os"
	"os/exec"
)

// SetupDHCP configures a DHCP server for automatic IP assignment
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
}`

	// Write DHCP config file
	if err := os.WriteFile("/tmp/vm-dhcp.conf", []byte(dhcpConfig), 0644); err != nil {
		return fmt.Errorf("failed to write DHCP config: %v", err)
	}

	// Start DHCP server
	cmd := exec.Command("sudo", "dhcpd", "-cf", "/tmp/vm-dhcp.conf", "br0")
	if err := cmd.Start(); err != nil {
		log.Printf("Failed to start DHCP server (might not be installed): %v", err)
		return nil // Non-fatal
	}

	log.Println("DHCP server started")
	return nil
}
