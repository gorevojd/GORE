#ifndef GORE_PROFILE_H_INCLUDED
#define GORE_PROFILE_H_INCLUDED

#include "gore_platform.h"

#include <SDL_atomic.h>
#include <SDL_thread.h>

enum profile_record_type {
	ProfileRecord_None,

	ProfileRecord_BeginTiming,
	ProfileRecord_EndTiming,

	//NOTE(dima): sections are just categories that will be visible in the debug overlay
	ProfileRecord_BeginSection,
	ProfileRecord_EndSection,
	ProfileRecord_Value,
};

struct profile_record {
	char* Name;
	char* UniqueName;

	u32 RecordType;

	union {
		struct {
			u64 Clocks;
			u16 ThreadID;
		};
	};
};

struct profile_timing_snapshot {
	u64 BeginClock;
	u64 ChildrenSumClocks;
	u64 ClocksElapsed;

	u16 ThreadID;

	u32 HitCount;
};

struct profile_record_table {
	SDL_atomic_t CurrentRecordIndex;
	SDL_atomic_t CurrentTableIndex;

	int RecordsMaxCount;
	profile_record Records[2][4096];
};

enum profile_block_entry_type {
	ProfileBlockEntry_None,

	ProfileBlockEntry_Timing,
	ProfileBlockEntry_Section,
};

struct profile_block_entry {
	u32 BlockEntryType;

	//TODO(dima): change this to ID and hash calculation
	char* Name;
	u32 ID;

	profile_block_entry* Parent;

	profile_block_entry* NextBro;
	profile_block_entry* PrevBro;

	profile_block_entry* ChildrenSentinel;

	union {
		profile_timing_snapshot TimingSnapshot;
	};
};

struct profile_frame {
	profile_block_entry* CurrentTimingBlock;

	profile_block_entry* TimingBlockSentinel;
};


#define PROFILE_FRAMES_COUNT 256
struct profile_state {
	profile_frame Frames[PROFILE_FRAMES_COUNT];
	u32 CollectionFrameIndex;

	profile_block_entry* FirstFreeBlock;

	stacked_memory ProfileMemory;
};

extern profile_record_table* GlobalRecordTable;

#define PROFILE_ID_TO_STRING(id) #id

#define PROFILE_UNIQUE_STRING_(id, func, line, counter) id "@" func "@" ## PROFILE_ID_TO_STRING(line) ## "@" ## PROFILE_ID_TO_STRING(counter)
#define PROFILE_UNIQUE_STRING(id) PROFILE_UNIQUE_STRING_(id, __FUNCTION__, __LINE__, __COUNTER__)

inline void PROFAddRecord(char* Name, char* UniqueName, u32 RecordType) {
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);
	Assert(Index < GlobalRecordTable->RecordsMaxCount);

	profile_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value] + Index;

	Record->Name = Name;
	Record->UniqueName = UniqueName;
	Record->Clocks = __rdtsc();
	Record->RecordType = RecordType;
	//TODO(dima): think about perfomance of this
	Record->ThreadID = SDL_ThreadID();
}

#define ADD_PROFILE_RECORD(name, type) PROFAddRecord(name, PROFILE_UNIQUE_STRING(name), type)

#define BEGIN_TIMING(name) ADD_PROFILE_RECORD(name, ProfileRecord_BeginTiming)
#define END_TIMING() ADD_PROFILE_RECORD("End", ProfileRecord_EndTiming)

#define FUNCTION_TIMING() profile_timing FunctionTiming_##__COUNTER__(__FUNCTION__, PROFILE_UNIQUE_STRING(__FUNCTION__));

struct profile_timing {
	char* Name;
	char* UniqueName;

	profile_timing(char* Name, char* UniqueName) {
		this->Name = Name;
		this->UniqueName = UniqueName;

		PROFAddRecord(Name, UniqueName, ProfileRecord_BeginTiming);
	}

	~profile_timing() {
		PROFAddRecord(Name, UniqueName, ProfileRecord_EndTiming);
	}
};

#endif