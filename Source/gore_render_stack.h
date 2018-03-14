#ifndef GORE_RENDER_STACK_H_INCLUDED
#define GORE_RENDER_STACK_H_INCLUDED

#include "gore_game_layer.h"
#include "gore_asset.h"
#include "gore_game_common.h"

struct render_stack {
	stacked_memory Data;

	int RenderWidth;
	int RenderHeight;

	u32 EntryCount;
};

enum render_stack_entry_type {
	RenderStackEntry_None = 0,

	RenderStackEntry_Bitmap,
	RenderStackEntry_Clear,
	RenderStackEntry_Gradient,
	RenderStackEntry_Rectangle,

	RenderStackEntry_Glyph,
	RenderStackEntry_BeginText,
	RenderStackEntry_EndText,

	RenderStackEntry_CameraSetup,
};

struct render_stack_entry_bitmap {
	rgba_buffer* Bitmap;
	v2 P;
	v2 Dim;
	v4 ModulationColor;
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

struct render_stack_entry_glyph {
	int Codepoint;

	v2 P;
	v2 Dim;

	v4 ModulationColor;
};

struct render_stack_entry_begin_text {
	font_info* FontInfo;
};

struct render_stack_entry_end_text {

};

struct render_stack_entry_camera_setup {
	game_camera_setup CameraSetup;
};

struct render_stack_entry_header {
	u32 Type;
	u32 SizeOfEntryType;
};

inline void* RENDERPushToStack(render_stack* Stack, u32 Size) {
	void* Result = 0;

	void* MemPushed = PushSomeMemory(&Stack->Data, Size);
	if (MemPushed) {
		Result = MemPushed;
	}
	else {
		Assert(!"Stack was corrupted");
	}

	return(Result);
}

inline void* RENDERPushEntryToStack(render_stack* Stack, u32 SizeOfType, u32 TypeEnum) {
	render_stack_entry_header* Header =
		(render_stack_entry_header*)RENDERPushToStack(Stack, sizeof(render_stack_entry_header));

	Stack->EntryCount++;
	Header->Type = TypeEnum;
	Header->SizeOfEntryType = SizeOfType;
	void* EntryData = RENDERPushToStack(Stack, SizeOfType);

	return(EntryData);
}
#define PUSH_RENDER_ENTRY(Stack, type, entry_type_enum)	(type *)(RENDERPushEntryToStack(Stack, sizeof(type), entry_type_enum))

inline void RENDERPushBitmap(render_stack* Stack, rgba_buffer* Bitmap, v2 P, float Height, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_bitmap* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_bitmap, RenderStackEntry_Bitmap);

	Entry->P = P;
	Entry->Dim = V2(Bitmap->WidthOverHeight * Height, Height);
	Entry->ModulationColor = ModulationColor;

	Entry->Bitmap = Bitmap;
}

inline void RENDERPushRect(render_stack* Stack, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);

	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}


inline void RENDERPushRect(render_stack* Stack, rect2 Rect, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);

	Entry->P = Rect.Min;
	Entry->Dim = Rect.Max - Rect.Min;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERPushRectOutline(render_stack* Stack, v2 P, v2 Dim, int PixelWidth, v4 ModulationColor = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), ModulationColor);
}

inline void RENDERPushRectOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

inline void RENDERPushRectInnerOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	RENDERPushRect(Stack, V2(P.x, P.y), V2(Dim.x, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + PixelWidth), V2(PixelWidth, Dim.y - PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + PixelWidth, P.y + Dim.y - PixelWidth), V2(Dim.x - PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x - PixelWidth, P.y + PixelWidth), V2(PixelWidth, Dim.y - 2 * PixelWidth), Color);
}

inline void RENDERPushClear(render_stack* Stack, v3 Clear) {
	render_stack_entry_clear* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_clear, RenderStackEntry_Clear);

	Entry->Color = Clear;
}

inline void RENDERPushGradient(render_stack* Stack, v3 Color) {
	render_stack_entry_gradient* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_gradient, RenderStackEntry_Gradient);

	Entry->Color = Color;
}

inline void RENDERPushBeginText(render_stack* Stack, font_info* FontInfo) {
	render_stack_entry_begin_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_begin_text, RenderStackEntry_BeginText);

	Entry->FontInfo = FontInfo;
}

inline void RENDERPushEndText(render_stack* Stack) {
	render_stack_entry_end_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_end_text, RenderStackEntry_EndText);
}

inline void RENDERPushGlyph(render_stack* Stack, int Codepoint, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderStackEntry_Glyph);

	Entry->Codepoint = Codepoint;
	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERPushCameraSetup(render_stack* Stack, game_camera_setup Setup) {
	render_stack_entry_camera_setup* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_camera_setup, RenderStackEntry_CameraSetup);

	Entry->CameraSetup = Setup;
}

extern render_stack RENDERBeginStack(u32 Size, int WindowWidth, int WindowHeight);
extern void RENDEREndStack(render_stack* Stack);

#endif