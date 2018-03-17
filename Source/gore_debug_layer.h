#ifndef GORE_DEBUG_LAYER_H
#define GORE_DEBUG_LAYER_H

#include "gore_types.h"
#include <SDL.h>

enum debug_record_type {
	DebugRecord_None,

	DebugRecord_BeginTiming,
	DebugRecord_EndTiming,

	//NOTE(dima): sections are just categories that will be visible in the debug overlay
	DebugRecord_BeginSection,
	DebugRecord_EndSection,
	DebugRecord_FrameBarrier,

	DebugRecord_I32,
	DebugRecord_U32,
	DebugRecord_F32,
	DebugRecord_Text,
	DebugRecord_Vector2,
	DebugRecord_Vector3,
	DebugRecord_Vector4,
	DebugRecord_Color,

	DebugRecord_FramesSlider,
	DebugRecord_FramesGraph,
	DebugRecord_TopTotalClocks,
	DebugRecord_TopExClocks,
};

struct debug_record {
	char* Name;
	char* UniqueName;

	u32 RecordType;

	u64 Clocks;
	u16 ThreadID;
	union {
		float Value_F32;
	};
};

#define DEBUG_RECORD_MAX_COUNT 65536
struct debug_record_table {
	SDL_atomic_t CurrentRecordIndex;
	SDL_atomic_t CurrentTableIndex;

	SDL_atomic_t Increment;

	debug_record Records[2][DEBUG_RECORD_MAX_COUNT];
};

extern debug_record_table* GlobalRecordTable;

#define DEBUG_ID_TO_STRING(id) #id

#define DEBUG_UNIQUE_STRING_(id, func, line, counter) id "@" func "@" ## DEBUG_ID_TO_STRING(line) ## "@" ## DEBUG_ID_TO_STRING(counter)
#define DEBUG_UNIQUE_STRING(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, __LINE__, __COUNTER__)
#define DEBUG_UNIQUE_STRING2(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, 123456, 123)

#if 1
inline debug_record* DEBUGAddRecord(char* Name, char* UniqueName, u32 RecordType) {
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, GlobalRecordTable->Increment.value);
	//int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);
	Assert(Index < DEBUG_RECORD_MAX_COUNT);

	debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value] + Index;

	Record->Name = Name;
	Record->UniqueName = UniqueName;
	Record->Clocks = __rdtsc();
	Record->RecordType = RecordType;
	//TODO(dima): think about perfomance of this
	Record->ThreadID = SDL_ThreadID();
	//Record->ThreadID = 0;

	return(Record);
}
#else

#define DEBUGAddRecord(Name, UniqueName, RecordType)	\
	{																											\
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);										\
	Assert(Index < DEBUG_RECORD_MAX_COUNT);																		\
																												\
	debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value] + Index;		\
	*Record = {};																								\
	Record->Name = Name;																						\
	Record->UniqueName = UniqueName;																			\
	Record->Clocks = __rdtsc();																					\
	Record->RecordType = RecordType;																			\
	Record->ThreadID = SDL_ThreadID();}																			
#endif

inline void DEBUGSetRecording(b32 Recording) {
	GlobalRecordTable->Increment.value = Recording;
}

#define ADD_DEBUG_RECORD(name, type) DEBUGAddRecord(name, DEBUG_UNIQUE_STRING(name), type)

#define BEGIN_TIMING(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginTiming)
#define BEGIN_REPEATED_TIMING(name)  DEBUGAddRecord(name, DEBUG_UNIQUE_STRING2(name), DebugRecord_BeginTiming)
#define END_TIMING() ADD_DEBUG_RECORD("End", DebugRecord_EndTiming)
#define FUNCTION_TIMING() debug_timing FunctionTiming_##__COUNTER__(__FUNCTION__, DEBUG_UNIQUE_STRING(__FUNCTION__))

#define BEGIN_SECTION(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginSection)
#define END_SECTION() ADD_DEBUG_RECORD("EndSection", DebugRecord_EndSection)

#define DEBUG_FRAME_BARRIER(delta) {debug_record* Rec = DEBUGAddRecord("FrameBarrier", DEBUG_UNIQUE_STRING("FrameBarrier"), DebugRecord_FrameBarrier); Rec->Value_F32 = delta;}

struct debug_timing {
	char* Name;
	char* UniqueName;

	debug_timing(char* Name, char* UniqueName) {
		this->Name = Name;
		this->UniqueName = UniqueName;

		DEBUGAddRecord(Name, UniqueName, DebugRecord_BeginTiming);
	}

	~debug_timing() {
		DEBUGAddRecord(Name, UniqueName, DebugRecord_EndTiming);
	}
};

#endif