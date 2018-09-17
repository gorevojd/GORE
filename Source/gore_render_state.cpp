#include "gore_render_state.h"

render_stack RenderBeginStack(render_state* RenderState, stacked_memory* SplitedRenderMemory) {
	render_stack Result = {};

	Result.CameraSetup = {};
	Result.CameraSetupIsSet = 0;

	Result.InitStack = SplitedRenderMemory;
	Result.Data = BeginTempStackedMemory(SplitedRenderMemory, SplitedRenderMemory->MaxSize);

	Result.ParentRenderState = RenderState;

	return(Result);
}



render_state RENDERBeginStack(stacked_memory* RenderMemory, int RenderWidth, int RenderHeight, asset_system* AssetSystem, input_system* InputSystem) {
	render_state Result = {};

	Result.InitStack = RenderMemory;
	//NOTE(dima): 16 is subtracted because of possible alignment problems. If alignment is happened then it might be not enough memory
	Result.Data = BeginTempStackedMemory(RenderMemory, RenderMemory->MaxSize);

	Result.EntryCount = 0;
	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;
	Result.AssetSystem = AssetSystem;
	Result.InputSystem = InputSystem;

	Result.LowPolyCylMeshID = GetAssetByBestFloatTag(AssetSystem, GameAsset_Cylynder, GameAssetTag_LOD, 0.0f, AssetType_Mesh);

	return(Result);
}

void RENDEREndStack(render_state* State) {
	EndTempStackedMemory(State->InitStack, &State->Data);
}
