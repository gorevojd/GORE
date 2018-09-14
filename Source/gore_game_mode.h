#ifndef GORE_GAME_MODE_H_INCLUDED
#define GORE_GAME_MODE_H_INCLUDED

#include "gore_platform.h"

enum game_mode_type {
	GameMode_EntryTitle,

	GameMode_MainMenu,

	GameMode_VoxelWorld,
	GameMode_LowPolyTerrain,
};


struct game_mode_state {
	b32 IsInitialized;

	stacked_memory GameModeMemory;

	u32 GameModeType;
};

struct permanent_state {
	b32 IsInitialized;

	stacked_memory PermanentStateMemory;

	input_system* InputSystem;
	asset_system* AssetSystem;
};

#endif