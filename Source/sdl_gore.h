#ifndef SDL_GORE_H_INCLUDED
#define SDL_GORE_H_INCLUDED

#include <SDL.h>
#include <stdio.h>

#include "gore_render_state.h"
#include "gore_renderer.h"
#include "gore_asset_common.h"
#include "gore_engine.h"

#include "gore_opengl.h"

#if defined(PLATFORM_WINDA)
#include <Windows.h>
#endif

PFNGLGETSTRINGIPROC glGetStringi;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLTEXBUFFERPROC glTexBuffer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
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

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

PFNGLCLEARBUFFERIVPROC glClearBufferiv;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
PFNGLCLEARBUFFERFIPROC glClearBufferfi;

MYPFNGLDRAWELEMENTSPROC _glDrawElements;
MYPFNGLACTIVETEXTURE _glActiveTexture;

#if defined(PLATFORM_WINDA)

struct winda_state {
	stacked_memory PlatformMemStack;
	void* PlatformLayerMemory;
	u32 PlatformLayerMemorySize;

	platform_system_info* SystemInfo;

	void* EngineLayerMemory;
	u32 EngineLayerMemorySize;

	stacked_memory GameModeStack;
	stacked_memory EngineSystemsStack;

	u32 WindowWidth;
	u32 WindowHeight;

	b32 GlobalRunning;
	input_system GlobalInput;
	u64 GlobalPerfomanceCounterFrequency;
	float GlobalTime;
};

struct platform_thread_queue {
	platform_threadwork* Entries;
	u32 EntriesCount;

	volatile unsigned int AddIndex;
	volatile unsigned int DoIndex;

	volatile unsigned int StartedEntries;
	volatile unsigned int FinishedEntries;

	u32 WorkingThreadsCount;

	HANDLE Semaphore;

	platform_mutex AddMutex;

	char* QueueName;
};

struct winda_thread_worker {
	platform_thread_queue* Queue;
	HANDLE ThreadHandle;
	u32 ThreadID;
};

#else

struct platform_thread_queue {
	platform_threadwork* Entries;
	u32 EntriesCount;

	SDL_atomic_t AddIndex;
	SDL_atomic_t DoIndex;

	SDL_atomic_t StartedEntries;
	SDL_atomic_t FinishedEntries;

	u32 WorkingThreadsCount;

	SDL_sem* Semaphore;

	platform_mutex AddMutex;

	char* QueueName;
};

struct sdl_thread_worker {
	platform_thread_queue* Queue;
	SDL_Thread* ThreadHandle;
	u32 ThreadID;
};

#endif

#endif