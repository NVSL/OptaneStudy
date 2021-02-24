/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/vfs.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/io.h>
#include <linux/mount.h>
#include <linux/mm.h>
#include <linux/bitops.h>
#include <linux/exportfs.h>
#include <linux/list.h>
#include <linux/pfn_t.h>
#include <linux/kthread.h>
#include <linux/dax.h>
#include <linux/delay.h>
#include <linux/version.h>

#include "lattester.h"

static const char* lattest_task_desc[] = {
	"Invalid",
	"Basic Latency Test",
	"Strided Latency Test ",
	"Strided Bandwidth Test",
	"Random Size-Bandwidth Test",
	"Overwrite Test",
	"Read-after-write Test",
	"Straight Write Test",
	"Writer Thread Test",
	"Flush/Fence BW Test",
	"APC Cache Probe Test",
	"iMC Probe Test",
	"Sequencial Test"
};

static const char* lattest_task_desc_op[] = {
	"",
	"(not configurable)",
	"[access_size] [stride_size]",
	"[access_size] [stride_size] [delay] [parallel] [runtime] [global]",
	"[bwtest_bit] [access_size] [parallel] [runtime] [align_mode]",
	"[stride_size]",
	"[stride_size] [access_size]",
	"[start_addr] [size]",
	"[bwsize_bit] [parallel] [align_mode]",
	"[acccess_size] [fence_rate] [flush_rate] [op (1=random)]",
	"[probe_count], [strided_size]",
	"[probe_count]",
	"[access_size], [parallel]"
};

struct latency_sbi *global_sbi = NULL;

uint32_t *lfs_random_array = NULL;

struct mutex latencyfs_lock;

int support_clwb = 0;

/* Module parameters handled by procfs */
/*
int task = 0;
int op = 0;
int access_size = 64;
int stride_size = 64;
int threads = 1;
int runtime = 60;

module_param(task, int, 0444);
MODULE_PARM_DESC(task, "Task");

module_param(op, int, 0444);
MODULE_PARM_DESC(op, "Operation? 0=Load, 1=Store (require clwb), 2=StoreNT");

module_param(access_size, int, 0444);
MODULE_PARM_DESC(access_size, "Access Size");

module_param(stride_size, int, 0444);
MODULE_PARM_DESC(stride_size, "Strided Size");

module_param(threads, int, 0444);
MODULE_PARM_DESC(threads, "#threads");

module_param(runtime, int, 0444);
MODULE_PARM_DESC(runtime, "runtime");
*/

static struct report_sbi *rep_sbi;

#ifndef X86_FEATURE_CLFLUSHOPT
#define X86_FEATURE_CLFLUSHOPT	(9*32+23) /* CLFLUSHOPT instruction */
#endif
#ifndef X86_FEATURE_CLWB
#define X86_FEATURE_CLWB	(9*32+24) /* CLWB instruction */
#endif

#define PAGENR_2_ADDR (ctx, pagenr, dimm) \
		ctx->addr + (dimm * pagenr << PAGE_SHIFT)


static inline bool arch_has_clwb(void)
{
	return static_cpu_has(X86_FEATURE_CLWB);
}

extern struct report_sbi* reportfs_get_sbi(void);

int latencyfs_print_help(struct seq_file *seq, void *v)
{
	int i;
	seq_printf(seq, "LATTester task=<task> op=<op> [options]\n");
#ifndef AEP_SUPPORTED
	seq_printf(seq, "!! Warning: clwb not supported on current platform.\n");
#endif
	seq_printf(seq, "Tasks:\n");
	for (i = 1; i < TASK_COUNT; i++)
		seq_printf(seq, "\t%d: %s %s\n", i, lattest_task_desc[i], lattest_task_desc_op[i]);
	seq_printf(seq, "Options:\n");
	seq_printf(seq, "\t [op|o]\tOperation: 0=Load, 1=Write(NT) 2=Write(clwb), 3=Write+Flush(clwb+flush) default=0\n");
	seq_printf(seq, "\t [access_size|a] Access size (bytes), default=64\n");
	seq_printf(seq, "\t [align_mode|l] Align mode: 0 = Global (anywhere), 1 = Per-Thread pool (default) 2 = Per-DIMM pool 3=Per-iMC pool\n");
	seq_printf(seq, "\t                            4 = Per-Channel Group, 5 = [NIOnly]: Global, 6 = [NIOnly]: Per-DIMM\n");
	seq_printf(seq, "\t [stride_size|s] Stride size (bytes), default=64\n");
	seq_printf(seq, "\t [delay|d] Delay (cycles), default=0\n");
	seq_printf(seq, "\t [parallel|p] Concurrent kthreads, default=1\n");
	seq_printf(seq, "\t [bwsize_bit|b] Aligned size (2^b bytes), default=6\n");
	seq_printf(seq, "\t [runtime|t] Runtime (seconds), default=10\n");
	seq_printf(seq, "\t [write_addr|w] For Straight Write: access location, default=0 \n");
	seq_printf(seq, "\t [write_size|z] For Straight Write: access size, default=0 \n");
	seq_printf(seq, "\t [fence_rate|f] Access size before issuing a sfence instruction \n");
	seq_printf(seq, "\t [clwb_rate|c] Access size before issuing a clwb instruction \n");
	return 0;
}

