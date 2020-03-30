#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`

###########################################
# New SizeBW Test                         #
# Use 4K align for access_size over 4K    #
###########################################


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

alignbit_array=( 12    12    12    12     12     12     12      12 )
#alignbit_array=( 6  7   8   9   10   11   12   12   12    12    12    12    12    12    12    12    12    12    12    12     12     12     12      12 )
size_array=(     45056 49152 65536 131072 262144 524288 1048576 2097152 )
#size_array=(     64 128 256 512 1024 2048 4096 8192 12288 16384 20480 24576 28672 32768 36864 40960 45056 49152 65536 131072 262144 524288 1048576 2097152 )
#parallel_array=( 23 24 ) # DRAM
parallel_array=( 4 1 2 ) # NT
#parallel_array=( 16 4 12 ) # PMEM
op_array=( 0 1 3 )
hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

len=${#size_array[@]}
len1=${#op_array[@]}
for (( i=0; i<$len; i++ )); do
	bwsizebit=${alignbit_array[i]}
        accesssize=${size_array[i]}
	if (( "$bwsizebit" <= "9" )); then
		sed -i 's/^#define SIZEBTNT_MACRO.*$/#define SIZEBTNT_MACRO SIZEBTNT_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTLD_MACRO.*$/#define SIZEBTLD_MACRO SIZEBTLD_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTST_MACRO.*$/#define SIZEBTST_MACRO SIZEBTST_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTSTFLUSH_MACRO.*$/#define SIZEBTSTFLUSH_MACRO SIZEBTSTFLUSH_'"$accesssize"'_AVX512/g' memaccess.c
		../testscript/umount.sh
		../testscript/mount.sh $1 $2
	fi

	for (( j=0; j<$len1; j++ )); do
		parallel=${parallel_array[j]}
		op=${op_array[j]}
		msg="$hostname-4-$TAG-$bwsizebit-$accesssize-$parallel-$op"
		echomsg="task=4,parallel=$parallel,runtime=5,bwsize_bit=$bwsizebit,access_size=$accesssize,message=$msg,op=$op"
		echo $echomsg
		export TaskID=$msg
		echo "sleep 2" > /tmp/aeprun.sh
		echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
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
cp -f memaccess.c.bak memaccess.c
mv output.txt  ../testscript/
