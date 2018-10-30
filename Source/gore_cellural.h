#ifndef GORE_CELLURAL_H_INCLUDED
#define GORE_CELLURAL_H_INCLUDED

#include "gore_types.h"
#include "gore_random.h"
#include "gore_platform.h"
#include "gore_asset.h"
#include "gore_asset_types.h"
#include "gore_voxshared.h"

#include "gore_asset_common.h"

typedef u8 cellural_t;

struct cellural_buffer {
	cellural_t* Buf;

	int InitX;
	int InitY;

	i32 Width;
	i32 Height;

	u32 Pitch;
};

//NOTE(dima): Function prototypes
cellural_buffer AllocateCelluralBuffer(i32 Width, i32 Height);
void DeallocateCelluralBuffer(cellural_buffer* Buffer);

void CelluralGenerateLaby(cellural_buffer* Buffer, random_state* RandomState);
void CelluralGenerateCave(cellural_buffer* Buffer, float FillPercentage, random_state* RandomState);
bitmap_info CelluralBufferToBitmap(cellural_buffer* Buffer);
mesh_info CelluralBufferToMesh(cellural_buffer* Buffer, int Height, voxel_atlas_info* Atlas);

#endif