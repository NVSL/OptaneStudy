/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LATTESTER_H
#define LATTESTER_H
/*
 * BRIEF DESCRIPTION
 *
 * Header for latfs
 *
 * Copyright 2019 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab
 */

#if __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#endif

#define CACHELINE_BITS			(6)
#define CACHELINE_SIZE			(64)
#define LATENCY_OPS_COUNT		1048576L

/*
 * Content on ReportFS
 *
 * Task 1: For random latency tests
 * | OP_COUNT * 8    |
 * +-----------------+---------------+---------------+-----+---------------+
 * |   Random Pool   | Report Task 0 | Report Task 1 | ... | Report Task N |
 * +-----------------+---------------+---------------+-----+---------------+
 *
 * Task 2: Strided latency
 * | OP_COUNT * 8  |
 * +---------------+---------------+-----+---------------+
 * | Report Task 0 | Report Task 1 | ... | Report Task N |
 * +---------------+---------------+-----+---------------+
 *
 * Task 5:Overwrite latency
 * | 2 * OP_COUNT * 8 |
 * +--------------------+
 * | Per-access latency |
 * +--------------------+
 *
 *
 */

#define OPT_NOPARAM	1
#define OPT_INT		2
#define OPT_STRING	4

/* LATENCY_OPS_COUNT = Subtasks (seq/rand) */
#define BASIC_OP_POOL_BITS		30
#define BASIC_OP_POOL_SIZE		(1L << POOL_BITS)  /* Size of the test region */
#define BASIC_OP_POOL_PAGE_BIT		(BASIC_OP_POOL_BITS - PAGE_SHIFT)
#define BASIC_OP_POOL_PAGES		(1L << BASIC_OP_POOL_PAGE_BIT) /* # Pages */

#define BASIC_OP_POOL_LINE_BITS		(BASIC_OP_POOL_BITS - CACHELINE_BITS)
#define BASIC_OP_MASK		0x3FFFFFC0 /*0b1{24, POOL_LINE_BITS}0{6, CACHELINE_BITS} */

#define PRETHREAD_BIT			31 /* 2GB/Thread */
//#define PRETHREAD_BIT			33 /* 8GB/Thread */
#define GLOBAL_BIT			34 /* 16GB/Global */
//#define GLOBAL_BIT			38 /* 256GB/Global */
#define GLOBAL_BIT_NI			32 /* 4GB/DIMM, 24 GB with software interleaving */
//#define PRETHREAD_BIT			28 /* 256MB/Thread */

#define GLOBAL_WORKSET			(1L << GLOBAL_BIT)
#define PERTHREAD_WORKSET		(1L << PRETHREAD_BIT)
#define PERDIMMGROUP_WORKSET		(PERTHREAD_WORKSET * 6)

//#define PERTHREAD_MASK		0xFFFFFC0 //256M
#define PERTHREAD_MASK			0x7FFFFFC0 //2G

#define PERTHREAD_CHECKSTOP		(1L << 25) /* yield after wrtting 32MB */

#define MB			0x100000
#define GB			0x40000000

#define DIMM_SIZE				(256UL * GB)
#define GLOBAL_WORKSET_NI		(1UL << GLOBAL_BIT_NI)


#define LFS_THREAD_MAX		48
#define LFS_ACCESS_MAX_BITS	22
#define LFS_ACCESS_MAX		(1 << LFS_ACCESS_MAX_BITS)  // 512KB * 4096 (rng_array) = 2G
#define LFS_RUNTIME_MAX		600
#define LFS_DELAY_MAX		1000000

#define LFS_PERMRAND_ENTRIES		0x1000
#define LFS_PERMRAND_SIZE			LFS_PERMRAND_ENTRIES * 4
#define LFS_PERMRAND_SIZE_IMM	"$0x4000,"



