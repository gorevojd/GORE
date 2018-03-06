#ifndef GORE_DEBUG_H_INCLUDED
#define GORE_DEBUG_H_INCLUDED


#include "gore_platform.h"
#include "gore_gui.h"

#include <SDL_atomic.h>
#include <SDL_thread.h>

enum debug_record_type {
	ProfileRecord_None,

	DebugRecord_BeginTiming,
	DebugRecord_EndTiming,

	//NOTE(dima): sections are just categories that will be visible in the debug overlay
	DebugRecord_BeginSection,
	DebugRecord_EndSection,
	DebugRecord_Value,
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

struct debug_timing_snapshot {
	u64 BeginClock;
	u64 ChildrenSumClocks;
	u64 ClocksElapsed;

	u32 ThreadID;

	u32 HitCount;
};

enum debug_value_type {
	DebugValue_I32,
	DebugValue_U32,
	DebugValue_Vector2,
	DebugValue_Vector3,
	DebugValue_Vector4,
};

struct debug_value_link {
	u32 Type;

	void* Value;
};

struct debug_record_table {
	SDL_atomic_t CurrentRecordIndex;
	SDL_atomic_t CurrentTableIndex;

	int RecordsMaxCount;
	debug_record Records[2][4096];
};

enum debug_statistic_type {
	DebugTimingStatistic_None,

	DebugStatistic_Function,
	DebugStatistic_Timing,
	DebugStatistic_Thread,
};

struct debug_statistic_timing {
	u64 TotalClocks;
	u64 TotalClocksInChildren;
	u32 HitCount;
};

struct debug_statistic {
	u32 Type;

	u32 ID;

	debug_statistic* NextInHash;

	debug_statistic* NextBro;
	debug_statistic* PrevBro;

	union {
		debug_statistic_timing Timing;
	};
};

enum debug_tree_node_type {
	DebugTreeNode_None,

	DebugTreeNode_Timing,
	DebugTreeNode_Section,
	DebugTreeNode_Value,
};

struct debug_id {
	char* UniqueName;
	char* Name;
	char* Function;
	u32 ID;
};

struct debug_tree_node {
	u32 TreeNodeType;

	char* UniqueName;
	u32 ID;

	debug_tree_node* Parent;

	debug_tree_node* NextBro;
	debug_tree_node* PrevBro;

	debug_tree_node* ChildrenSentinel;

	union {
		debug_timing_snapshot TimingSnapshot;
	};
};

#define DEBUG_TIMING_STATISTICS_COUNT 64

struct debug_profiled_frame {
	debug_tree_node* CurrentTiming;
	debug_tree_node* TimingSentinel;

	debug_tree_node* CurrentSection;
	debug_tree_node* SectionSentinel;

	debug_statistic* TimingStatistics[DEBUG_TIMING_STATISTICS_COUNT];
	debug_statistic* TimingStatisticSentinel;
};

#define DEBUG_FRAMES_COUNT 256
struct debug_state {
	debug_tree_node* FreeBlockSentinel;
	debug_statistic* FreeStatisticSentinel;

	debug_tree_node* RootSection;
	debug_tree_node* CurrentSection;

	debug_profiled_frame Frames[DEBUG_FRAMES_COUNT];
	u32 ProcessFrameIndex;

	stacked_memory DebugMemory;

	gui_state* GUIState;
};


extern debug_record_table* GlobalRecordTable;

#define DEBUG_ID_TO_STRING(id) #id

#define DEBUG_UNIQUE_STRING_(id, func, line, counter) id "@" func "@" ## DEBUG_ID_TO_STRING(line) ## "@" ## DEBUG_ID_TO_STRING(counter)
#define DEBUG_UNIQUE_STRING(id) DEBUG_UNIQUE_STRING_(id, __FUNCTION__, __LINE__, __COUNTER__)

inline void DEBUGAddRecord(char* Name, char* UniqueName, u32 RecordType) {
	int Index = SDL_AtomicAdd(&GlobalRecordTable->CurrentRecordIndex, 1);
	Assert(Index < GlobalRecordTable->RecordsMaxCount);

	debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value] + Index;

	Record->Name = Name;
	Record->UniqueName = UniqueName;
	Record->Clocks = __rdtsc();
	Record->RecordType = RecordType;
	//TODO(dima): think about perfomance of this
	Record->ThreadID = SDL_ThreadID();
}

#define ADD_DEBUG_RECORD(name, type) DEBUGAddRecord(name, DEBUG_UNIQUE_STRING(name), type)

#define BEGIN_TIMING(name) ADD_DEBUG_RECORD(name, ProfileRecord_BeginTiming)
#define END_TIMING() ADD_DEBUG_RECORD("End", ProfileRecord_EndTiming)

#define FUNCTION_TIMING() debug_timing FunctionTiming_##__COUNTER__(__FUNCTION__, DEBUG_UNIQUE_STRING(__FUNCTION__));

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