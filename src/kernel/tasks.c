/* SPDX-License-Identifier: GPL-2.0 */
#define _GNU_SOURCE

#include <linux/compiler.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/parser.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/ctype.h>
#include <linux/bitops.h>
#include <linux/magic.h>
#include <linux/random.h>
#include <linux/cred.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <asm/simd.h>
#include <asm/fpu/api.h>
#include <linux/kthread.h>
#include "lattester.h"
#include "memaccess.h"
#include "common.h"

#ifdef USE_PERF
#include "perf_util.h"
#endif

//#define KSTAT 1

#define kr_info(string, args...) \
            do { printk(KERN_INFO "{%d}" string, ctx->core_id, ##args); } while (0)

inline void wait_for_stop(void)
{
	/* Wait until we are told to stop */
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (kthread_should_stop())
			break;
		schedule();
	}
}

#define BENCHMARK_BEGIN(flags) \
	drop_cache(); \
	local_irq_save(flags); \
	local_irq_disable(); \
	kernel_fpu_begin();

#define BENCHMARK_END(flags) \
	kernel_fpu_end(); \
	local_irq_restore(flags); \
	local_irq_enable();


#ifdef USE_PERF

#define PERF_CREATE(x) lattest_perf_event_create()
#define PERF_START(x) \
		lattest_perf_event_reset(); \
		lattest_perf_event_enable();
#define PERF_STOP(x) \
		lattest_perf_event_disable(); \
		lattest_perf_event_read();
#define PERF_PRINT(x) \
		kr_info("Performance counters:\n"); \
		lattest_perf_event_print();
#define PERF_RELEASE(x) lattest_perf_event_release();


#else // USE_PERF

#define PERF_CREATE(x) do { } while (0)
#define PERF_START(x) do { } while (0)
#define PERF_STOP(x) do { } while (0)
#define PERF_PRINT(x) do { } while (0)
#define PERF_RELEASE(x) do { } while (0)

#endif // USE_PERF


#define TIMEDIFF(start, end)  ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec))


/* Latency Test
 * 1 Thread
 * Run a series of microbenchmarks on combination of store, flush and fence operations.
 * Sequential -> Random
 */
int latency_job(void *arg)
{
	int i, j;
	static unsigned long flags;
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	long *loc = (long *)sbi->rep->virt_addr;
	u8 *buf;
	long result;

	long *rbuf;
	int rpages;
#ifdef KSTAT
	long total;
	long average, stddev;
	long min, max;
#endif
	u8 hash = 0;
	int skip;

	kr_info("BASIC_OP at smp id %d\n", smp_processor_id());
	rpages = roundup((2 * BASIC_OPS_TASK_COUNT + 1) * LATENCY_OPS_COUNT * sizeof(long), PAGE_SIZE) >> PAGE_SHIFT;
	/* fill report area page tables */
	for (j = 0; j < rpages; j++)
	{
		buf = (u8 *)sbi->rep->virt_addr + (j << PAGE_SHIFT);
		hash = hash ^ *buf;
	}

	/* Sequential */
	for (i = 0; i < BASIC_OPS_TASK_COUNT; i++)
	{
		/* fill page tables */
		for (j = 0; j < BASIC_OP_POOL_PAGES; j++)
		{
			buf = ctx->addr + (j << PAGE_SHIFT);
			hash = hash ^ *buf;
		}

		kr_info("Running %s\n", latency_tasks_str[i]);
		buf = ctx->addr;

		skip = latency_tasks_skip[i]; /* request size */
#ifdef KSTAT
		total = 0;
		min = 0xffffffff;
		max = 0;
#endif
		rbuf = (long *)(sbi->rep->virt_addr + (i + 1) * LATENCY_OPS_COUNT * sizeof(long));
		BENCHMARK_BEGIN(flags);
		for (j = 0; j < LATENCY_OPS_COUNT; j++)
		{
			result = bench_func[i](buf);
			buf += skip;
			rbuf[j] = result;
#ifdef KSTAT
			if (result < min)
				min = result;
			if (result > max)
				max = result;
			total += result;
#endif
		}
		BENCHMARK_END(flags);
#ifdef KSTAT
		average = total / LATENCY_OPS_COUNT;
		stddev = 0;
		for (j = 0; j < LATENCY_OPS_COUNT; j++)
		{
			stddev += (rbuf[j] - average) * (rbuf[j] - average);
		}
		kr_info("[%d]%s avg %lu, stddev^2 %lu, max %lu, min %lu\n", i, latency_tasks_str[i], average, stddev / LATENCY_OPS_COUNT, max, min);
#else
		kr_info("[%d]%s Done\n", i, latency_tasks_str[i]);
#endif
	}

	/* Random */
	for (i = 0; i < BASIC_OPS_TASK_COUNT; i++)
	{
		kr_info("Generating random bytes at %p, size %lx", loc, LATENCY_OPS_COUNT * 8);
		get_random_bytes(sbi->rep->virt_addr, LATENCY_OPS_COUNT * 8);

		/* fill page tables */
		for (j = 0; j < BASIC_OP_POOL_PAGES; j++)
		{
			buf = ctx->addr + (j << PAGE_SHIFT);
			hash = hash ^ *buf;
		}

		kr_info("Running %s\n", latency_tasks_str[i]);
		buf = ctx->addr;
#ifdef KSTAT
		total = 0;
		min = 0xffffffff;
		max = 0;
#endif
		rbuf = (long *)(sbi->rep->virt_addr + (i + BASIC_OPS_TASK_COUNT + 1) * LATENCY_OPS_COUNT * sizeof(long));
		BENCHMARK_BEGIN(flags);
		for (j = 0; j < LATENCY_OPS_COUNT; j++)
		{
			result = bench_func[i](&buf[loc[j] & BASIC_OP_MASK]);
			rbuf[j] = result;
#ifdef KSTAT
			if (result < min)
				min = result;
			if (result > max)
				max = result;
			total += result;
#endif
		}
		BENCHMARK_END(flags);

#ifdef KSTAT
		average = total / LATENCY_OPS_COUNT;
		stddev = 0;
		for (j = 0; j < LATENCY_OPS_COUNT; j++)
		{
			stddev += (rbuf[j] - average) * (rbuf[j] - average);
		}
		kr_info("[%d]%s avg %lu, stddev^2 %lu, max %lu, min %lu\n", i, latency_tasks_str[i], average, stddev / LATENCY_OPS_COUNT, max, min);
#else
		kr_info("[%d]%s done\n", i, latency_tasks_str[i]);
#endif
	}
	kr_info("hash %d", hash);
	kr_info("LATTester_LAT_END:\n");
	wait_for_stop();
	return 0;
}

int strided_latjob(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	static unsigned long flags;
	long access_size = ctx->sbi->access_size;
	long stride_size = ctx->sbi->strided_size;
	long delay = ctx->sbi->delay;
	uint8_t *buf = ctx->addr;
	uint8_t *region_end = buf + GLOBAL_WORKSET;
	long count = GLOBAL_WORKSET / access_size;
	struct timespec tstart, tend;
	long pages, diff;
	int hash = 0;
	int i;

	PERF_CREATE();

	if (stride_size * count > GLOBAL_WORKSET)
		count = GLOBAL_WORKSET / stride_size;

	if (count > LATENCY_OPS_COUNT)
		count = LATENCY_OPS_COUNT;

	pages = roundup(count * stride_size, PAGE_SIZE) >> PAGE_SHIFT;

	/* fill page tables */
	for (i = 0; i < pages; i++)
	{
		buf = ctx->addr + (i << PAGE_SHIFT);
		hash = hash ^ *buf;
	}

	buf = ctx->addr;

	kr_info("Working set begin: %p end: %p, size=%ld, stride=%ld, delay=%ld, batch=%ld\n", buf, region_end, access_size, stride_size, delay, count);

	for (i = 0; i < BENCH_SIZE_TASK_COUNT; i++){
		kr_info("Running %s\n", bench_size_map[i]);
		BENCHMARK_BEGIN(flags);

		getrawmonotonic(&tstart);
		PERF_START();

		lfs_stride_bw[i](buf, access_size, stride_size, delay, count);
		asm volatile ("mfence \n" :::);

		PERF_STOP();
		getrawmonotonic(&tend);

		diff = TIMEDIFF(tstart, tend);
		BENCHMARK_END(flags);
		kr_info("[%d]%s count %ld total %ld ns, average %ld ns.\n", i, latency_tasks_str[i],
			count, diff, diff / count);

	PERF_PRINT();
	}
	PERF_RELEASE();
	kr_info("LATTester_LAT_END: %d\n", hash);
	wait_for_stop();
	return 0;
}


int sizebw_job(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	int op = ctx->sbi->op;
	struct latencyfs_timing *timing = &sbi->timing[ctx->core_id];
	int bit_length = sbi->bwsize_bit;
	long align_size = 1 << bit_length;
	long access_size = sbi->access_size;
	long count = PERTHREAD_CHECKSTOP / access_size;
	uint8_t *buf = ctx->addr;
	uint8_t *region_end = buf + PERTHREAD_WORKSET;
	static unsigned long flags;
	unsigned long bitmask;
	unsigned long seed;

	if (sbi->align == ALIGN_GLOBAL) {
		bitmask = (GLOBAL_WORKSET - 1) ^ (align_size - 1);
	} else if (sbi->align == ALIGN_PERTHREAD) {
		bitmask = (PERTHREAD_WORKSET - 1) ^ (align_size - 1);
	} else	{
		kr_info("Unsupported align mode %d\n", sbi->align);
		return 0;
	}

	get_random_bytes(&seed, sizeof(unsigned long));
	kr_info("Working set begin: %p end: %p, access_size=%ld, align_size=%ld, batch=%ld, bitmask=%lx, seed=%lx\n",
		 buf, region_end, access_size, align_size, count, bitmask, seed);

	BENCHMARK_BEGIN(flags);
	while (1) {
		lfs_size_bw[op](buf, access_size, count, &seed, bitmask);
		timing->v += access_size * count;

		if (kthread_should_stop())
			break;
		schedule(); // Schedule with IRQ disabled?
	}

	BENCHMARK_END(flags);
	wait_for_stop();

	return 0;
}

int strided_bwjob(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	static unsigned long flags;
	int op = ctx->sbi->op;
	long access_size = ctx->sbi->access_size;
	long stride_size = ctx->sbi->strided_size;
	long delay = ctx->sbi->delay;
	struct latencyfs_timing *timing = &sbi->timing[ctx->core_id];
	uint8_t *buf = ctx->addr;
	uint8_t *region_end = buf + PERTHREAD_WORKSET;
	long count = PERTHREAD_CHECKSTOP / access_size;

	if (stride_size * count > PERTHREAD_WORKSET)
		count = PERTHREAD_WORKSET / stride_size;

	kr_info("Working set begin: %p end: %p, size=%ld, stride=%ld, delay=%ld, batch=%ld\n", buf, region_end, access_size, stride_size, delay, count);

	BENCHMARK_BEGIN(flags);

	while (1) {
		lfs_stride_bw[op](buf, access_size, stride_size, delay, count);
		timing->v += access_size * count;
		buf += stride_size * count;
		if (buf + access_size * count >= region_end)
			buf = ctx->addr;

		if (kthread_should_stop())
			break;
		schedule();
	}
	BENCHMARK_END(flags);
	wait_for_stop();

	return 0;
}

int overwrite_job(void *arg)
{
	int j;
	static unsigned long flags;
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	int skip = sbi->strided_size;
	u8 *buf, *buf_end;

	long *rbuf;
	int rpages;

	u8 hash = 0;

	kr_info("OVERWRITE_OP at smp id %d\n", smp_processor_id());
	rpages = roundup((2 * BASIC_OPS_TASK_COUNT + 1) * LATENCY_OPS_COUNT * sizeof(long), PAGE_SIZE) >> PAGE_SHIFT;
	/* fill report area page tables */
	for (j = 0; j < rpages; j++)
	{
		buf = (u8 *)sbi->rep->virt_addr + (j << PAGE_SHIFT);
		hash = hash ^ *buf;
	}

	/* fill page tables */
	for (j = 0; j < BASIC_OP_POOL_PAGES; j++)
	{
		buf = ctx->addr + (j << PAGE_SHIFT);
		hash = hash ^ *buf;
	}

	buf = ctx->addr;
	buf_end = buf + 4096;

	rbuf = (long *)(sbi->rep->virt_addr);
	BENCHMARK_BEGIN(flags);
	for (j = 0; j < 2*LATENCY_OPS_COUNT; j++)
	{
		rbuf[j] = repeat_256byte_ntstore(buf);
		buf += skip;
		if (buf >= buf_end)
			buf = buf_end - 4096;
	}
	BENCHMARK_END(flags);

	kr_info("Done. hash %d", hash);
	kr_info("LATTester_LAT_END:\n");
	wait_for_stop();
	return 0;
}


int read_after_write_job(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	static unsigned long flags;
	long access_size = ctx->sbi->access_size;
	long stride_size = ctx->sbi->strided_size;
	long delay = ctx->sbi->delay;
	uint8_t *buf = ctx->addr;
	uint8_t *region_end = buf + GLOBAL_WORKSET;
	long count = GLOBAL_WORKSET / access_size;
	struct timespec tstart, tend;
	unsigned int c_store_start_hi, c_store_start_lo;
	unsigned int c_ntload_start_hi, c_ntload_start_lo;
	unsigned int c_ntload_end_hi, c_ntload_end_lo;
	unsigned long c_store_start;
	unsigned long c_ntload_start, c_ntload_end;
	long pages, diff;
	int hash = 0;
	int i;
	uint64_t *cindex;
	uint64_t csize;
	int ret;

	if (stride_size * count > GLOBAL_WORKSET)
		count = GLOBAL_WORKSET / stride_size;

	if (count > LATENCY_OPS_COUNT)
		count = LATENCY_OPS_COUNT;

	pages = roundup(count * stride_size, PAGE_SIZE) >> PAGE_SHIFT;

	/* fill page tables */
	for (i = 0; i < pages; i++)
	{
		buf = ctx->addr + (i << PAGE_SHIFT);
		hash = hash ^ *buf;
	}

	buf = ctx->addr;

	kr_info("Working set begin: %p end: %p, size=%ld, stride=%ld, delay=%ld, batch=%ld\n", buf, region_end, access_size, stride_size, delay, count);

	BENCHMARK_BEGIN(flags);

#define RAW_BEFORE_WRITE 												\
	getrawmonotonic(&tstart); 											\
	asm volatile ( 														\
		"rdtscp \n\t" 													\
		"lfence \n\t" 													\
		"mov %%edx, %[hi]\n\t" 											\
		"mov %%eax, %[lo]\n\t" 											\
		: [hi] "=r" (c_store_start_hi), [lo] "=r" (c_store_start_lo) 	\
		: 																\
		: "rdx", "rax", "rcx" 											\
	);

#define RAW_BEFORE_READ													\
	asm volatile (														\
		"rdtscp \n\t"													\
		"lfence \n\t"													\
		"mov %%edx, %[hi]\n\t"											\
		"mov %%eax, %[lo]\n\t"											\
		: [hi] "=r" (c_ntload_start_hi), [lo] "=r" (c_ntload_start_lo)	\
		:																\
		: "rdx", "rax", "rcx"											\
	);

#define RAW_FINAL(job_name)												\
	asm volatile (														\
		"rdtscp \n\t"													\
		"lfence \n\t"													\
		"mov %%edx, %[hi]\n\t"											\
		"mov %%eax, %[lo]\n\t"											\
		: [hi] "=r" (c_ntload_end_hi), [lo] "=r" (c_ntload_end_lo)		\
		:																\
		: "rdx", "rax", "rcx"											\
	);																	\
	getrawmonotonic(&tend);												\
	diff = TIMEDIFF(tstart, tend);										\
	c_store_start = (((unsigned long)c_store_start_hi) << 32) | c_store_start_lo;			\
	c_ntload_start = (((unsigned long)c_ntload_start_hi) << 32) | c_ntload_start_lo;		\
	c_ntload_end = (((unsigned long)c_ntload_end_hi) << 32) | c_ntload_end_lo;				\
	kr_info("[%s] count %ld total %ld ns, average %ld ns, cycle %ld - %ld - %ld.\n",		\
		#job_name, count, diff, diff / count, c_store_start, c_ntload_start, c_ntload_end);	\


	// Separate RaW job
	RAW_BEFORE_WRITE
	stride_storeclwb(buf, access_size, stride_size, delay, count);
	asm volatile ("mfence \n" :::);
	RAW_BEFORE_READ
	stride_nt(buf, access_size, stride_size, delay, count);
	asm volatile ("mfence \n" :::);
	RAW_FINAL("raw-separate")

	// Naive RaW job
	RAW_BEFORE_WRITE
	RAW_BEFORE_READ
	stride_read_after_write(buf, access_size, stride_size, delay, count);
	asm volatile ("mfence \n" :::);
	RAW_FINAL("raw-combined")

	// Pointer chasing RaW job
	// No need to fill report fs page table, init_chasing_index will do that
	csize = access_size / CACHELINE_SIZE;
	cindex = (uint64_t *)(ctx->sbi->rep->virt_addr);
	ret = init_chasing_index(cindex, csize);
	if (ret != 0)
		return ret;

	RAW_BEFORE_WRITE
	chasing_storeclwb(buf, access_size, stride_size, count, cindex);
	asm volatile ("mfence \n" :::);
	RAW_BEFORE_READ
	chasing_loadnt(buf, access_size, stride_size, count, cindex);
	asm volatile ("mfence \n" :::);
	RAW_FINAL("raw-chasing")


	#undef RAW_BEFORE_WRITE
	#undef RAW_BEFORE_READ
	#undef RAW_FINAL

	BENCHMARK_END(flags);
	kr_info("LATTester_LAT_END: %d\n", hash);
	wait_for_stop();
	return 0;
}


int straight_write_job(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	uint64_t workset = 16384ULL * MB;
	uint64_t itersize = 512ULL * MB;
	static unsigned long flags;
	uint64_t write_start = sbi->write_start;
	uint64_t write_size = sbi->write_size;
	uint64_t diff;
	uint8_t *buf = ctx->addr;
	uint8_t *region_end = buf + workset;
	long count = workset / write_size;
	long iter;
	struct timespec tstart, tend;


	if (write_size * count > workset)
		count = workset / write_size; /* Write workset, 64 GB */

	iter = itersize / write_size;
	if (!iter)
		iter = 1;

	buf += write_start;
	if (buf + write_size > region_end) {
		write_size = region_end - buf;
		kr_info("Warning: write_start + write_size > 64 GB, write_size asjusted to %lld\n", write_size);
	}

	kr_info("Working set begin: %p size=%lld\n", buf, write_size);


	BENCHMARK_BEGIN(flags);

	getrawmonotonic(&tstart);
	while (count >= 0) {

		stride_nt(buf, write_size, 0, 0, iter);
		count -= iter;

		if (kthread_should_stop())
			break;
		schedule(); // Schedule with IRQ disabled?
	}
	getrawmonotonic(&tend);

	BENCHMARK_END(flags);
	diff = TIMEDIFF(tstart, tend);
	kr_info("%llx bytes written, total %lld ns.\n", workset + itersize + count * write_size, diff);
	wait_for_stop();

	return 0;
}

/* Non-ASM implmentation of LFSR */

#define TAP(a) (((a) == 0) ? 0 : ((1ull) << (((uint64_t)(a)) - (1ull))))

#define RAND_LFSR_DECL(BITS, T1, T2, T3, T4)											\
     inline static uint##BITS##_t RandLFSR##BITS(uint##BITS##_t *seed) {				\
      const uint##BITS##_t mask = TAP(T1) | TAP(T2) | TAP(T3) | TAP(T4);				\
      *seed = (*seed >> 1) ^ (uint##BITS##_t)(-(*seed & (uint##BITS##_t)(1)) & mask);	\
      return *seed;																		\
     }

RAND_LFSR_DECL(64, 64,63,61,60);

int align_bwjob(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	int dimm = ctx->core_id % 6;
	int channel = dimm % 3;
	int bit_length = sbi->bwsize_bit;
	long access_size = 1 << bit_length;
	int op = ctx->sbi->op;
	int imc2 = (sbi->align == ALIGN_PERIMC && dimm % 2 == 0) ? PAGE_SIZE : 0;
	int i;
	struct latencyfs_timing *timing;
	uint8_t *base = ctx->addr;
	uint8_t *buf = NULL;
	uint64_t r, r1;
	static unsigned long flags;
	uint64_t bitmask, bitmask1 = 0;
	uint64_t seed;

 	timing = &sbi->timing[ctx->core_id];
	if (sbi->align == ALIGN_GLOBAL) { // 0
		bitmask = (GLOBAL_WORKSET - 1) ^ (access_size - 1);
	} else if (sbi->align == ALIGN_PERTHREAD || sbi->align == ALIGN_NI) { // 1 6
		bitmask = (PERTHREAD_WORKSET - 1) ^ (access_size - 1);
		base += DIMM_SIZE * dimm;
	} else if (sbi->align == ALIGN_PERIMC) { // 3
		bitmask = (GLOBAL_WORKSET - 1) ^ (access_size - 1) ^ (1 << PAGE_SHIFT);
	} else if (sbi->align == ALIGN_PERDIMM || sbi->align == ALIGN_PERCHANNELGROUP) { //2 4
		bitmask = (PERTHREAD_WORKSET / PAGE_SIZE) - 1;
		bitmask1 = 4095 ^ (access_size - 1);
	} else if (sbi->align == ALIGN_NI_GLOBAL) {
		bitmask = (GLOBAL_WORKSET_NI - 1) ^ (access_size - 1);
	} else {
		kr_info("Unsupported align mode %d\n", sbi->align);
		return 0;
	}
	get_random_bytes(&seed, sizeof(unsigned long));

	BENCHMARK_BEGIN(flags);

	if (sbi->align == ALIGN_PERCHANNELGROUP) {
		while (1)
		{
			for (i = 0; i < 1024; i++) {
				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				imc2 = (r & (1ULL << 60)) ? 4096 * 3 : 0;
				buf = base + channel * PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1 + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				imc2 = (r & (1ULL << 60)) ? 4096 * 3 : 0;
				buf = base + channel * PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1 + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				imc2 = (r & (1ULL << 60)) ? 4096 * 3 : 0;
				buf = base + channel * PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1 + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				imc2 = (r & (1ULL << 60)) ? 4096 * 3 : 0;
				buf = base + channel * PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1 + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */
			}
			timing->v += 4096 * access_size;

			if (kthread_should_stop())
				break;
			schedule();
		}
	} else if (sbi->align == ALIGN_PERDIMM) {
		while (1) {
			for (i = 0; i < 1024; i++) {
				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				buf = base + dimm*PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1 + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				buf = base + dimm*PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				buf = base + dimm*PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				r1 = (r >> 52) & bitmask1;
				buf = base + dimm*PAGE_SIZE + ((r & bitmask) * 6 * PAGE_SIZE) + r1;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */
			}
			timing->v += 4096 * access_size;

			if (kthread_should_stop())
				break;
			schedule();
		}
	} else if (sbi->align == ALIGN_NI_GLOBAL) {
		while (1) {
			for (i = 0; i < 1024; i++) {
				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) +  DIMM_SIZE * (r % 6);
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) +  DIMM_SIZE * (r % 6);
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) +  DIMM_SIZE * (r % 6);
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) +  DIMM_SIZE * (r % 6);
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */
			}
			timing->v += 4096 * access_size;

			if (kthread_should_stop())
				break;
			schedule();
		}
	} else {
		while (1) { // Global, Per-Thread, Per-iMC, NI (PD)
			for (i = 0; i < 1024; i++) {
				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */

				r = RandLFSR64(&seed);
				buf = base + (r & bitmask) + imc2;
				lfs_stride_bw[op](buf, access_size, 0, 0, 1); /* under 4096 access needs unrolling */
			}
			timing->v += 4096 * access_size;

			if (kthread_should_stop())
				break;
			schedule();
		}
	}
	BENCHMARK_END(flags);
	wait_for_stop();

	return 0;
}

