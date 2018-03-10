#include <SDL.h>

#include <stdio.h>
#include <thread>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include "gore_game_layer.h"

#include "geometrika.h"

#include "gore_render_stack.h"
#include "gore_renderer.h"
#include "gore_engine.h"

#include "gore_opengl.h"

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLUNIFORM3UIPROC glUniform3ui;
PFNGLUNIFORM4UIPROC glUniform4ui;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORM1UIVPROC glUniform1uiv;
PFNGLUNIFORM2UIVPROC glUniform2uiv;
PFNGLUNIFORM3UIVPROC glUniform3uiv;
PFNGLUNIFORM4UIVPROC glUniform4uiv;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
MYPFNGLDRAWELEMENTSPROC _glDrawElements;

/*
	NOTE(Dima):
		Images are stored in premultiplied alpha format
*/

/*
	TODO(Dima):
		DEBUG:
			VARARG macro functions
			Sort clock list functions
			Reorganize sections stuff

		GUI:
			Push text to list and render at the end of the frame... This should optimize OpenGL texture bindings
			New named-color system. Think about how to retrieve and store. Get by name maybe

			New Label system or depth
			Overlapping interactions handling

			Caching some elements calculations.

			GUI radio buttons
			GUI text windows
			GUI input text maybe

		Sound:
			Implement sound mixer
			Optimize sound mixer with SSE

		Assets:
			Implement asset packer
			Asset streaming
			Model loading

		Renderer:
			Implement bitmap alignment
			Optimize renderer with multithreading;
			Implement Gaussian blur

		Memory management:
			Splitting stacked memory
			Dynamically growing memory layout

		Other:
			Begin some debugging stuff
			Split code to platform dependent and platform independent parts

		Platform layer:
			Thread pool
			Correct aspect ratio handling
*/

GLOBAL_VARIABLE b32 GlobalRunning;
GLOBAL_VARIABLE rgba_buffer GlobalBuffer;
GLOBAL_VARIABLE input_system GlobalInput;

GLOBAL_VARIABLE u64 GlobalPerfomanceCounterFrequency;
GLOBAL_VARIABLE float GlobalTime;

platform_api PlatformApi;
debug_record_table GlobalRecordTable_ = {};
debug_record_table *GlobalRecordTable = &GlobalRecordTable_;


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
	FUNCTION_TIMING();

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
	FUNCTION_TIMING();

	int MouseX;
	int MouseY;
	u32 MouseState = SDL_GetMouseState(&MouseX, &MouseY);
	System->LastMouseP = System->MouseP;
	System->MouseP = V2(MouseX, MouseY);

	int GlobalMouseX;
	int GlobalMouseY;
	SDL_GetGlobalMouseState(&GlobalMouseX, &GlobalMouseY);
	System->LastGlobalMouseP = System->GlobalMouseP;
	System->GlobalMouseP = V2(GlobalMouseX, GlobalMouseY);

	//NOTE(dima): Setting center position of the window
	SDL_Window* Window = SDL_GL_GetCurrentWindow();
	int WindowWidth;
	int WindowHeight;
	SDL_GetWindowSize(Window, &WindowWidth, &WindowHeight);
	System->CenterP.x = WindowWidth >> 1;
	System->CenterP.y = WindowHeight >> 1;

	//NOTE(dima): Setting center position of the screen
	SDL_DisplayMode DisplayMode;
	SDL_GetDesktopDisplayMode(0, &DisplayMode);
	System->GlobalCenterP.x = DisplayMode.w >> 1;
	System->GlobalCenterP.y = DisplayMode.h >> 1;

	b32 MRightIsDown = MouseState & SDL_BUTTON_RMASK;
	b32 MLeftIsDown = MouseState & SDL_BUTTON_LMASK;
	b32 MMidIsDown = MouseState & SDL_BUTTON_MMASK;
	b32 MExt1IsDown = MouseState & SDL_BUTTON_X1MASK;
	b32 MExt2IsDown = MouseState & SDL_BUTTON_X2MASK;
}

