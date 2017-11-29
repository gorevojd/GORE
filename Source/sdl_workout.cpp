#include <SDL.h>
#include <stdio.h>

#include "workout_game_layer.h"

#include "workout_debug.h"
#include "workout_render_stack.h"
#include "workout_renderer.h"
#include "workout_asset.h"
#include "workout_gui.h"

/*
	NOTE(Dima):
		Images are stored in premultiplied alpha format
*/

/*
	TODO(Dima):
		Renderer:
			Implement bitmap alignment
			Optimize renderer with multithreading;
			Fix buffer overlapping in optimized renderer
			Optimize rectangle rendering
			Implement Gaussian blur

		Assets:
			Build font atlas
			Implement asset packer

		Memory management:
			Linear memory allocation

		Other:
			Begin some debugging stuff
			Split code to platform dependent and platform independent parts

		Platform layer:
			Mouse input handling
			alt-f4 close handling
			Thread pool
			Correct aspect ratio handling

		GUI:
			Lists
			Sliders
*/

GLOBAL_VARIABLE b32 GlobalRunning;
GLOBAL_VARIABLE rgba_buffer GlobalBuffer;
GLOBAL_VARIABLE input_system GlobalInput;

GLOBAL_VARIABLE u64 GlobalPerfomanceCounterFrequency;
GLOBAL_VARIABLE float GlobalTime;

debug_state GlobalDebugState_;
debug_state* GlobalDebugState = &GlobalDebugState_;

static u64 SDLGetClocks() {
	u64 Result = SDL_GetPerformanceCounter();

	return(Result);
}

static float SDLGetMSElapsed(u64 BeginClocks) {
	float Result;

	u64 ClocksElapsed = SDLGetClocks() - BeginClocks;
	Result = (float)ClocksElapsed / GlobalPerfomanceCounterFrequency;

	return(Result);
}

static void SDLGoFullscreen(SDL_Window* Window) {

	b32 IsFullscreen = SDL_GetWindowFlags(Window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);

	if (IsFullscreen) {
		SDL_SetWindowFullscreen(Window, 0);
	}
	else {
		SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);
	}
}

inline void ProcessButtonState(button_state* State, b32 IsDown, b32 TransitionHappened, b32 IsDoubleClick = false) {
	/*Transition happened*/
	State->IsDown = IsDown;
	State->TransitionHappened = TransitionHappened;
	
	State->IsDoubleClick = IsDoubleClick;
}

inline button_state* SDLGetButtonStateForKey(input_system* Input, u32 SDLKey) {
	button_state* Result = 0;



	return(Result);
}