inline void latencyfs_cleanup(struct latency_sbi *sbi)
{
	kfree(sbi->workers);
	kfree(sbi->timing);
	kfree(sbi->ctx);
}

inline void latencyfs_new_threads(struct latency_sbi *sbi, bench_func_t func)
{
	int i;
	struct latencyfs_worker_ctx *ctx;
	int cnt = sbi->worker_cnt;
	uint64_t dimmgroup;
	char kthread_name[20];

	if (sbi->align >= ALIGN_PERDIMM) {
		pr_info("Warning: PerDIMM/PerIMC/PerChannel alignment mode selected, it is only supported on certain tasks\n");
		if (sbi->access_size > 4096)
			pr_info("Warning: PerDIMM alignment with 4K+ accesses will write across DIMMs\n");
		if (sbi->worker_cnt > 6)
			pr_info("Warning: PerDIMM alignment with 6+ threads\n");
	}

	sbi->workers = kmalloc(sizeof(struct task_struct *) * cnt, GFP_KERNEL);
	sbi->ctx = kmalloc(sizeof(struct latencyfs_worker_ctx) * cnt, GFP_KERNEL);
	sbi->timing = kmalloc(sizeof(struct latencyfs_timing) * cnt, GFP_KERNEL);
	for (i = 0; i < cnt; i++) {
		ctx = &sbi->ctx[i];
		ctx->sbi = sbi;
		ctx->core_id = i;
		switch (sbi->align) {
			case ALIGN_GLOBAL:
			case ALIGN_PERIMC:
			case ALIGN_NI_GLOBAL:
				ctx->addr = (u8 *)sbi->virt_addr;
				break;
			case ALIGN_PERTHREAD:
				ctx->addr = (u8 *)sbi->virt_addr + (i * PERTHREAD_WORKSET);
				break;
			case ALIGN_PERDIMM:
			case ALIGN_PERCHANNELGROUP:
				dimmgroup = i / 6;
				ctx->addr = (u8 *)sbi->virt_addr + (dimmgroup * PERDIMMGROUP_WORKSET);
				break;
			case ALIGN_NI:
				ctx->addr = (u8 *)sbi->virt_addr + ((i % 6) *6* 4096);
				pr_info("dimm %d, addr %p", i, ctx->addr);
				break;
			default:
				pr_err("Undefined align mode %d\n", sbi->align);
		}
		sbi->timing[i].v = 0;
		ctx->seed_addr = (u8 *)sbi->rep->virt_addr + (i * PERTHREAD_WORKSET);
		sprintf(kthread_name, "lattester%d", ctx->core_id);
		sbi->workers[i] = kthread_create(func, (void *)ctx, kthread_name);
		kthread_bind(sbi->workers[i], ctx->core_id);
	}
}

void latencyfs_timer_callback(unsigned long data)
{
	//TODO
}

inline void latencyfs_create_seed(struct latency_sbi *sbi)
{
	latencyfs_prealloc_large(sbi->rep->virt_addr, sbi->worker_cnt * PERTHREAD_WORKSET, PERTHREAD_MASK);
	pr_info("Random buffer created.");
	drop_cache();
}

inline void latencyfs_stop_threads(struct latency_sbi *sbi)
{
	int i;
	int cnt = sbi->worker_cnt;

	for (i = 0; i < cnt; i++) {
		kthread_stop(sbi->workers[i]);
	}
}


