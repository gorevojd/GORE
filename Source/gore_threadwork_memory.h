#ifndef GORE_THREADWORK_MEMORY_H_INCLUDED
#define GORE_THREADWORK_MEMORY_H_INCLUDED

#include "gore_platform.h"

enum threadwork_memory_use_state {
	ThreadworkUseState_None,
	ThreadworkUseState_InUse,
};

struct threadwork_memory {
	stacked_memory Memory;
	stacked_memory MemoryInternal_;

	platform_atomic_type_u32 UseState;
};

extern void InitThreadworkMemories(stacked_memory* Alloc, threadwork_memory* Memories, int Count, int MemorySizePerOne);
extern threadwork_memory* BeginThreadworkMemory(threadwork_memory* Memories, int Count);
extern void EndThreadworkMemory(threadwork_memory* Memory);

#endif

