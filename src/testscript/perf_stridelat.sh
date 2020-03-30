#!/bin/bash

#stride_array=( 64 )
#access_array=( 64 128 192 256 320 384 448 512 )

stride_array=( 0 64 128 192 256 320 384 448 512 )
access_array=( 0 64 128 192 256 320 384 448 512 576 640 704 768 832 896 960 1024 1088 1152 1216 1280 1344 1408 1472 1536 1600 1664 1728 1792 1856 1920 1984 2048 )

output_file=`date +%Y%m%d-%H%M%S`-output.txt

wrmsr -a 0x1a4 0xf

for accesssize in ${access_array[@]};  do
	for stridesize in ${stride_array[@]}; do
		msg="AEP2-2-$accesssize-$stridesize"
		echo "===================task=2,access_size=$accesssize,stride_size=$stridesize,message=$msg======================" | tee -a $output_file
		echo "task=2,access_size=$accesssize,stride_size=$stridesize,message=$msg" > /proc/lattester
		while true; do
			sleep 1
			x=`dmesg | tail -n 5 | grep 'LATTester_LAT_END'`
			if [[ $x ]]; then
				break;
			fi
		done
		msgstart=`dmesg | grep -n 'LATTester_START' | tail -1 | awk -F':' {' print $1 '}`
		msgend=`dmesg | grep -n 'LATTester_LAT_END' | tail -1 | awk -F':' {' print $1 '}`
		msgend+="p"
		dmesg | sed -n "$msgstart,$msgend" | grep "average" | tee -a $output_file
		dmesg | sed -n "$msgstart,$msgend" | grep "Perf counter" | tee -a $output_file
	done
done

wrmsr -a 0x1a4 0x0
