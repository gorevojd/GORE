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
	GUIVarType_RGBABuffer,
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
		rgba_buffer* Value_RGBABuffer;
	};
};

enum gui_move_interaction_type {
	GUIMoveInteraction_None,

	GUIMoveInteraction_Move,
};

enum gui_resize_interaction_type {
	GUIResizeInteraction_None,

	GUIResizeInteraction_Default,
	GUIResizeInteraction_Proportional,
	GUIResizeInteraction_Horizontal,
	GUIResizeInteraction_Vertical,
};

struct gui_resize_interaction_context {
	v2* DimensionPtr;
	v2 Position;
	u32 Type;
};

struct gui_move_interaction_context {
	v2* MovePosition;
	u32 Type;
};

enum gui_interaction_type {
	GUIInteraction_None,
	GUIInteraction_VariableLink,
	GUIInteraction_ResizeInteraction,
	GUIInteraction_MoveInteraction,
};

struct gui_interaction {
	u32 ID;

	u32 Type;
	union {
		gui_variable_link VariableLink;
		gui_resize_interaction_context ResizeContext;
		gui_move_interaction_context MoveContext;
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
		case GUIVarType_RGBABuffer: {
			Result.VariableLink.Value_RGBABuffer = (rgba_buffer*)Variable;
		}break;
	}

	Result.VariableLink.Type = Type;
	Result.Type = GUIInteraction_VariableLink;
	Result.ID = 0;

	return(Result);
}

/* Type is the value of enum: gui_resize_interaction_type */
inline gui_interaction GUIResizeInteraction(v2 Position, v2* DimensionPtr, u32 Type) {
	gui_interaction Result;

	Result.Type = GUIInteraction_ResizeInteraction;

	Result.ResizeContext.DimensionPtr = DimensionPtr;
	Result.ResizeContext.Position = Position;
	Result.ResizeContext.Type = Type;

	return(Result);
}

/* Type is the value of enum: gui_move_interaction_type */
inline gui_interaction GUIMoveInteraction(v2* MovePosition, u32 Type) {
	gui_interaction Result;

	Result.Type = GUIInteraction_MoveInteraction;

	Result.MoveContext.MovePosition = MovePosition;
	Result.MoveContext.Type = Type;

	return(Result);
}

enum gui_view_type {
	GUIView_Tree,
	GUIView_Window,
};

struct gui_view {
	u32 ID;

	char Name[64];

	u32 ViewType;

	float CurrentX;
	float CurrentY;

	float LastElementWidth;
	float LastElementHeight;

	float RowBeginX;
	float RowBiggestHeight;

	float CurrentPreAdvance;

	gui_view* Parent;
	gui_view* NextBro;
	gui_view* PrevBro;

	b32 RowBeginned;
};

/*
	GUIElement_None - for the root element. Do not use it.

	GUIElement_TreeNode - this is for the tree view.
	This value means that this element will be cached
	and GUIActionText will be used for transitions 
	between root-child elements;

	GUIElement_InteractibleItem means that the element
	will have it's own unique ID by which it will be
	identified when GUIInteractionIsHot() called;
	This also means that this element will be cached;

	GUIElement_CachedItem means that the the element 
	will be cached. It means that it and it's children 
	elements won't be freed at the end of the frame.
	This is useful for elements that will have
	some other interactible elements in it;
	Difference between GUIElement_InteractibleItem
	is that the GUIElement_InteractibleItem should
	not have children.

	GUIElement_StaticItem means that the element
	will be freed at the end of every frame and 
	allocated at the beginning of every frame;
	This means also that the ID of the element 
	will not be calculated. The element should(or can) 
	have no name;

	P.S. Cahing means that the element won't be 
	created from scratch. It means that the element
	will not be freed at the end of the frame
*/
enum gui_element_type {
	GUIElement_None,

	GUIElement_TreeNode,
	GUIElement_InteractibleItem,
	GUIElement_CachedItem,
	GUIElement_StaticItem,
	GUIElement_Row,
	GUIElement_View,
};

