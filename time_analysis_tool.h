#ifndef TIMEANALYSISTOOL
#define TIMEANALYSISTOOL
#include <unordered_map>
#include <string.h>
#include <sys/time.h>
#include "common_types.h"

using namespace std;

struct time_struct
{
	uint64_t cnt;
	double latency;
};

class TimeAnalysis
{
	private:
		struct timeval start_tv;
		struct timeval end_tv;
		unordered_map<string,struct time_struct *> time_map;	
	public:
		void set_start_time();
		void set_end_time();
		void add_time(string key);
		double get_latency(string key);
		void print_all_latency();
};

extern TimeAnalysis _time_analysis;
#endif
