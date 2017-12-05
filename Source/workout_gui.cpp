#include "workout_gui.h"

void InitGUIState(gui_state* GUIState, font_info* FontInfo, input_system* Input) {
	GUIState->FontScale = 1.0f;
	GUIState->CurrentX = 10.0f;
	GUIState->CurrentY = 0.0f;
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
}

void BeginTempGUIRenderStack(gui_state* GUIState, render_stack* Stack) {
	GUIState->TempRenderStack = GUIState->RenderStack;
	GUIState->RenderStack = Stack;
}

void EndTempGUIRenderStack(gui_state* GUIState) {
	GUIState->RenderStack = GUIState->TempRenderStack;
}

void BeginFrameGUI(gui_state* GUIState, render_stack* RenderStack) {
	GUIState->RenderStack = RenderStack;

	GUIState->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;
}

void EndFrameGUI(gui_state* GUIState) {

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

void PrintText(gui_state* GUIState, char* Text) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, Text, GUIState->CurrentX, GUIState->CurrentY, GUIState->FontScale);

	GUIState->CurrentY += GetNextRowAdvance(GUIState->FontInfo);
}

void PrintLabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, GUIState->FontScale);
}

void HighlightedText(gui_state* GUIState, char* Text) {

	rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, GUIState->CurrentX, GUIState->CurrentY, GUIState->FontScale);
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
	
	PrintTextInternal(GUIState, PrintTextType_PrintText, Text, GUIState->CurrentX, GUIState->CurrentY, GUIState->FontScale, TextHighlightColor);

	GUIState->CurrentY += GetNextRowAdvance(GUIState->FontInfo);
}