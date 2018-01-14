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

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

	GUIState->PlusMinusSymbol = 0;

	GUIState->GUIMem = AllocateStackedMemory(1);

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

	CopyStrings(RootNode->Name, "Root");
	CopyStrings(RootNode->Text, "Root");

	RootNode->Expanded = 1;
	RootNode->Depth = 0;

	RootNode->Parent = 0;

	RootNode->ChildrenSentinel = PushStruct(&GUIState->GUIMem, gui_element);

	RootNode->ChildrenSentinel->NextBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->PrevBro = RootNode->ChildrenSentinel;

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

void GUIEndFrame(gui_state* GUIState) {
	GUIState->CurrentViewIndex = 0;
}

void GUIBeginView(gui_state* GUIState) {
	gui_view* View = GetCurrentView(GUIState);

	View->FontScale = 1.0f;

	View->CurrentX = View->ViewX;
	View->CurrentY = View->ViewY + GetNextRowAdvance(GUIState->FontInfo) * View->FontScale;

	View->CurrentNode = GUIState->RootNode;
	View->CurrentPreAdvance = 0.0f;
}

void GUIEndView(gui_state* State) {
	GUIBeginRootBlock(State, "GUI");
	gui_interaction Interaction = GUIVariableInteraction(&State->PlusMinusSymbol, GUIVarType_B32);
	GUIBoolButton(State, "PlusMinus", &Interaction);
	GUIEndRootBlock(State);


	State->CurrentViewIndex++;
}

void GUIBeginRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	Assert(!View->RowBeginned);

	GUIBeginElement(State, GUIElement_Row, "");

	View->RowBeginX = View->CurrentX;
	View->RowBeginned = true;
}

void GUIEndRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);
	
	Assert(View->RowBeginned);
	
	b32 NeedShow = GUIElementShouldBeUpdated(View->CurrentNode);

	if (NeedShow) {

		View->CurrentX = View->RowBeginX;
		View->CurrentY += View->LastElementHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
	}

	View->RowBeginned = false;

	GUIEndElement(State, GUIElement_Row);
}

inline void GUIPreAdvanceCursor(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	gui_element* Element = View->CurrentNode;

	View->CurrentPreAdvance = (Element->Depth - 1) * 2 * View->FontScale * State->FontInfo->AscenderHeight;
	View->CurrentX += View->CurrentPreAdvance;
}

