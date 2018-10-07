#ifndef GORE_H_INCLUDED
#define GORE_H_INCLUDED

#include "gore_engine.h"
#include "gore_game_mode.h"
#include "gore_game_common.h"

enum entity_type {
	Entity_Player,
	Entity_Wall,
};

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
	v2 PlayerGravity;
	v2 PlayerInitJumpVelocity;

	b32 PlayerFacingLeft;
	float PlayerHealth;
	float PlayerMaxHealth;

	v2 WallAt;
	v2 WallDim;
	v2 WallTopLeftAlign;

	v2 MasterGravity;

	float PixelsPerMeter;

	float CurrentCameraScaling;
	game_camera Camera;
	game_camera_setup CameraSetup;
};

void UpdateGore(game_mode_state* GameModeState, engine_systems* EngineSystems);

#endif