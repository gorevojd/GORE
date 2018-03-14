#include "gore_debug.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

inline void DEBUGDeallocateTreeNode(debug_state* State, debug_tree_node* Entry) {
	Entry->PrevBro->NextBro = Entry->NextBro;
	Entry->NextBro->PrevBro = Entry->PrevBro;

	Entry->NextBro = State->FreeBlockSentinel->NextBro;
	Entry->PrevBro = State->FreeBlockSentinel;

	Entry->NextBro->PrevBro = Entry;
	Entry->PrevBro->NextBro = Entry;
}

inline debug_tree_node* DEBUGAllocateTreeNode(debug_state* State) {
	debug_tree_node* Result = 0;

	if (State->FreeBlockSentinel->NextBro != State->FreeBlockSentinel) {
		Result = State->FreeBlockSentinel->NextBro;

		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->DebugMemory, debug_tree_node);
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

inline void DEBUGAllocateElementSentinelTreeNode(debug_state* State, debug_tree_node* Element) {
	Element->ChildrenSentinel= DEBUGAllocateTreeNode(State);

	Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->Parent = Element;
}

inline debug_statistic* DEBUGAllocateStatistic(debug_state* State) {
	debug_statistic* Result = 0;

	if (State->FreeStatisticSentinel->NextBro != State->FreeStatisticSentinel) {
		Result = State->FreeStatisticSentinel->NextBro;
		
		Result->PrevBro->NextBro = Result->NextBro;
		Result->NextBro->PrevBro = Result->PrevBro;
	}
	else {
		Result = PushStruct(&State->DebugMemory, debug_statistic);
	}

	Result->PrevBro = Result;
	Result->NextBro = Result;

	return(Result);
}

inline void DEBUGDeallocateStatistic(debug_state* State, debug_statistic* Statistic) {
	Statistic->NextBro->PrevBro = Statistic->PrevBro;
	Statistic->PrevBro->NextBro = Statistic->NextBro;

	Statistic->PrevBro = State->FreeStatisticSentinel;
	Statistic->NextBro = State->FreeStatisticSentinel->NextBro;

	Statistic->PrevBro->NextBro = Statistic;
	Statistic->NextBro->PrevBro = Statistic;

	Statistic->NextInHash = 0;
}

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

inline debug_tree_node* DEBUGInitLayerTreeNode(
	debug_state* State, 
	debug_tree_node* CurrentBlock, 
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
		NewBlock = DEBUGAllocateTreeNode(State);

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
			DEBUGAllocateElementSentinelTreeNode(State, NewBlock);
		}

		DEBUGInsertToList(CurrentBlock->ChildrenSentinel, NewBlock);
	}

	return(NewBlock);
}

inline debug_statistic* DEBUGInitDebugStatistic(
	debug_state* State, 
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
		Statistic = DEBUGAllocateStatistic(State);

		Statistic->NextBro = StatisticSentinel->NextBro;
		Statistic->PrevBro = StatisticSentinel;

		Statistic->NextBro->PrevBro = Statistic;
		Statistic->PrevBro->NextBro = Statistic;
		Statistic->NextInHash = 0;

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

static void DEBUGFreeFrameInfo(debug_state* State, debug_profiled_frame* Frame) {
	FUNCTION_TIMING();

	Frame->DeltaTime = 0.0f;

	debug_statistic* TimingAt = Frame->TimingStatisticSentinel->NextBro;
	while (TimingAt != Frame->TimingStatisticSentinel) {
		debug_statistic* TempNext = TimingAt->NextBro;
		
		DEBUGDeallocateStatistic(State, TimingAt);

		TimingAt = TempNext;
	}

	for (int TimingStatisticIndex = 0;
		TimingStatisticIndex < DEBUG_TIMING_STATISTICS_COUNT;
		TimingStatisticIndex++)
	{
		Frame->TimingStatistics[TimingStatisticIndex] = 0;
	}
}

void DEBUGInit(debug_state* State, gui_state* GUIState) {
	State->DebugMemory = AllocateStackedMemory(MEGABYTES(16));

	State->GUIState = GUIState;
	
	State->FreeBlockSentinel = PushStruct(&State->DebugMemory, debug_tree_node);
	State->FreeBlockSentinel->NextBro = State->FreeBlockSentinel;
	State->FreeBlockSentinel->PrevBro = State->FreeBlockSentinel;

	State->FreeStatisticSentinel = PushStruct(&State->DebugMemory, debug_statistic);
	State->FreeStatisticSentinel->NextBro = State->FreeStatisticSentinel;
	State->FreeStatisticSentinel->PrevBro = State->FreeStatisticSentinel;

	State->CollationFrameIndex = 0;
	State->ViewFrameIndex = 0;
	State->NewestFrameIndex = 0;
	State->OldestFrameIndex = 0;

	State->RootSection = DEBUGAllocateTreeNode(State);

	DEBUGAllocateElementSentinelTreeNode(State, State->RootSection);

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

		Frame->TimingSentinel = DEBUGAllocateTreeNode(State);
		DEBUGAllocateElementSentinelTreeNode(State, Frame->TimingSentinel);
		Frame->CurrentTiming = Frame->TimingSentinel;

		Frame->TimingStatisticSentinel = DEBUGAllocateStatistic(State);

		Frame->CurrentSection = State->RootSection;

		DEBUGFreeFrameInfo(State, Frame);
	}

	State->FramesGraphType = DEBUGFrameGraph_DeltaTime;

	State->IsRecording = 1;
}

