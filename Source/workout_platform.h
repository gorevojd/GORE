#ifndef WORKOUT_PLATFORM_H
#define WORKOUT_PLATFORM_H

#define GLOBAL_VARIABLE static

#define Assert(cond) if(!(cond)){ *((int*)0) = 0;}
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))


#ifndef PRETTY_TYPES_DEFINED
#define PRETTY_TYPES_DEFINED

typedef int b32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed char i8;
typedef signed short s16;
typedef signed short i16;
typedef signed int s32;
typedef signed int i32;
typedef signed long long s64;
typedef signed long long i64;
#endif


#ifndef Min
#define Min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef Max
#define Max(a, b) ((a) > (b) ? (a) : (b))
#endif

#include "workout_math.h"

#include <intrin.h>

#if 0
#define PLATFORM_THREAD_QUEUE_CALLBACK(name) int name(void* Data)
typedef PLATFORM_THREAD_QUEUE_CALLBACK(thread_queue_callback);

#define PLATFORM_THREAD_QUEUE_ADD_ENTRY(name) void name(thread_queue* Queue, thread_queue_callback* Callback, void* Data)
typedef PLATFORM_THREAD_QUEUE_ADD_ENTRY(platform_thread_queue_add_entry);

#define PLATFORM_THREAD_QUEUE_COMPLETE_WORK(name) void name(thread_queue* Queue)
typedef PLATFORM_THREAD_QUEUE_FINISH_ALL(platform_thread_queue_finish_all);

struct platform_api {
	platform_thread_queue_add_entry* AddEntry;
	platform_thread_queue_finish_all* FinishAll;
};
#endif


#endif