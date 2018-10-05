#include "gore_game_main_menu.h"

#define STB_PERLIN_IMPLEMENTATION
#define STB_PERLIN_STATIC
#include "stb_perlin.h"

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

	MenuState->PrevElement = Result;

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

	ButtonData->WeightAdditionX = 0.0f;
	ButtonData->WeightAdditionY = 0.0f;

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

static void MenuBeginButton(
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

static void MenuEndButton(main_menu_state* MenuState) {
	MenuEndElement(MenuState, MenuElement_Button);
}

static void MenuActionButton(
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

	ButtonElement->Element.Button.ActionFunction = Action;
	ButtonElement->Element.Button.ActionData = ActionData;

	MenuBeginElement(MenuState, ButtonElement);
	MenuEndElement(MenuState, MenuElement_Button);
}

static void MenuBeginLayout(main_menu_state* MenuState, menu_element_layout Layout) {
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

static void MenuBeginRectLayout(main_menu_state* MenuState, rect2 Rect, menu_element_layout Layout) {
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

static void MenuEndLayout(main_menu_state* MenuState){
	MenuEndElement(MenuState, MenuElement_Layout);
}

static void MenuAddWeightsToPrevElement(main_menu_state* MenuState, float Xweight, float Yweight) {
	MenuState->PrevElement->Layout.WeightX = Xweight;
	MenuState->PrevElement->Layout.WeightY = Yweight;
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

			v2 At = Elem->Layout.Rect.Min + FreeBordersDim;

			

			//NOTE(dima): this loop will precalculate weights for elements
			float TotalXWeight = 0.0f;
			float TotalYWeight = 0.0f;

			menu_element* AtChildren = Elem->ChildrenSentinel->PrevInList;
			while (AtChildren != Elem->ChildrenSentinel) {

				TotalXWeight += AtChildren->Layout.WeightX;
				TotalYWeight += AtChildren->Layout.WeightY;

				if (AtChildren->MenuElementType == MenuElement_Button) {
					TotalXWeight += AtChildren->Element.Button.WeightAdditionX;
					TotalYWeight += AtChildren->Element.Button.WeightAdditionY;
				}

				AtChildren = AtChildren->PrevInList;
			}


			//NOTE(dima): this loop will calculate everything i need
			AtChildren = Elem->ChildrenSentinel->PrevInList;
			while (AtChildren != Elem->ChildrenSentinel) {

				v2 ElementDim;
				v2 Increment;

				float ThisElementXWeight = AtChildren->Layout.WeightX;
				float ThisElementYWeight = AtChildren->Layout.WeightY;

				if (AtChildren->MenuElementType == MenuElement_Button) {
					ThisElementXWeight += AtChildren->Element.Button.WeightAdditionX;
					ThisElementYWeight += AtChildren->Element.Button.WeightAdditionY;
				}

				float XPercentageForThisElem = ThisElementXWeight / TotalXWeight;
				float YPercentageForThisElem = ThisElementYWeight / TotalYWeight;

				if (Layout->ChildrenElementCount > 1) {
					v2 ElemsArea = TotalFillDim - ElemsSpacingDim;
					v2 OneSpacingDim = ElemsSpacingDim / (float)(Layout->ChildrenElementCount - 1);

					switch (Layout->LayoutType) {
						case MenuElementLayout_Horizontal: {
							ElementDim = V2(ElemsArea.x * XPercentageForThisElem, TotalFillDim.y);
							Increment = V2(ElementDim.x + OneSpacingDim.x, 0.0f);
						}break;

						case MenuElementLayout_Vertical: {
							ElementDim = V2(TotalFillDim.x, ElemsArea.y * YPercentageForThisElem);
							Increment = V2(0.0f, ElementDim.y + OneSpacingDim.y);
						}break;
					}
				}
				else {
					ElementDim = TotalFillDim;
					FreeBordersDim = V2(0.0f, 0.0f);
					Increment = V2(0.0f, 0.0f);
				}

				if ((AtChildren->MenuElementType == MenuElement_Layout) &&
					(AtChildren->Element.LayoutData.InitRectIsSet))
				{
					AtChildren->Layout.Rect = AtChildren->Element.LayoutData.InitRect;
				}
				else {
					AtChildren->Layout.Rect = Rect2MinDim(At, ElementDim);
				}


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

						if (MouseInRect(Input, AtElem->Layout.Rect)) {
							ButtonData->TimeSinceActivation += Input->DeltaTime;
							ButtonData->TimeSinceDeactivation = 0.0f;

							if (MouseButtonWentDown(Input, MouseButton_Left)) {
								switch (ButtonData->ButtonActionType) {
									case MenuButtonAction_None: {

									}break;

									case MenuButtonAction_Action: {
										if (ButtonData->ActionFunction) {
											ButtonData->ActionFunction(ButtonData->ActionData);
										}
									}break;

									case MenuButtonAction_OpenTree: {
										AtElem->ParentViewElement = Elem;
										MenuState->ViewElement = AtElem;
									}break;
								}
							}
						}
						else {
							ButtonData->TimeSinceDeactivation += Input->DeltaTime;
							ButtonData->TimeSinceActivation = 0.0f;
						}

						float FadeinPercentage = ButtonData->TimeSinceActivation / ButtonData->TimeForFadeout;
						FadeinPercentage = Clamp01(FadeinPercentage);

						float FadeoutPercentage = ButtonData->TimeSinceDeactivation / ButtonData->TimeForFadeout;
						FadeoutPercentage = Clamp01(FadeoutPercentage);

#if 1
#if 1
						float WeightAddition = Lerp(0.0f, 0.4f, FadeinPercentage);
						WeightAddition = Lerp(WeightAddition, 0.0f, FadeoutPercentage + FadeinPercentage);
#else
						float WeightAdditionFadePercentage = (FadeoutPercentage - FadeinPercentage);
						//float WeightAddition = CosLerp(0.2f, 0.0f, WeightAdditionFadePercentage);
						float WeightAddition = Lerp(0.0f, 0.2f, FadeinPercentage);
#endif

						ButtonData->WeightAdditionX = WeightAddition;
						ButtonData->WeightAdditionY = WeightAddition;
#endif

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

						if (!AtElem->Layout.BackgroundTextureIsSet) {
							AtElem->Layout.BackgroundTexture = PushStruct(MenuState->GameModeMemory, bitmap_info);

							bitmap_info* Target = AtElem->Layout.BackgroundTexture;

							*Target = {};

							Target->Width = u32(GetRectWidth(AtElem->Layout.Rect) + 0.5f);
							Target->Height = u32(GetRectHeight(AtElem->Layout.Rect) + 0.5f);
							Target->Align = V2(0.0f, 0.0f);
							Target->Pitch = Target->Width * 4;
							Target->Pixels = PushArray(MenuState->GameModeMemory, u8, Target->Width * Target->Height * 4);
							Target->TextureHandle = 0;
							Target->WidthOverHeight = (float)Target->Width / (float)Target->Height;

							float OneOverWidth = 1.0f / (float)Target->Width;
							float OneOverHeight = 1.0f / (float)Target->Height;

#if 0
							for (int Ypix = 0; Ypix < Target->Height; Ypix++) {
								for (int Xpix = 0; Xpix < Target->Width; Xpix++) {
									float NoiseMultiplier = 4.0f;
									float Xval = ((float)Xpix + AtElem->Layout.Rect.Min.x) * OneOverWidth * NoiseMultiplier;
									float Yval = ((float)Ypix + AtElem->Layout.Rect.Min.y) * OneOverHeight * NoiseMultiplier;

									float Noise = stb_perlin_fbm_noise3(Xval, Yval, 0.0f, 2.0f, 0.5f, 5, 0, 0, 0);

									float Noise01 = Clamp01(Noise * 0.5f + 0.5f);

									v4 GrayscaleColor = V4(Noise01, Noise01, Noise01, 0.1f);

									u32 Packed = PackRGBA(GrayscaleColor);

									u32* TargetPixel = (u32*)Target->Pixels + Ypix * Target->Width + Xpix;

									*TargetPixel = Packed;
								}
							}
#endif

							AtElem->Layout.BackgroundTextureIsSet = 1;
						}

						if (AtElem->Layout.BackgroundTexture) {
							RENDERPushBitmap(
								RenderStack,
								AtElem->Layout.BackgroundTexture,
								AtElem->Layout.Rect.Min,
								GetRectHeight(AtElem->Layout.Rect));
						}

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

struct menu_go_back_action_data {
	main_menu_state* MenuState;
};

static MENU_BUTTON_ACTION(MenuGoBackAction) {
	menu_go_back_action_data* ActionData = (menu_go_back_action_data*)Data;

	if (ActionData->MenuState->ViewElement->ParentViewElement != 0) {
		ActionData->MenuState->ViewElement = ActionData->MenuState->ViewElement->ParentViewElement;
	}
}

static MENU_BUTTON_ACTION(MenuExitAction){
	PlatformApi.EndGameLoop();
}

struct menu_open_game_mode_action_data {
	game_mode_state* GameModeState;
	u32 NewGameMode;
};

static inline menu_open_game_mode_action_data OpenModeData(game_mode_state* GameModeState, u32 NewMode) {
	menu_open_game_mode_action_data Result = {};

	Result.GameModeState = GameModeState;
	Result.NewGameMode = NewMode;

	return(Result);
}

static MENU_BUTTON_ACTION(MenuOpenGameModeAction) {
	menu_open_game_mode_action_data* ActionData = (menu_open_game_mode_action_data*)Data;

	SwitchGameMode(ActionData->GameModeState, ActionData->NewGameMode);
}

void UpdateMainMenu(game_mode_state* GameModeState, engine_systems* EngineSystems) {
	stacked_memory* GameModeMemory = &GameModeState->GameModeMemory;
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

		menu_element_layout DefaultLayout10 = MenuInitLayout(MenuElementLayout_Horizontal, 1.0f, 1.0f);
		menu_element_layout DefaultLayout10Vert = MenuInitLayout(MenuElementLayout_Vertical, 1.0f, 1.0f);
		menu_element_layout DefaultLayout0608 = MenuInitLayout(MenuElementLayout_Vertical, 0.9f, 0.9f);
		menu_element_layout DefaultLayout09 = MenuInitLayout(MenuElementLayout_Horizontal, 0.9f, 0.9f);

		//NOTE(dima): This code should only be called once
		MenuBeginLayout(MenuState, DefaultLayout0608);

		MenuBeginButton(MenuState, "Play", ColorExt_orange2, DefaultLayout10);
		MenuAddWeightsToPrevElement(MenuState, 1.0f, 2.0f);
		MenuBeginRectLayout(MenuState, MenuState->WindowRect, DefaultLayout09);

		menu_open_game_mode_action_data* OpenGeomkaData = PushStruct(GameModeMemory, menu_open_game_mode_action_data);
		*OpenGeomkaData = OpenModeData(GameModeState, GameMode_Geometrica);
		MenuActionButton(MenuState, "Geometrica", MenuOpenGameModeAction, OpenGeomkaData, ColorExt_rebeccapurple, DefaultLayout10);

		menu_open_game_mode_action_data* OpenVoxelData = PushStruct(GameModeMemory, menu_open_game_mode_action_data);
		*OpenVoxelData = OpenModeData(GameModeState, GameMode_VoxelWorld);
		MenuActionButton(MenuState, "VoxelWorld", MenuOpenGameModeAction, OpenVoxelData, ColorExt_bisque3, DefaultLayout10);

		menu_open_game_mode_action_data* OpenLpterData = PushStruct(GameModeMemory, menu_open_game_mode_action_data);
		*OpenLpterData = OpenModeData(GameModeState, GameMode_GoreGame);
		MenuActionButton(MenuState, "Gore", MenuOpenGameModeAction, OpenLpterData, ColorExt_DarkSlateGray2, DefaultLayout10);

		MenuActionButton(MenuState, "Back", MenuGoBackAction, {&MenuState}, Color_Red, DefaultLayout10);
		MenuAddWeightsToPrevElement(MenuState, 0.5f, 1.0f);
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
		MenuBeginLayout(MenuState, DefaultLayout10);
		MenuActionButton(MenuState, "Options", 0, 0, ColorExt_rebeccapurple, DefaultLayout10);
		MenuActionButton(MenuState, "Credits", 0, 0, Color_PrettyBlue, DefaultLayout10);
		MenuActionButton(MenuState, "Exit", MenuExitAction, 0, Color_Red, DefaultLayout10);
		MenuEndLayout(MenuState);

#endif


		MenuEndLayout(MenuState);

		Assert(MenuState->CurrentElement == &MenuState->RootElement);

		FinalizeMenuCreation(MenuState);

		MenuState->IsInitialized = 1;
	}

	render_stack* MenuRenderStack = EngineSystems->RenderState->NamedStacks.GUI;

	RENDERPushRect(MenuRenderStack, MenuState->WindowRect, V4(0.1f, 0.1f, 0.1f, 1.0f));

	MenuWalkThrough(MenuState, MenuState->CurrentElement, MenuWalkThrough_CalculateRects);
	MenuWalkThrough(MenuState, MenuState->ViewElement, MenuWalkThrough_Output);
}