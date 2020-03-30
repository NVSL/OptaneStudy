#!/usr/bin/env python3
import sys
import os
import struct
import matplotlib.pyplot as plt
import numpy as np


OPS_COUNT = 1048576 * 16
INSANE = 1e18  # when rdtscp produces a negative number
THRESHOLD = 22000

def stat(times):
#    print('min,p25, p50, p75, p95, p99, p99.9, max, avg, stddev')
    a = np.array(times)
#    list = [np.amin(a), np.percentile(a, 25),
#            np.percentile(a, 50), np.percentile(a, 75),
#            np.percentile(a, 95), np.percentile(a, 99),
#            np.percentile(a, 99.9), np.amax(a), np.average(a), np.std(a)]
#    print(','.join(str(x) for x in list))
    list = ['RESULT', np.percentile(a, 99), np.percentile(a, 99.9), np.percentile(a, 99.99), np.percentile(a, 99.999), np.percentile(a, 99.9999)]
    print(','.join(str(x) for x in list))

def download(tag):
    os.system('gsutil cp gs://gcp-bucket/{0}/dump /tmp/dump'.format(tag))

def parse(tag):
    times = []
    spikes = []
    insanes = []
    print("Tag=", tag)
    with open('/tmp/dump', mode='rb') as f:
            for j in range(0, OPS_COUNT):
                item = struct.unpack('L', f.read(8))
                try:
                    line = int(item[0])
                    if line < INSANE:
                        if line > THRESHOLD:
                            spikes.append(line)
#                            line = THRESHOLD
                        times.append(line)
                    else:
                        insanes.append(line);
                except ValueError:
                    continue
    f.close()

    print("Ignored=", len(insanes))
    if insanes:
        print(insanes)

    print("Spike=", len(spikes))
    if spikes:
        print("AvgSpike=", sum(spikes)/len(spikes))

    stat(times)

#    plt.plot(times)
#    ax = plt.axes()
#    ax.set_ylim(0, THRESHOLD)
#    plt.savefig(tag + ".pdf")
#    plt.clf()
#    plt.cla()
#    plt.close()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        download(sys.argv[1])
        parse(sys.argv[1])
    else:
        parse('LOCAL')
