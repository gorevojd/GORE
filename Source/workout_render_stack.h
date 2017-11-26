#ifndef WORKOUT_RENDER_STACK_H
#define WORKOUT_RENDER_STACK_H

#include "workout_game_layer.h"

struct render_stack {
	u8* Base;
	u32 Used;
	u32 MaxSize;
};

enum render_stack_entry_type {
	RenderStackEntry_Bitmap = 1,
	RenderStackEntry_Clear,
	RenderStackEntry_Gradient,
};

struct render_stack_entry_bitmap {
	rgba_buffer* Bitmap;
	v2 P;
	v4 ModulationColor;
	float Height;
};

struct render_stack_entry_clear {
	v3 Color;
};

struct render_stack_entry_gradient {
	v3 Color;
};

struct render_stack_entry_header {
	u32 Type;
	u32 SizeOfEntryType;
};

extern void PushBitmap(render_stack* Stack, rgba_buffer* Bitmap, v2 P, float Height, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f));
extern void PushClear(render_stack* Stack, v3 Clear);
extern void PushGradient(render_stack* Stack, v3 Color);

extern render_stack BeginRenderStack();
extern void EndRenderStack(render_stack* Stack);

#endif