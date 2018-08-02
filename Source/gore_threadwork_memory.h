#ifndef GORE_THREADWORK_MEMORY_H_INCLUDED
#define GORE_THREADWORK_MEMORY_H_INCLUDED

#include "gore_platform.h"

struct threadwork_data {
	threadwork_data* Next;
	threadwork_data* Prev;

	//NOTE(dima): 1 - in use; 0 - free;
	platform_atomic_type_u32 UseState;

	stacked_memory Memory;
	stacked_memory MemoryInternal;
};

struct threadwork_data_set {
	int FreeThreadworksCount;
	int TotalThreadworksCount;

	int MemUsed;

	platform_mutex ThreadworksMutex;

	threadwork_data* UseSentinel;
	threadwork_data* FreeSentinel;
};

threadwork_data* BeginThreadworkData(threadwork_data_set* Set);
void EndThreadworkData(threadwork_data* Threadwork, threadwork_data_set* Set);

void InitThreadworkDataSet(
	stacked_memory* InitMemory,
	threadwork_data_set* Set,
	int ThreadworksCount,
	int OneThreadworkSize);
#endif

