#include "gore_profile.h"

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

				profile_block_entry* At = CurrentBlock->ChildrenSentinel->PrevBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->PrevBro) {
					//TODO(dima): compare IDs instead of strings
					if (StringsAreEqual(Record->UniqueName, At->Name)) {
						NewBlock = At;
						break;
					}
				}

				if (NewBlock == 0) {
					//TODO(dima): Allocate new block
				}

				profile_timing_snapshot* TimingSnapshot = &NewBlock->TimingSnapshot;

				TimingSnapshot->BeginClock = Record->Clocks;
				TimingSnapshot->ThreadID = Record->ThreadID;

			}break;

			case ProfileRecord_EndTiming: {
				profile_block_entry* CurrentBlock = Frame->CurrentTimingBlock;
				Assert(CurrentBlock->BlockEntryType == ProfileBlockEntry_Timing);


			}break;
		}
	}

	int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
	SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
	SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);
}
