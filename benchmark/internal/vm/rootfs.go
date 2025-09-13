package vm

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
)

// creates VM-specific rootfs images with pre-configured networking
func PrepareRootfs(baseRootfsPath string, numVMs int) ([]string, error) {
	var rootfsPaths []string

	for i := 0; i < numVMs; i++ {
		vmRootfsPath := fmt.Sprintf("/tmp/debian-rootfs-vm%d.ext4", i)

		// Copy base rootfs for this VM
		cmd := exec.Command("cp", baseRootfsPath, vmRootfsPath)
		if err := cmd.Run(); err != nil {
			return nil, fmt.Errorf("failed to copy rootfs for VM %d: %v", i, err)
		}

		// Create network configuration for this VM
		vmIP := fmt.Sprintf("192.168.100.%d", i+2)
		networkConfig := fmt.Sprintf(`auto eth0
iface eth0 inet static
    address %s/24
    gateway 192.168.100.254
    dns-nameservers 8.8.8.8 8.8.4.4
`, vmIP)

		// Write network config to a temporary file
		configFile := fmt.Sprintf("/tmp/interfaces-vm%d", i)
		if err := os.WriteFile(configFile, []byte(networkConfig), 0644); err != nil {
			return nil, fmt.Errorf("failed to write network config for VM %d: %v", i, err)
		}

		// Mount rootfs and copy network config
		mountPoint := fmt.Sprintf("/tmp/mnt-vm%d", i)
		os.MkdirAll(mountPoint, 0755)

		cmd = exec.Command("sudo", "mount", "-o", "loop", vmRootfsPath, mountPoint)
		if err := cmd.Run(); err != nil {
			return nil, fmt.Errorf("failed to mount rootfs for VM %d: %v", i, err)
		}

		cmd = exec.Command("sudo", "cp", configFile, filepath.Join(mountPoint, "etc/network/interfaces"))
		if err := cmd.Run(); err != nil {
			return nil, fmt.Errorf("failed to copy network config for VM %d: %v", i, err)
		}

		cmd = exec.Command("sudo", "umount", mountPoint)
		cmd.Run() // Ignore error

		// Cleanup
		os.Remove(configFile)
		os.RemoveAll(mountPoint)

		rootfsPaths = append(rootfsPaths, vmRootfsPath)
		log.Printf("Prepared rootfs for VM %d with IP: %s", i, vmIP)
	}

	return rootfsPaths, nil
}
