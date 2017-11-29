#include "workout_debug.h"

inline void DEBUGInitCounter(debug_state* State, u32 Type, char* BlockName) {
	debug_counter* Counter = &State->Counters[Type];

	Counter->Name = BlockName;
	Counter->Clocks = 0;
	Counter->Hits = 0;
	Counter->LastBeginClock = 0;
}

#ifndef ID_TO_STRING
#define ID_TO_STRING(id) #id
#endif

#define DEBUG_INIT_COUNTER(block) DEBUGInitCounter(GlobalDebugState, DebugCounterType_##block, ID_TO_STRING(block))

void InitDebugCounters(debug_state* State) {
	DEBUG_INIT_COUNTER(BitmapPixelFill);
	DEBUG_INIT_COUNTER(GradientPixelFill);
	DEBUG_INIT_COUNTER(ClearPixelFill);
	DEBUG_INIT_COUNTER(EventProcessing);
}

void ClearDebugCounters(debug_state* DebugState) {
	for (int CounterIndex = 0;
		CounterIndex < DebugCounterType_Count;
		CounterIndex++)
	{
		debug_counter* Counter = &DebugState->Counters[CounterIndex];

		Counter->Clocks = 0;
		Counter->Hits = 0;
		Counter->LastBeginClock = 0;
	}
}

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include <stdio.h>
void OverlayCycleCounters(debug_state* DebugState, gui_state* GUIState) {
	BeginTempGUIRenderStack(GUIState, &DebugState->GUIRenderStack);

	for (int CounterIndex = 0;
		CounterIndex < DebugCounterType_Count;
		CounterIndex++)
	{
		debug_counter* Counter = &DebugState->Counters[CounterIndex];

		char Buffer[256];
		
		float CyclesPerHit = 0;
		if (Counter->Hits) {
			CyclesPerHit = (float)Counter->Clocks / (float)Counter->Hits;

			stbsp_snprintf(Buffer, 256, "%20s: %10llucy, %10uh, %12.2fcy/h", 
				Counter->Name, 
				Counter->Clocks,
				Counter->Hits,
				CyclesPerHit);

			PrintText(GUIState, Buffer);
		}
	}

	EndTempGUIRenderStack(GUIState);
}

void BeginDEBUG(debug_state* State) {
	State->GUIRenderStack = BeginRenderStack();
	//BeginFrameGUI(&State->DebugGUI, &State->GUIRenderStack);
}

void EndDEBUG(debug_state* State) {
	//EndFrameGUI(&State->DebugGUI);
	EndRenderStack(&State->GUIRenderStack);
}

void InitDEBUG(debug_state* State, font_info* FontInfo) {
	//InitGUIState(&State->DebugGUI, FontInfo);

	InitDebugCounters(State);
}