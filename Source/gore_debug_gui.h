#ifndef GORE_GUI_H
#define GORE_GUI_H

#include "gore_render_state.h"

#include "gore_asset.h"
#include "gore_input.h"
#include "gore_colors.h"
#include "gore_font.h"

/*
	NOTE(dima): 
		It's not guaranteed that the gui code is thread safe
		so use it at your own risk in multithreaded code.

		My advice to you is that if you are going to use this
		code, make sure that you call all functions in one 
		thread.
*/

#define GUI_VALUE_VIEW_MULTIPLIER 8
#define GUI_VALUE_COLOR_VIEW_MULTIPLIER 5

struct debug_gui_state;

enum gui_variable_type {
	GUIVarType_F32,
	GUIVarType_U32,
	GUIVarType_I32,
	GUIVarType_U64,
	GUIVarType_I64,
	GUIVarType_B32,
	GUIVarType_STR,
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
		char* Value_STR;
		stacked_memory* Value_StackedMemory;
		bitmap_info* Value_RGBABuffer;
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
	v2 MinDim;
	v2 OffsetInAnchor;
	u32 Type;
};

struct gui_move_interaction_context {
	v2* MovePosition;
	u32 Type;
	v2 OffsetInAnchor;
};

struct gui_tree_interaction_context {
	struct gui_element* Elem;
	//rect2 ElemRc;
	v4* TextHighlightColor;
};

struct gui_bool_interaction_context {
	b32* InteractBool;
};

struct gui_menu_bar_interaction_context {
	gui_element* MenuElement;
};

struct gui_radio_button_interaction_context {
	gui_element* RadioGroup;
	u32 PressedIndex;
};

struct debug_gui_state_changer_group_interaction_context {
	gui_element* StateChangerGroup;
	u32 IncrementDirection;
};

enum gui_return_mouse_action_type {
	GUIReturnMouseAction_WentDown,
	GUIReturnMouseAction_WentUp,
	GUIReturnMouseAction_IsDown,
};

struct gui_return_mouse_action_interaction_context {
	b32* ActionHappened;

	u32 ActionType;
	u32 MouseButtonIndex;
	input_system* Input;
};

enum gui_interaction_type {
	GUIInteraction_None,

	GUIInteraction_VariableLink,
	GUIInteraction_Resize,
	GUIInteraction_Move,
	GUIInteraction_Tree,
	GUIInteraction_Bool,
	GUIInteraction_MenuBar,
	GUIInteraction_RadioButton,
	GUIInteraction_StateChangerGroup,
	GUIInteraction_ReturnMouseAction,
};

struct gui_interaction {
	u32 ID;

	u32 Type;
	union {
		gui_variable_link VariableLink;
		gui_resize_interaction_context ResizeContext;
		gui_move_interaction_context MoveContext;
		gui_tree_interaction_context TreeInteraction;
		gui_bool_interaction_context BoolInteraction;
		gui_menu_bar_interaction_context MenuMarInteraction;
		gui_radio_button_interaction_context RadioButtonInteraction;
		debug_gui_state_changer_group_interaction_context StateChangerGroupInteraction;
		gui_return_mouse_action_interaction_context ReturnMouseActionInteraction;
	};
};

#if 0
enum gui_interaction_rule_type {
	GUIInteractionRule_MouseClick,
	GUIInteractionRule_MouseOver,
	GUIInteractionRule_ButtonDown,
	GUIInteractionRule_ButtonUp,
};

struct gui_interaction_rule_mouse_click_data {
	u32 MouseButtonID;
};

struct gui_interaction_rule_button_data {
	u32 KeyID;
};

struct gui_interaction_rule_mouse_over_data {
	v2 MouseP;
};

struct gui_interaction_rule {
	u32 InteractionRuleType;

	union {
		gui_interaction_rule_mouse_click_data MouseClickData;
		gui_interaction_rule_mouse_over_data MouseOverData;
		gui_interaction_rule_button_data ButtonData;
	};
};
#endif


