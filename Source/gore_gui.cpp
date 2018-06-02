#include "gore_gui.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

void GUIInitState(
	gui_state* GUIState, 
	stacked_memory* GUIMemory,
	color_state* ColorState,
	font_info* FontInfo,
	input_system* Input, 
	i32 Width, i32 Height) 
{
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;
	GUIState->ColorsState = ColorState;

	GUIState->Input = Input;

	GUIState->FontScale = 1.0f;
	GUIState->LastFontScale = 1.0f;
	GUIState->TextElemsCacheShouldBeReinitialized = false;

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

	GUIState->PlusMinusSymbol = 0;

	GUIState->GUIMem = GUIMemory;

#if 0
	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
#endif

	//NOTE(dima): Initialization of tooltips
	GUIState->TooltipCount = 0;

	//NOTE(Dima): Initialization of the root node;
	GUIState->RootNode = PushStruct(GUIState->GUIMem, gui_element);
	gui_element* RootNode = GUIState->RootNode;

	RootNode->PrevBro = RootNode;
	RootNode->NextBro = RootNode;

	char* RootNodeName = "MainRoot";
	CopyStrings(RootNode->Name, RootNodeName);
	CopyStrings(RootNode->Text, RootNodeName);

	RootNode->ID = GUIStringHashFNV(RootNode->Name);
	RootNode->Expanded = 1;
	RootNode->Depth = 0;

	RootNode->Parent = 0;
	RootNode->Type = GUIElement_None;

	RootNode->ChildrenSentinel = PushStruct(GUIState->GUIMem, gui_element);

	RootNode->ChildrenSentinel->NextBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->PrevBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->Parent = RootNode;

	RootNode->NextBro = RootNode;
	RootNode->PrevBro = RootNode;

	RootNode->TempParentTree = 0;

	GUIState->WalkaroundEnabled = false;
	GUIState->WalkaroundIsHot = false;
	GUIState->WalkaroundElement = RootNode;

	GUIState->CurrentNode = RootNode;
	GUIState->CurrentTreeParent = RootNode;

	/*
	NOTE(Dima):
	Initialization of the "Free store" of the gui elements;
	Used for freeing and allocating static gui elements;
	Static gui elements has no cache and can be freed every frame;
	*/
	GUIState->FreeElementsSentinel = PushStruct(GUIState->GUIMem, gui_element);
	GUIState->FreeElementsSentinel->NextBro = GUIState->FreeElementsSentinel;
	GUIState->FreeElementsSentinel->PrevBro = GUIState->FreeElementsSentinel;

	//NOTE(Dima): Initialization of view sentinel
	GUIState->LayoutSentinel = PushStruct(GUIState->GUIMem, gui_layout);
	GUIState->LayoutSentinel->NextBro = GUIState->LayoutSentinel;
	GUIState->LayoutSentinel->PrevBro = GUIState->LayoutSentinel;

	//NOTE(Dima): Initialization of view free list sentinel element
	GUIState->FreeLayoutSentinel = PushStruct(GUIState->GUIMem, gui_layout);
	GUIState->FreeLayoutSentinel->NextBro = GUIState->FreeLayoutSentinel;
	GUIState->FreeLayoutSentinel->PrevBro = GUIState->FreeLayoutSentinel;

	//NOTE(DIMA): Allocating and initializing default view
	gui_layout* DefaultView = PushStruct(GUIState->GUIMem, gui_layout);
	*DefaultView = {};
	DefaultView->NextBro = GUIState->LayoutSentinel->NextBro;
	DefaultView->PrevBro = GUIState->LayoutSentinel;

	DefaultView->NextBro->PrevBro = DefaultView;
	DefaultView->PrevBro->NextBro = DefaultView;

	DefaultView->ID = GUIStringHashFNV("DefaultView");

	DefaultView->ViewType = GUILayout_Tree;

	DefaultView->CurrentX = 0;
	DefaultView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;

	GUIState->DefaultLayout = DefaultView;
	GUIState->CurrentLayout = DefaultView;

	//NOTE(DIMA): Initialization of the color theme
	GUIState->ColorTheme = GUIDefaultColorTheme();
}

enum gui_print_text_flag {
	GUIPrintText_GetSize = 1,
	GUIPrintText_Print = 2,
	GUIPrintText_Multiline = 4,
};

static rect2 PrintTextInternal(gui_state* State, u32 Flags, char* Text, v2 P, float Scale, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	FUNCTION_TIMING();

	rect2 TextRect = {};

	b32 IsMultiline = (Flags & GUIPrintText_Multiline);
	b32 IsPrint = (Flags & GUIPrintText_Print);
	b32 IsGetSize = (Flags & GUIPrintText_GetSize);

	v2 CurrentP = P;

	char* At = Text;

	font_info* FontInfo = State->FontInfo;
	render_state* Stack = State->RenderStack;

	RENDERPushBeginText(Stack, FontInfo);

	float MaxRowPX = 0.0f;
	float CurGlyphAdvance = 0.0f;

	while (*At) {
		b32 CharIsValid = (*At >= ' ') && (*At <= '~');
		int GlyphIndex = 0;
		if (CharIsValid) {
			GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		}

		glyph_info* Glyph = &FontInfo->Glyphs[GlyphIndex];
		CurGlyphAdvance = Glyph->Advance;

		float BitmapScale = Glyph->Height * Scale;
		
		if (IsMultiline) {
			if (*At == '\n' ||
				*At == '\r')
			{
				if (CurrentP.x > MaxRowPX) {
					MaxRowPX = CurrentP.x;
				}

				CurrentP.x = P.x;
				CurrentP.y += GetNextRowAdvance(FontInfo) * Scale;

				++At;
				continue;
			}
		}

		if (IsPrint && CharIsValid)
		{
			float BitmapMinY = CurrentP.y + (Glyph->YOffset - 1.0f) * Scale;
			float BitmapMinX = CurrentP.x + (Glyph->XOffset - 1.0f) * Scale;

			v2 BitmapDim = { Glyph->Bitmap.WidthOverHeight * BitmapScale, BitmapScale };

			//RENDERPushGlyph(Stack, *At, { BitmapMinX + 2, BitmapMinY + 2}, BitmapDim, V4(0.0f, 0.0f, 0.0f, 1.0f));
			RENDERPushGlyph(Stack, *At, { BitmapMinX, BitmapMinY }, BitmapDim, Color);
		}

		if (*At == '\t') {
			CurGlyphAdvance = Glyph->Advance * 2.0f;
		}

		float Kerning = 0.0f;
		char NextChar = *(At + 1);
		if (NextChar >= ' ' && NextChar <= '~') {
			Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
		}

		CurrentP.x += (CurGlyphAdvance + Kerning) * Scale;

		++At;
	}

	if (CurrentP.x > MaxRowPX) {
		MaxRowPX = CurrentP.x;
	}

	RENDERPushEndText(Stack);

	TextRect.Min.x = P.x;
	TextRect.Min.y = P.y - FontInfo->AscenderHeight * Scale;
	TextRect.Max.x = MaxRowPX;
	TextRect.Max.y = CurrentP.y - FontInfo->DescenderHeight * Scale;

	return(TextRect);
}

rect2 GUIPrintText(gui_state* GUIState, char* Text, v2 P, float Scale, v4 Color) {
	rect2 Result = PrintTextInternal(GUIState, GUIPrintText_Print, Text, P, Scale, Color);

	return(Result);
}

rect2 GUIPrintTextMultiline(gui_state* GUIState, char* Text, v2 P, float Scale, v4 Color) {
	rect2 Result = PrintTextInternal(GUIState, GUIPrintText_Print | GUIPrintText_Multiline, Text, P, Scale, Color);

	return(Result);
}

v2 GUIGetTextSize(gui_state* GUIState, char* Text, float Scale) {
	rect2 TextRc = PrintTextInternal(GUIState, GUIPrintText_GetSize, Text, {}, Scale);

	v2 Result = GetRectDim(TextRc);
	return(Result);
}

v2 GUIGetTextSizeMultiline(gui_state* GUIState, char* Text, float Scale) {
	rect2 TextRc = PrintTextInternal(GUIState, GUIPrintText_GetSize | GUIPrintText_Multiline, Text, {}, Scale);

	v2 Result = GetRectDim(TextRc);
	return(Result);
}

void GUIBeginFrame(gui_state* GUIState, render_state* RenderStack) {
	GUIState->RenderStack = RenderStack;

	GUIState->TooltipCount = 0;
}

inline b32 GUIElementIsValidForWalkaround(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;

		if (Parent) {
			if (Parent->ChildrenSentinel != Element) {
#if 1
				Result =
					(Element->Type != GUIElement_StaticItem) &&
					(Element->Type != GUIElement_Row) &&
					(Element->Type != GUIElement_Layout) &&
					(Element->Type != GUIElement_None);
#else
				Result = Element->Type == GUIElement_TreeNode;
#endif
			}
		}
		else {
			//NOTE(DIMA): This is the root element
			Assert(Element->Type == GUIElement_None);
			Result = 0;
		}
	}

	return(Result);
}

inline b32 GUIElementIsSentinelOfRow(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;
		if (Element == Parent->ChildrenSentinel &&
			Parent->Type == GUIElement_Row)
		{
			Result = 1;
		}
	}

	return(Result);
}

enum gui_walkaround_type {
	GUIWalkaround_None,

	GUIWalkaround_Next,
	GUIWalkaround_Prev,
};

inline gui_element* GUIWalkaroundStep(gui_element* Elem, u32 WalkaroundType) {
	gui_element* Result = 0;

	if (WalkaroundType == GUIWalkaround_Next) {
		Result = Elem->NextBro;
	}
	else if (WalkaroundType == GUIWalkaround_Prev) {
		Result = Elem->PrevBro;
	}
	else {
		Assert(!"Invalid walkaround type");
	}

	return(Result);
}

inline gui_element* GUIFindNextForWalkaroundInRow(gui_element* Row, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Row->ChildrenSentinel, WalkaroundType);

	while (At != Row->ChildrenSentinel) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					return(Result);
				}
			}
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

inline gui_element* GUIFindForWalkaround(gui_element* Element, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Element, WalkaroundType);
	while (At != Element) {

		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
#if 1
			if (GUIElementIsSentinelOfRow(At)) {
				At = At->Parent;
				At = GUIWalkaroundStep(At, WalkaroundType);
				continue;
			}

			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					break;
				}
			}
#endif
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

static gui_element* GUIFindNextForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Next);

	return(Result);
}

static gui_element* GUIFindPrevForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Prev);

	return(Result);
}

inline gui_element* GUIFindElementForWalkaroundBFS(gui_element* CurrentElement) {
	gui_element* Result = 0;

	u32 WalkaroundType = GUIWalkaround_Prev;

	if (CurrentElement->ChildrenSentinel) {
		gui_element* At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);

		while (At != CurrentElement->ChildrenSentinel) {

			if (GUIElementIsValidForWalkaround(At)) {
				Result = At;
				return(Result);
			}
			else {
#if 1
				if (GUIElementIsSentinelOfRow(At)) {
					At = At->Parent;
					At = GUIWalkaroundStep(At, WalkaroundType);
					continue;
				}

				if (At->Type == GUIElement_Row) {
					//Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
					Result = GUIFindElementForWalkaroundBFS(At);
					if (Result) {
						return(Result);
					}
				}
#endif
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}

		At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);
		while (At != CurrentElement->ChildrenSentinel) {
			Result = GUIFindElementForWalkaroundBFS(At);
			if (Result) {
				return(Result);
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}
	}

	return(Result);
}

