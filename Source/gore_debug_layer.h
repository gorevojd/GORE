#ifndef GORE_DEBUG_LAYER_H
#define GORE_DEBUG_LAYER_H

#include "gore_types.h"
#include "gore_math.h"
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

	DebugRecord_Value,
};

enum debug_log_type {
	DebugLog_Log,
	DebugLog_ErrLog,
	DebugLog_WarnLog,
	DebugLog_OkLog,
};

enum debug_value_type {
	DebugValue_I32,
	DebugValue_U32,
	DebugValue_F32,
	DebugValue_Text,
	DebugValue_Vector2,
	DebugValue_Vector3,
	DebugValue_Vector4,
	DebugValue_Color,
	DebugValue_StackedMemory,

	DebugValue_FramesSlider,
	DebugValue_ViewFrameInfo,
	DebugValue_ProfileOverlays,
	DebugValue_DebugStateInfo,

	DebugValue_Logger,

	//DebugValue_FramesGraph,
	//DebugValue_TopTotalClocks,
	//DebugValue_TopExClocks,
};

struct debug_record {
	char* Name;
	char* UniqueName;

	u32 RecordType;

	u64 Clocks;
	u16 ThreadID;

	struct {
		u32 ValueType;

		union {
			float Value_DebugValue_F32;
			i32 Value_DebugValue_I32;
			u32 Value_DebugValue_U32;
			char* Value_DebugValue_Text;
			v2* Value_DebugValue_Vector2;
			v3* Value_DebugValue_Vector3;
			v4* Value_DebugValue_Vector4;
			v4* Value_DebugValue_Color;
		};

	} Value_Value;
};

#define DEBUG_LOGS_COUNT 1024
#define DEBUG_LOG_SIZE 1024
#define DEBUG_RECORD_MAX_COUNT 65536
struct debug_record_table {
	SDL_atomic_t CurrentRecordIndex;
	SDL_atomic_t CurrentTableIndex;

	debug_record Records[2][DEBUG_RECORD_MAX_COUNT];
	SDL_atomic_t Increment;

	char Logs[DEBUG_LOGS_COUNT][DEBUG_LOG_SIZE];
	u32 LogsTypes[DEBUG_LOGS_COUNT];
	b32 LogsInited[DEBUG_LOGS_COUNT];
	SDL_atomic_t CurrentLogIndex;
	SDL_atomic_t LogIncrement;
};

extern debug_record_table* GlobalRecordTable;

#define DEBUG_ID_TO_STRING(id) #id

#define DEBUG_UNIQUE_STRING_(id, func, line, counter) id "@" func "@" ## DEBUG_ID_TO_STRING(line) ## "@" ## DEBUG_ID_TO_STRING(counter)
#define DEBUG_UNIQUE_STRING(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, __LINE__, __COUNTER__)
#define DEBUG_UNIQUE_STRING2(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, 123456, 123)

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

	return(Record);
}

void DEBUGAddLog(char* Text, char* File, int Line, u32 LogType);

inline void DEBUGSetRecording(b32 Recording) {
	SDL_AtomicSet(&GlobalRecordTable->Increment, Recording);
}

inline void DEBUGSetLogRecording(b32 Recording) {
	SDL_AtomicSet(&GlobalRecordTable->LogIncrement, Recording);
}

#define ADD_DEBUG_RECORD(name, type) DEBUGAddRecord(name, DEBUG_UNIQUE_STRING(name), type)

#define BEGIN_TIMING(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginTiming)
#define BEGIN_REPEATED_TIMING(name)  DEBUGAddRecord(name, DEBUG_UNIQUE_STRING2(name), DebugRecord_BeginTiming)
#define END_TIMING() ADD_DEBUG_RECORD("End", DebugRecord_EndTiming)
#define FUNCTION_TIMING() debug_timing FunctionTiming_##__COUNTER__(__FUNCTION__, DEBUG_UNIQUE_STRING(__FUNCTION__))

#define BEGIN_SECTION(name) ADD_DEBUG_RECORD(name, DebugRecord_BeginSection)
#define END_SECTION() ADD_DEBUG_RECORD("EndSection", DebugRecord_EndSection)

#define DEBUG_VALUE(value_type) {debug_record* Rec = ADD_DEBUG_RECORD("Value", DebugRecord_Value); Rec->Value_Value.ValueType = value_type;}
#define DEBUG_VALUE_SET_VALUE(rec, value_type, value) rec->Value_Value.Value_##value_type = value

#define DEBUG_LOG(log) DEBUGAddLog(log, __FILE__, __LINE__, DebugLog_Log)
#define DEBUG_ERROR_LOG(log) DEBUGAddLog(log, __FILE__, __LINE__, DebugLog_ErrLog)
#define DEBUG_OK_LOG(log) DEBUGAddLog(log, __FILE__, __LINE__, DebugLog_OkLog)
#define DEBUG_WARN_LOG(log) DEBUGAddLog(log, __FILE__, __LINE__, DebugLog_WarnLog)

#define DEBUG_FRAME_BARRIER(delta) {debug_record* Rec = DEBUGAddRecord("FrameBarrier", DEBUG_UNIQUE_STRING("FrameBarrier"), DebugRecord_FrameBarrier); Rec->Value_Value.Value_DebugValue_F32 = delta;}

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