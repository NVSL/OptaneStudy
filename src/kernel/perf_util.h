/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _PERF_UTIL_H_
#define _PERF_UTIL_H_

#include <linux/ioctl.h>
#include <linux/perf_event.h>
#include <linux/string.h>

/* There is a `perf_event` in `perf_event.h`
 * So use this name to prevent conflict
 */
struct __perf_event {
    struct perf_event_attr attr;
    u64 value;
    u64 enabled;
    u64 running;
    const char *name;
};

#define PERF_EVENT_READ(e, evt)                                                \
    e->value = perf_event_read_value(evt, &e->enabled, &e->running);

#define PERF_EVENT_PRINT(e)                                                    \
    pr_info("\tPerf counter [%s]\t: %lld\n", e.name, e.value);

#define PERF_EVENT_HW(cfg)                                                     \
    {                                                                          \
	.attr =                                                                \
	    {                                                                  \
		.type = PERF_TYPE_HARDWARE,                                    \
		.config = PERF_COUNT_HW_##cfg,                                 \
		.size = sizeof(struct perf_event_attr),                        \
		.pinned = 1,                                                   \
		.disabled = 1,                                                 \
		.exclude_user = 1,                                             \
		.exclude_hv = 1,                                               \
		.exclude_idle = 1,                                             \
	    },                                                                 \
	.name = "HW." #cfg, .value = 0xffff, .enabled = 0xffff,                \
	.running = 0xffff,                                                     \
    }

#define DEFINE_PERF_EVENT_HW(event, cfg)                                       \
    struct __perf_event event = PERF_EVENT_HW(cfg);

#define PERF_EVENT_CACHE(level, op, result)                                    \
    {                                                                          \
	.attr =                                                                \
	    {                                                                  \
		.type = PERF_TYPE_HW_CACHE,                                    \
		.config = (PERF_COUNT_HW_CACHE_##level)                        \
			  | (PERF_COUNT_HW_CACHE_OP_##op << 8)                 \
			  | (PERF_COUNT_HW_CACHE_RESULT_##result << 16),       \
		.size = sizeof(struct perf_event_attr),                        \
		.pinned = 1,                                                   \
		.disabled = 1,                                                 \
		.exclude_user = 1,                                             \
		.exclude_hv = 1,                                               \
		.exclude_idle = 1,                                             \
	    },                                                                 \
	.name = "CACHE." #level "." #op "." #result, .value = 0xffff,          \
	.enabled = 0xffff, .running = 0xffff,                                  \
    }

#define DEFINE_PERF_EVENT_CACHE(event, level, op, result)                      \
    struct __perf_event event = PERF_EVENT_CACHE(level, op, result);

int lattest_perf_event_create(void);
void lattest_perf_event_read(void);
void lattest_perf_event_enable(void);
void lattest_perf_event_disable(void);
void lattest_perf_event_reset(void);
void lattest_perf_event_release(void);
void lattest_perf_event_print(void);

#endif /* _PERF_UTIL_H_ */
