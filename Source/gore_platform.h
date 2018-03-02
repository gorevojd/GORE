#ifndef GORE_PLATFORM_H
#define GORE_PLATFORM_H

#include "gore_types.h"
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

struct platform_api {
	platform_thread_queue_add_entry* AddEntry;
	platform_thread_queue_finish_all* FinishAll;

	thread_queue* RenderQueue;

	platform_read_file* ReadFile;
	platform_write_file* WriteFile;
	platform_free_file_memory* FreeFileMemory;
};

extern platform_api PlatformApi;

#endif