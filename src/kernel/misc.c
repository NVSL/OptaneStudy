/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/random.h>
#include "lattester.h"

extern uint32_t *lfs_random_array;

int latencyfs_getopt(const char *caller, char **options,
					 const struct latency_option *opts, char **optopt, char **optarg,
					 unsigned long *value)
{
	char *token;
	char *val;

	do {
		if ((token = strsep(options, ",")) == NULL)
			return 0;
	} while (*token == '\0');
	if (optopt)
		*optopt = token;

	if ((val = strchr(token, '=')) != NULL)	{
		*val++ = 0;
	}
	*optarg = val;
	for (; opts->name; opts++)
	{
		if (!strcmp(opts->name, token))
		{
			if (!val) {
				if (opts->has_arg & OPT_NOPARAM) {
					return opts->val;
				}
				pr_info("%s: the %s option requires an argument\n",
						 caller, token);
				return -EINVAL;
			}
			if (opts->has_arg & OPT_INT) {
				char *v;

				*value = simple_strtoul(val, &v, 0);
				if (!*v)
				{
					return opts->val;
				}
				pr_info("%s: invalid numeric value in %s=%s\n",
						 caller, token, val);
				return -EDOM;
			}
			if (opts->has_arg & OPT_STRING) {
				return opts->val;
			}
			pr_info("%s: unexpected argument %s to the %s option\n",
					 caller, val, token);
			return -EINVAL;
		}
	}
	pr_info("%s: Unrecognized option %s\n", caller, token);
	return -EOPNOTSUPP;
}


/*
 * Generates a 16 KB (4K * sizeof int) global random array.
 * It seems such configuration is enough to prevent CPU from establishing
 * a pattern.
 * This array is almost guaranteed to be cached in L2.
 * We used to have a much larger random pool.
 */

/* pre-alloc RNG with a small permuation array */
inline void latencyfs_prealloc_global_permutation_array(int size)
{
	int i;
	uint32_t temp, j;
	lfs_random_array = kmalloc(sizeof(uint32_t) * LFS_PERMRAND_ENTRIES, GFP_KERNEL);


	// initial range of numbers
	for (i = 0; i < LFS_PERMRAND_ENTRIES - 1; i++)	{
		lfs_random_array[i] = i + 1;
	}

	lfs_random_array[LFS_PERMRAND_ENTRIES - 1] = 0;

	for (i = LFS_PERMRAND_ENTRIES - 1; i >= 0; i--)
	{
		//generate a random number [0, n-1]
		get_random_bytes(&j, sizeof(uint32_t));
		j = j % LFS_PERMRAND_ENTRIES;

		//swap the last element with element at random index
		temp = lfs_random_array[i];
		lfs_random_array[i] = lfs_random_array[j];
		lfs_random_array[j] = temp;
	}
	/* TODO: currently we allocate 2GB per thread so we store shifted values
	 * For 4GB and larger regions, we need to do `mul` or `shl` at runtime
	 */
	for (i = 0; i < LFS_PERMRAND_ENTRIES; i++) {
		lfs_random_array[i] *= size;
	}
}

/*
 * pre-alloc RNG, Large array approach, the array should be 2^n size
 * Address = rand_long & BIT_MASK
 */
inline void latencyfs_prealloc_large(uint8_t *addr, uint64_t size, uint64_t bit_mask)
{
    int i;
    uint32_t *buf;
	get_random_bytes(addr, size);
    buf = (uint32_t *)addr;
    for (i = 0; i < size >> 2; i++) {
        buf[i] &= bit_mask;
    }
}

/* in-flight fast pseudo-RNG */
inline unsigned long fastrand(unsigned long *x, unsigned long *y, unsigned long *z, uint64_t bit_mask)
{
	unsigned long t;

	*x ^= *x << 16;
	*x ^= *x >> 5;
	*x ^= *x << 1;

	t = *x;
	*x = *y;
	*y = *z;
	*z = t ^ *x ^ *y;

	return *z & bit_mask;
}
