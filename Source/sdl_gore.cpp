#include "sdl_gore.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include "geometrika.h"
/*
	NOTE(Dima):
		Images are stored in premultiplied alpha format
*/

/*
	TODO(Dima):

		Assets:
			Asset streaming
			Implement asset packer
			Model loading

			Generate UV's for cylynder and sphere

		DEBUG:
			Thread intervals for non-one frame records

			DEBUG console

			One timing - multiple hit counts
			
		GUI:
			WHOLE NEW GUI SYSTEM!!! Think about it)

			Message boxes

			Interaction rules list
			Interaction rules list processing

			Push text to list and render at the end of the frame... This should optimize OpenGL texture bindings
			New named-color system. Think about how to retrieve and store. Get by name maybe

			Finish menus

			Overlapping interactions handling

			GUI text windows
			GUI input text maybe

		Sound:
			Implement sound mixer
			Optimize sound mixer with SSE

		Renderer:
			Implement Gaussian blur

		Memory management:
			Dynamically growing memory layout

		Other:

		Platform layer:
			Correct aspect ratio handling
*/

GLOBAL_VARIABLE b32 GlobalRunning;
GLOBAL_VARIABLE bitmap_info GlobalBuffer;
GLOBAL_VARIABLE input_system GlobalInput;
GLOBAL_VARIABLE asset_system GlobalAssets;

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

