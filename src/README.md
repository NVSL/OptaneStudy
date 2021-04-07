# LatTester

LatTester is designed to measure memory latency on AEP platfrom.

It contains two modules, each of them need to be mounted on a pmem device (in `fsdax` mode):

**LatencyFS:** LatencyFS (LatFS) is the *working area* of the tester. A series of memory accesses will be issued to locations within this area.

**ReportFS:** ReportFS (RepFS) is used to store access latency of individual requests. It reports the CPU cycles of individual access requests as contigious 8-byte values. User can gather the result using `dd if=/dev/pmemX of=result bs=<bs> count=<count>`


## Requirement
- Linux 4.13 kernel (with devel)
- CPU with clwb/avx-512 support (i.e., support Optane)
- Two real or emulated PMem regions
- LatTester will overwrite content of the PMem devices

## Tasks
It supports following tasks:
1. Random and sequential latency of flush instructions
2. Strided access latency
3. Strided access bandwidth
4. Random Size-Bandwidth Test
5. Overwrite Test
6. Read-after-write Test
7. Straight Write Test
8. Writer Thread Test
9. Flush/Fence BW Test
10. APC Cache Probe Test
11. iMC Probe Test

## Configuration
LatTester can update results in GCP cloud datastore. Following fields are manually entered (`testscript/config.example.json`).
```
{
  "media": "lpmem",
  "interleaving": "1",
  "local": "1",
  "avgpower": "15000",
  "peakpower": "20000",
  "emon": "1",
  "aepwatch": "1",
  "fence": "0",
  "baddata": "0",
  "prefetching": "0",
  "rerun": "0"
}
```

Also see macros defined in `kernel/lattester.h`

## Usage (in testscripts)

#### Load modules

 `/mount.sh  [RepFS] [LatFS]`

- `RepFS` is an emulated PMem device that stores per-access latency.
- `LatFS` is a PMem device used for latency measurement. (LATTester will allocate from CPU #0.)

#### Usage

`cat /proc/lattester`

```
LATTester task=<task> op=<op> [options]
Tasks:
        1: Basic Latency Test (not configurable)
        2: Strided Latency Test  [access_size] [stride_size]
        3: Strided Bandwidth Test [access_size] [stride_size] [delay] [parallel] [runtime] [global]
        4: Random Size-Bandwidth Test [bwtest_bit] [access_size] [parallel] [runtime] [align_mode]
        5: Overwrite Test [stride_size]
        6: Read-after-write Test [stride_size] [access_size]
        7: Straight Write Test [start_addr] [size]
        8: Writer Thread Test [bwsize_bit] [parallel] [align_mode]
        9: Flush/Fence BW Test [acccess_size] [fence_rate] [flush_rate] [op (1=random)]
        10: APC Cache Probe Test [probe_count], [strided_size]
        11: iMC Probe Test [probe_count]
        12: Sequencial Test [access_size], [parallel]
Options:
         [op|o] Operation: 0=Load, 1=Write(NT) 2=Write(clwb), 3=Write+Flush(clwb+flush) default=0
         [access_size|a] Access size (bytes), default=64
         [align_mode|l] Align mode: 0 = Global (anywhere), 1 = Per-Thread pool (default) 2 = Per-DIMM pool 3=Per-iMC pool
                                    4 = Per-Channel Group, 5 = [NIOnly]: Global, 6 = [NIOnly]: Per-DIMM
         [stride_size|s] Stride size (bytes), default=64
         [delay|d] Delay (cycles), default=0
         [parallel|p] Concurrent kthreads, default=1
         [bwsize_bit|b] Aligned size (2^b bytes), default=6
         [runtime|t] Runtime (seconds), default=10
         [write_addr|w] For Straight Write: access location, default=0 
         [write_size|z] For Straight Write: access size, default=0 
         [fence_rate|f] Access size before issuing a sfence instruction 
         [clwb_rate|c] Access size before issuing a clwb instruction 
```

#### Run a test

`echo "task=<task> op=<op> [options] > /proc/lattester"`

`dmesg | tail` to find 

#### Sample script (testscript/run.example.sh)

