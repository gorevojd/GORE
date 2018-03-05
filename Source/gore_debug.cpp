#include "gore_debug.h"

inline void DEBUGDeallocateTreeNode(debug_state* State, debug_tree_node* Entry) {
	Entry->PrevBro->NextBro = Entry->NextBro;
	Entry->NextBro->PrevBro = Entry->PrevBro;

	Entry->NextBro = State->FirstFreeBlock->NextBro;
	Entry->PrevBro = State->FirstFreeBlock;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline debug_tree_node* DEBUGAllocateTreeNode(debug_state* State) {
	debug_tree_node* Result = 0;

	if (State->FirstFreeBlock->NextBro != State->FirstFreeBlock) {
		Result = State->FirstFreeBlock->NextBro;

		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->DebugMemory, debug_tree_node);
	}

	return(Result);
}

inline void DEBUGInsertToList(debug_tree_node* Sentinel, debug_tree_node* Entry) {
	Entry->NextBro = Sentinel->NextBro;
	Entry->PrevBro = Sentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline debug_tree_node* DEBUGAllocateSentinelTreeNode(debug_state* State) {
	debug_tree_node* Sentinel = PushStruct(&State->DebugMemory, debug_tree_node);

	Sentinel->NextBro = Sentinel;
	Sentinel->PrevBro = Sentinel;

	return(Sentinel);
}

inline debug_tree_node* DEBUGInitLayerTreeNode(debug_state* State, debug_tree_node* CurrentBlock, char* Name, u32 Type) {
	debug_tree_node* NewBlock = 0;

	u32 BlockHashID = StringHashFNV(Name);

	debug_tree_node* At = CurrentBlock->ChildrenSentinel->PrevBro;
	for (At; At != CurrentBlock->ChildrenSentinel; At = At->PrevBro) {
#if 0
		if (StringsAreEqual(Record->UniqueName, At->Name)) {
#else
		if (BlockHashID == At->ID) {
#endif
			NewBlock = At;
			break;
		}
	}

	if (NewBlock == 0) {
		//NOTE(dima): Debug node not exist. Should be allocated.
		NewBlock = DEBUGAllocateTreeNode(State);

		*NewBlock = {};
		NewBlock->TreeNodeType = Type;
		NewBlock->Parent = CurrentBlock;
		NewBlock->UniqueName = PushString(&State->DebugMemory, Name);
		CopyStrings(NewBlock->UniqueName, Name);
		NewBlock->ID = StringHashFNV(NewBlock->UniqueName);

		NewBlock->ChildrenSentinel = DEBUGAllocateSentinelTreeNode(State);

		DEBUGInsertToList(CurrentBlock, NewBlock);
	}

	return(NewBlock);
}

static void DEBUGFreeCollectedFrameInfo(debug_profiled_frame* Frame) {

}

void DEBUGInit(debug_state* State) {
	State->DebugMemory = AllocateStackedMemory(MEGABYTES(16));
	State->FirstFreeBlock = DEBUGAllocateTreeNode(State);

	for (int FrameIndex = 0;
		FrameIndex < DEBUG_FRAMES_COUNT;
		FrameIndex++)
	{
		debug_profiled_frame* Frame = &State->Frames[FrameIndex];

		Frame->SectionSentinel = DEBUGAllocateSentinelTreeNode(State);
		Frame->TimingSentinel = DEBUGAllocateSentinelTreeNode(State);

		Frame->CurrentSection = Frame->SectionSentinel;
		Frame->CurrentTiming = Frame->TimingSentinel;

		for (int TimingStatisticIndex = 0;
			TimingStatisticIndex < DEBUG_TIMING_STATISTICS_COUNT;
			TimingStatisticIndex++)
		{
			debug_statistic* Stat = Frame->TimingStatistics[TimingStatisticIndex];

			Stat = 0;
		}
	}
}

void DEBUGProcessCollectedRecords(debug_state* State) {
	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;
	for (u32 CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		debug_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value];
		debug_profiled_frame* Frame = &State->Frames[State->ProcessFrameIndex];

		switch (Record->RecordType) {
			case DebugRecord_BeginTiming: {
				debug_tree_node* CurrentBlock = Frame->CurrentTiming;

				debug_tree_node* TimingNode = DEBUGInitLayerTreeNode(State, CurrentBlock, Record->UniqueName, DebugTreeNode_Timing);
				debug_timing_snapshot* TimingSnapshot = &TimingNode->TimingSnapshot;

				TimingSnapshot->BeginClock = Record->Clocks;
				TimingSnapshot->ThreadID = Record->ThreadID;
				TimingSnapshot->ChildrenSumClocks = 0;
				TimingSnapshot->HitCount = 0;
				TimingSnapshot->ClocksElapsed = 0;

				Frame->CurrentTiming = TimingNode;
			}break;

			case DebugRecord_EndTiming: {
#if 0
				profile_block_entry* CurrentBlock = Frame->CurrentTimingBlock;
				Assert(CurrentBlock->BlockEntryType == ProfileBlockEntry_Timing);

				profile_timing_snapshot* TimingSnapshot = &CurrentBlock->TimingSnapshot;

				//TODO(dima): Add this block total time to parent ChildrenSumClocks
				//TODO(dima): Or calculate childrens time

				TimingSnapshot->ClocksElapsed = Record->Clocks - TimingSnapshot->BeginClock;
				//????
				TimingSnapshot->HitCount += 1;

				TimingSnapshot->ChildrenSumClocks = 0;
				profile_block_entry* At = CurrentBlock->ChildrenSentinel->NextBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->NextBro) {
					TimingSnapshot->ChildrenSumClocks += At->TimingSnapshot.ClocksElapsed;
				}

				//NOTE(dima): Setting frame's current block to this block's parent
				Frame->CurrentTimingBlock = CurrentBlock->Parent;
#endif
				debug_tree_node* CurrentBlock = Frame->CurrentTiming;
				Assert(CurrentBlock->TreeNodeType == DebugTreeNode_Section);
				debug_timing_snapshot* TimingSnapshot = &CurrentBlock->TimingSnapshot;

				//NOTE(dima): difference between start and end clocks
				TimingSnapshot->ClocksElapsed = Record->Clocks - TimingSnapshot->BeginClock;

				//NOTE(dima): Going througn children and summing all their total clocks
				TimingSnapshot->ChildrenSumClocks = 0;
				debug_tree_node* At = CurrentBlock->ChildrenSentinel->NextBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->NextBro) {
					TimingSnapshot->ChildrenSumClocks += At->TimingSnapshot.ClocksElapsed;
				}

				//NOTE(dima): Finding needed element in the hash table
				u32 RecordHash = StringHashFNV(Record->UniqueName);
				u32 TimingStatisticIndex = RecordHash & (DEBUG_TIMING_STATISTICS_COUNT - 1);
				debug_statistic* TimingStatistic = Frame->TimingStatistics[TimingStatisticIndex];

				TimingStatistic->Timing.TotalClocks += TimingSnapshot->ClocksElapsed;
				TimingStatistic->Timing.HitCount += 1;
				TimingStatistic->Timing.TotalClocksInChildren = TimingSnapshot->ChildrenSumClocks;

				Frame->CurrentTiming = CurrentBlock->Parent;
			}break;

			case DebugRecord_BeginSection: {
				debug_tree_node* CurrentBlock = Frame->CurrentSection;

				debug_tree_node* NewBlock = DEBUGInitLayerTreeNode(State, CurrentBlock, Record->UniqueName, DebugTreeNode_Section);

				//NOTE(dima): Setting frame's current section to newly pushed block
				Frame->CurrentSection = NewBlock;
			}break;

			case DebugRecord_EndSection: {
				debug_tree_node* CurrentBlock = Frame->CurrentSection;
				Assert(CurrentBlock->TreeNodeType == DebugTreeNode_Section);

				//NOTE(dima): Setting frame's current section to this block's parent
				Frame->CurrentSection = CurrentBlock->Parent;
			}break;
		}

		int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
		SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
		SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);
	}
}