#ifndef TYPES_H
#define TYPES_H
//#define DEBUG
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <sys/stat.h>
#include <stdexcept>
#pragma warning(disable : 4996)

#ifdef _WIN32
	#include <direct.h>
	#include <windows.h>
	#define MKDIR(x) _mkdir(x)
#else
	#define MKDIR(x) mkdir(x, 0755)
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::microseconds ms;
typedef std::chrono::duration<float> fsec;

#endif