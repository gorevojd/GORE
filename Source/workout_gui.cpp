#include "workout_gui.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height){
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

#if 0
	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
#endif
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
}

void GUIEndView(gui_state* State) {
	State->CurrentViewIndex++;
}

void GUIBeginRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	Assert(!View->RowBeginned);
	View->RowBeginned = true;

	View->RowBeginX = View->CurrentX;
}

void GUIEndRow(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	Assert(View->RowBeginned);
	View->RowBeginned = false;

	View->CurrentX = View->RowBeginX;
	View->CurrentY += View->LastElementHeight;
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
		View->CurrentY += View->LastElementHeight;
#endif
	}
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
	gui_view* View = GetCurrentView(GUIState);
	rect2 Rc = PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale);

	//NOTE(Dima): Remember last element width for BeginRow/EndRow
	View->LastElementWidth = Rc.Max.x - Rc.Min.x;
	View->LastElementHeight = Rc.Max.y - Rc.Min.y;

	GUIAdvanceCursor(GUIState);
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	gui_view* View = GetCurrentView(GUIState);

	rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, View->FontScale);
	v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

	v4 TextHighlightColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
	if (MouseInRect(GUIState->Input, Rc)) {
		TextHighlightColor = V4(1.0f, 1.0f, 0.0f, 1.0f);
		if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
			Interaction->IsHot = !Interaction->IsHot;
		}
	}

	PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale, TextHighlightColor);

	//NOTE(Dima): Remember last element width for BeginRow/EndRow
	View->LastElementWidth = Rc.Max.x - Rc.Min.x;
	View->LastElementHeight = Rc.Max.y - Rc.Min.y;

	GUIAdvanceCursor(GUIState);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	gui_view* View = GetCurrentView(GUIState);

	rect2 NameRc = PrintTextInternal(GUIState, PrintTextType_PrintText, ButtonName, View->CurrentX, View->CurrentY, View->FontScale);
	v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

	float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight;
	float PrintButY = View->CurrentY;

	rect2 FalseRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "false", PrintButX, PrintButY, View->FontScale);
	v2 FalseDim = GetRectDim(FalseRc);
	rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight), FalseDim);

	PushRect(GUIState->RenderStack, ButRc, V4(0.0f, 0.5f, 1.0f, 1.0f));
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

	v4 TextHighlightColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
	if (MouseInRect(GUIState->Input, ButRc)) {
		TextHighlightColor = V4(1.0f, 1.0f, 0.0f, 1.0f);
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

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction) {
	gui_view* View = GetCurrentView(GUIState);
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
	PushRect(GUIState->RenderStack, WorkRect, V4(0.0f, 0.5f, 1.0f, 1.0f));
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

	v4 CursorColor = V4(1.0f, 0.4f, 0.0f, 1.0f);
	if (MouseInRect(GUIState->Input, CursorRect)) {

		if (MouseButtonWentDown(GUIState->Input, MouseButton_Left) && !Interaction->IsHot) {
			Interaction->IsHot = true;
		}
	}

	if (MouseButtonWentUp(GUIState->Input, MouseButton_Left) && Interaction->IsHot) {
		Interaction->IsHot = false;
	}

	if (Interaction->IsHot) {
		CursorColor = V4(0.5f, 1.0f, 0.0f, 1.0f);

		v2 InteractMouseP = GUIState->Input->MouseP;
		if (InteractMouseP.x > (WorkRect.Max.x - 0.5f * CursorWidth)) {
			*Value = Max;
		}

		if (InteractMouseP.x < (WorkRect.Min.x + 0.5f * CursorWidth)) {
			*Value = Min;
		}

		float AT = GUIState->Input->MouseP.x - (WorkRect.Min.x + 0.5f * CursorWidth);
		AT = Clamp(AT, 0.0f, WorkRectDim.x - CursorWidth);
		float NewVal01 = AT / (WorkRectDim.x - CursorWidth);
		float NewValue = Min + NewVal01 * Range;
		*Value = NewValue;
	}

	PushRect(GUIState->RenderStack, CursorRect, CursorColor);
	PushRectOutline(GUIState->RenderStack, CursorRect, 2, V4(0.1f, 0.1f, 0.1f, 1.0f));

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