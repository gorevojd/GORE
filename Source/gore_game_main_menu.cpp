#include "gore_game_main_menu.h"

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems) {
	main_menu_state* MenuState = (main_menu_state*)GameModeMemory->BaseAddress;

	if (!MenuState->IsInitialized) {

		MenuState = PushStruct(GameModeMemory, main_menu_state);

		MenuState->WindowRect = Rect2MinDim(
			V2(0.0f, 0.0f),
			V2(EngineSystems->RenderState->RenderWidth, EngineSystems->RenderState->RenderHeight));

		MenuState->IsInitialized = 1;
	}

	render_stack* MenuRenderStack = EngineSystems->RenderState->NamedStacks.GUI;

	RENDERPushRect(MenuRenderStack, MenuState->WindowRect, V4(0.3f, 0.3f, 0.3f, 1.0f));
}