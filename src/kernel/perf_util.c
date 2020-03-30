#include "perf_util.h"
#include <asm/irq_regs.h>
#include <linux/slab.h>

#define NUM_PERF_EVENTS 13
static DEFINE_PER_CPU(struct perf_event **, lattest_perf_event_list);

#define for_each_lattest_perf_event(id) for (id = 0; id < NUM_PERF_EVENTS; ++id)

static struct __perf_event perf_counters[] = {
	PERF_EVENT_CACHE(L1D, READ, MISS),
	PERF_EVENT_CACHE(L1D, READ, ACCESS),
	PERF_EVENT_CACHE(L1D, WRITE, ACCESS),
	PERF_EVENT_CACHE(LL, READ, MISS),
	PERF_EVENT_CACHE(LL, WRITE, MISS),
	PERF_EVENT_CACHE(LL, READ, ACCESS),
	PERF_EVENT_CACHE(LL, WRITE, ACCESS),
	// PERF_EVENT_CACHE(NODE, READ, MISS),
	// PERF_EVENT_CACHE(NODE, WRITE, MISS),
	// PERF_EVENT_CACHE(NODE, READ, ACCESS),
	// PERF_EVENT_CACHE(NODE, WRITE, ACCESS),
	PERF_EVENT_CACHE(DTLB, READ, MISS),
	PERF_EVENT_CACHE(DTLB, WRITE, MISS),
	PERF_EVENT_CACHE(DTLB, READ, ACCESS),
	PERF_EVENT_CACHE(DTLB, WRITE, ACCESS),
	// PERF_EVENT_HW(INSTRUCTIONS),
	PERF_EVENT_HW(CACHE_REFERENCES),
	PERF_EVENT_HW(CACHE_MISSES),
};

static void lattest_perf_overflow_callback(struct perf_event *event,
					   struct perf_sample_data *data,
					   struct pt_regs *regs)
{
	event->hw.interrupts = 0;
	pr_err("Overflow detected for perf counter: %d - %lld\n",
	       event->attr.type, event->attr.config);
	// TODO: call `lattest_perf_exit()` ?
}

void lattest_perf_exit(void)
{
	int cpu, id;
	struct perf_event *event;
	for_each_possible_cpu(cpu)
	{
		for_each_lattest_perf_event(id)
		{
			event = per_cpu(lattest_perf_event_list, cpu)[id];
			if (event)
				perf_event_release_kernel(event);
		}
		kfree(per_cpu(lattest_perf_event_list, cpu));
	}
}

int lattest_perf_init(void)
{
	int cpu, ret = 0;
	for_each_possible_cpu(cpu)
	{
		per_cpu(lattest_perf_event_list, cpu) =
			kcalloc(NUM_PERF_EVENTS, sizeof(struct perf_event *),
				GFP_KERNEL);
		if (!per_cpu(lattest_perf_event_list, cpu)) {
			pr_err("lattest_perf_init: failed to allocate %d perf events for cpu %d\n",
			       NUM_PERF_EVENTS, cpu);
			ret = -ENOMEM;
			goto out;
		}
	}
out:
	if (ret)
		lattest_perf_exit();
	return ret;
}

int _lattest_perf_event_create(int cpu, int event_id)
{
	struct perf_event *evt;
	struct perf_event_attr *attr;

	if (event_id >= NUM_PERF_EVENTS) {
		pr_err("_lattest_perf_event_create: event_id %d out of range %d\n",
		       event_id, NUM_PERF_EVENTS);
		return -EINVAL;
	}

	/* If already created */
	if (per_cpu(lattest_perf_event_list, cpu)[event_id])
		return 0;

	// TODO: find a proper place to memset attr to 0?
	attr = &perf_counters[event_id].attr;

	evt = perf_event_create_kernel_counter(
		attr, cpu, NULL, lattest_perf_overflow_callback, NULL);
	if (IS_ERR(evt)) {
		pr_err("Perf event create on CPU %d failed with %ld\n", cpu,
		       PTR_ERR(evt));
		return PTR_ERR(evt);
	}

	per_cpu(lattest_perf_event_list, cpu)[event_id] = evt;

	return 0;
}

int lattest_perf_event_create(void)
{
	int event_id, ret = 0;

	lattest_perf_init();
	for_each_lattest_perf_event(event_id)
	{
		ret = _lattest_perf_event_create(smp_processor_id(), event_id);
		if (ret)
			return ret;
	}
	return ret;
}

void lattest_perf_event_read(void)
{
	struct perf_event *evt;
	struct __perf_event *counter;
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		evt = this_cpu_read(lattest_perf_event_list)[event_id];
		counter = &perf_counters[event_id];
		PERF_EVENT_READ(counter, evt);
	}
}

void lattest_perf_event_enable(void)
{
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		perf_event_enable(
			this_cpu_read(lattest_perf_event_list)[event_id]);
	}
}

void lattest_perf_event_disable(void)
{
	struct perf_event *evt;
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		evt = this_cpu_read(lattest_perf_event_list)[event_id];
		if (evt) {
			perf_event_disable(evt);
		}
	}
}

void lattest_perf_event_reset(void)
{
	struct perf_event *evt;
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		evt = this_cpu_read(lattest_perf_event_list)[event_id];
		if (evt) {
			local64_set(&evt->count, 0);
		}
	}
}

void _lattest_perf_event_release(int cpu)
{
	struct perf_event *evt;
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		evt = this_cpu_read(lattest_perf_event_list)[event_id];
		if (evt) {
			perf_event_release_kernel(evt);
		}
		per_cpu(lattest_perf_event_list, cpu)[event_id] = NULL;
	}
}

void lattest_perf_event_release(void)
{
	int cpu;

	for_each_possible_cpu(cpu)
	{
		_lattest_perf_event_release(cpu);
	}

	lattest_perf_exit();
}

void lattest_perf_event_print(void)
{
	int event_id = 0;
	for_each_lattest_perf_event(event_id)
	{
		PERF_EVENT_PRINT(perf_counters[event_id]);
	}
}
