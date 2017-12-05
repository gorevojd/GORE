#include <SDL.h>
#include <stdio.h>
#include <thread>

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
						ProcessButton = &Input->Buttons[KeyType_A];
					}break;
					case(SDLK_w): {
						ProcessButton = &Input->Buttons[KeyType_W];
					}break;
					case(SDLK_s): {
						ProcessButton = &Input->Buttons[KeyType_S];
					}break;
					case(SDLK_d): {
						ProcessButton = &Input->Buttons[KeyType_D];
					}break;
					case(SDLK_r): {
						ProcessButton = &Input->Buttons[KeyType_R];
					}break;
					case(SDLK_e): {
						ProcessButton = &Input->Buttons[KeyType_E];
					}break;
					case(SDLK_q): {
						ProcessButton = &Input->Buttons[KeyType_Q];
					}break;
					case(SDLK_ESCAPE): {
						ProcessButton = &Input->Buttons[KeyType_Esc];
					}break;
					case(SDLK_SPACE): {
						ProcessButton = &Input->Buttons[KeyType_Space];
					}break;
					case(SDLK_F1): {
						ProcessButton = &Input->Buttons[KeyType_F1];
					}break;
					case(SDLK_F2): {
						ProcessButton = &Input->Buttons[KeyType_F2];
					}break;
					case(SDLK_F3): {
						ProcessButton = &Input->Buttons[KeyType_F3];
					}break;
					case(SDLK_F4): {
						ProcessButton = &Input->Buttons[KeyType_F4];
					}break;
					case(SDLK_F5): {
						ProcessButton = &Input->Buttons[KeyType_F5];
					}break;
					case(SDLK_F6): {
						ProcessButton = &Input->Buttons[KeyType_F6];
					}break;
					case(SDLK_F7): {
						ProcessButton = &Input->Buttons[KeyType_F7];
					}break;
					case(SDLK_F8): {
						ProcessButton = &Input->Buttons[KeyType_F8];
					}break;
					case(SDLK_F9): {
						ProcessButton = &Input->Buttons[KeyType_F9];
					}break;
					case(SDLK_F10): {
						ProcessButton = &Input->Buttons[KeyType_F10];
					}break;
					case(SDLK_F11): {
						ProcessButton = &Input->Buttons[KeyType_F11];
					}break;
					case(SDLK_F12): {
						ProcessButton = &Input->Buttons[KeyType_F12];
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
	System->MouseP = V2(MouseX, MouseY);

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

#if 0
#define SDL_THREAD_QUEUE_CALLBACK(name) int name(void* Data)
typedef SDL_THREAD_QUEUE_CALLBACK(thread_queue_callback);

struct thread_queue_entry{
	thread_queue_callback* Callback;
	void* Data;
};

struct sdl_thread_entry {
	SDL_sem* Semaphore;
};

#define THREAD_QUEUE_ENTRY_COUNT 512
struct thread_queue {
	thread_queue_entry Entries[THREAD_QUEUE_ENTRY_COUNT];

	volatile int TotalEntries;
	volatile int FinishedEntries;
};

void SDLAddEntry(thread_queue* Queue, thread_queue_callback* Callback, void* Data){
	//Check the overlapping for total
	int NewTotal = (Queue->TotalEntries + 1) % ArrayCount(Queue->Entries);
	int OldVal = SDL_AtomicSet((SDL_atomic_t*)&Queue->TotalEntries, NewTotal);
	/*Should not overlap*/
	Assert(Queue->TotalEntries != Queue->FinishedEntries);

	thread_queue_entry* Entry = Queue->Entries + Queue->TotalEntries;
	Entry->Callback = Callback;
	Entry->Data = Data;

	SDL_CompilerBarrier();

	SDL_SemPost()
}

void SDL(thread_queue* Queue){
	for(;;){
		if(Queue->FinishedEntries != Queue->TotalEntries){
			int NewFinished = (Queue->FinishedEntries + 1) % ArrayCount(Queue->Entries);
			int ToDoEntryIndex = SDL_AtomicSet((SDL_atomic_t*)&Queue->FinishedEntries, NewFinished);
			thread_queue_entry* Entry = Queue->Entries + ToDoEntryIndex;

			Entry->Callback(Entry->Data);

			SDL_CompilerBarrier();

			SDL_SemPost(ThreadEntry->Semaphore);
		}
		else {
			SDL_SemWait(ThreadEntry->Semaphore);
		}
	}
}

void SDLCompleteQueueWork(thread_queue* Queue){
	while(Queue->FinishedEntries != Queue->TotalEntries){
		SDLDoNextWork(Queue);
	}
}

void SDLInitThreadQueue(thread_queue* Queue, sdl_thread_entry* Threads, int ThreadCount) {
	Queue->TotalEntries = 0;
	Queue->FinishedEntries = 0;

	for (int ThreadIndex = 0;
		ThreadIndex < ThreadCount;
		ThreadIndex++) 
	{
		sdl_thread_entry* Entry = Threads + ThreadIndex;

		Entry->Semaphore = SDL_CreateSemaphore(0);
	}

	for (int ThreadIndex = 0;
		ThreadIndex < ThreadCount;
		ThreadIndex++)
	{
		SDL_Thread* Thread = SDL_CreateThread(SDLDoNextWork, 0, 0);
		SDL_WaitThread(Thread);
	}
}

void SDLDestroyThreadQueue(thread_queue* Queue, sdl_thread_entry* Threads, int ThreadCount) {

}

#endif

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

	InitGUIState(GUIState, &FontInfo, &GlobalInput);
	InitDEBUG(GlobalDebugState, &FontInfo);

	float LastMSPerFrame = 0.0f;

	GlobalRunning = true;
	while (GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		BEGIN_TIMED_BLOCK(EventProcessing);
		ProcessEvents(Window, &GlobalInput);

		ProcessInput(&GlobalInput);

		if (ButtonWentDown(&GlobalInput, KeyType_Esc)) {
			GlobalRunning = false;
			break;
		}

		if (ButtonWentDown(&GlobalInput, KeyType_F12)) {
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
		HighlightedText(GUIState, "AASDALJD:LKAJ:LKDJSAKJAHSDLKJHALKSJDHLKJAHSDLKHALKSDLKJASDLKADF:JLKDF:LKSJhlkajsdfhaldhfadfs");
		HighlightedText(GUIState, "DickInjection");

		PrintLabel(GUIState, "Label", V2(GlobalInput.MouseX, GlobalInput.MouseY));
#endif

#if 1
		rect2* MainRect = &GUIState->TempRect.Rect;
		v2 RectDim = GetRectDim(*MainRect);

		PushRectOutline(Stack, *MainRect, 2);
		PushRect(Stack, *MainRect, V4(0.0f, 0.0f, 0.0f, 0.7));
		
		v2 AnchorDim = V2(7, 7);
		
		rect2 SizeAnchorRect;
		SizeAnchorRect.Min = MainRect->Max - V2(3.0f, 3.0f);
		SizeAnchorRect.Max = SizeAnchorRect.Min + AnchorDim;
		v4 SizeAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

		rect2 PosAnchorRect;
		PosAnchorRect.Min = MainRect->Min - V2(3.0f, 3.0f);
		PosAnchorRect.Max = PosAnchorRect.Min + AnchorDim;
		v4 PosAnchorColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

		if (MouseInRect(&GlobalInput, SizeAnchorRect)) {
			SizeAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

			if (MouseButtonWentDown(&GlobalInput, MouseButton_Left) && !GUIState->TempRect.SizeInteraction.IsHot) {
				GUIState->TempRect.SizeInteraction.IsHot = true;
			}
		}

		if (MouseInRect(&GlobalInput, PosAnchorRect)) {
			PosAnchorColor = V4(1.0f, 1.0f, 0.0f, 1.0f);

			if (MouseButtonWentDown(&GlobalInput, MouseButton_Left) && !GUIState->TempRect.PosInteraction.IsHot) {
				GUIState->TempRect.PosInteraction.IsHot = true;
			}
		}
		
		if (MouseButtonWentUp(&GlobalInput, MouseButton_Left)) {
			GUIState->TempRect.SizeInteraction.IsHot = false;
			GUIState->TempRect.PosInteraction.IsHot = false;
		}

		if (GUIState->TempRect.PosInteraction.IsHot) {
			MainRect->Min = GlobalInput.MouseP;
			MainRect->Max = MainRect->Min + RectDim;
			PosAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
		}

		v2 ResizedRectDim = RectDim;
		if (GUIState->TempRect.SizeInteraction.IsHot) {
			MainRect->Max = GlobalInput.MouseP;
			SizeAnchorColor = V4(1.0f, 0.1f, 0.1f, 1.0f);
			ResizedRectDim = GetRectDim(*MainRect);
		}
		
		if (ResizedRectDim.x < 10) {
			MainRect->Max.x = MainRect->Min.x + 10;
		}

		if (ResizedRectDim.y < 10) {
			MainRect->Max.y = MainRect->Min.y + 10;
		}

		PushRect(Stack, SizeAnchorRect.Min, AnchorDim, SizeAnchorColor);
		PushRect(Stack, PosAnchorRect.Min, AnchorDim, PosAnchorColor);
#endif

		RenderDickInjection(Stack, &GlobalBuffer);

		EndFrameGUI(GUIState);
		EndRenderStack(Stack);

		OverlayCycleCounters(GlobalDebugState, GUIState);
		RenderDickInjection(&GlobalDebugState->GUIRenderStack, &GlobalBuffer);

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