#include "gore_game_main_menu.h"

menu_element* InitMenuChildrenElementInternal(main_menu_state* MenuState, menu_element* Parent, u32 MenuElementType) {
	menu_element* Result = PushStruct(MenuState->GameModeMemory, menu_element);

	*Result = {};

	Result->MenuElementType = MenuElementType;

	Result->NextInList = Parent->NextInList;
	Result->PrevInList = Parent;

	Result->NextInList->PrevInList = Result;
	Result->NextInList->PrevInList = Result;

	Result->ChildrenSentinel = PushStruct(MenuState->GameModeMemory, menu_element);
	Result->ChildrenSentinel->NextInList = Result->ChildrenSentinel;
	Result->ChildrenSentinel->PrevInList = Result->ChildrenSentinel;

	return(Result);
}

menu_element* InitMenuElementInternal(main_menu_state* MenuState, u32 MenuElementType) {
	menu_element* Result = InitMenuChildrenElementInternal(
		MenuState,
		&MenuState->MenuElementSentinel,
		MenuElementType);

	return(Result);
}

menu_element* InitMenuElementButton(main_menu_state* MenuState, char* Text) {
	menu_element* Result = InitMenuElementInternal(MenuState, MenuElement_Button);

	menu_element_button* ButtonData = &Result->Element.Button;

	ButtonData->ActiveColor = ColorFrom255(255, 33, 0);
	ButtonData->InactiveColor = ColorFrom255(58, 55, 55);

	ButtonData->TimeForFadeout = 0.5f;
	ButtonData->TimeSinceDeactivation = 999999.0f;

	CopyStrings(ButtonData->Text, Text);

	return(Result);
}

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems) {
	main_menu_state* MenuState = (main_menu_state*)GameModeMemory->BaseAddress;

	if (!MenuState->IsInitialized) {

		MenuState = PushStruct(GameModeMemory, main_menu_state);
		MenuState->GameModeMemory = GameModeMemory;

		MenuState->WindowRect = Rect2MinDim(
			V2(0.0f, 0.0f),
			V2(EngineSystems->RenderState->RenderWidth, EngineSystems->RenderState->RenderHeight));

		MenuState->MenuElementSentinel = {};
		MenuState->MenuElementSentinel.NextInList = &MenuState->MenuElementSentinel;
		MenuState->MenuElementSentinel.PrevInList = &MenuState->MenuElementSentinel;

		BeginVerticalMenu();
		BeginHorizontalMenu("Play");
		MenuElement("Geometrica");
		MenuElement("VoxelVorld");
		MenuElement("Lpter");
		EndMenu();
		MenuElement("Options");
		MenuElement("Exit");
		EndMenu();

		MenuState->IsInitialized = 1;
	}

	render_stack* MenuRenderStack = EngineSystems->RenderState->NamedStacks.GUI;

	RENDERPushRect(MenuRenderStack, MenuState->WindowRect, V4(0.1f, 0.1f, 0.1f, 1.0f));


}