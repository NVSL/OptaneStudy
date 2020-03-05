# An Empirical Guide to the Behavior and Use of Scalable Persistent Memory

Performance characterization and empirical study of Intel's Optane persistent memory. This repo contains the data and source code for our [USENIX FAST paper](https://www.usenix.org/conference/fast20/presentation/yang).


## Table of Content

Directory | Description
----------|------------
[data/](https://github.com/NVSL/OptaneStudy/tree/master/data) | Raw data (in csv format) of the sweep test
[graphs/](https://github.com/NVSL/OptaneStudy/tree/master/graphs) | Data (in csv format) used in the paper graphs
[src/](https://github.com/NVSL/OptaneStudy/tree/master/src) | Source code of lattester tool (coming soon)


## Configuration

#### Hardware

Item | Description
---|---
CPUs | 2
CPU | Intel Xeon Platinum 8260 ES (Cascade Lake SP)
CPUFreq. | 24 Cores at 2.2 Ghz base clock
CPU L1 | 32 KB i-Cache, 32 KB d-cache
CPU L2 | 1 MB
CPU L3 | 33 MB (shared)
DRAM | 2x6x32 GB Micron DDR4 2666 MHz (36ASF4G72PZ)
PM | 2x6x256 GB Intel Optane DC 2666 MHz QS (NMA1XXD256GQS)

#### Software

Item | Description
---|---
GNU/Linux Distro | Fedora 27
Linux Kernel | 4.13.0
CPU Governor | Performance
HyperThreading (SMP) | Disabled
NVDIMM Firmware | 01.01.00.5253
KASLR | Disabled
CPU mitigations | Off

## Bibliography

```
@inproceedings {OptaneStudy,
author = {Jian Yang and Juno Kim and Morteza Hoseinzadeh and Joseph Izraelevitz and Steve Swanson},
title = {An Empirical Guide to the Behavior and Use of Scalable Persistent Memory},
booktitle = {18th {USENIX} Conference on File and Storage Technologies ({FAST} 20)},
year = {2020},
isbn = {978-1-939133-12-0},
address = {Santa Clara, CA},
pages = {169--182},
url = {https://www.usenix.org/conference/fast20/presentation/yang},
publisher = {{USENIX} Association},
month = feb,
}
```

## License
```
# SPDX-License-Identifier: GPL-2.0

Copyright 2019 Regents of the University of California, UCSD Non-Volatile Systems Lab
```