inline gui_variable_link GUIVariableLink(void* Variable, u32 Type) {
	gui_variable_link Link;

	switch (Type) {
		case GUIVarType_B32: {Link.Value_B32 = (b32*)Variable; }break;
		case GUIVarType_F32: {Link.Value_F32 = (float*)Variable; }break;
		case GUIVarType_I32: {Link.Value_I32 = (i32*)Variable; }break;
		case GUIVarType_U32: {Link.Value_U32 = (u32*)Variable; }break;
		case GUIVarType_I64: {Link.Value_I64 = (i64*)Variable; }break;
		case GUIVarType_U64: {Link.Value_U64 = (u64*)Variable; }break;
		case GUIVarType_STR: {Link.Value_STR = (char*)Variable; }break;
		case GUIVarType_StackedMemory: {
			Link.Value_StackedMemory = (stacked_memory*)Variable;
		}break;
		case GUIVarType_RGBABuffer: {
			Link.Value_RGBABuffer = (bitmap_info*)Variable;
		}break;
	}

	Link.Type = Type;

	return(Link);
}

inline gui_interaction GUIVariableInteraction(void* Variable, u32 Type) {
	gui_interaction Result = {};

	Result.VariableLink = GUIVariableLink(Variable, Type);
	Result.Type = GUIInteraction_VariableLink;
	Result.ID = 0;

	return(Result);
}

/* Type is the value of enum: gui_resize_interaction_type */
inline gui_interaction GUIResizeInteraction(v2 Position, v2* DimensionPtr, u32 Type) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Resize;

	Result.ResizeContext.DimensionPtr = DimensionPtr;
	Result.ResizeContext.Position = Position;
	Result.ResizeContext.Type = Type;

	return(Result);
}

/* Type is the value of enum: gui_move_interaction_type */
inline gui_interaction GUIMoveInteraction(v2* MovePosition, u32 Type) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Move;

	Result.MoveContext.MovePosition = MovePosition;
	Result.MoveContext.Type = Type;

	return(Result);
}

inline gui_interaction GUITreeInteraction(struct gui_element* Elem, rect2 ElemRc) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Tree;

	Result.TreeInteraction.Elem = Elem;

	return(Result);
}

inline gui_interaction GUIBoolInteraction(b32* InteractBool) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_Bool;

	Result.BoolInteraction.InteractBool = InteractBool;

	return(Result);
}

inline gui_interaction GUIMenuBarInteraction(gui_element* MenuElement) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_MenuBar;
	Result.MenuMarInteraction.MenuElement = MenuElement;

	return(Result);
}

inline gui_interaction GUIRadioButtonInteraction(gui_element* RadioGroup, u32 PressedIndex) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_RadioButton;
	Result.RadioButtonInteraction.RadioGroup = RadioGroup;
	Result.RadioButtonInteraction.PressedIndex = PressedIndex;

	return(Result);
}


/*
	IncrementDirection: 0 -> Forward; 1 -> Backward;
*/
inline gui_interaction GUIStateChangerGroupInteraction(gui_element* StateChangerGroup, u32 IncrementDirection) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_StateChangerGroup;
	Result.StateChangerGroupInteraction.StateChangerGroup = StateChangerGroup;
	Result.StateChangerGroupInteraction.IncrementDirection = IncrementDirection;

	return(Result);
}

inline gui_interaction GUIReturnMouseActionInteraction(input_system* Input, b32* ActionHappened, u32 ActionType, u32 MouseButtonIndex) {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_ReturnMouseAction;
	Result.ReturnMouseActionInteraction.Input = Input;
	Result.ReturnMouseActionInteraction.MouseButtonIndex = MouseButtonIndex;
	Result.ReturnMouseActionInteraction.ActionType = ActionType;
	Result.ReturnMouseActionInteraction.ActionHappened = ActionHappened;

	return(Result);
}

inline gui_interaction GUINullInteraction() {
	gui_interaction Result = {};

	Result.Type = GUIInteraction_None;

	return(Result);
}