static void ProcessEvents(SDL_Window* Window, input_system* Input) {
	SDL_Event CurrentEvent;

	for (int ButtonIndex = 0;
		ButtonIndex < ArrayCount(Input->Buttons);
		ButtonIndex++)
	{
		button_state* But = &Input->Buttons[ButtonIndex];

		But->IsDown = But->IsDown;
		But->TransitionHappened = 0;
	}

	for (int MouseButIndex = 0;
		MouseButIndex < ArrayCount(Input->MouseButtons);
		MouseButIndex++)
	{
		button_state* But = &Input->MouseButtons[MouseButIndex];

		But->IsDown = But->IsDown;
		But->TransitionHappened = 0;

		But->IsDoubleClick = 0;
	}

	while (SDL_PollEvent(&CurrentEvent)) {

		switch (CurrentEvent.type) {

			case(SDL_KEYUP):
			case(SDL_KEYDOWN): {
				SDL_Keysym KeySum = CurrentEvent.key.keysym;
				SDL_Keycode KeyCode = KeySum.sym;

				b32 AltIsDown = KeySum.mod & KMOD_ALT;
				b32 ShiftIsDown = KeySum.mod & KMOD_SHIFT;
				b32 CtrlISDown = KeySum.mod & KMOD_CTRL;

				b32 IsDown = (CurrentEvent.key.state == SDL_PRESSED);
				b32 IsUp = (CurrentEvent.key.state == SDL_RELEASED);
				b32 TransitionHappened = (CurrentEvent.key.repeat == 0);

				button_state* ProcessButton = 0;

				switch (KeyCode) {
					case(SDLK_a): {
						ProcessButton = &Input->Buttons[InputButtonType_A];
					}break;
					case(SDLK_w): {
						ProcessButton = &Input->Buttons[InputButtonType_W];
					}break;
					case(SDLK_s): {
						ProcessButton = &Input->Buttons[InputButtonType_S];
					}break;
					case(SDLK_d): {
						ProcessButton = &Input->Buttons[InputButtonType_D];
					}break;
					case(SDLK_r): {
						ProcessButton = &Input->Buttons[InputButtonType_R];
					}break;
					case(SDLK_e): {
						ProcessButton = &Input->Buttons[InputButtonType_E];
					}break;
					case(SDLK_q): {
						ProcessButton = &Input->Buttons[InputButtonType_Q];
					}break;
					case(SDLK_ESCAPE): {
						ProcessButton = &Input->Buttons[InputButtonType_Esc];
					}break;
					case(SDLK_SPACE): {
						ProcessButton = &Input->Buttons[InputButtonType_Space];
					}break;
					case(SDLK_F1): {
						ProcessButton = &Input->Buttons[InputButtonType_F1];
					}break;
					case(SDLK_F2): {
						ProcessButton = &Input->Buttons[InputButtonType_F2];
					}break;
					case(SDLK_F3): {
						ProcessButton = &Input->Buttons[InputButtonType_F3];
					}break;
					case(SDLK_F4): {
						ProcessButton = &Input->Buttons[InputButtonType_F4];
					}break;
					case(SDLK_F5): {
						ProcessButton = &Input->Buttons[InputButtonType_F5];
					}break;
					case(SDLK_F6): {
						ProcessButton = &Input->Buttons[InputButtonType_F6];
					}break;
					case(SDLK_F7): {
						ProcessButton = &Input->Buttons[InputButtonType_F7];
					}break;
					case(SDLK_F8): {
						ProcessButton = &Input->Buttons[InputButtonType_F8];
					}break;
					case(SDLK_F9): {
						ProcessButton = &Input->Buttons[InputButtonType_F9];
					}break;
					case(SDLK_F10): {
						ProcessButton = &Input->Buttons[InputButtonType_F10];
					}break;
					case(SDLK_F11): {
						ProcessButton = &Input->Buttons[InputButtonType_F11];
					}break;
					case(SDLK_F12): {
						ProcessButton = &Input->Buttons[InputButtonType_F12];
					}break;

					default: {

					}break;
				}

				if (TransitionHappened) {

					if (IsDown) {
						if (KeyCode == SDLK_F4 && AltIsDown) {
							GlobalRunning = false;
						}

						if (KeyCode == SDLK_RETURN && AltIsDown) {
							SDLGoFullscreen(Window);
						}
					}

					if (IsUp) {

					}
				}
				else {

				}
				
				if (ProcessButton) {
					ProcessButtonState(ProcessButton, IsDown, TransitionHappened);
				}

			}break;

			case(SDL_WINDOWEVENT): {
				SDL_WindowEvent* WindowEvent = &CurrentEvent.window;
				switch (WindowEvent->event) {
					/*Close is sent to window*/
					case(SDL_WINDOWEVENT_CLOSE): {
						GlobalRunning = false;
					}break;

						/*Mouse entered window*/
					case SDL_WINDOWEVENT_ENTER: {

					}break;

						/*Mouse leaved window*/
					case SDL_WINDOWEVENT_LEAVE: {

					}break;
				};
			}break;

			case(SDL_MOUSEBUTTONUP):
			case(SDL_MOUSEBUTTONDOWN): {
				SDL_MouseButtonEvent* MouseEvent = &CurrentEvent.button;

				b32 IsDown = (MouseEvent->state == SDL_PRESSED);
				b32 WasDown = (MouseEvent->type == SDL_MOUSEBUTTONUP);
				b32 IsDoubleClick = (MouseEvent->clicks == 2);
				b32 TransitionHappened = (WasDown != IsDown);

				if (TransitionHappened) {
					if (MouseEvent->button == SDL_BUTTON_LEFT) {
						ProcessButtonState(
							&Input->MouseButtons[MouseButton_Left],
							IsDown,
							TransitionHappened,
							IsDoubleClick);
					}
					else if (MouseEvent->button == SDL_BUTTON_RIGHT) {
						ProcessButtonState(
							&Input->MouseButtons[MouseButton_Right],
							IsDown,
							TransitionHappened,
							IsDoubleClick);
					}
					else if (MouseEvent->button == SDL_BUTTON_MIDDLE) {
						ProcessButtonState(
							&Input->MouseButtons[MouseButton_Middle],
							IsDown,
							TransitionHappened,
							IsDoubleClick);
					}
					else if (MouseEvent->button == SDL_BUTTON_X1) {
						ProcessButtonState(
							&Input->MouseButtons[MouseButton_Extended1],
							IsDown,
							TransitionHappened,
							IsDoubleClick);
					}
					else if (MouseEvent->button == SDL_BUTTON_X2) {
						ProcessButtonState(
							&Input->MouseButtons[MouseButton_Extended2],
							IsDown,
							TransitionHappened,
							IsDoubleClick);
					}
				}
			}break;

			case(SDL_MOUSEWHEEL): {

			}break;

			default: {

			}break;
		}
	}

}