int cachefence_bwjob(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	static unsigned long flags;
	struct latencyfs_timing *timing = &sbi->timing[ctx->core_id];
	uint8_t *buf = ctx->addr;
	uint8_t *base = buf;
	uint8_t *buf_end = buf + PERTHREAD_WORKSET;
	int op = sbi->op;
	long access_size = sbi->access_size; /* set in proc.c */
	long count = PERTHREAD_CHECKSTOP / access_size / 4;
	long cache = sbi->clwb_rate;
	long fence = sbi->fence_rate;
	uint64_t seed, bitmask, r;
	int i;

	if (access_size * count > PERTHREAD_WORKSET)
		count = PERTHREAD_WORKSET / access_size;

	kr_info("Working set begin: %p end: %p, size=%ld, cache=%ld, fence=%ld, count = %ld\n", buf, buf_end, access_size, cache, fence, count);

	if (op) {
		get_random_bytes(&seed, sizeof(unsigned long));
		bitmask = (PERTHREAD_WORKSET - 1) ^ (access_size - 1);
	}

	BENCHMARK_BEGIN(flags);
	if (op) {
		while (1)
		{
			for (i = 0; i < count; i++)
			{
				r = RandLFSR64(&seed);
				buf = base + (r & bitmask);
				cachefence(buf, access_size, cache, fence);
				buf += access_size;
			}
			timing->v += access_size * count;
			if (kthread_should_stop())
				break;
			schedule();
		}
	} else {
		while (1)
		{
			for (i = 0; i < count; i++)
			{
				cachefence(buf, access_size, cache, fence);
				buf += access_size;
			}
			timing->v += access_size * count;
			if (buf + access_size * count >= buf_end)
				buf = ctx->addr;
			if (kthread_should_stop())
				break;
			schedule();
		}
	}
	BENCHMARK_END(flags);
	wait_for_stop();

	return 0;
}


