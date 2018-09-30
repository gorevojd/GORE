#include "gore_game_main_menu.h"

static menu_element* InitMenuElementInternal(
	main_menu_state* MenuState,
	char* IdName,
	u32 MenuElementType,
	b32 ShouldIncrementChildrenCount,
	menu_element_layout Layout) 
{
	menu_element* Result = PushStruct(MenuState->GameModeMemory, menu_element);

	*Result = {};

	Result->MenuElementType = MenuElementType;

	Result->NextInList = MenuState->CurrentElement->ChildrenSentinel->NextInList;
	Result->PrevInList = MenuState->CurrentElement->ChildrenSentinel;

	Result->NextInList->PrevInList = Result;
	Result->PrevInList->NextInList = Result;

	Result->ChildrenSentinel = PushStruct(MenuState->GameModeMemory, menu_element);
	Result->ChildrenSentinel->NextInList = Result->ChildrenSentinel;
	Result->ChildrenSentinel->PrevInList = Result->ChildrenSentinel;
	
	CopyStrings(Result->IdName, IdName);
	CopyStrings(Result->ChildrenSentinel->IdName, "Sentinel");

	Result->Parent = MenuState->CurrentElement;

	Result->Layout = Layout;

	//NOTE(dima): Incrementing parent children count
	if (ShouldIncrementChildrenCount) {
		Result->Parent->Layout.ChildrenElementCount++;
	}
	else {
		Result->Parent->Layout.NotIncrementtedCount++;
	}

	return(Result);
}

static menu_element* InitMenuElementButton(
	main_menu_state* MenuState, 
	char* IdName, 
	u32 ButtonActionType, 
	u32 ActiveColorIndex,
	menu_element_layout Layout)
{
	menu_element* Result = InitMenuElementInternal(MenuState, IdName, MenuElement_Button, 1, Layout);

	//NOTE(dima): Initialization of button data
	menu_element_button* ButtonData = &Result->Element.Button;

	ButtonData->ActiveColor = GetColor(MenuState->EngineSystems->ColorsState, ActiveColorIndex);
	//ButtonData->InactiveColor = ColorFrom255(58, 55, 55);
	ButtonData->InactiveColor = ButtonData->ActiveColor * 
		V4(MenuState->InactiveColorPercentage, 
			MenuState->InactiveColorPercentage, 
			MenuState->InactiveColorPercentage, 
			1.0f);

	ButtonData->ButtonActionType = ButtonActionType;


	ButtonData->TimeForFadeout = 0.4f;
	ButtonData->TimeSinceDeactivation = 999999.0f;

	return(Result);
}

static void MenuBeginElement(main_menu_state* MenuState, menu_element* LayoutElem) {
	MenuState->CurrentElement = LayoutElem;
}

static void MenuEndElement(main_menu_state* MenuState, u32 MenuElementType) {
	Assert(MenuState->CurrentElement->MenuElementType == MenuElementType);

	MenuState->CurrentElement = MenuState->CurrentElement->Parent;
}

void MenuBeginButton(
	main_menu_state* MenuState, 
	char* Text, 
	u32 ActiveColorIndex,
	menu_element_layout Layout) 
{
	menu_element* ButtonElement = InitMenuElementButton(
		MenuState, Text, 
		MenuButtonAction_OpenTree,
		ActiveColorIndex,
		Layout);

	MenuBeginElement(MenuState, ButtonElement);
}

void MenuEndButton(main_menu_state* MenuState) {
	MenuEndElement(MenuState, MenuElement_Button);
}

void MenuActionButton(
	main_menu_state* MenuState, 
	char* Text, 
	menu_button_action_fp* Action,
	void* ActionData,
	u32 ActiveColorIndex,
	menu_element_layout Layout) 
{
	menu_element* ButtonElement = InitMenuElementButton(
		MenuState, Text, 
		MenuButtonAction_Action, 
		ActiveColorIndex, Layout);

	MenuBeginElement(MenuState, ButtonElement);
	MenuEndElement(MenuState, MenuElement_Button);
}

void MenuBeginLayout(main_menu_state* MenuState, menu_element_layout Layout) {
	char TempBuf[32];
	IntegerToString(MenuState->CurrentElement->Layout.ChildrenElementCount, TempBuf);
	char LayoutIDName[32];
	ConcatStringsUnsafe(LayoutIDName, "Layout", TempBuf);

	menu_element* LayoutElem = InitMenuElementInternal(
		MenuState, 
		LayoutIDName, 
		MenuElement_Layout, 
		1,
		Layout);

	LayoutElem->Element.LayoutData = {};
	LayoutElem->Element.LayoutData.InitRectIsSet = 0;
	LayoutElem->Element.LayoutData.InitRect = {};

	MenuBeginElement(MenuState, LayoutElem);
}

