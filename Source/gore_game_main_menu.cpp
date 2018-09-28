#include "gore_game_main_menu.h"

static menu_element* InitMenuElementInternal(
	main_menu_state* MenuState, 
	u32 MenuElementType, 
	menu_element_layout Layout) 
{
	menu_element* Result = PushStruct(MenuState->GameModeMemory, menu_element);

	*Result = {};

	Result->MenuElementType = MenuElementType;

	Result->NextInList = MenuState->CurrentElement->ChildrenSentinel->NextInList;
	Result->PrevInList = MenuState->CurrentElement->ChildrenSentinel;

	Result->NextInList->PrevInList = Result;
	Result->NextInList->PrevInList = Result;

	Result->ChildrenSentinel = PushStruct(MenuState->GameModeMemory, menu_element);
	Result->ChildrenSentinel->NextInList = Result->ChildrenSentinel;
	Result->ChildrenSentinel->PrevInList = Result->ChildrenSentinel;

	Result->Parent = MenuState->CurrentElement;

	Result->Layout = Layout;

	//NOTE(dima): Incrementing parent children count
	Result->Parent->Layout.ChildrenElementCount++;

	return(Result);
}

static menu_element* InitMenuElementButton(
	main_menu_state* MenuState, 
	char* Text, 
	u32 ButtonActionType, 
	menu_element_layout Layout) 
{
	menu_element* Result = InitMenuElementInternal(MenuState, MenuElement_Button, Layout);

	//NOTE(dima): Initialization of button data
	menu_element_button* ButtonData = &Result->Element.Button;

	ButtonData->ActiveColor = ColorFrom255(255, 33, 0);
	ButtonData->InactiveColor = ColorFrom255(58, 55, 55);

	ButtonData->ButtonActionType = ButtonActionType;

	ButtonData->TimeForFadeout = 0.5f;
	ButtonData->TimeSinceDeactivation = 999999.0f;

	CopyStrings(ButtonData->Text, Text);

	return(Result);
}

static void MenuBeginElement(main_menu_state* MenuState, menu_element* LayoutElem) {
	MenuState->CurrentElement = LayoutElem;
}

static void MenuEndElement(main_menu_state* MenuState, u32 MenuElementType) {
	Assert(MenuState->CurrentElement->MenuElementType == MenuElementType);

	menu_element_layout* Layout = &MenuState->CurrentElement->Layout;

	v2 LayoutRectDim = GetRectDim(Layout->Rect);
	v2 TotalFillDim = V2(
		LayoutRectDim.x * Layout->HorizontalFill01,
		LayoutRectDim.y * Layout->VerticalFill01);

	v2 TotalFreeDim = LayoutRectDim - TotalFillDim;

	v2 ElementDim;
	v2 FreeBordersDim;
	v2 Increment;
	switch (Layout->LayoutType) {
		case MenuElementLayout_Horizontal: {
			ElementDim = V2(TotalFillDim.x / (float)Layout->ChildrenElementCount, TotalFillDim.y);
			FreeBordersDim = V2(TotalFreeDim.x / (float)(Layout->ChildrenElementCount + 1), TotalFreeDim.y * 0.5f);
			Increment = V2(ElementDim.x + FreeBordersDim.x, 0.0f);
		}break;

		case MenuElementLayout_Vertical: {
			ElementDim = V2(TotalFillDim.x, TotalFillDim.y / (float)Layout->ChildrenElementCount);
			FreeBordersDim = V2(TotalFreeDim.x * 0.5f, TotalFreeDim.y / (float)(Layout->ChildrenElementCount + 1));
			Increment = V2(0.0f, ElementDim.y + FreeBordersDim.y);
		}break;
	}

	v2 At = V2(FreeBordersDim.x, FreeBordersDim.y);

	menu_element* AtChildren = MenuState->CurrentElement->ChildrenSentinel->PrevInList;
	while (AtChildren != MenuState->CurrentElement->ChildrenSentinel) {
		AtChildren->Layout.Rect = Rect2MinDim(At, ElementDim);

		At += Increment;

		AtChildren = AtChildren->PrevInList;
	}

	MenuState->CurrentElement = MenuState->CurrentElement->Parent;
}

void MenuBeginButton(main_menu_state* MenuState, char* Text, menu_element_layout Layout) {
	menu_element* ButtonElement = InitMenuElementButton(
		MenuState, Text, 
		MenuButtonAction_OpenTree,
		Layout);

	MenuBeginElement(MenuState, ButtonElement);
}