#define LAT_ASSERT(x)							\
	if (!(x)) {							\
		dump_stack();						\
		printk(KERN_WARNING "assertion failed %s:%d: %s\n",	\
	               __FILE__, __LINE__, #x);				\
	}


#define rdtscp(low,high,aux) \
     __asm__ __volatile__ (".byte 0x0f,0x01,0xf9" : "=a" (low), "=d" (high), "=c" (aux))


/* since Skylake, Intel uses non-inclusive last-level cache, drop cache before testing */
static inline void drop_cache(void)
{
	asm volatile("wbinvd": : :"memory");
	asm volatile("mfence": : :"memory");
}

typedef enum {
	TASK_INVALID = 0,
	TASK_BASIC_OP,
	TASK_STRIDED_LAT,
	TASK_STRIDED_BW,
	TASK_SIZE_BW,
	TASK_OVERWRITE,
	TASK_READ_AFTER_WRITE,
	TASK_STRAIGHT_WRITE,
	TASK_THBIND,
	TASK_FLUSHFENCE,
	TASK_CACHEPROBE,
	TASK_IMCPROBE,
	TASK_SEQ,

	TASK_COUNT
} lattest_task;

typedef enum {
	ALIGN_GLOBAL = 0, /* All threads writes to entire dataset, interleaved */
	ALIGN_PERTHREAD, /* Thread-local dataset (6 DIMMs), interleaved */
	ALIGN_PERDIMM, /* Per-DIMM dataset, interleaved */
	ALIGN_PERIMC, /* Per-iMC (3 DIMMs) dataset, interleaved */
	ALIGN_PERCHANNELGROUP, /* Two DIMMs on two iMCs, interleaved */
	ALIGN_NI_GLOBAL, /* ALIGN_GLOBAL in non-interleaved mode */
	ALIGN_NI, /* ALIGN_PERTHREAD in non-interleaved mode */

	ALIGN_INVALID
} align_mode;

typedef int(bench_func_t)(void *);

/* Fine if the struct is not cacheline aligned */
struct latencyfs_timing {
	uint64_t v;
	char padding[56]; /* 64 - 8 */
}__attribute((__packed__));

struct report_sbi {
	struct super_block *sb;
	struct block_device *s_bdev;
	void		*virt_addr;
	unsigned long	initsize;
	uint64_t	phys_addr;
};

struct latency_sbi
{
	struct super_block *sb;
	struct block_device *s_bdev;
	void *virt_addr;
	unsigned long initsize;
	uint64_t phys_addr;
	uint64_t write_start;
	uint64_t write_size; /* TODO: write_size cannot be over 4GB */
	uint64_t probe_count;
	uint64_t access_size;
	uint64_t strided_size;
	int align;
	int clwb_rate;
	int fence_rate;
	int worker_cnt;
	int runtime;
	int bwsize_bit;
	int delay;
	int op;

	struct latencyfs_timing *timing;
	struct report_sbi *rep;
	struct task_struct **workers;
	struct latencyfs_worker_ctx *ctx;
};

struct latencyfs_worker_ctx {
	int core_id;
	uint8_t *addr;
	uint8_t *seed_addr; /* for multi-threaded tasks only */
	struct latency_sbi *sbi;
};

int latency_job(void *arg);
int strided_latjob(void *arg);
int strided_bwjob(void *arg);
int sizebw_job(void *arg);
int latencyfs_proc_init(void);
int overwrite_job(void *arg);
int read_after_write_job(void *arg);
int straight_write_job(void *arg);
int align_bwjob(void *arg);
int cachefence_bwjob(void *arg);
int cacheprobe_job(void *arg);
int imcprobe_job(void *arg);
int seq_bwjob(void *arg);

struct latency_option {
	const char *name;
	unsigned int has_arg;
	int val;
};

#if __KERNEL__
/* misc */
inline void latencyfs_prealloc_large(uint8_t *addr, uint64_t size, uint64_t bit_mask);
inline unsigned long fastrand(unsigned long *x, unsigned long *y, unsigned long *z, uint64_t bit_mask);
inline void latencyfs_prealloc_global_permutation_array(int size);

int latencyfs_getopt(const char *caller, char **options, const struct latency_option *opts,
		char **optopt, char **optarg, unsigned long *value);
void latencyfs_start_task(struct latency_sbi *sbi, int task, int threads);

int latencyfs_print_help(struct seq_file *seq, void *v);
#endif

#endif // LATTESTER_H
