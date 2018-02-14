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
			Implement Gaussian blur


		Assets:
			Build font atlas
			Implement asset packer
			Asset streaming

		Memory management:
			Splitting stacked memory

		Other:
			Begin some debugging stuff
			Split code to platform dependent and platform independent parts

		Platform layer:
			Thread pool     
			Correct aspect ratio handling

		Sound:
			Implement sound mixer
			Optimize sound mixer with SSE

		Profiler:
			Some basic profile markers to measure perfomance

		GUI:
			New named-color system
			New row system
			New Label system or depth

			Caching some elements calculations.

			Make possibility to add sum stuff to elements that alredy exist in the tree view

			GUI radio buttons
			GUI text windows
			GUI input text maybe
*/

GLOBAL_VARIABLE b32 GlobalRunning;
GLOBAL_VARIABLE rgba_buffer GlobalBuffer;
GLOBAL_VARIABLE input_system GlobalInput;

GLOBAL_VARIABLE u64 GlobalPerfomanceCounterFrequency;
GLOBAL_VARIABLE float GlobalTime;


platform_api PlatformApi;

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
					case(SDLK_RETURN): {
						ProcessButton = &Input->Buttons[KeyType_Return];
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
					case(SDLK_BACKQUOTE): {
						ProcessButton = &Input->Buttons[KeyType_Backquote];
					}break;
					case(SDLK_BACKSPACE): {
						ProcessButton = &Input->Buttons[KeyType_Backspace];
					}break;
					case(SDLK_TAB): {
						ProcessButton = &Input->Buttons[KeyType_Tab];
					}break;
					case(SDLK_LEFT): {
						ProcessButton = &Input->Buttons[KeyType_Left];
					}break;
					case(SDLK_RIGHT): {
						ProcessButton = &Input->Buttons[KeyType_Right];
					}break;
					case(SDLK_UP): {
						ProcessButton = &Input->Buttons[KeyType_Up];
					}break;
					case(SDLK_DOWN): {
						ProcessButton = &Input->Buttons[KeyType_Down];
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

static void SDLSetWindowIcon(SDL_Window* Window) {
	SDL_Surface *Surface;     // Declare an SDL_Surface to be filled in with pixel data from an image file
	rgba_buffer Image = LoadIMG("../Data/Images/pot32.png");
	Surface = SDLSurfaceFromBuffer(&Image);
	// The icon is attached to the window pointer
	SDL_SetWindowIcon(Window, Surface);

	// ...and the surface containing the icon pixel data is no longer required.
	SDL_FreeSurface(Surface);
}

#if 1
struct sdl_thread_entry {
	thread_queue* Queue;
};

void SDLAddEntry(thread_queue* Queue, thread_queue_callback* Callback, void* Data){
	//Check the overlapping for total
	int NewNextToWrite = (Queue->NextToWrite + 1) % ArrayCount(Queue->Entries);
	/*Should not overlap*/
	Assert(Queue->NextToWrite != NewNextToWrite);

	thread_queue_entry* Entry = Queue->Entries + Queue->NextToWrite;
	Entry->Callback = Callback;
	Entry->Data = Data;
	++Queue->EntryCount;

	SDL_CompilerBarrier();
	SDL_AtomicSet((SDL_atomic_t*)&Queue->NextToWrite, NewNextToWrite);

	SDL_SemPost((SDL_sem*)Queue->Semaphore);
}

b32  SDLDoNextWork(thread_queue* Queue) {
	b32 ShouldSleep = 0;

	SDL_CompilerBarrier();

	int NextToRead = Queue->NextToRead;
	int NewNextToRead = (NextToRead + 1) % ArrayCount(Queue->Entries);
	if (NextToRead != Queue->NextToWrite) {
		if (SDL_AtomicCAS((SDL_atomic_t*)&Queue->NextToRead, NextToRead, NewNextToRead) == SDL_TRUE) {
			thread_queue_entry* Entry = Queue->Entries + NextToRead;
			Entry->Callback(Entry->Data);
			SDL_AtomicAdd((SDL_atomic_t*)&Queue->FinishedEntries, 1);
		}
	}
	else {
		ShouldSleep = 1;
	}

	return(ShouldSleep);
}

int SDLWorkerThread(void* Param){
	sdl_thread_entry* Thread = (sdl_thread_entry*)Param;
	thread_queue* Queue = Thread->Queue;

	for(;;){
		if (SDLDoNextWork(Queue)) {
			SDL_SemWait((SDL_sem*)Queue->Semaphore);
		}
	}
}

void SDLCompleteQueueWork(thread_queue* Queue){
	while(Queue->FinishedEntries != Queue->EntryCount){
		SDLDoNextWork(Queue);
	}
	Queue->FinishedEntries = 0;
	Queue->EntryCount = 0;
}

void SDLInitThreadQueue(thread_queue* Queue, sdl_thread_entry* Threads, int ThreadCount) {
	Queue->EntryCount = 0;
	Queue->FinishedEntries = 0;

	Queue->NextToRead = 0;
	Queue->NextToWrite = 0;

	Queue->Semaphore = SDL_CreateSemaphore(0);

	for (int ThreadIndex = 0;
		ThreadIndex < ThreadCount;
		ThreadIndex++) 
	{
		sdl_thread_entry* Entry = Threads + ThreadIndex;
		Entry->Queue = Queue;

		SDL_Thread* Thread = SDL_CreateThread(SDLWorkerThread, 0, Entry);
	}
}

void SDLDestroyThreadQueue(thread_queue* Queue, sdl_thread_entry* Threads, int ThreadCount) {

}

#endif

struct cellural_buffer {
	u8* Buf;
	i32 Width;
	i32 Height;
};

static cellural_buffer AllocateCelluralBuffer(i32 Width, i32 Height) {
	cellural_buffer Buffer;
	Buffer.Buf = (u8*)calloc(Width * Height, 1);
	Buffer.Width = Width;
	Buffer.Height = Height;

	return(Buffer);
}

static void DeallocateCelluralBuffer(cellural_buffer* Buffer) {
	if (Buffer->Buf) {
		free(Buffer->Buf);
	}
	Buffer->Width = 0;
	Buffer->Height = 0;
}

static void CelluralGenerateCave(cellural_buffer* Buffer, float FillPercentage, random_state* RandomState) {

	u8* At = Buffer->Buf;
	u8* To = Buffer->Buf;
	for (i32 Y = 0; Y < Buffer->Height; Y++) {
		for (i32 X = 0; X < Buffer->Width; X++) {

			u8 randval = XORShift32(RandomState) % 101;
			b32 ShouldBeFilled = (randval <= FillPercentage);
			if (ShouldBeFilled) {
				*To = 1;
			}
			else {
				*To = 0;
			}

			++To;
			++At;
		}
	}

#if 1
	for (int SmoothIteration = 0;
		SmoothIteration < 3;
		SmoothIteration++) 
	{
		cellural_buffer Temp = AllocateCelluralBuffer(Buffer->Width, Buffer->Height);

		At = Buffer->Buf;
		To = Temp.Buf;
		for (i32 Y = 0; Y < Buffer->Height; Y++) {
			for (i32 X = 0; X < Buffer->Width; X++) {

				if (X == 0 ||
					X == (Buffer->Width - 1) ||
					Y == 0 ||
					Y == (Buffer->Height - 1))
				{
					*To = 0;
					At++;
					To++;
					continue;
				}

				u8* Neighbours[8];
				Neighbours[0] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X - 1;
				Neighbours[1] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X;
				Neighbours[2] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X + 1;
				Neighbours[3] = Buffer->Buf + (Y * Buffer->Width) + X - 1;
				Neighbours[4] = Buffer->Buf + (Y * Buffer->Width) + X + 1;
				Neighbours[5] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X - 1;
				Neighbours[6] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X;
				Neighbours[7] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X + 1;

				int WallCount = 0;
				for (int i = 0; i < 8; i++) {
					if (*Neighbours[i]) {
						WallCount++;
					}
				}

				if (WallCount > 4) {
					*To = 1;
				}
				else if (WallCount < 4) {
					*To = 0;
				}
				else {
					*To = *At;
				}

				++To;
				++At;
			}
		}

#if 0
		At = Temp.Buf;
		To = Buffer->Buf;
		for (i32 Y = 0; Y < Buffer->Height; Y++) {
			for (i32 X = 0; X < Buffer->Width; X++) {
				*To++ = *At++;
			}
		}
#else
		CopyMemory(Buffer->Buf, Temp.Buf, Buffer->Width * Buffer->Height);
#endif

		DeallocateCelluralBuffer(&Temp);
	}
#endif
}

#define CELLURAL_CELL_WIDTH 4
static rgba_buffer CelluralBufferToRGBA(cellural_buffer* Buffer) {
	rgba_buffer Res = AllocateRGBABuffer(
		Buffer->Width * CELLURAL_CELL_WIDTH, 
		Buffer->Height * CELLURAL_CELL_WIDTH);

	render_stack Stack = BeginRenderStack(10 * 1024 * 1024);

	u8* At = Buffer->Buf;
	for (i32 Y = 0; Y < Buffer->Height; Y++) {
		for (i32 X = 0; X < Buffer->Width; X++) {

			rect2 DrawRect;
			DrawRect.Min.x = X * CELLURAL_CELL_WIDTH;
			DrawRect.Min.y = Y * CELLURAL_CELL_WIDTH;
			DrawRect.Max.x = (X + 1) * CELLURAL_CELL_WIDTH;
			DrawRect.Max.y = (Y + 1) * CELLURAL_CELL_WIDTH;

			v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f);
			if (*At) {
				Color = V4(0.9f, 0.1f, 0.1f, 1.0f);
			}

			/*
			extern void RenderRectFast(
				rgba_buffer* Buffer,
				v2 P,
				v2 Dim,
				v4 ModulationColor01,
				rect2 ClipRect);
			*/

			//RenderRectFast

			PushRect(&Stack, DrawRect, Color);

			++At;
		}
	}

	RenderDickInjection(&Stack, &Res);

	EndRenderStack(&Stack);

	return(Res);
}

int main(int ArgsCount, char** Args) {

	int SdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

	sdl_thread_entry RenderThreadEntries[4];
	thread_queue RenderThreadQueue;
	SDLInitThreadQueue(&RenderThreadQueue, RenderThreadEntries, 4);

	PlatformApi.AddEntry = SDLAddEntry;
	PlatformApi.FinishAll = SDLCompleteQueueWork;
	PlatformApi.RenderQueue = &RenderThreadQueue;

	if (SdlInitCode < 0) {
		printf("ERROR: SDL has been not initialized");
	}

	GlobalBuffer = AllocateRGBABuffer(1366, 768);
	GlobalPerfomanceCounterFrequency = SDL_GetPerformanceFrequency();
	GlobalTime = 0.0f;

	SDL_Window* Window = SDL_CreateWindow(
		"GORE",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GlobalBuffer.Width,
		GlobalBuffer.Height,
		SDL_WINDOW_OPENGL);

	if (!Window) {
		printf("ERROR: Window is not created");
	}

	SDLSetWindowIcon(Window);

	SDL_Renderer* renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer) {
		printf("ERROR: Renderer is not created");
	}

	random_state CellRandom = InitRandomStateWithSeed(1234);
	cellural_buffer Cellural = AllocateCelluralBuffer(64, 64);
	CelluralGenerateCave(&Cellural, 55, &CellRandom);
	rgba_buffer CelluralBitmap = CelluralBufferToRGBA(&Cellural);

	rgba_buffer Image = LoadIMG("../Data/Images/image.bmp");
	rgba_buffer AlphaImage = LoadIMG("../Data/Images/alpha.png");
	rgba_buffer PotImage = LoadIMG("../Data/Images/pot.png");

	font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Regular.ttf", 15);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/arial.ttf", 20);

	GUIInitState(GUIState, &FontInfo, &GlobalInput, GlobalBuffer.Width, GlobalBuffer.Height);

	float TempFloatForSlider = 4.0f;
	float TempFloatForVertSlider = 0.0f;
	b32 TempBoolForSlider = false;

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

		render_stack Stack_ = BeginRenderStack(MEGABYTES(1));
		render_stack* Stack = &Stack_;

		float GradR = sin(GlobalTime + 0.5f) * 0.5f + 0.5f;
		float GradG = cos(GlobalTime + 0.5f) * 0.4f + 0.5f;
		float GradB = sin(GlobalTime * 2.0f + 0.5f) * 0.5f + 0.5f;

		float AlphaImageX1 = sin(GlobalTime * 2 * 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX2 = cos(GlobalTime * 6) * 900 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX3 = sin(GlobalTime * 3 + 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5f;

		//PushGradient(Stack, V3(GradR, GradG, GradB));
		PushClear(Stack, V3(0.08f, 0.08f, 0.08f));
		//PushClear(Stack, V3(0.5, 0.5f, 0.5f));
		//PushBitmap(Stack, &Image, { 0, 0 }, 800);
		//DrawCelluralBuffer(Stack, &Cellural);
		if (TempBoolForSlider) {
			PushBitmap(Stack, &CelluralBitmap, V2(0, 0), CelluralBitmap.Height);
		}

		//PushBitmap(Stack, &AlphaImage, V2(AlphaImageX1, 400), 300.0f);
		//PushBitmap(Stack, &AlphaImage, V2(AlphaImageX2, 600), 300.0f);
		//PushBitmap(Stack, &AlphaImage, V2(AlphaImageX3, 200), 300.0f);
		//
		//PushRect(Stack, V2(AlphaImageX1, 400), V2(100, 100), V4(1.0f, 1.0f, 1.0f, 0.5f));


		gui_interaction BoolInteract = GUIVariableInteraction(&TempBoolForSlider, GUIVarType_B32);
		gui_interaction SliderInteract = GUIVariableInteraction(&TempFloatForSlider, GUIVarType_F32);
		gui_interaction VertSliderInteract = GUIVariableInteraction(&TempFloatForVertSlider, GUIVarType_F32);
		

		gui_interaction AlphaImageInteraction = GUIVariableInteraction(&AlphaImage, GUIVarType_RGBABuffer);
		gui_interaction PotImageInteraction = GUIVariableInteraction(&Image, GUIVarType_RGBABuffer);
		gui_interaction LabirImageInteraction = GUIVariableInteraction(&CelluralBitmap, GUIVarType_RGBABuffer);

		GUIBeginFrame(GUIState, Stack);
		char DebugStr[128];
		float LastFrameFPS = 1000.0f / LastMSPerFrame;
		sprintf(DebugStr, "Hello world! %.2fmsp/f %.2fFPS", LastMSPerFrame, LastFrameFPS);

#if 1
		GUIText(GUIState, DebugStr);
		GUIBeginView(GUIState, "Root", GUIView_Tree);

		GUITreeBegin(GUIState, "Test");
		GUIBeginRow(GUIState);
		GUIBoolButton(GUIState, "Button1", &BoolInteract);

		GUIWindow(
			GUIState, 
			"WindowName", 
			GUIWindow_DefaultSize | 
			GUIWindow_Resizable |
			GUIWindow_TopBar,
			0, 0);

		GUIVerticalSlider(GUIState, "VertSlider1", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider2", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider3", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider4", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider5", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider6", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider7", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider8", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider9", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider10", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider11", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider12", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider13", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider14", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider15", -10.0f, 10.0f, &VertSliderInteract);
		//GUIVerticalSlider(GUIState, "VertSlider16", -10.0f, 10.0f, &VertSliderInteract);
		GUIVerticalSlider(GUIState, "VertSlider17", -10.0f, 10.0f, &VertSliderInteract);
		GUIBoolButton(GUIState, "Button2", &BoolInteract);
		GUIActionText(GUIState, "ActionTextggg", &BoolInteract);
		GUIEndRow(GUIState);

		GUIBoolButton(GUIState, "Button3", &BoolInteract);
		GUIBoolButton(GUIState, "Button4", &BoolInteract);

		GUIBeginRow(GUIState);
		GUIButton(GUIState, "MyButton1", &BoolInteract);
		GUIButton(GUIState, "MyButton2", &BoolInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
		//GUISlider(GUIState, "Slider1", 0.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		//GUISlider(GUIState, "Slider2", -1000.0f, 10.0f, &SliderInteract);
		GUIText(GUIState, "Hello");
		GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);

		GUIBeginView(GUIState, "InnerView", GUIView_Tree);
		GUIText(GUIState, "Nikita loh");
		GUITreeBegin(GUIState, "InnerTree");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUIText(GUIState, "Nikita loh");
		GUITreeEnd(GUIState);
		GUITreeBegin(GUIState, "InnerTree1");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUIText(GUIState, "Ivan loh");
		GUITreeEnd(GUIState);
		GUIText(GUIState, "Dima pidor");
		GUIEndView(GUIState, GUIView_Tree);

		GUIEndRow(GUIState);

		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "Test2");
		GUIImageView(GUIState, "CelluralImage", &LabirImageInteraction);
		GUIImageView(GUIState, "AlphaImage", &AlphaImageInteraction);
		GUIImageView(GUIState, "PotImage", &PotImageInteraction);
		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "Test3");
		GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);

		GUIBeginRow(GUIState);
		GUIText(GUIState, "Hello");
		GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUIText(GUIState, "Hello");
		GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUIText(GUIState, "Hello");
		GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);		GUIBeginRow(GUIState);
		GUISlider(GUIState, "Slider0", -10.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);

		GUIBeginRow(GUIState);
		GUIText(GUIState, "Hello");
		GUISlider(GUIState, "Slider3", 0.0f, 10.0f, &SliderInteract);
		GUIEndRow(GUIState);
		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "Audio");
		GUIColorView(GUIState, V4(0.4f, 0.0f, 1.0f, 1.0f), "asd");
		GUIVector2View(GUIState, V2(1.0f, 256.0f), "Vector2");
		GUIVector3View(GUIState, V3(1.0f, 20.0f, 300.0f), "Vector3");
		GUIVector4View(GUIState, V4(12345.0f, 1234.0f, 123456.0f, 5324123.0f), "Vector4");
		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "DEBUG");
		GUITreeEnd(GUIState);

		//GUILabel(GUIState, "Label", V2(GlobalInput.MouseX, GlobalInput.MouseY));
		GUIEndView(GUIState, GUIView_Tree);

		GUIText(GUIState, DebugStr);
#endif

		RenderDickInjectionMultithreaded(&RenderThreadQueue, Stack, &GlobalBuffer);

		GUIEndFrame(GUIState);
		EndRenderStack(Stack);

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

		GlobalTime += SPerFrame;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	DeallocateRGBABuffer(&GlobalBuffer);
	DeallocateCelluralBuffer(&Cellural);

	printf("Program has been succesfully ended\n");

	system("pause");
	return(0);
}