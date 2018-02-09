#ifndef WORKOUT_DEBUG_H
#define WORKOUT_DEBUG_H

#include "workout_game_layer.h"
#include "workout_gui.h"

enum debug_counter_type {
	DebugCounterType_BitmapPixelFill,
	DebugCounterType_GradientPixelFill,
	DebugCounterType_ClearPixelFill,
	DebugCounterType_EventProcessing,
	DebugCounterType_RectPixelFill,

	DebugCounterType_Count,
};

struct debug_counter {
	u64 Clocks;
	u32 Hits;

	char* Name;

	u64 LastBeginClock;
};

struct debug_state {
	debug_counter Counters[DebugCounterType_Count];

	//TODO(Dima):
	int CountersToInit;
	int CountersInited;

	render_stack GUIRenderStack;
};

extern debug_state* GlobalDebugState;

inline void BeginTimedBlock(debug_state* DebugState, u32 Type) {
	debug_counter* Counter = &DebugState->Counters[Type];

	Counter->LastBeginClock = __rdtsc();
}

inline void EndTimedBlock(debug_state* DebugState, u32 Type, u32 HitCount = 1) {
	debug_counter* Counter = &DebugState->Counters[Type];

	Counter->Hits += HitCount;
	Counter->Clocks += (__rdtsc() - Counter->LastBeginClock);
}

#define BEGIN_TIMED_BLOCK(block) BeginTimedBlock(GlobalDebugState, DebugCounterType_##block)
#define END_TIMED_BLOCK(block, ...) EndTimedBlock(GlobalDebugState, DebugCounterType_##block, __VA_ARGS__)

extern void InitDebugCounters(debug_state* State);
extern void ClearDebugCounters(debug_state* DebugState);
extern void OverlayCycleCounters(debug_state* DebugState, gui_state* GUIState);
extern void InitDEBUG(debug_state* State, font_info* FontInfo);
extern void BeginDEBUG(debug_state* State);
extern void EndDEBUG(debug_state* State);

#endif