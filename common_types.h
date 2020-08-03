#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include <stdint.h>
#include <string>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <deque>
#include <uuid/uuid.h>
#include <assert.h>

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uuid_t   uint128;

using std::string;
using std::vector;

#endif