inline gui_element* GUIFindTrueParent(gui_element* Elem) {
	gui_element* Result = 0;

	gui_element* At = Elem->Parent;
	while (At) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}

		At = At->Parent;
	}

	if (!Result) {
		Result = Elem;
	}

	return(Result);
}

inline b32 GUIWalkaroundIsOnElement(gui_state* State, gui_element* Element) {
	b32 Result = 0;

	if (State->WalkaroundEnabled) {
		if (State->WalkaroundElement == Element) {
			Result = 1;
		}
	}

	return(Result);
}

inline b32 GUIWalkaroundIsHere(gui_state* State) {
	gui_layout* View = GUIGetCurrentLayout(State);

	b32 Result = GUIWalkaroundIsOnElement(State, State->CurrentNode);

	return(Result);
}

void GUIPrepareFrame(gui_state* GUIState) {
	//NOTE(dima): Tooltips processing
	float RowAdvance = GetNextRowAdvance(GUIState->FontInfo);
	v2 PrintP = GUIState->Input->MouseP;

	for (int TooltipIndex = 0;
		TooltipIndex < GUIState->TooltipCount;
		TooltipIndex++)
	{
		PrintTextInternal(GUIState, GUIPrintText_Print,
			GUIState->Tooltips[TooltipIndex],
			PrintP, GUIState->FontScale * 0.7f,
			GUIGetColor(GUIState, GUIState->ColorTheme.TooltipTextColor));

		PrintP.y += RowAdvance;
	}

	GUIState->TooltipCount = 0;
}

void GUIEndFrame(gui_state* GUIState) {
	FUNCTION_TIMING();

	BEGIN_SECTION("GUI");

	//NOTE(DIMA): Processing walkaround
	if (ButtonWentDown(GUIState->Input, KeyType_Backquote)) {
		GUIState->WalkaroundEnabled = !GUIState->WalkaroundEnabled;
	}

	if (GUIState->WalkaroundEnabled) {
		gui_element** Walk = &GUIState->WalkaroundElement;

		if (!GUIElementIsValidForWalkaround(*Walk)) {
			*Walk = GUIFindElementForWalkaroundBFS(*Walk);
		}

		if (!GUIState->WalkaroundIsHot) {

			if (ButtonWentDown(GUIState->Input, KeyType_Up)) {
				gui_element* PrevElement = GUIFindNextForWalkaround(*Walk);

				if (PrevElement) {
					*Walk = PrevElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Down)) {
				gui_element* NextElement = GUIFindPrevForWalkaround(*Walk);

				if (NextElement) {
					*Walk = NextElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Right)) {
				if ((*Walk)->Expanded) {
					gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);

					if (FirstChildren) {
						*Walk = FirstChildren;
					}
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Left)) {
				*Walk = GUIFindTrueParent(*Walk);
			}
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
#if 0
			if ((*Walk)->Type == GUIElement_TreeNode) {
				(*Walk)->Expanded = !(*Walk)->Expanded;
				(*Walk)->Cache.TreeNode.ExitState = GUITreeNodeExit_None;
			}

			if ((*Walk)->Type == GUIElement_InteractibleItem) {
				GUIState->WalkaroundIsHot = !GUIState->WalkaroundIsHot;
			}
#endif
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Backspace)) {
			*Walk = GUIFindTrueParent(*Walk);
			if ((*Walk)->Expanded) {
				(*Walk)->Expanded = 0;
			}

			GUIState->WalkaroundIsHot = 0;
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Tab)) {
			if (!(*Walk)->Expanded) {
				(*Walk)->Expanded = true;
			}
			gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);
			if (FirstChildren) {
				*Walk = FirstChildren;
			}
		}
	}

	//NOTE(DIMA): Resetting default view;
	gui_layout* DefView = GUIState->DefaultLayout;
	DefView->CurrentX = 0;
	DefView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;

	//NOTE(Dima): Checking for if cache should be reinitialized
	if (GUIState->TextElemsCacheShouldBeReinitialized) {
		GUIState->TextElemsCacheShouldBeReinitialized = false;
	}

	if (Abs(GUIState->FontScale - GUIState->LastFontScale) > 0.00001f) {
		GUIState->TextElemsCacheShouldBeReinitialized = true;
	}
	GUIState->LastFontScale = GUIState->FontScale;

	END_SECTION();
}

inline gui_layout* GUIAllocateViewElement(gui_state* State) {
	gui_layout* View = 0;

	if (State->FreeLayoutSentinel->NextBro != State->FreeLayoutSentinel) {
		View = State->FreeLayoutSentinel->NextBro;

		View->NextBro->PrevBro = View->PrevBro;
		View->PrevBro->NextBro = View->NextBro;
	}
	else {
		View = PushStruct(State->GUIMem, gui_layout);
	}

	*View = {};

	return(View);
}

inline void GUIInsertViewElement(gui_state* State, gui_layout* ToInsert) {
	ToInsert->NextBro = State->LayoutSentinel->NextBro;
	ToInsert->PrevBro = State->LayoutSentinel;

	ToInsert->NextBro->PrevBro = ToInsert;
	ToInsert->PrevBro->NextBro = ToInsert;
}

inline void GUIFreeViewElement(gui_state* State, gui_layout* ToFree) {
	ToFree->NextBro->PrevBro = ToFree->PrevBro;
	ToFree->PrevBro->NextBro = ToFree->NextBro;

	ToFree->NextBro = State->FreeLayoutSentinel->NextBro;
	ToFree->PrevBro = State->FreeLayoutSentinel;

	ToFree->NextBro->PrevBro = ToFree;
	ToFree->PrevBro->NextBro = ToFree;
}

void GUIBeginLayout(gui_state* GUIState, char* LayoutName, u32 LayoutType) {
	FUNCTION_TIMING();

	/*
		NOTE(DIMA):
			Here I make unique id for view and calculate
			it's hash. Then I try to find view in Global
			list with the same ID. It's perfomance better
			than comparing two strings. But there is the
			caveat. We need to make sure that the hash
			calculation function is crypto-strong enough
			so that the possibility to have 2 same id's
			for different strings is very small.
	*/

	char IdBuf[256];
	stbsp_snprintf(
		IdBuf, sizeof(IdBuf),
		"%s_TreeID_%u",
		LayoutName,
		GUITreeElementID(GUIState->CurrentNode));
	u32 IdBufHash = GUIStringHashFNV(IdBuf);

	gui_element* CurrentElement = GUIBeginElement(GUIState, GUIElement_Layout, IdBuf, 0, 1);
	gui_layout* ParentView = GUIState->CurrentLayout;

	if (!CurrentElement->Cache.IsInitialized) {
		//IMPORTANT(DIMA): Think about how to choose position that we want
		CurrentElement->Cache.Dimensional.Pos = V2(ParentView->CurrentX, ParentView->CurrentY);
		CurrentElement->Cache.Dimensional.Dim = V2(100, 100);

		CurrentElement->Cache.IsInitialized = true;
	}
	v2* ViewPosition = &CurrentElement->Cache.Dimensional.Pos;


	//NOTE(Dima): Find view in the existing list
	gui_layout* Layout = 0;
	for (gui_layout* At = GUIState->LayoutSentinel->NextBro;
		At != GUIState->LayoutSentinel;
		At = At->NextBro)
	{
		if (IdBufHash == At->ID) {
			Layout = At;
			break;
		}
	}

	//NOTE(Dima): View not found. Should allocate it
	if (Layout == 0) {
		Layout = GUIAllocateViewElement(GUIState);
		GUIInsertViewElement(GUIState, Layout);

		Layout->ID = IdBufHash;
		Layout->ViewType = LayoutType;
		Layout->Parent = ParentView;
		Layout->NeedHorizontalAdvance = (LayoutType == GUILayout_Tree);
	}

	Layout->CurrentX = ViewPosition->x;
	Layout->CurrentY = ViewPosition->y;
	Layout->BeginnedRowsCount = 0;

	GUIState->CurrentLayout = Layout;
}

void GUIEndLayout(gui_state* GUIState, u32 LayoutType) {

	gui_layout* View = GUIGetCurrentLayout(GUIState);

	Assert(View->ViewType == LayoutType);

	GUIEndElement(GUIState, GUIElement_Layout);

	GUIState->CurrentLayout = GUIState->CurrentLayout->Parent;
}

void GUIPreAdvanceCursor(gui_state* State) {
	gui_layout* View = GUIGetCurrentLayout(State);

	gui_element* Element = State->CurrentNode;

	float PreAdvanceValue = 0.0f;
	if (View->NeedHorizontalAdvance) {
		PreAdvanceValue = (Element->Depth - 1) * 2 * State->FontScale * State->FontInfo->AscenderHeight;
	}
	View->CurrentPreAdvance = PreAdvanceValue;

	View->CurrentX += View->CurrentPreAdvance;
}

inline b32 GUIIsRowBeginned(gui_layout* View) {
	b32 Result = (View->BeginnedRowsCount != 0);

	return(Result);
}

void GUIDescribeElement(gui_state* State, v2 ElementDim, v2 ElementP) {
	gui_layout* View = GUIGetCurrentLayout(State);

	View->LastElementP = ElementP;
	View->LastElementDim = ElementDim;

	if ((ElementDim.y > View->RowBiggestHeight) && GUIIsRowBeginned(View)) {
		View->RowBiggestHeight = ElementDim.y;
	}
}

