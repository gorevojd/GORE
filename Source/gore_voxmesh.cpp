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

#if USE_STD_VECTOR_FOR_VOXEL_MESH
	Mesh->Vertices.push_back(Value0);
	Mesh->Vertices.push_back(Value1);
	Mesh->Vertices.push_back(Value2);
	
	Mesh->Vertices.push_back(Value0);
	Mesh->Vertices.push_back(Value2);
	Mesh->Vertices.push_back(Value3);
#else
  	Mesh->Vertices[Index] = Value0;
	Mesh->Vertices[Index + 1] = Value1;
	Mesh->Vertices[Index + 2] = Value2;

	Mesh->Vertices[Index + 3] = Value0;
	Mesh->Vertices[Index + 4] = Value2;
	Mesh->Vertices[Index + 5] = Value3;
#endif

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

#if 0
#define GET_VOXEL_INDEX(width_index, depth_index, height_index)	\
	(height_index + VOXEL_CHUNK_HEIGHT * (width_index) + VOXEL_CHUNK_VERT_LAYER_VOXEL_COUNT * (depth_index))
#else
#define GET_VOXEL_INDEX(width_index, depth_index, height_index)	\
	(VOXEL_CHUNK_HORZ_LAYER_VOXEL_COUNT * (height_index) + VOXEL_CHUNK_WIDTH * (depth_index) + width_index)
#endif


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

	Result->VerticesCount = 0;

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
								//WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
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
								//WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
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
								//WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
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
								//WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
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
								//WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
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
								//WriteFaceAtTop(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Top]);
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


inline v3 GetPosForVoxelChunk(voxel_chunk_info* Chunk) {
	v3 Result;

	Result.x = Chunk->IndexX * VOXEL_CHUNK_WIDTH;
	Result.y = Chunk->IndexY * VOXEL_CHUNK_HEIGHT;
	Result.z = Chunk->IndexZ * VOXEL_CHUNK_WIDTH;

	return(Result);
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

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
void GenerateRandomChunk(voxel_chunk_info* Chunk) {

	Chunk->BackChunk = 0;
	Chunk->FrontChunk = 0;
	Chunk->TopChunk = 0;
	Chunk->BottomChunk = 0;
	Chunk->LeftChunk = 0;
	Chunk->RightChunk = 0;

	for (int BlockIndex = 0;
		BlockIndex < VOXEL_CHUNK_TOTAL_VOXELS_COUNT;
		BlockIndex++)
	{
		Chunk->Voxels[BlockIndex] = VoxelMaterial_None;
	}

	int StartHeight = 128;

	v3 ChunkPos = GetPosForVoxelChunk(Chunk);

	for (int j = 0; j < VOXEL_CHUNK_WIDTH; j++) {
		for (int i = 0; i < VOXEL_CHUNK_WIDTH; i++) {

#if 0
			float NoiseScale1 = 3.0f;&
			float NoiseScale2 = 20.0f;
			float NoiseScale3 = 120.0f;

			float Noise1Scale = 16.0f;
			float Noise1 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / Noise1Scale,
				(float)ChunkPos.y / Noise1Scale,
				(float)(ChunkPos.z + j) / Noise1Scale, 0, 0, 0);

			float Noise2Scale = 32.0f;
			float Noise2 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / Noise2Scale,
				(float)ChunkPos.y / Noise2Scale,
				(float)(ChunkPos.z + j) / Noise2Scale, 0, 0, 0);

			float Noise3Scale = 64.0f;
			float Noise3 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / Noise3Scale,
				(float)ChunkPos.y / Noise3Scale,
				(float)(ChunkPos.z + j) / Noise3Scale, 0, 0, 0);
		
			float RandHeight = (Noise1 * NoiseScale1 + Noise2 * NoiseScale2 + Noise3 * NoiseScale3) + StartHeight;
#else
			int Octaves = 6;
			float Lacunarity = 2.0f;
			float Gain = 0.5f;

			float NoiseS = 256.0f;
#if 1
			float Noise = stb_perlin_fbm_noise3(
				(float)(ChunkPos.x + i) / NoiseS,
				(float)ChunkPos.y / NoiseS,
				(float)(ChunkPos.z + j) / NoiseS,
				Lacunarity, Gain, Octaves, 0, 0, 0);
#else
#if 0
			float Noise = stb_perlin_ridge_noise3(
				(float)(ChunkPos.x + i) / NoiseS,
				(float)ChunkPos.y / NoiseS,
				(float)(ChunkPos.z + j) / NoiseS,
				Lacunarity, Gain, 1.0f, Octaves, 0, 0, 0);
#else

			float Noise = stb_perlin_turbulence_noise3(
				(float)(ChunkPos.x + i) / NoiseS,
				(float)ChunkPos.y / NoiseS,
				(float)(ChunkPos.z + j) / NoiseS,
				Lacunarity, Gain, Octaves, 0, 0, 0);
#endif
#endif

			float RandHeight = StartHeight + Noise * 127;
#endif


			int SetHeight = (int)RandHeight;
			SetHeight = Clamp(SetHeight, 0, VOXEL_CHUNK_HEIGHT - 1);

			Chunk->Voxels[GET_VOXEL_INDEX(i, j, SetHeight)] = VoxelMaterial_SnowGround;

			BuildColumn(Chunk, i, j, 0, SetHeight - 1, VoxelMaterial_WinterGround);
		}
	}
}