static gui_state GUIState_;
static gui_state* GUIState = &GUIState_;

static debug_state DEBUGState_;
static debug_state* DEBUGState = &DEBUGState_;

static gl_state GLState_;
static gl_state* GLState = &GLState_;

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

PLATFORM_READ_FILE(SDLReadEntireFile) {
	platform_read_file_result Result = {};

	SDL_RWops* File = SDL_RWFromFile(FilePath, "rb");

	if (File != 0) {
#if 0
		Sint64 FileSize = SDL_RWsize(File);
#else
		Sint64 FileSize;
		SDL_RWseek(File, 0, RW_SEEK_END);
		FileSize = SDL_RWtell(File);
		SDL_RWseek(File, 0, RW_SEEK_SET);
#endif

		Result.Data = SDL_calloc(FileSize + 1, 1);
		Result.Size = FileSize;

		SDL_RWread(File, Result.Data, 1, FileSize);

		SDL_RWclose(File);
	}

	return(Result);
}

PLATFORM_WRITE_FILE(SDLWriteEntireFile) {
	SDL_RWops* File = SDL_RWFromFile(FilePath, "wb");

	SDL_RWwrite(File, Data, 1, Size);

	SDL_RWclose(File);
}

PLATFORM_FREE_FILE_MEMORY(SDLFreeFileMemory) {
	if (FileReadResult->Data) {
		SDL_free(FileReadResult->Data);
	}
}

PLATFORM_PLACE_CURSOR_AT_CENTER(SDLPlaceCursorAtCenter) {
#if 0
	SDL_DisplayMode DisplayMode;
	SDL_GetDesktopDisplayMode(0, &DisplayMode);

	SDL_WarpMouseGlobal(DisplayMode.w >> 1, DisplayMode.h >> 1);
#else
	SDL_Window* Window = SDL_GL_GetCurrentWindow();

	SDL_WarpMouseInWindow(Window, GlobalInput.CenterP.x, GlobalInput.CenterP.y);
#endif
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

b32 SDLDoNextWork(thread_queue* Queue) {
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
		MEMCopy(Buffer->Buf, Temp.Buf, Buffer->Width * Buffer->Height);
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

	render_stack Stack = RENDERBeginStack(10 * 1024 * 1024, Buffer->Width, Buffer->Height);

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

			RENDERPushRect(&Stack, DrawRect, Color);

			++At;
		}
	}

	RenderDickInjection(&Stack, &Res);

	RENDEREndStack(&Stack);

	return(Res);
}

