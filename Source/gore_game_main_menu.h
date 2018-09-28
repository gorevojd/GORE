#ifndef GORE_GAME_MAIN_MENU_H_INCLUDED
#define GORE_GAME_MAIN_MENU_H_INCLUDED

#include "gore_platform.h"
#include "gore_engine.h"

/*
	NOTE(dima):
		1) Every element has it's own layout and
		can have childrens in it;

		2) 
*/

#define MENU_BUTTON_ACTION_TYPE(name) void name(void* ActionData)
typedef MENU_BUTTON_ACTION_TYPE(menu_button_action_fp);

enum menu_button_action_type {
	MenuButtonAction_None,

	MenuButtonAction_OpenTree,
	MenuButtonAction_Action,
};

enum menu_element_layout_type {
	MenuElementLayout_Horizontal,
	MenuElementLayout_Vertical,
};

enum menu_element_type{
	MenuElement_Root,

	MenuElement_Button,
	MenuElement_Layout,
};

struct menu_element_button {
	v4 ActiveColor;
	v4 InactiveColor;

	u32 ButtonActionType;

	float TimeSinceDeactivation;
	float TimeForFadeout;
};

struct menu_element_layout {
	u32 LayoutType;

	u32 ChildrenElementCount;

	float HorizontalFill01;
	float VerticalFill01;

	float ElementSpacingX;
	float ElementSpacingY;

	rect2 Rect;
};

inline menu_element_layout MenuInitLayout(
	u32 LayoutType, 
	float FillPercentageX01, 
	float FillPercentageY01) 
{
	menu_element_layout Result = {};

	Result.LayoutType = LayoutType;
	Result.ChildrenElementCount = 0;

	Result.HorizontalFill01 = FillPercentageX01;
	Result.VerticalFill01 = FillPercentageY01;

	Result.ElementSpacingX = 0.05f;
	Result.ElementSpacingY = 0.05f;

	Result.Rect = {};

	return(Result);
}

struct menu_element {
	char IdName[32];

	u32 MenuElementType;

	union {
		menu_element_button Button;
	}Element;

	menu_element_layout Layout;

	menu_element* NextInList;
	menu_element* PrevInList;

	menu_element* Parent;
	menu_element* ParentViewElement;

	menu_element* ChildrenSentinel;
};

enum menu_walkthrough_purpose {
	MenuWalkThrough_CalculateRects,
	MenuWalkThrough_Output,
};

struct main_menu_state {
	b32 IsInitialized;

	stacked_memory* GameModeMemory;

	rect2 WindowRect;

	menu_element RootElement;
	menu_element* CurrentElement;
	menu_element* ViewElement;

	engine_systems* EngineSystems;
};

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems);

#endif