#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

GS_Bucket="gcp-bucket"

hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

function run() {
	msg="$hostname-11-$count-$parallel-$TAG"
	echomsg="task=11,probe_count=$count,parallel=$parallel,runtime=25"
	echo $echomsg
	export TaskID=$msg
	echo "sleep 1" > /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	chmod +x /tmp/aeprun.sh
	aeprun /tmp/aeprun.sh
}

for parallel in `seq 1 12`; do
	for count in `seq 4 5`;  do
		run
	done
done

#count=5
#run
#count=7
#run
#count=9
#run
#count=10
#run