int main(int ArgsCount, char** Args) {

	int SdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

#define SDL_RENDER_THREAD_ENTRIES 4
	sdl_thread_entry RenderThreadEntries[SDL_RENDER_THREAD_ENTRIES];
	thread_queue RenderThreadQueue;
	SDLInitThreadQueue(&RenderThreadQueue, RenderThreadEntries, SDL_RENDER_THREAD_ENTRIES);

	PlatformApi.AddEntry = SDLAddEntry;
	PlatformApi.FinishAll = SDLCompleteQueueWork;
	PlatformApi.RenderQueue = &RenderThreadQueue;
	PlatformApi.ReadFile = SDLReadEntireFile;
	PlatformApi.WriteFile = SDLWriteEntireFile;
	PlatformApi.FreeFileMemory = SDLFreeFileMemory;
	PlatformApi.PlaceCursorAtCenter = SDLPlaceCursorAtCenter;

	if (SdlInitCode < 0) {
		printf("ERROR: SDL has been not initialized");
	}

#define GORE_WINDOW_WIDTH 1366
#define GORE_WINDOW_HEIGHT 768

	GlobalBuffer = AllocateRGBABuffer(GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);
	GlobalPerfomanceCounterFrequency = SDL_GetPerformanceFrequency();
	GlobalTime = 0.0f;

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	SDL_Window* Window = SDL_CreateWindow(
		"GORE",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GORE_WINDOW_WIDTH,
		GORE_WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);

	SDL_GLContext SDLOpenGLRenderContext = SDL_GL_CreateContext(Window);
	SDL_GL_MakeCurrent(Window, SDLOpenGLRenderContext);
	SDL_GL_SetSwapInterval(0);

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
	glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
	glMapBuffer = (PFNGLMAPBUFFERPROC)SDL_GL_GetProcAddress("glMapBuffer");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glGetAttribLocation");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
	glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
	glDetachShader = (PFNGLDETACHSHADERPROC)SDL_GL_GetProcAddress("glDetachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
	glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)SDL_GL_GetProcAddress("glUniform2i");
	glUniform3i = (PFNGLUNIFORM3IPROC)SDL_GL_GetProcAddress("glUniform3i");
	glUniform4i = (PFNGLUNIFORM4IPROC)SDL_GL_GetProcAddress("glUniform4i");
	glUniform1ui = (PFNGLUNIFORM1UIPROC)SDL_GL_GetProcAddress("glUniform1ui");
	glUniform2ui = (PFNGLUNIFORM2UIPROC)SDL_GL_GetProcAddress("glUniform2ui");
	glUniform3ui = (PFNGLUNIFORM3UIPROC)SDL_GL_GetProcAddress("glUniform3ui");
	glUniform4ui = (PFNGLUNIFORM4UIPROC)SDL_GL_GetProcAddress("glUniform4ui");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)SDL_GL_GetProcAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)SDL_GL_GetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)SDL_GL_GetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)SDL_GL_GetProcAddress("glUniform4fv");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)SDL_GL_GetProcAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)SDL_GL_GetProcAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)SDL_GL_GetProcAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)SDL_GL_GetProcAddress("glUniform4iv");
	glUniform1uiv = (PFNGLUNIFORM1UIVPROC)SDL_GL_GetProcAddress("glUniform1uiv");
	glUniform2uiv = (PFNGLUNIFORM2UIVPROC)SDL_GL_GetProcAddress("glUniform2uiv");
	glUniform3uiv = (PFNGLUNIFORM3UIVPROC)SDL_GL_GetProcAddress("glUniform3uiv");
	glUniform4uiv = (PFNGLUNIFORM4UIVPROC)SDL_GL_GetProcAddress("glUniform4uiv");
	glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2fv");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
	glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2x3fv");
	glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3x2fv");
	glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2x4fv");
	glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4x2fv");
	glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3x4fv");
	glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4x3fv");
	_glDrawElements = (MYPFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");

	if (!Window) {
		printf("ERROR: Window is not created");
	}

	SDLSetWindowIcon(Window);

	SDL_Renderer* renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
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

	font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Bold.ttf", 18);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Regular.ttf", 20);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/arial.ttf", 20);

	geometrika_state GameState = {};

	GUIInitState(GUIState, &FontInfo, &GlobalInput, GlobalBuffer.Width, GlobalBuffer.Height);
	OpenGLInitState(GLState);
	DEBUGInit(DEBUGState, GUIState);

	float TempFloatForSlider = 4.0f;
	float TempFloatForVertSlider = 0.0f;
	b32 TempBoolForSlider = false;

	float LastMSPerFrame = 0.0f;

	GlobalRunning = true;
	while (GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		DEBUG_FRAME_BARRIER(LastMSPerFrame);

		BEGIN_SECTION("LayerSDL");
		ProcessEvents(Window, &GlobalInput);

		ProcessInput(&GlobalInput);

		if (ButtonWentDown(&GlobalInput, KeyType_Esc)) {
			GlobalRunning = false;
			break;
		}

		if (ButtonWentDown(&GlobalInput, KeyType_F12)) {
			SDLGoFullscreen(Window);
		}


		render_stack Stack_ = RENDERBeginStack(MEGABYTES(1), GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);
		render_stack* Stack = &Stack_;

		float GradR = sin(GlobalTime + 0.5f) * 0.5f + 0.5f;
		float GradG = cos(GlobalTime + 0.5f) * 0.4f + 0.5f;
		float GradB = sin(GlobalTime * 2.0f + 0.5f) * 0.5f + 0.5f;

		float AlphaImageX1 = sin(GlobalTime * 2 * 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX2 = cos(GlobalTime * 6) * 900 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5;
		float AlphaImageX3 = sin(GlobalTime * 3 + 0.5f) * 400 + GlobalBuffer.Width * 0.5f - AlphaImage.Width * 0.5f;

		//RENDERPushGradient(Stack, V3(GradR, GradG, GradB));
		RENDERPushClear(Stack, V3(0.1f, 0.1f, 0.1f));
		//RENDERPushClear(Stack, V3(0.5, 0.5f, 0.5f));
		//RENDERPushBitmap(Stack, &Image, { 0, 0 }, 800);
		//DrawCelluralBuffer(Stack, &Cellural);
		//RENDERPushRect(Stack, Rect2MinDim(V2(0, 0), V2(300, 100)), V4(1.0f, 0.4f, 0.0f, 1.0f));
		if (TempBoolForSlider) {
			RENDERPushBitmap(Stack, &CelluralBitmap, V2(0, 0), CelluralBitmap.Height);
		}

		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX1, 400), 300.0f);
		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX2, 600), 300.0f);
		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX3, 200), 300.0f);
		//
		//RENDERPushRect(Stack, V2(AlphaImageX1, 400), V2(100, 100), V4(1.0f, 1.0f, 1.0f, 0.5f));


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

		GUIText(GUIState, DebugStr);