inline debug_profiled_frame* DEBUGGetCollationFrame(debug_state* State) {
	debug_profiled_frame* Frame = &State->Frames[State->CollationFrameIndex];

	return(Frame);
}

inline debug_profiled_frame* DEBUGGetFrameByIndex(debug_state* State, u32 Index) {
	debug_profiled_frame* Frame = &State->Frames[Index % DEBUG_FRAMES_COUNT];

	return(Frame);
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
	State->CollationFrameIndex = DEBUGIncrementFrameIndex(State->CollationFrameIndex);

	if (State->CollationFrameIndex == State->OldestFrameIndex) {
		State->OldeshShouldBeIncremented = 1;
	}
	
	if (State->OldeshShouldBeIncremented) {
		State->OldestFrameIndex = DEBUGIncrementFrameIndex(State->OldestFrameIndex);
	}
}

void DEBUGProcessRecords(debug_state* State) {
	FUNCTION_TIMING();

	u32 RecordCount = GlobalRecordTable->CurrentRecordIndex.value;

	for (u32 CollectedRecordIndex = 0;
		CollectedRecordIndex < RecordCount;
		CollectedRecordIndex++)
	{
		debug_record* Record = &GlobalRecordTable->Records[GlobalRecordTable->CurrentTableIndex.value][CollectedRecordIndex];

		debug_profiled_frame* Frame = DEBUGGetCollationFrame(State);

		switch (Record->RecordType) {
			case DebugRecord_BeginTiming: {
				debug_tree_node* CurrentBlock = Frame->CurrentTiming;

				debug_tree_node* TimingNode = DEBUGInitLayerTreeNode(State, CurrentBlock, Record->UniqueName, DebugTreeNode_Timing);
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

				//NOTE(dima): Going througn children and summing all their total clocks
				TimingSnapshot->ChildrenSumClocks = 0;
				debug_tree_node* At = CurrentBlock->ChildrenSentinel->NextBro;
				for (At; At != CurrentBlock->ChildrenSentinel; At = At->NextBro) {
					TimingSnapshot->ChildrenSumClocks += At->TimingSnapshot.ClocksElapsed;
				}

				//NOTE(dima): Finding needed element in the hash table
				debug_statistic* TimingStatistic = DEBUGInitDebugStatistic(
					State, 
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

				debug_tree_node* NewSection = DEBUGInitLayerTreeNode(State, State->RootSection, Record->UniqueName, DebugTreeNode_Section);

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
				float DeltaTime = Record->Value_F32;

				debug_profiled_frame* CollationFrame = DEBUGGetCollationFrame(State);
				CollationFrame->DeltaTime = DeltaTime;

				//NOTE(dima): Incrementing frame indices;
				DEBUGIncrementFrameIndices(State);

				debug_profiled_frame* NewFrame = DEBUGGetCollationFrame(State);;
				DEBUGFreeFrameInfo(State, NewFrame);

				//NOTE(dima): Section pointer must be equal to initial
				Assert(State->CurrentSection == State->RootSection);
			} break;
		}
	}

	if (State->RecordingChanged) {
		DEBUGSetRecording(State->IsRecording);
		State->RecordingChanged = 0;
	}

	//TODO(dima): Think about when to do this
	int NewTableIndex = !GlobalRecordTable->CurrentTableIndex.value;
	SDL_AtomicSet(&GlobalRecordTable->CurrentTableIndex, NewTableIndex);
	SDL_AtomicSet(&GlobalRecordTable->CurrentRecordIndex, 0);
}

inline void DEBUGOutputSectionChildrenToGUI(debug_state* State, debug_tree_node* TreeNode) {
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

			}break;
		}
	}
}

