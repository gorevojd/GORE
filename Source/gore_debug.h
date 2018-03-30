#ifndef GORE_DEBUG_H_INCLUDED
#define GORE_DEBUG_H_INCLUDED

/*
	NOTE(dima): Debug code and profiler idea were taken from
		Casey's Muratory show Handmade Hero but were written 
		from scratch. Now some basic frame collection code 
		was written and some debug overlays like frame slider,
		top clock list(both including and excluding children 
		timings), frame graphs(fps, delta time, collected record count). 

		
*/

#include "gore_platform.h"
#include "gore_gui.h"

#include <SDL_atomic.h>
#include <SDL_thread.h>

#define DEBUG_SHOW_FRAME_GRAPH_TOOLTIPS 1
#define DEBUG_NORMALIZE_FRAME_GRAPH 1
#define DEBUG_FRAME_UPDATE_NODE_NAME "Frame update"

#if DEBUG_NORMALIZE_FRAME_GRAPH
#define DEBUG_NORMALIZE_FRAME_FREQUENCY 64
#endif

struct debug_timing_snapshot {
	u64 BeginClocks;
	u64 ClocksElapsed;
	u64 ClocksElapsedInChildren;
	u32 HitCount;
};

struct debug_value_node {
	u32 ValueType;


};

enum debug_timing_statistic_type {
	DebugTimingStatistic_None,

	DebugStatistic_Timing,
};


struct debug_timing_statistic {
	u32 ID;
	char Name[64];

	debug_timing_statistic* NextInHash;

	debug_timing_statistic* NextAllocBro;
	debug_timing_statistic* PrevAllocBro;

	debug_timing_snapshot Timing;
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

#define DEBUG_TIMING_STATISTIC_TABLE_SIZE 64
#define DEBUG_THREAD_TABLE_SIZE 16
#define DEBUG_THREAD_FRAME_INFOS_POOL_SIZE 16
#define DEBUG_FRAMES_COUNT 256

struct debug_thread_frame_info{
	b32 Initialized;

	stacked_memory* MemoryAllocPointer;

	debug_tree_node* CurrentTiming;
	debug_tree_node* TimingRoot;
};

struct debug_thread {
	u32 ThreadID;

	debug_thread* NextAlloc;
	debug_thread* PrevAlloc;

	debug_thread* NextInHash;

	/* 
		NOTE(dima): ThreadFrameInfos is the array of size
		DEBUG_FRAMES_COUNT of structures debug_thread_frame_infos.
		Actually if we need to allocate it in the middle 
		of the code it means that we should clear it there
		too. It can cost a much processor time. So the 
		pattern here is to preallocate pool of arrays 
		debug_thread_frame_info** and then just increment
		int that array as we should allocate one.
		
	*/
	debug_thread_frame_info** ThreadFrameInfos;
};

struct debug_profiled_frame {
	float DeltaTime;

	u32 RecordCount;

	stacked_memory FrameMemory;

#if 0
	debug_tree_node* CurrentTiming;
	debug_tree_node* TimingRoot;
#endif

	debug_tree_node* FrameUpdateNode;

	debug_tree_node* CurrentSection;

	debug_timing_statistic* TimingStatistics[DEBUG_TIMING_STATISTIC_TABLE_SIZE];
	debug_timing_statistic* TimingStatisticSentinel;
};

struct debug_state {
	debug_tree_node* FreeBlockSentinel;

	debug_tree_node* RootSection;
	debug_tree_node* CurrentSection;

	debug_thread_frame_info* ThreadFrameInfosPool[DEBUG_THREAD_FRAME_INFOS_POOL_SIZE];
	u32 ThreadFrameInfoPoolIndex;
	debug_thread* Threads[DEBUG_THREAD_TABLE_SIZE];
	debug_thread* ThreadSentinel;
	debug_thread* MainThread;

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

#if 0
	char** DebugLogs;
	u32* DebugLogsTypes;
	b32* DebugLogsInited;
	int DebugWriteLogIndex;
	b32 DebugLogStopped;
#endif
	u32 DebugLoggerActionHappened;
	u32 DebugLoggerFilterType;
	float InLoggerFontScale;

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