#include "stb_sprintf.h"

inline void GetVoxelChunkPosForCamera(
	v3 CamPos, 
	int* IDChunkX,
	int* IDChunkY,
	int* IDChunkZ) 
{
	int ResX;
	int CamPosX = (int)(CamPos.x);
	if (CamPos.x >= 0.0f) {
		ResX = CamPosX / VOXEL_CHUNK_WIDTH;
	}
	else {
		ResX = (CamPosX / VOXEL_CHUNK_WIDTH) - 1;
	}

	int ResY;
	int CamPosY = (int)(CamPos.y);
	if (CamPos.y >= 0.0f) {
		ResY = CamPosY / VOXEL_CHUNK_HEIGHT;
	}
	else {
		ResY = (CamPosY / VOXEL_CHUNK_HEIGHT) - 1;
	}

	int ResZ;
	int CamPosZ = (int)CamPos.z;
	if (CamPos.z >= 0.0f) {
		ResZ = CamPosZ / VOXEL_CHUNK_WIDTH;
	}
	else {
		ResZ = (CamPosZ / VOXEL_CHUNK_WIDTH) - 1;
	}

	*IDChunkX = ResX;
	*IDChunkY = ResY;
	*IDChunkZ = ResZ;
}

static voxworld_threadwork* VoxelAllocateThreadwork(
	stacked_memory* Memory, 
	u32 ThreadworkMemorySize, 
	int* MemoryCounter) 
{
	voxworld_threadwork* Result = PushStruct(Memory, voxworld_threadwork);

	*MemoryCounter += sizeof(voxworld_threadwork) + ThreadworkMemorySize;

	Result->Next = Result;
	Result->Prev = Result;

	Result->UseState = 0;
	Result->MemoryInternal = SplitStackedMemory(Memory, ThreadworkMemorySize);

	return(Result);
}

static void VoxelInsertThreadworkAfter(
	voxworld_threadwork* ToInsert,
	voxworld_threadwork* Sentinel)
{
	ToInsert->Next = Sentinel->Next;
	ToInsert->Prev = Sentinel;

	ToInsert->Next->Prev = ToInsert;
	ToInsert->Prev->Next = ToInsert;
}

static voxworld_threadwork* VoxelBeginThreadwork(
	voxworld_threadwork* FreeSentinel,
	voxworld_threadwork* UseSentinel,
	platform_order_mutex* Mutex,
	int* FreeWorkCount)
{
	voxworld_threadwork* Result = 0;

	BeginOrderMutex(Mutex);

	if (FreeSentinel->Next != FreeSentinel) {
		//NOTE(dima): Putting threadwork list entry to use list
		Result = FreeSentinel->Next;

		Result->Prev->Next = Result->Next;
		Result->Next->Prev = Result->Prev;

		Result->Next = UseSentinel->Next;
		Result->Prev = UseSentinel;

		Result->Next->Prev = Result;
		Result->Prev->Next = Result;

		//NOTE(dima): Beginning temp memory
		Result->Memory = BeginTempStackedMemory(
			&Result->MemoryInternal,
			Result->MemoryInternal.MaxSize,
			MemAllocFlag_Align16);

		(*FreeWorkCount)--;
	}

	EndOrderMutex(Mutex);

	return(Result);
}

static void VoxelEndThreadwork(
	voxworld_threadwork* Threadwork,
	voxworld_threadwork* FreeSentinel,
	platform_order_mutex* Mutex,
	int* FreeWorkCount)
{
	BeginOrderMutex(Mutex);

	//NOTE(dima): Putting threadwork list entry to free list
	Threadwork->Prev->Next = Threadwork->Next;
	Threadwork->Next->Prev = Threadwork->Prev;

	Threadwork->Next = FreeSentinel->Next;
	Threadwork->Prev = FreeSentinel;

	Threadwork->Next->Prev = Threadwork;
	Threadwork->Prev->Next = Threadwork;

	//NOTE(dima): Freing temp memory
	EndTempStackedMemory(&Threadwork->MemoryInternal, &Threadwork->Memory);

	(*FreeWorkCount)++;

	EndOrderMutex(Mutex);
}

