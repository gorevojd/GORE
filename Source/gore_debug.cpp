#include "gore_debug.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

inline void DEBUGDeallocateTreeNode(debug_tree_node* FreeTreeNode, debug_tree_node* Entry) {
	Entry->PrevBro->NextBro = Entry->NextBro;
	Entry->NextBro->PrevBro = Entry->PrevBro;

	Entry->NextBro = FreeTreeNode->NextBro;
	Entry->PrevBro = FreeTreeNode;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}


inline debug_tree_node* DEBUGAllocateTreeNode(
	debug_tree_node* FreeSentinel, 
	stacked_memory* Mem) 
{
	debug_tree_node* Result = 0;

	if (FreeSentinel == 0) {
		Result = PushStruct(Mem, debug_tree_node);
	}
	else {
		if (FreeSentinel->NextBro != FreeSentinel) {
			Result = FreeSentinel->NextBro;

			Result->PrevBro->NextBro = Result->NextBro;
			Result->NextBro->PrevBro = Result->PrevBro;
		}
		else {
			Result = PushStruct(Mem, debug_tree_node);
		}
	}

	Result->PrevBro = Result;
	Result->NextBro = Result;

	return(Result);
}

inline void DEBUGInsertToList(debug_tree_node* Sentinel, debug_tree_node* Entry) {
	Entry->NextBro = Sentinel->NextBro;
	Entry->PrevBro = Sentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline void DEBUGAllocateElementSentinelTreeNode(
	debug_tree_node* FreeSentinel, 
	stacked_memory* Mem,
	debug_tree_node* Element) 
{
	Element->ChildrenSentinel= DEBUGAllocateTreeNode(FreeSentinel, Mem);

	Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->Parent = Element;
	CopyStrings(Element->ChildrenSentinel->UniqueName, "Sentinel");
}

inline debug_statistic* DEBUGAllocateStatistic(stacked_memory* Mem) {
	debug_statistic* Result = 0;

	Result = PushStruct(Mem, debug_statistic);

	Result->PrevBro = Result;
	Result->NextBro = Result;

	return(Result);
}

#if 0
inline void DEBUGDeallocateStatistic(debug_state* State, debug_statistic* Statistic) {
	Statistic->NextBro->PrevBro = Statistic->PrevBro;
	Statistic->PrevBro->NextBro = Statistic->NextBro;

	Statistic->PrevBro = State->FreeStatisticSentinel;
	Statistic->NextBro = State->FreeStatisticSentinel->NextBro;

	Statistic->PrevBro->NextBro = Statistic;
	Statistic->NextBro->PrevBro = Statistic;

	Statistic->NextInHash = 0;
}
#endif

inline void DEBUGInsertStatisticAfter(debug_statistic* After, debug_statistic* ToInsert) {
	if (After != ToInsert) {
		ToInsert->PrevBro->NextBro = ToInsert->NextBro;
		ToInsert->NextBro->PrevBro = ToInsert->PrevBro;

		ToInsert->PrevBro = After;
		ToInsert->NextBro = After->NextBro;

		ToInsert->NextBro->PrevBro = ToInsert;
		ToInsert->PrevBro->NextBro = ToInsert;
	}
}

inline void DEBUGInsertStatisticBefore(debug_statistic* Before, debug_statistic* ToInsert) {
	if (Before != ToInsert) {
		ToInsert->PrevBro->NextBro = ToInsert->NextBro;
		ToInsert->NextBro->PrevBro = ToInsert->PrevBro;

		ToInsert->PrevBro = Before->PrevBro;
		ToInsert->NextBro = Before;

		ToInsert->NextBro->PrevBro = ToInsert;
		ToInsert->PrevBro->NextBro = ToInsert;
	}
}

#define DEBUG_NEW_BLOCK_TEMP_NAME_SZ 128
inline void DEBUGParseNameFromUnique(char* UniqueName, char* Out, int Size) {
	int CharIndex;
	
	for (CharIndex = 0; CharIndex < Size; CharIndex++) {
		char Ch = UniqueName[CharIndex];
		if (Ch != '@' && Ch != 0) {
			Out[CharIndex] = UniqueName[CharIndex];
		}
		else {
			Out[CharIndex] = 0;
		}
	}
}

static debug_tree_node* DEBUGInitLayerTreeNode(
	debug_tree_node* CurrentBlock, 
	debug_tree_node* FreeSentinel,
	stacked_memory* Mem,
	char* UniqueName, 
	u32 Type) 
{
	debug_tree_node* NewBlock = 0;

	u32 BlockHashID = StringHashFNV(UniqueName);

	debug_tree_node* At = CurrentBlock->ChildrenSentinel->PrevBro;
	for (At; At != CurrentBlock->ChildrenSentinel; At = At->PrevBro) {
#if 0
		if (StringsAreEqual(Record->UniqueName, At->Name)) {
#else
		if (BlockHashID == At->ID) {
#endif
			NewBlock = At;
			break;
		}
	}

	if (NewBlock == 0) {
		//NOTE(dima): Debug node not exist. Should be allocated.
		NewBlock = DEBUGAllocateTreeNode(FreeSentinel, Mem);

		*NewBlock = {};
		NewBlock->TreeNodeType = Type;
		NewBlock->Parent = CurrentBlock;

		char NewName[DEBUG_NEW_BLOCK_TEMP_NAME_SZ];
		DEBUGParseNameFromUnique(UniqueName, NewName, DEBUG_NEW_BLOCK_TEMP_NAME_SZ);

		CopyStrings(NewBlock->UniqueName, UniqueName);

		NewBlock->ID = StringHashFNV(UniqueName);

		if (Type == DebugTreeNode_Section ||
			Type == DebugTreeNode_Timing) 
		{
			DEBUGAllocateElementSentinelTreeNode(FreeSentinel, Mem, NewBlock);
		}

		DEBUGInsertToList(CurrentBlock->ChildrenSentinel, NewBlock);
	}

	return(NewBlock);
}

static debug_statistic* DEBUGInitDebugStatistic(
	stacked_memory* Mem,
	debug_statistic** StatisticArray,
	debug_statistic* StatisticSentinel, 
	char* UniqueName) 
{
	debug_statistic* Statistic = 0;

	//NOTE(dima): Finding needed element in the hash table
	u32 RecordHash = StringHashFNV(UniqueName);
	u32 StatisticIndex = RecordHash & (DEBUG_TIMING_STATISTICS_COUNT - 1);
	Statistic = StatisticArray[StatisticIndex];

	debug_statistic* PrevInHash = 0;

	while (Statistic) {
		if (Statistic->ID == RecordHash) {
			break;
		}

		PrevInHash = Statistic;
		Statistic = Statistic->NextInHash;
	}

	//NOTE(dima): If element was not found in hash table
	if (Statistic == 0) {
		Statistic = DEBUGAllocateStatistic(Mem);

		Statistic->NextBro = StatisticSentinel->NextBro;
		Statistic->PrevBro = StatisticSentinel;

		Statistic->NextBro->PrevBro = Statistic;
		Statistic->PrevBro->NextBro = Statistic;
		Statistic->NextInHash = 0;

		//NOTE(dima): Inserting to hash table
		if (PrevInHash) {
			PrevInHash->NextInHash = Statistic;
		}
		else {
			StatisticArray[StatisticIndex] = Statistic;
		}

		Statistic->ID = RecordHash;

		char NewName[DEBUG_NEW_BLOCK_TEMP_NAME_SZ];
		DEBUGParseNameFromUnique(UniqueName, NewName, DEBUG_NEW_BLOCK_TEMP_NAME_SZ);
		CopyStrings(Statistic->Name, NewName);

		Statistic->Timing = {};
	}

	return(Statistic);
}

static void DEBUGInitFrameInfo(debug_state* State, debug_profiled_frame* Frame) {
	FUNCTION_TIMING();

	//NOTE(dima): Initialization of timing root
	Frame->TimingRoot = DEBUGAllocateTreeNode(0, &Frame->FrameMemory);
	DEBUGAllocateElementSentinelTreeNode(
		0,
		&Frame->FrameMemory,
		Frame->TimingRoot);
	Frame->CurrentTiming = Frame->TimingRoot;

	//NOTE(dima): Setting frame update node to zero
	Frame->FrameUpdateNode = 0;

	Frame->TimingStatisticSentinel = DEBUGAllocateStatistic(&Frame->FrameMemory);

	Frame->CurrentSection = State->RootSection;
}

static void DEBUGFreeFrameInfo(debug_state* State, debug_profiled_frame* Frame) {
	FUNCTION_TIMING();

	Frame->FrameMemory.Used = 0;

	Frame->DeltaTime = 0.0f;

#if 0
	debug_statistic* TimingAt = Frame->TimingStatisticSentinel->NextBro;
	while (TimingAt != Frame->TimingStatisticSentinel) {
		debug_statistic* TempNext = TimingAt->NextBro;
		
		DEBUGDeallocateStatistic(State, TimingAt);

		TimingAt = TempNext;
	}
#endif

	for (int TimingStatisticIndex = 0;
		TimingStatisticIndex < DEBUG_TIMING_STATISTICS_COUNT;
		TimingStatisticIndex++)
	{
		Frame->TimingStatistics[TimingStatisticIndex] = 0;
	}
}

void DEBUGInit(debug_state* State, stacked_memory* DEBUGMemoryBlock, gui_state* GUIState) {
	State->DebugMemory = DEBUGMemoryBlock;

	State->GUIState = GUIState;
	
	State->FreeBlockSentinel = PushStruct(State->DebugMemory, debug_tree_node);
	State->FreeBlockSentinel->NextBro = State->FreeBlockSentinel;
	State->FreeBlockSentinel->PrevBro = State->FreeBlockSentinel;
	CopyStrings(State->FreeBlockSentinel->UniqueName, "FreeBlockSentinel");

	State->CollationFrameIndex = 0;
	State->ViewFrameIndex = 0;
	State->NewestFrameIndex = 0;
	State->OldestFrameIndex = 0;

	State->RootSection = DEBUGAllocateTreeNode(State->FreeBlockSentinel, State->DebugMemory);

	DEBUGAllocateElementSentinelTreeNode(State->FreeBlockSentinel, State->DebugMemory, State->RootSection);

	CopyStrings(State->RootSection->UniqueName, "ROOT");
	State->RootSection->ID = StringHashFNV(State->RootSection->UniqueName);
	State->RootSection->Parent = 0;
	State->RootSection->TreeNodeType = DebugTreeNode_Section;
	State->CurrentSection = State->RootSection;

	for (int FrameIndex = 0;
		FrameIndex < DEBUG_FRAMES_COUNT;
		FrameIndex++)
	{
		debug_profiled_frame* Frame = &State->Frames[FrameIndex];
	
		Frame->FrameMemory = SplitStackedMemory(State->DebugMemory, KILOBYTES(500));
	}

	for (int FrameIndex = 0;
		FrameIndex < DEBUG_FRAMES_COUNT;
		FrameIndex++)
	{
		debug_profiled_frame* Frame = &State->Frames[FrameIndex];

		DEBUGFreeFrameInfo(State, Frame);
		DEBUGInitFrameInfo(State, Frame);
	}

	State->FramesGraphBarType = DEBUGFrameGraph_DeltaTime;
	State->RootNodeBarType = DEBUGFrameGraph_RootNodeBlocks;

	State->LastCollationFrameRecords = 0;
	State->IsRecording = 1;
	
	//NOTE(dima): Initialization of logs
	State->DebugLogs = PushArray(State->DebugMemory, char*, DEBUG_LOGS_COUNT);
	State->DebugLogsTypes = PushArray(State->DebugMemory, u32, DEBUG_LOGS_COUNT);
	State->DebugLogsInited = PushArray(State->DebugMemory, b32, DEBUG_LOGS_COUNT);
	for (int DebugLogIndex = 0;
		DebugLogIndex < DEBUG_LOGS_COUNT;
		DebugLogIndex++)
	{
		State->DebugLogs[DebugLogIndex] = (char*)PushSomeMemory(State->DebugMemory, DEBUG_LOG_SIZE);
		State->DebugLogsTypes[DebugLogIndex] = 0;
		State->DebugLogsInited[DebugLogIndex] = 0;
	}

#if DEBUG_NORMALIZE_FRAME_GRAPH
	State->SegmentFrameCount = 0;
	State->NotFirstSegment = 0;

	State->MaxSegmentCollectedRecords = 0;
	State->MaxSegmentDT = 0.0f;
	State->MaxSegmentFPS = 0.0f;
#endif
}

inline debug_profiled_frame* DEBUGGetCollationFrame(debug_state* State) {
	debug_profiled_frame* Frame = &State->Frames[State->CollationFrameIndex];

	return(Frame);
}

inline debug_profiled_frame* DEBUGGetFrameByIndex(debug_state* State, u32 Index) {
	debug_profiled_frame* Frame = &State->Frames[Index % DEBUG_FRAMES_COUNT];

	return(Frame);
}


inline debug_profiled_frame* DEBUGGetNewestFrame(debug_state* State) {
	debug_profiled_frame* Result = 0;

	int TargetIndex = (int)State->CollationFrameIndex - 1;
	if (TargetIndex < 0) {
		TargetIndex += DEBUG_FRAMES_COUNT;
	}

	Result = &State->Frames[TargetIndex % DEBUG_FRAMES_COUNT];

	return(Result);
}

inline u32 DEBUGIncrementFrameIndex(u32 Index) {
	u32 Result = (Index + GlobalRecordTable->Increment.value) % DEBUG_FRAMES_COUNT;

	return(Result);
}

inline void DEBUGIncrementFrameIndices(debug_state* State) {
	State->NewestFrameIndex = State->CollationFrameIndex;
	if (State->ViewFrameIndex != State->CollationFrameIndex) {
		State->ViewFrameIndex = DEBUGIncrementFrameIndex(State->ViewFrameIndex);
	}
	State->LastCollationFrameIndex = State->CollationFrameIndex;
	State->CollationFrameIndex = DEBUGIncrementFrameIndex(State->CollationFrameIndex);

	if (State->CollationFrameIndex == State->OldestFrameIndex) {
		State->OldeshShouldBeIncremented = 1;
	}
	
	if (State->OldeshShouldBeIncremented) {
		State->OldestFrameIndex = DEBUGIncrementFrameIndex(State->OldestFrameIndex);
	}
}

static void DEBUGProcessRecords(debug_state* State) {
	FUNCTION_TIMING();

	State->LastCollationFrameIndex = State->CollationFrameIndex;

	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;

	u32 FrameBarrierIndex = 0;
	u32 CollectedRecordIndex;
	for (CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		debug_record* Record = &GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value][CollectedRecordIndex];

		debug_profiled_frame* Frame = DEBUGGetCollationFrame(State);

		switch (Record->RecordType) {
			case DebugRecord_BeginTiming: {
				debug_tree_node* CurrentBlock = Frame->CurrentTiming;

				debug_tree_node* TimingNode = DEBUGInitLayerTreeNode(
					CurrentBlock,
					0,
					&Frame->FrameMemory,
					Record->UniqueName, 
					DebugTreeNode_Timing);
				debug_timing_snapshot* TimingSnapshot = &TimingNode->TimingSnapshot;

				TimingSnapshot->BeginClock = Record->Clocks;
				TimingSnapshot->ThreadID = Record->ThreadID;
				TimingSnapshot->ChildrenSumClocks = 0;
				TimingSnapshot->HitCount = 0;
				TimingSnapshot->ClocksElapsed = 0;

				Frame->CurrentTiming = TimingNode;
			}break;

			case DebugRecord_EndTiming: {

				debug_tree_node* CurrentBlock = Frame->CurrentTiming;
				Assert(CurrentBlock->TreeNodeType == DebugTreeNode_Timing);
				debug_timing_snapshot* TimingSnapshot = &CurrentBlock->TimingSnapshot;

				//NOTE(dima): difference between start and end clocks
				TimingSnapshot->ClocksElapsed = Record->Clocks - TimingSnapshot->BeginClock;

				//TODO(dima): think about not going through children.. Instead of it
				//TODO(dima): I might just change parent childrenSumCLocks

				//NOTE(dima): Going througn children and summing all their total clocks
				TimingSnapshot->ChildrenSumClocks = 0;
				debug_tree_node* At = CurrentBlock->ChildrenSentinel->NextBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->NextBro) {
					TimingSnapshot->ChildrenSumClocks += At->TimingSnapshot.ClocksElapsed;
				}

				//NOTE(dima): Finding needed element in the hash table
				debug_statistic* TimingStatistic = DEBUGInitDebugStatistic(
					&Frame->FrameMemory, 
					Frame->TimingStatistics, 
					Frame->TimingStatisticSentinel, 
					CurrentBlock->UniqueName);

				TimingStatistic->Type = DebugStatistic_Timing;
				TimingStatistic->Timing.TotalClocks += TimingSnapshot->ClocksElapsed;
				TimingStatistic->Timing.HitCount += 1;
				TimingStatistic->Timing.TotalClocksInChildren = TimingSnapshot->ChildrenSumClocks;

				Frame->CurrentTiming = CurrentBlock->Parent;
			}break;

			case DebugRecord_BeginSection: {
				debug_tree_node* CurrentSection = Frame->CurrentSection;

				debug_tree_node* NewSection = DEBUGInitLayerTreeNode(
					State->RootSection, 
					State->FreeBlockSentinel, 
					State->DebugMemory, 
					Record->UniqueName, 
					DebugTreeNode_Section);

				//NOTE(dima): Remembering last section
				NewSection->Parent = CurrentSection;

				//NOTE(dima): Setting frame's current section to newly pushed block
				Frame->CurrentSection = NewSection;
			}break;

			case DebugRecord_EndSection: {
				debug_tree_node* CurrentSection = Frame->CurrentSection;
				Assert(CurrentSection->TreeNodeType == DebugTreeNode_Section);

				//NOTE(dima): Setting frame's current section to this block's parent
				Frame->CurrentSection = CurrentSection->Parent;
			}break;

			case DebugRecord_FrameBarrier: {
				float DeltaTime = Record->Value_Value.Value_DebugValue_F32;

				FrameBarrierIndex = CollectedRecordIndex;

				debug_profiled_frame* CollationFrame = DEBUGGetCollationFrame(State);
				CollationFrame->DeltaTime = DeltaTime;
				CollationFrame->RecordCount = State->LastCollationFrameRecords + FrameBarrierIndex;

				//NOTE(dima): Setting frame update node if founded
				debug_tree_node* FrameUpdateNode = 0;
				for (debug_tree_node* At = Frame->TimingRoot->ChildrenSentinel->NextBro;
					At != Frame->TimingRoot->ChildrenSentinel;
					At = At->NextBro)
				{
					char NodeName[256];
					DEBUGParseNameFromUnique(At->UniqueName, NodeName, sizeof(NodeName));

					if (StringsAreEqual(NodeName, DEBUG_FRAME_UPDATE_NODE_NAME)) {
						FrameUpdateNode = At;
						break;
					}
				}
				CollationFrame->FrameUpdateNode = FrameUpdateNode;

				//NOTE(dima): Incrementing frame indices;
				DEBUGIncrementFrameIndices(State);

				//NOTE(dima): Freeing the frame
				debug_profiled_frame* NewFrame = DEBUGGetCollationFrame(State);
				DEBUGFreeFrameInfo(State, NewFrame);
				DEBUGInitFrameInfo(State, NewFrame);

				//NOTE(dima): Section pointer must be equal to initial
				Assert(State->CurrentSection == State->RootSection);
			} break;

			case DebugRecord_Value: {
				debug_tree_node* CurrentSection = Frame->CurrentSection;

				debug_tree_node* NewSection = DEBUGInitLayerTreeNode(
					CurrentSection,
					State->FreeBlockSentinel,
					State->DebugMemory,
					Record->UniqueName,
					DebugTreeNode_Value);

				NewSection->Value.ValueType = Record->Value_Value.ValueType;

				//NOTE(dima): Remembering last section
				NewSection->Parent = CurrentSection;
			}break;

			case DebugRecord_Log: {
				u32 LogIndex = State->DebugWriteLogIndex;

				stbsp_snprintf(State->DebugLogs[LogIndex], DEBUG_LOG_SIZE, "/> %s", Record->Value_Value.Value_DebugValue_Text);
				State->DebugLogsTypes[LogIndex] = DEBUGGetLogTypeFromValueType(Record->Value_Value.ValueType);
				State->DebugLogsInited[LogIndex] = 1;

				State->DebugWriteLogIndex = (State->DebugWriteLogIndex + 1) % DEBUG_LOGS_COUNT;
			}break;
		}
	}

	//BUG(dima): This should happen in other place.
	//NOTE(dima): If recording was set to 0, then we should stop new frame event recording
	if (State->RecordingChanged) {
		DEBUGSetRecording(State->IsRecording);
		State->RecordingChanged = 0;
	}

	//TODO(dima): Think about when to do this
	int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
	SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
	SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);

	State->LastCollationFrameRecords = CollectedRecordIndex - FrameBarrierIndex;
}