static void ProcessInput(input_system* System) {
	int MouseX;
	int MouseY;
	u32 MouseState = SDL_GetMouseState(&MouseX, &MouseY);

	System->MouseX = MouseX;
	System->MouseY = MouseY;

	int GlobalMouseX;
	int GlobalMouseY;
	SDL_GetGlobalMouseState(&GlobalMouseX, &GlobalMouseY);

	System->GlobalMouseX = GlobalMouseX;
	System->GlobalMouseY = GlobalMouseY;

	b32 MRightIsDown = MouseState & SDL_BUTTON_RMASK;
	b32 MLeftIsDown = MouseState & SDL_BUTTON_LMASK;
	b32 MMidIsDown = MouseState & SDL_BUTTON_MMASK;
	b32 MExt1IsDown = MouseState & SDL_BUTTON_X1MASK;
	b32 MExt2IsDown = MouseState & SDL_BUTTON_X2MASK;
}

static b32 ButtonWentDown(input_system* Input, u32 ButtonType) {

	b32 Result = 0;

	button_state* State = &Input->Buttons[ButtonType];

	if (State->IsDown == true && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

static b32 MouseButtonWentDown(input_system* Input, u32 MouseButton) {

	b32 Result = 0;

	button_state* State = &Input->MouseButtons[MouseButton];

	if (State->IsDown == true && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

static gui_state GUIState_;
static gui_state* GUIState = &GUIState_;

inline SDL_Surface* SDLSurfaceFromBuffer(rgba_buffer* Buffer) {
	SDL_Surface* Result = SDL_CreateRGBSurfaceFrom(
		Buffer->Pixels,
		Buffer->Width,
		Buffer->Height,
		32,
		Buffer->Width * 4,
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF);

	return(Result);
}

int main(int ArgsCount, char** Args) {

	int SdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

	if (SdlInitCode < 0) {
		printf("ERROR: SDL has been not initialized");
	}

	GlobalBuffer = AllocateRGBABuffer(1024, 768);
	GlobalPerfomanceCounterFrequency = SDL_GetPerformanceFrequency();
	GlobalTime = 0.0f;

	SDL_Window* Window = SDL_CreateWindow(
		"WindowName",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GlobalBuffer.Width,
		GlobalBuffer.Height,
		SDL_WINDOW_OPENGL);


	SDL_Renderer* renderer = SDL_CreateRenderer(Window, -1, 0);
	if (!Window) {
		printf("ERROR: Window is not created");
	}

	rgba_buffer Image = LoadIMG("../Data/Images/image.bmp");
	rgba_buffer AlphaImage = LoadIMG("../Data/Images/alpha.png");

	font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Regular.ttf", 20);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/arial.ttf", 20);

	InitGUIState(GUIState, &FontInfo);
	InitDEBUG(GlobalDebugState, &FontInfo);

	float LastMSPerFrame = 0.0f;

	GlobalRunning = true;
	while (GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		BEGIN_TIMED_BLOCK(EventProcessing);
		ProcessEvents(Window, &GlobalInput);

		ProcessInput(&GlobalInput);

		if (ButtonWentDown(&GlobalInput, InputButtonType_Esc)) {
			GlobalRunning = false;
			break;
		}

		if (ButtonWentDown(&GlobalInput, InputButtonType_F12)) {
			SDLGoFullscreen(Window);
		}

		END_TIMED_BLOCK(EventProcessing);

		BeginDEBUG(GlobalDebugState);

		render_stack Stack_ = BeginRenderStack();
		render_stack* Stack = &Stack_;

		float GradR = sin(GlobalTime + 0.5f) * 0.5f + 0.5f;
		float GradG = cos(GlobalTime + 0.5f) * 0.4f + 0.5f;
		float GradB = sin(GlobalTime * 2.0f + 0.5f) * 0.5f + 0.5f;

		float AlphaImageX1 = sin(GlobalTime * 2 * 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX2 = cos(GlobalTime * 6) * 900 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX3 = sin(GlobalTime * 3 + 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5f;

		//PushGradient(Stack, V3(GradR, GradG, GradB));
		PushClear(Stack, V3(0.5f, 0.5f, 0.5f));
		//PushBitmap(Stack, &Image, { 0, 0 }, 800);

		PushBitmap(Stack, &AlphaImage, V2(AlphaImageX1, 400), 300.0f);
		PushBitmap(Stack, &AlphaImage, V2(AlphaImageX2, 600), 300.0f);
		PushBitmap(Stack, &AlphaImage, V2(AlphaImageX3, 200), 300.0f);

		PushRect(Stack, V2(AlphaImageX1, 400), V2(100, 100), V4(1.0f, 1.0f, 1.0f, 0.5f));

		BeginFrameGUI(GUIState, Stack);
		char DebugStr[128];
		float LastFrameFPS = 1000.0f / LastMSPerFrame;
		sprintf(DebugStr, "Hello world! %.2fmsp/f %.2fFPS", LastMSPerFrame, LastFrameFPS);
#if 1
		PrintText(GUIState, DebugStr);
		//PrintText(GUIState, "Hello my friend");
		//PrintText(GUIState, "Sanya Surabko, Gorevoy Dmitry from LWO Corp");
		//PrintText(GUIState, "Gorevoy Dmitry, Nikita Laptev from BSTU hostel");

		HighlightedText(GUIState, "HelloButton xD -_- ._. T_T ^_^");
		HighlightedText(GUIState, "1234567890");

		PrintLabel(GUIState, "Label", V2(GlobalInput.MouseX, GlobalInput.MouseY));
#endif

		SoftwareRenderStackToOutput(Stack, &GlobalBuffer);

		EndFrameGUI(GUIState);
		EndRenderStack(Stack);

		OverlayCycleCounters(GlobalDebugState, GUIState);
		SoftwareRenderStackToOutput(&GlobalDebugState->GUIRenderStack, &GlobalBuffer);

		SDL_Surface* Surf = SDLSurfaceFromBuffer(&GlobalBuffer);
		SDL_Texture* GlobalRenderTexture = SDL_CreateTextureFromSurface(renderer, Surf);

		SDL_DisplayMode Mode;
		SDL_GetWindowDisplayMode(Window, &Mode);
		SDL_Rect GlobalRenderTextureRect;
		GlobalRenderTextureRect.x = 0;
		GlobalRenderTextureRect.y = 0;
		GlobalRenderTextureRect.w = Mode.w;
		GlobalRenderTextureRect.h = Mode.h;

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, GlobalRenderTexture, 0, &GlobalRenderTextureRect);
		SDL_RenderPresent(renderer);

		SDL_DestroyTexture(GlobalRenderTexture);
		SDL_FreeSurface(Surf);

		float SPerFrame = SDLGetMSElapsed(FrameBeginClocks);
		LastMSPerFrame = SPerFrame * 1000.0f;

		ClearDebugCounters(GlobalDebugState);
		EndDEBUG(GlobalDebugState);

		GlobalTime += SPerFrame;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	DeallocateRGBABuffer(&GlobalBuffer);

	printf("Program has been succesfully ended\n");

	system("pause");
	return(0);
}