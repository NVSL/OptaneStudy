#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

access_array=( 64 128 192 256 512 1024 2048 4096 8192 16384 )
#parallel_array=( 1 2 4 8 )
parallel_array=( 1 2 3 4 6 8 )
stride_array=( 0 64 128 192 256 512 1024 4096 16384 )
op_array=( 3 )
hostname=`hostname -s`
delay=0
TAG=${TAG:-`date +%Y%m%d%H%M%S`}


#- StrideBW: AEP1-3-TAG-64-256-1-300-2
# 3: Strided Bandwidth Test [access_size] [stride_size] [delay] [parallel] [runtime] [global]

for accesssize in ${access_array[@]};  do
	for stridesize in ${stride_array[@]}; do
		for parallel in ${parallel_array[@]}; do
			for op in ${op_array[@]}; do
				msg="$hostname-3-$TAG-$accesssize-$stridesize-$parallel-$delay-$op"
				echomsg="task=3,parallel=$parallel,runtime=5,access_size=$accesssize,stride_size=$stridesize,delay=$delay,message=$msg,op=$op"
				echo $echomsg
				if [[ $accesssize > $stridesize ]]; then
					continue
				fi
				export TaskID=$msg
				echo "sleep 3" > /tmp/aeprun.sh
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
				chmod +x /tmp/aeprun.sh
				aeprun /tmp/aeprun.sh
				sleep 1
				msgstart=`dmesg | grep -n 'LATTester_START' | tail -1 | awk -F':' {' print $1 '}`
				msgend=`dmesg | grep -n 'LATTester_END' | tail -1 | awk -F':' {' print $1 '}`
				msgend+="p"
				dmesg | sed -n "$msgstart,$msgend" | tee -a output.txt
			done
		done
	done
done
cp -f memaccess.c.bak memaccess.c
mv output.txt  ../testscript/
