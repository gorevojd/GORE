#include "gore_threadwork_memory.h"

static threadwork_data* VoxelAllocateThreadwork(
	stacked_memory* Memory,
	u32 ThreadworkMemorySize,
	int* MemoryCounter)
{
	threadwork_data* Result = PushStruct(Memory, threadwork_data);

	*MemoryCounter += sizeof(threadwork_data) + ThreadworkMemorySize;

	Result->Next = Result;
	Result->Prev = Result;

	Result->UseState = 0;
	Result->MemoryInternal = SplitStackedMemory(Memory, ThreadworkMemorySize);

	return(Result);
}

static inline void InsertThreadworkAfter(
	threadwork_data* ToInsert,
	threadwork_data* Sentinel)
{
	ToInsert->Next = Sentinel->Next;
	ToInsert->Prev = Sentinel;

	ToInsert->Next->Prev = ToInsert;
	ToInsert->Prev->Next = ToInsert;
}

threadwork_data* BeginThreadworkData(threadwork_data_set* Set)
{
	threadwork_data* Result = 0;

	BeginMutexAccess(&Set->ThreadworksMutex);

	if (Set->FreeSentinel->Next != Set->FreeSentinel) {
		//NOTE(dima): Putting threadwork list entry to use list
		Result = Set->FreeSentinel->Next;

		Result->Prev->Next = Result->Next;
		Result->Next->Prev = Result->Prev;

		Result->Next = Set->UseSentinel->Next;
		Result->Prev = Set->UseSentinel;

		Result->Next->Prev = Result;
		Result->Prev->Next = Result;

		//NOTE(dima): Beginning temp memory
		Result->Memory = BeginTempStackedMemory(
			&Result->MemoryInternal,
			Result->MemoryInternal.MaxSize);

		Set->FreeThreadworksCount--;
	}

	EndMutexAccess(&Set->ThreadworksMutex);

	return(Result);
}

void EndThreadworkData(
	threadwork_data* Threadwork,
	threadwork_data_set* Set)
{
	BeginMutexAccess(&Set->ThreadworksMutex);

	//NOTE(dima): Putting threadwork list entry to free list
	Threadwork->Prev->Next = Threadwork->Next;
	Threadwork->Next->Prev = Threadwork->Prev;

	Threadwork->Next = Set->FreeSentinel->Next;
	Threadwork->Prev = Set->FreeSentinel;

	Threadwork->Next->Prev = Threadwork;
	Threadwork->Prev->Next = Threadwork;

	//NOTE(dima): Freing temp memory
	EndTempStackedMemory(&Threadwork->MemoryInternal, &Threadwork->Memory);

	Set->FreeThreadworksCount++;

	EndMutexAccess(&Set->ThreadworksMutex);
}

void InitThreadworkDataSet(
	stacked_memory* InitMemory,
	threadwork_data_set* Set,
	int ThreadworksCount,
	int OneThreadworkSize)
{
	Set->ThreadworksMutex = {};
	Set->MemUsed = 0;

	Set->UseSentinel = VoxelAllocateThreadwork(
		InitMemory, 0,
		&Set->MemUsed);

	Set->FreeSentinel = VoxelAllocateThreadwork(
		InitMemory, 0,
		&Set->MemUsed);

	Set->FreeThreadworksCount = ThreadworksCount;
	Set->TotalThreadworksCount = ThreadworksCount;

	for (int NewWorkIndex = 0;
		NewWorkIndex < ThreadworksCount;
		NewWorkIndex++)
	{
		threadwork_data* NewThreadwork =
			VoxelAllocateThreadwork(
				InitMemory,
				OneThreadworkSize,
				&Set->MemUsed);

		InsertThreadworkAfter(NewThreadwork, Set->FreeSentinel);
	}
}