inline void latencyfs_monitor_threads(struct latency_sbi *sbi)
{
	int i;
	int elapsed = 0;
	int runtime = sbi->runtime;
	u64 last = 0, total;

	latencyfs_create_seed(sbi);

	/* Wake up workers */
	for (i = 0; i < sbi->worker_cnt; i++) {
		wake_up_process(sbi->workers[i]);
	}

	/* Print result every second
	 * Use a timer if accuracy is a concern
	 */
	while (elapsed < runtime) {
		msleep(1000);
		total = 0;
		for (i = 0; i < sbi->worker_cnt; i++) {
			total += sbi->timing[i].v;
		}
		printk(KERN_ALERT "%d\t%lld (%d)\n", elapsed + 1, (total - last) / MB, smp_processor_id());

		last = total;
		elapsed ++;
	}
    latencyfs_stop_threads(sbi);
}

int setup_singlethread_op(struct latency_sbi *sbi, int op)
{
	switch (op) {
		case TASK_BASIC_OP: // 1
			latencyfs_new_threads(sbi, &latency_job);
			break;
		case TASK_OVERWRITE: // 5
			latencyfs_new_threads(sbi, &overwrite_job);
			break;
		case TASK_READ_AFTER_WRITE: // 6
			latencyfs_new_threads(sbi, &read_after_write_job);
			break;
		case TASK_STRAIGHT_WRITE: // 7
			latencyfs_new_threads(sbi, &straight_write_job);
			break;
		case TASK_STRIDED_LAT: // 2
			latencyfs_new_threads(sbi, &strided_latjob);
			break;
		case TASK_CACHEPROBE: //10
			latencyfs_new_threads(sbi, &cacheprobe_job);
			break;

		default:
			pr_err("Single thread op %d not expected\n", op);
			return -EINVAL;
	}
	wake_up_process(sbi->workers[0]);
	return 0;
}

int setup_multithread_op(struct latency_sbi *sbi, int op) {
	unsigned long delta = 0;
	switch (op) {
		case TASK_STRIDED_BW: // 3
			latencyfs_new_threads(sbi, &strided_bwjob);
			break;
		case TASK_SIZE_BW: // 4
			//latencyfs_prealloc_global_permutation_array(sbi->access_size);
			delta = PERTHREAD_CHECKSTOP % sbi->access_size;
			latencyfs_new_threads(sbi, &sizebw_job);
			break;
		case TASK_THBIND: // 8
			latencyfs_new_threads(sbi, &align_bwjob);
			break;
		case TASK_FLUSHFENCE: // 9
			latencyfs_new_threads(sbi, &cachefence_bwjob);
			break;
		case TASK_IMCPROBE: //11
			latencyfs_new_threads(sbi, &imcprobe_job);
			break;
		case TASK_SEQ: //12
			latencyfs_new_threads(sbi, &seq_bwjob);
			break;

		default:
			pr_err("Single thread op %d not expected\n", op);
			return -EINVAL;
	}
	latencyfs_monitor_threads(sbi);
	return 0;
}


void latencyfs_start_task(struct latency_sbi *sbi, int task, int threads)
{
	int i, ret = 0;

	switch (task) {
		case TASK_BASIC_OP: // 1
		case TASK_STRIDED_LAT: // 2
		case TASK_OVERWRITE: // 5
		case TASK_READ_AFTER_WRITE: //6
		case TASK_STRAIGHT_WRITE: // 7
		case TASK_CACHEPROBE: // 10
			sbi->worker_cnt = 1;
			ret = setup_singlethread_op(sbi, task);
			break;

		case TASK_STRIDED_BW: // 2
		case TASK_SIZE_BW: // 3
		case TASK_THBIND: // 8
		case TASK_FLUSHFENCE: // 9
		case TASK_IMCPROBE: // 11
		case TASK_SEQ:
			sbi->worker_cnt = threads;
			ret = setup_multithread_op(sbi, task);
			break;

		case TASK_INVALID:
			pr_err("Remount using task=<id>, tasks:\n");
			for (i = 1; i < TASK_COUNT; i++) {
				pr_err("%d: %s\n", i, lattest_task_desc[i]);
			}
			break;

		default:
			pr_err("Invalid Task\n");
			return;
	}

	if (ret) {
		pr_info("Warning: start task returned %d", ret);
	}
}


