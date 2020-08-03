#include <string>
#include <sstream>
#include <string.h>
#include <time.h>
#include <ctime>

//#include "cfg_file.h"
//#include "common_types.h"
//#include "tfc_base_str.h"
#include "io_record.h"
//#include "io_generator.h"
//#include "io_router.h"
//#include "smcd_context.h"
//#include "cache_pool.h"
//#include "io_sampler.h"
//#include "log.h"
#include "cache_stat.h"


IoRecord* get_io_trace(int smcd_id,FILE* fp, IoRecord* _io_record) {
    //获取trace记录
    uint64_t io_time, offset, len, type;
    int disk_id;
    if (_io_record == NULL)
        return NULL;

    int ret = fscanf(fp, "%lu,%lu,%lu,%lu,%d",
        &io_time,
        &offset,
        &len,
        &type,
        &disk_id
    );

    if (0 == ret)
    {
        printf("err:%d, exit\n", errno);
        return NULL;
    }
    else if (EOF == ret) {
        printf("file over");
        return NULL;
    }
    else
    {
        _io_record->alloc_time = io_time;
        _io_record->io_type = type;
        _io_record->size = len;
        _io_record->disk_id = disk_id;
        sprintf(_io_record->disksn, "%d", disk_id);
        _io_record->range_key = 0;
        _io_record->offset = offset;
    }

    return _io_record;
}

//字符串时间转时间戳
time_t StringToDatetime(const char* str)
{
    tm tm_;
    int year, month, day, hour, minute, second;
    sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_.tm_year = year - 1900;
    tm_.tm_mon = month - 1;
    tm_.tm_mday = day;
    tm_.tm_hour = hour;
    tm_.tm_min = minute;
    tm_.tm_sec = second;
    tm_.tm_isdst = 0;

    time_t t_ = mktime(&tm_); //已经减了8个时区
    return t_; //秒时间
}

int main(int argc, char* argv[])
{
    if (2 != argc)
    {
        printf("Usage: ./rar_simulator smcd_id\n");
        exit(-1);
    }

    //清空日志
    //LogClear();

    //_time_analysis.set_end_time();
    //_time_analysis.add_time("simulator init time");

    //起始&结束时间
    long io_trace_start_time = StringToDatetime("2018-10-02 0:0:0");
    long re_dis_start_time = StringToDatetime("2018-10-04 0:0:0"); //重用距离记录开始时间
    long io_trace_end_time = StringToDatetime("2018-10-05 0:0:0");
    //获取smcd
    int smcd_id = atoi(argv[1]);

    StatCache* sc = new StatCache(io_trace_start_time, smcd_id);
    IoRecord* io_trace = new IoRecord();
    uint64_t total_trace_cnt = 0;
    bool get_reuse_dis = false;
    string filename = "/cbs_trace1/atc_2020_trace/orignial/";
    filename.append(to_string(static_cast<long long>(smcd_id)));
    cout << "filename=" << filename << endl;
    //LogWrite(1, "filename = /cbs_trace1/atc_2020_trace/orignial/%d\n", smcd_id);
    //int smcd_id = 0;
    //char filename[128] = "/cbs_trace1/atc_2020_trace/orignial/0";


    ////1538323199,55112392,16,1,1058
    FILE* fp = fopen(filename.c_str(), "r");
    while (1) {
        assert(NULL != fp);

        io_trace = get_io_trace(smcd_id, fp, io_trace);
        if (NULL == io_trace) break;
        
        //开始
        if (io_trace->alloc_time < io_trace_start_time) {
            continue;
        }
        if (io_trace->alloc_time >= re_dis_start_time) {
            get_reuse_dis = true;
        }
        total_trace_cnt++;
        if (total_trace_cnt % 50000000 == 0) {
            cout << "count=" << total_trace_cnt / 50000000 << "*5*10^7!" << endl;
            //LogWrite(1, "smcd_id=%d; count=%d*5*10^7! ;\n", smcd_id, total_trace_cnt / 50000000);
        }
        //if (total_trace_cnt > 200000000)
        //    break;

        //offset,size: 1 sector = 512B = 2^9 B
        //cache_unit:  1 block = 32KB = 2^15 B
        long block_id = io_trace->offset >> 6;
        int block_cnt = (io_trace->size - 1) >> 6;
        for (int i = 0; i <= block_cnt; i++) {
            sc->main_operation(io_trace, block_id + i, get_reuse_dis);
        }

        //结束
        if (io_trace->alloc_time >= io_trace_end_time) {
            break;
        }
    }
    sc->output_reuse_distance(smcd_id);
    cout << "process end!~~~~" << endl;
    cout << "map_size = " << sc->get_map_size() << endl;

    delete io_trace;
    io_trace = NULL;
    delete sc;
    sc = NULL;

    return 0;
}
