#!/usr/bin/bash

gsutil cat gs://gcp-bucket/AEP2-10-256-1-probe/stdout.txt | \
  grep -A12 'DIMM 0$' | awk '{print $1}' | awk -F, '{print $1}' > output.txt

stride=$1
for i in `seq 0 12`; do
  count=`echo "2^$i" | bc`
  gsutil cat gs://gcp-bucket/AEP2-10-$stride-$count-probe/stdout.txt | \
    grep -A12 'DIMM 0$' | awk '{print $2}' > one.txt
  paste output.txt one.txt -d, > temp.txt
  mv temp.txt output.txt
done

rm one.txt
cat output.txt
