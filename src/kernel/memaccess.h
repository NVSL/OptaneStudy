/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LATTESTER_MEMACCESS_H
#define LATTESTER_MEMACCESS_H
/*
 * BRIEF DESCRIPTION
 *
 * Header for memory access functions
 *
 * Copyright 2019 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab
 */

// Size-BW Random (char *start_addr, long size, long count, long *rand_seed, long access_mask);
typedef void (*lfs_test_randfunc_t)(char *, long, long, long *, long);

// Size-BW Stride: latency_test(char * start_addr, long size, long stride, long bufsize)
typedef void (*lfs_test_stridefunc_t)(char *start_addr, long size, long skip, long delay, long count);

// Size-BW Random (char *start_addr, long size, long count, long *rand_seed, long access_mask);
typedef void (*lfs_test_seq_t)(char *, char *, long);

void cachefence(char *start_addr, long size, long cache, long fence);

void stride_load(char *start_addr, long size, long skip, long delay, long count);
void stride_store(char *start_addr, long size, long skip, long delay, long count);
void stride_storeclwb(char *start_addr, long size, long skip, long delay, long count);
void stride_nt(char *start_addr, long size, long skip, long delay, long count);

void sizebw_load(char *start_addr, long size, long count, long *rand_seed, long access_mask);
void sizebw_store(char *start_addr, long size, long count, long *rand_seed, long access_mask);
void sizebw_storeclwb(char *start_addr, long size, long count, long *rand_seed, long access_mask);
void sizebw_nt(char *start_addr, long size, long count, long *rand_seed, long access_mask);

void seq_load(char * start_addr, char *end_addr, long size);
void seq_store(char * start_addr, char *end_addr, long size);
void seq_clwb(char * start_addr, char *end_addr, long size);
void seq_nt(char * start_addr, char *end_addr, long size);

void cacheprobe(char *start_addr, char *end_addr, long stride);
void imcprobe(char *start_addr, char *end_addr, long loop);

void stride_read_after_write(char *start_addr, long size, long skip, long delay, long count);
// Pointer chasing tests
int init_chasing_index(uint64_t *cindex, uint64_t csize);
void chasing_storeclwb(char *start_addr, long size, long skip, long count, uint64_t *cindex);
void chasing_loadnt(char *start_addr, long size, long skip, long count, uint64_t *cindex);

#define SIZEBTNT_64_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"add $0x40, %%r10 \n"

#define SIZEBTNT_128_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"add $0x80, %%r10 \n"

#define SIZEBTNT_256_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"add $0x100, %%r10 \n"

#define SIZEBTNT_512_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n"

#define SIZEBTNT_1024_AVX512	\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x200(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x240(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x280(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x2c0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x300(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x340(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x380(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

#define SIZEBTSTFLUSH_64_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"add $0x40, %%r10 \n"

#define SIZEBTSTFLUSH_128_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"add $0x80, %%r10 \n"

#define SIZEBTSTFLUSH_256_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"clwb  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"clwb  0xc0(%%r9, %%r10) \n" \
				"add $0x100, %%r10 \n"

#define SIZEBTSTFLUSH_512_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"clwb  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"clwb  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"clwb  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"clwb  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"clwb  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"clwb  0x1c0(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n"

#define SIZEBTSTFLUSH_1024_AVX512	\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"clwb  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"clwb  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"clwb  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"clwb  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"clwb  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"clwb  0x1c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x200(%%r9, %%r10) \n" \
				"clwb  0x200(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x240(%%r9, %%r10) \n" \
				"clwb  0x240(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x280(%%r9, %%r10) \n" \
				"clwb  0x280(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x2c0(%%r9, %%r10) \n" \
				"clwb  0x2c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x300(%%r9, %%r10) \n" \
				"clwb  0x300(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x340(%%r9, %%r10) \n" \
				"clwb  0x340(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x380(%%r9, %%r10) \n" \
				"clwb  0x380(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x3c0(%%r9, %%r10) \n" \
				"clwb  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

#define SIZEBTST_64_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"add $0x40, %%r10 \n"

