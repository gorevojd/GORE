#include "gore_render_state.h"

render_state RENDERBeginStack(stacked_memory* RenderMemory, int RenderWidth, int RenderHeight, asset_system* AssetSystem) {
	render_state Result = {};

	Result.RenderPushMutex = {};
	Result.InitStack = RenderMemory;
	//NOTE(dima): 16 is subtracted because of possible alignment problems. If alignment is happened then it might be not enough memory
	Result.Data = BeginTempStackedMemory(RenderMemory, RenderMemory->MaxSize - 16, MemAllocFlag_Align16);

	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;
	Result.AssetSystem = AssetSystem;

	Result.LowPolyCylMeshID = GetAssetByBestFloatTag(AssetSystem, GameAsset_Cylynder, GameAssetTag_LOD, 0.0f, AssetType_Mesh);

	return(Result);
}

void RENDEREndStack(render_state* State) {
	EndTempStackedMemory(State->InitStack, &State->Data);
}