#if 0

		GUIBeginLayout(GUIState, "Root", GUILayout_Tree);

		GUITreeBegin(GUIState, "Root");
		GUITreeBegin(GUIState, "Test");
		GUITreeBegin(GUIState, "Other");
		GUIBeginRow(GUIState);
		GUIBoolButton(GUIState, "Button1", &TempBoolForSlider);

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
		GUIBoolButton(GUIState, "Button2", &TempBoolForSlider);
		GUIActionText(GUIState, "ActionTextggg", &BoolInteract);
		GUIEndRow(GUIState);

		DEBUGFramesGraph(DEBUGState);
		DEBUGFramesSlider(DEBUGState);

		GUIBoolButton(GUIState, "Button3", &TempBoolForSlider);
		GUIBoolButton(GUIState, "Button4", &TempBoolForSlider);

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

#if 1
		GUIBeginLayout(GUIState, "InnerView", GUILayout_Tree);
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
		GUIEndLayout(GUIState, GUILayout_Tree);
#endif
		GUIEndRow(GUIState);
		GUITreeEnd(GUIState);// Other...

#if 1
		GUITreeBegin(GUIState, "TestMenus");
		GUIBeginMenuBar(GUIState, "Menu1");

		GUIBeginMenuBarItem(GUIState, "Dima");
		GUIMenuBarItem(GUIState, "Item_1_1");
		GUIMenuBarItem(GUIState, "Item_1_2");
		GUIMenuBarItem(GUIState, "Item_1_3");
		GUIMenuBarItem(GUIState, "Item_1_1asdf");
		GUIMenuBarItem(GUIState, "Item_1_2asdf");
		GUIMenuBarItem(GUIState, "Item_1_3asdf");
		GUIMenuBarItem(GUIState, "Item_1_1asdfasdf");
		GUIMenuBarItem(GUIState, "Item_1_2asdfasdf");
		GUIMenuBarItem(GUIState, "Item_1_3asdfasdf");
		GUIEndMenuBarItem(GUIState);

		GUIBeginMenuBarItem(GUIState, "Ivan");
		GUIMenuBarItem(GUIState, "Item_2_1");
		GUIMenuBarItem(GUIState, "Item_2_2");
		GUIMenuBarItem(GUIState, "Item_2_3");
		GUIMenuBarItem(GUIState, "Item_2_1asdf");
		GUIMenuBarItem(GUIState, "Item_2_2asdf");
		GUIMenuBarItem(GUIState, "Item_2_3asdf");
		GUIMenuBarItem(GUIState, "Item_2_1asdfasdf");
		GUIMenuBarItem(GUIState, "Item_2_2asdfasdf");
		GUIMenuBarItem(GUIState, "Item_2_3asdfasdf");
		GUIEndMenuBarItem(GUIState);

		GUIBeginMenuBarItem(GUIState, "Vovan");
		GUIMenuBarItem(GUIState, "Item_3_1");
		GUIMenuBarItem(GUIState, "Item_3_2");
		GUIMenuBarItem(GUIState, "Item_3_3");
		GUIEndMenuBarItem(GUIState);
		GUIEndMenuBar(GUIState);
		GUITreeEnd(GUIState);
