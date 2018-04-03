#include "gore_platform.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

void DEBUGAddLog(char* Text, char* File, int Line, u32 LogType) {
	int NewLogIndex = (GlobalRecordTable->CurrentLogIndex + GlobalRecordTable->LogIncrement) % DEBUG_LOGS_COUNT;
	int Index = PlatformApi.AtomicSet_I32(&GlobalRecordTable->CurrentLogIndex, NewLogIndex);

	char* BufToFill = GlobalRecordTable->Logs[Index];
	stbsp_sprintf(BufToFill, "%s@%s@%d", Text, File, Line);
	GlobalRecordTable->LogsInited[Index] = 1;
	GlobalRecordTable->LogsTypes[Index] = LogType;
}