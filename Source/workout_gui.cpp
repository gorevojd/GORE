#include "workout_gui.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

inline v4 GUIColor255(int R, int G, int B) {
	float OneOver255 = 1.0f / 255.0f;
	v4 Result = V4(R, G, B, 1.0f);
	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

inline int GUIIntFromHexChar(char C) {
	int Result = 0;

	if (C >= 'a' && C <= 'f') {
		C += 'A' - 'a';
	}

	if (C >= '0' && C <= '9') {
		Result = C - '0';
	}

	if (C >= 'A' && C <= 'F') {
		Result = C + 10 - 'A';
	}

	return(Result);
}

inline v4 GUIColorHex(char* Str) {
	float OneOver255 = 1.0f / 255.0f;
	
	v4 Result;

	Assert(Str[0] == '#');

	int R, G, B;
	R = GUIIntFromHexChar(Str[1]) * 16 + GUIIntFromHexChar(Str[2]);
	G = GUIIntFromHexChar(Str[3]) * 16 + GUIIntFromHexChar(Str[4]);
	B = GUIIntFromHexChar(Str[5]) * 16 + GUIIntFromHexChar(Str[6]);

	Result = V4(R, G, B, 1.0f);

	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height){
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->FontScale = 1.0f;

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

	GUIState->PlusMinusSymbol = 0;

	GUIState->GUIMem = AllocateStackedMemory(KILOBYTES(100));

#if 0
	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
#endif

	//NOTE(Dima): Initialization of the root node;
	GUIState->RootNode = PushStruct(&GUIState->GUIMem, gui_element);
	gui_element* RootNode = GUIState->RootNode;

	RootNode->PrevBro = RootNode;
	RootNode->NextBro = RootNode;

	CopyStrings(RootNode->Name, "MainRoot");
	CopyStrings(RootNode->Text, "MainRoot");

	RootNode->Expanded = 1;
	RootNode->Depth = 0;

	RootNode->Parent = 0;
	RootNode->Type = GUIElement_None;

	RootNode->ChildrenSentinel = PushStruct(&GUIState->GUIMem, gui_element);

	RootNode->ChildrenSentinel->NextBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->PrevBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->Parent = RootNode;

	RootNode->NextBro = RootNode;
	RootNode->PrevBro = RootNode;

	GUIState->WalkaroundEnabled = false;
	GUIState->WalkaroundElement = RootNode;

	/*
		NOTE(Dima): 
			Initialization of the "Free store" of the gui elements;
			Used for freeing and allocating static gui elements;
			Static gui elements has no cache and can be freed every frame;
	*/
	GUIState->FreeElementsSentinel = PushStruct(&GUIState->GUIMem, gui_element);
	GUIState->FreeElementsSentinel->NextBro = GUIState->FreeElementsSentinel;
	GUIState->FreeElementsSentinel->PrevBro = GUIState->FreeElementsSentinel;

	//NOTE(Dima): Initialization of the color table
	GUIState->ColorTable[GUIColor_Black] = V4(0.0f, 0.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_White] = V4(1.0f, 1.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Red] = V4(1.0f, 0.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Green] = V4(0.0f, 1.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Blue] = V4(0.0f, 0.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Yellow] = V4(1.0f, 1.0f, 0.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Magenta] = V4(1.0f, 0.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_Cyan] = V4(0.0f, 1.0f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_PrettyBlue] = V4(0.0f, 0.5f, 1.0f, 1.0f);
	GUIState->ColorTable[GUIColor_PrettyGreen] = V4(0.5f, 1.0f, 0.0f, 1.0f);

	GUIState->ColorTable[GUIColor_Purple] = GUIColor255(85, 26, 139);
	GUIState->ColorTable[GUIColor_DarkRed] = GUIColorHex("#cd0000");
	GUIState->ColorTable[GUIColor_Orange] = GUIColorHex("#ffa500");
	GUIState->ColorTable[GUIColor_OrangeRed] = GUIColorHex("#ff4500");
	GUIState->ColorTable[GUIColor_RoyalBlue] = GUIColorHex("#436eee");
	GUIState->ColorTable[GUIColor_PrettyPink] = GUIColorHex("#ee30a7");

	GUIState->ColorTable[GUIColor_BluishGray] = GUIColorHex("#778899");

	GUIState->ColorTable[GUIColor_Burlywood] = GUIColorHex("#deb887");
	GUIState->ColorTable[GUIColor_DarkGoldenrod] = GUIColorHex("#b8860b");
	GUIState->ColorTable[GUIColor_OliveDrab] = GUIColorHex("#6b8e23");

	//NOTE(DIMA): Initialization of the color theme
	GUIState->ColorTheme = GUIDefaultColorTheme();
}

void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack) {
	GUIState->TempRenderStack = GUIState->RenderStack;
	GUIState->RenderStack = Stack;
}
 
