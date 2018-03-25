#include "gore_debug_layer.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"


void DEBUGAddLog(char* Text, char* File, int Line, u32 LogType) {
	int NewLogIndex = (GlobalRecordTable->CurrentLogIndex.value + GlobalRecordTable->LogIncrement.value) % DEBUG_LOGS_COUNT;
	int Index = SDL_AtomicSet(&GlobalRecordTable->CurrentLogIndex, NewLogIndex);

	char* BufToFill = GlobalRecordTable->Logs[Index];
	stbsp_sprintf(BufToFill, "%s\n%s\n%d", Text, File, Line);
	GlobalRecordTable->LogsInited[Index] = 1;
	GlobalRecordTable->LogsTypes[Index] = LogType;
}