#ifndef GORE_VOXEL_SHARED_H_INCLUDED
#define GORE_VOXEL_SHARED_H_INCLUDED

#include "gore_math.h"
#include <vector>

#define VOXEL_WORLD_ENABLED 1

#define VOXEL_CHUNK_HEIGHT 256
#define VOXEL_CHUNK_WIDTH 16
#define VOXEL_CHUNK_TOTAL_VOXELS_COUNT 65536
#define VOXEL_CHUNK_HORZ_LAYER_VOXEL_COUNT 256
#define VOXEL_CHUNK_VERT_LAYER_VOXEL_COUNT 4096
#define VOXEL_MAX_MESH_BYTE_SIZE 9437184

typedef u32 voxel_vert_t;
#define VOXEL_VERTEX_SIZE sizeof(voxel_vert_t)

#define USE_STD_VECTOR_FOR_VOXEL_MESH 0

enum voxel_chunk_state {
	VoxelChunkState_None,
	VoxelChunkState_InProcess,
	VoxelChunkState_Ready,
};

enum voxel_mesh_state {
	VoxelMeshState_None,
	VoxelMeshState_InProcess,
	VoxelMeshState_Generated,
	VoxelMeshState_Unloaded,
};

struct voxel_mesh_info {
	platform_atomic_type_u32 State;

	void* MeshHandle;
	//NOTE(dima): MeshHandle2 used to store VBO in openGL
	void* MeshHandle2;

#if USE_STD_VECTOR_FOR_VOXEL_MESH
	std::vector<voxel_vert_t> Vertices;
#else
	voxel_vert_t* Vertices;
#endif
	u32 VerticesCount;

	platform_order_mutex MeshUseMutex;
};

struct voxworld_threadwork {
	char DEBUGData[64];

	voxworld_threadwork* Next;
	voxworld_threadwork* Prev;

	//NOTE(dima): 1 - in use; 0 - free;
	platform_atomic_type_u32 UseState;

	stacked_memory Memory;
	stacked_memory MemoryInternal;
};

struct voxel_chunk_info {
	platform_atomic_type_u32 State;

	int IndexX;
	int IndexY;
	int IndexZ;

	//u8* Voxels;
	u8 Voxels[VOXEL_CHUNK_TOTAL_VOXELS_COUNT];

	voxel_chunk_info* LeftChunk;
	voxel_chunk_info* RightChunk;
	voxel_chunk_info* FrontChunk;
	voxel_chunk_info* BackChunk;
	voxel_chunk_info* TopChunk;
	voxel_chunk_info* BottomChunk;

	voxel_mesh_info MeshInfo;

	//NOTE(dima): Used to store loaded chunk data
	voxworld_threadwork* Threadwork;
};

enum voxel_material_type {
	VoxelMaterial_None,
	VoxelMaterial_Stone,
	VoxelMaterial_Ground,
	VoxelMaterial_Sand,
	VoxelMaterial_GrassyGround,
	VoxelMaterial_Lava,
	VoxelMaterial_SnowGround,
	VoxelMaterial_WinterGround,
	VoxelMaterial_Leaves,
	VoxelMaterial_Brick,
	VoxelMaterial_Logs,
	VoxelMaterial_Birch,
	VoxelMaterial_Tree,

	VoxelMaterial_GrassyBigBrick,
	VoxelMaterial_DecorateBrick,
	VoxelMaterial_BigBrick,
	VoxelMaterial_BookShelf,

	VoxelMaterial_Secret,

	VoxelMaterial_Count,
};

union voxel_face_tex_coords_set {
	struct {
		v2 T0;
		v2 T1;
		v2 T2;
		v2 T3;
	};
	v2 T[4];
};

inline voxel_face_tex_coords_set
GetFaceTexCoordSetForTextureIndex(u32 TextureIndex, u32 OneTextureWidth, u32 AtlasWidth)
{
	float OneTexUVDelta = (float)OneTextureWidth / (float)AtlasWidth;

	int TexturesPerWidth = AtlasWidth / OneTextureWidth;

	int XIndex = TextureIndex % TexturesPerWidth;
	int YIndex = TextureIndex / TexturesPerWidth;

	voxel_face_tex_coords_set Result = {};

	Result.T0 = V2((float)XIndex * OneTexUVDelta, (float)YIndex * OneTexUVDelta);
	Result.T1 = V2((float)(XIndex + 1) * OneTexUVDelta, (float)YIndex * OneTexUVDelta);
	Result.T2 = V2((float)(XIndex + 1) * OneTexUVDelta, (float)(YIndex + 1) * OneTexUVDelta);
	Result.T3 = V2((float)XIndex * OneTexUVDelta, (float)(YIndex + 1) * OneTexUVDelta);

	return(Result);
}

#endif