void MenuBeginRectLayout(main_menu_state* MenuState, rect2 Rect, menu_element_layout Layout) {
	char TempBuf[32];
	IntegerToString(MenuState->CurrentElement->Layout.NotIncrementtedCount, TempBuf);
	char LayoutIDName[32];
	ConcatStringsUnsafe(LayoutIDName, "RectLayout", TempBuf);

	menu_element* LayoutElem = InitMenuElementInternal(
		MenuState,
		LayoutIDName,
		MenuElement_Layout,
		0,
		Layout);

	LayoutElem->Element.LayoutData = {};
	LayoutElem->Element.LayoutData.InitRectIsSet = 1;
	LayoutElem->Element.LayoutData.InitRect = Rect;

	MenuBeginElement(MenuState, LayoutElem);
}

void MenuEndLayout(main_menu_state* MenuState){
	MenuEndElement(MenuState, MenuElement_Layout);
}

static void MenuWalkThrough(main_menu_state* MenuState, menu_element* Elem, u32 WalkThroughPurpose) {
	switch (WalkThroughPurpose) {
		case MenuWalkThrough_CalculateRects: {
			menu_element_layout* Layout = &Elem->Layout;

			v2 LayoutRectDim = GetRectDim(Layout->Rect);
			v2 TotalFillDim = V2(
				LayoutRectDim.x * Layout->HorizontalFill01,
				LayoutRectDim.y * Layout->VerticalFill01);

			v2 TotalFreeDim = LayoutRectDim - TotalFillDim;
			v2 FreeBordersDim = V2(TotalFreeDim.x * 0.5f, TotalFreeDim.y * 0.5f);

			v2 ElemsSpacingDim = TotalFillDim * V2(Layout->ElementSpacingX, Layout->ElementSpacingY);

			v2 ElementDim;
			v2 Increment;

			if (Layout->ChildrenElementCount > 1) {
				v2 ElemsArea = TotalFillDim - ElemsSpacingDim;
				v2 OneSpacingDim = ElemsSpacingDim / (float)(Layout->ChildrenElementCount - 1);

				switch (Layout->LayoutType) {
					case MenuElementLayout_Horizontal: {
						ElementDim = V2(ElemsArea.x / (float)Layout->ChildrenElementCount, TotalFillDim.y);
						Increment = V2(ElementDim.x + OneSpacingDim.x, 0.0f);
					}break;

					case MenuElementLayout_Vertical: {
						ElementDim = V2(TotalFillDim.x, ElemsArea.y / (float)Layout->ChildrenElementCount);
						Increment = V2(0.0f, ElementDim.y + OneSpacingDim.y);
					}break;
				}
			}
			else {
				ElementDim = TotalFillDim;
				FreeBordersDim = V2(0.0f, 0.0f);
				Increment = V2(0.0f, 0.0f);
			}

			v2 At = Elem->Layout.Rect.Min + FreeBordersDim;

			menu_element* AtChildren = Elem->ChildrenSentinel->PrevInList;
			while (AtChildren != Elem->ChildrenSentinel) {



				AtChildren->Layout.Rect = Rect2MinDim(At, ElementDim);

				At += Increment;

				MenuWalkThrough(MenuState, AtChildren, WalkThroughPurpose);

				AtChildren = AtChildren->PrevInList;
			}
		}break;

		case MenuWalkThrough_Output: {
			render_stack* RenderStack = MenuState->EngineSystems->RenderState->NamedStacks.GUI;

			menu_element* AtElem = Elem->ChildrenSentinel->NextInList;
			for (AtElem; AtElem != Elem->ChildrenSentinel; AtElem = AtElem->NextInList)
			{
				switch (AtElem->MenuElementType) {
					case MenuElement_Layout: {
						MenuWalkThrough(MenuState, AtElem, WalkThroughPurpose);
					}break;

					case MenuElement_Button: {
						menu_element_button* ButtonData = &AtElem->Element.Button;

						input_system* Input = MenuState->EngineSystems->InputSystem;

						ButtonData->TimeSinceDeactivation += Input->DeltaTime;
						if (MouseInRect(Input, AtElem->Layout.Rect)) {
							ButtonData->TimeSinceDeactivation = 0.0f;

							if (MouseButtonWentDown(Input, MouseButton_Left)) {
								switch (ButtonData->ButtonActionType) {
									case MenuButtonAction_None: {

									}break;

									case MenuButtonAction_Action: {

									}break;

									case MenuButtonAction_OpenTree: {
										AtElem->ParentViewElement = Elem;
										MenuState->ViewElement = AtElem;
									}break;
								}
							}
						}
						float FadeoutPercentage = ButtonData->TimeSinceDeactivation / ButtonData->TimeForFadeout;
						FadeoutPercentage = Clamp01(FadeoutPercentage);

						v4 ButtonColor = Lerp(ButtonData->ActiveColor, ButtonData->InactiveColor, FadeoutPercentage);
						v4 OutlineColor = V4(0.0f, 0.0f, 0.0f, 1.0f);

						RENDERPushRect(RenderStack, AtElem->Layout.Rect, ButtonColor);
						RENDERPushRectOutline(RenderStack, AtElem->Layout.Rect, 2, OutlineColor);

						v4 ButtonTextColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
						v4 ResultTextColor = Lerp(
							ButtonTextColor, 
							Hadamard(ButtonTextColor, 
								V4(
									MenuState->InactiveColorPercentage, 
									MenuState->InactiveColorPercentage, 
									MenuState->InactiveColorPercentage, 
									1.0f)),
							FadeoutPercentage);

						PrintTextCenteredInRect(
							RenderStack,
							MenuState->MainFontInfo,
							AtElem->IdName,
							AtElem->Layout.Rect,
							1.0f,
							ResultTextColor);
					}break;

					default: {
						Assert(!"Invalid default value");
					}break;
				}
			}
		}break;
	}
}

