#include "sdl_gore.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include "gore_game_mode.h"

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

//NOTE(dima): Global variables
static winda_state WindaState;

platform_api PlatformApi;

#if GORE_DEBUG_ENABLED
debug_record_table *GlobalRecordTable;
#endif

static u64 SDLGetClocks() {
	u64 Result = SDL_GetPerformanceCounter();

	return(Result);
}

static float SDLGetMSElapsed(u64 BeginClocks) {
	float Result;

	u64 ClocksElapsed = SDLGetClocks() - BeginClocks;
	Result = (float)ClocksElapsed / WindaState.GlobalPerfomanceCounterFrequency;

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
							WindaState.GlobalRunning = false;
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
						WindaState.GlobalRunning = false;
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

static gl_state GLState_;
static gl_state* GLState = &GLState_;

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
	bitmap_info Image = AssetLoadIMG("../Data/Images/pot32.png");
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
	FileReadResult->Data = 0;
}

PLATFORM_PLACE_CURSOR_AT_CENTER(SDLPlaceCursorAtCenter) {
#if 0
	SDL_DisplayMode DisplayMode;
	SDL_GetDesktopDisplayMode(0, &DisplayMode);

	SDL_WarpMouseGlobal(DisplayMode.w >> 1, DisplayMode.h >> 1);
#else
	SDL_Window* Window = SDL_GL_GetCurrentWindow();

	SDL_WarpMouseInWindow(
		Window, 
		WindaState.GlobalInput.CenterP.x, 
		WindaState.GlobalInput.CenterP.y);
#endif
}

PLATFORM_TERMINATE_PROGRAM(SDLTerminateProgram) {
	WindaState.GlobalRunning = 0;
}

PLATFORM_END_GAME_LOOP(SDLEndGameLoop) {
	WindaState.GlobalRunning = 0;
}

#if defined(PLATFORM_WINDA)

PLATFORM_ADD_THREADWORK_ENTRY(WindaAddThreadworkEntry) {
	BeginMutexAccess(&Queue->AddMutex);

	_ReadBarrier();
	int NewToSet = (Queue->AddIndex + 1) % Queue->EntriesCount;
	int EntryIndex = Queue->AddIndex;

	platform_threadwork* Entry = Queue->Entries + EntryIndex;

	Entry->Callback = Callback;
	Entry->Data = Data;

	InterlockedIncrement((volatile unsigned int*)&Queue->StartedEntries);

	_WriteBarrier();

	InterlockedExchange((volatile unsigned int*)&Queue->AddIndex, NewToSet);
	
	ReleaseSemaphore(Queue->Semaphore, 1, 0);

	EndMutexAccess(&Queue->AddMutex);
}