inline void DEBUGPushFrameColumn(gui_state* GUIState, u32 FrameIndex, v2 InitP, v2 Dim, v4 Color) {
	rect2 ColumnRect = Rect2MinDim(V2(InitP.x + Dim.x * FrameIndex, InitP.y), Dim);

	RENDERPushRect(GUIState->RenderStack, ColumnRect, Color);
}

static void DEBUGFramesSlider(debug_state* State) {
	gui_state* GUIState = State->GUIState;
	
	//NOTE(dima): This is jush for safe
	b32 PrevRecording = State->IsRecording;

	GUIBeginRow(GUIState);
	GUIBoolButton2(GUIState, "Recording", &State->IsRecording);
	if (GUIButton(GUIState, "Newest")) {
		State->ViewFrameIndex = State->NewestFrameIndex;
	}

	if (GUIButton(GUIState, "Oldest")) {
		State->ViewFrameIndex = State->CollationFrameIndex + 1;
	}
	GUIEndRow(GUIState);

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "FrameSlider", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		if (!Element->Cache.IsInitialized) {
			Element->Cache.Dimensional.Dimension = V2((float)GUIState->ScreenWidth * 0.8f, AscByScale * 3);

			Element->Cache.IsInitialized = 1;
		}

		v2* GraphDim = &Element->Cache.Dimensional.Dimension;

		GUIPreAdvanceCursor(GUIState);

		v4 OutlineColor = GUIGetColor(GUIState, GUIColor_Black);

		v2 GraphMin = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);

		float OneColumnWidth = GraphDim->x / (float)DEBUG_FRAMES_COUNT;

		v2 ColumnDim = V2(OneColumnWidth, GraphDim->y);
		rect2 ColumnRect = Rect2MinDim(GraphMin, ColumnDim);

		RENDERPushRect(GUIState->RenderStack, GraphMin, *GraphDim, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor));

		//NOTE(dima): Collation frame column
		DEBUGPushFrameColumn(GUIState, State->CollationFrameIndex, GraphMin, ColumnDim, GUIGetColor(GUIState, GUIColor_Green));

		//NOTE(dima): Newest frame column
		DEBUGPushFrameColumn(GUIState, State->NewestFrameIndex, GraphMin, ColumnDim, GUIGetColor(GUIState, GUIColor_Red));

		//NOTE(dima): Oldest frame column
		DEBUGPushFrameColumn(GUIState, State->OldestFrameIndex, GraphMin, ColumnDim, GUIGetColor(GUIState, GUIColor_Blue));

		for (int ColumnIndex = 0;
			ColumnIndex < DEBUG_FRAMES_COUNT;
			ColumnIndex++)
		{
			if (MouseInRect(GUIState->Input, ColumnRect)) {
				char LabelBuf[256];
				stbsp_sprintf(LabelBuf, "Frame: %u", ColumnIndex);
				GUITooltip(GUIState, LabelBuf);

				if (MouseButtonIsDown(GUIState->Input, MouseButton_Left) &&
					ColumnIndex != State->CollationFrameIndex) 
				{
					State->ViewFrameIndex = ColumnIndex;
				}
			}

			ColumnRect.Min.x += OneColumnWidth;
			ColumnRect.Max.x += OneColumnWidth;
		}

		//NOTE(dima): Viewing frame column
		DEBUGPushFrameColumn(GUIState, State->ViewFrameIndex, GraphMin, ColumnDim, GUIGetColor(GUIState, GUIColor_Yellow));

		v2 BarDim = V2(1.0f, GraphDim->y);

		rect2 BarRect = Rect2MinDim(GraphMin + V2(OneColumnWidth, 0.0f), V2(1.0f, GraphDim->y));
		for (int BarIndex = 0;
			BarIndex < DEBUG_FRAMES_COUNT - 1;
			BarIndex++)
		{
			RENDERPushRect(GUIState->RenderStack, BarRect, OutlineColor);

			BarRect.Min.x += OneColumnWidth;
			BarRect.Max.x += OneColumnWidth;
		}

		rect2 GraphRect = Rect2MinDim(GraphMin, *GraphDim);
		RENDERPushRectOutline(GUIState->RenderStack, GraphRect, 3, GUIGetColor(GUIState, GUIColor_Black));

