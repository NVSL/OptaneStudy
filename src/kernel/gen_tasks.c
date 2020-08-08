/* SPDX-License-Identifier: GPL-2.0 */
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cpuid.h>
#include <pthread.h>
#include <stdlib.h>
#include "common.h"
#include "lattester.h"
#include "memaccess.h"

int main(int argc, char **argv)
{
	int i;
	char buf[200];
	FILE *fp;
	int task = atoi(argv[2]);
	int task_count = (task == 1) ? BASIC_OPS_TASK_COUNT : BENCH_SIZE_TASK_COUNT;
	int ops_mb = LATENCY_OPS_COUNT * 8 / 1048576;
	int ops_count = LATENCY_OPS_COUNT;

	if (argc != 3)
	{
		printf("%s <report_fs> <task_id>\n", argv[0]);
		return 0;
	}

	fp = fopen("../testscript/parsing/tasks.py", "w");
	if (!fp)
	{
		printf("fopen error %d\n", errno);
	}
	fprintf(fp, "#!/usr/bin/python\n");

	if (task == 1)
	{
		fprintf(fp, "tasks = [\n");
		for (i = 0; i < BASIC_OPS_TASK_COUNT; i++)
			fprintf(fp, "\t'%s-seq',\n", latency_tasks_str[i]);
		for (i = 0; i < BASIC_OPS_TASK_COUNT; i++)
			fprintf(fp, "\t'%s-rand',\n", latency_tasks_str[i]);

		fprintf(fp, "]\n");
		fprintf(fp, "dd='dd if=%s of=/tmp/dump bs=%dM skip=1 count=%d'\n", argv[1], ops_mb, BASIC_OPS_TASK_COUNT * 2);
	}else if (task == 2)
	{
		fprintf(fp, "tasks = [\n");
		for (i = 0; i < BENCH_SIZE_TASK_COUNT; i++)
			fprintf(fp, "\t'%s',\n", bench_size_map[i]);
		fprintf(fp, "]\n");
		fprintf(fp, "dd='dd if=%s of=/tmp/dump bs=%dM count=%d'\n", argv[1], ops_mb, BENCH_SIZE_TASK_COUNT);
	}else if (task == 5)
	{
		fprintf(fp, "dd='dd if=%s of=/tmp/dump bs=%dM count=1'\n", argv[1], ops_mb*2);
		ops_count *= 2;
	}

	fprintf(fp, "ops_count=%d\n", ops_count);
	fclose(fp);
	return 0;
}
