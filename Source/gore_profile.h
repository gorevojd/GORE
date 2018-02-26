#ifndef GORE_PROFILE_H_INCLUDED
#define GORE_PROFILE_H_INCLUDED

#include "gore_platform.h"

#include <SDL_atomic.h>
#include <SDL_thread.h>

enum profile_record_type {
	ProfileRecord_None,

	ProfileRecord_BeginTiming,
	ProfileRecord_EndTiming,

	ProfileRecord_BeginBlock,
	ProfileRecord_EndBlock,
	ProfileRecord_Value,
};

struct profile_record {
	char* Name;

	u32 RecordType;

	union {
		struct {
			u64 Clocks;
			u16 ThreadID;
		};
	};
};

struct profile_record_table {
	SDL_atomic_t CurrentRecordIndex;

	int RecordsMaxCount;
	profile_record* Records;
};

extern profile_record_table* GlobalRecordTable;

inline void PROFAddRecord(char* Name, u32 RecordType) {
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);
	Assert(Index < GlobalRecordTable->RecordsMaxCount);

	profile_record* Record = GlobalRecordTable->Records + Index;

	Record->Name = Name;
	Record->Clocks = __rdtsc();
	Record->RecordType = RecordType;
	//TODO(dima): think about perfomance of this
	Record->ThreadID = SDL_ThreadID();
}

#define BEGIN_TIMING() 
#define END_TIMING()

struct profile_timing {

};

#endif