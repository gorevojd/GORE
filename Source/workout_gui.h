#ifndef WORKOUT_GUI_H
#define WORKOUT_GUI_H

#include "workout_render_stack.h"

#include "workout_asset.h"

struct gui_interaction {
	b32 IsHot;
};

struct interactible_rect {
	rect2 Rect;

	gui_interaction SizeInteraction;
	gui_interaction PosInteraction;
};

struct gui_button {
	gui_interaction ChangeInteraction;
};

struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	interactible_rect TempRect;
	gui_button TempBut;

	input_system* Input;

	float FontScale;

	float CurrentX;
	float CurrentY;
};


extern void InitGUIState(gui_state* GUIState, font_info* FontInfo, input_system* Input);
extern void BeginFrameGUI(gui_state* GUIState, render_stack* RenderStack);
extern void EndFrameGUI(gui_state* GUIState);

extern void BeginTempGUIRenderStack(gui_state* GUIState, render_stack* Stack);
extern void EndTempGUIRenderStack(gui_state* GUIState);

extern void PrintText(gui_state* GUIState, char* Text);
extern void HighlightedText(gui_state* GUIState, char* Text);
extern void PrintLabel(gui_state* GUIState, char* LabelText, v2 At);

#endif