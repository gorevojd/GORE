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

	char Text[32];
};

struct menu_element_layout {
	u32 LayoutType;

	u32 ChildrenElementCount;

	float HorizontalFill01;
	float VerticalFill01;

	rect2 Rect;
};

inline menu_element_layout MenuInitLayout(u32 LayoutType, float FillPercentage01) {
	menu_element_layout Result = {};

	Result.ChildrenElementCount = 0;
	Result.HorizontalFill01 = FillPercentage01;
	Result.VerticalFill01 = FillPercentage01;
	Result.Rect = {};

	return(Result);
}

struct menu_element {

	u32 MenuElementType;

	union {
		menu_element_button Button;
	}Element;

	menu_element_layout Layout;

	menu_element* NextInList;
	menu_element* PrevInList;

	menu_element* Parent;

	menu_element* ChildrenSentinel;
};

struct main_menu_state {
	b32 IsInitialized;

	stacked_memory* GameModeMemory;

	rect2 WindowRect;

	menu_element RootElement;
	menu_element* CurrentElement;

	engine_systems* EngineSystems;
};

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems);

#endif