void MenuEndButton(main_menu_state* MenuState) {
	MenuEndElement(MenuState, MenuElement_Button);
}

void MenuActionButton(
	main_menu_state* MenuState, 
	char* Text, 
	u32 ButtonActionType, 
	menu_button_action_fp* Action,
	void* ActionData,
	menu_element_layout Layout) 
{
	menu_element* ButtonElement = InitMenuElementButton(MenuState, Text, ButtonActionType, Layout);

	MenuBeginElement(MenuState, ButtonElement);
	MenuEndElement(MenuState, MenuElement_Button);
}

void MenuBeginLayout(main_menu_state* MenuState, menu_element_layout Layout) {
	menu_element* LayoutElem = InitMenuElementInternal(MenuState, MenuElement_Layout, Layout);

	MenuBeginElement(MenuState, LayoutElem);
}

void MenuEndLayout(main_menu_state* MenuState){
	MenuEndElement(MenuState, MenuElement_Layout);
}

static void MenuWalkThrough(main_menu_state* MenuState, menu_element* Elem) {
	menu_element* AtElem = Elem->ChildrenSentinel->NextInList;
	for (AtElem; AtElem != Elem->ChildrenSentinel; AtElem = AtElem->NextInList)
	{
		switch (AtElem->MenuElementType) {
			case MenuElement_Layout: {
				MenuWalkThrough(MenuState, AtElem);
			}break;

			case MenuElement_Button: {
				menu_element_button* ButtonData = &AtElem->Element.Button;

				input_system* Input = MenuState->EngineSystems->InputSystem;

				ButtonData->TimeSinceDeactivation += Input->DeltaTime;
				if (MouseInRect(Input, AtElem->Layout.Rect)) {
					ButtonData->TimeSinceDeactivation = 0.0f;
					

				}
				float FadeoutPercentage = ButtonData->TimeSinceDeactivation / ButtonData->TimeForFadeout;
				FadeoutPercentage = Clamp01(FadeoutPercentage);


			}break;

			default: {
				Assert(!"Invalid default value");
			}break;
		}
	}
}

void UpdateMainMenu(stacked_memory* GameModeMemory, engine_systems* EngineSystems) {
	main_menu_state* MenuState = (main_menu_state*)GameModeMemory->BaseAddress;

	if (!MenuState->IsInitialized) {

		MenuState = PushStruct(GameModeMemory, main_menu_state);
		MenuState->GameModeMemory = GameModeMemory;

		MenuState->EngineSystems = EngineSystems;

		MenuState->WindowRect = Rect2MinDim(
			V2(0.0f, 0.0f),
			V2(EngineSystems->RenderState->RenderWidth, EngineSystems->RenderState->RenderHeight));

		//NOTE(dima): Initialization of the root element
		MenuState->RootElement = {};
		MenuState->RootElement.NextInList = &MenuState->RootElement;
		MenuState->RootElement.PrevInList = &MenuState->RootElement;

		MenuState->RootElement.ChildrenSentinel = PushStruct(GameModeMemory, menu_element);
		MenuState->RootElement.ChildrenSentinel->NextInList = MenuState->RootElement.ChildrenSentinel;
		MenuState->RootElement.ChildrenSentinel->PrevInList = MenuState->RootElement.ChildrenSentinel;

		MenuState->RootElement.MenuElementType = MenuElement_Root;

		MenuState->RootElement.Layout = MenuInitLayout(MenuElementLayout_Vertical, 1.0f);

		MenuState->RootElement.Parent = 0;

		//NOTE(dima): Setting current element to the root
		MenuState->CurrentElement = &MenuState->RootElement;

		menu_element_layout DefaultLayout10 = MenuInitLayout(MenuElementLayout_Vertical, 1.0f);
		menu_element_layout DefaultLayout09 = MenuInitLayout(MenuElementLayout_Vertical, 0.9f);

		//NOTE(dima): This code should only be called once
		MenuBeginLayout(MenuState, DefaultLayout09);
		MenuBeginButton(MenuState, "Play", DefaultLayout10);

		MenuEndButton(MenuState);

		MenuEndLayout(MenuState);

		MenuState->IsInitialized = 1;
	}

	render_stack* MenuRenderStack = EngineSystems->RenderState->NamedStacks.GUI;

	RENDERPushRect(MenuRenderStack, MenuState->WindowRect, V4(0.1f, 0.1f, 0.1f, 1.0f));

}