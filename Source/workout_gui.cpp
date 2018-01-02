#include "workout_gui.h"

void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input) {
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
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
}

inline void GUIAdvanceCursor(gui_state* State) {
	gui_view* View = GetCurrentView(State);

	if (View->RowBeginned) {

	}

	View->CurrentY += GetNextRowAdvance(State->FontInfo);
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

void GUIText(gui_state* GUIState, char* Text) {
	gui_view* View = GetCurrentView(GUIState);
	PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale);

	GUIAdvanceCursor(GUIState);
}

void GUILabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, 1.0f);
}

void GUIActionButton(gui_state* GUIState, char* Text) {
	gui_view* View = GetCurrentView(GUIState);

	rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, View->FontScale);
	v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

	v4 TextHighlightColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
	if (MouseInRect(GUIState->Input, Rc)) {
		TextHighlightColor = V4(1.0f, 1.0f, 0.0f, 1.0f);
		if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
			GUIState->TempBut.ChangeInteraction.IsHot = !GUIState->TempBut.ChangeInteraction.IsHot;
		}
	}
	
	v4 HighColor = GUIState->TempBut.ChangeInteraction.IsHot ? V4(0.0f, 0.5f, 1.0f, 1.0f) : V4(0.3f, 0.3f, 0.5f, 1.0f);

	PushRectOutline(GUIState->RenderStack, Rc);
	PushRect(GUIState->RenderStack, Rc.Min, Dim, HighColor);
	
	PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, View->FontScale, TextHighlightColor);

	GUIAdvanceCursor(GUIState);
}

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