void GUIEndTempRenderStack(gui_state* GUIState) {
	GUIState->RenderStack = GUIState->TempRenderStack;
}

void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack) {
	GUIState->RenderStack = RenderStack;

	Assert(GUIState->CurrentViewIndex == 0);
}

inline b32 GUIElementIsValidForWalkaround(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;

		if (Parent) {
			if (Parent->ChildrenSentinel != Element) {
				Result =
#if 0
					(Element->Type != GUIElement_StaticItem) &&
#endif
					(Element->Type != GUIElement_Row) &&
					(Element->Type != GUIElement_None);
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

inline gui_element* GUIFindNextForWalkaround(gui_element* Element) {
	gui_element* Result = 0;

	gui_element* At = Element->NextBro;
	while (At != Element) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}

		At = At->NextBro;
	}

	return(Result);
}

inline gui_element* GUIFindPrevForWalkaround(gui_element* Element) {
	gui_element* Result = 0;

	gui_element* At = Element->PrevBro;
	while (At != Element) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}

		At = At->PrevBro;
	}

	return(Result);
}

inline gui_element* GUIFindElementForWalkaroundBFS(gui_element* CurrentElement) {
	gui_element* Result = 0;

	if (CurrentElement->ChildrenSentinel) {
		gui_element* At = CurrentElement->ChildrenSentinel->PrevBro;
		if (At != CurrentElement->ChildrenSentinel) {
			while (At != CurrentElement->ChildrenSentinel) {

				if (GUIElementIsValidForWalkaround(At)) {
					Result = At;
					return(Result);
				}

				At = At->PrevBro;
			}

			At = CurrentElement->ChildrenSentinel->PrevBro;
			while (At != CurrentElement->ChildrenSentinel) {
				Result = GUIFindElementForWalkaroundBFS(At);
				if (Result) {
					return(Result);
				}

				At = At->PrevBro;
			}
		}
	}

	return(Result);
}

inline b32 GUIWalkaroundIsOnElement(gui_state* State, gui_element* Element) {
	b32 Result = 0;

	if (State->WalkaroundEnabled) {
		gui_view* View = GetCurrentView(State);
		if (State->WalkaroundElement == Element) {
			Result = 1;
		}
	}

	return(Result);
}

void GUIEndFrame(gui_state* GUIState) {
	GUIState->CurrentViewIndex = 0;

	if (ButtonWentDown(GUIState->Input, KeyType_Backquote)) {
		GUIState->WalkaroundEnabled = !GUIState->WalkaroundEnabled;
	}

	if (GUIState->WalkaroundEnabled) {
		gui_element** Walk = &GUIState->WalkaroundElement;

		if (!GUIElementIsValidForWalkaround(*Walk)) {
			*Walk = GUIFindElementForWalkaroundBFS(*Walk);
		}

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

		if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
			if ((*Walk)->Type == GUIElement_TreeNode) {
				(*Walk)->Expanded = !(*Walk)->Expanded;
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
			*Walk = (*Walk)->Parent;
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Backspace)) {
			*Walk = (*Walk)->Parent;
			if ((*Walk)->Expanded) {
				(*Walk)->Expanded = 0;
			}
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
}

void GUIBeginView(gui_state* GUIState) {
	gui_view* View = GetCurrentView(GUIState);


	View->CurrentX = View->ViewX;
	View->CurrentY = View->ViewY + GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;

	View->RowBeginned = 0;
	View->RowBiggestHeight = 0;
	View->RowBeginX = 0;

	View->CurrentNode = GUIState->RootNode;
	View->CurrentPreAdvance = 0.0f;
}

void GUIEndView(gui_state* State) {
	GUIBeginRootBlock(State, "GUI");
	
	gui_interaction PlusMinusInteraction = GUIVariableInteraction(&State->PlusMinusSymbol, GUIVarType_B32);
	GUIBoolButton(State, "PlusMinus", &PlusMinusInteraction);

	gui_interaction MemInteraction = GUIVariableInteraction(&State->GUIMem, GUIVarType_StackedMemory);
	GUIStackedMemGraph(State, "GUI memory graph", &MemInteraction);

	gui_view* View = GetCurrentView(State);
	gui_interaction FontScaleInteraction = GUIVariableInteraction(&State->FontScale, GUIVarType_F32);
	GUISlider(State, "Font scale", 0.5f, 1.5f, &FontScaleInteraction);

	GUIEndRootBlock(State);

	State->CurrentViewIndex++;
}

void GUIBeginRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	Assert(!View->RowBeginned);

	char NameBuf[16];
	stbsp_sprintf(NameBuf, "Row:%u", View->CurrentNode->RowCount);

	GUIBeginElement(State, GUIElement_Row, NameBuf, 0);

	View->RowBeginX = View->CurrentX;
	View->RowBeginned = true;
}

void GUIEndRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);
	
	Assert(View->RowBeginned);
	
	b32 NeedShow = GUIElementShouldBeUpdated(View->CurrentNode);

	if (NeedShow) {
		View->CurrentX = View->RowBeginX;
		View->CurrentY += View->RowBiggestHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
	}

	View->RowBeginned = false;
	View->RowBiggestHeight = 0;

	GUIEndElement(State, GUIElement_Row);

	View->CurrentNode->RowCount++;
}

inline void GUIPreAdvanceCursor(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	gui_element* Element = View->CurrentNode;

	View->CurrentPreAdvance = (Element->Depth - 1) * 2 * State->FontScale * State->FontInfo->AscenderHeight;
	View->CurrentX += View->CurrentPreAdvance;
}

inline void GUIDescribeRowElement(gui_state* State, v2 ElementDim) {
	gui_view* View = GetCurrentView(State);

	View->LastElementWidth = ElementDim.x;
	View->LastElementHeight = ElementDim.y;

	if (ElementDim.y > View->RowBiggestHeight) {
		View->RowBiggestHeight = ElementDim.y;
	}
}

inline void GUIAdvanceCursor(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	if (View->RowBeginned) {
		View->CurrentX += View->LastElementWidth + State->FontInfo->AscenderHeight * State->FontScale;
	}
	else {
#if 0
		View->CurrentY += GetNextRowAdvance(State->FontInfo, 1.2f);
#else
		View->CurrentY += View->LastElementHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
#endif
	}
	
	View->CurrentX -= View->CurrentPreAdvance;
}

inline gui_element* GUIAllocateListElement(gui_state* State) {
	gui_element* Element = 0;

	if (State->FreeElementsSentinel->NextBro != State->FreeElementsSentinel) {
		Element = State->FreeElementsSentinel->NextBro;

		Element->NextBro->PrevBro = Element->PrevBro;
		Element->PrevBro->NextBro = Element->NextBro;
	}
	else {
		Element = PushStruct(&State->GUIMem, gui_element);
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

static gui_element* GUIRequestElement(
	gui_state* GUIState, 
	u32 ElementType, 
	char* ElementName,
	gui_interaction* Interaction) 
{
	gui_view* View = GetCurrentView(GUIState);

	gui_element* Parent = View->CurrentNode;

	gui_element* Element = 0;

	//IMPORTANT(DIMA): I need to add cached elements here!!!!!
	b32 ElementIsDynamic = 
		(ElementType == GUIElement_TreeNode ||
		ElementType == GUIElement_CachedItem ||
		ElementType == GUIElement_InteractibleItem ||
		ElementType == GUIElement_Row);

	//IMPORTANT(DIMA): !!!
	if (ElementIsDynamic)
	{
		//NOTE(DIMA): Finding the element in the hierarchy
		for (gui_element* Node = Parent->ChildrenSentinel->NextBro;
			Node != Parent->ChildrenSentinel;
			Node = Node->NextBro)
		{
			if (StringsAreEqual(ElementName, Node->Name)) {
				Element = Node;
				break;
			}
		}
	}

	if (ElementType == GUIElement_StaticItem) {
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		Element->Expanded = 1;
		Element->Depth = Parent->Depth + 1;
		Element->RowCount = 0;
		Element->ID = 0;

		Element->ChildrenSentinel = 0;
	}

	//NOTE(Dima): Element not exist or not found. We should allocate it
	if (Element == 0) {
		//NOTE(DIMA): If the "Free store" of elements is not empty, get the memory from there
		//TODO(DIMA): Some elements memory might be initialzed again if we get it from here
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		if (ElementType == GUIElement_TreeNode ||
			ElementType == GUIElement_CachedItem)
		{
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
			Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
			Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
			Element->ChildrenSentinel->Parent = Element;
		}

		if (ElementType == GUIElement_Row) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
			Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
			Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
			Element->ChildrenSentinel->Parent = Element;
		}

		if (ElementType == GUIElement_InteractibleItem) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			Element->ChildrenSentinel = 0;
		}

		CopyStrings(Element->Name, ElementName);
		CopyStrings(Element->Text, ElementName);

		Element->ID = GUIStringHashFNV(ElementName);
		Element->RowCount = 0;
		Element->Cache = {};
	}

	//NOTE(Dima): Setting common values
	Element->Type = ElementType;
	Element->Parent = Parent;
	Element->TempParent = 0;

	//NOTE(Dima): Setting interaction ID for dynamic(cached) elements
	if (ElementIsDynamic && Interaction) {
		Interaction->ID = GUITreeElementID(Element);
	}

	return(Element);
}

