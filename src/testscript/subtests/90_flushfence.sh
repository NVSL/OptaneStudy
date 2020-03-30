#!/bin/bash


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

function run() {
	msg="$hostname-9-$TAG-$accesssize-$clwb_rate-$fence_rate-$parallel-$op"
	echomsg="task=9,parallel=$parallel,runtime=5,access_size=$accesssize,clwb_rate=$clwb_rate,fence_rate=$fence_rate,message=$msg,op=$op"
	echo $echomsg
	export TaskID=$msg
	echo "sleep 1" > /tmp/aeprun.sh
	echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	echo "sleep 1" >> /tmp/aeprun.sh
	chmod +x /tmp/aeprun.sh
	aeprun /tmp/aeprun.sh
	sleep 1
	msgstart=`dmesg | grep -n 'LATTester_START' | tail -1 | awk -F':' {' print $1 '}`
	msgend=`dmesg | grep -n 'LATTester_END' | tail -1 | awk -F':' {' print $1 '}`
	msgend+="p"
	dmesg | sed -n "$msgstart,$msgend" | tee -a output.txt

}



for sizebit in `seq 6 22`; do 
	accesssize=`echo 2^$sizebit | bc`
	for fencebit in `seq $sizebit $sizebit`; do # Fencing per accesssize
		fence_rate=`echo 2^$fencebit | bc`
		for clwbbit in `seq 6 $fencebit`; do
			clwb_rate=`echo 2^$clwbbit | bc`
			for parallel in `seq 1 4`; do
				for op in `seq 0 1`; do
					run
				done
			done
		done
	done
done

for sizebit in `seq 8 22`; do 
	accesssize=`echo 2^$sizebit | bc`
	for fencebit in `seq 8 8`; do # Fencing per aepline
		fence_rate=`echo 2^$fencebit | bc`
		for clwbbit in `seq 8 8`; do
			clwb_rate=`echo 2^$clwbbit | bc`
			for parallel in `seq 1 4`; do
				for op in `seq 0 1`; do
					run
				done
			done
		done
	done
done


cp -f memaccess.c.bak memaccess.c
mv output.txt  ../testscript/
