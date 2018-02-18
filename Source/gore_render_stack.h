#ifndef GORE_RENDER_STACK_H
#define GORE_RENDER_STACK_H

#include "gore_game_layer.h"

struct render_stack {
	stacked_memory Data;

	u32 EntryCount;
};

enum render_stack_entry_type {
	RenderStackEntry_Bitmap = 1,
	RenderStackEntry_Clear,
	RenderStackEntry_Gradient,
	RenderStackEntry_Rectangle,
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

struct render_stack_entry_rectangle {
	v4 ModulationColor;
	v2 P;
	v2 Dim;
};

struct render_stack_entry_header {
	u32 Type;
	u32 SizeOfEntryType;
};

extern void PushBitmap(render_stack* Stack, rgba_buffer* Bitmap, v2 P, float Height, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f));
extern void PushRect(render_stack* Stack, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f));
extern void PushRect(render_stack* Stack, rect2 Rect, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f));
extern void PushRectOutline(render_stack* Stack, v2 P, v2 Dim, int PixelWidth = 1, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f));
extern void PushRectOutline(render_stack* Stack, rect2 Rect, int PixelWidth = 1, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f));
extern void PushRectInnerOutline(render_stack* Stack, rect2 Rect, int PixelWidth = 1, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f));
extern void PushClear(render_stack* Stack, v3 Clear);
extern void PushGradient(render_stack* Stack, v3 Color);

extern render_stack BeginRenderStack(u32 Size);
extern void EndRenderStack(render_stack* Stack);

#endif