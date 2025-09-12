package main

import (
    "context"
    "log"
    "os"
    "time"

    firecracker "github.com/firecracker-microvm/firecracker-go-sdk"
)

func main() {
    ctx := context.Background()

    // Path to Firecracker binary
    fcBinary := "/usr/bin/firecracker"

    // Unix socket for API
    socketPath := "/tmp/fc0.sock"

    // Create a Firecracker command
    cmd := firecracker.ExecCommandBuilder{}.
        WithBin(fcBinary).
        WithSocketPath(socketPath).
        Build(ctx)

    // Define the VM configuration
    cfg := firecracker.Config{
        SocketPath:      socketPath,
        KernelImagePath: "/tmp/vmlinux-5.10.223-no-acpi",
        Drives: []firecracker.Drive{
            {
                DriveID:      firecracker.String("rootfs"),
                PathOnHost:   firecracker.String("/tmp/debian-rootfs.ext4"),
                IsRootDevice: firecracker.Bool(true),
                IsReadOnly:   firecracker.Bool(false),
            },
        },
        MachineCfg: firecracker.MachineCfg{
            VcpuCount:   1,
            MemSizeMib:  256,
            HtEnabled:   false,
            TrackDirtyPages: firecracker.Bool(false),
        },
        NetworkInterfaces: []firecracker.NetworkInterface{},
        LogFifo:           "/tmp/fc0.log",
        MetricsFifo:       "/tmp/fc0-metrics.log",
    }

    // Create a Firecracker machine
    m, err := firecracker.NewMachine(ctx, cfg, cmd)
    if err != nil {
        log.Fatal(err)
    }

    defer m.StopVMM() // ensures VM stops when program exits

    // Start the VM
    if err := m.Start(ctx); err != nil {
        log.Fatal(err)
    }

    log.Println("VM started! Waiting 10s...")
    time.Sleep(10 * time.Second)
    log.Println("Done")
}
