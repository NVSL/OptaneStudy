#!/bin/bash

#stride_array=$(seq -s ' ' 0 64 512)
#stride_array=$(seq -s ' ' 0 64 256)
stride_array=( 0 )

access_64b_4k=$(seq -s ' ' 64 64 4096)
access_4k_16k=$(seq -s ' ' 4352 256 16384)
access_16k_72k=$(seq -s ' ' 17408 1024 73728)
#access_array=$(seq -s ' ' 5120 256 16384)
access_array=("${access_64b_4k[@]}" "${access_4k_16k[@]}")
#access_array=("${access_16k_72k[@]}")

rm -f output.txt
for accesssize in ${access_array[@]};  do
	for stridesize in ${stride_array[@]}; do
		stridesize=`expr $accesssize + $stridesize`
		msg="AEP2-6-$accesssize-$stridesize"
		echo "task=6,access_size=$accesssize,stride_size=$stridesize,message=$msg" | tee -a output.txt
		echo "task=6,access_size=$accesssize,stride_size=$stridesize,message=$msg" > /proc/lattester
		while true; do
			sleep 2
			x=`dmesg | tail -n 4 | grep 'LATTester_LAT_END'`
			if [[ $x ]]; then
				break;
			fi
		done
		msgstart=`dmesg | grep -n 'LATTester_START' | tail -1 | awk -F':' {' print $1 '}`
		msgend=`dmesg | grep -n 'LATTester_LAT_END' | tail -1 | awk -F':' {' print $1 '}`
		msgend+="p"
		dmesg | sed -n "$msgstart,$msgend" | grep "average" | tee -a output.txt
	done
done