#if 1
		gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, GraphDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(10, 10), &ResizeInteraction);
#endif

		GUIDescribeElement(GUIState, *GraphDim, GraphMin);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);

	State->RecordingChanged = (PrevRecording == State->IsRecording);
}

static rect2 DEBUGFramesGraph(debug_state* State, u32 Type, debug_tree_node* ViewNode = 0) {
	FUNCTION_TIMING();

	rect2 Result = {};

	gui_state* GUIState = State->GUIState;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "ProfileFrameGraph", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		gui_element_cache* Cache = &Element->Cache;
		if (!Cache->IsInitialized) {

			Cache->Dimensional.Dimension = V2((float)GUIState->ScreenWidth * 0.8f, (float)GUIState->ScreenHeight * 0.15f);

			Cache->IsInitialized = 1;
		}


		v4 OutlineColor = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);

		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		v2 GraphMin = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);
		v2* GraphDim = &Cache->Dimensional.Dimension;

		float OneColumnWidth = GraphDim->x / (float)DEBUG_FRAMES_COUNT;

		v2 ColumnDim = V2(OneColumnWidth, GraphDim->y);
		rect2 ColumnRect = Rect2MinDim(GraphMin, ColumnDim);

		switch (Type) {

			case DEBUGFrameGraph_DeltaTime: {
				float OneOverMaxMs = 30.0f;
				float NormalizeValue = OneOverMaxMs; 

#if DEBUG_NORMALIZE_FRAME_GRAPH
				if (State->NotFirstSegment) {
					NormalizeValue = 0.5f / State->MaxLastSegmentDT;
				}
#endif

				for (int ColumnIndex = 0;
					ColumnIndex < DEBUG_FRAMES_COUNT;
					ColumnIndex++)
				{
					debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, ColumnIndex);

					if (ColumnIndex == State->CollationFrameIndex) {
						RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, GUIColorExt_green1));
					}
					else {
						float FilledPercentage = Frame->DeltaTime * NormalizeValue;
						FilledPercentage = Clamp01(FilledPercentage);

						rect2 FilledRect = Rect2MinMax(V2(ColumnRect.Min.x, ColumnRect.Max.y - ColumnDim.y * FilledPercentage), ColumnRect.Max);
						rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, ColumnDim.y * (1.0f - FilledPercentage)));

						v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);
						
						if (ColumnIndex == State->ViewFrameIndex) {
							ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
						}

						//RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIColorExt_green3) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor2) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor) * ColorMultiplier);