static void SDLProcessEvents(SDL_Window* Window, input_system* Input) {
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
					case(SDLK_LSHIFT): {
						ProcessButton = &Input->Buttons[KeyType_LShift];
					}break;
					case(SDLK_RSHIFT): {
						ProcessButton = &Input->Buttons[KeyType_RShift];
					}break;
					case(SDLK_LCTRL): {
						ProcessButton = &Input->Buttons[KeyType_LCtrl];
					}break;
					case(SDLK_RCTRL): {
						ProcessButton = &Input->Buttons[KeyType_RCtrl];
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

static void SDLProcessInput(input_system* System) {
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

static color_state ColorState_;
static color_state* ColorState = &ColorState_;

inline SDL_Surface* SDLSurfaceFromBuffer(bitmap_info* Buffer) {
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
	bitmap_info Image = LoadIMG("../Data/Images/pot32.png");
	Surface = SDLSurfaceFromBuffer(&Image);
	// The icon is attached to the window pointer
	SDL_SetWindowIcon(Window, Surface);

	// ...and the surface containing the icon pixel data is no longer required.
	SDL_FreeSurface(Surface);
}

PLATFORM_READ_FILE(SDLReadEntireFile) {
	platform_read_file_result Result = {};

	FILE* fp = fopen(FilePath, "rb");

	if (fp) {
		fseek(fp, 0, 2);
		u64 FileSize = ftell(fp);
		fseek(fp, 0, 0);

		Result.Size = FileSize;
		Result.Data = (u8*)calloc(FileSize + 1, 1);

		fread(Result.Data, 1, FileSize, fp);

		fclose(fp);
	}

	return(Result);
}

PLATFORM_WRITE_FILE(SDLWriteEntireFile) {
	FILE* File = (FILE*)fopen(FilePath, "wb");

	if (File) {
		fwrite(Data, 1, Size, File);

		fclose(File);
	}
}

PLATFORM_FREE_FILE_MEMORY(SDLFreeFileMemory) {
	if (FileReadResult->Data) {
		free(FileReadResult->Data);
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

PLATFORM_TERMINATE_PROGRAM(SDLTerminateProgram) {
	GlobalRunning = 0;
}


#if defined(PLATFORM_WINDA)

PLATFORM_ADD_THREADWORK_ENTRY(WindaAddThreadworkEntry) {
	int EntryIndex = Queue->AddIndex;
	int NewToSet = (EntryIndex + 1) % Queue->EntriesCount;
	platform_threadwork* Entry = Queue->Entries + EntryIndex;

	Entry->Callback = Callback;
	Entry->Data = Data;

	Queue->StartedEntries++;

	_ReadWriteBarrier();

	_InterlockedExchange((volatile unsigned int*)&Queue->AddIndex, NewToSet);
	
	ReleaseSemaphore(Queue->Semaphore, 1, 0);
}

b32 WindaDoNextThreadwork(platform_thread_queue* Queue) {
	b32 NoWorkLeft = 0;

	int OriginalToDoIndex = Queue->DoIndex;
	int NewToSet = (OriginalToDoIndex + 1) % Queue->EntriesCount;

	if (Queue->DoIndex != Queue->AddIndex) {
		if (InterlockedCompareExchange((volatile unsigned int*)&Queue->DoIndex, NewToSet, OriginalToDoIndex) == OriginalToDoIndex) {
			platform_threadwork* Work = Queue->Entries + OriginalToDoIndex;

			Work->Callback(Work->Data);

			_ReadWriteBarrier();

			InterlockedIncrement((volatile unsigned int*)&Queue->FinishedEntries);
		}
	}
	else {
		NoWorkLeft = 1;
	}

	//Assert(Queue->FinishedEntries <= Queue->StartedEntries);

	return(NoWorkLeft);
}

DWORD WINAPI WindaThreadWorkerWork(void* Data) {
	winda_thread_worker* Worker = (winda_thread_worker*)Data;
	platform_thread_queue* Queue = Worker->Queue;

	for (;;) {
		if (WindaDoNextThreadwork(Queue)) {
			WaitForSingleObject(Queue->Semaphore, INFINITE);
		}
	}

	return(0);
}

PLATFORM_COMPLETE_THREAD_WORKS(WindaCompleteThreadWorks) {
	while (Queue->FinishedEntries != Queue->StartedEntries) {
		WindaDoNextThreadwork(Queue);
	}

	Queue->FinishedEntries = 0;
	Queue->StartedEntries = 0;
}

void WindaInitThreadQueue(
	platform_thread_queue* Queue, 
	winda_thread_worker* Workers, 
	int WorkersCount, 
	char* QueueName,
	platform_threadwork* Entries,
	u32 EntriesCount) 
{
	Queue->AddIndex = 0;
	Queue->DoIndex = 0;
	Queue->FinishedEntries = 0;
	Queue->StartedEntries = 0;
	Queue->Entries = Entries;
	Queue->EntriesCount = EntriesCount;
	Queue->WorkingThreadsCount = WorkersCount;

	char SemaphoreNameBuf[256];
	stbsp_sprintf(SemaphoreNameBuf, "%s_Sem", QueueName);

	Queue->Semaphore = CreateSemaphoreExA(0, 0, WorkersCount, SemaphoreNameBuf, 0, SEMAPHORE_ALL_ACCESS);
	Queue->QueueName = QueueName;
	
	for (int i = 0; i < WorkersCount; i++) {
		winda_thread_worker* Worker = Workers + i;

		Worker->Queue = Queue;
		DWORD ThreadID;

		Worker->ThreadHandle = CreateThread(0, 0, WindaThreadWorkerWork, Worker, 0, &ThreadID);
		Worker->ThreadID = ThreadID;
	}
}

PLATFORM_GET_THREAD_ID(WindaGetThreadID) {
	u32 Result = GetCurrentThreadId();

	return(Result);
}

PLATFORM_GET_THREAD_QUEUE_INFO(WindaGetThreadQueueInfo) {
	platform_thread_queue_info Result = {};

	Result.TotalEntriesCount = Queue->EntriesCount;
	Result.WorkingThreadsCount = Queue->WorkingThreadsCount;
	
	u32 EntriesBusy = 0;
	if (Queue->DoIndex <= Queue->AddIndex) {
		EntriesBusy = Queue->AddIndex - Queue->DoIndex;
	}
	else {
		EntriesBusy = Queue->EntriesCount - Queue->DoIndex + Queue->AddIndex;
	}
	Result.EntriesBusy = EntriesBusy;

	return(Result);
}

PLATFORM_COMPILER_BARRIER_TYPE(WindaReadWriteBarrier) {
	_ReadWriteBarrier();
}

PLATFORM_COMPILER_BARRIER_TYPE(WindaWriteBarrier) {
	_WriteBarrier();
}

PLATFORM_COMPILER_BARRIER_TYPE(WindaReadBarrier) {
	_ReadBarrier();
}

#include <intrin.h>
PLATFORM_ATOMIC_CAS_I32(WindaAtomicCAS_I32) { _InterlockedCompareExchange((platform_atomic_type_i32*)Value, New, Old); return(*Value == New); }
PLATFORM_ATOMIC_CAS_U32(WindaAtomicCAS_U32) { _InterlockedCompareExchange((platform_atomic_type_u32*)Value, New, Old); return(*Value == New); }
PLATFORM_ATOMIC_CAS_I64(WindaAtomicCAS_I64) { _InterlockedCompareExchange((platform_atomic_type_u64*)Value, New, Old); return(*Value == New); }
PLATFORM_ATOMIC_CAS_U64(WindaAtomicCAS_U64) { _InterlockedCompareExchange((platform_atomic_type_u64*)Value, New, Old); return(*Value == New); }

PLATFORM_ATOMIC_ADD_I32(WindaAtomicAdd_I32) { return(_InterlockedExchangeAdd((platform_atomic_type_i32*)Value, Addend));  }
PLATFORM_ATOMIC_ADD_U32(WindaAtomicAdd_U32) { return(_InterlockedExchangeAdd((platform_atomic_type_u32*)Value, Addend));  }
PLATFORM_ATOMIC_ADD_I64(WindaAtomicAdd_I64) { return(_InterlockedExchangeAdd64((platform_atomic_type_i64*)Value, Addend));}
PLATFORM_ATOMIC_ADD_U64(WindaAtomicAdd_U64) { return(_InterlockedExchangeAdd64((platform_atomic_type_i64*)Value, Addend));}

PLATFORM_ATOMIC_SET_I32(WindaAtomicSet_I32) { return(_InterlockedExchange((platform_atomic_type_i32*)Value, New)); }
PLATFORM_ATOMIC_SET_U32(WindaAtomicSet_U32) { return(_InterlockedExchange((platform_atomic_type_u32*)Value, New)); }
PLATFORM_ATOMIC_SET_I64(WindaAtomicSet_I64) { return(_InterlockedExchange((platform_atomic_type_u64*)Value, New)); }
PLATFORM_ATOMIC_SET_U64(WindaAtomicSet_U64) { return(_InterlockedExchange((platform_atomic_type_u64*)Value, New)); }

PLATFORM_ATOMIC_INC_I32(WindaAtomicInc_I32) { return(_InterlockedExchangeAdd((platform_atomic_type_i32*)Value, 1)); }
PLATFORM_ATOMIC_INC_U32(WindaAtomicInc_U32) { return(_InterlockedExchangeAdd((platform_atomic_type_u32*)Value, 1)); }
PLATFORM_ATOMIC_INC_I64(WindaAtomicInc_I64) { return(_InterlockedExchangeAdd64((platform_atomic_type_i64*)Value, 1)); }
PLATFORM_ATOMIC_INC_U64(WindaAtomicInc_U64) { return(_InterlockedExchangeAdd64((platform_atomic_type_i64*)Value, 1)); }

#else

PLATFORM_ADD_THREADWORK_ENTRY(SDLAddThreadworkEntry) {

	int NewToSet = (Queue->AddIndex.value + 1) % Queue->EntriesCount;

	/*
		NOTE(dima): If NewToSet(new value for queue write index)
		is equal to the queue read index than thead queue overlap
		will happen. We can permanently assert it here because
		we can't allow to override work that should be performed.
	*/
	Assert(NewToSet != Queue->DoIndex.value);

	platform_threadwork* Entry = &Queue->Entries[Queue->AddIndex.value];
	Entry->Callback = Callback;
	Entry->Data = Data;
	SDL_AtomicAdd(&Queue->StartedEntries, 1);

	SDL_AtomicSet(&Queue->AddIndex, NewToSet);

	/*
		NOTE(dima): We need to make sure that when we increment
		started entries count, the current threadwork structure
		was properly set. Some optimizing compilers can move lines
		around as they see fit. it can lead to phantome reads.
		F.E if callback wasnt set, code can call it but the
		garbage will be in here. So SDL_CompilerBarrier was used
		here to prevent it.
	*/
	SDL_CompilerBarrier();

	SDL_SemPost(Queue->Semaphore);
}

b32 SDLPerformNextThreadwork(platform_thread_queue* Queue) {
	b32 NoWorkLeft = 0;

	int OriginalToDoIndex = Queue->DoIndex.value;
	int NewToSet = (Queue->DoIndex.value + 1) % Queue->EntriesCount;
	if (OriginalToDoIndex != Queue->AddIndex.value) {
		if (SDL_AtomicCAS(&Queue->DoIndex, OriginalToDoIndex, NewToSet)) {
			platform_threadwork* Work = Queue->Entries + OriginalToDoIndex;

			Work->Callback(Work->Data);

			/*
				NOTE(dima): We need to make sure that the work has
				been performed before we decrement queue started entries
				count. Some optimizing compilers can move lines around,
				for example SDL_AtomicAdd could be performed before
				Work->Callback(Work->Data); So SDL_CompilerBarrier was
				used here to prevent it;

				Seems like I just wrote something the same as that I
				wrote in SDLAddThreadEntry function. Sorry...
			*/
			SDL_CompilerBarrier();
			SDL_AtomicAdd(&Queue->FinishedEntries, 1);
		}
	}
	else {
		NoWorkLeft = 1;
	}

	SDL_CompilerBarrier();

	return(NoWorkLeft);
}


PLATFORM_COMPLETE_THREAD_WORKS(SDLCompleteThreadWorks) {
	while (Queue->FinishedEntries.value != Queue->StartedEntries.value) {
		//NOTE(dima): Main execution thread should also help to perform work!! xD
		SDLPerformNextThreadwork(Queue);
	}

	Queue->StartedEntries.value = 0;
	Queue->FinishedEntries.value = 0;
}

static int SDLThreadWorkerWork(void* Data) {
	platform_thread_queue* Queue = (platform_thread_queue*)Data;

	for (;;) {
		if (SDLPerformNextThreadwork(Queue)){
			SDL_SemWait(Queue->Semaphore);
		}
	}

	return(0);
}

void SDLInitThreadQueue(
	platform_thread_queue* Queue,
	sdl_thread_worker* Workers,
	int ThreadWorkersCount,
	char* ThreadQueueName,
	platform_threadwork* Entries,
	u32 EntriesCount)
{
	Queue->AddIndex.value = 0;
	Queue->DoIndex.value = 0;
	Queue->StartedEntries.value = 0;
	Queue->FinishedEntries.value = 0;
	Queue->Entries = Entries;
	Queue->EntriesCount = EntriesCount;
	Queue->WorkingThreadsCount = ThreadWorkersCount;

	Queue->Semaphore = SDL_CreateSemaphore(0);

	Queue->QueueName = ThreadQueueName;

	for (int i = 0; i < ThreadWorkersCount; i++) {
		sdl_thread_worker* ThreadWorker = &Workers[i];

		char ThreadNameBuf[64];
		stbsp_sprintf(ThreadNameBuf, "%s_%d", ThreadQueueName, i);

		ThreadWorker->Queue = Queue;
		ThreadWorker->ThreadHandle = SDL_CreateThread(SDLThreadWorkerWork, ThreadNameBuf, ThreadWorker->Queue);
		SDL_DetachThread(ThreadWorker->ThreadHandle);
		ThreadWorker->ThreadID = SDL_GetThreadID(ThreadWorker->ThreadHandle);
	}
}

PLATFORM_GET_THREAD_ID(SDLGetThreadID) {
	u32 Result = SDL_ThreadID();

	return(Result);
}

PLATFORM_GET_THREAD_QUEUE_INFO(SDLGetThreadQueueInfo) {
	platform_thread_queue_info Result = {};

	Result.TotalEntriesCount = Queue->EntriesCount;
	Result.WorkingThreadsCount = Queue->WorkingThreadsCount;

	u32 EntriesBusy = 0;
	if (Queue->DoIndex.value <= Queue->AddIndex.value) {
		EntriesBusy = Queue->AddIndex.value - Queue->DoIndex.value;
	}
	else {
		EntriesBusy = Queue->EntriesCount - Queue->DoIndex.value + Queue->AddIndex.value;
	}
	Result.EntriesBusy = EntriesBusy;

	return(Result);
}

PLATFORM_COMPILER_BARRIER_TYPE(SDLCompilerBarrier) {
	SDL_CompilerBarrier();
}

PLATFORM_ATOMIC_CAS_I32(SDLAtomicCAS_I32) { return(SDL_AtomicCAS((SDL_atomic_t*)Value, Old, New)); }
PLATFORM_ATOMIC_CAS_U32(SDLAtomicCAS_U32) { return(SDL_AtomicCAS((SDL_atomic_t*)Value, Old, New)); }
PLATFORM_ATOMIC_CAS_I64(SDLAtomicCAS_I64) { return(SDL_AtomicCAS((SDL_atomic_t*)Value, Old, New)); }
PLATFORM_ATOMIC_CAS_U64(SDLAtomicCAS_U64) { return(SDL_AtomicCAS((SDL_atomic_t*)Value, Old, New)); }

PLATFORM_ATOMIC_ADD_I32(SDLAtomicAdd_I32) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, Addend)); }
PLATFORM_ATOMIC_ADD_U32(SDLAtomicAdd_U32) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, Addend)); }
PLATFORM_ATOMIC_ADD_I64(SDLAtomicAdd_I64) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, Addend)); }
PLATFORM_ATOMIC_ADD_U64(SDLAtomicAdd_U64) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, Addend)); }

