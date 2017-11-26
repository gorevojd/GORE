#ifndef GAME_LAYER_H
#define GAME_LAYER_H

#define GLOBAL_VARIABLE static

#define Assert(cond) if(!(cond)){ *((int*)0) = 0;}
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef int b32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifndef Min
#define Min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef Max
#define Max(a, b) ((a) > (b) ? (a) : (b))
#endif

#include <intrin.h>
#include "workout_math.h"

struct data_buffer {
	u8* Data;
	u64 Size;
};

struct rgba_buffer {
	u8* Pixels;

	u32 Width;
	u32 Height;
	v2 Align;

	u32 Pitch;
};

extern data_buffer ReadFileToDataBuffer(char* FileName);
extern void FreeDataBuffer(data_buffer* DataBuffer);
extern rgba_buffer AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
extern void DeallocateRGBABuffer(rgba_buffer* Buffer);

enum button_type{
	InputButtonType_W,
	InputButtonType_A,
	InputButtonType_S,
	InputButtonType_D,

	InputButtonType_Q,
	InputButtonType_R,
	InputButtonType_E,

	InputButtonType_Space,
	InputButtonType_LCtrl,
	InputButtonType_LShift,
	InputButtonType_Esc,

	InputButtonType_Count,
};

typedef struct  button_state{
	b32 IsDown;
	b32 WasDown;
}button_state;

typedef struct input_system {
	button_state Buttons[InputButtonType_Count];
}input_system;

#endif