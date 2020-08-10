#include "time_analysis_tool.h"
#include <ctime>
#include "stdio.h"
#define TIME_ON 1

TimeAnalysis _time_analysis;

void TimeAnalysis::set_start_time()
{
	if (!TIME_ON)
	{
		return;
	}
	gettimeofday(&start_tv, NULL);
}

void TimeAnalysis::set_end_time()
{
	if (!TIME_ON)
	{
		return;
	}
	gettimeofday(&end_tv, NULL);
}

void TimeAnalysis::add_time(string key)
{
	if (!TIME_ON)
	{
		return;
	}
	auto it = time_map.find(key);
	if (it == time_map.end())
	{
		struct time_struct* ts = (struct time_struct*)
			malloc(sizeof(struct time_struct));
		ts->cnt = 1;
		ts->latency = end_tv.tv_sec * 1000 + end_tv.tv_usec * 1.0 / 1000 -
			start_tv.tv_sec * 1000 - start_tv.tv_usec * 1.0 / 1000;
		time_map[key] = ts;

	}
	else
	{
		struct time_struct* ts = it->second;
		ts->cnt++;
		ts->latency += end_tv.tv_sec * 1000 + end_tv.tv_usec * 1.0 / 1000 -
			start_tv.tv_sec * 1000 - start_tv.tv_usec * 1.0 / 1000;
	}
}

double TimeAnalysis::get_latency(string key)
{
	auto it = time_map.find(key);
	if (it == time_map.end())
	{
		return 0;
	}
	else
	{
		struct time_struct* ts = it->second;
		return ts->latency / ts->cnt;
	}
}

void TimeAnalysis::print_all_latency()
{
	char shell_str[256];
	strcpy(shell_str, "mkdir -p ./simulator_result");
	int ret = system(shell_str);
	assert(0 == ret);

	FILE* fp = fopen("./simulator_result/time_analysis.log", "w");
	double total_time = 0;
	for (auto it = time_map.begin();
		it != time_map.end();
		++it)
	{
		struct time_struct* ts = it->second;
		fprintf(fp, "%s : %.6f ms \t\t\t total_time : %.6f ms ( cnt : % llu )\n",
			(it->first).c_str(), ts->latency / ts->cnt,
			ts->latency, ts->cnt);
		total_time += ts->latency;
	}
	fprintf(fp, "---------------------------------------------\n");
	fprintf(fp, "total time-consuming : %.6f minutes\n", total_time / 1000 / 60);
	fclose(fp);
}
