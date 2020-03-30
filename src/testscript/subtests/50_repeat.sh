#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

stride_array=( 0)
stride_array=( 0 64 128 192 256 320 384 448 512 768 1024 1280 1536 2048 )
hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

GS_Bucket="gcp-bucket"

#- StrideBW: AEP1-5-0

for stridesize in ${stride_array[@]}; do
	msg="$hostname-5-$stridesize-$TAG"
	echomsg="task=5,stride_size=$stridesize"
	echo $echomsg
	export TaskID=$msg
	echo Report $1 Test $2
	../testscript/umount.sh
	../testscript/mount.sh $1 $2
	echo "sleep 5" > /tmp/aeprun.sh
	echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
	echo "sleep 5" >> /tmp/aeprun.sh
	chmod +x /tmp/aeprun.sh
	aeprun /tmp/aeprun.sh
	sleep 5
	dd if=$1 of=/tmp/dump bs=8M count=2
	gsutil cp /tmp/dump gs://$GS_Bucket/$TaskID/dump
done

