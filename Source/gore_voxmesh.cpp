#include "gore_voxmesh.h"

inline u32 GetEncodedVertexData32(
	v3 Pos,
	u8 TexIndex,
	voxel_texture_vert_type TexVertType,
	u8 NormIndex)
{
	u32 Result = 0;

	u32 EncP =
		(((u32)(Pos.x + 0.5f) & 31) << 27) |
		(((u32)(Pos.y + 0.5f) & 511) << 18) |
		(((u32)(Pos.z + 0.5f) & 31) << 13);

	u32 EncN = ((u32)(NormIndex) & 7) << 10;
	u32 EncT = ((u32)(TexIndex) << 2) | ((u32)(TexVertType) & 3);

	Result = EncP | EncN | EncT;

	return(Result);
}

static inline void VoxmeshWriteFace(
	voxel_mesh_info* Mesh,
	v3 P0,
	v3 P1,
	v3 P2,
	v3 P3,
	u8 TextureIndex,
	u8 NormalIndex)
{
	u32 Index = Mesh->VerticesCount;

	u32 Value0 = GetEncodedVertexData32(P0, TextureIndex, VoxelTextureVertType_UpLeft, NormalIndex);
	u32 Value1 = GetEncodedVertexData32(P1, TextureIndex, VoxelTextureVertType_UpRight, NormalIndex);
	u32 Value2 = GetEncodedVertexData32(P2, TextureIndex, VoxelTextureVertType_DownRight, NormalIndex);
	u32 Value3 = GetEncodedVertexData32(P3, TextureIndex, VoxelTextureVertType_DownLeft, NormalIndex);

	Mesh->Vertices[Index] = Value0;
	Mesh->Vertices[Index + 1] = Value1;
	Mesh->Vertices[Index + 2] = Value2;

	Mesh->Vertices[Index + 3] = Value0;
	Mesh->Vertices[Index + 4] = Value2;
	Mesh->Vertices[Index + 5] = Value3;

	Mesh->VerticesCount += 6;
}

static inline void WriteFaceAtFront(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Front);
}

static inline void WriteFaceAtBack(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Back);
}

static inline void WriteFaceAtLeft(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Left);
}

static inline void WriteFaceAtRight(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Right);
}

static inline void WriteFaceAtTop(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z - 0.5f),
		V3(Pos.x + 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		V3(Pos.x - 0.5f, Pos.y + 0.5f, Pos.z + 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Up);
}

static inline void WriteFaceAtBottom(
	voxel_mesh_info* Mesh,
	v3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z + 0.5f),
		V3(Pos.x + 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		V3(Pos.x - 0.5f, Pos.y - 0.5f, Pos.z - 0.5f),
		TextureIndexInAtlas,
		VoxelNormalIndex_Down);
}

#define GET_VOXEL_INDEX(width_index, depth_index, height_index)	\
	(height_index + VOXEL_CHUNK_HEIGHT * (width_index) + VOXEL_CHUNK_VERT_LAYER_VOXEL_COUNT * (depth_index))


static inline b32 NeighbourVoxelExistAndAir(
	voxel_chunk_info* Chunk, 
	int WidthIndex, 
	int HeightIndex,
	int DepthIndex) 
{
	int Result = 0;

	if ((WidthIndex >= 0 && (WidthIndex < VOXEL_CHUNK_WIDTH)) &&
		(HeightIndex >= 0 && (HeightIndex < VOXEL_CHUNK_HEIGHT)) &&
		(DepthIndex >= 0 && (DepthIndex < VOXEL_CHUNK_WIDTH)) &&
		Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex)] == VoxelMaterial_None)
	{
		Result = 1;
	}

	return(Result);
}

/*
	NOTE(dima):
		Memory of chunk is stored in 256-block 
		height columns width-first. Block in column 
		is stored from bottom to top
*/