b32 GUIBeginElement(
	gui_state* State, 
	u32 ElementType, 
	char* ElementName, 
	gui_interaction* ElementInteraction) 
{
	gui_view* View = GetCurrentView(State);

	gui_element* Element = GUIRequestElement(State, ElementType, ElementName, ElementInteraction);

	View->CurrentNode = Element;
	
	b32 NeedShow = GUIElementShouldBeUpdated(View->CurrentNode);

	return(NeedShow);
}

void GUIEndElement(gui_state* State, u32 ElementType) {
	gui_view* View = GetCurrentView(State);

	gui_element* Element = View->CurrentNode;

	Assert(ElementType == Element->Type);

	if (ElementType == GUIElement_StaticItem) {
		GUIFreeListElement(State, Element);
	}

	if (ElementType == GUIElement_Row) {
		View->CurrentNode->Parent->RowCount = 0;
	}

	View->CurrentNode = View->CurrentNode->Parent;
}

enum print_text_type {
	PrintTextType_PrintText,
	PrintTextType_GetTextSize,
};

static rect2 PrintTextInternal(gui_state* State, u32 Type, char* Text, float Px, float Py, float Scale, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 TextRect = {};

	float CurrentX = Px;
	float CurrentY = Py;

	char* At = Text;

	font_info* FontInfo = State->FontInfo;
	render_stack* Stack = State->RenderStack;

	while (*At) {
		int GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		glyph_info* Glyph = &FontInfo->Glyphs[GlyphIndex];

		float BitmapScale = Glyph->Height * Scale;

		if (Type == PrintTextType_PrintText) {
			float BitmapMinX;
			float BitmapMinY;
			
			BitmapMinY = CurrentY + (Glyph->YOffset - 1.0f) * Scale;
			BitmapMinX = CurrentX + (Glyph->XOffset - 1.0f) * Scale;

			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX + 2, BitmapMinY + 2 }, BitmapScale, V4(0.0f, 0.0f, 0.0f, 1.0f));
			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX, BitmapMinY }, BitmapScale, Color);
		}

		float Kerning = 0.0f;
		if (*(At + 1)) {
			Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
		}

		CurrentX += (Glyph->Advance + Kerning) * Scale;

		++At;
	}

	TextRect.Min.x = Px;
	TextRect.Min.y = Py - FontInfo->AscenderHeight * Scale;
	TextRect.Max.x = CurrentX;
	TextRect.Max.y = Py - FontInfo->DescenderHeight * Scale;

	return(TextRect);
}


void GUILabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, 1.0f, GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
}