b32 WindaDoNextThreadwork(platform_thread_queue* Queue) {
	b32 NoWorkLeft = 0;

	_ReadBarrier();

	int OriginalToDoIndex = Queue->DoIndex;
	int NewToSet = (OriginalToDoIndex + 1) % Queue->EntriesCount;

	if (Queue->DoIndex != Queue->AddIndex) {
		if (InterlockedCompareExchange(
			(volatile unsigned int*)&Queue->DoIndex, 
			NewToSet, 
			OriginalToDoIndex) == OriginalToDoIndex) 
		{
			platform_threadwork* Work = Queue->Entries + OriginalToDoIndex;

			Work->Callback(Work->Data);

			_WriteBarrier();

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
	int WorkersCount, 
	char* QueueName,
	u32 EntriesCount) 
{
	Queue->AddIndex = 0;
	Queue->DoIndex = 0;
	Queue->FinishedEntries = 0;
	Queue->StartedEntries = 0;

	winda_thread_worker* Workers = PushArray(
		&WindaState.PlatformMemStack,
		winda_thread_worker,
		WorkersCount);

	Queue->Entries = PushArray(
		&WindaState.PlatformMemStack,
		platform_threadwork,
		EntriesCount);
	Queue->EntriesCount = EntriesCount;

	Queue->WorkingThreadsCount = WorkersCount;
	Queue->AddMutex = {};

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

PLATFORM_ALLOCATE_MEMORY(WindaAllocateMemory) {
#if 1
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	void* Result = malloc(Size);
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
#else
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	void* Result = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
#endif
	return(Result);
}

PLATFORM_DEALLOCATE_MEMORY(WindaDeallocateMemory) {
#if 1
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	if (Memory) {
		free(Memory);
	}
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
#else
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	if (Memory) {
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
#endif
}

PLATFORM_GET_TIME_FROM_TIME_HANDLE(WindaGetTimeFromTimeHandle) {
	platform_time Result = {};

	FILETIME WinFileTime;
	WinFileTime.dwLowDateTime = Time & 0xFFFFFFFF;
	WinFileTime.dwHighDateTime = (Time >> 32) & 0xFFFFFFFF;

	SYSTEMTIME SysTime;
	FileTimeToSystemTime(&WinFileTime, &SysTime);

	Result.Day = SysTime.wDay;
	Result.Month = SysTime.wMonth;
	Result.Year = SysTime.wYear;
	Result.Hour = SysTime.wHour;
	Result.Minute = SysTime.wMinute;
	Result.Second = SysTime.wSecond;
	Result.DayOfWeek = SysTime.wDayOfWeek;
	Result.Millisecond = SysTime.wMilliseconds;

	return(Result);
}

PLATFORM_OPEN_ALL_FILES_OF_TYPE_BEGIN(WindaOpenAllFilesOfTypeBegin) {
	platform_file_group Result = {};

	u32 FileGroupMemorySize = KILOBYTES(128);
	Result.FreeFileGroupMemory = VirtualAlloc(
		0, 
		FileGroupMemorySize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);

	stacked_memory Mem = InitStackedMemory(
		Result.FreeFileGroupMemory, 
		FileGroupMemorySize);

	//NOTE(dima): Replacing back slashes with forward slashes
	int FolderPathLen = StringLength(FolderPath);
	char* CorrectedFolderPath = PushArray(&Mem, char, FolderPathLen + 4);
	int ScanIndex;
	for (ScanIndex = 0;
		ScanIndex < FolderPathLen;
		ScanIndex++)
	{
		CorrectedFolderPath[ScanIndex] = FolderPath[ScanIndex];
		if (CorrectedFolderPath[ScanIndex] == '\\') {
			CorrectedFolderPath[ScanIndex] = '/';
		}
	}

	//NOTE(dima): If last char isn't slash then set it
	char LastFolderPathChar = FolderPath[ScanIndex - 1];
	if (LastFolderPathChar != '\\' &&
		LastFolderPathChar != '/') 
	{
		CorrectedFolderPath[ScanIndex++] = '/';
	}
	CorrectedFolderPath[ScanIndex++] = 0;

	char* FindPattern = PushArray(&Mem, char, MAX_PATH);
	CopyStrings(FindPattern, CorrectedFolderPath);

	//NOTE(dima): Adding corresponding wildcards
	char WildCard[32];
	switch (Type) {
		case FileType_Asset: {
			CopyStrings(WildCard, "*.gass");
		}break;

		case FileType_SavedGame: {
			CopyStrings(WildCard, "*.gsav");
		}break;
	}

	ConcatStringsUnsafe(FindPattern, FindPattern, WildCard);

	//NOTE(dima): Iterating through matching files
	WIN32_FIND_DATAA FileFindData;
	HANDLE FindHandle = FindFirstFileA(FindPattern, &FileFindData);

	char* FullFileRelativePath = PushArray(&Mem, char, MAX_PATH);

	b32 NextFileFound = 1;
	if (FindHandle != INVALID_HANDLE_VALUE) {
		while (NextFileFound) {
			platform_file_entry* File = PushStruct(&Mem, platform_file_entry);

			File->Next = Result.FirstFileEntry;
			File->FileSize = (FileFindData.nFileSizeLow) | (((u64)FileFindData.nFileSizeHigh) << 32);

			ConcatStringsUnsafe(FullFileRelativePath, CorrectedFolderPath, FileFindData.cFileName);
			File->FileName = PushString(&Mem, FullFileRelativePath);
			CopyStrings(File->FileName, FullFileRelativePath);

			HANDLE FileHandle = CreateFileA(
				File->FileName,
				GENERIC_READ,
				FILE_SHARE_READ,
				0,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0);

			Assert(FileHandle != INVALID_HANDLE_VALUE);

			File->PlatformFileHandle = (u64)FileHandle;
			File->PlatformLastWriteTime =
				(FileFindData.ftLastWriteTime.dwLowDateTime) |
				((u64)FileFindData.ftLastWriteTime.dwHighDateTime << 32);

			LARGE_INTEGER FileSizeLI;
			GetFileSizeEx(FileHandle, &FileSizeLI);

			File->FileSize = FileSizeLI.QuadPart; 
#if 0
			platform_time Time = WindaGetTimeFromTimeHandle(File->PlatformLastWriteTime);
#endif

			Result.FirstFileEntry = File;
			Result.FileCount++;

			NextFileFound = FindNextFileA(FindHandle, &FileFindData);
		}
	}
	FindClose(FindHandle);

	return(Result);
}

PLATFORM_OPEN_ALL_FILES_OF_TYPE_END(WindaOpenAllFilesOfTypeEnd) {
	platform_file_entry* FirstFileEntry = Group->FirstFileEntry;

	//NOTE(dima): Closing file handles
	platform_file_entry* At = Group->FirstFileEntry;
	for (At; At; At = At->Next) {
		CloseHandle((HANDLE)At->PlatformFileHandle);
	}

	//NOTE(dima): Freeing file group memory
	if (Group->FreeFileGroupMemory) {
		VirtualFree(Group->FreeFileGroupMemory, 0, MEM_RELEASE);
	}
}

PLATFORM_READ_DATA_FROM_FILE_ENTRY(WindaReadDataFromFileEntry) {
	OVERLAPPED Overlapped = {};
	Overlapped.Offset = StartOffset & 0xFFFFFFFF;
	Overlapped.OffsetHigh = (StartOffset >> 32) & 0xFFFFFFFF;

	DWORD BytesRead;

	HANDLE FileHandle = (HANDLE)File->PlatformFileHandle;
	BOOL ReadSuccess = ReadFile(
		FileHandle,
		Dest,
		(u32)BytesCountToRead,
		&BytesRead,
		&Overlapped);

	Assert(ReadSuccess);
	Assert(BytesRead == BytesCountToRead);
}

PLATFORM_GET_DISPLAY_DEVICE_COUNT(WindaGetDisplayDeviceCount) {
	DWORD DisplayDeviceIndex = 0;

	DISPLAY_DEVICEA Device = {};
	Device.cb = sizeof(DISPLAY_DEVICEA);
	while (EnumDisplayDevicesA(0, DisplayDeviceIndex, &Device, 0)) {
		DisplayDeviceIndex++;
	}

	return(DisplayDeviceIndex);
}

PLATFORM_TRY_GET_DISPLAY_DEVICE(WindaTryGetDisplayDevice) {
	b32 Result = 0;

	DISPLAY_DEVICEA WinDevice = {};
	WinDevice.cb = sizeof(DISPLAY_DEVICEA);
	if (EnumDisplayDevicesA(NULL, DeviceIndex, &WinDevice, 0)) {
		CopyStrings(Device->DeviceName, WinDevice.DeviceName);
		CopyStrings(Device->DeviceString, WinDevice.DeviceString);
		CopyStrings(Device->DeviceRegistryKey, WinDevice.DeviceKey);

		Result = 1;
	}

	return(Result);
}

PLATFORM_GET_DISPLAY_MODE_COUNT(WindaGetDisplayModeCount) {
	int ModeNum = 0;
	DEVMODE DevMode;
	DevMode.dmSize = sizeof(DEVMODE);

	/*
	NOTE(dima):

	A NULL value specifies the current display
	device on the computer on which the calling
	thread is running
	*/
	while (EnumDisplaySettings(NULL, ModeNum, &DevMode)) {
		ModeNum++;
	}

	return(ModeNum);
}

PLATFORM_TRY_GET_DISPLAY_MODE(WindaTryGetDisplayMode)
{
	b32 Result = 0;

	DEVMODEA Mode = {};
	Mode.dmSize = sizeof(DEVMODEA);

	/*
	NOTE(dima):

	A NULL value specifies the current display
	device on the computer on which the calling
	thread is running
	*/
	if (EnumDisplaySettingsA(0, ModeIndex, &Mode)) {
		DisplayMode->BitsPerPixel = Mode.dmBitsPerPel;
		DisplayMode->PixelHeight = Mode.dmPelsHeight;
		DisplayMode->PixelWidth = Mode.dmPelsWidth;
		DisplayMode->RefreshRate = Mode.dmDisplayFrequency;

		Result = 1;
	}

	return(Result);
}

PLATFORM_GET_PERFOMANCE_COUNTER(WindaGetPerfomanceCounter) {
	LARGE_INTEGER Counter = {};
	QueryPerformanceCounter(&Counter);

	u64 Result = Counter.QuadPart;

	return(Result);
}

PLATFORM_GET_PERFOMANCE_FREQUENCY(WindaGetPerfomanceFrequency) {
	LARGE_INTEGER Freq = {};
	QueryPerformanceFrequency(&Freq);

	u64 Result = Freq.QuadPart;

	return(Result);
}

#else

PLATFORM_ADD_THREADWORK_ENTRY(SDLAddThreadworkEntry) {

	BeginMutexAccess(&Queue->AddMutex);
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
	EndMutexAccess(&Queue->AddMutex);
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
	Queue->AddMutex = {};
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

PLATFORM_ALLOCATE_MEMORY(SDLAllocateMemory) {
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	void* Result = malloc(Size);
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);

	return(Result);
}

PLATFORM_DEALLOCATE_MEMORY(SDLDeallocateMemory) {
	BeginMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
	if (Memory) {
		free(Memory);
	}
	EndMutexAccess(&PlatformApi.NativeMemoryAllocatorMutex);
}

PLATFORM_GET_PERFOMANCE_COUNTER(SDLGetPerfomanceCounter) {
	u64 Result = SDL_GetPerformanceCounter();

	return(Result);
}

PLATFORM_GET_PERFOMANCE_FREQUENCY(SDLGetPerfomanceFrequency) {
	u64 Result = SDL_GetPerformanceFrequency();

	return(Result);
}
#endif


inline void WindaInitProcFeature(
	stacked_memory* MemStack,
	platform_processor_feature* Feature, 
	b32 IsSupported,
	char* Description)
{
	Feature->Enabled = IsSupported;
	
	if (Description) {
		Feature->Description = PushString(MemStack, Description);
		CopyStrings(Feature->Description, Description);
	}
	else {
		Feature->Description = PushString(MemStack, "(none)");
		CopyStrings(Feature->Description, "(none)");
	}
}

typedef BOOL (WINAPI *win32_get_logical_procinfo)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
typedef BOOL (WINAPI *win32_get_logical_procinfoex)(LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

static void WindaGetSystemInfo(platform_system_info* Result) {
	SYSTEM_INFO SysInfo = {};

	GetSystemInfo(&SysInfo);

	u32 ToSetPA = 0;
	char* DescPA = Result->ProcessorArchitectureDesc;
	switch(SysInfo.wProcessorArchitecture){
		case PROCESSOR_ARCHITECTURE_AMD64: {
			ToSetPA = ProcessorArchitecture_AMD64;
			CopyStrings(DescPA, "x64 (AMD or Intel)");
		}break;

		case PROCESSOR_ARCHITECTURE_ARM: {
			ToSetPA = ProcessorArchitecture_ARM;
			CopyStrings(DescPA, "ARM");
		}break;

		case PROCESSOR_ARCHITECTURE_MIPS: {
			ToSetPA = ProcessorArchitecture_MIPS;
			CopyStrings(DescPA, "MIPS");
		}break;

		case PROCESSOR_ARCHITECTURE_IA64: {
			ToSetPA = ProcessorArchitecture_IA64;
			CopyStrings(DescPA, "Intel itanium based");
		}break;

		case PROCESSOR_ARCHITECTURE_INTEL: {
			ToSetPA = ProcessorArchitecture_Intel;
			CopyStrings(DescPA, "x86");
		}break;

		case 12: {
			ToSetPA = ProcessorArchitecture_ARM64;
			CopyStrings(DescPA, "ARM64");
		}break;

		case PROCESSOR_ARCHITECTURE_MSIL: {
			ToSetPA = ProcessorArchitecture_MSIL;
			CopyStrings(DescPA, "MSIL");
		}break;

		case PROCESSOR_ARCHITECTURE_PPC: {
			ToSetPA = ProcessorArchitecture_PPC;
			CopyStrings(DescPA, "PPC");
		}break;

		case PROCESSOR_ARCHITECTURE_SHX: {
			ToSetPA = ProcessorArchitecture_SHX;
			CopyStrings(DescPA, "SHX");
		} break;

		case PROCESSOR_ARCHITECTURE_ALPHA: {
			ToSetPA = ProcessorArchitecture_Alpha;
			CopyStrings(DescPA, "ALPHA");
		}break;

		case PROCESSOR_ARCHITECTURE_ALPHA64: {
			ToSetPA = ProcessorArchitecture_Alpha64;
			CopyStrings(DescPA, "ALPHA64");
		}break;

		default: {
			ToSetPA = ProcessorArchitecture_Unknown;
			CopyStrings(DescPA, "Unknown");
		}break;
	}
	Result->ProcessorArchitecture = ToSetPA;

	Result->MinimumApplicationAddress = SysInfo.lpMinimumApplicationAddress;
	Result->MaximumApplicationAddress = SysInfo.lpMaximumApplicationAddress;
	Result->PageSize = SysInfo.dwPageSize;
	
	platform_processor_feature* Feature = 0;
	b32 IsSupported = 0;
	stacked_memory* TempMem = &WindaState.PlatformMemStack;

	//NOTE(dima): Clear all features to 0
	for (int ProcFeatureIndex = 0;
		ProcFeatureIndex < ProcFeature_Count;
		ProcFeatureIndex++)
	{
		platform_processor_feature* Feature = &Result->ProcessorFeatures[ProcFeatureIndex];

		WindaInitProcFeature(
			TempMem,
			Feature,
			0,
			"(NONE DESCRIPTION)");
	}

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_FloatingPointPrecisionERRATA],
		IsProcessorFeaturePresent(PF_FLOATING_POINT_PRECISION_ERRATA),
		"On a Pentium, a floating-point precision error can occur in rare circumstances.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_FloatingPointEmulated],
		IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED),
		"Floating-point operations are emulated using a software emulator." \
		"This function returns a nonzero value if floating - point operations" \
		" are emulated; otherwise, it returns zero.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_CompareExchangeDouble],
		IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE),
		"The atomic compare and exchange operation (cmpxchg) is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_MMX],
		IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE),
		"The MMX instruction set is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_AlphaByte],
		IsProcessorFeaturePresent(PF_ALPHA_BYTE_INSTRUCTIONS),
		"The ALPHA BYTE instruction set is available.");

	//SSE
	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_XMMI],
		IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE),
		"The SSE instruction set is available");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_3DNOW],
		IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE),
		"The 3D-Now instruction set is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_RDTSC],
		IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE),
		"The RDTSC instruction is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_PAE],
		IsProcessorFeaturePresent(PF_PAE_ENABLED),
		"The processor is PAE-enabled.");

	//SSE2
	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_XMMI64],
		IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE),
		"The SSE2 instruction set is available. Windows 2000: This feature is not supported.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_SSE_DAZ],
		IsProcessorFeaturePresent(PF_SSE_DAZ_MODE_AVAILABLE),
		"SSE DAZ mode enabled.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_DataExecutionPrevention],
		IsProcessorFeaturePresent(PF_NX_ENABLED),
		"Data execution prevention is enabled.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_SSE3],
		IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE),
		"The SSE3 instruction set is available." \
		"Windows Server 2003 and Windows XP / 2000:" \
		"This feature is not supported.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_CompareExchange128],
		IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE128),
		"The atomic compare and exchange 128-bit operation (cmpxchg16b) is available. " \
		"Windows Server 2003 and Windows XP / 2000:  This feature is not supported.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_Compare64Exchange128],
		IsProcessorFeaturePresent(PF_COMPARE64_EXCHANGE128),
		"The atomic compare 64 and exchange 128-bit operation (cmp8xchg16) is available. " \
		"Windows Server 2003 and Windows XP / 2000:  This feature is not supported.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_Channels],
		IsProcessorFeaturePresent(PF_CHANNELS_ENABLED),
		"The processor channels are enabled.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_XSave],
		IsProcessorFeaturePresent(PF_XSAVE_ENABLED),
		"The processor implements the XSAVE and XRSTOR instructions.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_VFP_32_Registers],
		IsProcessorFeaturePresent(PF_ARM_VFP_32_REGISTERS_AVAILABLE),
		"The VFP/Neon: 32 x 64bit register bank is present.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_NEON],
		IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE),
		"ARM NEON Instructions available");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_SecondLevelAddressTranslation],
		IsProcessorFeaturePresent(PF_SECOND_LEVEL_ADDRESS_TRANSLATION),
		"Second Level Address Translation is supported by the hardware.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_VirtFirmvare],
		IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED),
		"Virtualization is enabled in the firmware.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_RDWRFSGSBASE],
		IsProcessorFeaturePresent(PF_RDWRFSGSBASE_AVAILABLE),
		"RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE instructions are available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_FastFail],
		IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE),
		"_fastfail() is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_DivideInstructions],
		IsProcessorFeaturePresent(PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE),
		"The divide instructions are available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_64BitLoadStore],
		IsProcessorFeaturePresent(PF_ARM_64BIT_LOADSTORE_ATOMIC),
		"The 64-bit load/store atomic instructions are available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_ExternalCache],
		IsProcessorFeaturePresent(PF_ARM_EXTERNAL_CACHE_AVAILABLE),
		"The external cache is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_ARM_FMACInstructions],
		IsProcessorFeaturePresent(PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE),
		"The floating-point multiply-accumulate instruction is available.");

	WindaInitProcFeature(
		TempMem,
		&Result->ProcessorFeatures[ProcFeature_RDRANDInstructions],
		IsProcessorFeaturePresent(PF_RDRAND_INSTRUCTION_AVAILABLE),
		"RDRAND Instruction available");

	/*
	WindaInitProcFeature(
		TempMem,
		&Result.ProcessorFeatures[ProcFeature_],
		IsProcessorFeaturePresent(),
		"");
	*/

	//GetLogicalProcessorInformation()

	HMODULE Kernel32Module = GetModuleHandleA("kernel32");

	win32_get_logical_procinfo GetLogicalProcInfo = (win32_get_logical_procinfo)GetProcAddress(
		Kernel32Module,
		"GetLogicalProcessorInformation");

	win32_get_logical_procinfoex* GetLogicalProInfoEx = (win32_get_logical_procinfoex*)GetProcAddress(
		Kernel32Module,
		"GetLogicalProcessorInformationEx");

	b32 ProcInfoSuccesfullyRetrieved = 0;
	if (GetLogicalProcInfo) {
		int SizeOfInfo = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		int TempBufferCount = 0;
		DWORD TempBufferSize = TempBufferCount * SizeOfInfo;
		int InitTempBufferSize = TempBufferSize;

		SYSTEM_LOGICAL_PROCESSOR_INFORMATION* TempBuffersBase = PushArray(
			&WindaState.PlatformMemStack,
			SYSTEM_LOGICAL_PROCESSOR_INFORMATION,
			TempBufferCount);

		int ResultInfosCount = 0;

		b32 Success = 0;
		while (!Success) {
			/*
				NOTE(dima):  When I first used to use this funciton
				I came up with Windows 998 ERROR_NOACCESS(Invalid 
				access to memory location) error. The reason of this 
				error was that alignment in memory allocation was not 
				working properly. For this function input buffer must
				be aligned by 4.
			*/
			Success = GetLogicalProcInfo(TempBuffersBase, &TempBufferSize);

			if (Success) {
				ResultInfosCount = TempBufferSize / SizeOfInfo;

				break;
			}
			else {
				DWORD ErrCode = GetLastError();

				if (ErrCode == ERROR_INSUFFICIENT_BUFFER) {
					int ToIncrementByteSize = (int)TempBufferSize - InitTempBufferSize;
					int TempBufferToIncrementCount = ToIncrementByteSize / SizeOfInfo;

					//NOTE(dima): Allocate array on top of previously defined array
					PushArray(
						&WindaState.PlatformMemStack,
						SYSTEM_LOGICAL_PROCESSOR_INFORMATION,
						TempBufferToIncrementCount);

					TempBufferSize = InitTempBufferSize + ToIncrementByteSize;
					InitTempBufferSize = TempBufferSize;

					continue;
				}
				else {
					Assert(!"Undefined behaviour");
				}
			}
		}


		u32 L1CacheCount = 0;
		u32 L2CacheCount = 0;
		u32 L3CacheCount = 0;

		u32 L1CacheTotalSize = 0;
		u32 L2CacheTotalSize = 0;
		u32 L3CacheTotalSize = 0;

		u32 L1CacheLineSize = 0;
		u32 L2CacheLineSize = 0;
		u32 L3CacheLineSize = 0;

		u32 CoresCount = 0;
		u32 LogicalCoresCount = 0;

		u32 InstructionCacheCount = 0;
		u32 DataCacheCount = 0;
		u32 UnifiedCacheCount = 0;

		for (int ProcInfoIndex = 0;
			ProcInfoIndex < ResultInfosCount;
			ProcInfoIndex++)
		{
			SYSTEM_LOGICAL_PROCESSOR_INFORMATION* ProcInfo = TempBuffersBase + ProcInfoIndex;

			switch (ProcInfo->Relationship) {
				case RelationProcessorCore: {
					/*
						The specified logical processors share a single
						processor core. The ProcessorCore member contains 
						additional information.
					*/

					u64 MaskToScan = ProcInfo->ProcessorMask;

					int SetBitsCount = 0;
					for (int ScanBitIndex = 0;
						ScanBitIndex < sizeof(MaskToScan) * 8;
						ScanBitIndex++)
					{
						b32 BitIsSet = (MaskToScan >> ScanBitIndex) & 1;

						SetBitsCount += BitIsSet;
					}

					CoresCount++;
					LogicalCoresCount += SetBitsCount;
				}break;

				case RelationNumaNode: {
					/*
						The specified logical processors are part of the 
						same NUMA node. The NumaNode member contains 
						additional information.
					*/


				}break;

				case RelationCache: {
					/*
						The specified logical processors share a 
						cache. The Cache member contains additional 
						information. Windows Server 2003:  This value 
						is not supported until Windows Server 2003 with 
						SP1 and Windows XP Professional x64 Edition.
					*/

					CACHE_DESCRIPTOR* Cache = &ProcInfo->Cache;
					if (Cache->Level == 1) {
						L1CacheCount++;
						L1CacheTotalSize += Cache->Size;
						L1CacheLineSize = Cache->LineSize;
					}
					else if (Cache->Level == 2) {
						L2CacheCount++;
						L2CacheTotalSize += Cache->Size;
						L2CacheLineSize = Cache->LineSize;
					}
					else if (Cache->Level == 3) {
						L3CacheCount++;
						L3CacheTotalSize += Cache->Size;
						L3CacheLineSize = Cache->LineSize;
					}

					switch (Cache->Type) {
						case CacheData: {
							DataCacheCount++;
						}break;

						case CacheInstruction: {
							InstructionCacheCount++;
						}break;

						case CacheUnified: {
							UnifiedCacheCount++;
						}break;
					}

				}break;

				case RelationProcessorPackage: {
					/*
						The specified logical processors share a physical 
						package. There is no additional information available.
						Windows Server 2003 and Windows XP Professional x64 
						Edition:  This value is not supported until Windows Server 
						2003 with SP1 and Windows XP with SP3.
					*/
				}break;
			}
		}

		Result->CoresCount = CoresCount;
		Result->LogicalCoresCount = LogicalCoresCount;

		Result->L1CacheCount = L1CacheCount;
		Result->L2CacheCount = L2CacheCount;
		Result->L3CacheCount = L3CacheCount;

		Result->L1CacheTotalSize = L1CacheTotalSize;
		Result->L2CacheTotalSize = L2CacheTotalSize;
		Result->L3CacheTotalSize = L3CacheTotalSize;

		Result->L1CacheLineSize = L1CacheLineSize;
		Result->L2CacheLineSize = L2CacheLineSize;
		Result->L3CacheLineSize = L3CacheLineSize;

		Result->InstructionCachesCount = InstructionCacheCount;
		Result->DataCachesCount = DataCacheCount;
		Result->UnifiedCachesCount = UnifiedCacheCount;

		ProcInfoSuccesfullyRetrieved = 1;
	}

	Result->ProcInfoSuccesfullyRetrieved = ProcInfoSuccesfullyRetrieved;
}

