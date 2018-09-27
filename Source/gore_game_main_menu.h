#ifndef GORE_GAME_MAIN_MENU_H_INCLUDED
#define GORE_GAME_MAIN_MENU_H_INCLUDED

#include "gore_platform.h"
#include "gore_engine.h"

enum menu_element_action_type {
	MenuElementAction_None,

	MenuElementAction_OpenTree,
	MenuElementAction_Action,
};

#define MENU_ELEMENT_ACTION(name) void name(void* ActionData)
typedef MENU_ELEMENT_ACTION(menu_element_action);

enum menu_element_type{
	MenuElement_Button,
};

struct menu_element_button {
	v4 ActiveColor;
	v4 InactiveColor;

	float TimeSinceDeactivation;
	float TimeForFadeout;

	rect2 Rect;

	char Text[32];
};

struct menu_element {

	u32 MenuElementType;

	union {
		menu_element_button Button;
	}Element;

	menu_element* NextInList;
	menu_element* PrevInList;

	menu_element* ChildrenSentinel;
};

struct main_menu_state {
	b32 IsInitialized;

	stacked_memory* GameModeMemory;

	rect2 WindowRect;

	menu_element MenuElementSentinel;
};

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems);

#endif