void GUIAdvanceCursor(gui_state* State, float AdditionalYSpacing) {
	gui_layout* View = GUIGetCurrentLayout(State);

#if 0
	if (View->RowBeginned) {
#else
	if(GUIIsRowBeginned(View)){
#endif
		View->CurrentX += View->LastElementDim.x + State->FontInfo->AscenderHeight * State->FontScale;
	}
	else {
#if 0
		View->CurrentY += GetNextRowAdvance(State->FontInfo, 1.2f);
#else
		View->CurrentY += View->LastElementDim.y + GetNextRowAdvance(State->FontInfo) * 0.2 + AdditionalYSpacing;
#endif
	}

	View->CurrentX -= View->CurrentPreAdvance;
}

void GUIBeginRow(gui_state* State) {
	gui_layout* View = GUIGetCurrentLayout(State);

	char NameBuf[16];
	stbsp_sprintf(NameBuf, "Row:%u", State->CurrentNode->RowCount);

	GUIBeginElement(State, GUIElement_Row, NameBuf, 0, 1, 0);

	if (!GUIIsRowBeginned) {
		View->RowBeginX = View->CurrentX;
		View->RowBiggestHeight = 0;
	}

	View->BeginnedRowsCount++;
}

void GUIEndRow(gui_state* State) {
	gui_layout* View = GUIGetCurrentLayout(State);

	b32 NeedShow = GUIElementShouldBeUpdated(State->CurrentNode);

	View->BeginnedRowsCount--;

	if (NeedShow) {
		if (!View->BeginnedRowsCount) {
			View->CurrentX = View->RowBeginX;
			View->CurrentY += View->RowBiggestHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
			View->RowBiggestHeight = 0;
		}
	}

	GUIEndElement(State, GUIElement_Row);

	State->CurrentNode->RowCount++;
}

inline gui_element* GUIAllocateListElement(gui_state* State) {
	gui_element* Element = 0;

	if (State->FreeElementsSentinel->NextBro != State->FreeElementsSentinel) {
		Element = State->FreeElementsSentinel->NextBro;

		Element->NextBro->PrevBro = Element->PrevBro;
		Element->PrevBro->NextBro = Element->NextBro;
	}
	else {
		Element = PushStruct(State->GUIMem, gui_element);
	}

	return(Element);
}

inline void GUIInsertListElement(gui_element* Sentinel, gui_element* ToInsert) {
	ToInsert->NextBro = Sentinel->NextBro;
	ToInsert->PrevBro = Sentinel;

	ToInsert->PrevBro->NextBro = ToInsert;
	ToInsert->NextBro->PrevBro = ToInsert;
}

inline void GUIFreeListElement(gui_state* State, gui_element* Element) {
	Element->NextBro->PrevBro = Element->PrevBro;
	Element->PrevBro->NextBro = Element->NextBro;

	Element->NextBro = State->FreeElementsSentinel->NextBro;
	Element->PrevBro = State->FreeElementsSentinel;

	Element->NextBro->PrevBro = Element;
	Element->PrevBro->NextBro = Element;
}

inline void GUIInitElementChildrenSentinel(gui_state* GUIState, gui_element* Element) {
	Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
	Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->Parent = Element;
}

static gui_element* GUIRequestElement(
	gui_state* GUIState,
	u32 ElementType,
	char* ElementName,
	gui_interaction* Interaction,
	b32 InitExpandedState,
	b32 IncrementDepth)
{
	FUNCTION_TIMING();

	gui_element* Parent = GUIState->CurrentNode;

	gui_element* Element = 0;
	i32 ElementIncrementDepthValue = (IncrementDepth != 0);

	b32 ElementIsDynamic =
		(ElementType != GUIElement_None &&
			ElementType != GUIElement_StaticItem);

	u32 ElementHash = 0;
	if (ElementIsDynamic)
	{
		ElementHash = GUIStringHashFNV(ElementName);

		//NOTE(DIMA): Finding the element in the hierarchy
		for (gui_element* Node = Parent->ChildrenSentinel->NextBro;
			Node != Parent->ChildrenSentinel;
			Node = Node->NextBro)
		{
			//TODO(Dima): Test perfomance
#if 1
			if (StringsAreEqual(ElementName, Node->Name)) {
#else
			if (ElementHash == Node->ID) {
#endif
				Element = Node;
				break;
			}
		}
	}

	if (ElementType == GUIElement_StaticItem) {
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		Element->Expanded = InitExpandedState;
		Element->Depth = Parent->Depth + 1;

		Element->RowCount = 0;
		Element->ID = 0;
		Element->Cache = {};

		Element->ChildrenSentinel = 0;
	}

	//NOTE(Dima): Element not exist or not found. We should allocate it
	if (Element == 0) {
		//NOTE(DIMA): If the "Free store" of elements is not empty, get the memory from there
		//TODO(DIMA): Some elements memory might be initialzed again if we get it from here
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		//NOTE(Dima): Pre-Setting common values
		Element->TempParentTree = GUIState->CurrentTreeParent;

#if 0
		if ((ElementType == GUIElement_TreeNode) ||
			(ElementType == GUIElement_CachedItem))
		{
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_Row) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_InteractibleItem) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			Element->ChildrenSentinel = 0;
		}

		if (ElementType == GUIElement_View) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}
#else
		if ((ElementType == GUIElement_TreeNode) ||
			(ElementType == GUIElement_CachedItem) ||
			(ElementType == GUIElement_RadioGroup) ||
			(ElementType == GUIElement_StateChangerGroup))
		{
			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_Row) {
			ElementIncrementDepthValue = 0;
			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_InteractibleItem) {
			Element->ChildrenSentinel = 0;
		}

		if (ElementType == GUIElement_Layout) {
			ElementIncrementDepthValue = 0;
			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_MenuBar ||
			ElementType == GUIElement_MenuItem)
		{
			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		Element->Expanded = InitExpandedState;
		Element->Depth = Parent->Depth + ElementIncrementDepthValue;
#endif

		CopyStrings(Element->Name, ElementName);
		CopyStrings(Element->Text, ElementName);

		Element->ID = ElementHash;
		Element->RowCount = 0;
		Element->Cache = {};
	}

	//NOTE(Dima): Post-Setting common values
	Element->Type = ElementType;
	Element->Parent = Parent;
	Element->RowCount = 0;

	//NOTE(Dima): Setting interaction ID for dynamic(cached) elements
	if (ElementIsDynamic && Interaction) {
		Interaction->ID = GUITreeElementID(Element);
	}

	return(Element);
}

gui_element* GUIBeginElement(
	gui_state* State,
	u32 ElementType,
	char* ElementName,
	gui_interaction* ElementInteraction,
	b32 InitExpandedState,
	b32 IncrementDepth)
{
	FUNCTION_TIMING();

	gui_element* Element = GUIRequestElement(State, ElementType, ElementName, ElementInteraction, InitExpandedState, IncrementDepth);

	State->CurrentNode = Element;

	if (Element->Type == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element;
	}

	return(Element);
}

void GUIEndElement(gui_state* State, u32 ElementType) {
	FUNCTION_TIMING();

	gui_element* Element = State->CurrentNode;

	Assert(ElementType == Element->Type);

#if 1
	gui_layout* View = GUIGetCurrentLayout(State);

	if (GUIElementShouldBeUpdated(Element)) {
		//NOTE(Dima): Here I remember view Y for exit sliding effect
		gui_element* CurrentTreeParent = State->CurrentTreeParent;
		Assert((CurrentTreeParent->Type == GUIElement_TreeNode) ||
			(CurrentTreeParent == State->RootNode));
		CurrentTreeParent->Cache.TreeNode.StackY = View->CurrentY;
	}
#endif

	if (ElementType == GUIElement_StaticItem) {
		GUIFreeListElement(State, Element);
	}

	if (ElementType == GUIElement_Row) {
		
	}

	if (ElementType == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element->TempParentTree;
	}

	State->CurrentNode = Element->Parent;
}

void GUIPerformInteraction(
	gui_state* GUIState, 
	gui_interaction* Interaction) 
{
	FUNCTION_TIMING();

	switch (Interaction->Type) {
		case GUIInteraction_Resize: {
			v2 WorkRectP = Interaction->ResizeContext.Position;
			v2 MouseP = GUIState->Input->MouseP - Interaction->ResizeContext.OffsetInAnchor;

			v2* WorkDim = Interaction->ResizeContext.DimensionPtr;
			if (Interaction->Type == GUIInteraction_Resize) {

				v2* WorkDim = Interaction->ResizeContext.DimensionPtr;
				switch (Interaction->ResizeContext.Type) {
					case GUIResizeInteraction_Default: {
						*WorkDim = MouseP - WorkRectP;

						if (MouseP.x - WorkRectP.x < Interaction->ResizeContext.MinDim.x) {
							WorkDim->x = Interaction->ResizeContext.MinDim.x;
						}

						if (MouseP.y - WorkRectP.y < Interaction->ResizeContext.MinDim.y) {
							WorkDim->y = Interaction->ResizeContext.MinDim.y;
						}
					}break;

					case GUIResizeInteraction_Horizontal: {
						if (MouseP.x - WorkRectP.x < Interaction->ResizeContext.MinDim.x) {
							WorkDim->x = Interaction->ResizeContext.MinDim.x;
						}
					}break;

					case GUIResizeInteraction_Vertical: {
						if (MouseP.y - WorkRectP.y < Interaction->ResizeContext.MinDim.y) {
							WorkDim->y = Interaction->ResizeContext.MinDim.y;
						}
					}break;

					case GUIResizeInteraction_Proportional: {
						float WidthToHeight = WorkDim->x / WorkDim->y;
						WorkDim->y = MouseP.y - WorkRectP.y;
						WorkDim->x = WorkDim->y * WidthToHeight;

						if (WorkDim->y < Interaction->ResizeContext.MinDim.y) {
							WorkDim->y = Interaction->ResizeContext.MinDim.y;
							WorkDim->x = WorkDim->y * WidthToHeight;
						}
					}break;
				}
			}
		}break;

		case GUIInteraction_Move: {
			v2* WorkP = Interaction->MoveContext.MovePosition;
			v2 MouseP = GUIState->Input->MouseP - Interaction->MoveContext.OffsetInAnchor;

			switch (Interaction->MoveContext.Type) {
				case GUIMoveInteraction_Move: {
					*WorkP = MouseP;
				}break;
			}
		}break;

		case GUIInteraction_Tree: {
			Interaction->TreeInteraction.Elem->Expanded = !Interaction->TreeInteraction.Elem->Expanded;
			Interaction->TreeInteraction.Elem->Cache.TreeNode.ExitState = 0;
			

		} break;

		case GUIInteraction_Bool: {
			b32* WorkValue = Interaction->BoolInteraction.InteractBool;
			if (WorkValue) {
				*WorkValue = !(*WorkValue);
			}
		}break;

		case GUIInteraction_MenuBar: {
			Interaction->MenuMarInteraction.MenuElement->Expanded = !Interaction->MenuMarInteraction.MenuElement->Expanded;
		}break;

		case GUIInteraction_RadioButton: {
			gui_radio_button_interaction_context* Context = &Interaction->RadioButtonInteraction;

			Context->RadioGroup->Cache.RadioCache.ActiveIndex = Context->PressedIndex;
		}break;

		case GUIInteraction_StateChangerGroup: {
			gui_state_changer_group_interaction_context* Context = &Interaction->StateChangerGroupInteraction;

			gui_element* CurrentElement = Context->StateChangerGroup->Cache.StateChangerGroupCache.ActiveElement;

			while (1) {
				gui_element* Next = 0;

				if (Context->IncrementDirection) {
					Next = CurrentElement->NextBro;
				}
				else {
					Next = CurrentElement->PrevBro;
				}

				if (Next != CurrentElement->Parent->ChildrenSentinel) {
					Context->StateChangerGroup->Cache.StateChangerGroupCache.ActiveElement = Next;
					break;
				}

				CurrentElement = Next;
			}

		}break;

		case GUIInteraction_ReturnMouseAction: {
			gui_return_mouse_action_interaction_context* Context = &Interaction->ReturnMouseActionInteraction;

			b32*  ActionHappened = Context->ActionHappened;

			switch (Context->ActionType) {
				case GUIReturnMouseAction_IsDown: {
					*ActionHappened = MouseButtonIsDown(Context->Input, Context->MouseButtonIndex);
				}break;

				case GUIReturnMouseAction_WentDown: {
					*ActionHappened = MouseButtonWentDown(Context->Input, Context->MouseButtonIndex);
				}break;

				case GUIReturnMouseAction_WentUp: {
					*ActionHappened = MouseButtonWentUp(Context->Input, Context->MouseButtonIndex);
				}break;
			}
		}break;

		case GUIInteraction_None: {

		}break;
	}
}