PLATFORM_ATOMIC_SET_I32(SDLAtomicSet_I32) { return(SDL_AtomicSet((SDL_atomic_t*)Value, New)); }
PLATFORM_ATOMIC_SET_U32(SDLAtomicSet_U32) { return(SDL_AtomicSet((SDL_atomic_t*)Value, New)); }
PLATFORM_ATOMIC_SET_I64(SDLAtomicSet_I64) { return(SDL_AtomicSet((SDL_atomic_t*)Value, New)); }
PLATFORM_ATOMIC_SET_U64(SDLAtomicSet_U64) { return(SDL_AtomicSet((SDL_atomic_t*)Value, New)); }

PLATFORM_ATOMIC_INC_I32(SDLAtomicInc_I32) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, 1)); }
PLATFORM_ATOMIC_INC_U32(SDLAtomicInc_U32) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, 1)); }
PLATFORM_ATOMIC_INC_I64(SDLAtomicInc_I64) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, 1)); }
PLATFORM_ATOMIC_INC_U64(SDLAtomicInc_U64) { return(SDL_AtomicAdd((SDL_atomic_t*)Value, 1)); }


#endif

int main(int ArgsCount, char** Args) {

	int SdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

	//NOTE(dima): Initializing of threads
	platform_thread_queue VoxelQueue;
	platform_thread_queue HighPriorityQueue;
	platform_thread_queue LowPriorityQueue;

	platform_threadwork VoxelQueueEntries[16384];
	platform_threadwork HighQueueEntries[512];
	platform_threadwork LowQueueEntries[512];

#define PLATFORM_VOXEL_QUEUE_THREADS_COUNT 8
#define PLATFORM_HIGH_QUEUE_THREADS_COUNT 4
#define PLATFORM_LOW_QUEUE_THREADS_COUNT 2

#if defined(PLATFORM_WINDA)
	winda_thread_worker VoxelThreadWorkers[PLATFORM_VOXEL_QUEUE_THREADS_COUNT];
	WindaInitThreadQueue(
		&VoxelQueue,
		VoxelThreadWorkers,
		ArrayCount(VoxelThreadWorkers),
		"VoxelQueue",
		VoxelQueueEntries,
		ArrayCount(VoxelQueueEntries));

	winda_thread_worker HighThreadWorkers[PLATFORM_HIGH_QUEUE_THREADS_COUNT];
	WindaInitThreadQueue(
		&HighPriorityQueue, 
		HighThreadWorkers, 
		ArrayCount(HighThreadWorkers), 
		"HighQueue",
		HighQueueEntries,
		ArrayCount(HighQueueEntries));

	winda_thread_worker LowThreadWorkers[PLATFORM_LOW_QUEUE_THREADS_COUNT];
	WindaInitThreadQueue(
		&LowPriorityQueue, 
		LowThreadWorkers, 
		ArrayCount(LowThreadWorkers), 
		"LowQueue",
		LowQueueEntries,
		ArrayCount(LowQueueEntries));
#else
	sdl_thread_worker VoxelThreadWorkers[PLATFORM_VOXEL_QUEUE_THREADS_COUNT];
	SDLInitThreadQueue(
		&VoxelQueue,
		VoxelThreadWorkers,
		ArrayCount(VoxelThreadWorkers),
		"VoxelQueue",
		VoxelQueueEntries,
		ArrayCount(VoxelQueueEntries));

	sdl_thread_worker HighThreadWorkers[PLATFORM_HIGH_QUEUE_THREADS_COUNT];
	SDLInitThreadQueue(
		&HighPriorityQueue, 
		HighThreadWorkers, 
		ArrayCount(HighThreadWorkers), 
		"HighQueue",
		HighQueueEntries,
		ArrayCount(HighQueueEntries));

	sdl_thread_worker LowThreadWorkers[PLATFORM_LOW_QUEUE_THREADS_COUNT];
	SDLInitThreadQueue(
		&LowPriorityQueue, 
		LowThreadWorkers, 
		ArrayCount(LowThreadWorkers), 
		"LowQueue",
		LowQueueEntries,
		ArrayCount(LowQueueEntries));
#endif

	//NOTE(dima): Initializing of Platform API
#if defined(PLATFORM_WINDA)
	PlatformApi.AddThreadworkEntry = WindaAddThreadworkEntry;
	PlatformApi.CompleteThreadWorks = WindaCompleteThreadWorks;
	PlatformApi.GetThreadID = WindaGetThreadID;
	PlatformApi.GetThreadQueueInfo = WindaGetThreadQueueInfo;

	PlatformApi.ReadWriteBarrier = WindaReadWriteBarrier;
	PlatformApi.ReadBarrier = WindaReadBarrier;
	PlatformApi.WriteBarrier = WindaWriteBarrier;

	PlatformApi.AtomicCAS_I32 = WindaAtomicCAS_I32;
	PlatformApi.AtomicCAS_U32 = WindaAtomicCAS_U32;
	PlatformApi.AtomicCAS_I64 = WindaAtomicCAS_I64;
	PlatformApi.AtomicCAS_U64 = WindaAtomicCAS_U64;

	PlatformApi.AtomicInc_I32 = WindaAtomicInc_I32;
	PlatformApi.AtomicInc_U32 = WindaAtomicInc_U32;
	PlatformApi.AtomicInc_I64 = WindaAtomicInc_I64;
	PlatformApi.AtomicInc_U64 = WindaAtomicInc_U64;

	PlatformApi.AtomicAdd_I32 = WindaAtomicAdd_I32;
	PlatformApi.AtomicAdd_U32 = WindaAtomicAdd_U32;
	PlatformApi.AtomicAdd_I64 = WindaAtomicAdd_I64;
	PlatformApi.AtomicAdd_U64 = WindaAtomicAdd_U64;

	PlatformApi.AtomicSet_I32 = WindaAtomicSet_I32;
	PlatformApi.AtomicSet_U32 = WindaAtomicSet_U32;
	PlatformApi.AtomicSet_I64 = WindaAtomicSet_I64;
	PlatformApi.AtomicSet_U64 = WindaAtomicSet_U64;
#else
	PlatformApi.AddThreadworkEntry = SDLAddThreadworkEntry;
	PlatformApi.CompleteThreadWorks = SDLCompleteThreadWorks;
	PlatformApi.GetThreadID = SDLGetThreadID;
	PlatformApi.GetThreadQueueInfo = SDLGetThreadQueueInfo;


	PlatformApi.ReadWriteBarrier = SDLCompilerBarrier;
	PlatformApi.ReadBarrier = SDLCompilerBarrier;
	PlatformApi.WriteBarrier = SDLCompilerBarrier;

	PlatformApi.AtomicCAS_I32 = SDLAtomicCAS_I32;
	PlatformApi.AtomicCAS_U32 = SDLAtomicCAS_U32;
	PlatformApi.AtomicCAS_I64 = SDLAtomicCAS_I64;
	PlatformApi.AtomicCAS_U64 = SDLAtomicCAS_U64;

	PlatformApi.AtomicInc_I32 = SDLAtomicInc_I32;
	PlatformApi.AtomicInc_U32 = SDLAtomicInc_U32;
	PlatformApi.AtomicInc_I64 = SDLAtomicInc_I64;
	PlatformApi.AtomicInc_U64 = SDLAtomicInc_U64;

	PlatformApi.AtomicAdd_I32 = SDLAtomicAdd_I32;
	PlatformApi.AtomicAdd_U32 = SDLAtomicAdd_U32;
	PlatformApi.AtomicAdd_I64 = SDLAtomicAdd_I64;
	PlatformApi.AtomicAdd_U64 = SDLAtomicAdd_U64;

	PlatformApi.AtomicSet_I32 = SDLAtomicSet_I32;
	PlatformApi.AtomicSet_U32 = SDLAtomicSet_U32;
	PlatformApi.AtomicSet_I64 = SDLAtomicSet_I64;
	PlatformApi.AtomicSet_U64 = SDLAtomicSet_U64;
#endif

	PlatformApi.VoxelQueue = &VoxelQueue;
	PlatformApi.HighPriorityQueue = &HighPriorityQueue;
	PlatformApi.LowPriorityQueue = &LowPriorityQueue;
	PlatformApi.ReadFile = SDLReadEntireFile;
	PlatformApi.WriteFile = SDLWriteEntireFile;
	PlatformApi.FreeFileMemory = SDLFreeFileMemory;
	PlatformApi.PlaceCursorAtCenter = SDLPlaceCursorAtCenter;
	PlatformApi.TerminateProgram = SDLTerminateProgram;

	//NOTE(dima): Allocation of alloc queue entries
	int PlatformAllocEntriesCount = 1 << 14;
	int MemoryForAllocEntriesRequired = PlatformAllocEntriesCount * sizeof(dealloc_queue_entry);
	int AllocQueueEntryIndex = 0;
	dealloc_queue_entry* Entries = (dealloc_queue_entry*)malloc(MemoryForAllocEntriesRequired);

	PlatformApi.FirstUseAllocQueueEntry = Entries + AllocQueueEntryIndex++;
	PlatformApi.FirstUseAllocQueueEntry->Next = PlatformApi.FirstUseAllocQueueEntry;
	PlatformApi.FirstUseAllocQueueEntry->Prev = PlatformApi.FirstUseAllocQueueEntry;

	PlatformApi.FirstFreeAllocQueueEntry = Entries + AllocQueueEntryIndex++;
	PlatformApi.FirstFreeAllocQueueEntry->Next = PlatformApi.FirstFreeAllocQueueEntry;
	PlatformApi.FirstFreeAllocQueueEntry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

	for (AllocQueueEntryIndex;
		AllocQueueEntryIndex < PlatformAllocEntriesCount;
		AllocQueueEntryIndex++)
	{
		dealloc_queue_entry* Entry = Entries + AllocQueueEntryIndex;

		Entry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		Entry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

		Entry->Next->Prev = Entry;
		Entry->Prev->Next = Entry;

		Entry->EntryType = 0;
		Entry->Data = {};
	}

	//NOTE(dima): Initializing of debug layer global record table
	DEBUGSetRecording(1);
	DEBUGSetLogRecording(1);

	for (int DebugLogIndex = 0;
		DebugLogIndex < DEBUG_LOGS_COUNT;
		DebugLogIndex++)
	{
		GlobalRecordTable->LogsInited[DebugLogIndex] = 0;
		GlobalRecordTable->LogsTypes[DebugLogIndex] = 0;
	}

	//NOTE(dima): Initialization of the memory
	u32 GameModeMemorySize = MEGABYTES(128);
	u32 GeneralPurposeMemorySize = MEGABYTES(512);
	u32 DEBUGMemorySize = MEGABYTES(128);

	u64 TotalMemoryBlockSize =
		GameModeMemorySize +
		GeneralPurposeMemorySize +
		DEBUGMemorySize;

	void* PlatformMemoryBlock = calloc(TotalMemoryBlockSize, 1);

	void* GameModeMemPointer = PlatformMemoryBlock;
	void* GeneralPurposeMemPointer = (u8*)GameModeMemPointer + GameModeMemorySize;
	void* DEBUGMemPointer = (u8*)GeneralPurposeMemPointer + GeneralPurposeMemorySize;

	PlatformApi.GameModeMemoryBlock = InitStackedMemory(PlatformMemoryBlock, GameModeMemorySize);
	PlatformApi.GeneralPurposeMemoryBlock = InitStackedMemory(GeneralPurposeMemPointer, GeneralPurposeMemorySize);
	PlatformApi.DEBUGMemoryBlock = InitStackedMemory(DEBUGMemPointer, DEBUGMemorySize);

#define GORE_WINDOW_WIDTH 1366
#define GORE_WINDOW_HEIGHT 768

	GlobalBuffer = AllocateRGBABuffer(GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);
	GlobalPerfomanceCounterFrequency = SDL_GetPerformanceFrequency();
	GlobalTime = 0.0f;

	if (SdlInitCode < 0) {
		printf("ERROR: SDL has been not initialized");
	}

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

	int SetR, SetG, SetB, SetD;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &SetR);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &SetG);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &SetB);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &SetD);

	char WindowBitsBuf[64];

	stbsp_sprintf(WindowBitsBuf, "Window RED bits count: %d", SetR);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window GREEN bits count: %d", SetG);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window BLUE bits count: %d", SetB);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window DEPTH bits count: %d", SetD);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

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
	_glActiveTexture = (MYPFNGLACTIVETEXTURE)SDL_GL_GetProcAddress("glActiveTexture");

	if (!Window) {
		printf("ERROR: Window is not created");
	}
	DEBUG_OK_LOG("Window successfully created");

	SDLSetWindowIcon(Window);

	SDL_Renderer* renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("ERROR: Renderer is not created");
	}

	ASSETSInit(&GlobalAssets, MEGABYTES(32));

	voxel_atlas_id VoxelAtlasID = GetFirstVoxelAtlas(&GlobalAssets, GameAsset_MyVoxelAtlas);
	voxel_atlas_info* VoxelAtlas = GetVoxelAtlasFromID(&GlobalAssets, VoxelAtlasID);
	random_state CellRandom = InitRandomStateWithSeed(1234);
	cellural_buffer Cellural = AllocateCelluralBuffer(127, 127);
	//CelluralGenerateCave(&Cellural, 55, &CellRandom);
	CelluralGenerateLaby(&Cellural, &CellRandom);
	//CelluralGenerateSquadLaby(&Cellural, &CellRandom);
	bitmap_info CaveBitmap = CelluralBufferToBitmap(&Cellural);
	mesh_info CaveMesh = CelluralBufferToMesh(&Cellural, 3, VoxelAtlas);

	//bitmap_info Image = LoadIMG("../Data/Images/image.bmp");
	//bitmap_info AlphaImage = LoadIMG("../Data/Images/alpha.png");
	//bitmap_info PotImage = LoadIMG("../Data/Images/pot.png");

	//font_info FontInfo = LoadFontInfoFromImage("../Data/Fonts/NewFontAtlas.png", 15, 8, 8, 0);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Bold.ttf", 18, AssetLoadFontFlag_BakeOffsetShadows);

	//font_info FontInfo = LoadFontInfoFromImage("../Data/Fonts/geebeeyay_copy.png", 15, 8, 8, AssetLoadFontFromImage_InitLowercaseWithUppercase);
	//font_info FontInfo = LoadFontInfoFromImage("../Data/Fonts/geebeeyay_8x16.png", 20, 8, 16);
	//font_info FontInfo = LoadFontInfoFromImage("../Data/Fonts/bubblemad_8x8.png", 15, 8, 8);
	//font_info FontInfo = LoadFontInfoFromImage("../Data/Fonts/geebeeyay-8x8.png", 15, 8, 8);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/Boxy-Bold.ttf", 20);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/typoster.outline.otf", 20);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Regular.ttf", 20);
	//font_info FontInfo = LoadFontInfoWithSTB("../Data/Fonts/arial.ttf", 18);

	geometrika_state GameState = {};

	stacked_memory RENDERMemory = SplitStackedMemory(&PlatformApi.GeneralPurposeMemoryBlock, MEGABYTES(5));
	stacked_memory GUIMemory = SplitStackedMemory(&PlatformApi.GeneralPurposeMemoryBlock, MEGABYTES(1));
	stacked_memory ColorsMemory = SplitStackedMemory(&PlatformApi.GeneralPurposeMemoryBlock, KILOBYTES(20));
	stacked_memory VoxelMemory = SplitStackedMemory(&PlatformApi.GeneralPurposeMemoryBlock, MEGABYTES(256));

	font_id GUIFontID = GetFirstFont(&GlobalAssets, GameAsset_Font);
	font_info* GUIFont = GetFontFromID(&GlobalAssets, GUIFontID);

	voxworld_generation_state VoxelGeneration;
	VoxelChunksGenerationInit(&VoxelGeneration, &VoxelMemory, 15);
	InitColorsState(ColorState, &ColorsMemory);
	GUIInitState(GUIState, &GUIMemory, ColorState, GUIFont, &GlobalInput, GlobalBuffer.Width, GlobalBuffer.Height);

	OpenGLInitState(GLState);
	DEBUGInit(DEBUGState, &PlatformApi.DEBUGMemoryBlock, GUIState);

	float TempFloatForSlider = 4.0f;
	float TempFloatForVertSlider = 0.0f;
	b32 TempBoolForSlider = false;

	float DeltaTime = 0.0f;

	GlobalRunning = true;
	while (GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		DEBUG_FRAME_BARRIER(DeltaTime);

		BEGIN_TIMING(DEBUG_FRAME_UPDATE_NODE_NAME);
		BEGIN_SECTION("Platform");

		DEBUG_STACKED_MEM("Platform GameMode Memory", &PlatformApi.GameModeMemoryBlock);
		DEBUG_STACKED_MEM("Platform GeneralPurpose Memory", &PlatformApi.GeneralPurposeMemoryBlock);
		DEBUG_STACKED_MEM("Platform DEBUG Memory", &PlatformApi.DEBUGMemoryBlock);

		DEBUG_STACKED_MEM("GUIMem", &GUIMemory);
		DEBUG_STACKED_MEM("ColorsMem", &ColorsMemory);

		BEGIN_TIMING("Input processing");
		SDLProcessEvents(Window, &GlobalInput);

		SDLProcessInput(&GlobalInput);

		if (ButtonWentDown(&GlobalInput, KeyType_Esc)) {
			GlobalRunning = false;
			break;
		}

		if (ButtonWentDown(&GlobalInput, KeyType_F12)) {
			SDLGoFullscreen(Window);
		}
		END_TIMING();

		BEGIN_REPEATED_TIMING("Other...");
		render_state Stack_ = RENDERBeginStack(&RENDERMemory, GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT, &GlobalAssets);
		render_state* Stack = &Stack_;

		RENDERPushClear(Stack, V3(0.3f, 0.3f, 0.3f));
		RENDERPushTest(Stack);

#if 0

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
		//if (TempBoolForSlider) {
		//	RENDERPushBitmap(Stack, &CelluralBitmap, V2(0, 0), CelluralBitmap.Height);
		//}

		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX1, 400), 300.0f);
		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX2, 600), 300.0f);
		RENDERPushBitmap(Stack, &PotImage, V2(AlphaImageX3, 200), 300.0f);
		//
		//RENDERPushRect(Stack, V2(AlphaImageX1, 400), V2(100, 100), V4(1.0f, 1.0f, 1.0f, 0.5f));