inline u32 GetKeyFromIndices(int X, int Y, int Z) {
	char KeyStr[64];
	stbsp_sprintf(KeyStr, "%d|%d|%d", X, Y, Z);
	u32 Result = StringHashFNV(KeyStr);

	return(Result);
}

static void VoxelInsertToTable(voxworld_generation_state* Generation, voxel_chunk_info* Info) 
{
	FUNCTION_TIMING();

	u32 Key = GetKeyFromIndices(Info->IndexX, Info->IndexY, Info->IndexZ);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	voxworld_table_entry** FirstEntry = &Generation->HashTable[InTableIndex];

	voxworld_table_entry* PrevEntry = 0;

	if (*FirstEntry) {
		PrevEntry = *FirstEntry;

		while (PrevEntry->NextInHash) {
			PrevEntry = PrevEntry->NextInHash;
		}

		Generation->HashTableCollisionCount++;
	}

	/*
		NOTE(dima): Now that the prev element found we
		can create slot to insert it at the new place
	*/
	voxworld_table_entry* NewEntry = 0; 
	if (Generation->FreeTableEntrySentinel->NextBro == Generation->FreeTableEntrySentinel) {
		//NOTE(dima): Allocate element
		NewEntry = PushStruct(Generation->TotalMemory, voxworld_table_entry);
		Generation->HashTableMemUsed += sizeof(voxworld_table_entry);
	}
	else {
		//NOTE(dima): Get the element from free list and remove it from there
		NewEntry = Generation->FreeTableEntrySentinel->NextBro;
		
		NewEntry->NextBro->PrevBro = NewEntry->PrevBro;
		NewEntry->PrevBro->NextBro = NewEntry->NextBro;
	}

	//NOTE(dima): Insert element to work list
	NewEntry->PrevBro = Generation->WorkTableEntrySentinel;
	NewEntry->NextBro = Generation->WorkTableEntrySentinel->NextBro;

	NewEntry->PrevBro->NextBro = NewEntry;
	NewEntry->NextBro->PrevBro = NewEntry;

	//NOTE(dima): Initialization
	NewEntry->ValueChunk = Info;
	NewEntry->NextInHash = 0;
	NewEntry->Key = Key;

	if (PrevEntry) {
		PrevEntry->NextInHash = NewEntry;
	}
	else {
		*FirstEntry = NewEntry;
	}

	Generation->HashTableTotalInsertedEntries++;
}

static void VoxelDeleteFromTable(
	voxworld_generation_state* Generation,
	int X, int Y, int Z)
{
	voxel_chunk_info* Result = 0;

	u32 Key = GetKeyFromIndices(X, Y, Z);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	voxworld_table_entry** FirstEntry = &Generation->HashTable[InTableIndex];

	//NOTE(dima): This element MUST exist
	Assert(*FirstEntry);

	voxworld_table_entry* PrevEntry = 0;

	voxworld_table_entry* At = *FirstEntry;
	while (At != 0) {
		if (At->Key == Key) {
			if (At->ValueChunk->IndexX == X &&
				At->ValueChunk->IndexY == Y &&
				At->ValueChunk->IndexZ == Z)
			{
				//NOTE(dima): Delete element from work list
				At->PrevBro->NextBro = At->NextBro;
				At->NextBro->PrevBro = At->PrevBro;

				//NOTE(dima): Insert element to free list
				At->PrevBro = Generation->FreeTableEntrySentinel;
				At->NextBro = Generation->FreeTableEntrySentinel->NextBro;

				At->PrevBro->NextBro = At;
				At->NextBro->PrevBro = At;

				//NOTE(dima): Delete element from hash table
				if (PrevEntry) {
					PrevEntry->NextInHash = At->NextInHash;
				}
				else {
					*FirstEntry = At->NextInHash;
				}

				At->NextInHash = 0;

				break;
			}
		}

		PrevEntry = At;
		At = At->NextInHash;
	}
}

static voxel_chunk_info* VoxelFindChunk(
	voxworld_generation_state* Generation,
	int X, int Y, int Z) 
{
	FUNCTION_TIMING();

	voxel_chunk_info* Result = 0;

	u32 Key = GetKeyFromIndices(X, Y, Z);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	voxworld_table_entry* FirstEntry = Generation->HashTable[InTableIndex];
	
	voxworld_table_entry* At = FirstEntry;

	while (At != 0) {
		if (At->Key == Key) {
			//NOTE(dima): Important to have additional check here!!!
			//NOTE(dima): Because of hash function might overlap with others chunks
			if (At->ValueChunk->IndexX == X &&
				At->ValueChunk->IndexY == Y &&
				At->ValueChunk->IndexZ == Z)
			{
				Result = At->ValueChunk;
				break;
			}
		}

		At = At->NextInHash;
	}

	return(Result);
}