#if DEBUG_SHOW_FRAME_GRAPH_TOOLTIPS
						if (MouseInRect(GUIState->Input, ColumnRect)) {
							char TooltipBuf[256];
							stbsp_sprintf(TooltipBuf, "dt: %.2fms", 1000.0f * Frame->DeltaTime);
							GUITooltip(GUIState, TooltipBuf);
						}
#endif
					}

					ColumnRect.Min.x += OneColumnWidth;
					ColumnRect.Max.x += OneColumnWidth;
				}
			}break;

			case DEBUGFrameGraph_FPS: {
				float OneOverMaxShowFPS = 1.0f / 300.0f;
				float NormalizeValue = OneOverMaxShowFPS;

#if DEBUG_NORMALIZE_FRAME_GRAPH
				if (State->NotFirstSegment) {
					NormalizeValue = 0.5f / State->MaxLastSegmentFPS;
				}
#endif

				for (int ColumnIndex = 0;
					ColumnIndex < DEBUG_FRAMES_COUNT;
					ColumnIndex++)
				{
					debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, ColumnIndex);
					float CurrentFPS = 1.0f / Frame->DeltaTime;

					if (ColumnIndex == State->CollationFrameIndex) {
						RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, GUIColorExt_green1));
					}
					else {
						float FilledPercentage = CurrentFPS * NormalizeValue;
						FilledPercentage = Clamp01(FilledPercentage);

						rect2 FilledRect = Rect2MinMax(V2(ColumnRect.Min.x, ColumnRect.Max.y - ColumnDim.y * FilledPercentage), ColumnRect.Max);
						rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, ColumnDim.y * (1.0f - FilledPercentage)));

						v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);

						if (ColumnIndex == State->ViewFrameIndex) {
							ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
						}

						RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor3) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor) * ColorMultiplier);

