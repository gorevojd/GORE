#include "gore_profile.h"

inline void PROFDeallocateBlockEntry(profile_state* State, profile_block_entry* Entry) {
	Entry->PrevBro->NextBro = Entry->NextBro;
	Entry->NextBro->PrevBro = Entry->PrevBro;

	Entry->NextBro = State->FirstFreeBlock->NextBro;
	Entry->PrevBro = State->FirstFreeBlock;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline profile_block_entry* PROFAllocateBlockEntry(profile_state* State) {
	profile_block_entry* Result = 0;

	if (State->FirstFreeBlock->NextBro != State->FirstFreeBlock) {
		Result = State->FirstFreeBlock->NextBro;

		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->ProfileMemory, profile_block_entry);
	}

	return(Result);
}

inline void PROFInsertToList(profile_block_entry* Sentinel, profile_block_entry* Entry) {
	Entry->NextBro = Sentinel->NextBro;
	Entry->PrevBro = Sentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

void PROFProcessCollectedFrames(profile_state* State) {
	
	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;
	for (u32 CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		profile_record* Record = GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value];
		profile_frame* Frame = &State->Frames[State->CollectionFrameIndex];

		switch (Record->RecordType) {
			case ProfileRecord_BeginTiming: {
				profile_block_entry* CurrentBlock = Frame->CurrentTimingBlock;

				profile_block_entry* NewBlock = 0;

				u32 BlockHashID = StringHashFNV(Record->UniqueName);

				profile_block_entry* At = CurrentBlock->ChildrenSentinel->PrevBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->PrevBro) {
					//TODO(dima): compare IDs instead of strings
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
					//NOTE(dima): Profile block not exist. Should be allocated.
					NewBlock = PROFAllocateBlockEntry(State);

					*NewBlock = {};
					NewBlock->BlockEntryType = ProfileBlockEntry_Timing;
					NewBlock->Parent = CurrentBlock;
					NewBlock->Name = PushString(&State->ProfileMemory, Record->UniqueName);
					CopyStrings(NewBlock->Name, Record->UniqueName);
					NewBlock->ID = StringHashFNV(NewBlock->Name);

					PROFInsertToList(CurrentBlock, NewBlock);
				}

				profile_timing_snapshot* TimingSnapshot = &NewBlock->TimingSnapshot;

				TimingSnapshot->BeginClock = Record->Clocks;
				TimingSnapshot->ThreadID = Record->ThreadID;
				TimingSnapshot->ChildrenSumClocks = 0;

				//TODO(dima): Allocate children sentinel;

				//NOTE(dima): Setting frame's current block to newly pushed block
				Frame->CurrentTimingBlock = NewBlock;
			}break;

			case ProfileRecord_EndTiming: {
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
				Frame->CurrentTimingBlock = CurrentBlock->Parent
			}break;
		}
	}

	int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
	SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
	SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);
}