static void VoxelRegenerateSetatistics(
	voxworld_generation_state* Generation,
	v3 CameraP) 
{
	voxel_generation_statistics* Result = &Generation->DEBUGStat;

	Result->HashTableCollisionCount = Generation->HashTableCollisionCount;
	Result->HashTableInsertedElements = Generation->HashTableTotalInsertedEntries;

	Result->FreeWorkThreadworksCount = Generation->FreeWorkThreadworksCount;
	Result->TotalWorkThreadworksCount = Generation->TotalWorkThreadworksCount;

	Result->FreeGenThreadworksCount = Generation->FreeGenThreadworksCount;
	Result->TotalGenThreadworksCount = Generation->TotalGenThreadworksCount;

	Result->ChunksViewDistance = Generation->ChunksViewDistance;
	Result->BlocksViewDistance = Generation->ChunksViewDistance * VOXEL_CHUNK_WIDTH;

	Result->CameraPos = CameraP;

	Result->Queue = PlatformApi.VoxelQueue;

	GetVoxelChunkPosForCamera(
		CameraP,
		&Result->CurrentChunkX,
		&Result->CurrentChunkY,
		&Result->CurrentCHunkZ);

	Result->MeshGenerationsStartedThisFrame = Generation->MeshGenerationsStartedThisFrame;
	Generation->MeshGenerationsStartedThisFrame = 0;

	Result->ChunksPushedToRender = Generation->ChunksPushedToRender;
	Generation->ChunksPushedToRender = 0;

	Result->TrianglesPushed = Generation->TrianglesPushed;
	Result->TrianglesLoaded = Generation->TrianglesLoaded;
	Generation->TrianglesLoaded = 0;
	Generation->TrianglesPushed = 0;

	Result->HashTableMemUsed = Generation->HashTableMemUsed;
	Result->GenTasksMemUsed = Generation->GenTasksMemUsed;
	Result->WorkTasksMemUsed = Generation->WorkTasksMemUsed;
	Result->MeshTasksMemUsed = Generation->MeshTasksMemUsed;
	Result->GenerationMem = Generation->TotalMemory;
}

struct generate_voxel_mesh_data {
	voxel_chunk_info* Chunk;
	voxel_atlas_info* VoxelAtlasInfo;

	voxworld_threadwork* MeshGenThreadwork;

	voxworld_generation_state* Generation;
};

PLATFORM_THREADWORK_CALLBACK(GenerateVoxelMeshThreadwork) {
	generate_voxel_mesh_data* GenData = (generate_voxel_mesh_data*)Data;

	voxel_chunk_info* ChunkInfo = GenData->Chunk;
	voxel_mesh_info* MeshInfo = &GenData->Chunk->MeshInfo;
	voxworld_generation_state* Generation = GenData->Generation;

	BeginOrderMutex(&MeshInfo->MeshUseMutex);

	if (PlatformApi.AtomicCAS_U32(
		&MeshInfo->State,
		VoxelMeshState_InProcess,
		VoxelMeshState_None))
	{
		Assert(MeshInfo->State == VoxelMeshState_InProcess);

		//TODO(dima): Better memory management here
#if USE_STD_VECTOR_FOR_VOXEL_MESH
		MeshInfo->Vertices = std::vector<u32>();
		//MeshInfo->Vertices.reserve(65536);
		VoxmeshGenerate(MeshInfo, ChunkInfo, GenData->VoxelAtlasInfo);
#else
		voxworld_threadwork* MeshThreadwork = VoxelBeginThreadwork(
			Generation->MeshFreeSentinel,
			Generation->MeshUseSentinel,
			&Generation->MeshMutex,
			&Generation->FreeMeshThreadworks);

		voxel_mesh_info TempMeshInfo = {};
		TempMeshInfo.Vertices = (voxel_vert_t*)MeshThreadwork->Memory.BaseAddress;

		//NOTE(dima): Generate mesh to temp buffer
		VoxmeshGenerate(&TempMeshInfo, ChunkInfo, GenData->VoxelAtlasInfo);

		//NOTE(dima): Allocated buffer with needed size
		int SizeForNewMesh = TempMeshInfo.VerticesCount * VOXEL_VERTEX_SIZE;
		MeshInfo->Vertices = (voxel_vert_t*)malloc(SizeForNewMesh);
		MeshInfo->VerticesCount = TempMeshInfo.VerticesCount;

		//NOTE(dima): Copy generated mesh into new buffer
		CopyMemory(MeshInfo->Vertices, TempMeshInfo.Vertices, SizeForNewMesh);

		VoxelEndThreadwork(
			MeshThreadwork,
			Generation->MeshFreeSentinel,
			&Generation->MeshMutex,
			&Generation->FreeMeshThreadworks);
#endif

		PlatformApi.WriteBarrier();

		PlatformApi.AtomicSet_U32(&MeshInfo->State, VoxelMeshState_Generated);

		PlatformApi.AtomicInc_I32(&GenData->Generation->MeshGenerationsStartedThisFrame);		
	}

	EndOrderMutex(&MeshInfo->MeshUseMutex);

	VoxelEndThreadwork(
		GenData->MeshGenThreadwork,
		GenData->Generation->GenFreeSentinel,
		&GenData->Generation->GenMutex,
		&GenData->Generation->FreeGenThreadworksCount);
}

