#!/bin/bash

#`sed 's/^#define SIZEBT_MACRO.*$/TEST/g'`


cd ../kernel

rm -f ../kernel/output.txt > /dev/null
cp -f memaccess.c memaccess.c.bak

hostname=`hostname -s`
TAG=${TAG:-`date +%Y%m%d%H%M%S`}

GS_Bucket="gcp-bucket"
op_array=( 0 1 2 3 )

for parallel in `seq 18 18`; do
  for bwsize_bit in `seq 6 12`; do
    for op in ${op_array[@]}; do
      for align_mode in `seq 5 6`; do
        msg="$hostname-8-$bwsize_bit-$parallel-$op-$align_mode-$TAG"
        echomsg="task=8,parallel=$parallel,bwsize_bit=$bwsize_bit,op=$op,align_mode=$align_mode,message=$msg,runtime=5"
        echo $echomsg
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