#if DEBUG_SHOW_FRAME_GRAPH_TOOLTIPS
						if (MouseInRect(GUIState->Input, ColumnRect)) {
							char TooltipBuf[256];
							stbsp_sprintf(TooltipBuf, "FPS: %.2f", CurrentFPS);
							GUITooltip(GUIState, TooltipBuf);
						}
#endif
					}

					ColumnRect.Min.x += OneColumnWidth;
					ColumnRect.Max.x += OneColumnWidth;
				}
			}break;

			case DEBUGFrameGraph_RootNodeBlocks: {
				u32 GraphColors[8];
				GraphColors[0] = GUIState->ColorTheme.GraphColor1;
				GraphColors[1] = GUIState->ColorTheme.GraphColor2;
				GraphColors[2] = GUIState->ColorTheme.GraphColor3;
				GraphColors[3] = GUIState->ColorTheme.GraphColor4;
				GraphColors[4] = GUIState->ColorTheme.GraphColor5;
				GraphColors[5] = GUIState->ColorTheme.GraphColor6;
				GraphColors[6] = GUIState->ColorTheme.GraphColor7;
				GraphColors[7] = GUIState->ColorTheme.GraphColor8;

				for (int ColumnIndex = 0;
					ColumnIndex < DEBUG_FRAMES_COUNT;
					ColumnIndex++)
				{
					debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, ColumnIndex);

					if (ColumnIndex == State->CollationFrameIndex) {
						RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, GUIColorExt_green1));
					}
					else {
						debug_tree_node* FrameUpdateNode = Frame->FrameUpdateNode;
						
						if (FrameUpdateNode) {
							float FilledPercentage = 1.0f;
							FilledPercentage = Clamp01(FilledPercentage);

							float OneOverFrameClocks = 1.0f / (float)FrameUpdateNode->TimingSnapshot.ClocksElapsed;

							v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);

							if (ColumnIndex == State->ViewFrameIndex) {
								ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
							}

							float ColY = ColumnRect.Max.y;

							int ColorIndex = 0;
							for (debug_tree_node* ChildrenAt = FrameUpdateNode->ChildrenSentinel->PrevBro;
								ChildrenAt != FrameUpdateNode->ChildrenSentinel;
								ChildrenAt = ChildrenAt->PrevBro)
							{
								float CurrentPercentage = (float)ChildrenAt->TimingSnapshot.ClocksElapsed * OneOverFrameClocks;

								float CurColY = ColumnDim.y * CurrentPercentage;
								ColY -= CurColY;
								
								rect2 CurrentRect = Rect2MinDim(V2(ColumnRect.Min.x, ColY), V2(ColumnDim.x, CurColY));
								v4 CurColColor = GUIGetColor(GUIState, GraphColors[ColorIndex++]);

								RENDERPushRect(GUIState->RenderStack, CurrentRect, CurColColor * ColorMultiplier);

								if (MouseInRect(GUIState->Input, CurrentRect)) {
									GUITooltip(GUIState, ChildrenAt->UniqueName);
								}
							}

							float FreePercentage = (ColY - ColumnRect.Min.y) / ColumnDim.y;;
							rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, (ColY - ColumnRect.Min.y)));

							if (MouseInRect(GUIState->Input, FreeRect)) {
								GUITooltip(GUIState, "Waiting...");
							}

							RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIColor_White) * ColorMultiplier);
						}
					}

					ColumnRect.Min.x += OneColumnWidth;
					ColumnRect.Max.x += OneColumnWidth;
				}
			}break;

			case DEBUGFrameGraph_CollectedRecords: {
				float OneOverMaxRecs = 1.0f / (float)DEBUG_RECORD_MAX_COUNT;
				float NormalizeValue = OneOverMaxRecs;

#if DEBUG_NORMALIZE_FRAME_GRAPH
				if (State->NotFirstSegment) {
					NormalizeValue = 0.5f / ((float)State->MaxLastSegmentCollectedRecords);
				}
#endif

				for (int ColumnIndex = 0;
					ColumnIndex < DEBUG_FRAMES_COUNT;
					ColumnIndex++)
				{
					debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, ColumnIndex);

					if (ColumnIndex == State->CollationFrameIndex) {
						RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, GUIColorExt_green1));
					}
					else {
						float FilledPercentage = (float)Frame->RecordCount * (float)NormalizeValue;
						FilledPercentage = Clamp01(FilledPercentage);

						rect2 FilledRect = Rect2MinMax(V2(ColumnRect.Min.x, ColumnRect.Max.y - ColumnDim.y * FilledPercentage), ColumnRect.Max);
						rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, ColumnDim.y * (1.0f - FilledPercentage)));

						v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);

						if (ColumnIndex == State->ViewFrameIndex) {
							ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
						}

						RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor3) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor) * ColorMultiplier);

#if DEBUG_SHOW_FRAME_GRAPH_TOOLTIPS
						if (MouseInRect(GUIState->Input, ColumnRect)) {
							char TooltipBuf[256];
							stbsp_sprintf(TooltipBuf, "Records: %u", Frame->RecordCount);
							GUITooltip(GUIState, TooltipBuf);
						}
#endif
					}

					ColumnRect.Min.x += OneColumnWidth;
					ColumnRect.Max.x += OneColumnWidth;
				}
			}break;
		}

		v2 BarDim = V2(1.0f, GraphDim->y);

		rect2 BarRect = Rect2MinDim(GraphMin + V2(OneColumnWidth, 0.0f), V2(1.0f, GraphDim->y));
		for (int BarIndex = 0;
			BarIndex < DEBUG_FRAMES_COUNT - 1;
			BarIndex++)
		{
			RENDERPushRect(GUIState->RenderStack, BarRect, OutlineColor);
		
			BarRect.Min.x += OneColumnWidth;
			BarRect.Max.x += OneColumnWidth;
		
		}

		rect2 GraphRect = Rect2MinDim(GraphMin, *GraphDim);
		Result = GraphRect;
		RENDERPushRectOutline(GUIState->RenderStack, GraphRect, 3, OutlineColor);

#if 1
		gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, GraphDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(10, 10), &ResizeInteraction);
#endif

#if 0
		v4 LabelTextColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor);
		v4 LabelHighColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor);
		gui_interaction NullInteraction = GUINullInteraction();
		GUITextBase(GUIState, "delta time", V2(GraphMin.x, GraphMin.y + AscByScale),
			LabelTextColor,
			GUIState->FontScale,
			&NullInteraction,
			LabelHighColor,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1, OutlineColor);
#else

#endif

		GUIDescribeElement(GUIState, *GraphDim, GraphMin);
		GUIAdvanceCursor(GUIState, AscByScale * 0.5f);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);

	return(Result);
}

static void DEBUGViewingFrameInfo(debug_state* State) {
	char Buf[256];

	debug_profiled_frame* ViewingFrame = DEBUGGetFrameByIndex(State, State->ViewFrameIndex);

	stbsp_sprintf(Buf, "Viewing frame: %.2fms, %.2fFPS %urs", ViewingFrame->DeltaTime * 1000.0f, 1.0f / ViewingFrame->DeltaTime, ViewingFrame->RecordCount);
	GUIText(State->GUIState, Buf);
	GUIStackedMemGraph(State->GUIState, "FrameMemory", &ViewingFrame->FrameMemory);
}

enum debug_clock_list_type {
	DebugClockList_Total,
	DebugClockList_Exclusive,
};

