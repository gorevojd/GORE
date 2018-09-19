#include "gore_render_state.h"

render_stack* RenderInitStack(render_state* RenderState, u32 StackByteSize, char* Name) {
	render_stack* Result = PushStruct(RenderState->RenderMemory, render_stack);
	*Result = {};

	Result->Next = RenderState->Sentinel.Next;
	Result->Prev = &RenderState->Sentinel;

	Result->Next->Prev = Result;
	Result->Prev->Next = Result;

	CopyStrings(Result->Name, Name);

	Result->CameraSetup = {};
	Result->CameraSetupIsSet = 0;

	Result->InitStack = SplitStackedMemory(RenderState->RenderMemory, StackByteSize);
	Result->Data = {};

	Result->ParentRenderState = RenderState;

	return(Result);
}

void RenderBeginFrame(render_state* RenderState) {
	render_stack* At = RenderState->Sentinel.Next;

	//NOTE(dima): Running through all render stacks
	while (At != &RenderState->Sentinel) {

		At->Data = BeginTempStackedMemory(&At->InitStack, At->InitStack.MaxSize);

		At = At->Next;
	}
}

void RenderEndFrame(render_state* RenderState) {
	render_stack* At = RenderState->Sentinel.Next;

	//NOTE(dima): Running through all render stacks
	while (At != &RenderState->Sentinel) {

		EndTempStackedMemory(&At->InitStack, &At->Data);

		At = At->Next;
	}
}

render_state RenderInitState(
	stacked_memory* RenderMemory,
	int RenderWidth,
	int RenderHeight,
	asset_system* AssetSystem,
	input_system* InputSystem)
{
	render_state Result = {};

	Result.RenderMemory = RenderMemory;

	//NOTE(dima): Render state initialization
	Result.AssetSystem = AssetSystem;
	Result.InputSystem = InputSystem;

	Result.RenderWidth = RenderWidth;
	Result.RenderHeight = RenderHeight;

	Result.LowPolyCylMeshID = GetAssetByBestFloatTag(AssetSystem, GameAsset_Cylynder, GameAssetTag_LOD, 0.0f, AssetType_Mesh);

	//NOTE(dima): Initialization of render stacks
	Result.Sentinel = {};
	Result.Sentinel.Next = &Result.Sentinel;
	Result.Sentinel.Prev = &Result.Sentinel;
	CopyStrings(Result.Sentinel.Name, "Sentinel");

	Result.NamedStacks = {};
	Result.NamedStacks.Main = RenderInitStack(&Result, MEGABYTES(1), "Main");
	Result.NamedStacks.GUI = RenderInitStack(&Result, MEGABYTES(1), "GUI");

	return(Result);
}