void GUIStackedMemGraph(gui_state* GUIState, char* Name, gui_interaction* Interaction) {
	GUITreeBegin(GUIState, Name);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Name, Interaction);

	Assert(Interaction->VariableLink.Type == GUIVarType_StackedMemory);

	if (NeedShow) {
		stacked_memory* WorkMem = Interaction->VariableLink.Value_StackedMemory;

		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 GraphRect;
		GraphRect.Min.x = View->CurrentX;
		GraphRect.Min.y = View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		GraphRect.Max = GraphRect.Min + V2((float)GUIState->ScreenWidth * 0.5f, (float)GUIState->ScreenHeight * 0.12f);

		v2 GraphRectDim = GetRectDim(GraphRect);

		u64 OccupiedCount = WorkMem->Used;
		u64 FreeCount = (u64)WorkMem->MaxSize - OccupiedCount;
		u64 TotalCount = WorkMem->MaxSize;
		
		float OccupiedPercentage = (float)OccupiedCount / (float)TotalCount;

		rect2 OccupiedRect = Rect2MinDim(GraphRect.Min, V2(GraphRectDim.x * OccupiedPercentage, GraphRectDim.y));
		rect2 FreeRect = Rect2MinDim(
			V2(OccupiedRect.Max.x, OccupiedRect.Min.y), 
			V2(GraphRectDim.x * (1.0f - OccupiedPercentage), GraphRectDim.y));

		float Inner = 2.0f;
		float Outer = 3.0f;

		PushRect(GUIState->RenderStack, OccupiedRect, GUIState->ColorTable[GUIState->ColorTheme.FirstColor]);
		PushRectOutline(GUIState->RenderStack, OccupiedRect, Inner, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);
		PushRect(GUIState->RenderStack, FreeRect, GUIState->ColorTable[GUIState->ColorTheme.SecondaryColor]);
		PushRectOutline(GUIState->RenderStack, FreeRect, Inner, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		PushRectOutline(GUIState->RenderStack, GraphRect, Outer, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		if (MouseInRect(GUIState->Input, GraphRect)) {
			char InfoStr[128];
			stbsp_sprintf(
				InfoStr, 
				"Occupied: %llu; Free: %llu; Total: %llu",
				OccupiedCount,
				FreeCount,
				TotalCount);

			GUILabel(GUIState, InfoStr, GUIState->Input->MouseP);
		}


		View->LastElementHeight = GraphRectDim.y;

		GUIDescribeRowElement(
			GUIState,
			V2(GraphRectDim.x + Outer,
				GraphRectDim.y + (2.0f * Outer)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
	GUITreeEnd(GUIState);
}

void GUIText(gui_state* GUIState, char* Text) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(
			GUIState, 
			PrintTextType_PrintText, 
			Text, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale, GUIState->ColorTable[GUIState->ColorTheme.TextColor]);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = Rc.Max.x - Rc.Min.x;
		View->LastElementHeight = Rc.Max.y - Rc.Min.y;

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, GUIState->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextColor];
		if (MouseInRect(GUIState->Input, Rc)) {
			TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				if (Interaction->Type == GUIInteraction_VariableLink) {
					*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
				}
			}
		}

		if (GUIWalkaroundIsOnElement(GUIState, View->CurrentNode)) {
			PushRectOutline(GUIState->RenderStack, Rc , 2, GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor]);
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = Rc.Max.x - Rc.Min.x;
		View->LastElementHeight = Rc.Max.y - Rc.Min.y;

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0);

	if (NeedShow) {

		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(
			GUIState, 
			PrintTextType_PrintText, 
			ButtonName, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale, 
			GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float PrintButY = View->CurrentY;

		rect2 FalseRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "false", PrintButX, PrintButY, GUIState->FontScale);
		v2 FalseDim = GetRectDim(FalseRc);
		rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), FalseDim);

		float OutlineWidth = 1;

		PushRect(GUIState->RenderStack, ButRc, GUIState->ColorTable[GUIState->ColorTheme.FirstColor]);
		PushRectOutline(GUIState->RenderStack, ButRc, OutlineWidth, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		char TextToPrint[8];
		if (*Interaction->VariableLink.Value_B32) {
			rect2 TrueRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "true", PrintButX, PrintButY, GUIState->FontScale);
			v2 TrueDim = GetRectDim(TrueRc);
			PrintButX = FalseRc.Min.x + (FalseDim.x - TrueDim.x) * 0.5f;

			stbsp_sprintf(TextToPrint, "%s", "true");
		}
		else {
			stbsp_sprintf(TextToPrint, "%s", "false");
		}

		v4 TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextColor];
		if (MouseInRect(GUIState->Input, ButRc)) {
			TextHighlightColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, TextToPrint, PrintButX, PrintButY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = FalseRc.Max.x - View->CurrentX;
		View->LastElementHeight = FalseRc.Max.y - FalseRc.Min.y + (2.0f * OutlineWidth);

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

static void GUIProcessVerticalSliderCache(
	gui_state* State,
	float CurrentFontScale,
	gui_vertical_slider_cache* Cache)
{
	if (Cache->IsInitialized) {


		Cache->IsInitialized = true;
	}
	else {

	}
}

void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(State, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		gui_view* View = GetCurrentView(State);
		gui_vertical_slider_cache* Cache = &View->CurrentNode->Cache.VerticalSlider;
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
			PrintTextType_GetTextSize,
			MaxValueTxt,
			0, 0,
			SmallTextScale);
		v2 MaxValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Calculating vertical rectangle
		v2 WorkRectMin = V2(View->CurrentX, MaxValueRcMin.y - State->FontInfo->DescenderHeight * SmallTextScale);
		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);

		//NOTE(DIMA): Calculating Min text rectangle
		//TODO(DIMA): Cache theese calculations
		char MinValueTxt[16];
		stbsp_sprintf(MinValueTxt, "%.2f", Min);
		v2 MinValueRcMin = V2(WorkRect.Min.x, WorkRect.Max.y + NextRowAdvanceSmall);
		rect2 MinValueRcSize = PrintTextInternal(
			State,
			PrintTextType_GetTextSize,
			MinValueTxt,
			0, 0,
			SmallTextScale);
		v2 MinValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Drawing Max value text
		rect2 MaxValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MaxValueTxt,
			MaxValueRcMin.x + 0.5f * (WorkRectDim.x - MaxValueRcDim.x), 
			MaxValueRcMin.y,
			SmallTextScale,
			State->ColorTable[State->ColorTheme.TextColor]);

		//NOTE(DIMA): Drawing vertical rectangle
		i32 RectOutlineWidth = 1;
		
