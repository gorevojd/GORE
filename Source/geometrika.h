#ifndef GORE_GEOMETRIKA_H_INCLUDED
#define GORE_GEOMETRIKA_H_INCLUDED

#include "gore_platform.h"
#include "gore_game_common.h"
#include "gore_input.h"
#include "gore_render_state.h"
#include "gore_voxshared.h"
#include "gore_voxmesh.h"
#include "gore_cellural.h"

struct geometrika_state {
	b32 IsInitialized;

	b32 CapturingMouse;

	game_camera Camera;

#if 1
	bitmap_info* VoxelAtalsBitmap;
	voxel_chunk_info TestChunk;
	voxel_mesh_info TestChunkMesh;
#endif

	surface_material CubeMat;
	surface_material PlaneMat;
};

extern void GEOMKAUpdateAndRender(
	geometrika_state* State, 
	asset_system* AssetSystem,
	render_state* RenderStack,
	input_system* Input);

#endif