inline u64 DEBUGGetClocksFromTiming(debug_statistic* Stat, u32 Type) {
	u64 Result = 0;

	if (Type == DebugClockList_Total) {
		Result = Stat->Timing.TotalClocks;
	}
	else if (Type == DebugClockList_Exclusive) {
		Result = Stat->Timing.TotalClocks - Stat->Timing.TotalClocksInChildren;
	}
	else {
		Assert(!"Invalid type");
	}

	return(Result);
}

static void DEBUGClocksList(debug_state* State, u32 Type) {
	gui_state* GUIState = State->GUIState;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "ClockList", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {

		debug_profiled_frame* Frame = &State->Frames[State->ViewFrameIndex];

		GUIPreAdvanceCursor(GUIState);

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);
		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float RowAdvance = GetNextRowAdvance(GUIState->FontInfo);
		
		gui_element_cache* Cache = &Element->Cache;
		if (!Cache->IsInitialized) {

			Cache->Dimensional.Dimension = V2(AscByScale * 60, AscByScale * 20);

			Cache->IsInitialized = 1;
		}

		v2 GroundMin = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);
		v2* GroundDim = &Cache->Dimensional.Dimension;

		rect2 GroundRc = Rect2MinDim(GroundMin, *GroundDim);
		v4 GroundC = GUIGetColor(GUIState, GUIColor_Black);
		GroundC = V4(GroundC.xyz, 0.8f);

		RENDERPushRect(GUIState->RenderStack, GroundRc, GroundC);

		float AtY = Layout->CurrentY;

		BEGIN_TIMING("ClockListSorting");
#if 1
		//NOTE(dima): Selection sort implemented
		debug_statistic* SortAt = Frame->TimingStatisticSentinel->NextBro;
		for (SortAt;
			SortAt != Frame->TimingStatisticSentinel->PrevBro;)
		{
			debug_statistic* Biggest = SortAt;

			for (debug_statistic* ScanAt = SortAt->NextBro;
				ScanAt != Frame->TimingStatisticSentinel;
				ScanAt = ScanAt->NextBro)
			{
				if (DEBUGGetClocksFromTiming(ScanAt, Type) >
					DEBUGGetClocksFromTiming(Biggest, Type)) 
				{
					Biggest = ScanAt;
				}
			}

			if (Biggest != SortAt) {
				DEBUGInsertStatisticBefore(SortAt, Biggest);
			}
			else {
				SortAt = SortAt->NextBro;
			}
		}
