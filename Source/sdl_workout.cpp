#include <SDL.h>
#include <stdio.h>

#include "workout_game_layer.h"

#include "workout_render_stack.h"
#include "workout_renderer.h"
#include "workout_asset.h"
#include "workout_debug.h"
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
			Fix buffer overlapping with optimized renderer
			Make rectangle rendering

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
*/



GLOBAL_VARIABLE b32 GlobalRunning;
GLOBAL_VARIABLE rgba_buffer GlobalBuffer;
GLOBAL_VARIABLE input_system GlobalInput;

GLOBAL_VARIABLE u64 GlobalPerfomanceCounterFrequency;
GLOBAL_VARIABLE float GlobalTime;

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

static void ProcessKeyboardButton(button_state* State, b32 IsDown, b32 WasDown) {
	State->IsDown = IsDown;
	State->WasDown = WasDown;
}

static void ProcessEvents(input_system* Input) {
	SDL_Event CurrentEvent;

	while (SDL_PollEvent(&CurrentEvent)) {

		SDL_Keysym KeySum = CurrentEvent.key.keysym;
		SDL_Keycode KeyCode = KeySum.sym;

		b32 AltIsDown = KeySum.mod & KMOD_ALT;
		b32 ShiftIsDown = KeySum.mod & KMOD_SHIFT;
		b32 CtrlISDown = KeySum.mod & KMOD_CTRL;

		b32 IsDown = (CurrentEvent.key.state == SDL_PRESSED);
		b32 WasDown = IsDown && (CurrentEvent.key.repeat != 0);

		if (CurrentEvent.key.repeat == 0) {
			if (KeyCode == SDLK_a) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_A], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_w) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_W], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_s) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_S], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_d) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_D], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_r) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_R], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_e) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_E], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_q) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_Q], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_ESCAPE) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_Esc], IsDown, WasDown);
			}
			else if (KeyCode == SDLK_SPACE) {
				ProcessKeyboardButton(&Input->Buttons[InputButtonType_Space], IsDown, WasDown);
			}
		}
	}
}

static b32 ButtonWentDown(input_system* Input, u32 ButtonType) {

	b32 Result = 0;

	button_state* State = &Input->Buttons[ButtonType];

	if (State->IsDown == true && State->WasDown == false) {
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

	GlobalBuffer = AllocateRGBABuffer(1025, 768);
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
	//font_info FontInfo = LoadFontInfoWithSTB("Data/Fonts/arial.ttf", 20);

	InitGUIState(GUIState, &FontInfo);

	float LastMSPerFrame = 0.0f;

	GlobalRunning = true;
	while (GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		ProcessEvents(&GlobalInput);

		if (ButtonWentDown(&GlobalInput, InputButtonType_Esc)) {
			GlobalRunning = false;
			break;
		}
		
		render_stack Stack_ = BeginRenderStack();
		render_stack* Stack = &Stack_;

		float GradR = sin(GlobalTime + 0.5f) * 0.5f + 0.5f;
		float GradG = cos(GlobalTime + 0.5f) * 0.4f + 0.5f;
		float GradB = sin(GlobalTime * 2.0f + 0.5f) * 0.5f + 0.5f;

		PushGradient(Stack, V3(GradR, GradG, GradB));
		//PushClear(Stack, V3(1.0f, 1.0f, 1.0f));
		//PushBitmap(Stack, &Image, { 0, 0 }, 800);

		BeginFrameGUI(GUIState, Stack);
		char DebugStr[128];
		float LastFrameFPS = 1000.0f / LastMSPerFrame;
		sprintf(DebugStr, "Hello world! %.2fmsp/f %.2fFPS", LastMSPerFrame, LastFrameFPS);
		PrintText(GUIState, DebugStr);
		PrintText(GUIState, "Hello my friend");
		PrintText(GUIState, "Sanya Surabko, Gorevoy Dmitry from LWO Corp");
		PrintText(GUIState, "Gorevoy Dmitry, Nikita Laptev from BSTU hostel");
		EndFrameGUI(GUIState);

		SoftwareRenderStackToOutput(Stack, &GlobalBuffer);

		EndRenderStack(Stack);

		SDL_Surface* Surf = SDLSurfaceFromBuffer(&GlobalBuffer);
		SDL_Texture* GlobalRenderTexture = SDL_CreateTextureFromSurface(renderer, Surf);

		SDL_Rect GlobalRenderTextureRect;
		GlobalRenderTextureRect.x = 0;
		GlobalRenderTextureRect.y = 0;
		GlobalRenderTextureRect.w = GlobalBuffer.Width;
		GlobalRenderTextureRect.h = GlobalBuffer.Height;

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, GlobalRenderTexture, 0, &GlobalRenderTextureRect);
		SDL_RenderPresent(renderer);

		SDL_DestroyTexture(GlobalRenderTexture);
		SDL_FreeSurface(Surf);

		float SPerFrame = SDLGetMSElapsed(FrameBeginClocks);
		LastMSPerFrame = SPerFrame * 1000.0f;

		GlobalTime += SPerFrame;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	DeallocateRGBABuffer(&GlobalBuffer);

	printf("Program has been succesfully ended\n");

	system("pause");
	return(0);
}