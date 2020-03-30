#!/usr/bin/python
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

ymax=1E6
ymin=1E1
pp = PdfPages('timeseries.pdf')

for i in range(1, len(sys.argv)):
    fileName=sys.argv[i]
    chartTitle=''
    x=[]
    times=[]
    with open(fileName, mode='r') as f:
        for line in f:
            line = line.strip('\n').strip('\r')
            try:
                line = int(line)
                times.append(min(ymax, line))
            except ValueError:
                chartTitle = line
    #plt.yticks(np.arange(ymin, ymax, 25))
    #plt.xticks(np.arange(0, len(times) - 1, 100000))
    #plt.xticks(rotation='vertical')
    plt.title(chartTitle, fontsize=10)
    plt.ylim(ymin, ymax)
    plt.yscale('log')
    plt.plot(times)
    plt.grid(True)
    plt.ylabel('Latency (cycles)')
    plt.savefig(pp, format='pdf')

pp.close()
