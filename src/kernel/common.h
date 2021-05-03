/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LATTESTER_COMMON_H
#define LATTESTER_COMMON_H
/*
 * BRIEF DESCRIPTION
 *
 * Header for commonly used defines
 *
 * Copyright 2019 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab
 */

// #ifdef __KERNEL__
#if 0
  #include <asm/fpu/api.h>
  #define KERNEL_BEGIN \
	 kernel_fpu_begin();
  #define KERNEL_END \
	 kernel_fpu_end();
#else
  #define KERNEL_BEGIN do { } while (0);
  #define KERNEL_END do { } while (0);
#endif

/* Assembly instructions utilize the following registers:
 * rsi: memory address
 * rax, rdx, rcx, r8d and r9d: timing
 * rdx: populating cache-lines
 * ymm0: streaming instructions
 */
#define REGISTERS "rsi", "rax", "rdx", "rcx", "r8", "r9", "ymm0"
#define REGISTERS_NONSSE "rsi", "rax", "rdx", "rcx", "r8", "r9"

/* ymm0: 256-bit register (requires AVX support)
 * vbroadcastsd: VEX.256-bit version (r[0] = r[1] = r[2] = r[3] = v)
 */
#define LOAD_VALUE              "vbroadcastsd %[value], %%ymm0 \n"
#define LOAD_ADDR               "mov %[memarea], %%rsi \n" \
                                "mfence \n"

#define FLUSH_CACHE_LINE        "clflush 0*32(%%rsi) \n" \
                                "clflush 2*32(%%rsi) \n" \
                                "clflush 4*32(%%rsi) \n" \
                                "clflush 6*32(%%rsi) \n" \
                                "mfence \n"

#define LOAD_CACHE_LINE         "movq 0*8(%%rsi), %%rdx \n" \
                                "movq 1*8(%%rsi), %%rdx \n" \
                                "movq 2*8(%%rsi), %%rdx \n" \
                                "movq 3*8(%%rsi), %%rdx \n" \
                                "movq 4*8(%%rsi), %%rdx \n" \
                                "movq 5*8(%%rsi), %%rdx \n" \
                                "movq 6*8(%%rsi), %%rdx \n" \
                                "movq 7*8(%%rsi), %%rdx \n" \
                                "movq 8*8(%%rsi), %%rdx \n" \
                                "movq 9*8(%%rsi), %%rdx \n" \
                                "movq 10*8(%%rsi), %%rdx \n" \
                                "movq 11*8(%%rsi), %%rdx \n" \
                                "movq 12*8(%%rsi), %%rdx \n" \
                                "movq 13*8(%%rsi), %%rdx \n" \
                                "movq 14*8(%%rsi), %%rdx \n" \
                                "movq 15*8(%%rsi), %%rdx \n" \
                                "movq 16*8(%%rsi), %%rdx \n" \
                                "movq 17*8(%%rsi), %%rdx \n" \
                                "movq 18*8(%%rsi), %%rdx \n" \
                                "movq 19*8(%%rsi), %%rdx \n" \
                                "movq 20*8(%%rsi), %%rdx \n" \
                                "movq 21*8(%%rsi), %%rdx \n" \
                                "movq 22*8(%%rsi), %%rdx \n" \
                                "movq 23*8(%%rsi), %%rdx \n" \
                                "movq 24*8(%%rsi), %%rdx \n" \
                                "movq 25*8(%%rsi), %%rdx \n" \
                                "movq 26*8(%%rsi), %%rdx \n" \
                                "movq 27*8(%%rsi), %%rdx \n" \
                                "movq 28*8(%%rsi), %%rdx \n" \
                                "movq 29*8(%%rsi), %%rdx \n" \
                                "movq 30*8(%%rsi), %%rdx \n" \
                                "movq 31*8(%%rsi), %%rdx \n" \
                                "mfence \n"

#define CLEAR_PIPELINE          "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n"

/* rdtscp: reads current timestamp to EDX:EAX and also sets ECX
 * higher 32-bits of RAX, RDX and RCX are cleared
 */
#define TIMING_BEG              "rdtscp \n" \
                                "lfence \n" \
                                "mov %%edx, %%r9d \n" \
                                "mov %%eax, %%r8d \n"

/* r9d = old EDX
 * r8d = old EAX
 * Here is what we do to compute t1 and t2:
 * - RDX holds t2
 * - RAX holds t1
 */
#define TIMING_END              "mfence \n" \
                                "rdtscp \n" \
                                "lfence \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

#define TIMING_END_NOFENCE      "rdtscp \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

