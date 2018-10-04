#ifndef ROW_H_INCLUDED
#define ROW_H_INCLUDED

#include "gore_engine.h"
#include "gore_game_mode.h"
#include "gore_game_common.h"

/*
	 /$$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$         /$$$$$$  /$$$$$$$$       /$$      /$$  /$$$$$$  /$$$$$$$  /$$$$$$$  /$$$$$$  /$$$$$$  /$$$$$$$ 
	| $$__  $$ /$$__  $$ /$$__  $$| $$__  $$       /$$__  $$| $$_____/      | $$  /$ | $$ /$$__  $$| $$__  $$| $$__  $$|_  $$_/ /$$__  $$| $$__  $$
	| $$  \ $$| $$  \ $$| $$  \ $$| $$  \ $$      | $$  \ $$| $$            | $$ /$$$| $$| $$  \ $$| $$  \ $$| $$  \ $$  | $$  | $$  \ $$| $$  \ $$
	| $$$$$$$/| $$  | $$| $$$$$$$$| $$  | $$      | $$  | $$| $$$$$         | $$/$$ $$ $$| $$$$$$$$| $$$$$$$/| $$$$$$$/  | $$  | $$  | $$| $$$$$$$/
	| $$__  $$| $$  | $$| $$__  $$| $$  | $$      | $$  | $$| $$__/         | $$$$_  $$$$| $$__  $$| $$__  $$| $$__  $$  | $$  | $$  | $$| $$__  $$
	| $$  \ $$| $$  | $$| $$  | $$| $$  | $$      | $$  | $$| $$            | $$$/ \  $$$| $$  | $$| $$  \ $$| $$  \ $$  | $$  | $$  | $$| $$  \ $$
	| $$  | $$|  $$$$$$/| $$  | $$| $$$$$$$/      |  $$$$$$/| $$            | $$/   \  $$| $$  | $$| $$  | $$| $$  | $$ /$$$$$$|  $$$$$$/| $$  | $$
	|__/  |__/ \______/ |__/  |__/|_______/        \______/ |__/            |__/     \__/|__/  |__/|__/  |__/|__/  |__/|______/ \______/ |__/  |__/
*/

struct row_state {
	b32 IsInitialized;
	stacked_memory* GameModeMemory;

	bitmap_id PlayerBitmapID;

	v2 PlayerP;
	v2 PlayerVelocity;
	v2 PlayerDim;
	b32 PlayerFacingLeft;
	v2 InitJumpVelocity;

	v2 Gravity;

	float PixelsPerMeter;

	game_camera Camera;
	game_camera_setup CameraSetup;
};

void UpdateROW(game_mode_state* GameModeState, engine_systems* EngineSystems);

#endif