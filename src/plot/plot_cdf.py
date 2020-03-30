#!/usr/bin/python
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

pp = PdfPages('cdf.pdf')

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
                times.append(line)
                x.append(len(x))
            except ValueError:
                chartTitle = line
        chartSize = len(x)
        x[:] = [float(item + 1) / chartSize for item in x]
        times.sort()
        plt.title(chartTitle, fontsize=10)
        plt.xticks(np.arange(0, 1.01, 0.05), rotation='vertical')
        plt.yscale('log')
        plt.ylim(1E1, 1E6)
        plt.plot(x, times)
        plt.grid(True)
        plt.ylabel('Latency (cycles)')
        plt.savefig(pp, format='pdf')

pp.close()
