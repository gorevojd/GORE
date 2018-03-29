#ifndef GORE_PLATFORM_H
#define GORE_PLATFORM_H

#include "gore_types.h"
#include "gore_math.h"
#include "gore_random.h"
#include "gore_memory.h"
#include "gore_debug_layer.h"

#include <intrin.h>

#if defined(WIN32) || defined(WIN64)
#define PLATFORM_WINDA
#endif

#define PLATFORM_THREADWORK_CALLBACK(name) void name(void* Data)
typedef PLATFORM_THREADWORK_CALLBACK(platform_threadwork_callback);

enum platform_threadwork_state {
	PlatformThreadworkState_None,

	PlatformThreadworkState_Initialized,
};

struct platform_threadwork {
	platform_threadwork_callback* Callback;
	void* Data;
};

//NOTE(dima): Platform thread queue structure defined in platform dependent code
#define PLATFORM_THREAD_QUEUE_SIZE 512
struct platform_thread_queue;

#define PLATFORM_ADD_THREADWORK_ENTRY(name) void name(platform_thread_queue* Queue, void* Data, platform_threadwork_callback* Callback)
typedef PLATFORM_ADD_THREADWORK_ENTRY(platform_add_threadwork_entry);

#define PLATFORM_COMPLETE_THREAD_WORKS(name) void name(platform_thread_queue* Queue)
typedef PLATFORM_COMPLETE_THREAD_WORKS(platform_complete_thread_works);

#define PLATFORM_GET_THREAD_ID(name) u32 name()
typedef PLATFORM_GET_THREAD_ID(platform_get_thread_id);

inline void MEMCopy(void* Dest, void* Src, u64 Size) {
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

inline u32 StringHashFNV(char* Name) {
	u32 Result = 2166136261;

	char* At = Name;
	while (*At) {

		Result *= 16777619;
		Result ^= *At;

		At++;
	}

	return(Result);
}

struct platform_read_file_result {
	u64 Size;
	void* Data;
};

#define PLATFORM_READ_FILE(name) platform_read_file_result name(char* FilePath)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_WRITE_FILE(name) void name(char* FilePath, void* Data, u64 Size)
typedef PLATFORM_WRITE_FILE(platform_write_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(platform_read_file_result* FileReadResult)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

#define PLATFORM_PLACE_CURSOR_AT_CENTER(name) void name()
typedef PLATFORM_PLACE_CURSOR_AT_CENTER(platform_place_cursor_at_center);

#define PLATFORM_TERMINATE_PROGRAM(name) void name()
typedef PLATFORM_TERMINATE_PROGRAM(platform_terminate_program);

struct platform_api {
	platform_add_threadwork_entry* AddThreadworkEntry;
	platform_complete_thread_works* CompleteThreadWorks;
	platform_get_thread_id* GetThreadID;

	platform_thread_queue* HighPriorityQueue;
	platform_thread_queue* LowPriorityQueue;

	stacked_memory GameModeMemoryBlock;
	stacked_memory GeneralPurposeMemoryBlock;
	stacked_memory DEBUGMemoryBlock;

	platform_read_file* ReadFile;
	platform_write_file* WriteFile;
	platform_free_file_memory* FreeFileMemory;

	platform_place_cursor_at_center* PlaceCursorAtCenter;
	platform_terminate_program* TerminateProgram;
};

extern platform_api PlatformApi;

#endif