void WindaInitState(winda_state* State) {
	//NOTE(dima): Allocating platform layer memory
#if GORE_DEBUG_ENABLED
	WindaState.PlatformLayerMemorySize = MEGABYTES(10) + sizeof(debug_record_table);
#else
	WindaState.PlatformLayerMemorySize = MEGABYTES(10);
#endif
	WindaState.PlatformLayerMemory = VirtualAlloc(
		0,
		WindaState.PlatformLayerMemorySize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);

	WindaState.PlatformMemStack = InitStackedMemory(
		WindaState.PlatformLayerMemory,
		WindaState.PlatformLayerMemorySize);

	//NOTE(dima): Initialization of the memory
	u32 GameModeMemorySize = MEGABYTES(256);
	u32 PermanentMemorySize = MEGABYTES(512);
	State->EngineLayerMemorySize =
		GameModeMemorySize +
		PermanentMemorySize;

	State->EngineLayerMemory = VirtualAlloc(
		0,
		State->EngineLayerMemorySize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);

	void* GameModeMemPointer = State->EngineLayerMemory;
	void* EngineSystemsMemPointer = (u8*)State->EngineLayerMemory + GameModeMemorySize;

	State->GameModeStack = InitStackedMemory(GameModeMemPointer, GameModeMemorySize);
	State->EngineSystemsStack = InitStackedMemory(EngineSystemsMemPointer, PermanentMemorySize);

	PlatformApi.GameModeMemoryBlock = State->GameModeStack;
	PlatformApi.EngineSystemsMemoryBlock = State->EngineSystemsStack;

	//NOTE(dima): Initializing of threads
	platform_thread_queue* SuperHighPriorityQueue = PushStruct(&WindaState.PlatformMemStack, platform_thread_queue);
	platform_thread_queue* HighPriorityQueue = PushStruct(&WindaState.PlatformMemStack, platform_thread_queue);
	platform_thread_queue* LowPriorityQueue = PushStruct(&WindaState.PlatformMemStack, platform_thread_queue);


#define PLATFORM_SUPERHIGH_QUEUE_THREADS_COUNT 5
	WindaInitThreadQueue(
		SuperHighPriorityQueue,
		PLATFORM_SUPERHIGH_QUEUE_THREADS_COUNT,
		"SuperHighQueue",
		16384);

#define PLATFORM_HIGH_QUEUE_THREADS_COUNT 4
	WindaInitThreadQueue(
		HighPriorityQueue,
		PLATFORM_HIGH_QUEUE_THREADS_COUNT,
		"HighQueue",
		4096);

#define PLATFORM_LOW_QUEUE_THREADS_COUNT 2
	WindaInitThreadQueue(
		LowPriorityQueue,
		PLATFORM_LOW_QUEUE_THREADS_COUNT,
		"LowQueue",
		4096);

	//NOTE(dima): Initializing of Platform API
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

	PlatformApi.NativeMemoryAllocatorMutex = {};
	PlatformApi.AllocateMemory = WindaAllocateMemory;
	PlatformApi.DeallocateMemory = WindaDeallocateMemory;

	PlatformApi.OpenAllFilesOfTypeBegin = WindaOpenAllFilesOfTypeBegin;
	PlatformApi.OpenAllFilesOfTypeEnd = WindaOpenAllFilesOfTypeEnd;
	PlatformApi.ReadDataFromFileEntry = WindaReadDataFromFileEntry;
	PlatformApi.GetFileTimeFromTimeHandle = WindaGetTimeFromTimeHandle;

	PlatformApi.GetDisplayDeviceCount = WindaGetDisplayDeviceCount;
	PlatformApi.GetDisplayModeCount = WindaGetDisplayModeCount;
	PlatformApi.TryGetDisplayDevice = WindaTryGetDisplayDevice;
	PlatformApi.TryGetDisplayMode = WindaTryGetDisplayMode;

	PlatformApi.SuperHighQueue = SuperHighPriorityQueue;
	PlatformApi.HighPriorityQueue = HighPriorityQueue;
	PlatformApi.LowPriorityQueue = LowPriorityQueue;
	PlatformApi.ReadFile = SDLReadEntireFile;
	PlatformApi.WriteFile = SDLWriteEntireFile;
	PlatformApi.FreeFileMemory = SDLFreeFileMemory;
	PlatformApi.PlaceCursorAtCenter = SDLPlaceCursorAtCenter;
	PlatformApi.TerminateProgram = SDLTerminateProgram;
	PlatformApi.EndGameLoop = SDLEndGameLoop;

	PlatformApi.GetPerfomanceCounter = WindaGetPerfomanceCounter;
	PlatformApi.GetPerfomanceFrequency = WindaGetPerfomanceFrequency;

	//NOTE(dima): Allocation of alloc queue entries
	int PlatformAllocEntriesCount = 1 << 12;
	int MemoryForAllocEntriesRequired = PlatformAllocEntriesCount * sizeof(alloc_queue_entry);
	int AllocQueueEntryIndex = 0;

	alloc_queue_entry* AllocQueueEntries = PushArray(
		&WindaState.PlatformMemStack,
		alloc_queue_entry,
		PlatformAllocEntriesCount);

	PlatformApi.FirstUseAllocQueueEntry = AllocQueueEntries + AllocQueueEntryIndex++;
	PlatformApi.FirstUseAllocQueueEntry->Next = PlatformApi.FirstUseAllocQueueEntry;
	PlatformApi.FirstUseAllocQueueEntry->Prev = PlatformApi.FirstUseAllocQueueEntry;

	PlatformApi.FirstFreeAllocQueueEntry = AllocQueueEntries + AllocQueueEntryIndex++;
	PlatformApi.FirstFreeAllocQueueEntry->Next = PlatformApi.FirstFreeAllocQueueEntry;
	PlatformApi.FirstFreeAllocQueueEntry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

	for (AllocQueueEntryIndex;
		AllocQueueEntryIndex < PlatformAllocEntriesCount;
		AllocQueueEntryIndex++)
	{
		alloc_queue_entry* Entry = AllocQueueEntries + AllocQueueEntryIndex;

		Entry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		Entry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

		Entry->Next->Prev = Entry;
		Entry->Prev->Next = Entry;

		Entry->EntryType = 0;
		Entry->Data = {};
	}

	//NOTE(dima): Initializing debug table if needed
#if GORE_DEBUG_ENABLED
	GlobalRecordTable = PushStruct(&WindaState.PlatformMemStack, debug_record_table);
	memset(GlobalRecordTable, 0, sizeof(debug_record_table));

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
#endif

	//NOTE(dima): INitialization of System info
	State->SystemInfo = PushStruct(&State->PlatformMemStack, platform_system_info);
	WindaGetSystemInfo(State->SystemInfo);

	PlatformApi.SystemInfo = State->SystemInfo;
}