int cacheprobe_job(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
 	static unsigned long flags;
	long stride = sbi->strided_size;
	long count = sbi->probe_count;
	long acount = count;
	uint8_t *buf = ctx->addr;
	uint8_t *buf_end;
	struct timespec tstart, tend;
	uint64_t diff;
	uint64_t iters; /* At least write 1 GB */
	uint64_t aiters;
	//uint64_t iters = 1; /* 256MB by default */
	long i, j;

	if (stride * count > DIMM_SIZE) {
		count = DIMM_SIZE / stride;
		aiters = acount / count;
	} else {
		aiters = 1;
	}
	

	buf_end = buf + stride * count;
	iters = 4 * MB / count;
	if (!iters) iters = 1;

	kr_info("CacheProbe buf %p->%p count 0x%ld(repeat %lld), stride 0x%lx, iters %lld\n", buf, buf_end, count, aiters, stride, iters);

	getrawmonotonic(&tstart);
	BENCHMARK_BEGIN(flags);

	for (j = 0; j < aiters; j++){
		buf = ctx->addr;
		for (i = 0; i < iters; i++) {
			cacheprobe(buf, buf_end, stride);
			cacheprobe(buf + 128, buf_end + 128, stride);
		}
	}
	BENCHMARK_END(flags);
	getrawmonotonic(&tend);

	diff = TIMEDIFF(tstart, tend);
	kr_info("%lld ns.\n", diff);

	return 0;
}