void VoxmeshGenerate(voxel_mesh_info* Result, voxel_chunk_info* Chunk, voxel_atlas_info* Atlas){

	Result->MeshHandle = 0;

	voxel_chunk_info* LeftChunk = Chunk->LeftChunk;
	voxel_chunk_info* RightChunk = Chunk->RightChunk;
	voxel_chunk_info* TopChunk = Chunk->TopChunk;
	voxel_chunk_info* BottomChunk = Chunk->BottomChunk;
	voxel_chunk_info* BackChunk = Chunk->BackChunk;
	voxel_chunk_info* FrontChunk = Chunk->FrontChunk;

	for (int DepthIndex = 0; DepthIndex < VOXEL_CHUNK_WIDTH; DepthIndex++) {
		for (int WidthIndex = 0; WidthIndex < VOXEL_CHUNK_WIDTH; WidthIndex++) {
			for (int HeightIndex = 0; HeightIndex < VOXEL_CHUNK_HEIGHT; HeightIndex++) {

				u8 ToCheck = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex)];

				v3 VoxelPos;
				VoxelPos.x = WidthIndex + 0.5f;
				VoxelPos.y = HeightIndex + 0.5f;
				VoxelPos.z = DepthIndex + 0.5f;

				voxel_tex_coords_set* TexSet = &Atlas->Materials[ToCheck];

				if (ToCheck != VoxelMaterial_None && TexSet) {
					if ((WidthIndex >= 1 && WidthIndex < (VOXEL_CHUNK_WIDTH - 1)) &&
						(DepthIndex >= 1 && DepthIndex < (VOXEL_CHUNK_WIDTH - 1)) &&
						(HeightIndex >= 1 && HeightIndex < (VOXEL_CHUNK_HEIGHT - 1)))
					{
						u8 UpVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex + 1)];
						u8 DownVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex - 1)];
						u8 RightVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex + 1, DepthIndex, HeightIndex)];
						u8 LeftVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex - 1, DepthIndex, HeightIndex)];
						u8 FrontVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex - 1, HeightIndex)];
						u8 BackVoxel = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex + 1, HeightIndex)];

						if (UpVoxel == VoxelMaterial_None) {
							WriteFaceAtTop(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Top]);
						}

						if (DownVoxel == VoxelMaterial_None) {
							WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
						}

						if (RightVoxel == VoxelMaterial_None) {
							WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
						}

						if (LeftVoxel == VoxelMaterial_None) {
							WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
						}

						if (FrontVoxel == VoxelMaterial_None) {
							WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
						}

						if (BackVoxel == VoxelMaterial_None) {
							WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
						}
					}
					else {
						int IndexInNeighbourChunk = 0;

						if (WidthIndex == 0) {
							if (LeftChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(VOXEL_CHUNK_WIDTH - 1, DepthIndex, HeightIndex);

								if (LeftChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
								}
							}
							else {
								WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
							}
						}
						if (WidthIndex == (VOXEL_CHUNK_WIDTH - 1)) {
							if (RightChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(0, DepthIndex, HeightIndex);

								if (RightChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
								}
							}
							else {
								WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
							}
						}
						if (DepthIndex == 0) {
							if (FrontChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(WidthIndex, VOXEL_CHUNK_WIDTH - 1, HeightIndex);

								if (FrontChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
								}
							}
							else {
								WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
							}
						}

						if (DepthIndex == (VOXEL_CHUNK_WIDTH - 1)) {
							if (BackChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(WidthIndex, 0, HeightIndex);

								if (BackChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
								}
							}
							else {
								WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
							}
						}

						if (HeightIndex == 0) {
							if (BottomChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(WidthIndex, DepthIndex, VOXEL_CHUNK_HEIGHT - 1);

								if (BottomChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
								}
							}
							else {
								WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
							}
						}

						if (HeightIndex == (VOXEL_CHUNK_HEIGHT - 1)) {
							if (TopChunk) {
								IndexInNeighbourChunk = GET_VOXEL_INDEX(WidthIndex, DepthIndex, VOXEL_CHUNK_HEIGHT - 1);

								if (TopChunk->Voxels[IndexInNeighbourChunk] == VoxelMaterial_None) {
									WriteFaceAtTop(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Top]);
								}
							}
							else {
								WriteFaceAtTop(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Top]);
							}
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex - 1,
							HeightIndex,
							DepthIndex))
						{
							WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex + 1,
							HeightIndex,
							DepthIndex))
						{
							WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex,
							HeightIndex + 1,
							DepthIndex))
						{
							WriteFaceAtTop(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Top]);
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex,
							HeightIndex - 1,
							DepthIndex))
						{
							WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex,
							HeightIndex,
							DepthIndex + 1))
						{
							WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
						}

						if (NeighbourVoxelExistAndAir(
							Chunk,
							WidthIndex,
							HeightIndex,
							DepthIndex - 1))
						{
							WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
						}
					}
				}
			}
		}
	}
}

static void BuildColumn(
	voxel_chunk_info* Chunk, 
	int InChunkX, int InChunkY, 
	int StartHeight, int EndHeight,
	u8 BlockType) 
{
	int X = Clamp(InChunkX, 0, VOXEL_CHUNK_WIDTH - 1);
	int Y = Clamp(InChunkY, 0, VOXEL_CHUNK_WIDTH - 1);

	int InStartHeight = Clamp(StartHeight, 0, VOXEL_CHUNK_HEIGHT - 1);
	int InEndHeight = Clamp(EndHeight, 0, VOXEL_CHUNK_HEIGHT - 1);

	int ExactStartHeight;
	int ExactEndHeight;

	if (InStartHeight < InEndHeight){
		ExactStartHeight = InStartHeight;
		ExactEndHeight = InEndHeight;
	}
	else {
		ExactStartHeight = InEndHeight;
		ExactEndHeight = InStartHeight;
	}

	for (int HeightIndex = ExactStartHeight;
		HeightIndex <= ExactEndHeight;
		HeightIndex++)
	{
		Chunk->Voxels[GET_VOXEL_INDEX(X, Y, HeightIndex)] = BlockType;
	}
}

void GenerateTestChunk(voxel_chunk_info* Chunk) {

	Chunk->BackChunk = 0;
	Chunk->FrontChunk = 0;
	Chunk->TopChunk = 0;
	Chunk->BottomChunk = 0;
	Chunk->LeftChunk = 0;
	Chunk->RightChunk = 0;

	int TestStartHeight = 100;

	for (int j = 0; j < VOXEL_CHUNK_WIDTH; j++) {
		for (int i = 0; i < VOXEL_CHUNK_WIDTH; i++) {
			int SetHeightIndex = TestStartHeight + i + j;
			Chunk->Voxels[GET_VOXEL_INDEX(i, j, SetHeightIndex)] = VoxelMaterial_GrassyGround;

			BuildColumn(Chunk, i, j, 0, SetHeightIndex - 1, VoxelMaterial_Ground);
		}
	}
}