#endif
		VoxelChunksGenerationUpdate(&VoxelGeneration, Stack, GameState.Camera.Position);
		GEOMKAUpdateAndRender(&GameState, &GlobalAssets, Stack, &GlobalInput);

		GUIBeginFrame(GUIState, Stack);
		END_TIMING();

		BEGIN_TIMING("Debug update");
		BEGIN_SECTION("Profile");
		DEBUG_VALUE(DebugValue_FramesSlider);
		DEBUG_VALUE(DebugValue_ViewFrameInfo);
		DEBUG_VALUE(DebugValue_ProfileOverlays);
		END_SECTION();

		DEBUGUpdate(DEBUGState);
		END_TIMING();

		BEGIN_REPEATED_TIMING("Other...");
		GUIPrepareFrame(GUIState);
		END_TIMING();

#if 1
		BEGIN_TIMING("Rendering");
		glViewport(0, 0, GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);

		OpenGLProcessAllocationQueue();
		OpenGLRenderStackToOutput(GLState, Stack);
		END_TIMING();

		BEGIN_TIMING("Swapping");
		SDL_GL_SwapWindow(Window);
		END_TIMING();
#else

		RenderMultithreaded(&HighPriorityQueue, Stack, &GlobalBuffer);

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
#endif

		BEGIN_REPEATED_TIMING("Other...");
		GUIEndFrame(GUIState);
		RENDEREndStack(Stack);
		END_TIMING();

		END_SECTION();
		END_TIMING();

		DeltaTime = SDLGetMSElapsed(FrameBeginClocks);
		GlobalInput.DeltaTime = DeltaTime;
		GlobalInput.Time = GlobalTime;

		GlobalTime += DeltaTime;
	}

	SDL_GL_DeleteContext(SDLOpenGLRenderContext);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	DeallocateRGBABuffer(&GlobalBuffer);

	printf("Program has been succesfully ended\n");

	return(0);
}