#!/usr/bin/env bash
set -ex

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Function to create a cloud-init ISO
create_iso() {
    local output="$1"
    local netconfig="$2"

    rm -f "/tmp/${output}"
    mkdosfs -n CIDATA -C "/tmp/${output}" 8192
    mcopy -oi "/tmp/${output}" -s "${SCRIPT_DIR}/user-data" ::
    mcopy -oi "/tmp/${output}" -s "${SCRIPT_DIR}/meta-data" ::
    mcopy -oi "/tmp/${output}" -s "${SCRIPT_DIR}/${netconfig}" ::
}

# Create the ISOs
create_iso "ubuntu-cloudinit.img" "network-config"
create_iso "cloudinit-vm0.img" "network-config-vm0"
create_iso "cloudinit-vm1.img" "network-config-vm1"
