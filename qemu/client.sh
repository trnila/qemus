#!/bin/sh

qemu-system-x86_64 -hda client1.qcow2 -m 512M -nographic -enable-kvm -net nic,macaddr=12:00:00:00:00:01 -net bridge,br=br0
