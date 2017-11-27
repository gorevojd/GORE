#ifndef WORKOUT_GUI_H
#define WORKOUT_GUI_H

#include "workout_render_stack.h"

#include "workout_asset.h"

struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	float FontScale;

	float CurrentX;
	float CurrentY;
};

extern void InitGUIState(gui_state* GUIState, font_info* FontInfo);
extern void BeginFrameGUI(gui_state* GUIState, render_stack* RenderStack);
extern void EndFrameGUI(gui_state* GUIState);

extern void BeginTempGUIRenderStack(gui_state* GUIState, render_stack* Stack);
extern void EndTempGUIRenderStack(gui_state* GUIState);

extern void PrintText(gui_state* GUIState, char* Text);
extern void HighlightedText(gui_state* GUIState, char* Text);

#endif