#ifndef WORKOUT_GUI_H
#define WORKOUT_GUI_H

#include "workout_render_stack.h"

#include "workout_asset.h"

enum gui_variable_type {
	GUIVarType_F32,
	GUIVarType_U32,
	GUIVarType_I32,
	GUIVarType_U64,
	GUIVarType_I64,
	GUIVarType_B32,
};

struct gui_variable_link {
	u32 Type;
	union {
		float* Value_F32;
		u32* Value_U32;
		i32* Value_I32;
		u64* Value_U64;
		i64* Value_I64;
		b32* Value_B32;
	};
};

enum gui_interaction_type {
	GUIInteraction_None,
	GUIInteraction_VariableLink,
};

struct gui_interaction {
	b32 IsHot;

	u32 Type;
	union {
		gui_variable_link VariableLink;
	};
};

inline gui_interaction GUIVariableInteraction(void* Variable, u32 Type) {
	gui_interaction Result;

	switch (Type) {
		case GUIVarType_B32: {Result.VariableLink.Value_B32 = (b32*)Variable; }break;
		case GUIVarType_F32: {Result.VariableLink.Value_F32 = (float*)Variable; }break;
		case GUIVarType_I32: {Result.VariableLink.Value_I32 = (i32*)Variable; }break;
		case GUIVarType_U32: {Result.VariableLink.Value_U32 = (u32*)Variable; }break;
		case GUIVarType_I64: {Result.VariableLink.Value_I64 = (i64*)Variable; }break;
		case GUIVarType_U64: {Result.VariableLink.Value_U64 = (u64*)Variable; }break;
	}
	Result.VariableLink.Type = Type;
	Result.Type = GUIInteraction_VariableLink;
	Result.IsHot = 0;

	return(Result);
}

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

	input_system* Input;

	i32 ScreenWidth;
	i32 ScreenHeight;

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

extern void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height);
extern void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack);
extern void GUIEndFrame(gui_state* GUIState);

extern void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack);
extern void GUIEndTempRenderStack(gui_state* GUIState);

extern void GUIText(gui_state* GUIState, char* Text);
extern void GUIBoolButton(gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUILabel(gui_state* GUIState, char* LabelText, v2 At);
extern void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction);


extern void GUIBeginView(gui_state* GUIState);
extern void GUIEndView(gui_state* State);
extern void GUIBeginRow(gui_state* State);
extern void GUIEndRow(gui_state* State);

#endif