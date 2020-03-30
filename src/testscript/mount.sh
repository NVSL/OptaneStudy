#!/bin/bash
# Mount file systems

if [[ $# -ne 2 ]]; then
	echo './mount.sh [DRAM_DEV] [PMEM_DEV]'
	echo 'e.g., ./mount.sh /dev/pmem0 /dev/pmem14'
	exit 1
fi

if [[ $EUID -ne 0 ]]; then
	echo "Please run as root" 
	exit 1
fi

cd ../kernel

DRAM_DEV=$1
PMEM_DEV=$2


repdev=`mount | grep ReportFS | awk {'print \$1'}`
testdev=`mount | grep LatencyFS | awk {'print \$1'}`


if [ ! -z $repdev ] || [ ! -z $testdev ]; then
	echo "Please run umount.sh first"
	exit 1
fi


echo 0 > /proc/sys/kernel/soft_watchdog
# Hard lock watchdog at nmi_watchdog

make clean 2>&1 > /dev/null
make -j 2>&1

mkdir -p /mnt/latency
mkdir -p /mnt/report

REPFS=`lsmod | grep repfs`
LATFS=`lsmod | grep latfs`

if [ ! -z "$REPFS" ]; then
	echo Unmounting existing ReportFS
	../testscript/umount.sh
elif [ ! -z "$LATFS" ]; then
	echo Unmounting existing LatencyFS
	../testscript/umount.sh
fi

insmod repfs.ko
insmod latfs.ko

mount -t ReportFS $DRAM_DEV /mnt/report
mount -t LatencyFS $PMEM_DEV /mnt/latency