struct generate_voxel_chunk_data {
	//NOTE(dima): Used to store temporary generation work data while generating chunk
	voxworld_threadwork* ChunkGenThreadwork;

	voxworld_generation_state* Generation;

	voxel_chunk_info* Chunk;
	voxel_atlas_info* VoxelAtlasInfo;
};

struct unload_voxel_chunk_data {
	voxworld_generation_state* Generation;
	
	voxel_chunk_info* Chunk;

	voxworld_threadwork* ChunkUnloadThreadwork;
};

PLATFORM_THREADWORK_CALLBACK(GenerateVoxelChunkThreadwork) {
	generate_voxel_chunk_data* GenData = (generate_voxel_chunk_data*)Data;

	voxel_chunk_info* WorkChunk = GenData->Chunk;

	if (PlatformApi.AtomicCAS_U32(
		&WorkChunk->State,
		VoxelChunkState_InProcess,
		VoxelChunkState_None))
	{
		GenerateRandomChunk(WorkChunk);

		PlatformApi.WriteBarrier();

		PlatformApi.AtomicSet_U32(&WorkChunk->State, VoxelChunkState_Ready);

		voxworld_threadwork* MeshGenThreadwork = VoxelBeginThreadwork(
			GenData->Generation->GenFreeSentinel,
			GenData->Generation->GenUseSentinel,
			&GenData->Generation->GenMutex,
			&GenData->Generation->FreeGenThreadworksCount);

		Assert(MeshGenThreadwork);

		generate_voxel_mesh_data* MeshGenerationData = PushStruct(
			&MeshGenThreadwork->Memory,
			generate_voxel_mesh_data);

		MeshGenerationData->Chunk = WorkChunk;
		MeshGenerationData->Generation = GenData->Generation;
		MeshGenerationData->MeshGenThreadwork = MeshGenThreadwork;
		MeshGenerationData->VoxelAtlasInfo = GenData->VoxelAtlasInfo;

		CopyStrings(MeshGenThreadwork->DEBUGData, "MeshGeneration");

		PlatformApi.AddThreadworkEntry(
			PlatformApi.VoxelQueue,
			MeshGenerationData,
			GenerateVoxelMeshThreadwork);
	}

	VoxelEndThreadwork(
		GenData->ChunkGenThreadwork,
		GenData->Generation->GenFreeSentinel,
		&GenData->Generation->GenMutex,
		&GenData->Generation->FreeGenThreadworksCount);
}

