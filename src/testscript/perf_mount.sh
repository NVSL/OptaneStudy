#!/bin/sh


if [ $# -ne 2 ]; then
    echo './mount.sh [DRAM_DEV] [PMEM_DEV]'
    echo 'e.g., ./mount.sh /dev/pmem0 /dev/pmem14'
    exit 1
fi

cd ../kernel

DRAM_DEV=$1
PMEM_DEV=$2


echo 0 > /proc/sys/kernel/soft_watchdog
# Hard lock watchdog at nmi_watchdog

make clean 2>&1 > /dev/null
make -j 2 PERF=1>&1

mkdir -p /mnt/latency
mkdir -p /mnt/report

insmod repfs.ko
insmod latfs.ko

mount -t ReportFS $DRAM_DEV /mnt/report
mount -t LatencyFS $PMEM_DEV /mnt/latency

