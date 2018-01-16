#include "workout_render_stack.h"

render_stack BeginRenderStack(u32 Size) {
	render_stack Result;

	u32 MemoryToAlloc = Size;

	Result.Data = AllocateStackedMemory(MemoryToAlloc);

	return(Result);
}

void EndRenderStack(render_stack* Stack) {
	DeallocateStackedMemory(&Stack->Data);
}

inline void* PushToRenderStack(render_stack* Stack, u32 Size) {
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

inline void* PushRenderEntryToStack(render_stack* Stack, u32 SizeOfType, u32 TypeEnum){
	render_stack_entry_header* Header = 
		(render_stack_entry_header*)PushToRenderStack(Stack, sizeof(render_stack_entry_header));
	
	Header->Type = TypeEnum;
	Header->SizeOfEntryType = SizeOfType;
	void* EntryData = PushToRenderStack(Stack, SizeOfType);

	return(EntryData);
}
#define PUSH_RENDER_ENTRY(Stack, type, entry_type_enum)	(PushRenderEntryToStack(Stack, sizeof(type), entry_type_enum))

void PushBitmap(render_stack* Stack, rgba_buffer* Bitmap, v2 P, float Height, v4 ModulationColor) {
	void* EntryData = PUSH_RENDER_ENTRY(Stack, render_stack_entry_bitmap, RenderStackEntry_Bitmap);
	render_stack_entry_bitmap* Entry = (render_stack_entry_bitmap*)EntryData;

	Entry->P = P;
	Entry->Height = Height;
	Entry->ModulationColor = ModulationColor;

	Entry->Bitmap = Bitmap;
}

void PushRect(render_stack* Stack, v2 P, v2 Dim, v4 ModulationColor) {
	void* EntryData = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);
	render_stack_entry_rectangle* Entry = (render_stack_entry_rectangle*)EntryData;

	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}


void PushRect(render_stack* Stack, rect2 Rect, v4 ModulationColor) {
	void* EntryData = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderStackEntry_Rectangle);
	render_stack_entry_rectangle* Entry = (render_stack_entry_rectangle*)EntryData;

	Entry->P = Rect.Min;
	Entry->Dim = Rect.Max - Rect.Min;
	Entry->ModulationColor = ModulationColor;
}

void PushRectOutline(render_stack* Stack, v2 P, v2 Dim, int PixelWidth, v4 Color) {
	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	PushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	PushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	PushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	PushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

void PushRectOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	PushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	PushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	PushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	PushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

void PushRectInnerOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	PushRect(Stack, V2(P.x, P.y), V2(Dim.x, PixelWidth), Color);
	PushRect(Stack, V2(P.x, P.y + PixelWidth), V2(PixelWidth, Dim.y - PixelWidth), Color);
	PushRect(Stack, V2(P.x + PixelWidth, P.y + Dim.y - PixelWidth), V2(Dim.x - PixelWidth, PixelWidth), Color);
	PushRect(Stack, V2(P.x + Dim.x - PixelWidth, P.y + PixelWidth), V2(PixelWidth, Dim.y - 2 * PixelWidth), Color);
}

void PushClear(render_stack* Stack, v3 Clear){
	void* EntryData = PUSH_RENDER_ENTRY(Stack, render_stack_entry_clear, RenderStackEntry_Clear);
	render_stack_entry_clear* Entry = (render_stack_entry_clear*)EntryData;

	Entry->Color = Clear;
}

void PushGradient(render_stack* Stack, v3 Color) {
	void* EntryData = PUSH_RENDER_ENTRY(Stack, render_stack_entry_gradient, RenderStackEntry_Gradient);
	render_stack_entry_gradient* Entry = (render_stack_entry_gradient*)EntryData;

	Entry->Color = Color;
}