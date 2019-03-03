#!/bin/sh

qemu-system-x86_64 -hda server.qcow2 -m 512M -nographic -enable-kvm -net nic,macaddr=12:00:00:00:00:00 -net bridge,br=br0 -net nic,macaddr=14:00:00:00:00:00 -net bridge,br=br1
