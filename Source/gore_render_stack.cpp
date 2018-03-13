#include "gore_render_stack.h"

render_stack RENDERBeginStack(u32 Size, int RenderWidth, int RenderHeight) {
	render_stack Result = {};

	u32 MemoryToAlloc = Size;

	Result.Data = AllocateStackedMemory(MemoryToAlloc);
	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;

	return(Result);
}

void RENDEREndStack(render_stack* Stack) {
	DeallocateStackedMemory(&Stack->Data);
}

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

inline void* RENDERPushEntryToStack(render_stack* Stack, u32 SizeOfType, u32 TypeEnum){
	render_stack_entry_header* Header = 
		(render_stack_entry_header*)RENDERPushToStack(Stack, sizeof(render_stack_entry_header));
	
	Stack->EntryCount++;
	Header->Type = TypeEnum;
	Header->SizeOfEntryType = SizeOfType;
	void* EntryData = RENDERPushToStack(Stack, SizeOfType);

	return(EntryData);
}
#define PUSH_RENDER_ENTRY(Stack, type, entry_type_enum)	(type *)(RENDERPushEntryToStack(Stack, sizeof(type), entry_type_enum))

void RENDERPushBitmap(render_stack* Stack, rgba_buffer* Bitmap, v2 P, float Height, v4 ModulationColor) {
	render_stack_entry_bitmap* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_bitmap, RenderStackEntry_Bitmap);

	Entry->P = P;
	Entry->Dim = V2(Bitmap->WidthOverHeight * Height, Height);
	Entry->ModulationColor = ModulationColor;

	Entry->Bitmap = Bitmap;
}

void RENDERPushRect(render_stack* Stack, v2 P, v2 Dim, v4 ModulationColor) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);

	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}


void RENDERPushRect(render_stack* Stack, rect2 Rect, v4 ModulationColor) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);

	Entry->P = Rect.Min;
	Entry->Dim = Rect.Max - Rect.Min;
	Entry->ModulationColor = ModulationColor;
}

void RENDERPushRectOutline(render_stack* Stack, v2 P, v2 Dim, int PixelWidth, v4 Color) {
	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

void RENDERPushRectOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

void RENDERPushRectInnerOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	RENDERPushRect(Stack, V2(P.x, P.y), V2(Dim.x, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + PixelWidth), V2(PixelWidth, Dim.y - PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + PixelWidth, P.y + Dim.y - PixelWidth), V2(Dim.x - PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x - PixelWidth, P.y + PixelWidth), V2(PixelWidth, Dim.y - 2 * PixelWidth), Color);
}

void RENDERPushClear(render_stack* Stack, v3 Clear){
	render_stack_entry_clear* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_clear, RenderStackEntry_Clear);

	Entry->Color = Clear;
}

void RENDERPushGradient(render_stack* Stack, v3 Color) {
	render_stack_entry_gradient* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_gradient, RenderStackEntry_Gradient);

	Entry->Color = Color;
}

void RENDERPushBeginText(render_stack* Stack, font_info* FontInfo) {
	render_stack_entry_begin_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_begin_text, RenderStackEntry_BeginText);

	Entry->FontInfo = FontInfo;
}

void RENDERPushEndText(render_stack* Stack) {
	render_stack_entry_end_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_end_text, RenderStackEntry_EndText);


}

void RENDERPushGlyph(render_stack* Stack, int Codepoint, v2 P, v2 Dim, v4 ModulationColor) {
	FUNCTION_TIMING();

	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderStackEntry_Glyph);

	Entry->Codepoint = Codepoint;
	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}

void RENDERPushCameraSetup(render_stack* Stack, game_camera_setup Setup) {
	render_stack_entry_camera_setup* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_camera_setup, RenderStackEntry_CameraSetup);

	Entry->CameraSetup = Setup;
}