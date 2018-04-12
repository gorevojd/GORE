#ifndef GORE_GEOMETRIKA_H_INCLUDED
#define GORE_GEOMETRIKA_H_INCLUDED

#include "gore_platform.h"
#include "gore_game_common.h"
#include "gore_input.h"
#include "gore_render_state.h"

struct geometrika_state {
	b32 IsInitialized;

	b32 CapturingMouse;

	game_camera Camera;
};

extern void GEOMKAUpdateAndRender(
	geometrika_state* State, 
	asset_system* AssetSystem,
	render_state* RenderStack,
	input_system* Input);

#endif