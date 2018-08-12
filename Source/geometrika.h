#ifndef GORE_GEOMETRIKA_H_INCLUDED
#define GORE_GEOMETRIKA_H_INCLUDED

#include "gore_platform.h"
#include "gore_game_common.h"
#include "gore_input.h"
#include "gore_render_state.h"
#include "gore_voxshared.h"
#include "gore_voxmesh.h"
#include "gore_lpterrain.h"
#include "gore_cellural.h"

struct geometrika_state {
	b32 IsInitialized;

	b32 CapturingMouse;

	game_camera Camera;
	b32 CameraAutoMove;

#if 1
	bitmap_info* VoxelAtalsBitmap;
	voxel_chunk_info TestChunk;
#endif

	lpter_terrain Terrain[25];
	lpter_mesh Mesh[25];

	surface_material CubeMat;
	surface_material PlaneMat;
};

extern void GEOMKAUpdateAndRender(
	stacked_memory* GameMemoryBlock,
	asset_system* AssetSystem,
	render_state* RenderStack,
	input_system* Input);

#endif