#endif
		END_TIMING();

		v4 TextColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		v4 TextHighColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);

		float OneOverFrameClocks = 100.0f / (float)Frame->FrameUpdateNode->TimingSnapshot.ClocksElapsed;

		debug_statistic* Timing = Frame->TimingStatisticSentinel->NextBro;
		for (
			Timing; 
			Timing != Frame->TimingStatisticSentinel; 
			Timing = Timing->NextBro) 
		{
			if (AtY < GroundRc.Max.y + GUIState->FontInfo->DescenderHeight * GUIState->FontScale) {
				char TextBuf[256];
		
				u64 ToViewClocks = DEBUGGetClocksFromTiming(Timing, Type);
		
				float CoveragePercentage = (float)ToViewClocks * OneOverFrameClocks;

#if 0
				stbsp_sprintf(TextBuf, "%11lluc %13.2fc/h  %6.2f%% %8u  %-30s",
					ToViewClocks,
					(float)ToViewClocks / (float)Timing->Timing.HitCount,
					CoveragePercentage,
					Timing->Timing.HitCount,
					Timing->Name);
#else
				stbsp_sprintf(TextBuf, "%11lluc %8.2f%% %8u  %-30s",
					ToViewClocks,
					CoveragePercentage,
					Timing->Timing.HitCount,
					Timing->Name);
#endif

				gui_interaction NullInteraction = GUINullInteraction();

				GUITextBase(GUIState, TextBuf, V2(Layout->CurrentX, AtY), TextColor, 
					GUIState->FontScale, &NullInteraction, TextHighColor, V4(0.0f, 0.0f, 0.0f, 0.0f), 0);
		
				AtY += RowAdvance;
			}
			else {
				break;
			}
		}

		RENDERPushRectOutline(GUIState->RenderStack, GroundRc, 3, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		gui_interaction ResizeInteraction = GUIResizeInteraction(GroundMin, GroundDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GroundRc.Max, V2(10, 10), &ResizeInteraction);

		GUIDescribeElement(GUIState, *GroundDim, GroundMin);
		GUIAdvanceCursor(GUIState, 0.5f * RowAdvance);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

static void DEBUGDrawConsole(debug_state* State) {

}

static void DEBUGLogger(debug_state* State) {
	gui_state* GUIState = State->GUIState;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, "Logr", 0, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		GUIPreAdvanceCursor(GUIState);

		float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float RowAdvance = GetNextRowAdvance(GUIState->FontInfo);
		float DescPlusGap = RowAdvance - AscByScale;

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		gui_element_cache* Cache = &Element->Cache;
		if (!Cache->IsInitialized) {

			Cache->Dimensional.Dimension = V2(650, AscByScale * 20);

			Cache->IsInitialized = 1;
		}

		v2* WorkDim = &Cache->Dimensional.Dimension;
		v2 WorkP = V2(Layout->CurrentX, Layout->CurrentY - AscByScale);
		rect2 WorkRect = Rect2MinDim(WorkP, *WorkDim);

		v4 WindowColor = GUIGetColor(GUIState, GUIColor_Black);
		WindowColor = V4(WindowColor.rgb, 0.90f);
		v4 WindowOutlineColor = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);

		RENDERPushRect(GUIState->RenderStack, WorkRect, WindowColor);
		RENDERPushRectOutline(GUIState->RenderStack, WorkRect, 3, WindowOutlineColor);

		v4 LogCol = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		v4 WarnLogCol = GUIGetColor(GUIState, GUIState->ColorTheme.WarningLogColor);
		v4 OkLogCol = GUIGetColor(GUIState, GUIState->ColorTheme.OkLogColor);
		v4 ErrLogCol = GUIGetColor(GUIState, GUIState->ColorTheme.ErrLogColor);

		float PrintY = WorkRect.Max.y - (RowAdvance - AscByScale) * GUIState->FontScale;

		int CurrentLogQueueIndex = State->DebugWriteLogIndex - 1;
		if (CurrentLogQueueIndex < 0) {
			CurrentLogQueueIndex = DEBUG_LOGS_COUNT - 1;
		}

		while (PrintY >= WorkRect.Min.y - DescPlusGap) {

			b32 LogInited = State->DebugLogsInited[CurrentLogQueueIndex];

			if (LogInited) {

				v4 LogResColor = LogCol;
				u32 LogType = State->DebugLogsTypes[CurrentLogQueueIndex];
				if (LogType == DebugLog_ErrLog) {
					LogResColor = ErrLogCol;
				}
				else if (LogType == DebugLog_Log) {
					LogResColor = LogCol;
				}
				else if (LogType == DebugLog_WarnLog) {
					LogResColor = WarnLogCol;
				}
				else if (LogType == DebugLog_OkLog) {
					LogResColor = OkLogCol;
				}

				gui_interaction NullInteraction = GUINullInteraction();
				GUITextBase(
					GUIState,
					State->DebugLogs[CurrentLogQueueIndex],
					V2(Layout->CurrentX, PrintY),
					LogResColor,
					GUIState->FontScale,
					&NullInteraction,
					LogResColor,
					V4(0.0f, 0.0f, 0.0f, 0.0f),
					0);

				PrintY -= RowAdvance;

				CurrentLogQueueIndex = CurrentLogQueueIndex - 1;
				if (CurrentLogQueueIndex < 0) {
					CurrentLogQueueIndex = DEBUG_LOGS_COUNT - 1;
				}
				
				if (CurrentLogQueueIndex == State->DebugWriteLogIndex) {
					break;
				}
			}
			else {
				break;
			}
		}

		gui_interaction ResizeInteraction = GUIResizeInteraction(WorkP, WorkDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", WorkRect.Max, V2(10, 10), &ResizeInteraction);

		GUIDescribeElement(GUIState, *WorkDim, WorkP);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

enum debug_profile_active_element {
	DebugProfileActiveElement_TopClocks,
	DebugProfileActiveElement_TopExClocks,
	DebugProfileActiveElement_Threads,
	DebugProfileActiveElement_RootNodeBlocks,
	DebugProfileActiveElement_FrameGraph,
};

static void DEBUGOutputSectionChildrenToGUI(debug_state* State, debug_tree_node* TreeNode) {
	debug_tree_node* At = TreeNode->ChildrenSentinel->PrevBro;

	for (At; At != TreeNode->ChildrenSentinel; At = At->PrevBro) {
		switch (At->TreeNodeType) {
			case DebugTreeNode_Section: {
				char NodeName[DEBUG_NEW_BLOCK_TEMP_NAME_SZ];
				DEBUGParseNameFromUnique(At->UniqueName, NodeName, DEBUG_NEW_BLOCK_TEMP_NAME_SZ);

				GUITreeBegin(State->GUIState, NodeName);
				DEBUGOutputSectionChildrenToGUI(State, At);
				GUITreeEnd(State->GUIState);
			}break;

			case DebugTreeNode_Value: {
				switch (At->Value.ValueType) {
					case DebugValue_FramesSlider: {
						DEBUGFramesSlider(State);
					}break;

					case DebugValue_ViewFrameInfo: {
						DEBUGViewingFrameInfo(State);
					}break;

					case DebugValue_ProfileOverlays: {
						u32 ActiveProfileElement = 0;

						gui_state* GUIState = State->GUIState;

						GUIBeginRow(State->GUIState);
						GUIBeginRadioGroup(State->GUIState, "ProfileMenuRG", 0);
						GUIRadioButton(State->GUIState, "Clocks", DebugProfileActiveElement_TopClocks);
						GUIRadioButton(State->GUIState, "ClocksEx", DebugProfileActiveElement_TopExClocks);
						GUIRadioButton(State->GUIState, "Frames", DebugProfileActiveElement_FrameGraph);
						GUIRadioButton(State->GUIState, "Threads", DebugProfileActiveElement_Threads);
						GUIRadioButton(State->GUIState, "RootNode", DebugProfileActiveElement_RootNodeBlocks);
						GUIEndRadioGroup(State->GUIState, &ActiveProfileElement);
						GUIEndRow(State->GUIState);

						if (ActiveProfileElement == DebugProfileActiveElement_TopClocks) {
							DEBUGClocksList(State, DebugClockList_Total);
						}
						else if (ActiveProfileElement == DebugProfileActiveElement_TopExClocks) {
							DEBUGClocksList(State, DebugClockList_Exclusive);
						}
						else if (ActiveProfileElement == DebugProfileActiveElement_FrameGraph) {
							u32 ActiveElement = 0;

							rect2 GraphRect = DEBUGFramesGraph(State, State->FramesGraphBarType);

							GUIBeginStateChangerGroup(GUIState, "FrameGraphRG", DEBUGFrameGraph_DeltaTime);
							GUIStateChanger(GUIState, "delta time", DEBUGFrameGraph_DeltaTime);
							GUIStateChanger(GUIState, "FPS", DEBUGFrameGraph_FPS);
							GUIStateChanger(GUIState, "records", DEBUGFrameGraph_CollectedRecords);
							GUIEndStateChangerGroupAt(
								GUIState, 
								V2(GraphRect.Min.x, 
									GraphRect.Min.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale), 
								&ActiveElement);

							if (ActiveElement == DEBUGFrameGraph_DeltaTime) {
								State->FramesGraphBarType = DEBUGFrameGraph_DeltaTime;
							}
							else if (ActiveElement == DEBUGFrameGraph_FPS) {
								State->FramesGraphBarType = DEBUGFrameGraph_FPS;
							}
							else if (ActiveElement == DEBUGFrameGraph_CollectedRecords) {
								State->FramesGraphBarType = DEBUGFrameGraph_CollectedRecords;
							}
						}
						else if (ActiveProfileElement == DebugProfileActiveElement_Threads) {
							//DEBUGThreadsOverlay(State);
						}
						else if (ActiveProfileElement == DebugProfileActiveElement_RootNodeBlocks) {
							u32 ActiveElement = 0;

							debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, State->ViewFrameIndex);

							u64 Clocks = 0;
							if (Frame->FrameUpdateNode) {
								Clocks = Frame->FrameUpdateNode->TimingSnapshot.ClocksElapsed;
							}

							char InfoBuf[64];
							stbsp_sprintf(InfoBuf, "Clocks - %llu", Clocks);
							GUIText(GUIState, InfoBuf);

							rect2 GraphRect = DEBUGFramesGraph(State, State->RootNodeBarType);

							GUIBeginStateChangerGroup(GUIState, "RootNodeRG", DEBUGFrameGraph_RootNodeBlocks);
							GUIStateChanger(GUIState, "Blocks", DEBUGFrameGraph_RootNodeBlocks);
							GUIEndStateChangerGroupAt(
								GUIState,
								V2(GraphRect.Min.x,
									GraphRect.Min.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale),
								&ActiveElement);

							if (ActiveElement == DEBUGFrameGraph_RootNodeBlocks) {
								State->RootNodeBarType = DEBUGFrameGraph_RootNodeBlocks;
							}
						}
					}break;

					case DebugValue_Logger: {
						DEBUGLogger(State);
					}break;
				}
			}break;
		}
	}
}

static void DEBUGOverlayToOutput(debug_state* State) {
	FUNCTION_TIMING();

	gui_state* GUIState = State->GUIState;

	int LastFrameIndex;

	LastFrameIndex = (int)State->CollationFrameIndex - 1;
	if (LastFrameIndex < 0) {
		LastFrameIndex += DEBUG_FRAMES_COUNT;
	}


	float TempDT = State->GUIState->Input->DeltaTime;
	char DebugStr[128];
	float LastFrameFPS = 1.0f / TempDT;
	stbsp_sprintf(DebugStr, "%.2fmsp/f %.2fFPS", TempDT * 1000.0f, LastFrameFPS);


	GUIBeginLayout(State->GUIState, "DEBUG", GUILayout_Tree);
	GUITreeBegin(State->GUIState, "DEBUG", DebugStr);
	//GUIChangeTreeNodeText(State->GUIState, "Hello world Pazha Biceps my friend");

	DEBUGOutputSectionChildrenToGUI(State, State->RootSection);

#if 0
	GUITreeBegin(GUIState, "Test");
	GUITreeBegin(GUIState, "Other");

	GUIBeginRow(GUIState);
	//GUISlider(GUIState, "Slider2", -1000.0f, 10.0f, &SliderInteract);
	GUIText(GUIState, "Hello");

#if 1
	GUIBeginLayout(GUIState, "InnerView", GUILayout_Tree);
	GUIText(GUIState, "Nikita loh");
	GUITreeBegin(GUIState, "InnerTree");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUIText(GUIState, "Nikita loh");
	GUITreeEnd(GUIState);
	GUITreeBegin(GUIState, "InnerTree1");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUIText(GUIState, "Ivan loh");
	GUITreeEnd(GUIState);
	GUIText(GUIState, "Dima pidor");
	GUIEndLayout(GUIState, GUILayout_Tree);
#endif
	GUIEndRow(GUIState);
	GUITreeEnd(GUIState);// Other...

#if 0
	GUITreeBegin(GUIState, "TestMenus");
	GUIBeginMenuBar(GUIState, "Menu1");

	GUIBeginMenuBarItem(GUIState, "Dima");
	GUIMenuBarItem(GUIState, "Item_1_1");
	GUIMenuBarItem(GUIState, "Item_1_2");
	GUIMenuBarItem(GUIState, "Item_1_3");
	GUIMenuBarItem(GUIState, "Item_1_1asdf");
	GUIMenuBarItem(GUIState, "Item_1_2asdf");
	GUIMenuBarItem(GUIState, "Item_1_3asdf");
	GUIMenuBarItem(GUIState, "Item_1_1asdfasdf");
	GUIMenuBarItem(GUIState, "Item_1_2asdfasdf");
	GUIMenuBarItem(GUIState, "Item_1_3asdfasdf");
	GUIEndMenuBarItem(GUIState);

	GUIBeginMenuBarItem(GUIState, "Ivan");
	GUIMenuBarItem(GUIState, "Item_2_1");
	GUIMenuBarItem(GUIState, "Item_2_2");
	GUIMenuBarItem(GUIState, "Item_2_3");
	GUIMenuBarItem(GUIState, "Item_2_1asdf");
	GUIMenuBarItem(GUIState, "Item_2_2asdf");
	GUIMenuBarItem(GUIState, "Item_2_3asdf");
	GUIMenuBarItem(GUIState, "Item_2_1asdfasdf");
	GUIMenuBarItem(GUIState, "Item_2_2asdfasdf");
	GUIMenuBarItem(GUIState, "Item_2_3asdfasdf");
	GUIEndMenuBarItem(GUIState);

	GUIBeginMenuBarItem(GUIState, "Vovan");
	GUIMenuBarItem(GUIState, "Item_3_1");
	GUIMenuBarItem(GUIState, "Item_3_2");
	GUIMenuBarItem(GUIState, "Item_3_3");
	GUIEndMenuBarItem(GUIState);
	GUIEndMenuBar(GUIState);
	GUITreeEnd(GUIState);
#endif

	GUITreeBegin(GUIState, "TestText");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
	GUITreeEnd(GUIState);

	GUITreeBegin(GUIState, "Colors");
	for (int ColorIndex = 0;
		ColorIndex < Min(30, GUIColor_Count);
		ColorIndex++)
	{
		gui_color_slot* Slot = &GUIState->ColorTable[ColorIndex];

		char ColorNameBuf[32];
		stbsp_sprintf(ColorNameBuf, "%-15s", Slot->Name);

		GUIColorView(GUIState, Slot->Color, ColorNameBuf);
	}
	GUITreeEnd(GUIState);

	GUITreeEnd(GUIState); //Test

	GUITreeBegin(GUIState, "Test2");
	GUIImageView(GUIState, "CelluralImage", 0);
	GUIImageView(GUIState, "AlphaImage", 0);
	GUIStackedMemGraph(GUIState, "NULLMemGraph", 0);
	GUIStackedMemGraph(GUIState, "NullImageTest", 0);
	GUIImageView(GUIState, "PotImage", 0);
	GUIImageView(GUIState, "FontAtlas", &GUIState->FontInfo->FontAtlasImage);
	GUITreeEnd(GUIState);

#if 0
	GUITreeBegin(GUIState, "Test3");
	GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);

	GUIBeginRow(GUIState);
	GUIText(GUIState, "Hello");
	GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);

	GUIBeginRow(GUIState);
	GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);

	GUIBeginRow(GUIState);
	GUIText(GUIState, "Hello");
	GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);

	GUIBeginRow(GUIState);
	GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);

	GUIBeginRow(GUIState);
	GUIText(GUIState, "Hello");
	GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);		GUIBeginRow(GUIState);
	GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);

	GUIBeginRow(GUIState);
	GUIText(GUIState, "Hello");
	GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
	GUIEndRow(GUIState);
	GUITreeEnd(GUIState);
