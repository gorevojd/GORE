#include "gore_profile.h"

void ProcessCollectedFrames(profile_state* State) {
	
	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;

	//TODO(DIma): atomic set current record index to zero

	for (u32 CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		profile_record* Record = &GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value][0];

		switch (Record->RecordType) {
			case ProfileRecord_BeginTiming: {

			}break;

			case ProfileRecord_EndTiming: {

			}break;
		}
	}
}