rect2 GUITextBase(
	gui_state* GUIState,
	char* Text,
	v2 Pos,
	v4 TextColor,
	float FontScale,
	v4 TextHighlightColor,
	v4 BackgroundColor,
	u32 OutlineWidth,
	v4 OutlineColor)
{
	rect2 TextRc = PrintTextInternal(GUIState, GUIPrintText_GetSize, Text, Pos, FontScale);

	v4 CurTextColor = TextColor;
	if (MouseInRect(GUIState->Input, TextRc)) {
		CurTextColor = TextHighlightColor;
	}

	RENDERPushRect(GUIState->RenderStack, TextRc, BackgroundColor);

	if (OutlineWidth > 0) {
		RENDERPushRectOutline(GUIState->RenderStack, TextRc, OutlineWidth, OutlineColor);
	}

	PrintTextInternal(GUIState, GUIPrintText_Print, Text, Pos, FontScale, CurTextColor);

	return(TextRc);
}

void GUILabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, GUIPrintText_Print,
		LabelText, At,
		GUIState->FontScale * 0.7f,
		GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));
}

void GUITooltip(gui_state* GUIState, char* TooltipText) {
	if (GUIState->TooltipCount < GUI_TOOLTIPS_MAX_COUNT) {
		CopyStrings(GUIState->Tooltips[GUIState->TooltipCount++], TooltipText);
	}
}

void GUIAnchor(gui_state* GUIState, char* Name, v2 Pos, v2 Dim, gui_interaction* Interaction, b32 Centered) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction, 1);

	if (GUIElementShouldBeUpdated(Element)) {

#if 0
		//NOTE(dima): Not centered
		rect2 WorkRect = Rect2MinDim(Pos, Dim);
#else
		//NOTE(dima) Centered
		v2 MinP = {};
		if (Centered) {
			MinP = Pos - Dim * 0.5f;
		}
		else {
			MinP = Pos;
		}

		rect2 WorkRect = Rect2MinDim(MinP, Dim);

#endif

#if 1
		v4 WorkColor = GUIGetColor(GUIState, GUIState->ColorTheme.AnchorColor);
#else
		v4 WorkColor = GUIGetColor(GUIState, Color_Red);
#endif
		v2 MouseP = GUIState->Input->MouseP;

		gui_element* Anchor = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Anchor->Cache;

		if (!Cache->IsInitialized) {
			Cache->Anchor.OffsetInAnchor = {};

			Cache->IsInitialized = true;
		}
		v2* OffsetInAnchor = &Cache->Anchor.OffsetInAnchor;


		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);
		if (MouseInRect(GUIState->Input, WorkRect)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				IsHot = GUISetInteractionHot(GUIState, Interaction, true);
				*OffsetInAnchor = MouseP - Pos;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left)) {
			IsHot = GUISetInteractionHot(GUIState, Interaction, false);
			*OffsetInAnchor = {};
		}

		if (IsHot) {

			/*Getting true position*/
			MouseP = MouseP - *OffsetInAnchor;
			

			float MinDimLength = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;

			Assert((Interaction->Type == GUIInteraction_Move) ||
				(Interaction->Type == GUIInteraction_Resize));

			if (Interaction->Type == GUIInteraction_Resize) {
				Interaction->ResizeContext.MinDim = V2(MinDimLength, MinDimLength);
				Interaction->ResizeContext.OffsetInAnchor = *OffsetInAnchor;
			}
			else if (Interaction->Type == GUIInteraction_Move) {
				Interaction->MoveContext.OffsetInAnchor = *OffsetInAnchor;
			}

			GUIPerformInteraction(GUIState, Interaction);
		}


		RENDERPushRect(GUIState->RenderStack, WorkRect, WorkColor);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIImageView(gui_state* GUIState, char* Name, bitmap_info* Buffer) {
	GUITreeBegin(GUIState, Name);


	if (Buffer) {
		gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 1);
		if (GUIElementShouldBeUpdated(Element)) {
			//bitmap_info* Buffer = Interaction->VariableLink.Value_RGBABuffer;


			gui_element* ImageView = GUIGetCurrentElement(GUIState);
			gui_element_cache* Cache = &ImageView->Cache;

			if (!Cache->IsInitialized) {
				Cache->Dimensional.Dim = V2(
					(float)Buffer->Width /
					(float)Buffer->Height * 100, 100);
				Cache->IsInitialized = true;
			}
			v2* WorkDim = &Cache->Dimensional.Dim;
			int OutlineWidth = 3;

			gui_layout* View = GUIGetCurrentLayout(GUIState);

			GUIPreAdvanceCursor(GUIState);

			rect2 ImageRect;
			ImageRect.Min = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);
			ImageRect.Max = ImageRect.Min + *WorkDim;

			RENDERPushBitmap(GUIState->RenderStack, Buffer, ImageRect.Min, GetRectHeight(ImageRect));
			RENDERPushRectOutline(GUIState->RenderStack, ImageRect, OutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

			gui_interaction ResizeInteraction = GUIResizeInteraction(ImageRect.Min, WorkDim, GUIResizeInteraction_Proportional);
			GUIAnchor(GUIState, "Anchor0", ImageRect.Max, V2(10, 10), &ResizeInteraction);
			GUIAnchor(GUIState, "Anchor1", ImageRect.Min + V2(0, WorkDim->y), V2(WorkDim->x, OutlineWidth), &ResizeInteraction, 0);

			GUIDescribeElement(GUIState, GetRectDim(ImageRect), ImageRect.Min);
			GUIAdvanceCursor(GUIState);
		}
		GUIEndElement(GUIState, GUIElement_CachedItem);
	}
	else {
		GUIText(GUIState, "NULL");
	}

	GUITreeEnd(GUIState);
}

void GUIStackedMemGraph(gui_state* GUIState, char* Name, stacked_memory* MemoryStack) {
	if (MemoryStack) {
		gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 1);
		if (GUIElementShouldBeUpdated(Element)) {
			gui_layout* View = GUIGetCurrentLayout(GUIState);

			GUIPreAdvanceCursor(GUIState);

			gui_element* StackedMem = GUIGetCurrentElement(GUIState);
			gui_element_cache* Cache = &StackedMem->Cache;

			if (!Cache->IsInitialized) {

				Cache->Dimensional.Dim =
					V2(GUIState->FontInfo->AscenderHeight * 40.0f,
						GUIState->FontInfo->AscenderHeight * 5.0f);

				Cache->IsInitialized = true;
			}

			v2* WorkDim = &Cache->Dimensional.Dim;

			float AscByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;

			rect2 GraphRect;
			GraphRect.Min.x = View->CurrentX;
			GraphRect.Min.y = View->CurrentY - AscByScale;
			GraphRect.Max = GraphRect.Min + *WorkDim;

			v2 GraphRectDim = GetRectDim(GraphRect);

			u64 OccupiedCount = MemoryStack->Used;
			u64 FreeCount = (u64)MemoryStack->MaxSize - OccupiedCount;
			u64 TotalCount = MemoryStack->MaxSize;

			float OccupiedPercentage = (float)OccupiedCount / (float)TotalCount;

			rect2 OccupiedRect = Rect2MinDim(GraphRect.Min, V2(GraphRectDim.x * OccupiedPercentage, GraphRectDim.y));
			rect2 FreeRect = Rect2MinDim(
				V2(OccupiedRect.Max.x, OccupiedRect.Min.y),
				V2(GraphRectDim.x * (1.0f - OccupiedPercentage), GraphRectDim.y));

			float Inner = 2.0f;
			float Outer = 3.0f;

			RENDERPushRect(GUIState->RenderStack, OccupiedRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor3));
			RENDERPushRectOutline(GUIState->RenderStack, OccupiedRect, Inner, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));
			RENDERPushRect(GUIState->RenderStack, FreeRect, V4(GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor).xyz, GUIState->ColorTheme.GraphAlpha));

			RENDERPushRectOutline(GUIState->RenderStack, GraphRect, Outer, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

			v4 TextHighlighColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor);

			gui_interaction NullInteraction = GUINullInteraction();
			GUITextBase(GUIState, Name, V2(View->CurrentX, View->CurrentY),
				TextHighlighColor,
				GUIState->FontScale,
				TextHighlighColor,
				GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
				2, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

			if (MouseInRect(GUIState->Input, GraphRect)) {
				char InfoStr[128];
				stbsp_sprintf(
					InfoStr,
					"Occupied: %llu(%.2f%%); Total: %llu; Fragmentation: %.2f%%",
					OccupiedCount,
					(float)OccupiedCount / (float)TotalCount * 100.0f,
					TotalCount, 
					(float)MemoryStack->FragmentationBytesCount / (float)TotalCount);

				GUITooltip(GUIState, InfoStr);
			}

			gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, WorkDim, GUIResizeInteraction_Default);
			GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(10, 10), &ResizeInteraction);

			GUIDescribeElement(
				GUIState,
				V2(GraphRectDim.x + Outer, GraphRectDim.y + (2.0f * Outer)),
				GraphRect.Min);

			GUIAdvanceCursor(GUIState);
		}

		GUIEndElement(GUIState, GUIElement_CachedItem);
	}
	else {
		GUIText(GUIState, "NULL");
	}
}

