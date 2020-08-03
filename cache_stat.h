#ifndef __STAT__
#define __STAT__

#include <unordered_map>
#include <string.h>
#include "common_types.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include "time_analysis_tool.h"
#include "io_record.h"
//#include "log.h"


struct SeqNum
{
	/*y = alog(x) + b*/
	double* a; 
	double* b;
	double* re_retio;
	long start_time;
	uint64_t start_total_seq;	//利用全局总数
	uint64_t re_seq; //单独统计
};

struct BlockStat
{
	long last_time;
	uint64_t last_total_seq;
};

class StatCache
{
private:
	uint64_t total_seq;
	uint64_t re_seq;
	uint64_t map_size;
	long current_hour;
	long current_day;
	long start_time;
	long reuse_dis_array[1024];
	unordered_map<uint64_t, struct BlockStat*> stat_map;
	unordered_map<long, struct SeqNum *> seq_map;
	double a;
	double b;
	int smcd_id;
	double para[40][4];

public :
	//StatCache();
	StatCache(long st, int smcd_id);
	~StatCache();
	void main_operation(IoRecord* ir, bool get_reuse_dis, int sample);
	void plot_rar_curve(struct SeqNum* sn_tmp);
	void output_reuse_distance(int smcd_id);
	uint64_t get_map_size();
	long get_time_hour(long timestamp);
	long get_time_day(long timestamp);
	bool is_new_hour(IoRecord* ir);
	bool is_new_day(IoRecord* ir);
	double get_reuse_curve(long time_span, long timestamp);
};
#endif
