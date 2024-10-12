#ifndef TYPES_H
#define TYPES_H
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#pragma warning(disable : 4996)
//#define DEBUG 1
#ifdef _WIN32
	#include <direct.h>
	#define MKDIR(x) _mkdir(x)
#else
	#define MKDIR(x) mkdir(x, 0755)
#endif

#ifdef __GNUC__
	#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
	#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma(pack(pop)) 
	#define NOINLINE __declspec(noinline)
#endif


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#endif