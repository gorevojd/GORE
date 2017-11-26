#include "workout_render_stack.h"

render_stack BeginRenderStack() {
	render_stack Result;

	u32 MemoryToAlloc = 1024 * 1024;

	Result.Base = (u8*)malloc(MemoryToAlloc);
	Result.Used = 0;
	Result.MaxSize = MemoryToAlloc;

	return(Result);
}

void EndRenderStack(render_stack* Stack) {
	Stack->Used = 0;
	free(Stack->Base);
}

inline void* PushToRenderStack(render_stack* Stack, u32 Size) {
	void* Result = 0;

	if (Stack->Used + Size <= Stack->MaxSize) {
		Result = Stack->Base + Stack->Used;
		Stack->Used += Size;
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