#!/bin/sh

ip link add br0 type bridge
ip link set br0 up
ip addr flush dev br0
ip addr add 10.1.0.1/24 dev br0

ip link add br1 type bridge
ip link set br1 up
ip addr flush dev br1
