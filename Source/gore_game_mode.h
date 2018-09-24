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
	GameMode_LowPolyTerrain,
};


struct game_mode_state {
	b32 IsInitialized;

	stacked_memory GameModeMemory;

	u32 GameModeType;
};

class game_mode_abstract {
private:
	b32 IsInitialized;

public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void FinalizeFrame() = 0;

	void SetInitialized(b32 Value) {
		this->IsInitialized = Value;
	}

	b32 GetInitialized() {
		return(this->IsInitialized);
	}
};

void GameModeUpdate(engine_systems* EngineSystems);
void GameModeFinalizeFrame(engine_systems* EngineSystems);

#endif