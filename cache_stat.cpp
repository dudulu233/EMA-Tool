#include "cache_stat.h"
#include <assert.h>
#include <cmath>
#include <iostream>
#include <stdio.h>

StatCache::StatCache(long st, int smcd)
{
	total_seq = re_seq = map_size = current_hour = current_day = 0;
	start_time = st;
	smcd_id = smcd;
	for (int i = 0; i < 1024; i++)
	{
		reuse_dis_array[i] = 0l;
	}
}

StatCache::~StatCache()
{

}

void StatCache::main_operation(IoRecord* ir, bool get_reuse_dis, int cache_size, int sp, int st)
{
	struct SeqNum* sn_new = NULL;
	struct SeqNum* sn_old = NULL;
	if (is_new_day(ir))

	{
		//_time_analysis.set_start_time();

		sn_new = (struct SeqNum*)malloc(sizeof(struct SeqNum));
		sn_new->start_total_seq = total_seq;
		sn_new->re_seq = 0;
		sn_new->start_time = ir->alloc_time;
		sn_new->a = (double*)malloc(2 * sizeof(double));
		sn_new->b = (double*)malloc(2 * sizeof(double));
		sn_new->re_retio = (double*)malloc(3 * sizeof(double));
		sn_new->a[0] = sn_new->a[1] = sn_new->b[0] = sn_new->b[1] =
			sn_new->re_retio[0] = sn_new->re_retio[1] = sn_new->re_retio[2] = 0.0;

		seq_map[current_day] = sn_new;
		//_time_analysis.set_end_time();
		//_time_analysis.add_time("avg_process_time1_1");

		if (current_day - get_time_day(start_time) > 1) {
			//绘制两天前的曲线
			struct SeqNum* sn_tmp = seq_map[current_day - 2];
			assert(sn_tmp);
			plot_rar_curve(sn_tmp);
		}
	}
	else
	{
		sn_new = seq_map[current_day];
	}
	if (current_day - get_time_day(start_time) > 0)
	{
		//_time_analysis.set_start_time();

		sn_old = seq_map[current_day - 1];
		assert(sn_old);

		//_time_analysis.set_end_time();
		//_time_analysis.add_time("avg_process_time1_2");
	}
	assert(sn_new);

	if (is_new_hour(ir)) {
		if (current_hour - get_time_hour(sn_new->start_time) == 1) {
			assert(total_seq - sn_new->start_total_seq != 0);
			sn_new->re_retio[0] = (double)(sn_new->re_seq) / (total_seq - sn_new->start_total_seq);
		}
		if (current_hour - get_time_hour(sn_new->start_time) == 20) {
			assert(total_seq - sn_new->start_total_seq != 0);
			sn_new->re_retio[1] = (double)(sn_new->re_seq) / (total_seq - sn_new->start_total_seq);
		}
		if (NULL != sn_old) {
			if (current_hour - get_time_hour(sn_old->start_time) == 47) {
				assert(total_seq - sn_old->start_total_seq != 0);
				sn_old->re_retio[2] = (double)(sn_old->re_seq) / (total_seq - sn_old->start_total_seq);
			}
		}

	}

	//_time_analysis.set_start_time();

	total_seq++;

	auto it = stat_map.find(ir->cache_addr);
	struct BlockStat* bs;
	int reuse_dis = -1;

	//_time_analysis.set_end_time();
	//_time_analysis.add_time("avg_process_time2");
	//_time_analysis.set_start_time();

	if (it == stat_map.end())
	{
		bs = (struct BlockStat*)malloc(sizeof(struct BlockStat));
		bs->last_time = ir->alloc_time;
		bs->last_total_seq = total_seq;
		stat_map[ir->cache_addr] = bs;
		++map_size;
		reuse_dis = 1023;
	}
	else
	{
		bs = it->second;
		re_seq++;
		if (current_day == get_time_day(bs->last_time)) {
			sn_new->re_seq++;
		}
		if ((current_day - get_time_day(bs->last_time) <= 1) && (NULL != sn_old)) {
			sn_old->re_seq++;
		}
		long x = (ir->alloc_time - bs->last_time);
		double re_ratio = get_reuse_curve(x, ir->alloc_time);
		double reuse_tmp = (total_seq - bs->last_total_seq) *
			(1 - re_ratio);
		reuse_dis = (int)(reuse_tmp * sp * cache_size / st / 1024 / 1024);
		if (reuse_dis >= 1024)
		{
			FILE* fp = fopen("big_reuse.txt", "w");
			fprintf(fp, "%ld\t%llu\t%lf\n", x, total_seq - bs->last_total_seq, re_ratio);
			fclose(fp);
			reuse_dis = 1023;
		}
		if (reuse_dis <= 0)
		{
			reuse_dis = 0;
		}
		bs->last_time = ir->alloc_time;
		bs->last_total_seq = total_seq;
	}
	if (get_reuse_dis) {
		reuse_dis_array[reuse_dis]++;
	}
	//_time_analysis.set_end_time();
	//_time_analysis.add_time("avg_process_time3");

}