inline void DEBUGPushFrameColumn(gui_state* GUIState, u32 FrameIndex, v2 InitP, v2 Dim, v4 Color) {
	rect2 ColumnRect = Rect2MinDim(V2(InitP.x + Dim.x * FrameIndex, InitP.y), Dim);

	RENDERPushRect(GUIState->RenderStack, ColumnRect, Color);
}

void DEBUGFramesSlider(debug_state* State) {
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

		RENDERPushRect(GUIState->RenderStack, GraphMin, *GraphDim, GUIGetColor(GUIState, GUIColorExt_gray15));

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
				GUILabel(GUIState, LabelBuf, GUIState->Input->MouseP);

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

void DEBUGFramesGraph(debug_state* State) {
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

		switch (State->FramesGraphType) {

			case DEBUGFrameGraph_DeltaTime: {
				float OneOverMaxMs = 30.0f;
				for (int ColumnIndex = 0;
					ColumnIndex < DEBUG_FRAMES_COUNT;
					ColumnIndex++)
				{
					debug_profiled_frame* Frame = DEBUGGetFrameByIndex(State, ColumnIndex);

					if (ColumnIndex == State->CollationFrameIndex) {
						RENDERPushRect(GUIState->RenderStack, ColumnRect, GUIGetColor(GUIState, GUIColorExt_green1));
					}
					else {
						float FilledPercentage = Frame->DeltaTime * OneOverMaxMs;
						if (FilledPercentage > 1.0f) {
							FilledPercentage = 1.0f;
						}

						rect2 FilledRect = Rect2MinMax(V2(ColumnRect.Min.x, ColumnRect.Max.y - ColumnDim.y * FilledPercentage), ColumnRect.Max);
						rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, ColumnDim.y * (1.0f - FilledPercentage)));

						v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);
						
						if (ColumnIndex == State->ViewFrameIndex) {
							ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
						}

						RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIColor_Purple) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIColorExt_gray80) * ColorMultiplier);
					}

					ColumnRect.Min.x += OneColumnWidth;
					ColumnRect.Max.x += OneColumnWidth;
				}
			}break;

			case DEBUGFrameGraph_FPS: {
				float OneOverMaxShowFPS = 1.0f / 300.0f;
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
						float FilledPercentage = CurrentFPS * OneOverMaxShowFPS;
						if (FilledPercentage > 1.0f) {
							FilledPercentage = 1.0f;
						}

						rect2 FilledRect = Rect2MinMax(V2(ColumnRect.Min.x, ColumnRect.Max.y - ColumnDim.y * FilledPercentage), ColumnRect.Max);
						rect2 FreeRect = Rect2MinDim(ColumnRect.Min, V2(ColumnDim.x, ColumnDim.y * (1.0f - FilledPercentage)));

						v4 ColorMultiplier = V4(1.0f, 1.0f, 1.0f, 1.0f);

						if (ColumnIndex == State->ViewFrameIndex) {
							ColorMultiplier = V4(0.5f, 0.5f, 0.5f, 1.0f);
						}

						RENDERPushRect(GUIState->RenderStack, FilledRect, GUIGetColor(GUIState, GUIColor_FPSOrange) * ColorMultiplier);
						RENDERPushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIColor_FPSBlue) * ColorMultiplier);
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
		u32 ActiveElement = 0;

		GUIBeginStateChangerGroup(GUIState, DEBUGFrameGraph_DeltaTime);
		GUIStateChanger(GUIState, "delta time", DEBUGFrameGraph_DeltaTime);
		GUIStateChanger(GUIState, "FPS", DEBUGFrameGraph_FPS);
		GUIEndStateChangerGroupAt(GUIState, V2(GraphMin.x, GraphMin.y + AscByScale), &ActiveElement);

		if (ActiveElement == DEBUGFrameGraph_DeltaTime) {
			State->FramesGraphType = DEBUGFrameGraph_DeltaTime;
		}
		else if (ActiveElement == DEBUGFrameGraph_FPS) {
			State->FramesGraphType = DEBUGFrameGraph_FPS;
		}