#endif

	GUITreeBegin(GUIState, "Audio");
	GUIColorView(GUIState, V4(0.4f, 0.0f, 1.0f, 1.0f), "asd");
	GUIBeginRow(GUIState);
	GUIVector2View(GUIState, V2(1.0f, 256.0f), "Vector2");
	GUIText(GUIState, "Hello");
	GUIVector3View(GUIState, V3(1.0f, 20.0f, 300.0f), "Vector3");
	GUIEndRow(GUIState);
	GUIVector4View(GUIState, V4(12345.0f, 1234.0f, 123456.0f, 5324123.0f), "Vector4");
	GUIInt32View(GUIState, 12345, "TestInt");
	GUITreeEnd(GUIState);

	GUITreeBegin(GUIState, "PlatformMemoryBlocks");
	GUIStackedMemGraph(GUIState, "GameModeMem", &PlatformApi.GameModeMemoryBlock);
	GUIStackedMemGraph(GUIState, "GeneralMem", &PlatformApi.GeneralPurposeMemoryBlock);
	GUIStackedMemGraph(GUIState, "DEBUGMem", &PlatformApi.DEBUGMemoryBlock);
	GUITreeEnd(GUIState);
#endif


	GUITreeEnd(State->GUIState);
	GUIEndLayout(State->GUIState, GUILayout_Tree);
}

void DEBUGUpdate(debug_state* State) {
	FUNCTION_TIMING();

	BEGIN_SECTION("DEBUG");
	DEBUG_VALUE(DebugValue_Logger);
	END_SECTION();

	DEBUGProcessRecords(State);

#if DEBUG_NORMALIZE_FRAME_GRAPH
	if (State->LastCollationFrameIndex != State->CollationFrameIndex) {
		debug_profiled_frame* NewFrame = DEBUGGetNewestFrame(State);

		float TempFPS = 1.0f / NewFrame->DeltaTime;
		State->MaxSegmentFPS = Max(State->MaxSegmentFPS, TempFPS);
		State->MaxSegmentDT = Max(State->MaxSegmentDT, NewFrame->DeltaTime);
		State->MaxSegmentCollectedRecords = Max(State->MaxSegmentCollectedRecords, NewFrame->RecordCount);

		State->SegmentFrameCount++;
	}


	if ((State->SegmentFrameCount % DEBUG_NORMALIZE_FRAME_FREQUENCY) == 0) {
		State->SegmentFrameCount = 0;
		State->NotFirstSegment = 1;

		State->MaxLastSegmentCollectedRecords = State->MaxSegmentCollectedRecords;
		State->MaxLastSegmentDT = State->MaxSegmentDT;
		State->MaxLastSegmentFPS = State->MaxSegmentFPS;

		State->MaxSegmentCollectedRecords = 0;
		State->MaxSegmentDT = 0.0f;
		State->MaxSegmentFPS = 0.0f;
	}
#endif

	DEBUGOverlayToOutput(State);
}