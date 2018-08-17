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

#define LPTER_CHUNKS_SIDE_COUNT 5
	lpter_terrain Terrain[LPTER_CHUNKS_SIDE_COUNT * LPTER_CHUNKS_SIDE_COUNT];
	lpter_water Water[LPTER_CHUNKS_SIDE_COUNT * LPTER_CHUNKS_SIDE_COUNT];

	surface_material CubeMat;
	surface_material PlaneMat;
};

extern void GEOMKAUpdateAndRender(
	stacked_memory* GameMemoryBlock,
	asset_system* AssetSystem,
	render_state* RenderStack,
	input_system* Input);

#endif