#define SIZEBTST_128_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"add $0x80, %%r10 \n"

#define SIZEBTST_256_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"add $0x100, %%r10 \n"

#define SIZEBTST_512_AVX512		\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n"

#define SIZEBTST_1024_AVX512	\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x200(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x240(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x280(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x2c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x300(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x340(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x380(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

#define SIZEBTLD_64_AVX512	\
				"vmovntdqa 0x0(%%r9, %%r10), %%zmm0 \n" \
				"add $0x40, %%r10 \n"

#define SIZEBTLD_128_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"add $0x80, %%r10 \n"

#define SIZEBTLD_256_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"add $0x100, %%r10 \n"

#define SIZEBTLD_512_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n" \
				"vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n" \
				"vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n" \
				"add $0x200, %%r10 \n"

#define SIZEBTLD_1024_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n" \
				"vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n" \
				"vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n" \
				"vmovntdqa  0x200(%%r9, %%r10), %%zmm8 \n" \
				"vmovntdqa  0x240(%%r9, %%r10), %%zmm9 \n" \
				"vmovntdqa  0x280(%%r9, %%r10), %%zmm10 \n" \
				"vmovntdqa  0x2c0(%%r9, %%r10), %%zmm11 \n" \
				"vmovntdqa  0x300(%%r9, %%r10), %%zmm12 \n" \
				"vmovntdqa  0x340(%%r9, %%r10), %%zmm13 \n" \
				"vmovntdqa  0x380(%%r9, %%r10), %%zmm14 \n" \
				"vmovntdqa  0x3c0(%%r9, %%r10), %%zmm15 \n" \
				"add $0x400, %%r10 \n"

#define SIZEBT_NT_64	"movnti %[random], 0x0(%%r9, %%r10) \n" \
			"movnti %[random], 0x8(%%r9, %%r10) \n" \
			"movnti %[random], 0x10(%%r9, %%r10) \n" \
			"movnti %[random], 0x18(%%r9, %%r10) \n" \
			"movnti %[random], 0x20(%%r9, %%r10) \n" \
			"movnti %[random], 0x28(%%r9, %%r10) \n" \
			"movnti %[random], 0x30(%%r9, %%r10) \n" \
			"movnti %[random], 0x38(%%r9, %%r10) \n" \
			"add $0x40, %%r10 \n"

#define SIZEBT_LOAD_64	"mov 0x0(%%r9, %%r10),  %%r13  \n" \
			"mov 0x8(%%r9, %%r10),  %%r13  \n" \
			"mov 0x10(%%r9, %%r10), %%r13  \n" \
			"mov 0x18(%%r9, %%r10), %%r13  \n" \
			"mov 0x20(%%r9, %%r10), %%r13  \n" \
			"mov 0x28(%%r9, %%r10), %%r13  \n" \
			"mov 0x30(%%r9, %%r10), %%r13  \n" \
			"mov 0x38(%%r9, %%r10), %%r13  \n" \

// Benchmark functions map
static const char *bench_size_map[] = {
	"load",
	"non-temporal",
	"store",
#ifdef AEP_SUPPORTED
	"store-flush"
#endif
};

static lfs_test_stridefunc_t lfs_stride_bw[] = {
	&stride_load,
	&stride_nt,
	&stride_store,
#ifdef AEP_SUPPORTED
	&stride_storeclwb,
#endif
};

static lfs_test_randfunc_t lfs_size_bw[] = {
	&sizebw_load,
	&sizebw_nt,
	&sizebw_store,
#ifdef AEP_SUPPORTED
	&sizebw_storeclwb,
#endif
};

static lfs_test_seq_t lfs_seq_bw[] = {
	&seq_load,
	&seq_nt,
	&seq_store,
#ifdef AEP_SUPPORTED
	&seq_clwb
#endif
};

enum bench_size_tasks
{
	bench_load,
	bench_store_nt,
	bench_store,
#ifdef AEP_SUPPORTED
	bench_store_flush,
#endif
	BENCH_SIZE_TASK_COUNT
};

#endif
