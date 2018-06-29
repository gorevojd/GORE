#ifndef GORE_PLATFORM_H
#define GORE_PLATFORM_H

#include "gore_types.h"
#include "gore_math.h"
#include "gore_random.h"
#include "gore_memory.h"

#include <intrin.h>

#if defined(_WIN32) || defined(_WIN64)
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
//#define PLATFORM_THREAD_QUEUE_SIZE 4086
struct platform_thread_queue;

struct platform_thread_queue_info {
	int WorkingThreadsCount;
	int TotalEntriesCount;
	int EntriesBusy;
};

#define PLATFORM_GET_THREAD_QUEUE_INFO(name) platform_thread_queue_info name(platform_thread_queue* Queue)
typedef PLATFORM_GET_THREAD_QUEUE_INFO(platform_get_thread_queue_info);

typedef volatile i32 platform_atomic_type_i32;
typedef volatile u32 platform_atomic_type_u32;
typedef volatile i64 platform_atomic_type_i64;
typedef volatile u64 platform_atomic_type_u64;

//NOTE(dima): Atomic CAS operations macros
#define PLATFORM_ATOMIC_CAS_I32(name) i32 name(platform_atomic_type_i32* Value, i32 New, i32 Old)
#define PLATFORM_ATOMIC_CAS_U32(name) u32 name(platform_atomic_type_u32* Value, u32 New, u32 Old)
#define PLATFORM_ATOMIC_CAS_I64(name) i64 name(platform_atomic_type_i64* Value, i64 New, i64 Old)
#define PLATFORM_ATOMIC_CAS_U64(name) u64 name(platform_atomic_type_u64* Value, u64 New, u64 Old)

typedef PLATFORM_ATOMIC_CAS_I32(platform_atomic_cas_i32);
typedef PLATFORM_ATOMIC_CAS_U32(platform_atomic_cas_u32);
typedef PLATFORM_ATOMIC_CAS_I64(platform_atomic_cas_i64);
typedef PLATFORM_ATOMIC_CAS_U64(platform_atomic_cas_u64);

//NOTE(dima) Atomic increment operations macros
#define PLATFORM_ATOMIC_INC_I32(name) i32 name(platform_atomic_type_i32* Value)
#define PLATFORM_ATOMIC_INC_U32(name) u32 name(platform_atomic_type_u32* Value)
#define PLATFORM_ATOMIC_INC_I64(name) i64 name(platform_atomic_type_i64* Value)
#define PLATFORM_ATOMIC_INC_U64(name) u64 name(platform_atomic_type_u64* Value)

typedef PLATFORM_ATOMIC_INC_I32(platform_atomic_inc_i32);
typedef PLATFORM_ATOMIC_INC_U32(platform_atomic_inc_u32);
typedef PLATFORM_ATOMIC_INC_I64(platform_atomic_inc_i64);
typedef PLATFORM_ATOMIC_INC_U64(platform_atomic_inc_u64);

//NOTE(dima): Atomic add operations macros
#define PLATFORM_ATOMIC_ADD_I32(name) i32 name(platform_atomic_type_i32* Value, i32 Addend)
#define PLATFORM_ATOMIC_ADD_U32(name) u32 name(platform_atomic_type_u32* Value, u32 Addend)
#define PLATFORM_ATOMIC_ADD_I64(name) i64 name(platform_atomic_type_i64* Value, i64 Addend)
#define PLATFORM_ATOMIC_ADD_U64(name) u64 name(platform_atomic_type_u64* Value, u64 Addend)

typedef PLATFORM_ATOMIC_ADD_I32(platform_atomic_add_i32);
typedef PLATFORM_ATOMIC_ADD_U32(platform_atomic_add_u32);
typedef PLATFORM_ATOMIC_ADD_I64(platform_atomic_add_i64);
typedef PLATFORM_ATOMIC_ADD_U64(platform_atomic_add_u64);

//NOTE(dima): Atomic set operations macros
#define PLATFORM_ATOMIC_SET_I32(name) i32 name(platform_atomic_type_i32* Value, i32 New)
#define PLATFORM_ATOMIC_SET_U32(name) u32 name(platform_atomic_type_u32* Value, u32 New)
#define PLATFORM_ATOMIC_SET_I64(name) i64 name(platform_atomic_type_i64* Value, i64 New)
#define PLATFORM_ATOMIC_SET_U64(name) u64 name(platform_atomic_type_u64* Value, u64 New)

typedef PLATFORM_ATOMIC_SET_I32(platform_atomic_set_i32);
typedef PLATFORM_ATOMIC_SET_U32(platform_atomic_set_u32);
typedef PLATFORM_ATOMIC_SET_I64(platform_atomic_set_i64);
typedef PLATFORM_ATOMIC_SET_U64(platform_atomic_set_u64);

#define PLATFORM_ADD_THREADWORK_ENTRY(name) void name(platform_thread_queue* Queue, void* Data, platform_threadwork_callback* Callback)
typedef PLATFORM_ADD_THREADWORK_ENTRY(platform_add_threadwork_entry);

#define PLATFORM_COMPLETE_THREAD_WORKS(name) void name(platform_thread_queue* Queue)
typedef PLATFORM_COMPLETE_THREAD_WORKS(platform_complete_thread_works);

#define PLATFORM_GET_THREAD_ID(name) u32 name()
typedef PLATFORM_GET_THREAD_ID(platform_get_thread_id);

#define PLATFORM_COMPILER_BARRIER_TYPE(name) void name()
typedef PLATFORM_COMPILER_BARRIER_TYPE(platform_compiler_barrier_type);

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
	platform_atomic_cas_i32* AtomicCAS_I32;
	platform_atomic_cas_u32* AtomicCAS_U32;
	platform_atomic_cas_i64* AtomicCAS_I64;
	platform_atomic_cas_u64* AtomicCAS_U64;

	platform_atomic_inc_i32* AtomicInc_I32;
	platform_atomic_inc_u32* AtomicInc_U32;
	platform_atomic_inc_i64* AtomicInc_I64;
	platform_atomic_inc_u64* AtomicInc_U64;

	platform_atomic_add_i32* AtomicAdd_I32;
	platform_atomic_add_u32* AtomicAdd_U32;
	platform_atomic_add_i64* AtomicAdd_I64;
	platform_atomic_add_u64* AtomicAdd_U64;

	platform_atomic_set_i32* AtomicSet_I32;
	platform_atomic_set_u32* AtomicSet_U32;
	platform_atomic_set_i64* AtomicSet_I64;
	platform_atomic_set_u64* AtomicSet_U64;

	platform_add_threadwork_entry* AddThreadworkEntry;
	platform_complete_thread_works* CompleteThreadWorks;
	platform_get_thread_id* GetThreadID;
	platform_get_thread_queue_info* GetThreadQueueInfo;

	platform_compiler_barrier_type* ReadWriteBarrier;
	platform_compiler_barrier_type* ReadBarrier;
	platform_compiler_barrier_type* WriteBarrier;

	platform_thread_queue* VoxelQueue;
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

#include "gore_debug_layer.h"

#endif