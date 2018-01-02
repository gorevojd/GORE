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

struct gui_view {
	float FontScale;

	float CurrentX;
	float CurrentY;

	float ViewX;
	float ViewY;

	gui_interaction PosInteraction;

	float LastElementWidth;
	float LastElementHeight;

	float RowBeginX;

	b32 RowBeginned;
};

struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	interactible_rect TempRect;
	gui_button TempBut;

	input_system* Input;

	gui_view GUIViews[8];
	int CurrentViewIndex;
};

inline gui_view* GetCurrentView(gui_state* GUIState) {
	gui_view* Result = 0;

	if (GUIState->CurrentViewIndex < ArrayCount(GUIState->GUIViews) &&
		GUIState->CurrentViewIndex >= 0)
	{
		Result = &GUIState->GUIViews[GUIState->CurrentViewIndex];
	}

	return(Result);
}

extern void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input);
extern void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack);
extern void GUIEndFrame(gui_state* GUIState);

extern void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack);
extern void GUIEndTempRenderStack(gui_state* GUIState);

extern void GUIText(gui_state* GUIState, char* Text);
extern void GUIActionButton(gui_state* GUIState, char* Text);
extern void GUILabel(gui_state* GUIState, char* LabelText, v2 At);


extern void GUIBeginView(gui_state* GUIState);
extern void GUIEndView(gui_state* State);
extern void GUIBeginRow(gui_state* State);
extern void GUIEndRow(gui_state* State);

#endif