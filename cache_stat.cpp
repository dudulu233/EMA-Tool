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

	//default
	/*para[0][0] = 0.027475843568899108; para[0][1] = 0.6420718335758969; para[0][2] = 0.010670436226304114; para[0][3] = 0.8300305910604904;
	para[1][0] = 0.03539685061900993; para[1][1] = 0.5270405067223953; para[1][2] = 0.024744366959090963; para[1][3] = 0.6461823729102157;
	para[2][0] = 0.035163017274833445; para[2][1] = 0.5439918616415994; para[2][2] = 0.020763273389141516; para[2][3] = 0.7050446652824242;
	para[3][0] = 0.03357959865204152; para[3][1] = 0.538524331091105; para[3][2] = 0.029294979172445085; para[3][3] = 0.5864453208809799;
	para[4][0] = 0.04318859953152237; para[4][1] = 0.453143859714916; para[4][2] = 0.012783031286299693; para[4][3] = 0.793212547815077;
	para[5][0] = 0.02740686603738314; para[5][1] = 0.6477317821060005; para[5][2] = 0.017113619430681466; para[5][3] = 0.762855789708865;
	para[6][0] = 0.030764120316634388; para[6][1] = 0.600905264528081; para[6][2] = 0.015506761120061866; para[6][3] = 0.7715499992031676;
	para[7][0] = 0.04824826799533361; para[7][1] = 0.3881237939494582; para[7][2] = 0.018916208213433552; para[7][3] = 0.7161859110215023;
	para[8][0] = 0.04724782194292421; para[8][1] = 0.4062625318831218; para[8][2] = 0.020012337395797893; para[8][3] = 0.7108756680368485;
	para[9][0] = 0.047167812675696266; para[9][1] = 0.4095530978443018; para[9][2] = 0.020139031709896348; para[9][3] = 0.7118543740399996;*/
	
}

StatCache::~StatCache()
{
	
}

void StatCache::main_operation(IoRecord* ir, bool get_reuse_dis, int sample)
{
	struct SeqNum *sn_new = NULL;
	struct SeqNum *sn_old = NULL;
	if(is_new_day(ir))

	{
		cout << "new day!~" <<current_day<< endl;
		//_time_analysis.set_start_time();

		sn_new = (struct SeqNum *)malloc(sizeof(struct SeqNum));
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

	//采样,1，20，47小时的时候
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
	
	//缓存引用总量
	total_seq++;


	auto it = stat_map.find(ir->cache_addr);
	struct BlockStat *bs;
	int reuse_dis = -1;

	//_time_analysis.set_end_time();
	//_time_analysis.add_time("avg_process_time2");
	//_time_analysis.set_start_time();

	//根据cache地址索引
	if(it == stat_map.end())
	{
		bs = (struct BlockStat *)malloc(sizeof(struct BlockStat));
		bs->last_time = ir->alloc_time;
		bs->last_total_seq = total_seq;
		stat_map[ir->cache_addr] = bs;
		++ map_size;
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
		reuse_dis = (int)( reuse_tmp * sample * 32 / 1024/ 1024 );
		if(reuse_dis >= 1024)
		{
			FILE *fp = fopen("big_reuse.txt","w");
			fprintf(fp,"%ld\t%llu\t%lf\n",x,total_seq-bs->last_total_seq,re_ratio);
			fclose(fp);
			reuse_dis = 1023;
		}
		if(reuse_dis <= 0)
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

//获取rar曲线
void StatCache::plot_rar_curve(struct SeqNum* sn_tmp)
{
	//y = alog(t) + b
	//x = log(t)
	//a = (y1-y2)/(x1-x2)
	//b = (x1*y2-x2*y1)/(x1-x2)
	double x1 = log(3600);
	double x2 = log(3600*20);
	double x3 = log(3600*47);
	double y1 = sn_tmp->re_retio[0];
	double y2 = sn_tmp->re_retio[1];
	double y3 = sn_tmp->re_retio[2];
	sn_tmp->a[0] = (y1 - y2) / (x1 - x2);
	sn_tmp->a[1] = (y3 - y2) / (x3 - x2);
	sn_tmp->b[0] = (x1*y2 - x2*y1) / (x1 - x2);
	sn_tmp->b[1] = (x3*y2 - x2*y3) / (x3 - x2);

	cout << "time:" << get_time_day(sn_tmp->start_time) <<
		",a[0]=" << sn_tmp->a[0] <<
		",b[0]=" << sn_tmp->b[0] <<
		",a[1]=" << sn_tmp->a[1] <<
		",b[1]=" << sn_tmp->b[1] << endl;
	cout << "re_ratio=" << sn_tmp->re_retio[0] << "," << sn_tmp->re_retio[1] << "," << sn_tmp->re_retio[2] << endl;
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
		cout << "create "<< filename <<" fail~!!!!~!" << endl;
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
	if(tmp - current_hour > 0)
	{
		result = true;
		current_hour = tmp;
	}
	else if( tmp < current_hour )
	{
		ir->alloc_time += 1;
	}
	return result;
}

bool StatCache::is_new_day(IoRecord* ir)
{
	bool result = false;
	long tmp = get_time_day(ir->alloc_time);
	if(tmp - current_day > 0)
	{
		result = true;
		current_day = tmp;
	}
	else if( tmp < current_day)
	{
		ir->alloc_time += 1;
	}
	return result;
}

double StatCache::get_reuse_curve(long time_span, long timestamp)
{
	/*
 	get reuse distance
 	y = alog(x) + b
	reusedistance = total_block_cnt * (1 - re_access_ratio)
	*/
	double result = 0;
	if (time_span <= 1 || (get_time_day(timestamp) - get_time_day(start_time) < 2))
	{
		result = 0;
	}
	else
	{
	//根据获取的
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
	//else
	//{
	////根据预定的
	//	if(time_span <= 72000)
	//	{
	//		result = para[smcd_id][0] * log(time_span) + para[smcd_id][1];
	//	}
	//	else
	//	{
	//		result = para[smcd_id][2] * log(time_span) + para[smcd_id][3];
	//	}
	//}

	return result < 0.99 ? result : 0.99;
}
