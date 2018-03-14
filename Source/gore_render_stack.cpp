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