enum gui_tree_node_exit_state {
	GUITreeNodeExit_None, 
	GUITreeNodeExit_Exiting,
	GUITreeNodeExit_Finished,
};

struct gui_tree_node_cache {
	u32 ExitState;

	float StackBeginY;
	float StackY;
	float ExitY;
};

struct gui_vertical_slider_cache {
};

struct gui_slider_cache {
};

struct gui_bool_button_cache {
};

struct gui_image_view_cache {
	v2 Dimension;
};

struct gui_stackedmem_cache {
	v2 Dimension;
};

struct gui_anchor_cache {
	v2 OffsetInAnchor;

};

struct gui_view_cache {
	v2 Position;
	v2 Dimension;
};

struct gui_element_cache {
	union {
		gui_tree_node_cache TreeNode;
		gui_vertical_slider_cache VerticalSlider;
		gui_slider_cache Slider;
		gui_bool_button_cache BoolButton;
		gui_image_view_cache ImageView;
		gui_stackedmem_cache StackedMem;
		gui_anchor_cache Anchor;
		gui_view_cache View;
	};

	b32 IsInitialized;
};

struct gui_element {
	u32 ID;

	s32 Depth;

	gui_element* Parent;
	gui_element* TempParent;

	//NOTE(Dima): Used for remembering last tree parent for tree nodes
	gui_element* TempParentTree;

	gui_element* NextBro;
	gui_element* PrevBro;

	gui_element* ChildrenSentinel;

	b32 Expanded;
	char Name[64];
	char Text[64];

	u32 RowCount;

	u32 Type;

	gui_element_cache Cache;
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

	GUIColor_Black_x20,

	GUIColor_Count,
};

struct gui_color_theme {
	u32 TextColor;
	u32 TextHighlightColor;

	u32 OutlineColor;

	u32 FirstColor;
	u32 SecondaryColor;

	u32 WindowBackgroundColor;
	u32 WindowTextColor;
	u32 WindowHelpColor;
	u32 WindowKeywordColor;

	u32 WalkaroundHotColor;
};

inline gui_color_theme GUIDefaultColorTheme() {
	gui_color_theme Result;

#if 1
	Result.TextColor = GUIColor_White;
	Result.TextHighlightColor = GUIColor_Yellow;

	Result.OutlineColor = GUIColor_Black;

	Result.WalkaroundHotColor = GUIColor_PrettyGreen;
	Result.WindowBackgroundColor = GUIColor_Black_x20;
	Result.WindowTextColor = GUIColor_Burlywood;
	Result.WindowHelpColor = GUIColor_White;
	Result.WindowKeywordColor = GUIColor_DarkGoldenrod;

#if 1
	Result.FirstColor = GUIColor_PrettyBlue;
	Result.SecondaryColor = GUIColor_BluishGray;
#else
	Result.FirstColor = GUIColor_OliveDrab;
	Result.SecondaryColor = GUIColor_Orange;

#endif
#else
	Result.TextColor = GUIColor_Red;
	Result.TextHighlightColor = GUIColor_Blue;

	Result.OutlineColor = GUIColor_Green;

	Result.FirstColor = GUIColor_PrettyBlue;
	Result.SecondaryColor = GUIColor_BluishGray;
#endif

	return(Result);
}

struct gui_state {
	font_info* FontInfo;
	render_stack* RenderStack;
	render_stack* TempRenderStack;

	float FontScale;

	input_system* Input;

	i32 ScreenWidth;
	i32 ScreenHeight;

	gui_element* CurrentNode;
	gui_element* CurrentTreeParent;
	gui_element* RootNode;
	gui_element* FreeElementsSentinel;
	gui_element* WalkaroundElement;
	b32 WalkaroundEnabled;
	b32 WalkaroundIsHot;

	gui_view* CurrentView;
	gui_view* ViewSentinel;
	gui_view* FreeViewSentinel;
	gui_view* DefaultView;

