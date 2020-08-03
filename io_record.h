#ifndef _IO_RECORD_H
#define _IO_RECORD_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/*
uint64_t alloc_time;
uint64_t cache_addr;
*/
typedef struct _IoRecord
{
    uint64_t alloc_time;
    uint64_t cache_addr;

} IoRecord;

#endif
