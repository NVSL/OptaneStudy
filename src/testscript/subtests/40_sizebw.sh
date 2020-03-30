#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

#bwsizebit_array=( 6 7 8 )
bwsizebit_array=( 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 )
#parallel_array=( 1 )
parallel_array=( 1 2 3 4 6 8 )
#op_array=( 0 )
op_array=( 3 )
hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}


for bwsizebit in ${bwsizebit_array[@]};  do
        accesssize=`echo 2^$bwsizebit | bc`
	if (( "$bwsizebit" <= "9" )); then
		sed -i 's/^#define SIZEBTNT_MACRO.*$/#define SIZEBTNT_MACRO SIZEBTNT_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTLD_MACRO.*$/#define SIZEBTLD_MACRO SIZEBTLD_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTST_MACRO.*$/#define SIZEBTST_MACRO SIZEBTST_'"$accesssize"'_AVX512/g' memaccess.c
		sed -i 's/^#define SIZEBTSTFLUSH_MACRO.*$/#define SIZEBTSTFLUSH_MACRO SIZEBTSTFLUSH_'"$accesssize"'_AVX512/g' memaccess.c
		../testscript/umount.sh
		../testscript/mount.sh $1 $2
	fi
	for parallel in ${parallel_array[@]}; do
		for op in ${op_array[@]}; do
			msg="$hostname-4-$TAG-$bwsizebit-$accesssize-$parallel-$op"
			echomsg="task=4,parallel=$parallel,runtime=5,bwsize_bit=$bwsizebit,access_size=$accesssize,message=$msg,op=$op"
            echo $echomsg
            export TaskID=$msg
            echo "sleep 5" > /tmp/aeprun.sh
			echo "echo $echomsg > /proc/lattester" >> /tmp/aeprun.sh
            echo "sleep 3" >> /tmp/aeprun.sh
            echo "sleep 3" >> /tmp/aeprun.sh
            echo "sleep 3" >> /tmp/aeprun.sh
            echo "sleep 3" >> /tmp/aeprun.sh
            echo "sleep 3" >> /tmp/aeprun.sh
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
cp -f memaccess.c.bak memaccess.c
mv output.txt  ../testscript/