	stacked_memory GUIMem;

	b32 PlusMinusSymbol;

	//gui_interaction* HotInteraction;
	u32 HotInteractionID;

	v4 ColorTable[GUIColor_Count];
	gui_color_theme ColorTheme;
};

/*
inline v4 GUIGetThemeColor(gui_state* State, u32 Color) {
	v4 Result = State->ColorTable[State->ColorTheme[]]
}
*/

inline gui_view* GUIGetCurrentView(gui_state* GUIState) {
	gui_view* Result = 0;

	Result = GUIState->CurrentView;

	return(Result);
}

inline gui_element* GUIGetCurrentElement(gui_state* State) {
	gui_element* Result = State->CurrentNode;

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

inline b32 GUIInteractionIsHot(gui_state* State, gui_interaction* Interaction) {
	b32 Result = 0;

	if (Interaction->ID == State->HotInteractionID) {
		Result = 1;
	}

	return(Result);
}

inline u32 GUIStringHashFNV(char* Name) {
	u32 Result = 2166136261;

	char* At = Name;
	while (*At) {

		Result *= 16777619;
		Result ^= *At;

		At++;
	}

	return(Result);
}

inline u32 GUITreeElementID(gui_element* Element) {
	u32 Result = 1;

	gui_element* At = Element;

	//TODO(Dima): Better hash function
	while (At->Parent != 0) {
		Result *= At->ID;

		At = At->Parent;
	}

	return(Result);
}

inline void GUISwapWalkaroundHot(gui_state* State) {
	State->WalkaroundIsHot = !State->WalkaroundIsHot;
}

inline b32 GUIWalkaroundIsHot(gui_state* State) {
	b32 Result = State->WalkaroundIsHot;

	return(Result);
}

inline b32 GUISetInteractionHot(gui_state* State, gui_interaction* Interaction, b32 IsHot) {
	b32 Result = IsHot;

	if (IsHot) {
		State->HotInteractionID = Interaction->ID;
	}
	else {
		State->HotInteractionID = 0;
	}

	return(Result);
}

enum gui_window_creation_flags {
	GUIWindow_Resizable = 1,
	GUIWindow_Movable = 2,
	GUIWindow_Collapsible = 4,
	GUIWindow_DefaultSize = 8,

	GUIWindow_Fullscreen = 16,

	GUIWindow_TopBar = 32,
	GUIWindow_TopBar_Movable = 64,
	//GUIWindow_TopBar_Close,
	//GUIWindow_TopBar_PrintName,
};

extern void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height);
extern void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack);
extern void GUIEndFrame(gui_state* GUIState);

extern void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack);
extern void GUIEndTempRenderStack(gui_state* GUIState);

extern void GUIText(gui_state* GUIState, char* Text);
extern void GUIButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction);
extern void GUIBoolButton(gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUILabel(gui_state* GUIState, char* LabelText, v2 At);
extern void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction);
extern void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, gui_interaction* Interaction);
extern void GUIStackedMemGraph(gui_state* GUIState, char* Name, gui_interaction* Interaction);
extern void GUIImageView(gui_state* GUIState, char* Name, gui_interaction* Interaction);

extern void GUIWindow(gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height);

extern void GUIBeginView(gui_state* GUIState, char* ViewName, u32 ViewType);
extern void GUIEndView(gui_state* GUIState, u32 ViewType);
extern void GUIBeginRow(gui_state* State);
extern void GUIEndRow(gui_state* State);

extern b32 GUIBeginElement(gui_state* State, u32 ElementType, char* ElementName, gui_interaction* ElementInteraction);
extern void GUIEndElement(gui_state* State, u32 ElementType);

extern void GUITreeBegin(gui_state* State, char* NodeText);
extern void GUITreeEnd(gui_state* State);

#if 0
extern void GUIBeginTreeFind(gui_state* State, char* NodeName);
extern void GUIEndTreeFind(gui_state* State);
#endif
#endif