void WindaFreeState(winda_state* State) {
	//NOTE(dima): Freing platform layer memory
	if (State->PlatformLayerMemory) {
		VirtualFree(State->PlatformLayerMemory, 0, MEM_RELEASE);
	}

	//NOTE(dima); Freeing engine layer memory
	if (State->EngineLayerMemory) {
		VirtualFree(State->EngineLayerMemory, 0, MEM_RELEASE);
	}
}

int main(int ArgsCount, char** Args) {

	WindaInitState(&WindaState);

	int SdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

#if 0
	int DisplayModeCount = SDL_GetNumDisplayModes(0);

	for (int DisplayModeIndex = 0;
		DisplayModeIndex < DisplayModeCount;
		DisplayModeIndex++)
	{
		SDL_DisplayMode DisplayMode;
		if (SDL_GetDisplayMode(0, DisplayModeIndex, &DisplayMode)) {
			SDL_Log("SDL_GetDisplayModef failed: %s", SDL_GetError());
		}

		u32 Format = DisplayMode.format;
		u32 Width = DisplayMode.w;
		u32 Height = DisplayMode.h;
		u32 RefreshRate = DisplayMode.refresh_rate;
		
		SDL_Log("Mode index %i \t %ibpp, %s, %ux%u, %uhz",
			DisplayModeIndex,
			SDL_BITSPERPIXEL(Format),
			SDL_GetPixelFormatName(Format),
			Width, Height,
			RefreshRate);
	}
#else
	int DisplayModeCount = WindaGetDisplayModeCount();

	for (int DisplayModeIndex = 0;
		DisplayModeIndex < DisplayModeCount;
		DisplayModeIndex++)
	{
		platform_display_mode DisplayMode;
		if (WindaTryGetDisplayMode(DisplayModeIndex, &DisplayMode) == 0) {
			SDL_Log("WindaGetDisplayMode failed: %s", SDL_GetError());
		}

		u32 Width = DisplayMode.PixelWidth;
		u32 Height = DisplayMode.PixelHeight;
		u32 RefreshRate = DisplayMode.RefreshRate;
		u32 BitsPerPixel = DisplayMode.BitsPerPixel;

		SDL_Log("Mode index %u \t %ibpp, %ux%u, %uhz",
			DisplayModeIndex,
			BitsPerPixel,
			Width, Height,
			RefreshRate);
	}
#endif



	//NOTE(dima): Loading game settings from settings file
	game_settings GameSettings = {};
	TryReadGameSettings(&GameSettings);

#define GORE_WINDOW_WIDTH 1366
#define GORE_WINDOW_HEIGHT 768

	//TODO(dima): Load window width and height from settings
	WindaState.WindowWidth = GORE_WINDOW_WIDTH;
	WindaState.WindowHeight = GORE_WINDOW_HEIGHT;

	WindaState.GlobalInput.WindowDim.x = WindaState.WindowWidth;
	WindaState.GlobalInput.WindowDim.y = WindaState.WindowHeight;

	WindaState.GlobalPerfomanceCounterFrequency = SDL_GetPerformanceFrequency();
	WindaState.GlobalTime = 0.0f;

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
		WindaState.WindowWidth,
		WindaState.WindowHeight,
		SDL_WINDOW_OPENGL);

	SDL_GLContext SDLOpenGLRenderContext = SDL_GL_CreateContext(Window);
	SDL_GL_MakeCurrent(Window, SDLOpenGLRenderContext);

	int SwapInterval = 0;
	if (GameSettings.Named.VSyncEnabledSetting->BoolValue) {
		SwapInterval = 1;
	}
	SDL_GL_SetSwapInterval(SwapInterval);

	int SetR, SetG, SetB, SetD;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &SetR);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &SetG);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &SetB);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &SetD);

	char WindowBitsBuf[64];

	stbsp_sprintf(WindowBitsBuf, "Window RED bits count: %d\n", SetR);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window GREEN bits count: %d\n", SetG);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window BLUE bits count: %d\n", SetB);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	stbsp_sprintf(WindowBitsBuf, "Window DEPTH bits count: %d\n", SetD);
	DEBUG_LOG(WindowBitsBuf);
	printf(WindowBitsBuf);

	glGetStringi = (PFNGLGETSTRINGIPROC)SDL_GL_GetProcAddress("glGetStringi");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)SDL_GL_GetProcAddress("glGenerateMipmap");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
	glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
	glMapBuffer = (PFNGLMAPBUFFERPROC)SDL_GL_GetProcAddress("glMapBuffer");
	glTexBuffer = (PFNGLTEXBUFFERPROC)SDL_GL_GetProcAddress("glTexBuffer");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glGetAttribLocation");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribIPointer");
	glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribLPointer");
	glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
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

	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebuffer");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
	glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)SDL_GL_GetProcAddress("glTexImage2DMultisample");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageMultisample");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)SDL_GL_GetProcAddress("glClearBufferiv");
	glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)SDL_GL_GetProcAddress("glClearBufferuiv");
	glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)SDL_GL_GetProcAddress("glClearBufferfv");
	glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)SDL_GL_GetProcAddress("glClearBufferfi");

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

