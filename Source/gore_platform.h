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

#if defined(PLATFORM_WINDA)

#include <Windows.h>

struct platform_mutex {
	volatile long long Ticket;
	volatile long long ServingTicket;
};

inline void BeginMutexAccess(platform_mutex* Mutex) {
 	long long Ticket = _InterlockedExchangeAdd64((volatile long long*)&Mutex->Ticket, 1);
	while (Ticket != Mutex->ServingTicket) {
   		_mm_pause();
	}
}

inline void EndMutexAccess(platform_mutex* Mutex) {
	_InterlockedExchangeAdd64((volatile long long*)&Mutex->ServingTicket, 1);
}

#else

#include <SDL_atomic.h>

struct platform_mutex {
	SDL_atomic_t Ticket;
	SDL_atomic_t ServingTicket;
};

inline void BeginMutexAccess(platform_mutex* Mutex) {
	int Ticket = SDL_AtomicAdd((SDL_atomic_t*)&Mutex->Ticket, 1);
	while (Ticket != Mutex->ServingTicket.value) {
		int a = 1;
	}
}

inline void EndMutexAccess(platform_mutex* Mutex) {
	SDL_AtomicAdd((SDL_atomic_t*)&Mutex->ServingTicket, 1);
}

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

typedef long platform_type_i32;
typedef unsigned long platform_type_u32;
typedef __int64 platform_type_i64;
typedef unsigned long long platform_type_u64;

typedef volatile platform_type_i32 platform_atomic_type_i32;
typedef volatile platform_type_u32 platform_atomic_type_u32;
typedef volatile platform_type_i64 platform_atomic_type_i64;
typedef volatile platform_type_u64 platform_atomic_type_u64;

//NOTE(dima): Atomic CAS operations macros. Must return true if value being set, or false othervise
#define PLATFORM_ATOMIC_CAS_I32(name) platform_type_i32 name(platform_atomic_type_i32* Value, platform_type_i32 New, platform_type_i32 Old)
#define PLATFORM_ATOMIC_CAS_U32(name) platform_type_u32 name(platform_atomic_type_u32* Value, platform_type_u32 New, platform_type_u32 Old)
#define PLATFORM_ATOMIC_CAS_I64(name) platform_type_i64 name(platform_atomic_type_i64* Value, platform_type_i64 New, platform_type_i64 Old)
#define PLATFORM_ATOMIC_CAS_U64(name) platform_type_u64 name(platform_atomic_type_u64* Value, platform_type_u64 New, platform_type_u64 Old)

typedef PLATFORM_ATOMIC_CAS_I32(platform_atomic_cas_i32);
typedef PLATFORM_ATOMIC_CAS_U32(platform_atomic_cas_u32);
typedef PLATFORM_ATOMIC_CAS_I64(platform_atomic_cas_i64);
typedef PLATFORM_ATOMIC_CAS_U64(platform_atomic_cas_u64);

//NOTE(dima) Atomic increment operations macros
#define PLATFORM_ATOMIC_INC_I32(name) platform_type_i32 name(platform_atomic_type_i32* Value)
#define PLATFORM_ATOMIC_INC_U32(name) platform_type_u32 name(platform_atomic_type_u32* Value)
#define PLATFORM_ATOMIC_INC_I64(name) platform_type_i64 name(platform_atomic_type_i64* Value)
#define PLATFORM_ATOMIC_INC_U64(name) platform_type_u64 name(platform_atomic_type_u64* Value)

typedef PLATFORM_ATOMIC_INC_I32(platform_atomic_inc_i32);
typedef PLATFORM_ATOMIC_INC_U32(platform_atomic_inc_u32);
typedef PLATFORM_ATOMIC_INC_I64(platform_atomic_inc_i64);
typedef PLATFORM_ATOMIC_INC_U64(platform_atomic_inc_u64);

//NOTE(dima): Atomic add operations macros. Must return previous value
#define PLATFORM_ATOMIC_ADD_I32(name) platform_type_i32 name(platform_atomic_type_i32* Value, platform_type_i32 Addend)
#define PLATFORM_ATOMIC_ADD_U32(name) platform_type_u32 name(platform_atomic_type_u32* Value, platform_type_u32 Addend)
#define PLATFORM_ATOMIC_ADD_I64(name) platform_type_i64 name(platform_atomic_type_i64* Value, platform_type_i64 Addend)
#define PLATFORM_ATOMIC_ADD_U64(name) platform_type_u64 name(platform_atomic_type_u64* Value, platform_type_u64 Addend)