void GUIText(gui_state* GUIState, char* Text) {
	FUNCTION_TIMING();

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_StaticItem, Text, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* View = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(
			GUIState,
			GUIPrintText_Print,
			Text,
			V2(View->CurrentX, View->CurrentY),
			GUIState->FontScale, GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		GUIDescribeElement(GUIState, GetRectDim(Rc), V2(View->CurrentX, View->CurrentY - GUIState->FontScale * GUIState->FontInfo->AscenderHeight));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

static void GUIValueView(gui_state* GUIState, gui_variable_link* Link, char* Name, float ViewMultiplier) {
	FUNCTION_TIMING();

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_StaticItem, "", 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* View = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		char ValueBuf[32];

		if (Link->Type == GUIVarType_F32) {
			stbsp_sprintf(ValueBuf, "%.2f", *Link->Value_F32);
		}
		else if (Link->Type == GUIVarType_B32) {
			if (*Link->Value_B32) {
				stbsp_sprintf(ValueBuf, "%s", "true");
			}
			else {
				stbsp_sprintf(ValueBuf, "%s", "false");
			}
		}
		else if (Link->Type == GUIVarType_U32) {
			stbsp_sprintf(ValueBuf, "%u", *Link->Value_U32);
		}
		else if (Link->Type == GUIVarType_I32) {
			stbsp_sprintf(ValueBuf, "%d", *Link->Value_I32);
		}
		else if (Link->Type == GUIVarType_STR) {
			stbsp_sprintf(ValueBuf, "%s", Link->Value_STR);
		}

		char Buf[64];
		if (Name) {
			stbsp_sprintf(Buf, "%s: %s", Name, ValueBuf);
		}
		else {
			stbsp_sprintf(Buf, "%s", ValueBuf);
		}

#if 0
		if (!Element->Cache.IsInitialized ||
			GUIState->TextElemsCacheShouldBeReinitialized) 
		{
			rect2 TxtSizeRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Buf, {}, GUIState->FontScale);
			//NOTE(Dima): Using cache for button because of the same purpose
			Element->Cache.Button.ButtonRectDim = GetRectDim(TxtSizeRc);
			Element->Cache.IsInitialized = true;
		}
		v2* TxtDim = &Element->Cache.Button.ButtonRectDim;
#else
		//NOTE(Dima): Using cache for button because of the same purpose
		v2 TxtDim_ = GUIGetTextSize(GUIState, Buf, GUIState->FontScale);
		v2* TxtDim = &TxtDim_;
#endif

		rect2 WorkRect = Rect2MinDim(
			V2(View->CurrentX, View->CurrentY - GUIState->FontScale * GUIState->FontInfo->AscenderHeight),
			V2(GUIState->FontInfo->AscenderHeight* GUIState->FontScale * ViewMultiplier, TxtDim->y));

		v2 WorkRectDim = GetRectDim(WorkRect);

		RENDERPushRect(GUIState->RenderStack, WorkRect, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor));
		RENDERPushRectOutline(GUIState->RenderStack, WorkRect, 1, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		PrintTextInternal(
			GUIState,
			GUIPrintText_Print,
			Buf,
			V2(WorkRect.Min.x + WorkRectDim.x * 0.5f - TxtDim->x * 0.5f, View->CurrentY),
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		GUIDescribeElement(GUIState, WorkRectDim, WorkRect.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void ColorRectView(gui_state* GUIState, v4 Color) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_StaticItem, "", 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* View = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 WorkRect = Rect2MinDim(
			V2(View->CurrentX, View->CurrentY - GUIState->FontScale * GUIState->FontInfo->AscenderHeight),
			V2(GUIState->FontInfo->AscenderHeight * GUI_VALUE_COLOR_VIEW_MULTIPLIER, GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale));

		RENDERPushRect(GUIState->RenderStack, WorkRect, Color);
		RENDERPushRectOutline(
			GUIState->RenderStack, WorkRect, 1,
			GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		GUIDescribeElement(GUIState, GetRectDim(WorkRect), WorkRect.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIInt32View(gui_state* GUIState, i32 Int, char* Name) {
	GUIBeginRow(GUIState);

	if (Name) {
		GUIText(GUIState, Name);
	}

	gui_variable_link Link1 = GUIVariableLink(&Int, GUIVarType_I32);

	GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);

	GUIEndRow(GUIState);
}

void ColorView(gui_state* GUIState, v4 Color, char* Name) {
	FUNCTION_TIMING();
	
	gui_element* Elem = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 0);

	if (GUIElementShouldBeUpdated(Elem)) {
		GUIBeginRow(GUIState);

		if (Name) {
			GUIText(GUIState, Name);
		}

		int ValR = (int)(Color.r * 255.0f);
		int ValG = (int)(Color.g * 255.0f);
		int ValB = (int)(Color.b * 255.0f);

		char HexViewBuf[8];
		stbsp_sprintf(HexViewBuf, "#%02.X%02.X%02.X", (u8)ValR, (u8)ValG, (u8)ValB);

		gui_variable_link Link1 = GUIVariableLink(&ValR, GUIVarType_I32);
		gui_variable_link Link2 = GUIVariableLink(&ValG, GUIVarType_I32);
		gui_variable_link Link3 = GUIVariableLink(&ValB, GUIVarType_I32);
		gui_variable_link Link4 = GUIVariableLink(HexViewBuf, GUIVarType_STR);

		GUIValueView(GUIState, &Link1, "R", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link2, "G", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link3, "B", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link4, 0, GUI_VALUE_VIEW_MULTIPLIER);

		ColorRectView(GUIState, Color);

		GUIEndRow(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIVector2View(gui_state* GUIState, v2 Value, char* Name) {
	gui_element* Elem = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 0);

	if (GUIElementShouldBeUpdated(Elem)) {

		GUIBeginRow(GUIState);

		if (Name) {
			GUIText(GUIState, Name);
		}

		gui_variable_link Link1 = GUIVariableLink(&Value.x, GUIVarType_F32);
		gui_variable_link Link2 = GUIVariableLink(&Value.y, GUIVarType_F32);

		GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);

		GUIEndRow(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIVector3View(gui_state* GUIState, v3 Value, char* Name) {
	gui_element* Elem = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 0);

	if (GUIElementShouldBeUpdated(Elem)) {
		GUIBeginRow(GUIState);

		if (Name) {
			GUIText(GUIState, Name);
		}

		gui_variable_link Link1 = GUIVariableLink(&Value.x, GUIVarType_F32);
		gui_variable_link Link2 = GUIVariableLink(&Value.y, GUIVarType_F32);
		gui_variable_link Link3 = GUIVariableLink(&Value.z, GUIVarType_F32);

		GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link3, 0, GUI_VALUE_VIEW_MULTIPLIER);

		GUIEndRow(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIVector4View(gui_state* GUIState, v4 Value, char* Name) {
	gui_element* Elem = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1, 0);

	if (GUIElementShouldBeUpdated(Elem)) {
		GUIBeginRow(GUIState);

		if (Name) {
			GUIText(GUIState, Name);
		}

		gui_variable_link Link1 = GUIVariableLink(&Value.r, GUIVarType_F32);
		gui_variable_link Link2 = GUIVariableLink(&Value.g, GUIVarType_F32);
		gui_variable_link Link3 = GUIVariableLink(&Value.b, GUIVarType_F32);
		gui_variable_link Link4 = GUIVariableLink(&Value.a, GUIVarType_F32);

		GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link3, 0, GUI_VALUE_VIEW_MULTIPLIER);
		GUIValueView(GUIState, &Link4, 0, GUI_VALUE_VIEW_MULTIPLIER);

		GUIEndRow(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Text, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* View = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, GUIPrintText_GetSize, Text, V2(View->CurrentX, View->CurrentY), GUIState->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);

		if (MouseInRect(GUIState->Input, Rc)) {
			TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIPerformInteraction(GUIState, Interaction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(GUIState->RenderStack, Rc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, Interaction);
			}
		}

		PrintTextInternal(GUIState, GUIPrintText_Print, Text, V2(View->CurrentX, View->CurrentY), GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElement(GUIState, GetRectDim(Rc), Rc.Min);

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

b32 GUIButton(gui_state* GUIState, char* ButtonName) {
	b32 Result = 0;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 ButRc = GUITextBase(GUIState, ButtonName, V2(Layout->CurrentX, Layout->CurrentY),
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor),
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor),
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));


		if (MouseInRect(GUIState->Input, ButRc)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				Result = 1;
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(GUIState->RenderStack, ButRc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				Result = 1;
			}
		}

		GUIDescribeElement(GUIState, GetRectDim(ButRc), ButRc.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);

	return(Result);
}

b32 GUIButtonAt(gui_state* GUIState, char* ButtonName, v2 At, rect2* ButtonRect, v4* TextColor) {

	b32 Result = 0;

	v4 ButtonTextColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor);

	if (TextColor) {
		ButtonTextColor = *TextColor;
	}

	gui_interaction Interaction = GUIReturnMouseActionInteraction(
		GUIState->Input,
		&Result,
		GUIReturnMouseAction_IsDown,
		MouseButton_Left);

	rect2 ButRc = GUITextBase(GUIState, ButtonName, V2(At.x, At.y),
		ButtonTextColor,
		GUIState->FontScale,
		GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor),
		GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
		2, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

	if (MouseInRect(GUIState->Input, ButRc)) {
		if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
			Result = 1;
		}
	}

	if (ButtonRect) {
		*ButtonRect = ButRc;
	}

	return(Result);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, b32* Value) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {

		gui_layout* View = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		if (!Element->Cache.IsInitialized ||
			GUIState->TextElemsCacheShouldBeReinitialized)
		{


			Element->Cache.IsInitialized = true;
		}

		rect2 NameRect = PrintTextInternal(
			GUIState,
			GUIPrintText_Print,
			ButtonName,
			V2(View->CurrentX, View->CurrentY),
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));
		v2 NameDim = GetRectDim(NameRect);

		v2 ButDim = GUIGetTextSize(GUIState, "false", GUIState->FontScale);
		v2 TrueDim = GUIGetTextSize(GUIState, "true", GUIState->FontScale);

		float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float PrintButY = View->CurrentY;

		rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), ButDim);
		rect2 NameRc = Rect2MinDim(V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), NameDim);

		float OutlineWidth = 1;

		gui_variable_link BoolLink = GUIVariableLink(Value, GUIVarType_B32);
		gui_interaction BoolInteraction = GUIBoolInteraction(Value);

		char TextToPrint[8];
		if (Value) {
			if (*Value) {

				PrintButX = ButRc.Min.x + (ButDim.x - TrueDim.x) * 0.5f;

				stbsp_sprintf(TextToPrint, "%s", "true");
			}
			else {
				stbsp_sprintf(TextToPrint, "%s", "false");
			}
		}
		else {
			stbsp_sprintf(TextToPrint, "%s", "NULL");
		}

		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		if (Value) {
			if (MouseInRect(GUIState->Input, ButRc)) {
				TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
				if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
					*Value = !(*Value);
				}
			}

			if (GUIWalkaroundIsHere(GUIState)) {
				RENDERPushRectOutline(
					GUIState->RenderStack,
					NameRc,
					2,
					GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

				if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
					*Value = !(*Value);
				}
			}
		}

		RENDERPushRect(GUIState->RenderStack, ButRc, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor1));
		RENDERPushRectOutline(GUIState->RenderStack, ButRc, OutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		PrintTextInternal(GUIState, GUIPrintText_Print, TextToPrint, V2(PrintButX, PrintButY), GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElement(GUIState, V2(ButRc.Max.x - View->CurrentX, ButRc.Max.y - ButRc.Min.y + OutlineWidth), V2(View->CurrentX, ButRc.Min.y));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIBoolButton2(gui_state* GUIState, char* ButtonName, b32* Value) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		GUIPreAdvanceCursor(GUIState);

		gui_interaction BoolInteraction = GUIBoolInteraction(Value);

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		v4 TextColor = *Value ? 
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor) :
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor);

		rect2 ButRc = GUITextBase(GUIState, ButtonName, V2(Layout->CurrentX, Layout->CurrentY),
			TextColor, GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor2), 
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

		if (MouseInRect(GUIState->Input, ButRc)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIPerformInteraction(GUIState, &BoolInteraction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(GUIState->RenderStack, ButRc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));
		}
		
		GUIDescribeElement(GUIState, GetRectDim(ButRc), ButRc.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIBoolChecker(gui_state* GUIState, char* Name, b32* Value) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		GUIPreAdvanceCursor(GUIState);

		gui_interaction BoolInteraction = GUIBoolInteraction(Value);

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);


		float AscByScale = GUIState->FontScale * GUIState->FontInfo->AscenderHeight;
		float RowAdvance = GetNextRowAdvance(GUIState->FontInfo);
		rect2 CheckerRc = Rect2MinDim(V2(Layout->CurrentX, Layout->CurrentY - AscByScale), V2(RowAdvance, RowAdvance));
		v2 CheckerDim = GetRectDim(CheckerRc);
		v2 Delta = V2(1.0f, 1.0f);

		rect2 InnerRc = Rect2MinMax(CheckerRc.Min + Delta, CheckerRc.Max - Delta);

		RENDERPushRect(GUIState->RenderStack, CheckerRc, GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor1));
		RENDERPushRectOutline(GUIState->RenderStack, CheckerRc, 1, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		if (MouseInRect(GUIState->Input, CheckerRc)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIPerformInteraction(GUIState, &BoolInteraction);
			}
		}

		if (*Value) {
			RENDERPushRect(GUIState->RenderStack, InnerRc, GUIGetColor(GUIState, GUIState->ColorTheme.GraphBackColor));
		}

		rect2 TextRc = GUIPrintText(
			GUIState, Name, 
			V2(CheckerRc.Max.x + AscByScale, Layout->CurrentY), 
			GUIState->FontScale, 
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		if (GUIWalkaroundIsHere(GUIState)) {
			v4 OutlineColor = GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor);
			RENDERPushRectOutline(GUIState->RenderStack, CheckerRc, 2, OutlineColor);
			RENDERPushRectOutline(GUIState->RenderStack, TextRc, 2, OutlineColor);

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, &BoolInteraction);
			}
		}

		rect2 RowRc = Rect2MinMax(CheckerRc.Min, TextRc.Max);

		GUIDescribeElement(GUIState, GetRectDim(RowRc), RowRc.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, float* InteractValue) {
	gui_interaction Interaction_ = GUIVariableInteraction(InteractValue, GUIVarType_F32);
	gui_interaction* Interaction = &Interaction_;

	gui_element* Element = GUIBeginElement(State, GUIElement_InteractibleItem, Name, Interaction, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* View = GUIGetCurrentLayout(State);
		float FontScale = State->FontScale;
		float SmallTextScale = FontScale * 0.8f;
		float NextRowAdvanceFull = GetNextRowAdvance(State->FontInfo, FontScale);
		float NextRowAdvanceSmall = GetNextRowAdvance(State->FontInfo, SmallTextScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull, NextRowAdvanceFull * 5);

		GUIPreAdvanceCursor(State);

		float* Value = Interaction->VariableLink.Value_F32;
		Assert(Max > Min);

		//NOTE(DIMA): Calculating Max text rectangle
		//TODO(DIMA): Cache theese calculations
		char MaxValueTxt[16];
		stbsp_sprintf(MaxValueTxt, "%.2f", Max);
		v2 MaxValueRcMin = V2(View->CurrentX, View->CurrentY);
		rect2 MaxValueRcSize = PrintTextInternal(
			State,
			GUIPrintText_GetSize,
			MaxValueTxt,
			{},
			SmallTextScale);
		v2 MaxValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Calculating vertical rectangle
		v2 WorkRectMin = V2(View->CurrentX, MaxValueRcMin.y - State->FontInfo->DescenderHeight * SmallTextScale);
		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);

		//NOTE(DIMA): Drawing vertical rectangle
		i32 RectOutlineWidth = 1;

		RENDERPushRect(State->RenderStack, WorkRect, GUIGetColor(State, State->ColorTheme.GraphColor1));
		RENDERPushRectOutline(State->RenderStack, WorkRect, RectOutlineWidth, GUIGetColor(State, State->ColorTheme.OutlineColor));

		//NOTE(DIMA): Calculating Min text rectangle
		//TODO(DIMA): Cache theese calculations
		char MinValueTxt[16];
		stbsp_sprintf(MinValueTxt, "%.2f", Min);
		v2 MinValueRcMin = V2(WorkRect.Min.x, WorkRect.Max.y + NextRowAdvanceSmall);
		rect2 MinValueRcSize = PrintTextInternal(
			State,
			GUIPrintText_GetSize,
			MinValueTxt,
			{},
			SmallTextScale);
		v2 MinValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Drawing Max value text
		rect2 MaxValueRc = PrintTextInternal(
			State,
			GUIPrintText_Print,
			MaxValueTxt,
			V2(MaxValueRcMin.x + 0.5f * (WorkRectDim.x - MaxValueRcDim.x),
			MaxValueRcMin.y),
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(DIMA): Printing Min value text
		rect2 MinValueRc = PrintTextInternal(
			State,
			GUIPrintText_Print,
			MinValueTxt,
			V2(MinValueRcMin.x + 0.5f * (WorkRectDim.x - MinValueRcDim.x),
			MinValueRcMin.y),
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(DIMA): Printing name of the element that consist from first 3 chars of the name
		char SmallTextToPrint[8];
		stbsp_sprintf(SmallTextToPrint, "%.4s", Name);
		char *SmallBufAt = SmallTextToPrint;
		while (*SmallBufAt) {
			char CurChar = *SmallBufAt;

			if (CurChar >= 'a' && CurChar <= 'z') {
				*SmallBufAt = CurChar - 'a' + 'A';
			}

			SmallBufAt++;
		}
		rect2 SmallTextRect = PrintTextInternal(State, GUIPrintText_GetSize, SmallTextToPrint, {}, SmallTextScale);
		v2 SmallTextRcDim = GetRectDim(SmallTextRect);

		float SmallTextX = WorkRect.Min.x + WorkRectDim.x * 0.5f - SmallTextRcDim.x * 0.5f;
		float SmallTextY = MinValueRc.Min.y + MinValueRcDim.y + State->FontInfo->AscenderHeight * SmallTextScale;

		rect2 SmallTextRc = PrintTextInternal(
			State,
			GUIPrintText_Print,
			SmallTextToPrint,
			V2(SmallTextX,
			SmallTextY),
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(Dima): Processing the value
		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = 1.0f - (((float)(*Value) - Min) / (float)Range);

		float CursorWidth = WorkRectDim.x;
		float CursorHeight = CursorWidth * 0.66f;

		float CursorX = WorkRectMin.x - (CursorWidth - WorkRectDim.x) * 0.5f;
		float CursorY = WorkRectMin.y + (WorkRectDim.y - CursorHeight) * RelativePos01;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);

		float MaxWidth = Max(Max(Max(WorkRectDim.x, MaxValueRcDim.x), MinValueRcDim.x), SmallTextRcDim.x);


		//NOTE(DIMA): Processing interactions
		v4 CursorColor = GUIGetColor(State, State->ColorTheme.GraphColor2);
		b32 IsHot = GUIInteractionIsHot(State, Interaction);
		b32 MouseInWorkRect = MouseInRect(State->Input, WorkRect);
		b32 MouseInCursRect = MouseInRect(State->Input, CursorRect);
		if (MouseInWorkRect || MouseInCursRect) {
			if (MouseButtonWentDown(State->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(State, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseInCursRect && !IsHot) {
			char ValStr[64];
			stbsp_sprintf(ValStr, "%.3f", *Value);

			GUITooltip(State, ValStr);
		}

		if (MouseButtonWentUp(State->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(State, Interaction, false);
			IsHot = false;
		}

		if (GUIWalkaroundIsHere(State)) {

			if (ButtonWentDown(State->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(State);
			}

			if (GUIWalkaroundIsHot(State)) {
				RENDERPushRectOutline(State->RenderStack, WorkRect, 2, GUIGetColor(State, State->ColorTheme.WalkaroundHotColor));

				float ChangeStep = 0.02f * Range;
				if (ButtonIsDown(State->Input, KeyType_Down)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(State->Input, KeyType_Up)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				char ValStr[16];
				stbsp_sprintf(ValStr, "%.3f", *Value);
				GUILabel(State, ValStr, CursorRect.Max);
			}
			else {
				//RENDERPushRectOutline(State->RenderStack, Rect2MinDim(MaxValueRcMin, V2(MaxWidth, SmallTextRc.Max.y - MaxValueRc.Min.y)));
				RENDERPushRectOutline(State->RenderStack, SmallTextRc, 2, GUIGetColor(State, State->ColorTheme.WalkaroundColor));
				RENDERPushRectOutline(State->RenderStack, WorkRect, 2, GUIGetColor(State, State->ColorTheme.WalkaroundColor));
			}
		}

		if (IsHot) {
			CursorColor = GUIGetColor(State, State->ColorTheme.WalkaroundHotColor);

			v2 InteractMouseP = State->Input->MouseP;
			if (InteractMouseP.y > (WorkRect.Min.y - 0.5f * CursorHeight)) {
				*Value = Max;
			}

			if (InteractMouseP.y < (WorkRect.Max.y + 0.5f * CursorHeight)) {
				*Value = Min;
			}

			float AT = InteractMouseP.y - (WorkRect.Min.y + 0.5f * CursorHeight);
			AT = Clamp(AT, 0.0f, WorkRectDim.y - CursorHeight);
			float NewVal01 = 1.0f - (AT / (WorkRectDim.y - CursorHeight));
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;

			char ValStr[16];
			stbsp_sprintf(ValStr, "%.3f", *Value);
			GUILabel(State, ValStr, CursorRect.Max);
		}

		//NOTE(DIMA): Drawing cursor
		RENDERPushRect(State->RenderStack, CursorRect, CursorColor);
		RENDERPushRectOutline(State->RenderStack, CursorRect, 2, GUIGetColor(State, State->ColorTheme.OutlineColor));

		//NOTE(DIMA): Postprocessing
		//TODO(DIMA): Get correct begin position
		GUIDescribeElement(
			State,
			V2(MaxWidth, SmallTextRc.Max.y - MaxValueRc.Min.y),
			V2(View->CurrentX, SmallTextScale * State->FontInfo->AscenderHeight));

		GUIAdvanceCursor(State);
	}

	GUIEndElement(State, GUIElement_InteractibleItem);
}

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, float* InteractValue) {
	gui_interaction Interaction_ = GUIVariableInteraction(InteractValue, GUIVarType_F32);
	gui_interaction* Interaction = &Interaction_;

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		float NextRowAdvanceFull = GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale);

		float* Value = Interaction->VariableLink.Value_F32;

		Assert(Max > Min);

		char ValueBuf[32];
		stbsp_sprintf(ValueBuf, "%.3f", *Value);
		rect2 ValueTextSize = PrintTextInternal(GUIState, GUIPrintText_GetSize, ValueBuf, {}, GUIState->FontScale);

		//NOTE(Dima): Next element to the text is advanced by AscenderHeight
		v2 WorkRectMin = V2(
			Layout->CurrentX,
			Layout->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull * 10, NextRowAdvanceFull);

		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);
		v4 WorkRectColor = GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor1);

		float RectOutlineWidth = 1.0f;

		RENDERPushRect(GUIState->RenderStack, WorkRect, WorkRectColor);
		RENDERPushRectOutline(GUIState->RenderStack, WorkRect, RectOutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		v2 NameTextDim = GUIGetTextSize(GUIState, Name, GUIState->FontScale);
		rect2 NameTextSize = GUIPrintText(
			GUIState, 
			Name, 
			V2(WorkRect.Max.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale, Layout->CurrentY), 
			GUIState->FontScale, 
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = ((float)(*Value) - Min) / (float)Range;

		float CursorHeight = WorkRectDim.y;
		float CursorWidth = CursorHeight * 0.75f;

		float CursorX = WorkRectMin.x + (WorkRectDim.x - CursorWidth) * RelativePos01;
		float CursorY = WorkRectMin.y - (CursorHeight - WorkRectDim.y) * 0.5f;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);
		v4 CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.GraphColor2);

		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);

		if (MouseInRect(GUIState->Input, CursorRect) || MouseInRect(GUIState->Input, WorkRect)) {


			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(GUIState, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(GUIState, Interaction, false);
			IsHot = false;
		}

		if (IsHot) {
			CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundHotColor);

			v2 InteractMouseP = GUIState->Input->MouseP;
#if 0
			if (InteractMouseP.x > (WorkRect.Max.x - 0.5f * CursorWidth)) {
				*Value = Max;
				InteractMouseP.x = (WorkRect.Max.x - 0.5f * CursorWidth);
			}

			if (InteractMouseP.x < (WorkRect.Min.x + 0.5f * CursorWidth)) {
				*Value = Min;
				InteractMouseP.x = (WorkRect.Min.x + 0.5f * CursorWidth);
			}
#endif

			float AT = InteractMouseP.x - (WorkRect.Min.x + 0.5f * CursorWidth);
			AT = Clamp(AT, 0.0f, WorkRectDim.x - CursorWidth);
			float NewVal01 = AT / (WorkRectDim.x - CursorWidth);
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(GUIState);
			}

			if (GUIState->WalkaroundIsHot) {
				RENDERPushRectOutline(GUIState->RenderStack, WorkRect, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundHotColor));

				CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundHotColor);

				float ChangeStep = Range * 0.02f;
				if (ButtonIsDown(GUIState->Input, KeyType_Right)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(GUIState->Input, KeyType_Left)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}
			}
			else {
				RENDERPushRectOutline(GUIState->RenderStack, NameTextSize, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));
			}
		}

		RENDERPushRect(GUIState->RenderStack, CursorRect, CursorColor);
		RENDERPushRectOutline(GUIState->RenderStack, CursorRect, 2, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		float ValueTextY = WorkRectMin.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float ValueTextX = WorkRectMin.x + WorkRectDim.x * 0.5f - (ValueTextSize.Max.x - ValueTextSize.Min.x) * 0.5f;
		PrintTextInternal(GUIState, GUIPrintText_Print, ValueBuf, V2(ValueTextX, ValueTextY), GUIState->FontScale);

#if 0
		char TextBuf[64];
		stbsp_snprintf(TextBuf, sizeof(TextBuf), "Min: %.3f; Max: %.3f;", Min, Max);

		float DrawTextX = View->CurrentX + WorkRectMin.x + WorkRectDim.x + 10;
		PrintTextInternal(GUIState, PrintTextType_PrintText, TextBuf, DrawTextX, View->CurrentY, View->FontScale);
#endif

		GUIDescribeElement(
			GUIState,
			V2(NameTextSize.Max.x - Layout->CurrentX,
				WorkRect.Max.y - WorkRect.Min.y + (2.0f * RectOutlineWidth)),
			V2(Layout->CurrentX, WorkRect.Min.y));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIChangeTreeNodeText(gui_state* GUIState, char* Text) {
	gui_element* Elem = GUIGetCurrentElement(GUIState);

	if (Elem->Type == GUIElement_TreeNode) {
		CopyStrings(Elem->Text, Text);
	}
}

void GUITreeBegin(gui_state* GUIState, char* NodeName, char* NameText) {
	gui_layout* View = GUIGetCurrentLayout(GUIState);

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_TreeNode, NodeName, 0, 0);
	gui_tree_node_cache* Cache = &Element->Cache.TreeNode;

	//gui_interaction ActionInteraction = GUIVariableInteraction(&State->CurrentNode->Expanded, GUIVarType_B32);
	char TextBuf[128];
	char *ToWriteName = (NameText == 0) ? Element->Name : NameText;
	if (GUIState->PlusMinusSymbol) {
		stbsp_sprintf(TextBuf, "%c %s", Element->Expanded ? '+' : '-', ToWriteName);
	}
	else {
		stbsp_sprintf(TextBuf, "%s", ToWriteName);
	}
	
	Cache->OutFlags = 0;

	if (GUIElementShouldBeUpdated(Element)) {
		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, GUIPrintText_GetSize, TextBuf, V2(View->CurrentX, View->CurrentY), GUIState->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		gui_interaction TreeInteraction = GUITreeInteraction(Element, Rc);

		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		if (MouseInRect(GUIState->Input, Rc)) {
			Cache->OutFlags |= GUITreeOutFlag_MouseOverRect;

			TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				Cache->OutFlags |= GUITreeOutFlag_MouseLeftClickedInRect;

				GUIPerformInteraction(GUIState, &TreeInteraction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(
				GUIState->RenderStack,
				Rc, 2,
				GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, &TreeInteraction);
			}
		}

		RENDERPushRect(GUIState->RenderStack, Rc, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor));
		RENDERPushRectOutline(GUIState->RenderStack, Rc, 1, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));
		PrintTextInternal(GUIState, GUIPrintText_Print, TextBuf, V2(View->CurrentX, View->CurrentY), GUIState->FontScale, TextHighlightColor);

		GUIDescribeElement(GUIState, GetRectDim(Rc), Rc.Min);
		GUIAdvanceCursor(GUIState);
	}
	Element->Cache.TreeNode.StackBeginY = View->CurrentY;
}

