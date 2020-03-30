#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

GS_Bucket="gcp-bucket"


for writesize in `seq 4096 4096 49152`; do
	msg="$hostname-7-$writesize-$TAG"
	echomsg="task=7,write_size=$writesize,write_start=204795904" # 4999x4096
	echo $echomsg
	export TaskID=$msg
	echo Report $1 Test $2
	../testscript/umount.sh
	../testscript/mount.sh $1 $2
	echo "sleep 5" > /tmp/aeprun.sh
	echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
	echo "sleep 30" >> /tmp/aeprun.sh
	chmod +x /tmp/aeprun.sh
	aeprun /tmp/aeprun.sh
done

