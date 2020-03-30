#!/usr/bin/python
import sys
import os
import struct
import numpy as np

from tasks import *

insane = 1e18  # when rdtscp produces a negative number

freqstr = os.popen('lscpu | grep MHz | head -n 1 | awk {\'print $3\'}').read()
cpufreq = float(freqstr)/1000

def stat(series):
    print 'CPU Freq: ', cpufreq
    print 'item,min,p25, p50, p75, p95, p99, p99.9, max, avg, stddev'
    #print 'item,avg,min,stddev,p50,p95,p99,p99.9,max'
    for fn in series:
        times = []
        with open('/tmp/'+fn, mode='r') as f:
            for line in f:
                line = line.strip('\n').strip('\r')
                try:
                    line = int(line)
                    if line < insane:
                        # times.append(line)
                        times.append(line/float(cpufreq))
                except ValueError:
                    continue
        f.close()
        a = np.array(times)
        list = [fn, np.amin(a), np.percentile(a, 25),
                np.percentile(a, 50), np.percentile(a, 75),
                np.percentile(a, 95), np.percentile(a, 99),
                np.percentile(a, 99.9), np.amax(a), np.average(a), np.std(a)]
        print ','.join(str(x) for x in list)

parsed = []

os.system(dd)

f = open('/tmp/dump', 'rb')
try:
    for i in range(0, len(tasks)):
        parsed.append(tasks[i])
        txt = open('/tmp/'+tasks[i], 'w')
        for j in range(0, ops_count):
            item = struct.unpack('L', f.read(8))
            txt.write(str(item[0])+"\n")
        txt.write(str(tasks[i]))
        txt.close()

finally:
    f.close()

stat(parsed)

#cmd = " ".join('/tmp/'+str(e) for e in parsed)
#os.system('python stat.py ' + cmd)