u32 GUITreeGetOutFlags(gui_state* GUIState) {
	gui_element* Element = GUIGetCurrentElement(GUIState);

	Assert(Element->Type == GUIElement_TreeNode);

	u32 Result = Element->Cache.TreeNode.OutFlags;

	return(Result);
}

void GUITreeEnd(gui_state* State) {
	gui_element* Elem = GUIGetCurrentElement(State);

	gui_layout* View = GUIGetCurrentLayout(State);

	gui_tree_node_cache* Cache = &Elem->Cache.TreeNode;
	Cache->OutFlags = 0;

	if ((Elem->Expanded == false) &&
		(Cache->ExitState == GUITreeNodeExit_None))
	{
		Cache->ExitY = Cache->StackY - Cache->StackBeginY;
		//Cache->ExitY = 100;
		Cache->ExitState = GUITreeNodeExit_Exiting;
	}

	if (Cache->ExitState == GUITreeNodeExit_Exiting) {
		float ExitSpeed = (Cache->ExitY / 8.0f + 4) * State->Input->DeltaTime * 65.0f;
		Cache->ExitY -= ExitSpeed;

		if (Cache->ExitY < 0.0f) {
			Cache->ExitY = 0.0f;
			Cache->ExitState = GUITreeNodeExit_Finished;
		}
		else {
			GUIPreAdvanceCursor(State);
			GUIDescribeElement(State, V2(0, Cache->ExitY), V2(View->CurrentX, View->CurrentY));
			GUIAdvanceCursor(State);
		}
	}

	GUIEndElement(State, GUIElement_TreeNode);
}

