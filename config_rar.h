#ifndef _CONFIG_RAR_H
#define _CONFIG_RAR_H
#include <string>
#include <sstream>
#include <string.h>

typedef struct Config_rar {
    string cache_file;
    int  cache_size;    //KB
    int  sampling_P;
    int  sampling_T;
    long io_trace_start_time;
    long re_dis_start_time;   //reuse distance begin to be computed
    long io_trace_end_time;

}cfg_rar;

#endif