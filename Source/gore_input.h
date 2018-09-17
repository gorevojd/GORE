#ifndef GORE_INPUT_H_INCLUDED
#define GORE_INPUT_H_INCLUDED

#include "gore_platform.h"

enum button_type {
	KeyType_W,
	KeyType_A,
	KeyType_S,
	KeyType_D,

	KeyType_Q,
	KeyType_R,
	KeyType_E,

	KeyType_Space,
	KeyType_LCtrl,
	KeyType_RCtrl,
	KeyType_LShift,
	KeyType_RShift,
	KeyType_Esc,
	KeyType_Return,
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
	KeyType_Backquote,
	KeyType_Backspace,
	KeyType_Tab,

	KeyType_Left,
	KeyType_Right,
	KeyType_Up,
	KeyType_Down,

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

typedef struct  button_state {
	b32 IsDown;
	b32 TransitionHappened;

	b32 IsDoubleClick;
}button_state;

typedef struct input_system {
	button_state Buttons[KeyType_Count];
	button_state MouseButtons[MouseButton_Count];

	v2 MouseP;
	v2 CenterP;
	v2 LastMouseP;

	v2 GlobalMouseP;
	v2 GlobalCenterP;
	v2 LastGlobalMouseP;

	v2 WindowDim;

	float DeltaTime;
	float Time;
}input_system;

inline b32 ButtonIsDown(input_system* Input, u32 KeyType) {
	b32 Result = Input->Buttons[KeyType].IsDown;

	return(Result);
}

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
		(Input->MouseP.x >= Rect.Min.x) &&
		(Input->MouseP.y >= Rect.Min.y) &&
		(Input->MouseP.x <= Rect.Max.x) &&
		(Input->MouseP.y <= Rect.Max.y);

	return(Result);
}

inline b32 MouseInRect(input_system* Input, v2 P, v2 Dim) {
	b32 Result = 0;

	rect2 Rect;
	Rect.Min = P;
	Rect.Max = P + Dim;

	Result =
		(Input->MouseP.x >= Rect.Min.x) &&
		(Input->MouseP.y >= Rect.Min.y) &&
		(Input->MouseP.x <= Rect.Max.x) &&
		(Input->MouseP.y <= Rect.Max.y);

	return(Result);
}

inline b32 MouseButtonIsDown(input_system* Input, u32 MouseBut) {
	b32 Result = Input->MouseButtons[MouseBut].IsDown;

	return(Result);
}

inline b32 MouseLeftWentDownInRect(input_system* Input, rect2 Rect) {
	b32 Result = 0;

	if (MouseInRect(Input, Rect)) {
		if (MouseButtonWentDown(Input, MouseButton_Left)) {
			Result = 1;
		}
	}

	return(Result);
}

inline b32 MouseRightWentDownInRect(input_system* Input, rect2 Rect) {
	b32 Result = 0;

	if (MouseInRect(Input, Rect)) {
		if (MouseButtonWentDown(Input, MouseButton_Right)) {
			Result = 1;
		}
	}

	return(Result);
}

#endif