enum gui_layout_type {
	GUILayout_Simple,
	GUILayout_Tree,
};

struct gui_layout {
	u32 ID;

	u32 ViewType;

	float CurrentX;
	float CurrentY;

	v2 LastElementP;
	v2 LastElementDim;

	float RowBeginX;
	float RowBiggestHeight;

	float CurrentPreAdvance;

	gui_layout* Parent;
	gui_layout* NextBro;
	gui_layout* PrevBro;

	int BeginnedRowsCount;
	b32 NeedHorizontalAdvance;
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
	GUIElement_Layout,
	GUIElement_RadioGroup,
	GUIElement_StateChangerGroup,

	GUIElement_MenuBar,
	GUIElement_MenuItem,
};

enum gui_tree_node_exit_state {
	GUITreeNodeExit_None, 
	GUITreeNodeExit_Exiting,
	GUITreeNodeExit_Finished,
};

enum gui_tree_node_out_flags {
	GUITreeOutFlag_MouseOverRect = 1,
	GUITreeOutFlag_MouseLeftClickedInRect = 2,
};

struct gui_tree_node_cache {
	u32 ExitState;

	u32 OutFlags;

	float StackBeginY;
	float StackY;
	float ExitY;
};

struct gui_anchor_cache {
	v2 OffsetInAnchor;
};

struct gui_menu_node_cache{
	float MaxWidth;
	float MaxHeight;

	float SumHeight;
	float SumWidth;

	u32 ChildrenCount;
	u32 Type;
};

struct gui_radio_group_cache {
	u32 ActiveIndex;
};

struct gui_radio_button_cache {
	b32 IsActive;
};

struct debug_gui_state_changer_group_cache {
	gui_element* ActiveElement;
};

struct debug_gui_state_changer_cache {
	u32 StateID;
};

struct gui_dimensional_cache {
	v2 Pos;
	v2 Dim;
};

struct gui_element_cache {
	union {
		gui_tree_node_cache TreeNode;
		gui_anchor_cache Anchor;
		gui_menu_node_cache MenuNode;
		gui_radio_group_cache RadioCache;
		gui_radio_button_cache RadioButton;
		debug_gui_state_changer_group_cache StateChangerGroupCache;
		debug_gui_state_changer_cache StateChangerCache;
		gui_dimensional_cache Dimensional;
	};

	b32 IsInitialized;
};

struct gui_element {
	u32 ID;

	s32 Depth;

	gui_element* Parent;

	//NOTE(Dima): Used for remembering last tree parent for tree nodes
	gui_element* TempParentTree;

	gui_element* NextBro;
	gui_element* PrevBro;

	gui_element* ChildrenSentinel;

	b32 Expanded;
	char Name[32];
	char Text[32];

	u16 RowCount;

	u32 Type;

	gui_element_cache Cache;
};

struct gui_color_theme {

	u32 TextColor;
	u32 TextHighlightColor;
	u32 TooltipTextColor;
	u32 CommentColor;

	u32 OutlineColor;

	float GraphAlpha;
	u32 GraphColor1;
	u32 GraphColor2;
	u32 GraphColor3;
	u32 GraphColor4;
	u32 GraphColor5;
	u32 GraphColor6;
	u32 GraphColor7;
	u32 GraphColor8;
	u32 GraphBackColor;

	u32 LogColor;
	u32 ErrLogColor;
	u32 OkLogColor;
	u32 WarningLogColor;

	u32 PlayColor;
	u32 PauseColor;

	u32 ButtonTextColor;
	u32 ButtonBackColor;
	u32 ButtonTextHighColor;
	u32 ButtonTextHighColor2;
	u32 ButtonOutlineColor;

	u32 AnchorColor;

	u32 WalkaroundColor;
	u32 WalkaroundHotColor;
};