/*
 * 64-byte benchmarks
 */
uint64_t store_64byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    // vmovdqa: 32-byte store to memory
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflush (%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clwb (%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflushopt (%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t nstore_64byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /*
     * vmovntpd: 32-byte non-temporal store (check below)
     * https://software.intel.com/en-us/node/524246
     */
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovdqa 1*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte_fence_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    /*
     * Requires avx2
     * https://www.felixcloutier.com/x86/MOVNTDQA.html
     */
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}


uint64_t baseline(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(CLEAR_PIPELINE
        TIMING_BEG
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        :: REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        "clflush (%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq 0*8(%%rsi), %%rdx \n"
        "movq 1*8(%%rsi), %%rdx \n"
        "movq 2*8(%%rsi), %%rdx \n"
        "movq 3*8(%%rsi), %%rdx \n"
        "movq 4*8(%%rsi), %%rdx \n"
        "movq 5*8(%%rsi), %%rdx \n"
        "movq 6*8(%%rsi), %%rdx \n"
        "movq 7*8(%%rsi), %%rdx \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t(*latency_funcs_64byte[])(char *) = {
    &load_64byte_fence, // load + fence
    &load_64byte_fence_nt, // non-temporal load + fence
    &store_64byte_fence, // store + fence
    &store_64byte_clflush, // store + clflush
    &store_64byte_clwb, // store + clwb
    &store_64byte_clflushopt, // store + clflushopt
    &nstore_64byte_fence, // non-temporal store + fence
    &store_64byte_fence_movq, // store + fence (movq)
    &store_64byte_clflush_movq, // store - clflush (movq)
    &load_64byte_fence_movq, // load + fence (movq)
    &baseline
};

/*
 * 128-byte benchmarks
 */
uint64_t store_128byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflush (%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clflush 2*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clwb (%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clwb 2*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflushopt (%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clflushopt 2*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t nstore_128byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        "vmovntpd %%ymm0, 2*32(%%rsi) \n"
        "vmovntpd %%ymm0, 3*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovdqa 2*32(%%rsi), %%ymm1 \n"
        "vmovdqa 3*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte_fence_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        "movq %%rdx, 8*8(%%rsi) \n"
        "movq %%rdx, 9*8(%%rsi) \n"
        "movq %%rdx, 10*8(%%rsi) \n"
        "movq %%rdx, 11*8(%%rsi) \n"
        "movq %%rdx, 12*8(%%rsi) \n"
        "movq %%rdx, 13*8(%%rsi) \n"
        "movq %%rdx, 14*8(%%rsi) \n"
        "movq %%rdx, 15*8(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        "clflush (%%rsi) \n"
        "movq %%rdx, 8*8(%%rsi) \n"
        "movq %%rdx, 9*8(%%rsi) \n"
        "movq %%rdx, 10*8(%%rsi) \n"
        "movq %%rdx, 11*8(%%rsi) \n"
        "movq %%rdx, 12*8(%%rsi) \n"
        "movq %%rdx, 13*8(%%rsi) \n"
        "movq %%rdx, 14*8(%%rsi) \n"
        "movq %%rdx, 15*8(%%rsi) \n"
        "clflush 8*8(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq 0*8(%%rsi), %%rdx \n"
        "movq 1*8(%%rsi), %%rdx \n"
        "movq 2*8(%%rsi), %%rdx \n"
        "movq 3*8(%%rsi), %%rdx \n"
        "movq 4*8(%%rsi), %%rdx \n"
        "movq 5*8(%%rsi), %%rdx \n"
        "movq 6*8(%%rsi), %%rdx \n"
        "movq 7*8(%%rsi), %%rdx \n"
        "movq 8*8(%%rsi), %%rdx \n"
        "movq 9*8(%%rsi), %%rdx \n"
        "movq 10*8(%%rsi), %%rdx \n"
        "movq 11*8(%%rsi), %%rdx \n"
        "movq 12*8(%%rsi), %%rdx \n"
        "movq 13*8(%%rsi), %%rdx \n"
        "movq 14*8(%%rsi), %%rdx \n"
        "movq 15*8(%%rsi), %%rdx \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t(*latency_funcs_128byte[])(char *) = {
    &load_128byte_fence,
    &load_128byte_fence_nt,
    &store_128byte_fence,
    &store_128byte_clflush,
    &store_128byte_clwb,
    &store_128byte_clflushopt,
    &nstore_128byte_fence,
    &store_128byte_fence_movq,
    &store_128byte_clflush_movq,
    &load_128byte_fence_movq,
    &baseline
};

/*
 * 256-byte benchmarks
 */
uint64_t store_256byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "vmovdqa %%ymm0, 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 5*32(%%rsi) \n"
        "vmovdqa %%ymm0, 6*32(%%rsi) \n"
        "vmovdqa %%ymm0, 7*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}



uint64_t store_256byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflush 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clflush 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 5*32(%%rsi) \n"
        "clflush 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 6*32(%%rsi) \n"
        "vmovdqa %%ymm0, 7*32(%%rsi) \n"
        "clflush 6*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clwb 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clwb 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 5*32(%%rsi) \n"
        "clwb 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 6*32(%%rsi) \n"
        "vmovdqa %%ymm0, 7*32(%%rsi) \n"
        "clwb 6*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clflushopt 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 3*32(%%rsi) \n"
        "clflushopt 2*32(%%rsi) \n"
        "vmovdqa %%ymm0, 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 5*32(%%rsi) \n"
        "clflushopt 4*32(%%rsi) \n"
        "vmovdqa %%ymm0, 6*32(%%rsi) \n"
        "vmovdqa %%ymm0, 7*32(%%rsi) \n"
        "clflushopt 6*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t nstore_256byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        "vmovntpd %%ymm0, 2*32(%%rsi) \n"
        "vmovntpd %%ymm0, 3*32(%%rsi) \n"
        "vmovntpd %%ymm0, 4*32(%%rsi) \n"
        "vmovntpd %%ymm0, 5*32(%%rsi) \n"
        "vmovntpd %%ymm0, 6*32(%%rsi) \n"
        "vmovntpd %%ymm0, 7*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_256byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovdqa 2*32(%%rsi), %%ymm1 \n"
        "vmovdqa 3*32(%%rsi), %%ymm1 \n"
        "vmovdqa 4*32(%%rsi), %%ymm1 \n"
        "vmovdqa 5*32(%%rsi), %%ymm1 \n"
        "vmovdqa 6*32(%%rsi), %%ymm1 \n"
        "vmovdqa 7*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_256byte_fence_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 4*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 5*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 6*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 7*32(%%rsi), %%ymm1 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        "movq %%rdx, 8*8(%%rsi) \n"
        "movq %%rdx, 9*8(%%rsi) \n"
        "movq %%rdx, 10*8(%%rsi) \n"
        "movq %%rdx, 11*8(%%rsi) \n"
        "movq %%rdx, 12*8(%%rsi) \n"
        "movq %%rdx, 13*8(%%rsi) \n"
        "movq %%rdx, 14*8(%%rsi) \n"
        "movq %%rdx, 15*8(%%rsi) \n"
        "movq %%rdx, 16*8(%%rsi) \n"
        "movq %%rdx, 17*8(%%rsi) \n"
        "movq %%rdx, 18*8(%%rsi) \n"
        "movq %%rdx, 19*8(%%rsi) \n"
        "movq %%rdx, 20*8(%%rsi) \n"
        "movq %%rdx, 21*8(%%rsi) \n"
        "movq %%rdx, 22*8(%%rsi) \n"
        "movq %%rdx, 23*8(%%rsi) \n"
        "movq %%rdx, 24*8(%%rsi) \n"
        "movq %%rdx, 25*8(%%rsi) \n"
        "movq %%rdx, 26*8(%%rsi) \n"
        "movq %%rdx, 27*8(%%rsi) \n"
        "movq %%rdx, 28*8(%%rsi) \n"
        "movq %%rdx, 29*8(%%rsi) \n"
        "movq %%rdx, 30*8(%%rsi) \n"
        "movq %%rdx, 31*8(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq %%rdx, 0*8(%%rsi) \n"
        "movq %%rdx, 1*8(%%rsi) \n"
        "movq %%rdx, 2*8(%%rsi) \n"
        "movq %%rdx, 3*8(%%rsi) \n"
        "movq %%rdx, 4*8(%%rsi) \n"
        "movq %%rdx, 5*8(%%rsi) \n"
        "movq %%rdx, 6*8(%%rsi) \n"
        "movq %%rdx, 7*8(%%rsi) \n"
        "clflush 0*8(%%rsi) \n"
        "movq %%rdx, 8*8(%%rsi) \n"
        "movq %%rdx, 9*8(%%rsi) \n"
        "movq %%rdx, 10*8(%%rsi) \n"
        "movq %%rdx, 11*8(%%rsi) \n"
        "movq %%rdx, 12*8(%%rsi) \n"
        "movq %%rdx, 13*8(%%rsi) \n"
        "movq %%rdx, 14*8(%%rsi) \n"
        "movq %%rdx, 15*8(%%rsi) \n"
        "clflush 8*8(%%rsi) \n"
        "movq %%rdx, 16*8(%%rsi) \n"
        "movq %%rdx, 17*8(%%rsi) \n"
        "movq %%rdx, 18*8(%%rsi) \n"
        "movq %%rdx, 19*8(%%rsi) \n"
        "movq %%rdx, 20*8(%%rsi) \n"
        "movq %%rdx, 21*8(%%rsi) \n"
        "movq %%rdx, 22*8(%%rsi) \n"
        "movq %%rdx, 23*8(%%rsi) \n"
        "clflush 16*8(%%rsi) \n"
        "movq %%rdx, 24*8(%%rsi) \n"
        "movq %%rdx, 25*8(%%rsi) \n"
        "movq %%rdx, 26*8(%%rsi) \n"
        "movq %%rdx, 27*8(%%rsi) \n"
        "movq %%rdx, 28*8(%%rsi) \n"
        "movq %%rdx, 29*8(%%rsi) \n"
        "movq %%rdx, 30*8(%%rsi) \n"
        "movq %%rdx, 31*8(%%rsi) \n"
        "clflush 24*8(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_256byte_fence_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "movq 0*8(%%rsi), %%rdx \n"
        "movq 1*8(%%rsi), %%rdx \n"
        "movq 2*8(%%rsi), %%rdx \n"
        "movq 3*8(%%rsi), %%rdx \n"
        "movq 4*8(%%rsi), %%rdx \n"
        "movq 5*8(%%rsi), %%rdx \n"
        "movq 6*8(%%rsi), %%rdx \n"
        "movq 7*8(%%rsi), %%rdx \n"
        "movq 8*8(%%rsi), %%rdx \n"
        "movq 9*8(%%rsi), %%rdx \n"
        "movq 10*8(%%rsi), %%rdx \n"
        "movq 11*8(%%rsi), %%rdx \n"
        "movq 12*8(%%rsi), %%rdx \n"
        "movq 13*8(%%rsi), %%rdx \n"
        "movq 14*8(%%rsi), %%rdx \n"
        "movq 15*8(%%rsi), %%rdx \n"
        "movq 16*8(%%rsi), %%rdx \n"
        "movq 17*8(%%rsi), %%rdx \n"
        "movq 18*8(%%rsi), %%rdx \n"
        "movq 19*8(%%rsi), %%rdx \n"
        "movq 20*8(%%rsi), %%rdx \n"
        "movq 21*8(%%rsi), %%rdx \n"
        "movq 22*8(%%rsi), %%rdx \n"
        "movq 23*8(%%rsi), %%rdx \n"
        "movq 24*8(%%rsi), %%rdx \n"
        "movq 25*8(%%rsi), %%rdx \n"
        "movq 26*8(%%rsi), %%rdx \n"
        "movq 27*8(%%rsi), %%rdx \n"
        "movq 28*8(%%rsi), %%rdx \n"
        "movq 29*8(%%rsi), %%rdx \n"
        "movq 30*8(%%rsi), %%rdx \n"
        "movq 31*8(%%rsi), %%rdx \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr)
        : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t(*latency_funcs_256byte[])(char *) = {
    &load_256byte_fence,
    &load_256byte_fence_nt,
    &store_256byte_fence,
    &store_256byte_clflush,
    &store_256byte_clwb,
    &store_256byte_clflushopt,
    &nstore_256byte_fence,
    &store_256byte_fence_movq,
    &store_256byte_clflush_movq,
    &load_256byte_fence_movq,
    &baseline
};

// Benchmark functions map
const char *latency_bench_map[] = {
    "load-fence",
    "ntload-fence",
    "store-fence",
    "store-clflush",
    "store-clwb",
    "store-clflushopt",
    "nstore-fence",
    "store-fence-movq",
    "store-clflush-movq",
    "load-fence-movq",
    "baseline"
};

// Kernel-level task lists

enum latency_tasks {
    load_fence_64 = 0,
    ntload_fence_64,
    store_fence_64,
    store_clflush_64,
#ifdef AEP_SUPPORTED
    store_clwb_64,
    store_clflushopt_64,
#endif
    nstore_fence_64,
    store_fence_movq_64,
    store_clflush_movq_64,
    load_fence_movq_64,

    load_fence_128,
    ntload_fence_128,
    store_fence_128,
    store_clflush_128,
#ifdef AEP_SUPPORTED
    store_clwb_128,
    store_clflushopt_128,
#endif
    nstore_fence_128,
    store_fence_movq_128,
    store_clflush_movq_128,
    load_fence_movq_128,

    load_fence_256,
    ntload_fence_256,
    store_fence_256,
    store_clflush_256,
#ifdef AEP_SUPPORTED
    store_clwb_256,
    store_clflushopt_256,
#endif
    nstore_fence_256,
    store_fence_movq_256,
    store_clflush_movq_256,
    load_fence_movq_256,
    task_baseline,

    BASIC_OPS_TASK_COUNT
};

static const int latency_tasks_skip[BASIC_OPS_TASK_COUNT] = {
64,
64,
64,
64,
#ifdef AEP_SUPPORTED
64,
64,
#endif
64,
64,
64,
64,

128,
128,
128,
128,
#ifdef AEP_SUPPORTED
128,
128,
#endif
128,
128,
128,
128,

256,
256,
256,
256,
#ifdef AEP_SUPPORTED
256,
256,
#endif
256,
256,
256,
256,
0
};

static const char *latency_tasks_str[BASIC_OPS_TASK_COUNT] = {
    "load-fence-64",
    "ntload-fence-64",
    "store-fence-64",
    "store-clflush-64",
#ifdef AEP_SUPPORTED
    "store-clwb-64",
    "store-clflushopt-64",
#endif
    "nstore-fence-64",
    "store-fence-movq-64",
    "store-clflush-movq-64",
    "load-fence-movq-64",

    "load-fence-128",
    "ntload-fence-128",
    "store-fence-128",
    "store-clflush-128",
#ifdef AEP_SUPPORTED
    "store-clwb-128",
    "store-clflushopt-128",
#endif
    "nstore-fence-128",
    "store-fence-movq-128",
    "store-clflush-movq-128",
    "load-fence-movq-128",

    "load-fence-256",
    "ntload-fence-256",
    "store-fence-256",
    "store-clflush-256",
#ifdef AEP_SUPPORTED
    "store-clwb-256",
    "store-clflushopt-256",
#endif
    "nstore-fence-256",
    "store-fence-movq-256",
    "store-clflush-movq-256",
    "load-fence-movq-256",
    "baseline"
};


uint64_t (*bench_func[BASIC_OPS_TASK_COUNT])(char *) = {
    &load_64byte_fence,
    &load_64byte_fence_nt,
    &store_64byte_fence,
    &store_64byte_clflush,
#ifdef AEP_SUPPORTED
    &store_64byte_clwb,
    &store_64byte_clflushopt,
#endif
    &nstore_64byte_fence,
    &store_64byte_fence_movq,
    &store_64byte_clflush_movq,
    &load_64byte_fence_movq,

    &load_128byte_fence,
    &load_128byte_fence_nt,
    &store_128byte_fence,
    &store_128byte_clflush,
#ifdef AEP_SUPPORTED
    &store_128byte_clwb,
    &store_128byte_clflushopt,
#endif
    &nstore_128byte_fence,
    &store_128byte_fence_movq,
    &store_128byte_clflush_movq,
    &load_128byte_fence_movq,


    &load_256byte_fence,
    &load_256byte_fence_nt,
    &store_256byte_fence,
    &store_256byte_clflush,
#ifdef AEP_SUPPORTED
    &store_256byte_clwb,
    &store_256byte_clflushopt,
#endif
    &nstore_256byte_fence,
    &store_256byte_fence_movq,
    &store_256byte_clflush_movq,
    &load_256byte_fence_movq,
    &baseline
};

/*
 * 256-byte benchmarks
 */
uint64_t repeat_256byte_ntstore(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        "vmovntpd %%ymm0, 2*32(%%rsi) \n"
        "vmovntpd %%ymm0, 3*32(%%rsi) \n"
        "vmovntpd %%ymm0, 4*32(%%rsi) \n"
        "vmovntpd %%ymm0, 5*32(%%rsi) \n"
        "vmovntpd %%ymm0, 6*32(%%rsi) \n"
        "vmovntpd %%ymm0, 7*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

/*
 * 256-byte benchmarks
 */
uint64_t repeat_256byte_load(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        TIMING_BEG
        "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 2*32(%%rsi), %%ymm2 \n"
        "vmovntdqa 3*32(%%rsi), %%ymm3 \n"
        "vmovntdqa 4*32(%%rsi), %%ymm4 \n"
        "vmovntdqa 5*32(%%rsi), %%ymm5 \n"
        "vmovntdqa 6*32(%%rsi), %%ymm6 \n"
        "vmovntdqa 7*32(%%rsi), %%ymm7 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

#endif // LATTESTER_COMMON_H
