#ifndef GORE_DEBUG_H_INCLUDED
#define GORE_DEBUG_H_INCLUDED

/*
	NOTE(dima): Debug code and profiler idea were taken from
		Casey's Muratory show Handmade Hero but were written 
		from scratch. Now some basic frame collection code 
		was written and some debug overlays like frame slider,
		top clock list(both including and excluding children 
		timings), frame graphs(fps, delta time). 

		
*/

#include "gore_platform.h"
#include "gore_gui.h"

#include <SDL_atomic.h>
#include <SDL_thread.h>

#define DEBUG_SHOW_FRAME_GRAPH_TOOLTIPS 1
#define DEBUG_NORMALIZE_FRAME_GRAPH 1
#define DEBUG_FRAME_UPDATE_NODE_NAME "Frame update"

#if DEBUG_NORMALIZE_FRAME_GRAPH
#define DEBUG_NORMALIZE_FRAME_FREQUENCY 256
#endif

struct debug_timing_snapshot {
	u64 BeginClock;
	u64 ChildrenSumClocks;
	u64 ClocksElapsed;

	u32 ThreadID;

	u32 HitCount;
};

struct debug_value_node {
	u32 ValueType;
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
	char Name[64];

	debug_statistic* NextInHash;

	debug_statistic* NextAllocBro;
	debug_statistic* PrevAllocBro;

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

	//char* Name;
	char UniqueName[128];
	u32 ID;

	debug_tree_node* Parent;

	debug_tree_node* NextBro;
	debug_tree_node* PrevBro;

	debug_tree_node* ChildrenSentinel;

	union {
		debug_timing_snapshot TimingSnapshot;
		debug_value_node Value;
	};
};

#define DEBUG_TIMING_STATISTICS_COUNT 64

struct debug_profiled_frame {
	float DeltaTime;

	u32 RecordCount;

	stacked_memory FrameMemory;

	debug_tree_node* CurrentTiming;
	debug_tree_node* TimingRoot;
	debug_tree_node* FrameUpdateNode;

	debug_tree_node* CurrentSection;
	debug_tree_node* SectionSentinel;

	debug_statistic* TimingStatistics[DEBUG_TIMING_STATISTICS_COUNT];
	debug_statistic* TimingStatisticSentinel;
};

#define DEBUG_FRAMES_COUNT 256
#define DEBUG_LOGS_COUNT 1024
#define DEBUG_LOG_SIZE 1024
struct debug_state {
	debug_tree_node* FreeBlockSentinel;

	debug_tree_node* RootSection;
	debug_tree_node* CurrentSection;

	debug_profiled_frame Frames[DEBUG_FRAMES_COUNT];
	u32 NewestFrameIndex;
	u32 CollationFrameIndex;
	u32 LastCollationFrameIndex = 0;
	u32 ViewFrameIndex;
	u32 OldestFrameIndex;
	b32 OldeshShouldBeIncremented;

	stacked_memory* DebugMemory;

	u32 FramesGraphBarType;
	u32 RootNodeBarType;

#if DEBUG_NORMALIZE_FRAME_GRAPH
	u32 SegmentFrameCount;
	b32 NotFirstSegment;

	u32 MaxSegmentCollectedRecords;
	float MaxSegmentDT;
	float MaxSegmentFPS;

	u32 MaxLastSegmentCollectedRecords;
	float MaxLastSegmentDT;
	float MaxLastSegmentFPS;
#endif

	char** DebugLogs;
	u32* DebugLogsTypes;
	b32* DebugLogsInited;
	int DebugWriteLogIndex;

	u32 LastCollationFrameRecords;
	b32 IsRecording;
	b32 RecordingChanged;
	b32 RecordingChangedWasReenabled;

	gui_state* GUIState;
};

enum debug_frame_graph_type {
	DEBUGFrameGraph_DeltaTime,
	DEBUGFrameGraph_FPS,
	DEBUGFrameGraph_CollectedRecords,
	DEBUGFrameGraph_RootNodeBlocks,

	DEBUGFrameGraph_NodeClocks,

	DEBUGFrameGraph_Count,
};


extern void DEBUGInit(debug_state* State, stacked_memory* DEBUGMemoryBlock, gui_state* GUIState);
extern void DEBUGUpdate(debug_state* State);

#endif