#include "gore_threadwork_memory.h"

void InitThreadworkMemories(stacked_memory* Alloc, threadwork_memory* Memories, int Count, int MemorySizePerOne) {
	for (int ThreadworkMemory = 0;
		ThreadworkMemory < Count;
		ThreadworkMemory++)
	{
		threadwork_memory* Mem = Memories + ThreadworkMemory;
		Mem->UseState = ThreadworkUseState_None;
		Mem->MemoryInternal_ = SplitStackedMemory(Alloc, MemorySizePerOne);
	}
}

threadwork_memory* BeginThreadworkMemory(threadwork_memory* Memories, int Count) {
	threadwork_memory* Result = 0;

	b32 NotFound = 1;

	for (int ThreadworkIndex = 0;
		ThreadworkIndex < Count;
		ThreadworkIndex++)
	{
		threadwork_memory* Mem = Memories + ThreadworkIndex;

		if (PlatformApi.AtomicCAS_U32(
			(platform_atomic_type_u32*)&Mem->UseState, 
			ThreadworkUseState_InUse, 
			ThreadworkUseState_None) == ThreadworkUseState_None) 
		{
			Result = Mem;

			BeginTempStackedMemory(&Mem->MemoryInternal_, Mem->MemoryInternal_.MaxSize, MemAllocFlag_Align16);
			NotFound = 0;
			break;
		}
	}

	if (NotFound) {
		Assert(!"Empty threadwork was not found");
	}

	return(Result);
}

void EndThreadworkMemory(threadwork_memory* Memory) {
	EndTempStackedMemory(&Memory->MemoryInternal_, &Memory->Memory);

	PLATFORM_COMPILER_BARRIER();

	Memory->UseState = ThreadworkUseState_None;
}