int imcprobe_job(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
 	static unsigned long flags;
	long access_size = 64 * sbi->probe_count;
	long count = (1UL * GB / access_size);
	uint8_t *buf = ctx->addr;
	uint8_t *buf_end = ctx->addr + access_size;
	struct timespec tstart, tend;
	uint64_t diff;
	int i;

	kr_info("iMCProbe begin %p, access_size %ld, iters %ld\n", buf, access_size, count);

	getrawmonotonic(&tstart);
	BENCHMARK_BEGIN(flags);
	for (i = 0; i < 8; i++) {
		imcprobe(buf, buf_end, count);
		if (kthread_should_stop())
			break;
		schedule(); // Schedule with IRQ disabled?
	}
	BENCHMARK_END(flags);
	getrawmonotonic(&tend);

	diff = TIMEDIFF(tstart, tend);
	kr_info("%lld ns.\n", diff);

	wait_for_stop();
	return 0;
}


int seq_bwjob(void *arg)
{
	struct latencyfs_worker_ctx *ctx = (struct latencyfs_worker_ctx *)arg;
	struct latency_sbi *sbi = ctx->sbi;
	static unsigned long flags;
	struct latencyfs_timing *timing = &sbi->timing[ctx->core_id];
	uint8_t *buf = ctx->addr;
	uint8_t *buf_end = buf + PERTHREAD_WORKSET;
	long step;
	int op = sbi->op;
	long access_size = sbi->access_size; /* set in proc.c */
	long count = PERTHREAD_CHECKSTOP / access_size;

	if (access_size * count > PERTHREAD_WORKSET)
		count = PERTHREAD_WORKSET / access_size;
	step = count * access_size;


	BENCHMARK_BEGIN(flags);
	while (1) {
		lfs_seq_bw[op](buf, buf+step, access_size);
		timing->v += step;
		buf += step;
		if (buf >= buf_end)
			buf = ctx->addr;

		if (kthread_should_stop())
			break;
		schedule();
	}
	BENCHMARK_END(flags);
	wait_for_stop();

	return 0;
}
