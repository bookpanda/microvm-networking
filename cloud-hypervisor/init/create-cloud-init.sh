#!/usr/bin/env bash
set -ex

rm -f /tmp/ubuntu-cloudinit.img
mkdosfs -n CIDATA -C /tmp/ubuntu-cloudinit.img 8192
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/user-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/meta-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./init/network-config ::