static int latencyfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct latency_sbi *sbi;
	void *virt_addr = NULL;
	struct inode *root;
	struct dax_device *dax_dev;
	pfn_t __pfn_t;
	long size;
	int ret = 0;

	if (rep_sbi) {
		pr_err("Already mounted\n");
		return -EEXIST;
	}

	rep_sbi = reportfs_get_sbi();

	if (!rep_sbi) {
		pr_err("Mount ReportFS first\n");
		return -EINVAL;
	}

	sbi = kzalloc(sizeof(struct latency_sbi), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;
	sb->s_fs_info = sbi;
	sbi->sb = sb;
	sbi->rep = rep_sbi;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 6)
	ret = bdev_dax_supported(sb, PAGE_SIZE);
#else
	ret = bdev_dax_supported(sb->s_bdev, PAGE_SIZE);
#endif
	pr_info("%s: dax_supported = %d; bdev->super=0x%p",
			__func__, ret, sb->s_bdev->bd_super);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 6)
	if (ret)
#else
	if (!ret)
#endif
	{
		pr_err("device does not support DAX\n");
		return ret;
	}

	sbi->s_bdev = sb->s_bdev;

	dax_dev = fs_dax_get_by_host(sb->s_bdev->bd_disk->disk_name);
	if (!dax_dev)
	{
		pr_err("Couldn't retrieve DAX device.\n");
		return -EINVAL;
	}

	size = dax_direct_access(dax_dev, 0, LONG_MAX / PAGE_SIZE,
				 &virt_addr, &__pfn_t) * PAGE_SIZE;
	if (size <= 0)
	{
		pr_err("direct_access failed\n");
		return -EINVAL;
	}

	sbi->virt_addr = virt_addr;
	sbi->phys_addr = pfn_t_to_pfn(__pfn_t) << PAGE_SHIFT;
	sbi->initsize = size;

	pr_info("%s: dev %s, phys_addr 0x%llx, virt_addr %p, size %ld\n",
			__func__, sbi->s_bdev->bd_disk->disk_name,
			sbi->phys_addr, sbi->virt_addr, sbi->initsize);

	root = new_inode(sb);
	if (!root)
	{
		pr_err("root inode allocation failed\n");
		return -ENOMEM;
	}

	root->i_ino = 0;
	root->i_sb = sb;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
	root->i_atime = root->i_mtime = root->i_ctime = current_kernel_time();
#else
	root->i_atime = root->i_mtime = root->i_ctime = ktime_to_timespec64(ktime_get_real());
#endif
	inode_init_owner(root, NULL, S_IFDIR);

	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		pr_err("d_make_root failed\n");
		return -ENOMEM;
	}

	global_sbi = sbi;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 6)
	return ret;
#else
	return !ret;
#endif
}

static struct dentry *latencyfs_mount(struct file_system_type *fs_type,
	 int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, latencyfs_fill_super);
}

static struct file_system_type latencyfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "LatencyFS",
	.mount		= latencyfs_mount,
	.kill_sb	= kill_block_super,
};

static int __init init_latencyfs(void)
{
	int rc = 0;
	struct file *fp;

	mutex_init(&latencyfs_lock);

	pr_info("%s: %d cpus online\n", __func__, num_online_cpus());
	if (arch_has_clwb())
		support_clwb = 1;

	pr_info("Arch new instructions support: CLWB %s\n",
			support_clwb ? "YES" : "NO");

	fp = filp_open("/proc/lattester", O_RDONLY, 0);
	if (!PTR_ERR(fp))
	{
		pr_err("LATTester running, exiting.\n");
		return 0;
	}

	rc = latencyfs_proc_init();
	if (rc)
		return rc;

	rc = register_filesystem(&latencyfs_fs_type);
	if (rc)
		return rc;

	return 0;
}

static void __exit exit_latencyfs(void)
{
	if (lfs_random_array)
		kfree(lfs_random_array);
	remove_proc_entry("lattester", NULL);
	unregister_filesystem(&latencyfs_fs_type);
}

MODULE_LICENSE("GPL");

module_init(init_latencyfs)
module_exit(exit_latencyfs)
