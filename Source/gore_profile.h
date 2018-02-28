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
	char* UniqueName;

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
	SDL_atomic_t CurrentTableIndex;

	int RecordsMaxCount;
	profile_record Records[2][1024];
};

struct profile_frame {

};


#define PROFILE_FRAMES_COUNT 256
struct profile_state {
	profile_frame Frames[PROFILE_FRAMES_COUNT];
	u32 CollectionFrameIndex;


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