#endif

		GUITreeBegin(GUIState, "TestText");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUIText(GUIState, "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZMNOPQRSTUVWXYZ");
		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "Colors");
		for (int ColorIndex = 0;
			ColorIndex < Min(30, GUIColor_Count);
			ColorIndex++)
		{
			gui_color_slot* Slot = &GUIState->ColorTable[ColorIndex];

			char ColorNameBuf[32];
			stbsp_sprintf(ColorNameBuf, "%-15s", Slot->Name);

			GUIColorView(GUIState, Slot->Color, ColorNameBuf);
		}
		GUITreeEnd(GUIState);

		GUITreeEnd(GUIState); //Test

		GUITreeBegin(GUIState, "Test2");
		GUIImageView(GUIState, "CelluralImage", &CelluralBitmap);
		GUIImageView(GUIState, "AlphaImage", &AlphaImage);
		GUIStackedMemGraph(GUIState, "NULLMemGraph", 0);
		GUIStackedMemGraph(GUIState, "NullImageTest", 0);
		GUIImageView(GUIState, "PotImage", &Image);
		GUIImageView(GUIState, "FontAtlas", &FontInfo.FontAtlasImage);
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
		GUIBeginRow(GUIState);
		GUIVector2View(GUIState, V2(1.0f, 256.0f), "Vector2");
		GUIText(GUIState, "Hello");
		GUIVector3View(GUIState, V3(1.0f, 20.0f, 300.0f), "Vector3");
		GUIEndRow(GUIState);
		GUIVector4View(GUIState, V4(12345.0f, 1234.0f, 123456.0f, 5324123.0f), "Vector4");
		GUIInt32View(GUIState, 12345, "TestInt");
		GUITreeEnd(GUIState);

		GUITreeBegin(GUIState, "RENDER");
		GUIStackedMemGraph(GUIState, "RenderMemGraph", &Stack->Data);
		GUIInt32View(GUIState, Stack->EntryCount, "RenderEntriesCount");
		GUITreeEnd(GUIState);

		GUITreeEnd(GUIState);

		//GUILabel(GUIState, "Label", V2(GlobalInput.MouseX, GlobalInput.MouseY));
		GUIEndLayout(GUIState, GUILayout_Tree);

		//GUIText(GUIState, DebugStr);
#endif

		DEBUGUpdate(DEBUGState);

		//GEOMKAUpdateAndRender(&GameState, Stack, &GlobalInput);


#if 1
		glViewport(0, 0, GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);

		OpenGLRenderStackToOutput(GLState, Stack);
		
		SDL_GL_SwapWindow(Window);
	
#else
		BEGIN_SECTION("RENDERING");
		RenderMultithreaded(&RenderThreadQueue, Stack, &GlobalBuffer);

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
		END_SECTION();
#endif

		GUIEndFrame(GUIState);
		RENDEREndStack(Stack);

		END_SECTION();

		float SPerFrame = SDLGetMSElapsed(FrameBeginClocks);
		LastMSPerFrame = SPerFrame * 1000.0f;
		GlobalInput.DeltaTime = SPerFrame;

		GlobalTime += SPerFrame;
	}

	SDL_GL_DeleteContext(SDLOpenGLRenderContext);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	DeallocateRGBABuffer(&GlobalBuffer);
	DeallocateCelluralBuffer(&Cellural);

	printf("Program has been succesfully ended\n");

	return(0);
}