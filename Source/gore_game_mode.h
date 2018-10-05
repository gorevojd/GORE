#ifndef GORE_GAME_MODE_H_INCLUDED
#define GORE_GAME_MODE_H_INCLUDED

#include "gore_platform.h"

#include "gore_colors.h"
#include "gore_game_settings.h"
#include "gore_engine.h"


enum game_mode_type {
	GameMode_EntryTitle,

	GameMode_MainMenu,

	GameMode_Geometrica,
	GameMode_VoxelWorld,
	GameMode_GoreGame,
};


struct game_mode_state {
	b32 IsInitialized;

	stacked_memory GameModeMemory;

	u32 GameModeType;

	b32 GameModeShouldBeSwitched;
	u32 NewGameModeType;
};


void GameModeUpdate(engine_systems* EngineSystems);
void GameModeFinalizeFrame(engine_systems* EngineSystems);
void SwitchGameMode(game_mode_state* GameModeState, u32 NewGameModeType);

#endif