void StatCache::plot_rar_curve(struct SeqNum* sn_tmp)
{
	//y = a*log(t) + b
	//x = log(t)
	//a = (y1-y2)/(x1-x2)
	//b = (x1*y2-x2*y1)/(x1-x2)
	double x1 = log(3600);
	double x2 = log(3600 * 20);
	double x3 = log(3600 * 47);
	double y1 = sn_tmp->re_retio[0];
	double y2 = sn_tmp->re_retio[1];
	double y3 = sn_tmp->re_retio[2];
	sn_tmp->a[0] = (y1 - y2) / (x1 - x2);
	sn_tmp->a[1] = (y3 - y2) / (x3 - x2);
	sn_tmp->b[0] = (x1 * y2 - x2 * y1) / (x1 - x2);
	sn_tmp->b[1] = (x3 * y2 - x2 * y3) / (x3 - x2);

}

void StatCache::output_reuse_distance(int smcd_id)
{
	string filepath = "./simulator_result/smcd" + to_string(static_cast<long long>(smcd_id));
	string shell_str = "mkdir -p " + filepath;
	int ret = system(shell_str.c_str());
	assert(0 == ret);
	string filename = filepath.append("/reuse_dis.txt");
	FILE* fp = fopen(filename.c_str(), "w");
	if (NULL == fp) {
		cout << "create " << filename << " fail~!!!!~!" << endl;
		return;
	}
	for (int i = 0; i < 1024; i++) {
		fprintf(fp, "%ld,", reuse_dis_array[i]);
	}
	fclose(fp);
}

uint64_t StatCache::get_map_size()
{
	return map_size;
}

long StatCache::get_time_hour(long timestamp)
{
	return timestamp / 3600;
}

long StatCache::get_time_day(long timestamp)
{
	return (timestamp + 3600 * 8) / (3600 * 24);
}

bool StatCache::is_new_hour(IoRecord* ir)
{
	bool result = false;
	long tmp = get_time_hour(ir->alloc_time);
	if (tmp - current_hour > 0)
	{
		result = true;
		current_hour = tmp;
	}
	else if (tmp < current_hour)
	{
		ir->alloc_time += 1;
	}
	return result;
}

bool StatCache::is_new_day(IoRecord* ir)
{
	bool result = false;
	long tmp = get_time_day(ir->alloc_time);
	if (tmp - current_day > 0)
	{
		result = true;
		current_day = tmp;
	}
	else if (tmp < current_day)
	{
		ir->alloc_time += 1;
	}
	return result;
}

double StatCache::get_reuse_curve(long time_span, long timestamp)
{
	/*
	get reuse distance
	y = a*log(x) + b
	reusedistance = total_block_cnt * (1 - re_access_ratio)
	*/
	double result = 0;
	if (time_span <= 1 || (get_time_day(timestamp) - get_time_day(start_time) < 2))
	{
		result = 0;
	}
	else
	{
		SeqNum* sn_tmp = seq_map[get_time_day(timestamp) - 2];
		if (time_span <= 72000)
		{
			result = sn_tmp->a[0] * log(time_span) + sn_tmp->b[0];
		}
		else
		{
			result = sn_tmp->a[1] * log(time_span) + sn_tmp->b[1];
		}
	}

	return result < 0.99 ? result : 0.99;
}
