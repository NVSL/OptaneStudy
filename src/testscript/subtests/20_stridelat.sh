#!/bin/bash

stride_array=$(seq -s ' ' 0 64 512)
access_64b_4k=$(seq -s ' ' 64 64 4096)
access_4k_16k=$(seq -s ' ' 4352 256 16384)
access_array=("${access_64b_4k[@]}" "${access_4k_16k[@]}")

rm -f output.txt
for accesssize in ${access_array[@]};  do
	for stridesize in ${stride_array[@]}; do
		stridesize=`expr $accesssize + $stridesize`
		msg="AEP2-2-$accesssize-$stridesize"
		echo "task=2,access_size=$accesssize,stride_size=$stridesize,message=$msg" | tee -a output.txt
		echo "task=2,access_size=$accesssize,stride_size=$stridesize,message=$msg" > /proc/lattester
		if [[ $accesssize > $stridesize ]]; then
			continue
		fi
		while true; do
			x=`dmesg | tail -n 3 | grep 'LATTester_LAT_END'`
			if [[ $x ]]; then
				break;
			fi
			sleep 1
		done
		msgstart=`dmesg | grep -n 'LATTester_START' | tail -1 | awk -F':' {' print $1 '}`
		msgend=`dmesg | grep -n 'LATTester_LAT_END' | tail -1 | awk -F':' {' print $1 '}`
		msgend+="p"
		dmesg | sed -n "$msgstart,$msgend" | grep "average" | tee -a output.txt
	done
done