#if 0
	int FloatsPerRow = 8;
	
	int NumOfFloatsToGen = 2048;

	FILE* fp = fopen("RandomFloats.txt", "w");

	if (fp) {

		float* RandomFloatArray = (float*)malloc(NumOfFloatsToGen * sizeof(float));

		int CurrentRowFillCount = 0;
		for (int RandomFloatIndex = 0;
			RandomFloatIndex < NumOfFloatsToGen;
			RandomFloatIndex++)
		{
			RandomFloatArray[RandomFloatIndex] = RandomBetween11(&CellRandom);

			char ThisFloatString[64];

			stbsp_sprintf(ThisFloatString, "%.5f, ", RandomFloatArray[RandomFloatIndex]);

			fputs(ThisFloatString, fp);
			CurrentRowFillCount++;

			if (CurrentRowFillCount >= FloatsPerRow) {
				fputc('\n', fp);

				CurrentRowFillCount = 0;
			}
		}

		free(RandomFloatArray);
		fclose(fp);
	}
#endif

	engine_systems* EngineSystems = EngineSystemsInit(
		&WindaState.GlobalInput, 
		&GameSettings);

	OpenGLInitState(
		GLState, 
		WindaState.WindowWidth, 
		WindaState.WindowHeight, 
		&GameSettings);

	float TempFloatForSlider = 4.0f;
	float TempFloatForVertSlider = 0.0f;
	b32 TempBoolForSlider = false;

	float DeltaTime = 0.0f;

	WindaState.GlobalRunning = true;
	while (WindaState.GlobalRunning) {
		u64 FrameBeginClocks = SDLGetClocks();

		DEBUG_FRAME_BARRIER(DeltaTime);

		BEGIN_TIMING(DEBUG_FRAME_UPDATE_NODE_NAME);
		BEGIN_SECTION("Platform");

		BEGIN_TIMING("Input processing");
		SDLProcessEvents(Window, &WindaState.GlobalInput);

		SDLProcessInput(&WindaState.GlobalInput);

		if (ButtonWentDown(&WindaState.GlobalInput, KeyType_Esc)) {
			WindaState.GlobalRunning = false;
			break;
		}

		if (ButtonWentDown(&WindaState.GlobalInput, KeyType_F12)) {
			SDLGoFullscreen(Window);
		}
		END_TIMING();

		GameModeUpdate(EngineSystems);

		BEGIN_TIMING("FramePreparing");
		END_TIMING();

#if 1
		BEGIN_TIMING("Rendering");
		glViewport(0, 0, GORE_WINDOW_WIDTH, GORE_WINDOW_HEIGHT);

		OpenGLRenderStateToOutput(GLState, EngineSystems->RenderState, &GameSettings);

		/*
			NOTE(dima): I think processing allocation queue 
			afetr rendering is initially good idea. I have 
			few reasons to say so.

			Imagine if we've implemented on-the-fly asset 
			streaming and our asset can  load and free on the fly.
			In that case we can end up with situation when
			we've pushed asset to the render queue and
			code goes on and at some point in time it would 
			be rendered. But in this interval of time it can 
			be freed and render code won't know anything about it!!!!
			Renderer would literally render the garbage because
			it pointing to the location where asset was stored
			previously but it was cleared and location is garbage now.

			So in that case when we process allocation queue after
			rendering we get rid of the possibility of this
			situation. Every asset will be freed only and only when
			it was rendered. And next render call or code that
			wants to use this asset will know about it...
			
		*/
		OpenGLProcessAllocationQueue();
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

		BEGIN_TIMING("FrameEnding");
		GameModeFinalizeFrame(EngineSystems);
		END_TIMING();

		END_SECTION();
		END_TIMING();

		DeltaTime = SDLGetMSElapsed(FrameBeginClocks);
		WindaState.GlobalInput.DeltaTime = DeltaTime;
		WindaState.GlobalInput.Time = WindaState.GlobalTime;

		WindaState.GlobalTime += DeltaTime;
	}

	SDL_GL_DeleteContext(SDLOpenGLRenderContext);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(Window);

	WindaFreeState(&WindaState);

	return(0);
}