inline gui_color_theme GUIDefaultColorTheme() {
	gui_color_theme Result;

#if 1
	Result.TextColor = ColorExt_burlywood;
	Result.TextHighlightColor = Color_Yellow;
	Result.TooltipTextColor = Color_White;
	Result.CommentColor = ColorExt_gray50;

	Result.OutlineColor = Color_Black;

	Result.WalkaroundColor = Color_Yellow;
	Result.WalkaroundHotColor = Color_Red;

	Result.LogColor = Result.TextColor;
	Result.OkLogColor = ColorExt_OliveDrab;
	Result.WarningLogColor = ColorExt_DarkGoldenrod1;
	Result.ErrLogColor = ColorExt_red3;

	Result.PlayColor = ColorExt_DarkGoldenrod1;
	Result.PauseColor = Result.CommentColor;

	Result.ButtonTextColor = ColorExt_burlywood;
	Result.ButtonBackColor = ColorExt_gray10;
	Result.ButtonTextHighColor = ColorExt_DarkGoldenrod1;
	Result.ButtonOutlineColor = Color_Black;
	Result.ButtonTextHighColor2 = Result.TextHighlightColor;
#else
	Result.TextColor = ColorExt_cyan4;
	Result.TextHighlightColor = Color_Cyan;
	Result.TooltipTextColor = Color_White;
	Result.CommentColor = ColorExt_gray50;

	Result.OutlineColor = Color_Black;

	Result.WalkaroundColor = Color_Yellow;
	Result.WalkaroundHotColor = Color_Red;

	Result.LogColor = Result.TextColor;
	Result.OkLogColor = ColorExt_OliveDrab;
	Result.WarningLogColor = ColorExt_DarkGoldenrod1;
	Result.ErrLogColor = ColorExt_red3;

	Result.PlayColor = ColorExt_DarkGoldenrod1;
	Result.PauseColor = Result.CommentColor;

	Result.ButtonTextColor = ColorExt_cyan4;
	Result.ButtonBackColor = ColorExt_gray5;
	Result.ButtonTextHighColor = Color_White;
	Result.ButtonOutlineColor = Color_Black;
	Result.ButtonTextHighColor2 = Result.TextHighlightColor;
#endif

	Result.GraphAlpha = 0.75f;
	Result.GraphColor1 = ColorExt_green3;
	Result.GraphColor2 = ColorExt_purple1;
	Result.GraphColor3 = ColorExt_red3;
	Result.GraphColor4 = ColorExt_blue1;
	Result.GraphColor5 = ColorExt_orange2;
	Result.GraphColor6 = Color_PrettyBlue;
	Result.GraphColor7 = ColorExt_DarkGoldenrod3;
	Result.GraphColor8 = ColorExt_chartreuse3;

	Result.GraphBackColor = Color_Black;

	Result.AnchorColor = ColorExt_OrangeRed1;

	return(Result);
}

struct debug_gui_state {
	font_info* FontInfo;

	render_state* RenderState;
	render_stack* RenderStack;

	float FontScale;
	float LastFontScale;

	b32 TextElemsCacheShouldBeReinitialized;

	input_system* Input;
	asset_system* AssetSystem;

	i32 ScreenWidth;
	i32 ScreenHeight;

	gui_element* CurrentNode;
	gui_element* CurrentTreeParent;
	gui_element* RootNode;
	gui_element* FreeElementsSentinel;
	gui_element* WalkaroundElement;
	b32 WalkaroundEnabled;
	b32 WalkaroundIsHot;

	gui_layout* CurrentLayout;
	gui_layout* LayoutSentinel;
	gui_layout* FreeLayoutSentinel;
	gui_layout* DefaultLayout;

	stacked_memory* GUIMem;
	stacked_memory RenderMemorySplit;

	b32 PlusMinusSymbol;

	//gui_interaction* HotInteraction;
	u32 HotInteractionID;

#define GUI_TOOLTIPS_MAX_COUNT 64
	char Tooltips[GUI_TOOLTIPS_MAX_COUNT][256];
	int TooltipCount;

	color_state* ColorsState;
	gui_color_theme ColorTheme;
};


