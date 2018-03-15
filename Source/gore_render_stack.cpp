#include "gore_render_stack.h"

render_stack RENDERBeginStack(stacked_memory* RenderMemory, int RenderWidth, int RenderHeight) {
	render_stack Result = {};

	Result.InitStack = RenderMemory;
	Result.Data = BeginTempStackedMemory(RenderMemory, RenderMemory->MaxSize, MemAllocFlag_Align16);

	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;

	return(Result);
}

void RENDEREndStack(render_stack* Stack) {
	EndTempStackedMemory(Stack->InitStack, &Stack->Data);
}
