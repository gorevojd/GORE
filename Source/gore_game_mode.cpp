#include "gore_game_mode.h"

#include "gore_voxshared.h"
#include "gore_voxmesh.h"
#include "gore_lpterrain.h"
#include "geometrika.h"
#include "gore_game_main_menu.h"

void SwitchGameMode(game_mode_state* GameModeState, u32 NewGameModeType) {
	GameModeState->GameModeShouldBeSwitched = 1;
	GameModeState->NewGameModeType = NewGameModeType;
}

void GameModeUpdate(engine_systems* EngineSystems) {

	game_mode_state* GameModeState = (game_mode_state*)PlatformApi.GameModeMemoryBlock.BaseAddress;
	if (!GameModeState->IsInitialized) {

		GameModeState = PushStructUnaligned(&PlatformApi.GameModeMemoryBlock, game_mode_state);

		GameModeState->GameModeMemory = InitStackedMemory(
			GetCurrentMemoryBase(&PlatformApi.GameModeMemoryBlock),
			PlatformApi.GameModeMemoryBlock.MaxSize - sizeof(game_mode_state));

		GameModeState->GameModeType = GameMode_MainMenu;

		GameModeState->IsInitialized = 1;
	}

	RenderBeginFrame(EngineSystems->RenderState);

#if GORE_DEBUG_ENABLED
	GUIBeginFrame(EngineSystems->DEBUGState->GUIState);
#endif

	BEGIN_TIMING("GameModeUpdate");
	switch (GameModeState->GameModeType)
	{
		case GameMode_MainMenu: {
			UpdateMainMenu(GameModeState, EngineSystems);
		}break;

		case GameMode_Geometrica: {
			GEOMKAUpdateAndRender(GameModeState, EngineSystems);
		}break;

		case GameMode_VoxelWorld: {
			VoxelChunksGenerationUpdate(
				GameModeState,
				EngineSystems->RenderState,
				PlatformApi.GetThreadQueueInfo(PlatformApi.SuperHighQueue).WorkingThreadsCount,
				EngineSystems->InputSystem);
		}break;

		case GameMode_LowPolyTerrain: {
			//UpdateLowPolyterrainWorld();
		}break;
	}
	END_TIMING();

#if GORE_DEBUG_ENABLED
	BEGIN_TIMING("Debug update");
	BEGIN_SECTION("Profile");
	DEBUG_VALUE(DebugValue_FramesSlider);
	DEBUG_VALUE(DebugValue_ViewFrameInfo);
	DEBUG_VALUE(DebugValue_ProfileOverlays);
	END_SECTION();

	DEBUGUpdate(EngineSystems->DEBUGState);
	END_TIMING();
#endif
}

/*
	NOTE(dima):
		Some system might want to deinitialize their states at
		the end of the frame. So, that function will be called 
		at the end of the frame to perform this kind of 
		operations stored in there...... :D
*/

void GameModeFinalizeFrame(engine_systems* EngineSystems) {
	game_mode_state* GameModeState = (game_mode_state*)PlatformApi.GameModeMemoryBlock.BaseAddress;

	switch (GameModeState->GameModeType) {
		case GameMode_MainMenu: {
		}break;

		case GameMode_Geometrica: {
		}break;

		case GameMode_VoxelWorld: {
		}break;

		case GameMode_LowPolyTerrain: {
		}break;
	}

#if GORE_DEBUG_ENABLED
	GUIEndFrame(EngineSystems->DEBUGState->GUIState);
#endif

	RenderEndFrame(EngineSystems->RenderState);

	if (GameModeState->GameModeShouldBeSwitched) {
		ClearStackedMemory(&GameModeState->GameModeMemory);
		GameModeState->GameModeType = GameModeState->NewGameModeType;

		GameModeState->GameModeShouldBeSwitched = 0;
	}
}