static void FinalizeMenuCreation(main_menu_state* MenuState) {
	MenuWalkThrough(MenuState, MenuState->CurrentElement, MenuWalkThrough_CalculateRects);
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

		//NOTE(dima): Loading needed fonts
		font_id FontID = GetFirstFont(EngineSystems->AssetSystem, GameAsset_MainMenuFont);
		MenuState->MainFontInfo = GetFontFromID(EngineSystems->AssetSystem, FontID);

		MenuState->InactiveColorPercentage = 0.5f;

		//NOTE(dima): Initialization of the root element
		MenuState->RootElement = {};
		MenuState->RootElement.NextInList = &MenuState->RootElement;
		MenuState->RootElement.PrevInList = &MenuState->RootElement;

		MenuState->RootElement.ChildrenSentinel = PushStruct(GameModeMemory, menu_element);
		MenuState->RootElement.ChildrenSentinel->NextInList = MenuState->RootElement.ChildrenSentinel;
		MenuState->RootElement.ChildrenSentinel->PrevInList = MenuState->RootElement.ChildrenSentinel;
		CopyStrings(MenuState->RootElement.ChildrenSentinel->IdName, "Sentinel");

		MenuState->RootElement.MenuElementType = MenuElement_Root;

		MenuState->RootElement.Layout = MenuInitLayout(MenuElementLayout_Vertical, 1.0f, 1.0f);
		MenuState->RootElement.Layout.Rect = MenuState->WindowRect;

		MenuState->RootElement.Parent = 0;
		CopyStrings(MenuState->RootElement.IdName, "Root");

		//NOTE(dima): Setting current element to the root
		MenuState->CurrentElement = &MenuState->RootElement;

		//NOTE(dima): Setting viewing element to the root element
		MenuState->ViewElement = &MenuState->RootElement;

		menu_element_layout DefaultLayout10 = MenuInitLayout(MenuElementLayout_Vertical, 1.0f, 1.0f);
		menu_element_layout DefaultLayout09 = MenuInitLayout(MenuElementLayout_Horizontal, 0.9, 0.9f);

		//NOTE(dima): This code should only be called once
		MenuBeginLayout(MenuState, DefaultLayout09);

		MenuBeginButton(MenuState, "Play", Color_Green, DefaultLayout10);
		MenuBeginRectLayout(MenuState, MenuState->WindowRect, DefaultLayout10);
		MenuActionButton(MenuState, "Geometrica", 0, 0, Color_Green, DefaultLayout10);
		MenuActionButton(MenuState, "VoxelWorld", 0, 0, Color_Green, DefaultLayout10);
		MenuActionButton(MenuState, "LpterWorld", 0, 0, Color_Green, DefaultLayout10);
		MenuEndLayout(MenuState);
		MenuEndButton(MenuState);

#if 0
		MenuBeginLayout(MenuState, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode11", 0, 0, Color_Blue, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode12", 0, 0, Color_Orange, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode13", 0, 0, Color_Purple, DefaultLayout10);
		MenuEndLayout(MenuState);

		MenuBeginLayout(MenuState, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode1", 0, 0, Color_Red, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode2", 0, 0, Color_Red, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode3", 0, 0, Color_Red, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode5", 0, 0, Color_Red, DefaultLayout10);
		MenuEndLayout(MenuState);
#else
		MenuBeginButton(MenuState, "Options", Color_Blue, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode11", 0, 0, Color_Blue, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode12", 0, 0, Color_Orange, DefaultLayout10);
		MenuActionButton(MenuState, "GameMode13", 0, 0, Color_Purple, DefaultLayout10);
		MenuEndButton(MenuState);

		MenuActionButton(MenuState, "Exit", 0, 0, Color_Red, DefaultLayout10);
#endif


		MenuEndLayout(MenuState);

		Assert(MenuState->CurrentElement == &MenuState->RootElement);

		FinalizeMenuCreation(MenuState);

		MenuState->IsInitialized = 1;
	}

	render_stack* MenuRenderStack = EngineSystems->RenderState->NamedStacks.GUI;

	RENDERPushRect(MenuRenderStack, MenuState->WindowRect, V4(0.1f, 0.1f, 0.1f, 1.0f));

	MenuWalkThrough(MenuState, MenuState->ViewElement, MenuWalkThrough_Output);
}