#include "row.h"

void ROWPushRectEntity(render_stack* Stack, v2 P, v2 AlignFromTopLeft, v2 Dim) {
	v2 TargetRectMin = V2(
		P.x - AlignFromTopLeft.x * Dim.x,
		P.y - (1.0f - AlignFromTopLeft.y) * Dim.y);

	rect2 ResultEntityRect = Rect2MinDim(TargetRectMin, Dim);

	RENDERPushSprite(Stack, 0, ResultEntityRect);
}

void UpdateROW(game_mode_state* GameModeState, engine_systems* EngineSystems) {
	row_state* ROWState = (row_state*)GameModeState->GameModeMemory.BaseAddress;

	if (!ROWState->IsInitialized) {

		PushStruct(&GameModeState->GameModeMemory, row_state);
		ROWState->GameModeMemory = &GameModeState->GameModeMemory;

		ROWState->Camera = GAMECreateCamera();
		ROWState->CameraSetup = {};

		ROWState->PlayerP = {};
		ROWState->PlayerDim = V2(1.0f, 3.0f / 2.0f) * 100;

		ROWState->IsInitialized = 1;
	}

	ROWState->Camera.Position.z = 5;

	GAMEUpdateCameraVectorsBasedOnUpAndFront(
		&ROWState->Camera, 
		V3(0.0f, 0.0f, -1.0f), 
		V3(0.0f, 1.0f, 0.0f));

	ROWState->CameraSetup = GAMECameraSetup(
		ROWState->Camera,
		EngineSystems->RenderState->RenderWidth,
		EngineSystems->RenderState->RenderHeight,
		CameraProjection_Orthographic,
		0.1f, 100.0f);

	render_stack* RenderStack = EngineSystems->RenderState->NamedStacks.Main;

	RENDERSetCameraSetup(RenderStack, &ROWState->CameraSetup);

	//RENDERPushClear(RenderStack, V3(1.0f, 1.0f, 1.0f));

	ROWPushRectEntity(RenderStack, V2(0.0f, 0.0f), V2(0.5f, 1.0f), ROWState->PlayerDim);
}