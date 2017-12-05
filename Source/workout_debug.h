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

#if 0

enum event_type {
	EventType_BeginTimedBlock,
	EventType_EndTimedBlock,
};

struct event_record {
	u32 Type;
	u64 Cycles;

	char* Name;
	char* UniversalName;
};

struct event_table {
#define MAX_EVENT_TABLE_COUNT 1024
	event_record Events[MAX_EVENT_TABLE_COUNT];

	volatile u64 CurrentEventIndex;
};

struct timed_block_entry {
	b32 IsValid;

	char* Name;
	u64 Cycles;

	u64 BeginCycles;
};

struct event_system {
#define MAX_TIMED_BLOCKS 128
	timed_block_entry TimedBlocks[MAX_TIMED_BLOCKS];
};

static event_table GlobalEventTable_;
static event_table* GlobalEventTable = &GlobalEventTable_;

inline void RecordEvent(u32 Type, char* Name, char* UniversalName) {
	u32 ToWriteIndex = SDL_AtomicAdd((SDL_atomic_t* volatile)&GlobalEventTable->CurrentEventIndex, 1);

	event_record* Event = &GlobalEventTable->Events[ToWriteIndex];
	Event->Cycles = __rdtsc();
	Event->Type = Type;
	Event->Name = Name;
	Event->UniversalName = UniversalName;
}

#define UNIVERSAL_EVENT_NAME__(name, file, line) name "#" file "#" #line
#define UNIVERSAL_EVENT_NAME_(name, file, line) UNIVERSAL_EVENT_NAME__(name, file, line)
#define UNIVERSAL_EVENT_NAME(name) UNIVERSAL_EVENT_NAME_(name, __FILE__, __LINE__)

#define BEGIN_TIMED_BLOCK(name) RecordEvent(EventType_BeginTimedBlock, name, UNIVERSAL_EVENT_NAME(name))
#define END_TIMED_BLOCK(name) RecordEvent(EventType_EndTimedBlock, name, UNIVERSAL_EVENT_NAME(name))

struct timed_block {
	char* Name;
	timed_block(char* Name) {
		this->Name = Name;
		//BEGIN_TIMED_BLOCK(Name);
	}

	~timed_block() {
		//END_TIMED_BLOCK(Name);
	}
};

#define TIMED_FUNCTION() 

inline u32 GetHashForEvent(event_record* Event) {
	char* At = Event->UniversalName;

	u32 ResultHash = 1531;

	while (*At) {
		ResultHash += (*At) * 14851 + 1901;

		At++;
	}

	return(ResultHash);
}

static void ProcessEvents(event_system* System) {
	u32 EventCount = GlobalEventTable->CurrentEventIndex;

	for (u32 EventIndex = 0;
		EventIndex < EventCount;
		EventIndex++)
	{
		event_record* Event = &GlobalEventTable->Events[EventIndex];

		switch (Event->Type) {
		case(EventType_BeginTimedBlock): {
			u32 EventHash = GetHashForEvent(Event);

			u32 BlockEntryIndex = EventHash % ArrayCount(System->TimedBlocks);
			timed_block_entry* Entry = System->TimedBlocks + BlockEntryIndex;
			Entry->IsValid = true;


			Entry->BeginCycles = Event->Cycles;
		}break;

		case(EventType_EndTimedBlock): {
			u32 EventHash = GetHashForEvent(Event);

			u32 BlockEntryIndex = EventHash % ArrayCount(System->TimedBlocks);
			timed_block_entry* Entry = System->TimedBlocks + BlockEntryIndex;

			//Entry->Cycles += Event->Cycles - Entry->BeginCycles;
			Entry->Cycles = Event->Cycles - Entry->BeginCycles;
		}break;

		default: {
			Assert(!"Invalid event type");
		}break;
		}
	}
}

static void OverlayTimedBlockStates(event_system* System, gui_state* GUIState) {
	for (u32 EntryIndex = 0;
		EntryIndex < ArrayCount(System->TimedBlocks);
		EntryIndex++)
	{
		timed_block_entry* Entry = System->TimedBlocks + EntryIndex;

		if (Entry->IsValid) {
			char Buffer[256];

			stbsp_snprintf(Buffer, 256, "%s %Uc", Entry->Name, Entry->Cycles);

			PrintText(Buffer);
		}
	}
}

#endif

#endif