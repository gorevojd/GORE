#ifndef GORE_H_INCLUDED
#define GORE_H_INCLUDED

#include "gore_engine.h"
#include "gore_game_mode.h"
#include "gore_game_common.h"


struct gore_state {
	b32 IsInitialized;
	stacked_memory* GameModeMemory;

	bitmap_id PlayerBitmapID;

	float ScreenWidth;
	float ScreenHeight;
	float OneOverScreenWidth;
	float OneOverScreenHeight;

	v2 PlayerP;
	v2 PlayerVelocity;
	v2 PlayerDim;
	v2 InitJumpVelocity;

	b32 PlayerFacingLeft;
	float PlayerHealth;
	float PlayerMaxHealth;

	v2 Gravity;

	float PixelsPerMeter;

	float CurrentCameraScaling;
	game_camera Camera;
	game_camera_setup CameraSetup;
};

void UpdateGore(game_mode_state* GameModeState, engine_systems* EngineSystems);

#endif