#!/bin/bash
if [[ $EUID -ne 0 ]]; then
	echo "Please run as root" 
	exit 1
fi

umount /mnt/latency
umount /mnt/report

rmmod latfs
rmmod repfs

echo 1 > /proc/sys/kernel/soft_watchdog
