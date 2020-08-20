#include <time.h>
#include <ctime>

#include "common_types.h"
#include "io_record.h"
#include "cache_stat.h"
#include "config_ema.h"
#include "log.h"

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

	time_t t_ = mktime(&tm_);
	return t_;
}

void get_cfg(const char* cfg_path, cfg_ema* cfg) {

	string key[7] = { "cache_file","cache_size","sampling_P","sampling_T","io_trace_start_time","re_dis_start_time","io_trace_end_time" };
	string value[7] = { "" };
	int index = 0;
	fstream cfg_file;
	cfg_file.open(cfg_path);
	if (!cfg_file.is_open()) {
		printf("error! config file open fail!\n");
		exit(-1);
	}
	while (index < 7)
	{
		char tmp[128];
		cfg_file.getline(tmp, 128);//128 probably enough
		string line(tmp);

		size_t pos = line.find('=');//find the location of "=",get the key and value
		if (pos == string::npos) {
			printf("error! config format incorrect!\n");
			exit(-1);
		}

		string tmpKey = line.substr(0, pos);//get key
		if (key[index] != tmpKey)
		{
			printf("error! config key incorrect!\n");
			exit(-1);
		}
		value[index] = line.substr(pos + 1);//get value
		++index;

	}
	cfg_file.close();

	cfg->cache_file = value[0];
	cfg->cache_size = stoi(value[1]);
	cfg->sampling_P = stoi(value[2]);
	cfg->sampling_T = stoi(value[3]);
	cfg->io_trace_start_time = StringToDatetime(value[4].c_str());
	cfg->re_dis_start_time = StringToDatetime(value[5].c_str());
	cfg->io_trace_end_time = StringToDatetime(value[6].c_str());

}

//designed by MIT
uint32_t murmurhash(const char* key, uint32_t len, uint32_t seed) {
	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;
	uint32_t r1 = 15;
	uint32_t r2 = 13;
	uint32_t m = 5;
	uint32_t n = 0xe6546b64;
	uint32_t h = 0;
	uint32_t k = 0;
	uint8_t* d = (uint8_t*)key; // 32 bit extract from `key'
	const uint32_t* chunks = NULL;
	const uint8_t* tail = NULL; // tail - last 8 bytes
	int i = 0;
	int l = len / 4; // chunk length

	h = seed;

	chunks = (const uint32_t*)(d + l * 4); // body
	tail = (const uint8_t*)(d + l * 4); // last 8 byte chunk of `key'

	// for each 4 byte chunk of `key'
	for (i = -l; i != 0; ++i) {
		// next 4 byte chunk of `key'
		k = chunks[i];

		// encode next 4 byte chunk of `key'
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;

		// append to hash
		h ^= k;
		h = (h << r2) | (h >> (32 - r2));
		h = h * m + n;
	}

	k = 0;

	// remainder
	switch (len & 3) { // `len % 4'
	case 3: k ^= (tail[2] << 16);
	case 2: k ^= (tail[1] << 8);

	case 1:
		k ^= tail[0];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
		h ^= k;
	}

	h ^= len;

	h ^= (h >> 16);
	h *= 0x85ebca6b;
	h ^= (h >> 13);
	h *= 0xc2b2ae35;
	h ^= (h >> 16);

	return h;
}

int main(int argc, char* argv[])
{
	if (2 != argc)
	{
		printf("Usage: ./ema_tool config_file_name\n");
		exit(-1);
	}

	if (0 != access(argv[1], F_OK))
	{
		printf("Error, config file %s not exist\n");
		exit(-1);
	}

	_time_analysis.set_start_time();

	bool get_reuse_dis = false;
	uint64 total_trace_cnt = 0;

	cfg_ema* cfg = new cfg_ema();
	string conf_file_name(argv[1]);
	assert(NULL != cfg);
	get_cfg(conf_file_name.c_str(), cfg);
	LogClear();

	StatCache* sc = new StatCache(cfg->io_trace_start_time);
	IoRecord* io_trace = new IoRecord();

	LogWrite(1, "process start~");

	FILE* fp = fopen(cfg->cache_file.c_str(), "r");

	_time_analysis.set_end_time();
	_time_analysis.add_time("ema init time");

	while (1) {
		assert(NULL != fp);
		assert(NULL != io_trace);

		_time_analysis.set_start_time();

		if(fscanf(fp, "%lu,%lu",
			&io_trace->alloc_time,
			&io_trace->cache_addr
		) == EOF) break;

		_time_analysis.set_end_time();
		_time_analysis.add_time("read trace");

		//program begin
		_time_analysis.set_start_time();
		if (io_trace->alloc_time < cfg->io_trace_start_time) {
			continue;
		}
		//begin to get the reuse distance
		if (io_trace->alloc_time >= cfg->re_dis_start_time) {
			get_reuse_dis = true;
		}

		//hash(A) mod P < T, sampling rate = T / P * 100%
		//e.g. P=100, T=1,so the sampling rate = 0.01
		string str_cache_addr = to_string(static_cast<long long>(io_trace->cache_addr));
		uint32_t hash = murmurhash(str_cache_addr.c_str(), (str_cache_addr).length(), 0);


		if (hash % cfg->sampling_P < cfg->sampling_T) {
			sc->main_operation(io_trace, get_reuse_dis, cfg->cache_size, cfg->sampling_P, cfg->sampling_T);
		}

		//finish
		if (io_trace->alloc_time >= cfg->io_trace_end_time) {
			sc->main_operation(io_trace, get_reuse_dis, cfg->cache_size, cfg->sampling_P, cfg->sampling_T);
			break;
		}

		_time_analysis.set_end_time();
		_time_analysis.add_time("process time");
	}


	_time_analysis.set_start_time();
	sc->output_reuse_distance();
	_time_analysis.set_end_time();
	_time_analysis.add_time("output time");

	_time_analysis.print_all_latency();

	LogWrite(1, "process end~~");

	delete io_trace;
	io_trace = NULL;
	delete sc;
	sc = NULL;

	return 0;
}
