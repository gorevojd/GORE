#include "gore_render_state.h"

render_state RENDERBeginStack(stacked_memory* RenderMemory, int RenderWidth, int RenderHeight) {
	render_state Result = {};

	Result.InitStack = RenderMemory;
	Result.Data = BeginTempStackedMemory(RenderMemory, RenderMemory->MaxSize, MemAllocFlag_Align16);

	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;

	return(Result);
}

void RENDEREndStack(render_state* State) {
	EndTempStackedMemory(State->InitStack, &State->Data);
}