#endif

		GUIDescribeElement(GUIState, *GraphDim, GraphMin);
		GUIAdvanceCursor(GUIState, AscByScale * 0.5f);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void DEBUGViewingFrameInfo(debug_state* State) {
	char Buf[256];

	debug_profiled_frame* ViewingFrame = DEBUGGetFrameByIndex(State, State->ViewFrameIndex);

	stbsp_sprintf(Buf, "Viewing frame: %.2fms, %.2fFPS", ViewingFrame->DeltaTime * 1000.0f, 1.0f / ViewingFrame->DeltaTime);
	GUIText(State->GUIState, Buf);
}

inline u64 DEBUGGetClocksFromTiming(debug_statistic* Stat, u32 Type) {
	u64 Result = 0;

	if (Type == DebugRecord_TopTotalClocks) {
		Result = Stat->Timing.TotalClocks;
	}
	else if (Type == DebugRecord_TopExClocks) {
		Result = Stat->Timing.TotalClocks - Stat->Timing.TotalClocksInChildren;
	}
	else {
		Assert(!"Invalid type");
	}

	return(Result);
}

void DEBUGClocksList(debug_state* State, u32 Type) {
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

			Cache->Dimensional.Dimension = V2(650, AscByScale * 20);

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

		debug_statistic* Timing = Frame->TimingStatisticSentinel->NextBro;
		for (
			Timing; 
			Timing != Frame->TimingStatisticSentinel; 
			Timing = Timing->NextBro) 
		{
			if (AtY < GroundRc.Max.y + GUIState->FontInfo->DescenderHeight * GUIState->FontScale) {
				char TextBuf[256];
		
				u64 ToViewClocks = DEBUGGetClocksFromTiming(Timing, Type);
		
				stbsp_sprintf(TextBuf, "%11lluc %13.2fc/h %8u  %-30s",
					ToViewClocks,
					(float)ToViewClocks / (float)Timing->Timing.HitCount,
					Timing->Timing.HitCount,
					Timing->Name);

				v4 TextColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
				v4 TextHighColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);

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

enum debug_profile_active_element {
	DebugProfileActiveElement_TopClocks,
	DebugProfileActiveElement_TopExClocks,
	DebugProfileActiveElement_FrameGraph,
};

void DEBUGOverlayToOutput(debug_state* State) {
	FUNCTION_TIMING();

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

	DEBUGFramesSlider(State);

	DEBUGViewingFrameInfo(State);

	u32 ActiveProfileElement = 0;

	GUIBeginRow(State->GUIState);
	GUIBeginRadioGroup(State->GUIState, 0);
	GUIRadioButton(State->GUIState, "Clocks", DebugProfileActiveElement_TopClocks);
	GUIRadioButton(State->GUIState, "ClocksEx", DebugProfileActiveElement_TopExClocks);
	GUIRadioButton(State->GUIState, "Frames", DebugProfileActiveElement_FrameGraph);
	GUIEndRadioGroup(State->GUIState, &ActiveProfileElement);
	GUIEndRow(State->GUIState);

	if (ActiveProfileElement == DebugProfileActiveElement_TopClocks) {
		DEBUGClocksList(State, DebugRecord_TopTotalClocks);
	}
	else if (ActiveProfileElement == DebugProfileActiveElement_TopExClocks) {
		DEBUGClocksList(State, DebugRecord_TopExClocks);
	}
	else if (ActiveProfileElement == DebugProfileActiveElement_FrameGraph) {
		DEBUGFramesGraph(State);
	}

	GUITreeEnd(State->GUIState);
	GUIEndLayout(State->GUIState, GUILayout_Tree);
}

void DEBUGUpdate(debug_state* State) {
	FUNCTION_TIMING();

	BEGIN_SECTION("Profile");

	DEBUGProcessRecords(State);

	DEBUGOverlayToOutput(State);

	END_SECTION();
}