#ifndef GAME_LAYER_H
#define GAME_LAYER_H

#include "workout_platform.h"

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
extern void CopyRGBABuffer(rgba_buffer* Dst, rgba_buffer* Src);
extern void DeallocateRGBABuffer(rgba_buffer* Buffer);

enum button_type{
	KeyType_W,
	KeyType_A,
	KeyType_S,
	KeyType_D,

	KeyType_Q,
	KeyType_R,
	KeyType_E,

	KeyType_Space,
	KeyType_LCtrl,
	KeyType_LShift,
	KeyType_Esc,
	KeyType_F1,
	KeyType_F2,
	KeyType_F3,
	KeyType_F4,
	KeyType_F5,
	KeyType_F6,
	KeyType_F7,
	KeyType_F8,
	KeyType_F9,
	KeyType_F10,
	KeyType_F11,
	KeyType_F12,

	KeyType_Count,
};

enum mouse_button_type {
	MouseButton_Left,
	MouseButton_Right,
	MouseButton_Middle,
	MouseButton_Extended1,
	MouseButton_Extended2,

	MouseButton_Count,
};

typedef struct  button_state{
	b32 IsDown;
	b32 TransitionHappened;

	b32 IsDoubleClick;
}button_state;

typedef struct input_system {
	button_state Buttons[KeyType_Count];
	button_state MouseButtons[MouseButton_Count];

	int MouseX;
	int MouseY;

	v2 MouseP;

	int GlobalMouseX;
	int GlobalMouseY;
}input_system;

inline b32 ButtonWentDown(input_system* Input, u32 KeyType) {

	b32 Result = 0;

	button_state* State = &Input->Buttons[KeyType];

	if (State->IsDown && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

inline b32 MouseButtonWentDown(input_system* Input, u32 MouseButton) {

	b32 Result = 0;

	button_state* State = &Input->MouseButtons[MouseButton];

	if (State->IsDown && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

inline b32 ButtonWentUp(input_system* Input, u32 KeyType) {
	b32 Result = 0;

	button_state* State = &Input->Buttons[KeyType];

	if (!State->IsDown && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

inline b32 MouseButtonWentUp(input_system* Input, u32 MouseButton) {

	b32 Result = 0;

	button_state* State = &Input->MouseButtons[MouseButton];

	if (!State->IsDown && State->TransitionHappened) {
		Result = true;
	}

	return(Result);
}

inline b32 MouseInRect(input_system* Input, rect2 Rect) {
	b32 Result = 0;

	Result = 
		(Input->MouseX >= Rect.Min.x) &&
		(Input->MouseY >= Rect.Min.y) &&
		(Input->MouseX <= Rect.Max.x) &&
		(Input->MouseY <= Rect.Max.y);

	return(Result);
}

inline b32 MouseInRect(input_system* Input, v2 P, v2 Dim) {
	b32 Result = 0;

	rect2 Rect;
	Rect.Min = P;
	Rect.Max = P + Dim;

	Result =
		(Input->MouseX >= Rect.Min.x) &&
		(Input->MouseY >= Rect.Min.y) &&
		(Input->MouseX <= Rect.Max.x) &&
		(Input->MouseY <= Rect.Max.y);

	return(Result);
}

inline b32 MouseButtonIsDown(input_system* Input, u32 MouseBut) {
	b32 Result = Input->MouseButtons[MouseBut].IsDown;

	return(Result);
}

#endif