void GUIBeginRadioGroup(gui_state* GUIState, char* Name, u32 DefaultSetIndex) {
	gui_layout* Layout = GUIGetCurrentLayout(GUIState);

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_RadioGroup, Name, 0, 1, 0);

	if (!Element->Cache.IsInitialized) {
		Element->Cache.RadioCache.ActiveIndex = DefaultSetIndex;

		Element->Cache.IsInitialized = 1;
	}
}

inline gui_element* GUIFindRadioGroupParent(gui_element* CurrentElement) {
	gui_element* Result = 0;

	gui_element* At = CurrentElement;
	while (At != 0) {
		if (At->Type == GUIElement_RadioGroup) {
			Result = At;
			break;
		}

		At = At->Parent;
	}

	return(Result);
}

void GUIRadioButton(gui_state* GUIState, char* Name, u32 UniqueIndex) {
#if 1
	gui_element* RadioGroup = GUIFindRadioGroupParent(GUIState->CurrentNode);
#else
	gui_element* RadioGroup = GUIState->CurrentNode;
#endif

	gui_interaction RadioButtonInteraction = GUIRadioButtonInteraction(RadioGroup, UniqueIndex);

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, &RadioButtonInteraction, 1, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		if (!Element->Cache.IsInitialized) {

			Element->Cache.RadioButton.IsActive = 0;

			Element->Cache.IsInitialized = 1;
		}

		b32* IsActive = &Element->Cache.RadioButton.IsActive;

		*IsActive = 0;
		if (RadioGroup->Cache.RadioCache.ActiveIndex == UniqueIndex) {
			*IsActive = 1;
		}

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		GUIPreAdvanceCursor(GUIState);

		v4 TextHighColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor);
		v4 TextColor = GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor);

		rect2 Rc = GUITextBase(
			GUIState, Name, V2(Layout->CurrentX, Layout->CurrentY),
			*IsActive ? TextHighColor : TextColor,
			GUIState->FontScale,
			TextHighColor,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

		if (MouseInRect(GUIState->Input, Rc)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIPerformInteraction(GUIState, &RadioButtonInteraction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(GUIState->RenderStack, Rc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, &RadioButtonInteraction);
			}
		}

		GUIDescribeElement(GUIState, GetRectDim(Rc), Rc.Min);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIEndRadioGroup(gui_state* GUIState, u32* ActiveElement) {
	gui_element* CurrentElement = GUIGetCurrentElement(GUIState);
	*ActiveElement = CurrentElement->Cache.RadioCache.ActiveIndex;

	GUIEndElement(GUIState, GUIElement_RadioGroup);
}

void GUIBeginStateChangerGroup(gui_state* GUIState, char* Name, u32 DefaultSetIndex) {
	gui_layout* Layout = GUIGetCurrentLayout(GUIState);

	gui_element* Element = GUIBeginElement(GUIState, GUIElement_StateChangerGroup, Name, 0, 1, 0);

	if (!Element->Cache.IsInitialized) {
		Element->Cache.StateChangerGroupCache.ActiveElement = 0;

		Element->Cache.IsInitialized = 1;
	}
}

inline gui_element* GUIFindStateChangerGroupParent(gui_element* CurrentElement) {
	gui_element* Result = 0;

	gui_element* At = CurrentElement;
	while (At != 0) {
		if (At->Type == GUIElement_StateChangerGroup) {
			Result = At;
			break;
		}

		At = At->Parent;
	}

	return(Result);
}

void GUIStateChanger(gui_state* GUIState, char* Name, u32 StateID) {
	gui_element* StateChangerGroup = GUIFindStateChangerGroupParent(GUIState->CurrentNode);
	
	gui_interaction NullInteraction = GUINullInteraction();
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, &NullInteraction, 1, 0);

	if (GUIElementShouldBeUpdated(Element)) {
		if (!Element->Cache.IsInitialized) {
			Element->Cache.StateChangerCache.StateID = StateID;

			Element->Cache.IsInitialized = 1;
		}

		if (StateChangerGroup->Cache.StateChangerGroupCache.ActiveElement == 0) {
			StateChangerGroup->Cache.StateChangerGroupCache.ActiveElement = Element;
		}
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIEndStateChangerGroupAt(gui_state* GUIState, v2 Pos, u32* ActiveElement) {
	gui_element* CurrentElement = GUIGetCurrentElement(GUIState);

	if (GUIElementShouldBeUpdated(CurrentElement)) {
		//GUIPreAdvanceCursor(GUIState);

		gui_layout* Layout = GUIGetCurrentLayout(GUIState);

		gui_interaction Interaction = GUINullInteraction();

		char* Text = "NULL";
		if (CurrentElement->Cache.StateChangerGroupCache.ActiveElement) {
			Text = CurrentElement->Cache.StateChangerGroupCache.ActiveElement->Name;
			Interaction = GUIStateChangerGroupInteraction(CurrentElement, 0);
		}

		rect2 Rc = GUITextBase(GUIState, Text, Pos,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextColor),
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor),
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

		if (MouseLeftWentDownInRect(GUIState->Input, Rc)) {
			GUIPerformInteraction(GUIState, &Interaction);
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, &Interaction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			RENDERPushRectOutline(GUIState->RenderStack, Rc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIPerformInteraction(GUIState, &Interaction);
			}
		}

		/*
		rect2 Rc = GUITextBase(GUIState, Text, V2(Layout->CurrentX, Layout->CurrentY),
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor),
			GUIState->FontScale, &Interaction,
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonTextHighColor),
			GUIGetColor(GUIState, GUIState->ColorTheme.ButtonBackColor),
			1, GUIGetColor(GUIState, GUIState->ColorTheme.ButtonOutlineColor));

		GUIDescribeElement(GUIState, GetRectDim(Rc), Rc.Min);
		GUIAdvanceCursor(GUIState);
		*/
	}

	if (CurrentElement->Cache.StateChangerGroupCache.ActiveElement) {
		*ActiveElement = CurrentElement->Cache.StateChangerGroupCache.ActiveElement->Cache.StateChangerCache.StateID;
	}
	else {
		*ActiveElement = 0xFFFFFFFF;
	}

	GUIEndElement(GUIState, GUIElement_StateChangerGroup);
}

#if 0
void GUIWindowBegin(gui_state* GUIState, char* Name) {

}

void GUIWindowEnd(gui_state* GUIState) {

}

void GUIWindow(gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0, 1);

	if (GUIElementShouldBeUpdated(Element)) {
		GUIPreAdvanceCursor(GUIState);

		gui_layout* View = GUIGetCurrentLayout(GUIState);

		gui_element* Window = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Window->Cache;

		if (!Cache->IsInitialized) {
			if (CreationFlags & GUIWindow_DefaultSize) {
				Cache->Layout.Dimension.x = 100;
				Cache->Layout.Dimension.y = 100;
			}
			else {
				Cache->Layout.Dimension.x = Width;
				Cache->Layout.Dimension.y = Height;
			}

			Cache->Layout.Position = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

			Cache->IsInitialized = true;
		}

		v2* WindowDimension = &Cache->Layout.Dimension;
		v2* WindowPosition = &Cache->Layout.Position;

		int WindowOutlineWidth = 3;
		int InnerSubWindowWidth = 2;
		rect2 WindowRect = Rect2MinDim(*WindowPosition, *WindowDimension);

		RENDERPushRect(GUIState->RenderStack, WindowRect, GUIGetColor(GUIState, GUIState->ColorTheme.WindowBackgroundColor));
		RENDERPushRectOutline(GUIState->RenderStack, WindowRect, WindowOutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		if (CreationFlags & GUIWindow_TopBar) {
			float TopBarHeight = GUIState->FontScale * GetNextRowAdvance(GUIState->FontInfo);

			if (WindowDimension->y > TopBarHeight) {
				RENDERPushRect(
					GUIState->RenderStack,
					Rect2MinDim(
						*WindowPosition + V2(0, TopBarHeight),
						V2(WindowDimension->x, InnerSubWindowWidth)),
					GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));
			}
		}

		if (CreationFlags & GUIWindow_Resizable) {
			gui_interaction ResizeInteraction = GUIResizeInteraction(WindowRect.Min, WindowDimension, GUIResizeInteraction_Default);
			GUIAnchor(GUIState, "AnchorBR", WindowRect.Max, V2(5, 5), &ResizeInteraction);
		}

		gui_interaction MoveInteraction = GUIMoveInteraction(WindowPosition, GUIMoveInteraction_Move);
		GUIAnchor(GUIState, "AnchorMV", WindowRect.Min, V2(5, 5), &MoveInteraction);

		GUIDescribeElement(GUIState, *WindowDimension, WindowRect.Min);
		GUIAdvanceCursor(GUIState);
	}
	GUIEndElement(GUIState, GUIElement_CachedItem);
}

