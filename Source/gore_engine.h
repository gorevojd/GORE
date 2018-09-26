#ifndef GORE_ENGINE_H_INCLUDED
#define GORE_ENGINE_H_INCLUDED

#include "gore_platform.h"

#include "gore_colors.h"
#include "gore_asset.h"
#include "gore_voxshared.h"

#include "gore_debug.h"

struct engine_systems {
	stacked_memory SystemsStateMemory;

	input_system* InputSystem;
	asset_system* AssetSystem;

	stacked_memory ColorsMemory;
	color_state* ColorsState;

	stacked_memory RENDERMemory;
	render_state* RenderState;

	game_settings* GameSettings;

#if GORE_DEBUG_ENABLED
	stacked_memory DEBUGMem;
	debug_state* DEBUGState;
#endif
};

engine_systems* EngineSystemsInit(input_system* InputSystem, game_settings* GameSettings);

#endif