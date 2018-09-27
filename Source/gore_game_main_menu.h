#ifndef GORE_GAME_MAIN_MENU_H_INCLUDED
#define GORE_GAME_MAIN_MENU_H_INCLUDED

#include "gore_platform.h"
#include "gore_engine.h"

enum menu_element_type{
	MenuElement_Button,
};

struct menu_element_button {
	v4 ActiveColor;
	v4 InactiveColor;

	float TimeSinceDeactivation;
	float TimeForFadeout;

	char Text[32];
};

struct game_gui_entry {

	union {
		menu_element_button Button;
	}Element;

	game_gui_entry* NextInList;
	game_gui_entry* PrevInList;
};

struct main_menu_state {
	b32 IsInitialized;

	rect2 WindowRect;
};

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems);

#endif