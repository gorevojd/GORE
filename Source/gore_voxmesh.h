#ifndef GORE_VOXMESH_H_INCLUDED
#define GORE_VOXMESH_H_INCLUDED

#include "gore_asset.h"
#include "gore_voxshared.h"

enum voxel_normal_type_index{
	VoxelNormalIndex_Up,
	VoxelNormalIndex_Down,
	VoxelNormalIndex_Left,
	VoxelNormalIndex_Right,
	VoxelNormalIndex_Front,
	VoxelNormalIndex_Back,

	VoxelNormalIndex_Count,
};

enum voxel_texture_vert_type{
	VoxelTextureVertType_UpLeft,
	VoxelTextureVertType_UpRight,
	VoxelTextureVertType_DownRight,
	VoxelTextureVertType_DownLeft,
};

void GenerateTestChunk(voxel_chunk_info* Chunk);
void VoxmeshGenerate(voxel_mesh_info* Result, voxel_chunk_info* Chunk, voxel_atlas_info* Atlas);

#endif