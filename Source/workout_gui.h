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
	GUIVarType_StackedMemory,
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
		stacked_memory* Value_StackedMemory;
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
		case GUIVarType_StackedMemory: {
			Result.VariableLink.Value_StackedMemory = (stacked_memory*)Variable; 
		}break;
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

enum gui_element_type {
	GUIElement_None,

	GUIElement_TreeNode,
	GUIElement_StaticItem,
	GUIElement_CachedItem,
	GUIElement_Row,
};

struct gui_element {
	s32 Depth;

	gui_element* Parent;
	gui_element* TempParent;

	gui_element* NextBro;
	gui_element* PrevBro;

	gui_element* ChildrenSentinel;
	//gui_element* StaticElementsSentinel;
	//gui_element* ChildrenFreeSentinel;

	u32 Type;
	union {
		struct {
			char Name[64];
			char Text[64];

			b32 Expanded;
		};
	};
};

struct gui_view {
	float FontScale;

	float CurrentX;
	float CurrentY;

	float ViewX;
	float ViewY;

	float LastElementWidth;
	float LastElementHeight;

	float RowBeginX;

	float CurrentPreAdvance;

	b32 RowBeginned;

	gui_element* CurrentNode;
};

enum gui_color_table_type {
	GUIColor_Black,
	GUIColor_White,

	GUIColor_Red,
	GUIColor_Green,
	GUIColor_Blue,

	GUIColor_Yellow,
	GUIColor_Magenta,
	GUIColor_Cyan,

	GUIColor_PrettyBlue,
	GUIColor_PrettyGreen,

	GUIColor_Purple,

	GUIColor_Orange,
	GUIColor_OrangeRed,

	GUIColor_DarkRed,
	GUIColor_RoyalBlue,
	GUIColor_PrettyPink,
	GUIColor_BluishGray,

	GUIColor_Burlywood,
	GUIColor_DarkGoldenrod,
	GUIColor_OliveDrab,

	GUIColor_Count,
};

struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	input_system* Input;

	i32 ScreenWidth;
	i32 ScreenHeight;

	gui_element* RootNode;
	gui_element* FreeElementsSentinel;

	stacked_memory GUIMem;

	gui_view GUIViews[8];
	int CurrentViewIndex;

	b32 PlusMinusSymbol;

	v4 ColorTable[GUIColor_Count];
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

inline b32 GUIElementShouldBeUpdated(gui_element* Node) {
	b32 Result = 1;

	gui_element* At = Node->Parent;
	while (At->Parent != 0) {
		Result = At->Expanded & Result;

		if (Result == 0) {
			break;
		}

		At = At->Parent;
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
extern void GUIStackedMemGraph(gui_state* GUIState, char* Name, gui_interaction* Interaction);

extern void GUIBeginView(gui_state* GUIState);
extern void GUIEndView(gui_state* State);
extern void GUIBeginRow(gui_state* State);
extern void GUIEndRow(gui_state* State);

extern b32 GUIBeginElement(gui_state* State, u32 ElementType, char* ElementName);
extern void GUIEndElement(gui_state* State, u32 ElementType);

extern void GUITreeBegin(gui_state* State, char* NodeText);
extern void GUITreeEnd(gui_state* State);
extern void GUIBeginRootBlock(gui_state* State, char* BlockName);
extern void GUIEndRootBlock(gui_state* State);


#if 0
extern void GUIBeginTreeFind(gui_state* State, char* NodeName);
extern void GUIEndTreeFind(gui_state* State);
#endif
#endif