PLATFORM_THREADWORK_CALLBACK(UnloadVoxelChunkThreadwork) {
	unload_voxel_chunk_data* UnloadData = (unload_voxel_chunk_data*)Data;

	voxel_chunk_info* WorkChunk = UnloadData->Chunk;

	//TODO(dima): Deallocate mesh from VAO in opengl
	//TODO(dima): Fix waiting for mesh bug??? IMPORTANT

	while (WorkChunk->State == VoxelChunkState_InProcess) {
		int a = 1;
	}

	u32 TempChunkState = WorkChunk->State;

	//NOTE(dima): Chunk is out of range and should be deallocated
	if (PlatformApi.AtomicCAS_U32(
		&WorkChunk->State,
		VoxelChunkState_None,
		VoxelChunkState_Ready))
	{
		voxel_mesh_info* MeshInfo = &WorkChunk->MeshInfo;

		//NOTE(dima): Infinite loop to wait for the mesh to become generated
		while (MeshInfo->State == VoxelMeshState_InProcess) {
			/*NOTE(dima): Reaching this point means that
			when we are about to unload the chunk the mesh
			is still generating..

			So what I do here is that I just wait until
			mesh becomes generated.
			*/
			int a = 1;
		}

		//Assert(MeshInfo->State == VoxelMeshState_Generated);

		BeginOrderMutex(&MeshInfo->MeshUseMutex);
		
		if (MeshInfo->State == VoxelMeshState_Generated) {
#if USE_STD_VECTOR_FOR_VOXEL_MESH
			MeshInfo->Vertices.clear();
#else
			if (MeshInfo->Vertices) {
				free(MeshInfo->Vertices);
			}
			else {
				int a = 1;
			}
			MeshInfo->Vertices = 0;
#endif
			MeshInfo->VerticesCount = 0;

			dealloc_queue_entry* AllocEntry = PlatformRequestDeallocEntry();
			AllocEntry->EntryType = DeallocQueueEntry_VoxelMesh;
			AllocEntry->Data.VoxelMeshData.MeshInfo = MeshInfo;
			PlatformInsertDellocEntry(AllocEntry);
		}

		PlatformApi.WriteBarrier();
		PlatformApi.AtomicSet_U32(&MeshInfo->State, VoxelMeshState_Unloaded);

		EndOrderMutex(&MeshInfo->MeshUseMutex);

		//NOTE(dima): Close threadwork that contains chunk data
		VoxelEndThreadwork(
			WorkChunk->Threadwork,
			UnloadData->Generation->WorkFreeSentinel,
			&UnloadData->Generation->WorkMutex,
			&UnloadData->Generation->FreeWorkThreadworksCount);
	}
	else {
		Assert(!"Chunk state must be READY");
	}

	//NOTE(dima): Close threadwork that contains data for this function(thread)
	VoxelEndThreadwork(
		UnloadData->ChunkUnloadThreadwork,
		UnloadData->Generation->GenFreeSentinel,
		&UnloadData->Generation->GenMutex,
		&UnloadData->Generation->FreeGenThreadworksCount);
}

void VoxelChunksGenerationInit(
	voxworld_generation_state* Generation,
	stacked_memory* Memory,
	int ChunksViewDistanceCount,
	int VoxelThreadQueueSize)
{
	int TotalChunksSideCount = (ChunksViewDistanceCount * 2 + 1);
	int TotalChunksCount = TotalChunksSideCount * TotalChunksSideCount;

	Generation->ChunksViewDistance = ChunksViewDistanceCount;
	Generation->ChunksSideCount = TotalChunksSideCount;
	Generation->ChunksCount = TotalChunksCount;
	
	Generation->ChunksPushedToRender = 0;

	Generation->MeshGenerationsStartedThisFrame = 0;

	Generation->TotalMemory = Memory;

	/* 
		NOTE(dima): Initialization of mesh threadworks.
		They are used to store mesh data for generating mesh.
	*/
	Generation->MeshMutex = {};
	Generation->MeshTasksMemUsed = 0;

	Generation->MeshUseSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->MeshTasksMemUsed);

	Generation->MeshFreeSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->MeshTasksMemUsed);

	Generation->FreeMeshThreadworks = VoxelThreadQueueSize;
	Generation->TotalMeshThreadworks = VoxelThreadQueueSize;

	/*
		NOTE(dima): 65536 possible cubes in chunk by 6 sides
		by 6 verts per side. The worst case will be when every
		second cube is filled so I divided by 2 here
	*/
	int SizeForOneMeshThreadwork = 65536 * 6 * 6 * 4 / 2;

	for (int MeshThreadworkIndex = 0;
		MeshThreadworkIndex < VoxelThreadQueueSize;
		MeshThreadworkIndex++)
	{
		voxworld_threadwork* Threadwork = VoxelAllocateThreadwork(
			Generation->TotalMemory,
			SizeForOneMeshThreadwork,
			&Generation->MeshTasksMemUsed);

		VoxelInsertThreadworkAfter(Threadwork, Generation->MeshFreeSentinel);
	}

	/*
		NOTE(dima): Initialization of work threadworks.
		They are used to store loaded chunk data;
	*/
	Generation->WorkMutex = {};
	Generation->WorkTasksMemUsed = 0;

	Generation->WorkUseSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->WorkTasksMemUsed);

	Generation->WorkFreeSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->WorkTasksMemUsed);

	Generation->FreeWorkThreadworksCount = TotalChunksCount;
	Generation->TotalWorkThreadworksCount = TotalChunksCount;

	int SizeForOneWorkThreadwork = KILOBYTES(70);

	for (int NewWorkIndex = 0;
		NewWorkIndex < TotalChunksCount;
		NewWorkIndex++) 
	{
		voxworld_threadwork* NewThreadwork = 
			VoxelAllocateThreadwork(
				Generation->TotalMemory, 
				SizeForOneWorkThreadwork,
				&Generation->WorkTasksMemUsed);
		VoxelInsertThreadworkAfter(NewThreadwork, Generation->WorkFreeSentinel);
	}

	/*
		NOTE(dima): Initialization of generation threadworks.
		They are used to store temporary data( for chunk
		generation threads.
	*/
	int GenThreadworksCount = 10000;

	Generation->GenMutex = {};

	Generation->GenUseSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->GenTasksMemUsed);

	Generation->GenFreeSentinel = VoxelAllocateThreadwork(
		Generation->TotalMemory, 0,
		&Generation->GenTasksMemUsed);

	Generation->FreeGenThreadworksCount = GenThreadworksCount;
	Generation->TotalGenThreadworksCount = GenThreadworksCount;

	int SizeForGenThreadwork = Max(
		sizeof(generate_voxel_chunk_data), 
		sizeof(generate_voxel_mesh_data));

	SizeForGenThreadwork = Max(SizeForGenThreadwork, sizeof(unload_voxel_chunk_data));

	SizeForGenThreadwork += 16;

	Generation->GenTasksMemUsed = 0;
	for (int GenThreadworkIndex = 0;
		GenThreadworkIndex < GenThreadworksCount;
		GenThreadworkIndex++)
	{
		//NOTE(dima): 16 bytes added here because of the alignment problems that may arrive
		voxworld_threadwork* NewThreadwork =
			VoxelAllocateThreadwork(
				Generation->TotalMemory, 
				SizeForGenThreadwork,
				&Generation->GenTasksMemUsed);
		VoxelInsertThreadworkAfter(NewThreadwork, Generation->GenFreeSentinel);
	}

	//NOTE(dima): Initializing of world chunks hash table
	for (int EntryIndex = 0;
		EntryIndex < VOXWORLD_TABLE_SIZE;
		EntryIndex++)
	{
		Generation->HashTable[EntryIndex] = 0;
	}

	Generation->HashTableCollisionCount = 0;
	Generation->HashTableTotalInsertedEntries = 0;
	Generation->HashTableMemUsed = 0;

	//NOTE(dima): Initialization of free sentinel for table entries
	Generation->FreeTableEntrySentinel = PushStruct(Generation->TotalMemory, voxworld_table_entry);
	Generation->FreeTableEntrySentinel->NextBro = Generation->FreeTableEntrySentinel;
	Generation->FreeTableEntrySentinel->PrevBro = Generation->FreeTableEntrySentinel;
	Generation->HashTableMemUsed += sizeof(voxworld_table_entry);

	//NOTE(dima): Initialization of work sentinel for table entries
	Generation->WorkTableEntrySentinel = PushStruct(Generation->TotalMemory, voxworld_table_entry);
	Generation->WorkTableEntrySentinel->NextBro = Generation->WorkTableEntrySentinel;
	Generation->WorkTableEntrySentinel->PrevBro = Generation->WorkTableEntrySentinel;
	Generation->HashTableMemUsed += sizeof(voxworld_table_entry);
}

