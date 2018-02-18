#ifndef GORE_PLATFORM_H
#define GORE_PLATFORM_H

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

#ifndef Abs
#define Abs(a) ((a) >= 0) ? (a) : -(a)
#endif

#ifndef GET_ALIGN_OFFSET
#define GET_ALIGN_OFFSET(val, align) ((align - ((size_t)val & (align - 1))) & (align - 1))
#endif 

#ifndef MEGABYTES
#define MEGABYTES(count) ((count) * 1024 * 1024)
#endif

#ifndef KILOBYTES
#define KILOBYTES(count) ((count) * 1024)
#endif

#include "gore_math.h"
#include "gore_random.h"
#include "gore_stacked_memory.h"

#include <intrin.h>

#define PLATFORM_THREAD_QUEUE_CALLBACK(name) void name(void* Data)
typedef PLATFORM_THREAD_QUEUE_CALLBACK(thread_queue_callback);

struct thread_queue_entry {
	thread_queue_callback* Callback;
	void* Data;
};

#define THREAD_QUEUE_ENTRY_COUNT 512
struct thread_queue {
	thread_queue_entry Entries[THREAD_QUEUE_ENTRY_COUNT];

	volatile int EntryCount;
	volatile int FinishedEntries;

	volatile int NextToRead;
	volatile int NextToWrite;

	void* Semaphore;
};

#define PLATFORM_THREAD_QUEUE_ADD_ENTRY(name) void name(thread_queue* Queue, thread_queue_callback* Callback, void* Data)
typedef PLATFORM_THREAD_QUEUE_ADD_ENTRY(platform_thread_queue_add_entry);

#define PLATFORM_THREAD_QUEUE_FINISH_ALL(name) void name(thread_queue* Queue)
typedef PLATFORM_THREAD_QUEUE_FINISH_ALL(platform_thread_queue_finish_all);

struct platform_api {
	platform_thread_queue_add_entry* AddEntry;
	platform_thread_queue_finish_all* FinishAll;

	thread_queue* RenderQueue;
};

inline void CopyMemory(void* Dest, void* Src, u64 Size) {
	for (int i = 0; i < Size; i++) {
		*((u8*)Dest + i) = *((u8*)Src + i);
	}
}

inline b32 StringsAreEqual(char* A, char* B) {
	b32 Result = false;

	while (*A && *B) {

		if (*A != *B) {
			Result = false;
			break;
		}

		A++;
		B++;
	}

	if (*A == 0 && *B == 0) {
		Result = true;
	}

	return(Result);
}

inline void CopyStrings(char* Dst, char* Src) {
	if (Src) {
		while (*Src) {
			*Dst++ = *Src++;
		}
	}
	*Dst = 0;
}

inline int StringLength(char* Text) {
	int Res = 0;

	char* At = Text;
	while (*At) {
		Res++;

		At++;
	}

	return(Res);
}

#endif