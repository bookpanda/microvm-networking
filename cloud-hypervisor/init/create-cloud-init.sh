#!/usr/bin/env bash
set -ex

rm -f /tmp/ubuntu-cloudinit.img
mkdosfs -n CIDATA -C /tmp/ubuntu-cloudinit.img 8192
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./user-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./meta-data ::
mcopy -oi /tmp/ubuntu-cloudinit.img -s ./network-config ::
