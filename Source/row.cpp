#include "row.h"

void UpdateROW(game_mode_state* GameModeState, engine_systems* EngineSystems) {
	row_state* ROWState = (row_state*)GameModeState->GameModeMemory.BaseAddress;

	if (!ROWState->IsInitialized) {

		PushStruct(&GameModeState->GameModeMemory, row_state);
		ROWState->GameModeMemory = &GameModeState->GameModeMemory;

		ROWState->Camera = GAMECreateCamera();

		ROWState->PlayerP = {};
		ROWState->PlayerDim = V2(1.0f, 5.0f / 3.0f);

		ROWState->IsInitialized = 1;
	}

	game_camera_setup CameraSetup = GAMECameraSetup(
		ROWState->Camera,
		EngineSystems->RenderState->RenderWidth,
		EngineSystems->RenderState->RenderHeight,
		CameraProjection_Orthographic,
		0.1f, 100.0f);

	render_stack* RenderStack = EngineSystems->RenderState->NamedStacks.Main;

	RENDERSetCameraSetup(RenderStack, CameraSetup);


}