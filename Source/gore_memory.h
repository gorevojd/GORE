#ifndef GORE_STACKED_MEMORY_H_DEFINED
#define GORE_STACKED_MEMORY_H_DEFINED


#include "gore_types.h"

enum memory_allocation_flag {
	MemAllocFlag_None = 0,

	MemAllocFlag_ClearTo0 = 1,

	MemAllocFlag_Align4 = 2,
	MemAllocFlag_Align8 = 4,
	MemAllocFlag_Align16 = 8,
};

struct stacked_memory {
	void* BaseAddress;
	u32 Used;
	u32 MaxSize;

	u32 FragmentationBytesCount;

	char* DEBUGName;

	//NOTE(dima): Used for temp memory
	u32 InitUsed;
};

inline u32 GetAlignValueFromAllocationFlag(u32 AllocationFlag) {
	u32 Result = 4;

	if (AllocationFlag & MemAllocFlag_Align4) {
		Result = 4;
	}
	
	if (AllocationFlag & MemAllocFlag_Align8) {
		Result = 8;
	}

	if (AllocationFlag & MemAllocFlag_Align16) {
		Result = 16;
	}

	return(Result);
}

inline stacked_memory InitStackedMemory(void* Memory, u32 MaxSize, u32 AllocationFlag = 0) {
	stacked_memory Result = {};

	Result.BaseAddress = (u8*)Memory;
	Result.Used = 0;
	Result.MaxSize = MaxSize;
	Result.FragmentationBytesCount = 0;

	return(Result);
}

inline stacked_memory BeginTempStackedMemory(stacked_memory* Stack, u32 Size) {
	stacked_memory Result = {};

	//NOTE(dima): checking for overlapping the initial stack
	Assert(Stack->Used + Size <= Stack->MaxSize);

	Result.BaseAddress = (u8*)Stack->BaseAddress + Stack->Used;
	Result.MaxSize = Size;
	Result.Used = 0;
	Result.InitUsed = Stack->Used;
	Result.FragmentationBytesCount = 0;

	Stack->Used += Size;

	return(Result);
}

inline void EndTempStackedMemory(stacked_memory* Stack, stacked_memory* TempMem) {
	Stack->BaseAddress = TempMem->BaseAddress;
	Stack->Used = TempMem->InitUsed;
}

inline stacked_memory SplitStackedMemory(stacked_memory* Stack, u32 Size) {
	stacked_memory Result = {};

	Assert(Stack->Used + Size <= Stack->MaxSize);
	Result.BaseAddress = (u8*)Stack->BaseAddress + Stack->Used;
	Result.Used = 0;
	Result.MaxSize = Size;
	Result.FragmentationBytesCount = 0;

	Stack->Used += Size;

	return(Result);
}

inline u8* PushSomeMemory(stacked_memory* Mem, u32 ByteSize, i32 Align = 4) {

	u32 AlignOffset = GET_ALIGN_OFFSET(Mem->BaseAddress, Align);

	Assert(Mem->Used + ByteSize + AlignOffset <= Mem->MaxSize);

#if 1
	u8* Result = (u8*)Mem->BaseAddress + Mem->Used + AlignOffset;
	Mem->Used = Mem->Used + ByteSize + AlignOffset;
#else
	u32 MemUsedPrev = PlatformApi.AtomicAdd_U32((platform_atomic_type_u32*)&Mem->Used, ByteSize + AlignOffset);
	u8* Result = (u8*)Mem->BaseAddress + MemUsedPrev + AlignOffset;
#endif

	Mem->FragmentationBytesCount += AlignOffset;

	return(Result);
}

inline void* GetCurrentMemoryBase(stacked_memory* Mem) {
	void* Result = (void*)((u8*)Mem->BaseAddress + Mem->Used);

	return(Result);
}

#define PushStruct(StMem, type) (type*)PushSomeMemory(StMem, sizeof(type))
#define PushStructUnaligned(StMem, type) (type*)PushSomeMemory(StMem, sizeof(type), 1)
#define PushArray(StMem, type, count) (type*)PushSomeMemory(StMem, sizeof(type) * count)
#define PushString(StMem, str) (char*)PushSomeMemory(StMem, StringLength(str) + 1)

void ClearStackedMemory(stacked_memory* Mem);

#endif