#if 0
		v4 WorkRectColor = State->ColorTable[State->ColorTheme.FirstColor];
		if (MouseInRect(State->Input, WorkRect)) {
			WorkRectColor = State->ColorTable[GUIColor_Red];
		}
		PushRect(State->RenderStack, WorkRect, WorkRectColor);
#else
		PushRect(State->RenderStack, WorkRect, State->ColorTable[State->ColorTheme.FirstColor]);
#endif

		PushRectOutline(State->RenderStack, WorkRect, RectOutlineWidth, State->ColorTable[State->ColorTheme.OutlineColor]);

		//NOTE(DIMA): Printing Min value text
		rect2 MinValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MinValueTxt,
			MinValueRcMin.x + 0.5f * (WorkRectDim.x - MinValueRcDim.x),
			MinValueRcMin.y,
			SmallTextScale,
			State->ColorTable[State->ColorTheme.TextColor]);

		//NOTE(DIMA): Printing name of the element that consist from first 3 chars of the name
		char SmallTextToPrint[8];
		stbsp_sprintf(SmallTextToPrint, "%.3s", Name);
		char *SmallBufAt = SmallTextToPrint;
		while (*SmallBufAt) {
			char CurChar = *SmallBufAt;

			if (CurChar >= 'a' && CurChar <= 'z') {
				*SmallBufAt = CurChar - 'a' + 'A';
			}

			SmallBufAt++;
		}
		rect2 SmallTextRect = PrintTextInternal(State, PrintTextType_GetTextSize, SmallTextToPrint, 0, 0, SmallTextScale);
		v2 SmallTextRcDim = GetRectDim(SmallTextRect);

		float SmallTextX = WorkRect.Min.x + WorkRectDim.x * 0.5f - SmallTextRcDim.x * 0.5f;
		float SmallTextY = MinValueRc.Min.y + MinValueRcDim.y + State->FontInfo->AscenderHeight * SmallTextScale;

		rect2 SmallTextRc = PrintTextInternal(
			State, 
			PrintTextType_PrintText, 
			SmallTextToPrint, 
			SmallTextX, 
			SmallTextY, 
			SmallTextScale, 
			State->ColorTable[State->ColorTheme.TextColor]);

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

		//NOTE(DIMA): Processing interactions
		v4 CursorColor = State->ColorTable[State->ColorTheme.SecondaryColor];
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

			GUILabel(State, ValStr, State->Input->MouseP);
		}

		if (MouseButtonWentUp(State->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(State, Interaction, false);
			IsHot = false;
		}

		if (IsHot) {
			CursorColor = State->ColorTable[State->ColorTheme.TextHighlightColor];

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
		PushRect(State->RenderStack, CursorRect, CursorColor);
		PushRectOutline(State->RenderStack, CursorRect, 2, State->ColorTable[State->ColorTheme.OutlineColor]);

		//NOTE(DIMA): Postprocessing
		float MaxWidth = Max(Max(Max(WorkRectDim.x, MaxValueRcDim.x), MinValueRcDim.x), SmallTextRcDim.x);

		GUIDescribeRowElement(
			State, 
			V2(MaxWidth,
			SmallTextRc.Max.y - MaxValueRc.Min.y));

		GUIAdvanceCursor(State);
	}

	GUIEndElement(State, GUIElement_InteractibleItem);
}

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction);
	
	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		float NextRowAdvanceFull = GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale);

		float* Value = Interaction->VariableLink.Value_F32;

		Assert(Max > Min);

		rect2 NameTextSize = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText, 
			Name, 
			View->CurrentX, 
			View->CurrentY, 
			GUIState->FontScale,
			GUIState->ColorTable[GUIState->ColorTheme.TextColor]);
		v2 NameTextDim = GetRectDim(NameTextSize);

		char ValueBuf[32];
		stbsp_sprintf(ValueBuf, "%.3f", *Value);
		rect2 ValueTextSize = PrintTextInternal(GUIState, PrintTextType_GetTextSize, ValueBuf, 0, 0, GUIState->FontScale);

		//NOTE(Dima): Next element to the text is advanced by AscenderHeight
		v2 WorkRectMin = V2(
			View->CurrentX + NameTextDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale,
			View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull * 10, NextRowAdvanceFull);

		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);
		v4 WorkRectColor = GUIState->ColorTable[GUIState->ColorTheme.FirstColor];

		float RectOutlineWidth = 1.0f;

		PushRect(GUIState->RenderStack, WorkRect, WorkRectColor);
		PushRectOutline(GUIState->RenderStack, WorkRect, RectOutlineWidth, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

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
		v4 CursorColor = GUIState->ColorTable[GUIState->ColorTheme.SecondaryColor];

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

		if(IsHot){
			CursorColor = GUIState->ColorTable[GUIState->ColorTheme.TextHighlightColor];

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

		PushRect(GUIState->RenderStack, CursorRect, CursorColor);
		PushRectOutline(GUIState->RenderStack, CursorRect, 2, GUIState->ColorTable[GUIState->ColorTheme.OutlineColor]);

		float ValueTextY = WorkRectMin.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float ValueTextX = WorkRectMin.x + WorkRectDim.x * 0.5f - (ValueTextSize.Max.x - ValueTextSize.Min.x) * 0.5f;
		PrintTextInternal(GUIState, PrintTextType_PrintText, ValueBuf, ValueTextX, ValueTextY, GUIState->FontScale);

#if 0
		char TextBuf[64];
		stbsp_snprintf(TextBuf, sizeof(TextBuf), "Min: %.3f; Max: %.3f;", Min, Max);

		float DrawTextX = View->CurrentX + WorkRectMin.x + WorkRectDim.x + 10;
		PrintTextInternal(GUIState, PrintTextType_PrintText, TextBuf, DrawTextX, View->CurrentY, View->FontScale);
#endif

		GUIDescribeRowElement(
			GUIState,
			V2(WorkRect.Max.x - View->CurrentX, 
			WorkRect.Max.y - WorkRect.Min.y + (2.0f * RectOutlineWidth)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

#if 0
void GUIResizableRect(gui_state* State) {
	render_stack* RenderStack = State->RenderStack;
	input_system* Input = State->Input;;

	rect2* MainRect = &State->TempRect.Rect;
	v2 RectDim = GetRectDim(*MainRect);

	PushRectOutline(RenderStack, *MainRect, 2);
	PushRect(RenderStack, *MainRect, V4(0.0f, 0.0f, 0.0f, 0.7));

	v2 AnchorDim = V2(7, 7);

	rect2 SizeAnchorRect;
	SizeAnchorRect.Min = MainRect->Max - V2(3.0f, 3.0f);
	SizeAnchorRect.Max = SizeAnchorRect.Min + AnchorDim;
	v4 SizeAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

	rect2 PosAnchorRect;
	PosAnchorRect.Min = MainRect->Min - V2(3.0f, 3.0f);
	PosAnchorRect.Max = PosAnchorRect.Min + AnchorDim;
	v4 PosAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

	if (MouseInRect(State->Input, SizeAnchorRect)) {
		SizeAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

		if (MouseButtonWentDown(Input, MouseButton_Left) && State->TempRect.SizeInteraction.IsHot) {
			State->TempRect.SizeInteraction.IsHot = true;
		}
	}

	if (MouseInRect(Input, PosAnchorRect)) {
		PosAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

		if (MouseButtonWentDown(Input, MouseButton_Left) && State->TempRect.PosInteraction.IsHot) {
			State->TempRect.PosInteraction.IsHot = true;
		}
	}

	if (MouseButtonWentUp(Input, MouseButton_Left)) {
		State->TempRect.SizeInteraction.IsHot = false;
		State->TempRect.PosInteraction.IsHot = false;
	}

	if (State->TempRect.PosInteraction.IsHot) {
		MainRect->Min = Input->MouseP;
		MainRect->Max = MainRect->Min + RectDim;
		PosAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
	}

	v2 ResizedRectDim = RectDim;
	if (State->TempRect.SizeInteraction.IsHot) {
		MainRect->Max = Input->MouseP;
		SizeAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
		ResizedRectDim = GetRectDim(*MainRect);
	}

	if (ResizedRectDim.x < 10) {
		MainRect->Max.x = MainRect->Min.x + 10;
	}

	if (ResizedRectDim.y < 10) {
		MainRect->Max.y = MainRect->Min.y + 10;
	}

	PushRect(RenderStack, SizeAnchorRect.Min, AnchorDim, SizeAnchorColor);
	PushRect(RenderStack, PosAnchorRect.Min, AnchorDim, PosAnchorColor);
}
#endif

void GUITreeBegin(gui_state* State, char* NodeName) {
	gui_view* View = GetCurrentView(State);
	
	b32 NeedShow = GUIBeginElement(State, GUIElement_TreeNode, NodeName, 0);

	gui_interaction ActionInteraction = GUIVariableInteraction(&View->CurrentNode->Expanded, GUIVarType_B32);
	char TextBuf[64];
	if (State->PlusMinusSymbol) {
		stbsp_sprintf(TextBuf, "%c %s", View->CurrentNode->Expanded ? '+' : '-', View->CurrentNode->Text);
	}
	else {
		stbsp_sprintf(TextBuf, "%s", View->CurrentNode->Text);
	}
	
	if (NeedShow) {
		GUIPreAdvanceCursor(State);

		gui_interaction Interaction_ = GUIVariableInteraction(&View->CurrentNode->Expanded, GUIVarType_B32);
		gui_interaction* Interaction = &Interaction_;

		rect2 Rc = PrintTextInternal(State, PrintTextType_GetTextSize, TextBuf, View->CurrentX, View->CurrentY, State->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = State->ColorTable[State->ColorTheme.TextColor];
		if (MouseInRect(State->Input, Rc)) {
			TextHighlightColor = State->ColorTable[State->ColorTheme.TextHighlightColor];
			if (MouseButtonWentDown(State->Input, MouseButton_Left)) {
				if (Interaction->Type == GUIInteraction_VariableLink) {
					*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
				}
			}
		}

		if (GUIWalkaroundIsOnElement(State, View->CurrentNode)) {
			PushRectOutline(State->RenderStack, Rc, 2, State->ColorTable[State->ColorTheme.TextHighlightColor]);
		}

		PrintTextInternal(State, PrintTextType_PrintText, TextBuf, View->CurrentX, View->CurrentY, State->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = Rc.Max.x - Rc.Min.x;
		View->LastElementHeight = Rc.Max.y - Rc.Min.y;

		GUIAdvanceCursor(State);
	}
}

void GUITreeEnd(gui_state* State) {
	GUIEndElement(State, GUIElement_TreeNode);
}

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
	gui_view* View = GetCurrentView(State);

	gui_element* NeededElement = GUIWalkaroundBFS(State->RootNode, NodeName);
	Assert(NeededElement);

	gui_element* OldParent = View->CurrentNode;
	View->CurrentNode = NeededElement->Parent;
	View->CurrentNode->TempParent = OldParent;

	GUITreeBegin(State, NodeName);
}

void GUIEndTreeFind(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	GUITreeEnd(State);

	gui_element* Temp = View->CurrentNode;
	View->CurrentNode = View->CurrentNode->TempParent;
	Temp->TempParent = 0;
}
#endif

void GUIBeginRootBlock(gui_state* State, char* BlockName) {
	gui_view* View = GetCurrentView(State);

	Assert(StringsAreEqual(State->RootNode->ChildrenSentinel->NextBro->Name, "Root"));
	gui_element* Root = State->RootNode->ChildrenSentinel->NextBro;

	gui_element* OldParent = View->CurrentNode;
	View->CurrentNode = Root;
	View->CurrentNode->TempParent = OldParent;

	GUITreeBegin(State, BlockName);
}

void GUIEndRootBlock(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	GUITreeEnd(State);

	gui_element* Temp = View->CurrentNode;
	View->CurrentNode = View->CurrentNode->TempParent;
	Temp->TempParent = 0;
}