inline v4 GUIGetColor(debug_gui_state* GUIState, u32 ColorIndex) {
	v4 Result = GetColor(GUIState->ColorsState, ColorIndex);

	return(Result);
}
/*
inline v4 GUIGetThemeColor(debug_gui_state* State, u32 Color) {
	v4 Result = State->ColorTable[State->ColorTheme[]]
}
*/

inline gui_layout* GUIGetCurrentLayout(debug_gui_state* GUIState) {
	gui_layout* Result = 0;

	Result = GUIState->CurrentLayout;

	return(Result);
}

inline gui_element* GUIGetCurrentElement(debug_gui_state* State) {
	gui_element* Result = State->CurrentNode;

	return(Result);
}

inline b32 GUIElementShouldBeUpdated(gui_element* Node) {
	FUNCTION_TIMING();

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

inline b32 GUIInteractionIsHot(debug_gui_state* State, gui_interaction* Interaction) {
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

inline void GUISwapWalkaroundHot(debug_gui_state* State) {
	State->WalkaroundIsHot = !State->WalkaroundIsHot;
}

inline b32 GUIWalkaroundIsHot(debug_gui_state* State) {
	b32 Result = State->WalkaroundIsHot;

	return(Result);
}

inline b32 GUISetInteractionHot(debug_gui_state* State, gui_interaction* Interaction, b32 IsHot) {
	b32 Result = IsHot;

	if (IsHot) {
		State->HotInteractionID = Interaction->ID;
	}
	else {
		State->HotInteractionID = 0;
	}

	return(Result);
}

#if 0
enum gui_align_type {
	GUIAlign_Begin,
	GUIAlign_Center,
	GUIAlign_End,
};

inline float GUIGetAlignValueForType(u32 AlignType) {
	float Result = 0.0f;

	if (AlignType == GUIAlign_Begin) {
		Result = 0.0f;
	}
	else if (AlignType == GUIAlign_Center) {
		Result = 0.5f;
	}
	else if (AlignType == GUIAlign_End) {
		Result = 1.0f;
	}
	else {
		Assert(!"Invalid align type")
	}

	return(Result);
}
#endif

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

enum gui_menu_item_type {
	GUIMenuItem_MenuBarItem,
	GUIMenuItem_MenuItem,
};

extern void GUIInitState(
	debug_gui_state* GUIState, 
	stacked_memory* GUIMemory, 
	color_state* ColorState, 
	asset_system* AssetSystem,
	input_system* Input, 
	render_state* RenderState);

extern void GUIBeginFrame(debug_gui_state* GUIState);
extern void GUIPrepareFrame(debug_gui_state* GUIState);
extern void GUIEndFrame(debug_gui_state* GUIState);

extern rect2 GUIPrintText(debug_gui_state* GUIState, char* Text, v2 P, float Scale, v4 Color);
extern rect2 GUIPrintTextMultiline(debug_gui_state* GUIState, char* Text, v2 P, float Scale, v4 Color);
extern v2 GUIGetTextSize(debug_gui_state* GUIState, char* Text, float Scale);
extern v2 GUIGetTextSizeMultiline(debug_gui_state* GUIState, char* Text, float Scale);

extern void GUIPerformInteraction(debug_gui_state* GUIState, gui_interaction* Interaction);

extern void GUIText(debug_gui_state* GUIState, char* Text);
extern b32 GUIButton(debug_gui_state* GUIState, char* ButtonName);
extern b32 GUIButtonAt(debug_gui_state* GUIState, char* ButtonName, v2 At, rect2* ButtonRect = 0, v4* TextColor = 0);
extern void GUIBoolButton(debug_gui_state* GUIState, char* ButtonName, b32* Value);
extern void GUIBoolButton2(debug_gui_state* GUIState, char* ButtonName, b32* Value);
extern void GUIBoolChecker(debug_gui_state* GUIState, char* Name, b32* Value);
extern void GUIActionText(debug_gui_state* GUIState, char* Text, gui_interaction* Interaction);
extern void GUITooltip(debug_gui_state* GUIState, char* TooltipText);
extern void GUILabel(debug_gui_state* GUIState, char* LabelText, v2 At);
extern void GUISlider(debug_gui_state* GUIState, char* Name, float Min, float Max, float* InteractValue);
extern void GUIVerticalSlider(debug_gui_state* State, char* Name, float Min, float Max, float* InteractValue);
extern void GUIStackedMemGraph(debug_gui_state* GUIState, char* Name, stacked_memory* MemoryStack);
extern void GUIImageView(debug_gui_state* GUIState, char* Name, bitmap_info* Buffer);
extern void ColorView(debug_gui_state* GUIState, v4 Color, char* Name);
extern void GUIVector2View(debug_gui_state* GUIState, v2 Value, char* Name);
extern void GUIVector3View(debug_gui_state* GUIState, v3 Value, char* Name);
extern void GUIVector4View(debug_gui_state* GUIState, v4 Value, char* Name);
extern void GUIInt32View(debug_gui_state* GUIState, i32 Int, char* Name);

extern void GUIAnchor(debug_gui_state* GUIState, char* Name, v2 Pos, v2 Dim, gui_interaction* Interaction, b32 Centered = 1);

extern void GUIWindow(debug_gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height);

extern void GUIBeginMenuBar(debug_gui_state* GUIState, char* MenuName);
extern void GUIEndMenuBar(debug_gui_state* GUIState);
extern void GUIBeginMenuBarItem(debug_gui_state* GUIState, char* Name);
extern void GUIEndMenuBarItem(debug_gui_state* GUIState);
extern void GUIMenuBarItem(debug_gui_state* GUIState, char* ItemName);

extern void GUIBeginLayout(debug_gui_state* GUIState, char* LayoutName, u32 LayoutType);
extern void GUIChangeTreeNodeText(debug_gui_state* GUIState, char* Text);
extern void GUIEndLayout(debug_gui_state* GUIState, u32 LayoutType);
extern void GUIBeginRow(debug_gui_state* State);
extern void GUIEndRow(debug_gui_state* State);

extern gui_element* GUIBeginElement(
	debug_gui_state* State,
	u32 ElementType,
	char* ElementName,
	gui_interaction* ElementInteraction,
	b32 InitExpandedState = 0,
	b32 IncrementDepth = 1);
extern void GUIEndElement(debug_gui_state* State, u32 ElementType);

extern void GUIPreAdvanceCursor(debug_gui_state* State);
extern void GUIDescribeElement(debug_gui_state* State, v2 ElementDim, v2 ElementP);
extern void GUIAdvanceCursor(debug_gui_state* State, float AdditionalYSpacing = 0.0f);

extern void GUITreeBegin(debug_gui_state* State, char* NodeText, char* NameText = 0);
extern u32 GUITreeGetOutFlags(debug_gui_state* GUIState);
extern void GUITreeEnd(debug_gui_state* State);


extern void GUIBeginRadioGroup(debug_gui_state* GUIState, char* Name, u32 DefaultSetIndex);
extern void GUIRadioButton(debug_gui_state* GUIState, char* Name, u32 UniqueIndex);
extern void GUIEndRadioGroup(debug_gui_state* GUIState, u32* ActiveElement);

void GUIBeginStateChangerGroup(debug_gui_state* GUIState, char* Name, u32 DefaultSetIndex);
void GUIStateChanger(debug_gui_state* GUIState, char* Name, u32 StateID);
void GUIEndStateChangerGroupAt(debug_gui_state* GUIState, v2 Pos, u32* ActiveElement);

extern rect2 GUITextBase(
	debug_gui_state* GUIState,
	char* Text,
	v2 Pos,
	v4 TextColor = V4(1.0f, 1.0f, 1.0f, 1.0f),
	float FontScale = 1.0f,
	v4 TextHighlightColor = V4(1.0f, 1.0f, 1.0f, 1.0f),
	v4 BackgroundColor = V4(0.0f, 0.5f, 1.0f, 1.0f),
	u32 OutlineWidth = 0,
	v4 OutlineColor = V4(0.0f, 0.0f, 0.0f, 1.0f));

#endif