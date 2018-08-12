#ifndef GORE_VOXEL_SHARED_H_INCLUDED
#define GORE_VOXEL_SHARED_H_INCLUDED

#include "gore_math.h"
#include <vector>

#define VOXEL_WORLD_ENABLED 0

#define VOXEL_ENABLE_FRUSTUM_CULLING 1

#define VOXEL_CHUNK_HEIGHT 256
#define VOXEL_CHUNK_WIDTH 16
#define VOXEL_CHUNK_TOTAL_VOXELS_COUNT 65536
#define VOXEL_CHUNK_HORZ_LAYER_VOXEL_COUNT 256
#define VOXEL_CHUNK_VERT_LAYER_VOXEL_COUNT 4096
#define VOXEL_MAX_MESH_BYTE_SIZE 9437184

#define VOXEL_WATER_LEVEL 100

typedef u32 voxel_vert_t;
#define VOXEL_VERTEX_SIZE sizeof(voxel_vert_t)

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

	voxel_vert_t* Vertices;
	u32 VerticesCount;

	platform_mutex MeshUseMutex;
};

inline b32 IsVoxelSetInNeighbour(u8* Array, int Index) {
	int TargetByte = Index >> 3;
	int TargetBit = Index & 7;

	int Res = Array[TargetByte] & (1 << TargetBit);
	return(Res != 0);
}

struct neighbours_set_info {
	b32 LeftExist;
	b32 RightExist;
	b32 FrontExist;
	b32 BackExist;
	b32 TopExist;
	b32 BottomExist;

	u8 Left[VOXEL_CHUNK_HEIGHT * VOXEL_CHUNK_WIDTH / 8];
	u8 Right[VOXEL_CHUNK_HEIGHT * VOXEL_CHUNK_WIDTH / 8];
	u8 Front[VOXEL_CHUNK_HEIGHT * VOXEL_CHUNK_WIDTH / 8];
	u8 Back[VOXEL_CHUNK_HEIGHT * VOXEL_CHUNK_WIDTH / 8];
	u8 Top[VOXEL_CHUNK_WIDTH * VOXEL_CHUNK_WIDTH / 8];
	u8 Bottom[VOXEL_CHUNK_WIDTH * VOXEL_CHUNK_WIDTH / 8];
};

struct neighbours_chunks {

	union {
		struct {
			struct voxel_chunk_info* LeftChunk;
			struct voxel_chunk_info* RightChunk;
			struct voxel_chunk_info* TopChunk;
			struct voxel_chunk_info* BottomChunk;
			struct voxel_chunk_info* FrontChunk;
			struct voxel_chunk_info* BackChunk;
		};
		struct voxel_chunk_info* ChunksPointers[6];
	};

	union {
		struct {
			u32 LeftChunkState;
			u32 RightChunkState;
			u32 TopChunkState;
			u32 BottomChunkState;
			u32 FrontChunkState;
			u32 BackChunkState;
		};
		u32 ChunksStates[6];
	};
};

struct voxel_chunk_info {
	platform_atomic_type_u32 State;

	platform_mutex ChunkUseMutex;

	int IndexX;
	int IndexY;
	int IndexZ;

	//u8* Voxels;
	u8 Voxels[VOXEL_CHUNK_TOTAL_VOXELS_COUNT];

	voxel_mesh_info MeshInfo;

	neighbours_chunks OldNeighbours;

	//NOTE(dima): Used to store loaded chunk data
	struct threadwork_data* Threadwork;
};

inline b32 ChunkNotZeroAndGenerated(voxel_chunk_info* Chunk) {
	b32 Result = 0;

	if (Chunk) {
		if (Chunk->State == VoxelChunkState_Ready) {
			Result = 1;
		}
	}

	return(Result);
}

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
	VoxelMaterial_Water,

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