typedef PLATFORM_ATOMIC_ADD_I32(platform_atomic_add_i32);
typedef PLATFORM_ATOMIC_ADD_U32(platform_atomic_add_u32);
typedef PLATFORM_ATOMIC_ADD_I64(platform_atomic_add_i64);
typedef PLATFORM_ATOMIC_ADD_U64(platform_atomic_add_u64);

//NOTE(dima): Atomic set operations macros. Must return previous value
#define PLATFORM_ATOMIC_SET_I32(name) platform_type_i32 name(platform_atomic_type_i32* Value, platform_type_i32 New)
#define PLATFORM_ATOMIC_SET_U32(name) platform_type_u32 name(platform_atomic_type_u32* Value, platform_type_u32 New)
#define PLATFORM_ATOMIC_SET_I64(name) platform_type_i64 name(platform_atomic_type_i64* Value, platform_type_i64 New)
#define PLATFORM_ATOMIC_SET_U64(name) platform_type_u64 name(platform_atomic_type_u64* Value, platform_type_u64 New)

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

#define PLATFORM_ALLOCATE_MEMORY(name) void* name(u32 Size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

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

inline void ConcatStringsUnsafe(char* Dst, char* Src1, char* Src2) {
	int Index = 0;

	char* To = Dst;
	char* At = Src1;
	while (*At != 0) {
		*To++ = *At++;
	}

	At = Src2;
	while (*At) {
		*To++ = *At++;
	}

	*To = 0;
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

inline b32 StringIsDecimalInteger(char* String) {
	b32 Result = 1;

	int FirstCheckIndex = 0;
	if (String[0] == '-') {
		FirstCheckIndex = 1;
	}

	char* At = String + FirstCheckIndex;
	while (*At)
	{
		if (*At >= '0' &&
			*At <= '9')
		{

		}
		else {
			return(0);
		}

		*At++;
	}

	return(Result);
}

inline int StringToInteger(char* String) {
	int Result = 0;

	char* At = String;

	int Len = StringLength(String);

	int NumberIsNegative = 1;
	int FirstNumberIndex = 0;
	if (String[0] == '-') {
		FirstNumberIndex = 1;
		NumberIsNegative = -1;
	}

	int CurrentMultiplier = 1;
	for (int CharIndex = Len - 1;
		CharIndex >= FirstNumberIndex;
		CharIndex--)
	{
		Result += (String[CharIndex] - '0') * CurrentMultiplier;
		CurrentMultiplier *= 10;
	}

	Result *= NumberIsNegative;

	return(Result);
}

inline float StringToFloat(char* String) {
	float Result = 0.0f;

	//NOTE(dima): Detecting if negative and whole part start index
	float IsNegative = 1.0f;
	int WholeStart = 0;
	if (String[0] == '-') {
		IsNegative = -1.0f;
		WholeStart = 1;
	}

	char* At = String + WholeStart;
	b32 DotExist = 0;
	char* DotAt = 0;
	//NOTE(dima): Detecting whole part end
	int WholeEndIndex = WholeStart;
	while (*At) {
		if (*At == '.') {
			DotExist = 1;
			DotAt = At;
			break;
		}
		At++;
		WholeEndIndex++;
	}

	//NOTE(dima): Converting whole part
	float CurrentMultiplier = 1.0f;
	for (int Index = WholeEndIndex - 1;
		Index >= WholeStart;
		Index--)
	{
		Result += (float)(String[Index] - '0') * CurrentMultiplier;
		CurrentMultiplier *= 10.0f;
	}

	//NOTE(dima): Converting fractional part if exist
	if (DotExist) {
		int FractionalPartLen = 0;
		At = DotAt;
		++At;
		while (*At) {
			FractionalPartLen++;

			At++;
		}

		if (FractionalPartLen) {
			char* FractionalBegin = DotAt + 1;
			char* FractionalEnd = At;

			char* FractionalAt = FractionalBegin;
			CurrentMultiplier = 0.1f;
			while (FractionalAt != FractionalEnd) {
				float CurrentDigit = (float)(*FractionalAt - '0');

				Result += CurrentDigit * CurrentMultiplier;
				CurrentMultiplier /= 10.0f;
				FractionalAt++;
			}
		}
	}

	Result *= IsNegative;

	return(Result);
}

inline void IntegerToString(int Value, char* String) {
	int DigitIndex = 0;

	do {
		String[DigitIndex++] = '0' + (Value % 10);

		Value /= 10;
	} while (Value);

	//NOTE(dima): Reversing string
	int ScanBeginIndex = 0;
	int ScanEndIndex = DigitIndex - 1;
	while (ScanBeginIndex < ScanEndIndex) {
		char Temp = String[ScanBeginIndex];
		String[ScanBeginIndex] = String[ScanEndIndex];
		String[ScanEndIndex] = Temp;

		ScanBeginIndex++;
		ScanEndIndex--;
	}

	//NOTE(dima): Null terminating the string
	String[DigitIndex] = 0;
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

#define PLATFORM_END_GAME_LOOP(name) void name()
typedef PLATFORM_END_GAME_LOOP(platform_end_game_loop);

enum open_file_type {
	FileType_Asset,
	FileType_SavedGame,
};

struct platform_file_entry {
	platform_file_entry* Next;

	char* FileName;

	u64 FileHandle;

	u32 FileSize;
};

struct platform_file_group {
	u32 FileCount;

	void* FreeFileGroupMemory;

	platform_file_entry* FirstFileEntry;
};

#define PLATFORM_OPEN_ALL_FILES_OF_TYPE_BEGIN(name) platform_file_group name(char* FolderPath, u32 Type)
typedef PLATFORM_OPEN_ALL_FILES_OF_TYPE_BEGIN(platform_open_all_files_of_type_begin);

#define PLATFORM_OPEN_ALL_FILES_OF_TYPE_END(name) void name(platform_file_group* Group)
typedef PLATFORM_OPEN_ALL_FILES_OF_TYPE_END(platform_open_all_files_of_type_end);

struct dealloc_queue_bitmap_data {
	void* TextureHandle;
};

struct dealloc_queue_voxelmesh_data {
	void* Handle1;
	void* Handle2;
};

enum dealloc_queue_entry_type {
	DeallocQueueEntry_Bitmap,
	DeallocQueueEntry_VoxelMesh,
};

struct dealloc_queue_entry {
	dealloc_queue_entry* Prev;
	dealloc_queue_entry* Next;

	u32 EntryType;

	union {
		dealloc_queue_bitmap_data BitmapData;
		dealloc_queue_voxelmesh_data VoxelMeshData;
	} Data;
};

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

	platform_mutex NativeMemoryAllocatorMutex;
	platform_allocate_memory* AllocateMemory;
	platform_deallocate_memory* DeallocateMemory;

	platform_add_threadwork_entry* AddThreadworkEntry;
	platform_complete_thread_works* CompleteThreadWorks;
	platform_get_thread_id* GetThreadID;
	platform_get_thread_queue_info* GetThreadQueueInfo;

	platform_compiler_barrier_type* ReadWriteBarrier;
	platform_compiler_barrier_type* ReadBarrier;
	platform_compiler_barrier_type* WriteBarrier;

	platform_end_game_loop* EndGameLoop;

	platform_thread_queue* SuperHighQueue;
	platform_thread_queue* HighPriorityQueue;
	platform_thread_queue* LowPriorityQueue;

	stacked_memory GameModeMemoryBlock;
	stacked_memory EngineSystemsMemoryBlock;
	stacked_memory PlatformMemoryBlock;

	platform_read_file* ReadFile;
	platform_write_file* WriteFile;
	platform_free_file_memory* FreeFileMemory;
	platform_open_all_files_of_type_begin* OpenAllFilesOfTypeBegin;
	platform_open_all_files_of_type_end* OpenAllFilesOfTypeEnd;

	platform_place_cursor_at_center* PlaceCursorAtCenter;
	platform_terminate_program* TerminateProgram;

	platform_mutex DeallocQueueMutex;
	dealloc_queue_entry* FirstUseAllocQueueEntry;
	dealloc_queue_entry* FirstFreeAllocQueueEntry;
};

extern platform_api PlatformApi;

inline dealloc_queue_entry* PlatformRequestDeallocEntry() {
	dealloc_queue_entry* Result = 0;

	BeginMutexAccess(&PlatformApi.DeallocQueueMutex);

	Assert(PlatformApi.FirstFreeAllocQueueEntry->Next != PlatformApi.FirstFreeAllocQueueEntry);

	Result = PlatformApi.FirstFreeAllocQueueEntry->Next;

	Result->Next->Prev = Result->Prev;
	Result->Prev->Next = Result->Next;

	Result->Data = {};
	Result->EntryType = {};

	EndMutexAccess(&PlatformApi.DeallocQueueMutex);

	return(Result);
}

inline void PlatformInsertDellocEntry(dealloc_queue_entry* Entry) {
	BeginMutexAccess(&PlatformApi.DeallocQueueMutex);

	Entry->Next = PlatformApi.FirstUseAllocQueueEntry->Next;
	Entry->Prev = PlatformApi.FirstUseAllocQueueEntry;

	Entry->Next->Prev = Entry;
	Entry->Prev->Next = Entry;

	EndMutexAccess(&PlatformApi.DeallocQueueMutex);
}


#include "gore_debug_layer.h"

#endif