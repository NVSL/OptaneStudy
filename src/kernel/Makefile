#
# Makefile for the Latency and Report psudo file systems.
#

obj-m += repfs.o latfs.o

ccflags-y+=-mmmx -msse2 -mavx -mpreferred-stack-boundary=4
repfs-y := rep.o
latfs-y := lat.o proc.o misc.o memaccess.o tasks.o

avx_supported := $(call as-instr,vpxor %xmm0$(comma)%xmm0$(comma)%xmm0,yes,no)
avx2_supported := $(call as-instr,vpgatherdd %ymm0$(comma)(%eax$(comma)%ymm1\
	$(comma)4)$(comma)%ymm2,yes,no)

AEP := $(shell cat /proc/cpuinfo  | grep clwb | wc -l)

ifeq ($(AEP), 0)
  ccflags-y+= -DNOAVX512
else
  ccflags-y+= -DAEP_SUPPORTED
endif

ifeq ($(PERF), 1)
  ccflags-y+=-DUSE_PERF
  latfs-y+=perf_util.o
endif

ccflags-y+= -DFENCE

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd`
	gcc -O2 -o gen_tasks gen_tasks.c $(ccflags-y)

debug:
	gcc -O0 -o debug -mavx -g debug.c memaccess.c  $(ccflags-y)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` clean
	rm -f tasks.py gen_tasks debug *.pyc -f 

