#ifndef _CONFIG_EMA_H
#define _CONFIG_EMA_H
#include <string>
#include <sstream>
#include <string.h>

//match the content of ema.conf
typedef struct Config_ema {
	string cache_file;
	int  cache_size;    //KB
	int  sampling_P;
	int  sampling_T;
	long io_trace_start_time;
	long re_dis_start_time;   //reuse distance begin to be computed
	long io_trace_end_time;

}cfg_ema;

#endif
