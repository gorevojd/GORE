#include "gore_game_mode.h"

#include "gore_voxshared.h"
#include "gore_voxmesh.h"
#include "gore_lpterrain.h"

void SwitchGameMode(game_mode_state* GameModeState, u32 NewGameModeType) {

}

void UpdateGameMode(input_system* InputSystem, game_settings* GameSettings) {

	game_mode_state* GameModeState = (game_mode_state*)PlatformApi.GameModeMemoryBlock.BaseAddress;
	if (!GameModeState->IsInitialized) {

		GameModeState = PushStructUnaligned(&PlatformApi.GameModeMemoryBlock, game_mode_state);

		GameModeState->GameModeMemory = InitStackedMemory(
			GetCurrentMemoryBase(&PlatformApi.GameModeMemoryBlock),
			PlatformApi.GameModeMemoryBlock.MaxSize - sizeof(game_mode_state));

		GameModeState->IsInitialized = 1;
	}

	permanent_state* PermanentState = (permanent_state*)PlatformApi.PermanentMemoryBlock.BaseAddress;
	if(!PermanentState->IsInitialized){

		PermanentState = PushStructUnaligned(&PlatformApi.PermanentMemoryBlock, permanent_state);

		PermanentState->PermanentStateMemory = InitStackedMemory(
			GetCurrentMemoryBase(&PlatformApi.PermanentMemoryBlock),
			PlatformApi.PermanentMemoryBlock.MaxSize - sizeof(permanent_state));

		PermanentState->InputSystem = InputSystem;

		//NOTE(dima): Assets initialization
		PermanentState->AssetSystem = PushStruct(&PermanentState->PermanentStateMemory, asset_system);
		ASSETSInit(PermanentState->AssetSystem);

		//NOTE(dima): Render initialization
		stacked_memory RENDERMemory = SplitStackedMemory(&PlatformApi.PermanentMemoryBlock, MEGABYTES(5));
		PermanentState->RenderState = PushStruct(&PermanentState->PermanentStateMemory, render_state);
		*PermanentState->RenderState = RenderInitState(
			RENDERMemory,
			InputSystem->WindowDim.x,
			InputSystem->WindowDim.y,
			PermanentState->AssetSystem,
			PermanentState->InputSystem);

		//NOTE(dima): Initialization of colors state
		PermanentState->ColorsState = PushStruct(&PlatformApi.PermanentMemoryBlock, color_state);
		stacked_memory ColorsMemory = SplitStackedMemory(&PlatformApi.PermanentMemoryBlock, KILOBYTES(20));
		InitColorsState(PermanentState->ColorsState, &ColorsMemory);

		//NOTE(dima): Initialization of GUI state
		PermanentState->GUIState = PushStruct(&PlatformApi.PermanentMemoryBlock, gui_state);
		stacked_memory GUIMemory = SplitStackedMemory(&PlatformApi.PermanentMemoryBlock, MEGABYTES(2));
		GUIInitState(
			PermanentState->GUIState, 
			&GUIMemory, 
			PermanentState->ColorsState, 
			PermanentState->AssetSystem, 
			PermanentState->InputSystem, 
			PermanentState->InputSystem->WindowDim.x, 
			PermanentState->InputSystem->WindowDim.y);
		
		PermanentState->IsInitialized = 1;
	}

	RenderBeginFrame(PermanentState->RenderState);

	switch (GameModeState->GameModeType)
	{
		case GameMode_MainMenu: {
			//UpdateMainMenu();
		}break;

		case GameMode_VoxelWorld: {
			VoxelChunksGenerationUpdate(
				&GameModeState->GameModeMemory,
				PermanentState->RenderState,
				PlatformApi.GetThreadQueueInfo(PlatformApi.SuperHighQueue).WorkingThreadsCount,
				PermanentState->InputSystem);
		}break;

		case GameMode_LowPolyTerrain: {
			//UpdateLowPolyterrainWorld();
		}break;
	}
}

void FinalizeFrameGameMode() {
	game_mode_state* GameModeState = (game_mode_state*)PlatformApi.GameModeMemoryBlock.BaseAddress;
	permanent_state* PermanentState = (permanent_state*)PlatformApi.PermanentMemoryBlock.BaseAddress;

	switch (GameModeState->GameModeType) {
		case GameMode_MainMenu: {
		}break;

		case GameMode_VoxelWorld: {
		}break;

		case GameMode_LowPolyTerrain: {
		}break;
	}

	RenderEndFrame(PermanentState->RenderState);
}

