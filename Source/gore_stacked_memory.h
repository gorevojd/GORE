#ifndef GORE_STACKED_MEMORY_H_DEFINED
#define GORE_STACKED_MEMORY_H_DEFINED

#include <stdlib.h>

struct stacked_memory {
	u8* BaseAddress;
	u32 Used;
	u32 MaxSize;
};

inline stacked_memory AllocateStackedMemory(u32 Size) {
	stacked_memory Result = {};

	u32 MemoryToAlloc = Size;

	Result.BaseAddress = (u8*)malloc(MemoryToAlloc);
	Result.Used = 0;
	Result.MaxSize = MemoryToAlloc;
	return(Result);
}

inline void DeallocateStackedMemory(stacked_memory* Mem) {
	if (Mem->BaseAddress) {
		free(Mem->BaseAddress);
	}
	Mem->MaxSize = 0;
	Mem->Used = 0;
}

inline u8* PushSomeMemory(stacked_memory* Mem, u32 ByteSize, i32 Align = 1) {

	u32 AlignOffset = GET_ALIGN_OFFSET(Mem->BaseAddress, Align);

	Assert(Mem->Used + ByteSize + AlignOffset <= Mem->MaxSize);

	u8* Result = Mem->BaseAddress + Mem->Used + AlignOffset;
	Mem->Used = Mem->Used + ByteSize + AlignOffset;

	return(Result);
}

#define PushStruct(StMem, type) (type*)PushSomeMemory(StMem, sizeof(type))
#define PushArray(StMem, type, count) (type*)PushSomeMemory(StMem, sizeof(type) * count)
#define PushString(StMem, str) (char*)PushSomeMemory(StMem, StringLength(str) + 1)

#endif