#include "workout_gui.h"

void InitGUIState(gui_state* GUIState, font_info* FontInfo) {
	GUIState->FontScale = 1.0f;
	GUIState->CurrentX = 0.0f;
	GUIState->CurrentY = 0.0f;
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;
}

void BeginFrameGUI(gui_state* GUIState, render_stack* RenderStack) {
	GUIState->RenderStack = RenderStack;

	GUIState->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;
}

void EndFrameGUI(gui_state* GUIState) {

}

static void PrintTextInternal(gui_state* State, char* Text, float Px, float Py, float Scale, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	float CurrentX = Px;
	float CurrentY = Py;

	char* At = Text;

	font_info* FontInfo = State->FontInfo;
	render_stack* Stack = State->RenderStack;

	while (*At) {
		int GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		glyph_info* Glyph = &FontInfo->Glyphs[GlyphIndex];

		float BitmapMinX;
		float BitmapMinY;

		BitmapMinY = CurrentY + (Glyph->YOffset - 1.0f) * Scale;
		BitmapMinX = CurrentX + (Glyph->XOffset - 1.0f) * Scale;

		float BitmapScale = Glyph->Height * Scale;

		PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX + 2, BitmapMinY + 2 }, BitmapScale, V4(0.0f, 0.0f, 0.0f, 1.0f));
		PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX, BitmapMinY }, BitmapScale);

		float Kerning = 0.0f;
		if (*(At + 1)) {
			Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
		}

		CurrentX += (Glyph->Advance + Kerning) * Scale;

		++At;
	}
}

void PrintText(gui_state* GUIState, char* Text) {
	PrintTextInternal(GUIState, Text, GUIState->CurrentX, GUIState->CurrentY, GUIState->FontScale);

	GUIState->CurrentY += GetNextRowAdvance(GUIState->FontInfo);
}