inline gui_element* GUIFindMenuParentNode(gui_element* CurrentElement) {
	gui_element* Result = 0;

	gui_element* At = CurrentElement->Parent;
	while (At->Parent != 0) {
		if (At->Type == GUIElement_MenuBar ||
			At->Type == GUIElement_MenuItem) 
		{
			Result = At;
			break;
		}

		At = At->Parent;
	}

	return(Result);
}

void GUIBeginMenuBar(gui_state* GUIState, char* MenuName) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_MenuBar, MenuName, 0, 1, 0);
	Element->Cache.MenuNode.ChildrenCount = 0;
	Element->Cache.MenuNode.MaxHeight = 0;
	Element->Cache.MenuNode.MaxWidth = 0;
	GUIBeginRow(GUIState);
}

void GUIEndMenuBar(gui_state* GUIState) {

	GUIEndRow(GUIState);
	GUIEndElement(GUIState, GUIElement_MenuBar);
}

void GUIBeginMenuItemInternal(gui_state* GUIState, char* ItemName, u32 MenuItemType) {
	gui_element* Element = GUIBeginElement(GUIState, GUIElement_MenuItem, ItemName, 0, 0, 0);

	Element->Cache.MenuNode.ChildrenCount = 0;
	Element->Cache.MenuNode.MaxHeight = 0;
	Element->Cache.MenuNode.MaxWidth = 0;
	Element->Cache.MenuNode.SumHeight = 0;
	Element->Cache.MenuNode.SumWidth = 0;
	Element->Cache.MenuNode.Type = MenuItemType;
}

void GUIEndMenuItemInternal(gui_state* GUIState, u32 MenuItemType) {
	gui_element* Element = GUIGetCurrentElement(GUIState);
	Assert(Element->Cache.MenuNode.Type == MenuItemType);

	gui_element* MenuElem = GUIFindMenuParentNode(Element);

	GUIEndElement(GUIState, GUIElement_MenuItem);
	
	gui_layout* View = GUIGetCurrentLayout(GUIState);
	if (GUIElementShouldBeUpdated(Element)) {
		gui_interaction Interaction = GUIMenuBarInteraction(Element);

		if (MenuItemType == GUIMenuItem_MenuBarItem) {
			char Buf[64];
			stbsp_sprintf(Buf, "@#!%s", Element->Name);
			GUIButton(GUIState, Buf, &Interaction);
		}

		if (Element->Expanded) {

			float AscenderByScale = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
			float RowAdvance = GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale);

			float InMenuElementSpacingPersentage = 1.2f;
			rect2 WorkRect; 
			v2 WorkRectDim = V2(Element->Cache.MenuNode.MaxWidth + GUIState->FontInfo->AscenderHeight * 2.0f, 
				(Element->Cache.MenuNode.ChildrenCount) * RowAdvance * InMenuElementSpacingPersentage + 
				(InMenuElementSpacingPersentage - 1.0f) * RowAdvance);

			if (Element->Cache.MenuNode.Type == GUIMenuItem_MenuBarItem) {
				WorkRect = Rect2MinDim(
					View->LastElementP + V2(0, View->LastElementDim.y),
					WorkRectDim);
			}
			else if (Element->Cache.MenuNode.Type == GUIMenuItem_MenuItem){
				//WorkRect = Rect2MinDim()
			}
			else {
				Assert(!"Invalid path!");
			}

			RENDERPushRect(GUIState->RenderStack, WorkRect, GUIGetColor(GUIState, GUIState->ColorTheme.WindowBackgroundColor));
			RENDERPushRectOutline(GUIState->RenderStack, WorkRect, 1, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

			gui_element* At = Element->ChildrenSentinel->PrevBro;
			float AtY = WorkRect.Min.y + AscenderByScale + (InMenuElementSpacingPersentage - 1.0f) * RowAdvance;
			for (; At != Element->ChildrenSentinel; At = At->PrevBro) {

				rect2 MenuItemTextRc = PrintTextInternal(
					GUIState,
					PrintTextType_GetTextSize,
					At->Name, 0, 0,
					GUIState->FontScale);
				v2 MenuItemTextDim = GetRectDim(MenuItemTextRc);

				float PrintX = WorkRect.Min.x + WorkRectDim.x * 0.5f - MenuItemTextDim.x * 0.5f;
				float PrintY = AtY;

				v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);

				rect2 InteractTextRc = Rect2MinDim(
					V2(WorkRect.Min.x, PrintY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), 
					V2(WorkRectDim.x, MenuItemTextDim.y));

#if 1
				InteractTextRc.Min.x += AscenderByScale * 0.2f;
				InteractTextRc.Max.x -= AscenderByScale * 0.2f;
#endif

				if (MouseInRect(GUIState->Input, InteractTextRc)) {
					//TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);

					RENDERPushRect(GUIState->RenderStack, InteractTextRc, GUIGetColor(GUIState, GUIState->ColorTheme.FirstColor));
					RENDERPushRectOutline(GUIState->RenderStack, InteractTextRc, 1, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));
				}

				PrintTextInternal(GUIState, PrintTextType_PrintText, At->Name, PrintX, PrintY, GUIState->FontScale, TextHighlightColor);

				AtY += GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale) * InMenuElementSpacingPersentage;
			}
		}

		//NOTE(DIMA): 
		if (MenuItemType == GUIMenuItem_MenuItem) {
			rect2 ElemTextRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Element->Name, 0, 0, GUIState->FontScale);
			GUIDescribeElement(GUIState, GetRectDim(ElemTextRc), V2(0, 0));

			MenuElem->Cache.MenuNode.MaxWidth = Max(View->LastElementDim.x, MenuElem->Cache.MenuNode.MaxWidth);
			MenuElem->Cache.MenuNode.MaxHeight = Max(View->LastElementDim.y, MenuElem->Cache.MenuNode.MaxHeight);
			MenuElem->Cache.MenuNode.SumHeight += View->LastElementDim.y;
			MenuElem->Cache.MenuNode.SumWidth += View->LastElementDim.x;
			MenuElem->Cache.MenuNode.ChildrenCount++;
		}
	}
}

void GUIBeginMenuBarItem(gui_state* GUIState, char* Name) {
	GUIBeginMenuItemInternal(GUIState, Name, GUIMenuItem_MenuBarItem);
}

void GUIEndMenuBarItem(gui_state* GUIState) {
	GUIEndMenuItemInternal(GUIState, GUIMenuItem_MenuBarItem);
}

void GUIMenuBarItem(gui_state* GUIState, char* ItemName) {
	GUIBeginMenuItemInternal(GUIState, ItemName, GUIMenuItem_MenuItem);
	GUIEndMenuItemInternal(GUIState, GUIMenuItem_MenuItem);
}
#endif

#if 0
static gui_element* GUIWalkaroundBFS(gui_element* At, char* NodeName) {
	gui_element* Result = 0;

	//NOTE(Dima): First - walk through all elements on the level
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		if (StringsAreEqual(Element->Name, NodeName)) {
			Result = Element;
			return(Result);
		}
	}

	//NOTE(Dima): If nothing found - recursivery go through every children
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		Result = GUIWalkaroundBFS(Element, NodeName);
		if (Result) {
			return(Result);
		}
	}

	//NOTE(Dima): Return 0 if nothing found
	return(Result);
}

void GUIBeginTreeFind(gui_state* State, char* NodeName) {
	gui_layout* View = GUIGetCurrentLayout(State);

	gui_element* NeededElement = GUIWalkaroundBFS(State->RootNode, NodeName);
	Assert(NeededElement);

	gui_element* OldParent = View->CurrentNode;
	View->CurrentNode = NeededElement->Parent;
	View->CurrentNode->TempParent = OldParent;

	GUITreeBegin(State, NodeName);
}

void GUIEndTreeFind(gui_state* State) {
	gui_layout* View = GUIGetCurrentLayout(State);

	GUITreeEnd(State);

	gui_element* Temp = View->CurrentNode;
	View->CurrentNode = View->CurrentNode->TempParent;
	Temp->TempParent = 0;
}
#endif