inline void GUIAdvanceCursor(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	if (View->RowBeginned) {
		View->CurrentX += View->LastElementWidth + State->FontInfo->AscenderHeight;
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

inline void GUIFreeListElement(gui_state* State, gui_element* Element) {
	Element->NextBro->PrevBro = Element->PrevBro;
	Element->PrevBro->NextBro = Element->NextBro;

	Element->NextBro = State->FreeElementsSentinel->NextBro;
	Element->PrevBro = State->FreeElementsSentinel;

	Element->NextBro->PrevBro = Element;
	Element->PrevBro->NextBro = Element;
}

static gui_element* GUIRequestElement(gui_state* GUIState, u32 ElementType, char* ElementName) {
	gui_view* View = GetCurrentView(GUIState);

	gui_element* Parent = View->CurrentNode;

	gui_element* Element = 0;

	//IMPORTANT(DIMA): !!!
	if (ElementType == GUIElement_TreeNode)
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

		Element->NextBro = Parent->ChildrenSentinel->NextBro;
		Element->PrevBro = Parent->ChildrenSentinel;

		Element->PrevBro->NextBro = Element;
		Element->NextBro->PrevBro = Element;

		Element->Expanded = 1;
		Element->Depth = Parent->Depth + 1;

		Element->Parent = Parent;
		Element->TempParent = 0;

		Element->ChildrenSentinel = 0;
	}

	if (ElementType == GUIElement_Row) {
		Element = GUIAllocateListElement(GUIState);

		Element->NextBro = Parent->ChildrenSentinel->NextBro;
		Element->PrevBro = Parent->ChildrenSentinel;

		Element->PrevBro->NextBro = Element;
		Element->NextBro->PrevBro = Element;

		Element->Expanded = 1;
		Element->Depth = Parent->Depth;

		Element->Parent = Parent;
		Element->TempParent = 0;

		Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
		Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
		Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	}

	//NOTE(Dima): Element not exist or not found. We should allocate it
	if (Element == 0) {
		//NOTE(DIMA): If the "Free store" of elements is not empty, get the memory from there
		//TODO(DIMA): Some elements memory might be initialzed again if we get it from here
		Element = GUIAllocateListElement(GUIState);

		Element->NextBro = Parent->ChildrenSentinel->NextBro;
		Element->PrevBro = Parent->ChildrenSentinel;

		Element->PrevBro->NextBro = Element;
		Element->NextBro->PrevBro = Element;

		CopyStrings(Element->Name, ElementName);
		CopyStrings(Element->Text, ElementName);

		Element->Expanded = 0;
		Element->Depth = Parent->Depth + 1;

		Element->Parent = Parent;
		Element->TempParent = 0;

		Element->ChildrenSentinel = PushStruct(&GUIState->GUIMem, gui_element);

		Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
		Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	}

	return(Element);
}

b32 GUIBeginElement(gui_state* State, u32 ElementType, char* ElementName) {
	gui_view* View = GetCurrentView(State);

	gui_element* Element = GUIRequestElement(State, ElementType, ElementName);

	switch (ElementType) {
		case GUIElement_None: {

		}break;

		case GUIElement_TreeNode: {
			gui_interaction ActionInteraction = GUIVariableInteraction(&Element->Expanded, GUIVarType_B32);
			if (State->PlusMinusSymbol) {
				char TextBuf[64];
				stbsp_sprintf(TextBuf, "%c %s", Element->Expanded ? '+' : '-', Element->Text);
				GUIActionText(State, TextBuf, &ActionInteraction);
			}
			else {
				GUIActionText(State, ElementName, &ActionInteraction);
			}
		}break;

		case GUIElement_StaticItem: {

		}break;

		case GUIElement_CachedItem: {

		}break;
	}

	View->CurrentNode = Element;
	
	b32 NeedShow = GUIElementShouldBeUpdated(View->CurrentNode);

	return(NeedShow);
}

void GUIEndElement(gui_state* State, u32 ElementType) {
	gui_view* View = GetCurrentView(State);

	switch (ElementType) {
		case GUIElement_None: {

		}break;

		case GUIElement_TreeNode: {

		}break;

		case GUIElement_Row: {
			gui_element* Element = View->CurrentNode;

			GUIFreeListElement(State, Element->ChildrenSentinel);
			GUIFreeListElement(State, Element);
		}break;

		case GUIElement_StaticItem: {
			gui_element* Element = View->CurrentNode;

			GUIFreeListElement(State, Element);
		}break;

		case GUIElement_CachedItem: {

		}break;
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
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, 1.0f);
}

void GUIText(gui_state* GUIState, char* Text) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Text);

	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = Rc.Max.x - Rc.Min.x;
		View->LastElementHeight = Rc.Max.y - Rc.Min.y;

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Text);

	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, View->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIState->ColorTable[GUIColor_White];
		if (MouseInRect(GUIState->Input, Rc)) {
			TextHighlightColor = GUIState->ColorTable[GUIColor_Yellow];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				Interaction->IsHot = !Interaction->IsHot;

				if (Interaction->Type == GUIInteraction_VariableLink) {
					*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
				}
			}
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = Rc.Max.x - Rc.Min.x;
		View->LastElementHeight = Rc.Max.y - Rc.Min.y;

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, ButtonName);

	if (NeedShow) {

		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(GUIState, PrintTextType_PrintText, ButtonName, View->CurrentX, View->CurrentY, View->FontScale);
		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight;
		float PrintButY = View->CurrentY;

		rect2 FalseRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "false", PrintButX, PrintButY, View->FontScale);
		v2 FalseDim = GetRectDim(FalseRc);
		rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight), FalseDim);

		PushRect(GUIState->RenderStack, ButRc, GUIState->ColorTable[GUIColor_PrettyBlue]);
		PushRectOutline(GUIState->RenderStack, ButRc);

		char TextToPrint[8];
		if (*Interaction->VariableLink.Value_B32) {
			rect2 TrueRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "true", PrintButX, PrintButY, View->FontScale);
			v2 TrueDim = GetRectDim(TrueRc);
			PrintButX = FalseRc.Min.x + (FalseDim.x - TrueDim.x) * 0.5f;

			stbsp_sprintf(TextToPrint, "%s", "true");
		}
		else {
			stbsp_sprintf(TextToPrint, "%s", "false");
		}

		v4 TextHighlightColor = GUIState->ColorTable[GUIColor_White];
		if (MouseInRect(GUIState->Input, ButRc)) {
			TextHighlightColor = GUIState->ColorTable[GUIColor_Yellow];
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				Interaction->IsHot = !Interaction->IsHot;
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, TextToPrint, PrintButX, PrintButY, View->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		View->LastElementWidth = FalseRc.Max.x - View->CurrentX;
		View->LastElementHeight = FalseRc.Max.y - FalseRc.Min.y;

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Name);
	
	if (NeedShow) {
		gui_view* View = GetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		float* Value = Interaction->VariableLink.Value_F32;

		Assert(Max > Min);

		rect2 NameTextSize = PrintTextInternal(GUIState, PrintTextType_PrintText, Name, View->CurrentX, View->CurrentY, View->FontScale);

		char ValueBuf[32];
		stbsp_sprintf(ValueBuf, "%.3f", *Value);
		rect2 ValueTextSize = PrintTextInternal(GUIState, PrintTextType_GetTextSize, ValueBuf, 0, 0, View->FontScale);

		//NOTE(Dima): Next element to the text is advanced by AscenderHeight
		v2 WorkRectMin = V2(
			View->CurrentX + (NameTextSize.Max.x - NameTextSize.Min.x) + GUIState->FontInfo->AscenderHeight,
			View->CurrentY - GUIState->FontInfo->AscenderHeight);
		v2 WorkRectDim = V2(200, ValueTextSize.Max.y - ValueTextSize.Min.y);

		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);
		v4 WorkRectColor = GUIState->ColorTable[GUIColor_PrettyBlue];

		PushRect(GUIState->RenderStack, WorkRect, WorkRectColor);
		PushRectOutline(GUIState->RenderStack, WorkRect);

		//NOTE(Dima): Remember last element width and height for BeginRow/EndRow
		View->LastElementWidth = WorkRect.Max.x - View->CurrentX;
		View->LastElementHeight = WorkRect.Max.y - WorkRect.Min.y;

		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = ((float)(*Value) - Min) / (float)Range;

		float CursorWidth = 15.0f;
		float CursorHeight = WorkRectDim.y;

		float CursorX = WorkRectMin.x + (WorkRectDim.x - CursorWidth) * RelativePos01;
		float CursorY = WorkRectMin.y - (CursorHeight - WorkRectDim.y) * 0.5f;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);

		v4 CursorColor = GUIState->ColorTable[GUIColor_OrangeRed];
		if (MouseInRect(GUIState->Input, CursorRect) || MouseInRect(GUIState->Input, WorkRect)) {

			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left) && !Interaction->IsHot) {
				Interaction->IsHot = true;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left) && Interaction->IsHot) {
			Interaction->IsHot = false;
		}

		if (Interaction->IsHot) {
			CursorColor = GUIState->ColorTable[GUIColor_PrettyGreen];

			v2 InteractMouseP = GUIState->Input->MouseP;
			if (InteractMouseP.x > (WorkRect.Max.x - 0.5f * CursorWidth)) {
				*Value = Max;
			}

			if (InteractMouseP.x < (WorkRect.Min.x + 0.5f * CursorWidth)) {
				*Value = Min;
			}

			float AT = InteractMouseP.x - (WorkRect.Min.x + 0.5f * CursorWidth);
			AT = Clamp(AT, 0.0f, WorkRectDim.x - CursorWidth);
			float NewVal01 = AT / (WorkRectDim.x - CursorWidth);
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;
		}

		PushRect(GUIState->RenderStack, CursorRect, CursorColor);
		PushRectOutline(GUIState->RenderStack, CursorRect, 2, GUIState->ColorTable[GUIColor_Black]);

		float ValueTextY = WorkRectMin.y + GUIState->FontInfo->AscenderHeight;
		float ValueTextX = WorkRectMin.x + WorkRectDim.x * 0.5f - (ValueTextSize.Max.x - ValueTextSize.Min.x) * 0.5f;
		PrintTextInternal(GUIState, PrintTextType_PrintText, ValueBuf, ValueTextX, ValueTextY, View->FontScale);

#if 0
		char TextBuf[64];
		stbsp_snprintf(TextBuf, sizeof(TextBuf), "Min: %.3f; Max: %.3f;", Min, Max);

		float DrawTextX = View->CurrentX + WorkRectMin.x + WorkRectDim.x + 10;
		PrintTextInternal(GUIState, PrintTextType_PrintText, TextBuf, DrawTextX, View->CurrentY, View->FontScale);
#endif

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
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
	GUIBeginElement(State, GUIElement_TreeNode, NodeName);
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