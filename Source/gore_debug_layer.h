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
};

struct debug_record {
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

#define DEBUG_RECORD_MAX_COUNT 4096
struct debug_record_table {
	SDL_atomic_t CurrentRecordIndex;
	SDL_atomic_t CurrentTableIndex;

	debug_record Records[2][DEBUG_RECORD_MAX_COUNT];
};

extern debug_record_table* GlobalRecordTable;

#define DEBUG_ID_TO_STRING(id) #id

#define DEBUG_UNIQUE_STRING_(id, func, line, counter) id "@" func "@" ## DEBUG_ID_TO_STRING(line) ## "@" ## DEBUG_ID_TO_STRING(counter)
#define DEBUG_UNIQUE_STRING(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, __LINE__, __COUNTER__)

inline void DEBUGAddRecord(char* Name, char* UniqueName, u32 RecordType) {
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);
	Assert(Index < DEBUG_RECORD_MAX_COUNT);

	debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value] + Index;

	Record->Name = Name;
	Record->UniqueName = UniqueName;
	Record->Clocks = __rdtsc();
	Record->RecordType = RecordType;
	//TODO(dima): think about perfomance of this
	Record->ThreadID = SDL_ThreadID();
	//Record->ThreadID = 0;
}

#define ADD_DEBUG_RECORD(name, type) DEBUGAddRecord(name, DEBUG_UNIQUE_STRING(name), type)

#define BEGIN_TIMING(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginTiming)
#define END_TIMING() ADD_DEBUG_RECORD("End", DebugRecord_EndTiming)
#define FUNCTION_TIMING() debug_timing FunctionTiming_##__COUNTER__(__FUNCTION__, DEBUG_UNIQUE_STRING(__FUNCTION__))

#define BEGIN_SECTION(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginSection)
#define END_SECTION() ADD_DEBUG_RECORD("EndSection", DebugRecord_EndSection)

#define DEBUG_FRAME_BARRIER() DEBUGAddRecord("FrameBarrier", DEBUG_UNIQUE_STRING("FrameBarrier"), DebugRecord_FrameBarrier)

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