void VoxelChunksGenerationUpdate(
	voxworld_generation_state* Generation,
	render_state* RenderState,
	v3 CameraPos)
{
	FUNCTION_TIMING();

	voxel_atlas_id VoxelAtlasID = GetFirstVoxelAtlas(RenderState->AssetSystem, GameAsset_MyVoxelAtlas);
	voxel_atlas_info* VoxelAtlas = GetVoxelAtlasFromID(RenderState->AssetSystem, VoxelAtlasID);
	
	int CamChunkIndexX;
	int CamChunkIndexY;
	int CamChunkIndexZ;

	GetVoxelChunkPosForCamera(CameraPos, &CamChunkIndexX, &CamChunkIndexY, &CamChunkIndexZ);

	int testviewdist = 10;
#if 0
	int MinCellX = -testviewdist;
	int MaxCellX = testviewdist;
	int MinCellZ = -testviewdist;
	int MaxCellZ = testviewdist;
#else
	int MinCellX = CamChunkIndexX - Generation->ChunksViewDistance;
	int MaxCellX = CamChunkIndexX + Generation->ChunksViewDistance;
	int MinCellZ = CamChunkIndexZ - Generation->ChunksViewDistance;
	int MaxCellZ = CamChunkIndexZ + Generation->ChunksViewDistance;
#endif

	BEGIN_TIMING("VoxelCellsWalkaround");
	int CellY = 0;
	for (int CellX = MinCellX; CellX <= MaxCellX; CellX++) {
		for (int CellZ = MinCellZ; CellZ <= MaxCellZ; CellZ++) {
			
			voxel_chunk_info* NeededChunk = VoxelFindChunk(Generation, CellX, CellY, CellZ);

			if (NeededChunk) {
				if (NeededChunk->State == VoxelChunkState_Ready) {
					/*
						NOTE(dima): It was interesting to see this
						but if I delete this check then some meshes
						will be visible partially. This is because
						they wasn't generated at this time and when
						time came to generating VAOs in the renderer
						they were generated partially too. :D
					*/
					if (NeededChunk->MeshInfo.State == VoxelMeshState_Generated) {

						v3 ChunkPos = GetPosForVoxelChunk(NeededChunk);

						//Assert(NeededChunk->MeshInfo.VerticesCount != 0);

						RENDERPushVoxelMesh(
							RenderState,
							&NeededChunk->MeshInfo,
							ChunkPos,
							&VoxelAtlas->Bitmap);

						Generation->ChunksPushedToRender++;
						Generation->TrianglesPushed += NeededChunk->MeshInfo.VerticesCount / 3;
						Generation->TrianglesLoaded += NeededChunk->MeshInfo.VerticesCount / 3;
					}
				}
			}
			else {
				voxworld_threadwork* Threadwork = VoxelBeginThreadwork(
					Generation->WorkFreeSentinel,
					Generation->WorkUseSentinel,
					&Generation->WorkMutex,
					&Generation->FreeWorkThreadworksCount);

				if (Threadwork) {
					voxworld_threadwork* ChunkGenThreadwork = VoxelBeginThreadwork(
						Generation->GenFreeSentinel,
						Generation->GenUseSentinel,
						&Generation->GenMutex,
						&Generation->FreeGenThreadworksCount);

					CopyStrings(ChunkGenThreadwork->DEBUGData, "ChunkGeneration");

					Assert(ChunkGenThreadwork);

					generate_voxel_chunk_data* ChunkGenerationData = PushStruct(
						&ChunkGenThreadwork->Memory, 
						generate_voxel_chunk_data);

					voxel_chunk_info* ChunkInfo = PushStruct(
						&Threadwork->Memory,
						voxel_chunk_info);

					ChunkGenerationData->Chunk = ChunkInfo;
					ChunkGenerationData->Chunk->IndexX = CellX;
					ChunkGenerationData->Chunk->IndexY = CellY;
					ChunkGenerationData->Chunk->IndexZ = CellZ;
					ChunkGenerationData->Chunk->State = VoxelChunkState_None;
					ChunkGenerationData->Chunk->Threadwork = Threadwork;
					ChunkGenerationData->Chunk->MeshInfo = {};

					ChunkGenerationData->VoxelAtlasInfo = VoxelAtlas;
					ChunkGenerationData->Generation = Generation;
					ChunkGenerationData->ChunkGenThreadwork = ChunkGenThreadwork;

					PlatformApi.AddThreadworkEntry(
						PlatformApi.VoxelQueue,
						ChunkGenerationData,
						GenerateVoxelChunkThreadwork);

					VoxelInsertToTable(Generation, ChunkInfo);
				}
			}
		}
	}
	END_TIMING();
	
	BEGIN_TIMING("VoxelListWalkaround");
	for (voxworld_table_entry* At = Generation->WorkTableEntrySentinel->NextBro;
		At != Generation->WorkTableEntrySentinel;)
	{
		voxworld_table_entry* NextTableEntry = At->NextBro;

		voxel_chunk_info* NeededChunk = At->ValueChunk;

#if 1
		if (NeededChunk->IndexX < MinCellX || NeededChunk->IndexX > MaxCellX ||
			NeededChunk->IndexZ < MinCellZ || NeededChunk->IndexZ > MaxCellZ)
		{
			voxworld_threadwork* UnloadThreadwork = VoxelBeginThreadwork(
				Generation->GenFreeSentinel, 
				Generation->GenUseSentinel,
				&Generation->GenMutex,
				&Generation->FreeGenThreadworksCount);

			Assert(UnloadThreadwork);

			CopyStrings(UnloadThreadwork->DEBUGData, "UnloadThreadwork");

			unload_voxel_chunk_data* UnloadData = PushStruct(
				&UnloadThreadwork->Memory,
				unload_voxel_chunk_data);

			UnloadData->Chunk = NeededChunk;
			UnloadData->ChunkUnloadThreadwork = UnloadThreadwork;
			UnloadData->Generation = Generation;

			PlatformApi.AddThreadworkEntry(
				PlatformApi.VoxelQueue,
				UnloadData,
				UnloadVoxelChunkThreadwork);

			voxel_mesh_info* MeshInfo = &NeededChunk->MeshInfo;

			VoxelDeleteFromTable(
				Generation,
				NeededChunk->IndexX,
				NeededChunk->IndexY,
				NeededChunk->IndexZ);
		}
#endif

		At = NextTableEntry;
	}
	END_TIMING();

	VoxelRegenerateSetatistics(Generation, CameraPos);

	BEGIN_SECTION("VoxelGeneration");
	DEBUG_VOXEL_STATISTICS(&Generation->DEBUGStat);
	END_SECTION();
}
