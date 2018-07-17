#include "gore_render_state.h"

render_state RENDERBeginStack(stacked_memory* RenderMemory, int RenderWidth, int RenderHeight, asset_system* AssetSystem) {
	render_state Result = {};

	Result.RenderPushMutex = {};
	Result.InitStack = RenderMemory;
	Result.Data = BeginTempStackedMemory(RenderMemory, RenderMemory->MaxSize, MemAllocFlag_Align16);

	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;
	Result.AssetSystem = AssetSystem;

	return(Result);
}

void RENDEREndStack(render_state* State) {
	EndTempStackedMemory(State->InitStack, &State->Data);
}
