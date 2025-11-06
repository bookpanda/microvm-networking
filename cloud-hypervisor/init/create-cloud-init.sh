#!/usr/bin/env bash
set -ex

rm -f /tmp/ubuntu-cloudinit.img
mkdosfs -n CIDATA -C /tmp/ubuntu-cloudinit.img 8192
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/user-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/meta-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/network-config ::

rm -f /tmp/cloudinit-vm0.img
mkdosfs -n CIDATA -C /tmp/cloudinit-vm0.img 8192
mcopy -oi /tmp/cloudinit-vm0.img -s ./init/user-data ::
mcopy -oi /tmp/cloudinit-vm0.img -s ./init/meta-data ::
mcopy -oi /tmp/cloudinit-vm0.img -s ./init/network-config-vm0 ::

rm -f /tmp/cloudinit-vm1.img
mkdosfs -n CIDATA -C /tmp/cloudinit-vm1.img 8192
mcopy -oi /tmp/cloudinit-vm1.img -s ./init/user-data ::
mcopy -oi /tmp/cloudinit-vm1.img -s ./init/meta-data ::
mcopy -oi /tmp/cloudinit-vm1.img -s ./init/network-config-vm1 ::