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
#include "gore_game_mode.h"


#define CELLURAL_CELL_PIXEL_WIDTH 9

enum machine_cell_type {
	MachineCell_None,

	MachineCell_Solid, 
	MachineCell_Fadeout,
};

struct cellural_machine {
	int CellsXCount;
	int CellsYCount;

	int StartOffsetX;
	int StartOffsetY;

	bitmap_info Bitmap;

	u16* Colors;
	u8* Alphas;
	u8* Types;

	float* LifeStart;
	u8* LifeLenSeconds;

	float TimeCounterForSpawning;
	int ColorIncIndex;
};

struct geometrika_state {
	b32 IsInitialized;

	random_state Random;

	b32 CapturingMouse;

	game_camera Camera;
	b32 CameraAutoMove;

	stacked_memory CelluralMachineMemory;
	cellural_machine CelluralMachine;

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
	engine_systems* EngineSysState);

#endif