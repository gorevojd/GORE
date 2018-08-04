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
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x - 0), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y - 0), (float)(Pos.z - 0)),
		V3((float)(Pos.x - 0), (float)(Pos.y - 0), (float)(Pos.z - 0)),
		TextureIndexInAtlas,
		VoxelNormalIndex_Front);
}

static inline void WriteFaceAtBack(
	voxel_mesh_info* Mesh,
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		V3((float)(Pos.x - 0), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		V3((float)(Pos.x - 0), (float)(Pos.y - 0), (float)(Pos.z + 1)),
		V3((float)(Pos.x + 1), (float)(Pos.y - 0), (float)(Pos.z + 1)),
		TextureIndexInAtlas,
		VoxelNormalIndex_Back);
}

static inline void WriteFaceAtLeft(
	voxel_mesh_info* Mesh,
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		V3((float)(Pos.x), (float)(Pos.y - 0), (float)(Pos.z + 1)),
		V3((float)(Pos.x), (float)(Pos.y - 0), (float)(Pos.z - 0)),
		TextureIndexInAtlas,
		VoxelNormalIndex_Left);
}

static inline void WriteFaceAtRight(
	voxel_mesh_info* Mesh,
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y - 0), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y - 0), (float)(Pos.z + 1)),
		TextureIndexInAtlas,
		VoxelNormalIndex_Right);
}

static inline void WriteFaceAtTop(
	voxel_mesh_info* Mesh,
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x - 0), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z - 0)),
		V3((float)(Pos.x + 1), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		V3((float)(Pos.x - 0), (float)(Pos.y + 1), (float)(Pos.z + 1)),
		TextureIndexInAtlas,
		VoxelNormalIndex_Up);
}

static inline void WriteFaceAtBottom(
	voxel_mesh_info* Mesh,
	iv3 Pos,
	u8 TextureIndexInAtlas)
{
	VoxmeshWriteFace(
		Mesh,
		V3((float)(Pos.x - 0), (float)(Pos.y), (float)(Pos.z + 1)),
		V3((float)(Pos.x + 1), (float)(Pos.y), (float)(Pos.z + 1)),
		V3((float)(Pos.x + 1), (float)(Pos.y), (float)(Pos.z - 0)),
		V3((float)(Pos.x - 0), (float)(Pos.y), (float)(Pos.z - 0)),
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

struct voxmesh_generate_data {
	voxel_chunk_info* Chunk;
	voxel_atlas_info* Atlas;
	
	neighbours_set_info* NeighboursSetInfo;
};

void VoxmeshGenerate(
	voxel_mesh_info* Result,
	voxmesh_generate_data* Data)
{
	voxel_chunk_info* Chunk = Data->Chunk;
	voxel_atlas_info* Atlas = Data->Atlas;

	Result->MeshHandle = 0;
	Result->VerticesCount = 0;

	for (int DepthIndex = 0; DepthIndex < VOXEL_CHUNK_WIDTH; DepthIndex++) {
		for (int WidthIndex = 0; WidthIndex < VOXEL_CHUNK_WIDTH; WidthIndex++) {
			for (int HeightIndex = 0; HeightIndex < VOXEL_CHUNK_HEIGHT; HeightIndex++) {

				u8 ToCheck = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex)];

				iv3 VoxelPos;
				VoxelPos.x = WidthIndex;
				VoxelPos.y = HeightIndex;
				VoxelPos.z = DepthIndex;

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
							if (Data->NeighboursSetInfo->LeftExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Left,
									(VOXEL_CHUNK_WIDTH - 1 - DepthIndex) + (HeightIndex * VOXEL_CHUNK_WIDTH)))
								{
									WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
								}
							}
							else {
								//WriteFaceAtLeft(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Left]);
							}
						}
						if (WidthIndex == (VOXEL_CHUNK_WIDTH - 1)) {
							if (Data->NeighboursSetInfo->RightExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Right,
									DepthIndex + HeightIndex * VOXEL_CHUNK_WIDTH))
								{
									WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
								}
							}
							else {
								//WriteFaceAtRight(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Right]);
							}
						}
						if (DepthIndex == 0) {
							if (Data->NeighboursSetInfo->FrontExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Front,
									(VOXEL_CHUNK_WIDTH - 1 - WidthIndex) + HeightIndex * VOXEL_CHUNK_WIDTH))
								{
									WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
								}
							}
							else {
								//WriteFaceAtFront(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Front]);
							}
						}

						if (DepthIndex == (VOXEL_CHUNK_WIDTH - 1)) {
							if (Data->NeighboursSetInfo->BackExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Back,
									WidthIndex + HeightIndex * VOXEL_CHUNK_WIDTH))
								{
									WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
								}
							}
							else {
								//WriteFaceAtBack(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Back]);
							}
						}

						if (HeightIndex == 0) {
							if (Data->NeighboursSetInfo->BottomExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Bottom,
									WidthIndex + DepthIndex * VOXEL_CHUNK_WIDTH)) 
								{
									WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
								}
							}
							else {
								//WriteFaceAtBottom(Result, VoxelPos, TexSet->Sets[VoxelFaceTypeIndex_Bottom]);
							}
						}

						if (HeightIndex == (VOXEL_CHUNK_HEIGHT - 1)) {
							if (Data->NeighboursSetInfo->TopExist) {
								if (!IsVoxelSetInNeighbour(
									Data->NeighboursSetInfo->Top,
									WidthIndex + DepthIndex * VOXEL_CHUNK_WIDTH))
								{
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

static void BuildColumnInChunk(
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

	int TestStartHeight = 100;

	for (int j = 0; j < VOXEL_CHUNK_WIDTH; j++) {
		for (int i = 0; i < VOXEL_CHUNK_WIDTH; i++) {
			int SetHeightIndex = TestStartHeight + i + j;
			Chunk->Voxels[GET_VOXEL_INDEX(i, j, SetHeightIndex)] = VoxelMaterial_GrassyGround;

			BuildColumnInChunk(Chunk, i, j, 0, SetHeightIndex - 1, VoxelMaterial_Ground);
		}
	}
}

inline float GetNextRandomSmoothFloat(voxworld_generation_state* Generation) {
	float Result = VoxelSmoothRandoms[Generation->SmoothRandomIndex % ArrayCount(VoxelSmoothRandoms)];

	PlatformApi.AtomicAdd_U32((platform_atomic_type_u32*)&Generation->SmoothRandomIndex, 1);

	return(Result);
}

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

struct biome_based_params {
	int TreeMinHeight;
	int TreeMaxHeight;

	u8 GrassMaterial;
	u8 GroundMaterial;
	u8 TreeMaterial;
	float BiomeNoiseDivisor;
	float BiomeNoiseEffect;
};

inline void FillParametersBasedOnBiome(
	biome_based_params* Params, 
	float BiomeNoise,
	float InitBiomeNoiseDivisor) 
{
	if (BiomeNoise < 0.5f && BiomeNoise >= 0.0f) {
		Params->GrassMaterial = VoxelMaterial_GrassyGround;
		Params->GroundMaterial = VoxelMaterial_Ground;
		Params->TreeMaterial = VoxelMaterial_Birch;
		Params->BiomeNoiseDivisor = InitBiomeNoiseDivisor;

		Params->BiomeNoiseEffect = 0.5f;

		Params->TreeMinHeight = 6;
		Params->TreeMinHeight = 9;
	}
	else if (BiomeNoise >= 0.5f && BiomeNoise <= 1.0f) {
		Params->GrassMaterial = VoxelMaterial_SnowGround;
		Params->GroundMaterial = VoxelMaterial_WinterGround;
		Params->TreeMaterial = VoxelMaterial_Tree;
		//BiomeNoiseDivisor = 512.0f;
		Params->BiomeNoiseDivisor = InitBiomeNoiseDivisor / 2;

		Params->BiomeNoiseEffect = 1.0f;

		Params->TreeMinHeight = 8;
		Params->TreeMaxHeight = 17;
	}
	else {
		Assert(!"INVALID");
	}
}

inline float CalculateHeightForBiomeParams(
	biome_based_params* Params,
	v3 ChunkPos,
	int i, int j,
	int StartHeight,
	float* OutNoise = 0) 
{
	int Octaves = 6;
	float Lacunarity = 2.0f;
	float Gain = 0.5f;

#if 1
	float Noise = stb_perlin_fbm_noise3(
		(float)(ChunkPos.x + i) / Params->BiomeNoiseDivisor,
		(float)0.0f,
		(float)(ChunkPos.z + j) / Params->BiomeNoiseDivisor,
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

	float RandHeight = StartHeight + Noise * Params->BiomeNoiseEffect * 120.0f;

	if (OutNoise) {
		*OutNoise = Noise;
	}

	return(RandHeight);
}

void GenerateRandomChunk(voxel_chunk_info* Chunk, voxworld_generation_state* Generation) {

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
#if 1

			int Octaves = 6;
			float Lacunarity = 2.0f;
			float Gain = 0.5f;

			float BiomeNoiseDivisor = 1000.0f;

			biome_based_params BiomeParams = {};
			BiomeParams.TreeMinHeight = 6;
			BiomeParams.TreeMaxHeight = 9;
			BiomeParams.GrassMaterial = VoxelMaterial_GrassyGround;
			BiomeParams.GroundMaterial = VoxelMaterial_Ground;
			BiomeParams.TreeMaterial = VoxelMaterial_Birch;
			BiomeParams.BiomeNoiseDivisor = BiomeNoiseDivisor / 3;
			BiomeParams.BiomeNoiseEffect = 1.0f;

			float Noise;
			float RandHeight = CalculateHeightForBiomeParams(
				&BiomeParams,
				ChunkPos,
				i, j,
				StartHeight,
				&Noise);

			int RandHeightInt = (int)(RandHeight + 0.5f);
#if 0
			float BiomeTransitionNoiseScale = 128.0f;
			float BiomeTransitionNoise = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / BiomeTransitionNoiseScale,
				(float)ChunkPos.y / BiomeTransitionNoiseScale,
				(float)(ChunkPos.z + j) / BiomeTransitionNoiseScale,
				0, 0, 0);

			float SmoothRandomValue = GetNextRandomSmoothFloat(Generation);
			float SmoothRandomValueNorm = 0.0025f;

			float TransitionValue = BiomeTransitionNoise * 0.02f;

			TransitionValue += SmoothRandomValue * SmoothRandomValueNorm;

			float BiomeNoise = Noise;
			BiomeNoise = BiomeNoise * 0.5f + 0.5f;

			BiomeNoise += TransitionValue;
			BiomeNoise = Clamp01(BiomeNoise);

			FillParametersBasedOnBiome(
				&BiomeParams, 
				BiomeNoise, 
				BiomeNoiseDivisor);
#endif

			int ChunkYRangeMin = VOXEL_CHUNK_HEIGHT * Chunk->IndexY;
			int ChunkYRangeMax = VOXEL_CHUNK_HEIGHT * (Chunk->IndexY + 1);

			int SetHeight = RandHeightInt - Chunk->IndexY * VOXEL_CHUNK_HEIGHT;
			if (RandHeightInt >= ChunkYRangeMin && RandHeightInt < ChunkYRangeMax) {

				SetHeight = Clamp(SetHeight, 0, VOXEL_CHUNK_HEIGHT - 1);
				Chunk->Voxels[GET_VOXEL_INDEX(i, j, SetHeight)] = BiomeParams.GrassMaterial;
				if (SetHeight - 1 > 0) {
					BuildColumnInChunk(Chunk, i, j, 0, SetHeight - 1, BiomeParams.GroundMaterial);
				}
			}
			else if (RandHeightInt >= ChunkYRangeMax) {
				BuildColumnInChunk(Chunk, i, j, 0, VOXEL_CHUNK_HEIGHT - 1, BiomeParams.GroundMaterial);
			}

			if (i == 8 && j == 8) {
				float NextRandom = GetNextRandomSmoothFloat(Generation);
				NextRandom = NextRandom * 0.5f + 0.5f;

				int RandomTreeHeight = (int)((float)BiomeParams.TreeMinHeight + (float)(BiomeParams.TreeMaxHeight - BiomeParams.TreeMinHeight) * NextRandom);

				BuildColumnInChunk(Chunk, 8, 8, SetHeight, SetHeight + RandomTreeHeight - 1, BiomeParams.TreeMaterial);
			}
#else
			BuildColumnInChunk(Chunk, i, j, 0, GetNextRandomSmoothFloat(Generation) * 2.5f + 100, VoxelMaterial_Stone);
#endif
		}
	}
}

static inline b32 VoxelChunkNeighboursChanged(
	voxel_chunk_info* Chunk, 
	neighbours_chunks* Neighbours) 
{
#if 1
	//NOTE(dima): Checking chunks pointers
	for (int ChunkPointerIndex = 0;
		ChunkPointerIndex < 6;
		ChunkPointerIndex++)
	{
		if (Chunk->OldNeighbours.ChunksPointers[ChunkPointerIndex] != 
			Neighbours->ChunksPointers[ChunkPointerIndex]) 
		{
			return(1);
		}
	}

	//NOTE(dima): Checking chunk states
	for (int ChunkStateIndex = 0;
		ChunkStateIndex < 6;
		ChunkStateIndex++) 
	{
		if (Neighbours->ChunksStates[ChunkStateIndex] == VoxelChunkState_Ready) {
			if (Neighbours->ChunksStates[ChunkStateIndex] !=
				Chunk->OldNeighbours.ChunksStates[ChunkStateIndex])
			{
				return(1);
			}
		}
	}
#endif

	return(0);
}

static void FillNeighbourSetBasedOnChunk(
	u8* Array, b32* Exist,
	voxel_chunk_info* Chunk,
	int WidthStartIndex, int WidthEndIndex,
	int DepthStartIndex, int DepthEndIndex,
	int HeightStartIndex, int HeightEndIndex)
{
	if (Chunk) {
		if (Chunk->State == VoxelChunkState_Ready) {
			*Exist = 1;

			int WidthInc = 1;
			if (WidthStartIndex > WidthEndIndex) {
				WidthInc = -1;
			}

			int DepthInc = 1;
			if (DepthStartIndex > DepthEndIndex) {
				DepthInc = -1;
			}

			int HeightInc = 1;
			if (HeightStartIndex > HeightEndIndex) {
				HeightInc = -1;
			}

			int CurrentBit = 0;
			for (int HeightIndex = HeightStartIndex;
				; HeightIndex += HeightInc)
			{
				for (int DepthIndex = DepthStartIndex;
					; DepthIndex += DepthInc)
				{
					for (int WidthIndex = WidthStartIndex;
						; WidthIndex += WidthInc)
					{
						u8 Src = Chunk->Voxels[GET_VOXEL_INDEX(WidthIndex, DepthIndex, HeightIndex)];

						int TargetByte = CurrentBit / 8;
						int TargetBit = CurrentBit & 7;

						Array[TargetByte] = ((1 & (Src != 0)) << TargetBit) | Array[TargetByte];

						CurrentBit++;

						if (WidthIndex == WidthEndIndex) {
							break;
						}
					}

					if (DepthIndex == DepthEndIndex) {
						break;
					}
				}

				if (HeightIndex == HeightEndIndex) {
					break;
				}
			}
		}
	}
}

void FillNeighboursSetInfoBasedOnNeighbours(
	neighbours_set_info* SetInfo,
	neighbours_chunks* Neighbours)
{
	*SetInfo = {};

#if 0
	//NOTE(dima): This is for left chunk
	if (Neighbours->LeftChunk) {
		if (Neighbours->LeftChunk->State == VoxelChunkState_Ready) {
			SetInfo->LeftExist = 1;

			int CurrentBit = 0;
			for (int DepthIndex = VOXEL_CHUNK_WIDTH - 1;
				DepthIndex >= 0;
				DepthIndex--)
			{
				for (int HeightIndex = 0;
					HeightIndex < VOXEL_CHUNK_HEIGHT;
					HeightIndex++)
				{
					u8 Src = Neighbours->LeftChunk->Voxels[GET_VOXEL_INDEX(VOXEL_CHUNK_WIDTH - 1, DepthIndex, HeightIndex)];
					
					int TargetByte = CurrentBit / 8;
					int TargetBit = CurrentBit & 7;

					SetInfo->Left[TargetByte] = ((1 & (Src != 0)) << TargetBit) | SetInfo->Left[TargetByte];

					CurrentBit++;
				}
			}
		}
	}

	//NOTE(dima): This is for right chunk
	if (Neighbours->RightChunk) {
		if (Neighbours->RightChunk->State == VoxelChunkState_Ready) {
			SetInfo->RightExist = 1;

			int CurrentBit = 0;
			for (int DepthIndex = 0;
				DepthIndex < VOXEL_CHUNK_WIDTH;
				DepthIndex++)
			{
				for (int HeightIndex = 0;
					HeightIndex < VOXEL_CHUNK_HEIGHT;
					HeightIndex++)
				{
					u8 Src = Neighbours->RightChunk->Voxels[GET_VOXEL_INDEX(0, DepthIndex, HeightIndex)];

					int TargetByte = CurrentBit / 8;
					int TargetBit = CurrentBit & 7;

					SetInfo->Right[TargetByte] = ((1 & (Src != 0)) << TargetBit) | SetInfo->Right[TargetByte];

					CurrentBit++;
				}
			}
		}
	}
#else
	FillNeighbourSetBasedOnChunk(
		SetInfo->Left, &SetInfo->LeftExist,
		Neighbours->LeftChunk,
		VOXEL_CHUNK_WIDTH - 1, VOXEL_CHUNK_WIDTH - 1,
		VOXEL_CHUNK_WIDTH - 1, 0,
		0, VOXEL_CHUNK_HEIGHT - 1);

	FillNeighbourSetBasedOnChunk(
		SetInfo->Right, &SetInfo->RightExist,
		Neighbours->RightChunk,
		0, 0,
		0, VOXEL_CHUNK_WIDTH - 1,
		0, VOXEL_CHUNK_HEIGHT - 1);

	FillNeighbourSetBasedOnChunk(
		SetInfo->Front, &SetInfo->FrontExist,
		Neighbours->FrontChunk,
		VOXEL_CHUNK_WIDTH - 1, 0,
		VOXEL_CHUNK_WIDTH - 1, VOXEL_CHUNK_WIDTH - 1,
		0, VOXEL_CHUNK_HEIGHT - 1);

	FillNeighbourSetBasedOnChunk(
		SetInfo->Back, &SetInfo->BackExist,
		Neighbours->BackChunk,
		0, VOXEL_CHUNK_WIDTH - 1,
		0, 0,
		0, VOXEL_CHUNK_HEIGHT - 1);

	FillNeighbourSetBasedOnChunk(
		SetInfo->Top, &SetInfo->TopExist,
		Neighbours->TopChunk,
		0, VOXEL_CHUNK_WIDTH - 1,
		0, VOXEL_CHUNK_WIDTH - 1,
		0, 0);

	FillNeighbourSetBasedOnChunk(
		SetInfo->Bottom, &SetInfo->BottomExist,
		Neighbours->BottomChunk,
		0, VOXEL_CHUNK_WIDTH - 1,
		0, VOXEL_CHUNK_WIDTH - 1,
		VOXEL_CHUNK_HEIGHT - 1, VOXEL_CHUNK_HEIGHT - 1);
#endif
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



inline u32 GetKeyFromIndices(int X, int Y, int Z) {
#if 0
	char KeyStr[64];
	stbsp_sprintf(KeyStr, "%d|%d|%d", X, Y, Z);
	u32 Result = StringHashFNV(KeyStr);
#else
	u32 Result = 0xAAAAAAAA;
	Result = Result * 1000003351 ^ (X + 16777619);
	Result = Result * 1000010233 ^ (Y + 16777619);
	Result = Result * 1000008899 ^ (Z + 16777619);
#endif

	return(Result);
}

static void VoxelInsertToTable(voxworld_generation_state* Generation, voxel_chunk_info* Info) 
{
	FUNCTION_TIMING();

	u32 Key = GetKeyFromIndices(Info->IndexX, Info->IndexY, Info->IndexZ);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	BeginMutexAccess(&Generation->HashTableOpMutex);

	voxworld_table_entry** FirstEntry = &Generation->HashTable[InTableIndex];

	if (*FirstEntry) {
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
	NewEntry->NextInHash = *FirstEntry;
	NewEntry->Key = Key;

	*FirstEntry = NewEntry;

	Generation->HashTableTotalInsertedEntries++;

	EndMutexAccess(&Generation->HashTableOpMutex);
}

static void VoxelDeleteFromTable(
	voxworld_generation_state* Generation,
	int X, int Y, int Z)
{
	FUNCTION_TIMING();

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
				BeginMutexAccess(&Generation->HashTableOpMutex);
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
				EndMutexAccess(&Generation->HashTableOpMutex);

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
	//FUNCTION_TIMING();

	/* NOTE(dima): Mutexes deleted from here. They must
	be used in caller code. */

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

static neighbours_chunks VoxelFindNeighboursChunks(
	voxworld_generation_state* Generation,
	int X, int Y, int Z)
{
	neighbours_chunks Result = {};

	Result.LeftChunk = VoxelFindChunk(Generation, X - 1, Y, Z);
	Result.RightChunk = VoxelFindChunk(Generation, X + 1, Y, Z);
	Result.TopChunk = VoxelFindChunk(Generation, X, Y + 1, Z);
	Result.BottomChunk = VoxelFindChunk(Generation, X, Y - 1, Z);
	Result.FrontChunk = VoxelFindChunk(Generation, X, Y, Z - 1);
	Result.BackChunk = VoxelFindChunk(Generation, X, Y, Z + 1);

	if (Result.LeftChunk) {
		Result.LeftChunkState = Result.LeftChunk->State;
	}

	if (Result.RightChunk) {
		Result.RightChunkState = Result.RightChunk->State;
	}

	if (Result.TopChunk) {
		Result.TopChunkState = Result.TopChunk->State;
	}

	if (Result.BottomChunk) {
		Result.BottomChunkState = Result.BottomChunk->State;
	}

	if (Result.FrontChunk) {
		Result.FrontChunkState = Result.FrontChunk->State;
	}

	if (Result.BackChunk) {
		Result.BackChunkState = Result.BackChunk->State;
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

	Result->FreeChunkThreadworksCount = Generation->ChunkSet.FreeThreadworksCount;
	Result->TotalChunkThreadworksCount = Generation->ChunkSet.TotalThreadworksCount;

	Result->FreeGenThreadworksCount = Generation->GenSet.FreeThreadworksCount;
	Result->TotalGenThreadworksCount = Generation->GenSet.TotalThreadworksCount;

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

	Result->ChunksLoaded = Generation->ChunksLoaded;
	Result->ChunksPushed = Generation->ChunksPushed;
	Generation->ChunksPushed = 0;
	Generation->ChunksLoaded = 0;

	Result->TrianglesPushed = Generation->TrianglesPushed;
	Result->TrianglesLoaded = Generation->TrianglesLoaded;
	Generation->TrianglesLoaded = 0;
	Generation->TrianglesPushed = 0;

	Result->HashTableMemUsed = Generation->HashTableMemUsed;
	Result->GenTasksMemUsed = Generation->GenSet.MemUsed;
	Result->ChunkTasksMemUsed = Generation->ChunkSet.MemUsed;
	Result->MeshTasksMemUsed = Generation->MeshSet.MemUsed;
	Result->GenerationMem = Generation->TotalMemory;
}

struct generate_voxel_mesh_threadwork_data {
	voxmesh_generate_data MeshGenerateData;

	voxworld_generation_state* Generation;

	threadwork_data* MeshGenThreadwork;
	threadwork_data* NeighboursSidesThreadwork;
};

static void GenerateMeshInternal(
	voxel_mesh_info* MeshInfo,
	voxmesh_generate_data* MeshGenerateData,
	voxworld_generation_state* Generation) 
{
	//TODO(dima): Better memory management here
	threadwork_data* MeshThreadwork = BeginThreadworkData(&Generation->MeshSet);

	Assert(MeshThreadwork);

	voxel_mesh_info TempMeshInfo = {};
	TempMeshInfo.Vertices = (voxel_vert_t*)MeshThreadwork->Memory.BaseAddress;

	VoxmeshGenerate(&TempMeshInfo, MeshGenerateData);

	//NOTE(dima): Allocated buffer with needed size
	int SizeForNewMesh = TempMeshInfo.VerticesCount * VOXEL_VERTEX_SIZE;
	MeshInfo->Vertices = (voxel_vert_t*)malloc(SizeForNewMesh);
	Assert(MeshInfo->Vertices);

	MeshInfo->VerticesCount = TempMeshInfo.VerticesCount;

	//NOTE(dima): Copy generated mesh into new buffer
	memcpy(MeshInfo->Vertices, TempMeshInfo.Vertices, SizeForNewMesh);

	EndThreadworkData(MeshThreadwork, &Generation->MeshSet);

	PlatformApi.WriteBarrier();

	MeshInfo->State = VoxelMeshState_Generated;
}

static void UnloadMeshInternal(
	voxel_mesh_info* MeshInfo,
	voxworld_generation_state* Generation)
{
	PlatformApi.ReadBarrier();

	Assert(MeshInfo->State == VoxelMeshState_Generated);

	if (MeshInfo->Vertices) {
		free(MeshInfo->Vertices);
	}
	else {
		Assert(!"Vertices should be allocated");
	}

	MeshInfo->Vertices = 0;
	MeshInfo->VerticesCount = 0;

	dealloc_queue_entry* AllocEntry = PlatformRequestDeallocEntry();
	AllocEntry->EntryType = DeallocQueueEntry_VoxelMesh;
	AllocEntry->Data.VoxelMeshData.Handle1 = MeshInfo->MeshHandle;
	AllocEntry->Data.VoxelMeshData.Handle2 = MeshInfo->MeshHandle2;
	PlatformInsertDellocEntry(AllocEntry);

	MeshInfo->MeshHandle = 0;
	MeshInfo->MeshHandle2 = 0;

	PlatformApi.WriteBarrier();
	MeshInfo->State = VoxelMeshState_Unloaded;
}

//NOTE(dima): Mesh generation threadwork
PLATFORM_THREADWORK_CALLBACK(GenerateVoxelMeshThreadwork) {
	generate_voxel_mesh_threadwork_data* GenData = (generate_voxel_mesh_threadwork_data*)Data;

	voxel_chunk_info* ChunkInfo = GenData->MeshGenerateData.Chunk;
	voxel_mesh_info* MeshInfo = &ChunkInfo->MeshInfo;
	voxworld_generation_state* Generation = GenData->Generation;

	if (PlatformApi.AtomicCAS_U32(
		&MeshInfo->State,
		VoxelMeshState_InProcess,
		VoxelMeshState_None))
	{
		BeginMutexAccess(&MeshInfo->MeshUseMutex);
		GenerateMeshInternal(MeshInfo, &GenData->MeshGenerateData, Generation);
		EndMutexAccess(&MeshInfo->MeshUseMutex);

		PlatformApi.AtomicInc_I32(&GenData->Generation->MeshGenerationsStartedThisFrame);		
	}

	EndThreadworkData(GenData->NeighboursSidesThreadwork, &Generation->NeighboursSidesSet);
	EndThreadworkData(GenData->MeshGenThreadwork, &Generation->GenSet);
}

//NOTE(dima): Mesh REgeneration threadwork
PLATFORM_THREADWORK_CALLBACK(RegenerateVoxelMeshThreadwork) {
	generate_voxel_mesh_threadwork_data* GenData = (generate_voxel_mesh_threadwork_data*)Data;

	voxel_chunk_info* ChunkInfo = GenData->MeshGenerateData.Chunk;
	voxel_mesh_info* MeshInfo = &ChunkInfo->MeshInfo;
	voxworld_generation_state* Generation = GenData->Generation;

	PlatformApi.ReadBarrier();

#if 0
	if (MeshInfo->State != VoxelMeshState_None) {
		//NOTE(dima): Infinite loop to wait for the mesh to become generated
		while (MeshInfo->State == VoxelMeshState_InProcess) {
			int a = 1;
		}

		if (MeshInfo->State == VoxelMeshState_Generated) {
			UnloadMeshInternal(MeshInfo, Generation);
		}
	}
	GenerateMeshInternal(MeshInfo, &GenData->MeshGenerateData, Generation);
#else
	if (MeshInfo->State == VoxelMeshState_Generated) {
		BeginMutexAccess(&MeshInfo->MeshUseMutex);
		UnloadMeshInternal(MeshInfo, Generation);
		GenerateMeshInternal(MeshInfo, &GenData->MeshGenerateData, Generation);
		EndMutexAccess(&MeshInfo->MeshUseMutex);
	}
#endif


	EndThreadworkData(GenData->NeighboursSidesThreadwork, &GenData->Generation->NeighboursSidesSet);
	EndThreadworkData(GenData->MeshGenThreadwork, &Generation->GenSet);
}

struct generate_voxel_chunk_data {
	//NOTE(dima): Used to store temporary generation work data while generating chunk
	threadwork_data* ChunkGenThreadwork;

	voxworld_generation_state* Generation;

	voxel_chunk_info* Chunk;
	voxel_atlas_info* VoxelAtlasInfo;
};

struct unload_voxel_chunk_data {
	voxworld_generation_state* Generation;
	
	voxel_chunk_info* Chunk;

	threadwork_data* ChunkUnloadThreadwork;
};

PLATFORM_THREADWORK_CALLBACK(GenerateVoxelChunkThreadwork) {
	generate_voxel_chunk_data* GenData = (generate_voxel_chunk_data*)Data;

	voxel_chunk_info* WorkChunk = GenData->Chunk;

	if (PlatformApi.AtomicCAS_U32(
		&WorkChunk->State,
		VoxelChunkState_InProcess,
		VoxelChunkState_None))
	{
		BeginMutexAccess(&WorkChunk->ChunkUseMutex);
		GenerateRandomChunk(WorkChunk, GenData->Generation);
		EndMutexAccess(&WorkChunk->ChunkUseMutex);

		PlatformApi.WriteBarrier();

		WorkChunk->State = VoxelChunkState_Ready;
		//PlatformApi.AtomicSet_U32(&WorkChunk->State, VoxelChunkState_Ready);

		threadwork_data* MeshGenThreadwork = BeginThreadworkData(&GenData->Generation->GenSet);

		Assert(MeshGenThreadwork);

		generate_voxel_mesh_threadwork_data* MeshGenerationData = PushStruct(
			&MeshGenThreadwork->Memory,
			generate_voxel_mesh_threadwork_data);

		MeshGenerationData->Generation = GenData->Generation;
		MeshGenerationData->MeshGenThreadwork = MeshGenThreadwork;

		MeshGenerationData->MeshGenerateData.Chunk = WorkChunk;
		MeshGenerationData->MeshGenerateData.Atlas = GenData->VoxelAtlasInfo;

		MeshGenerationData->NeighboursSidesThreadwork = BeginThreadworkData(&GenData->Generation->NeighboursSidesSet);
		MeshGenerationData->MeshGenerateData.NeighboursSetInfo = PushStruct(
			&MeshGenerationData->NeighboursSidesThreadwork->Memory,
			neighbours_set_info);

		FillNeighboursSetInfoBasedOnNeighbours(
			MeshGenerationData->MeshGenerateData.NeighboursSetInfo,
			&WorkChunk->OldNeighbours);

		PlatformApi.AddThreadworkEntry(
			PlatformApi.VoxelQueue,
			MeshGenerationData,
			GenerateVoxelMeshThreadwork);
	}

	EndThreadworkData(GenData->ChunkGenThreadwork, &GenData->Generation->GenSet);
}


PLATFORM_THREADWORK_CALLBACK(UnloadVoxelChunkThreadwork) {
	unload_voxel_chunk_data* UnloadData = (unload_voxel_chunk_data*)Data;

	voxel_chunk_info* WorkChunk = UnloadData->Chunk;

	voxel_mesh_info* MeshInfo = &WorkChunk->MeshInfo;

	PlatformApi.ReadBarrier();

	if (MeshInfo->State != VoxelMeshState_None) {

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

		BeginMutexAccess(&MeshInfo->MeshUseMutex);
		UnloadMeshInternal(MeshInfo, UnloadData->Generation);
		EndMutexAccess(&MeshInfo->MeshUseMutex);
	}

	//NOTE(dima): Close threadwork that contains chunk data
	EndThreadworkData(WorkChunk->Threadwork, &UnloadData->Generation->ChunkSet);

	//NOTE(dima): Updating chunk state to unloaded
	PlatformApi.WriteBarrier();
	WorkChunk->State = VoxelChunkState_None;

	//NOTE(dima): Close threadwork that contains data for this function(thread)
	EndThreadworkData(
		UnloadData->ChunkUnloadThreadwork,
		&UnloadData->Generation->GenSet);
}

struct voxel_cell_render_entry {
	voxel_cell_render_entry* Next;

	voxel_mesh_info* MeshInfo;
	v3 Pos;
};

struct voxel_cell_walkaround_threadwork_data {
	int MinX;
	int MinZ;
	int MaxX;
	int MaxZ;
	int MinY;
	int MaxY;

	int TotalMinX;
	int TotalMinZ;
	int TotalMaxX;
	int TotalMaxZ;
	int TotalMinY;
	int TotalMaxY;

	int ChunksSetsXCount;
	int ChunksSetsZCount;
	int ChunksSetsYCount;

	voxworld_generation_state* Generation;
	voxel_atlas_info* VoxelAtlas;

	threadwork_data* Threadwork;

	voxel_cell_render_entry* FirstRenderEntry;

	int TrianglesPushed;
	int TrianglesLoaded;
	int ChunksPushed;
	int ChunksLoaded;

	b32* FCPrecomp;
};

PLATFORM_THREADWORK_CALLBACK(VoxelCellWalkaroundThreadwork) {
	voxel_cell_walkaround_threadwork_data* ThreadworkData =
		(voxel_cell_walkaround_threadwork_data*)Data;

	voxworld_generation_state* Generation = ThreadworkData->Generation;
	voxel_atlas_info* VoxelAtlas = ThreadworkData->VoxelAtlas;

	int MinX = ThreadworkData->MinX;
	int MinY = ThreadworkData->MinY;
	int MinZ = ThreadworkData->MinZ;
	int MaxX = ThreadworkData->MaxX;
	int MaxY = ThreadworkData->MaxY;
	int MaxZ = ThreadworkData->MaxZ;

	int IncrementX = 1;
	int IncrementZ = 1;

	BEGIN_TIMING("VoxelCellsWalkaround");
	for (int CellY = MinY; CellY <= MaxY; CellY++) {
		for (int CellX = MinX; CellX <= MaxX; CellX += IncrementX) {
			for (int CellZ = MinZ; CellZ <= MaxZ; CellZ += IncrementZ) {

				//BEGIN_TIMING("Finding");
				voxel_chunk_info* NeededChunk = VoxelFindChunk(
					ThreadworkData->Generation,
					CellX, CellY, CellZ);
				//END_TIMING();

				if (NeededChunk) {
					PlatformApi.ReadBarrier();
 					if (NeededChunk->State == VoxelChunkState_Ready) {
						/*
						NOTE(dima): It was interesting to see this
						but if I delete this check then some meshes
						will be visible partially. This is because
						they wasn't generated at this time and when
						time came to generating VAOs in the renderer
						they were generated partially too. :D
						*/
						PlatformApi.ReadBarrier();
						if (NeededChunk->MeshInfo.State == VoxelMeshState_Generated) {

							v3 ChunkPos = GetPosForVoxelChunk(NeededChunk);
							  
#define CHUNKS_IN_CHUNK_SET_WIDTH 8
#define CHUNKS_IN_CHUNK_SET_HEIGHT 1

							int ChunkSetIndexX = (CellX - ThreadworkData->TotalMinX) / CHUNKS_IN_CHUNK_SET_WIDTH;
							int ChunkSetIndexZ = (CellZ - ThreadworkData->TotalMinZ) / CHUNKS_IN_CHUNK_SET_WIDTH;
							int ChunkSetIndexY = CellY / CHUNKS_IN_CHUNK_SET_HEIGHT;

							int IndexInPrecompArray = ChunkSetIndexX +
								ChunkSetIndexZ * ThreadworkData->ChunksSetsXCount +
								ChunkSetIndexY * ThreadworkData->ChunksSetsXCount * ThreadworkData->ChunksSetsZCount;

							b32 ShouldBePushed = ThreadworkData->FCPrecomp[IndexInPrecompArray];
							
							if (ShouldBePushed) {
								voxel_cell_render_entry* RenderEntry = PushStruct(
									&ThreadworkData->Threadwork->Memory,
									voxel_cell_render_entry);

								RenderEntry->Next = ThreadworkData->FirstRenderEntry;
								ThreadworkData->FirstRenderEntry = RenderEntry;
								RenderEntry->Pos = ChunkPos;
								RenderEntry->MeshInfo = &NeededChunk->MeshInfo;

								ThreadworkData->ChunksPushed++;
								ThreadworkData->TrianglesPushed += NeededChunk->MeshInfo.VerticesCount / 3;
							}

							ThreadworkData->TrianglesLoaded += NeededChunk->MeshInfo.VerticesCount / 3;
						}

						ThreadworkData->ChunksLoaded++;
					}

					/*NOTE(dima): Loaded chunk is in range. And some
					additional checks will be made here*/

#if 1

					//BEGIN_TIMING("Finding neighbours");
					neighbours_chunks Neighbours = VoxelFindNeighboursChunks(
						Generation,
						NeededChunk->IndexX,
						NeededChunk->IndexY,
						NeededChunk->IndexZ);
					//END_TIMING();

					if (VoxelChunkNeighboursChanged(NeededChunk, &Neighbours)) {
						//NOTE(dima): Updating neighbours
						NeededChunk->OldNeighbours = Neighbours;

						threadwork_data* MeshGenThreadwork = BeginThreadworkData(&Generation->GenSet);

						Assert(MeshGenThreadwork);

						generate_voxel_mesh_threadwork_data* MeshGenerationData = PushStruct(
							&MeshGenThreadwork->Memory,
							generate_voxel_mesh_threadwork_data);

						MeshGenerationData->Generation = Generation;
						MeshGenerationData->MeshGenThreadwork = MeshGenThreadwork;

						MeshGenerationData->MeshGenerateData.Atlas = VoxelAtlas;
						MeshGenerationData->MeshGenerateData.Chunk = NeededChunk;

						MeshGenerationData->NeighboursSidesThreadwork = BeginThreadworkData(&Generation->NeighboursSidesSet);
						MeshGenerationData->MeshGenerateData.NeighboursSetInfo = PushStruct(
							&MeshGenerationData->NeighboursSidesThreadwork->Memory,
							neighbours_set_info);

						FillNeighboursSetInfoBasedOnNeighbours(
							MeshGenerationData->MeshGenerateData.NeighboursSetInfo,
							&NeededChunk->OldNeighbours);

						PlatformApi.AddThreadworkEntry(
							PlatformApi.VoxelQueue,
							MeshGenerationData,
							RegenerateVoxelMeshThreadwork);
					}
#endif
				}
				else {
					BEGIN_TIMING("Insertion and starting generation");
					BEGIN_TIMING("Finding threadwork");
					threadwork_data* Threadwork = BeginThreadworkData(&Generation->ChunkSet);
					END_TIMING();

					if (Threadwork) {
						threadwork_data* ChunkGenThreadwork = BeginThreadworkData(&Generation->GenSet);

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
						ChunkGenerationData->Chunk->OldNeighbours = VoxelFindNeighboursChunks(
							Generation,
							CellX,
							CellY,
							CellZ);

						ChunkGenerationData->VoxelAtlasInfo = VoxelAtlas;
						ChunkGenerationData->Generation = Generation;
						ChunkGenerationData->ChunkGenThreadwork = ChunkGenThreadwork;

						VoxelInsertToTable(Generation, ChunkInfo);

#if 1
						BEGIN_TIMING("Pushing work to thread queue");
						PlatformApi.AddThreadworkEntry(
							PlatformApi.VoxelQueue,
							ChunkGenerationData,
							GenerateVoxelChunkThreadwork);
						END_TIMING();
#else
						GenerateVoxelChunkThreadwork(ChunkGenerationData);
#endif
					}
					END_TIMING();
				}
			}
		}
	}
	END_TIMING();
}

static void VoxelChunksGenerationInit(
	voxworld_generation_state* Generation,
	stacked_memory* Memory,
	int VoxelThreadQueueSize,
	input_system* Input)
{
	int ChunksViewDistanceCount = 20;
	int ChunksHeightCount = 1;

	int TotalChunksSideCount = (ChunksViewDistanceCount * 2 + 1);
	int TotalChunksCount = TotalChunksSideCount * TotalChunksSideCount * ChunksHeightCount;

	Generation->ChunksViewDistance = ChunksViewDistanceCount;
	Generation->ChunksSideCount = TotalChunksSideCount;
	Generation->ChunksCount = TotalChunksCount;
	Generation->ChunksHeightCount = ChunksHeightCount;
	
	Generation->RenderPushMutex = {};
	Generation->ChunksPushed = 0;
	Generation->ChunksLoaded = 0;
	Generation->TrianglesLoaded = 0;
	Generation->TrianglesPushed = 0;
	Generation->MeshGenerationsStartedThisFrame = 0;

	Generation->Input = Input;
	Generation->MeshRegenTimeCounter = 0.0f;

	Generation->TotalMemory = Memory;
	Generation->FCPMemoryBlock = SplitStackedMemory(Generation->TotalMemory, KILOBYTES(100));

	Generation->MemoryAllocatorMutex = {};

	/* 
		NOTE(dima): Initialization of mesh threadworks.
		They are used to store mesh data for generating mesh.
	*/
	/*
		NOTE(dima): 65536 possible cubes in chunk by 6 sides
		by 6 verts per side. The worst case will be when every
		second cube is filled so I divided by 2 here
	*/
	int SizeForOneMeshThreadwork = 65536 * 6 * 6 * 4 / 2;
	InitThreadworkDataSet(
		Generation->TotalMemory, 
		&Generation->MeshSet, 
		VoxelThreadQueueSize, 
		SizeForOneMeshThreadwork);


	/*
		NOTE(dima): Initialization of work threadworks.
		They are used to store loaded chunk data;
	*/
	int SizeForOneWorkThreadwork = KILOBYTES(70);
	InitThreadworkDataSet(
		Generation->TotalMemory,
		&Generation->ChunkSet,
		TotalChunksCount,
		SizeForOneWorkThreadwork);

	/*
		NOTE(dima): Initialization of generation threadworks.
		They are used to store temporary data( for chunk
		generation threads.
	*/
	int GenThreadworksCount = 32768;

	int SizeForGenThreadwork = Max(
		sizeof(generate_voxel_chunk_data), 
		sizeof(generate_voxel_mesh_threadwork_data));

	SizeForGenThreadwork = Max(SizeForGenThreadwork, sizeof(unload_voxel_chunk_data));

	SizeForGenThreadwork += 16;

	InitThreadworkDataSet(
		Generation->TotalMemory,
		&Generation->GenSet,
		GenThreadworksCount,
		SizeForGenThreadwork);

	/*
		NOTE(dima): Initialization of cell walkaround
		threadworks. They are used to store data for
		cell walkarounds in update code every frame.
	*/
	int CellWalkaroundThreadsCount = 4;
	int SizeForOneCellWalkaroundThreadwork = MEGABYTES(1);

	InitThreadworkDataSet(
		Generation->TotalMemory,
		&Generation->CellWalkaroundSet,
		CellWalkaroundThreadsCount,
		SizeForOneCellWalkaroundThreadwork);

	/*
	NOTE(dima): Initialization of chunk neighbours
	threadworks. They hold information about chunk 
	neighbours sides for if their set or not.
	*/
	InitThreadworkDataSet(
		Generation->TotalMemory,
		&Generation->NeighboursSidesSet,
		Generation->ChunksCount * 10,
		sizeof(neighbours_set_info) + 16);

	//NOTE(dima): Initializing of world chunks hash table
	Generation->HashTableOpMutex = {};
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

	//NOTE(dima): Initialization of random state
	Generation->SmoothRandomIndex = 0;

	Generation->Initialized = 1;
}

void VoxelChunksGenerationUpdate(
	stacked_memory* Memory,
	render_state* RenderState,
	int VoxelThreadQueueSize,
	v3 CameraPos,
	input_system* Input)
{
	FUNCTION_TIMING();

	/*
		NOTE(dima): Some ideas to optimize all this stuff

		1) Create read-copy table. It means that i might 
		create copy of the hash table at the end of each 
		frame and make all reads from it but not from the 
		table that i write to.

		!!! It means that we will not need to have mutex
		in hash table lookup(VoxelFindChunk). It would be 
		instant!)!)!)!)!))!((!0101)!)!)!!!)!)!)!)11!11!!10

		2) Create insertion, deletion lists. It means that
		i might want to make linked lists that i will insert 
		or delete instantly. Not one-by-one.

		3) Multithread cells walkaround and list walkaround

		4) Frustum culling. First - try to make frustum culling
		by checking all 16 16x16x16 mini-chunks. Then second - 
		try to check if all vertices are outside at least one plane
	*/

	voxworld_generation_state* Generation = (voxworld_generation_state*)Memory->BaseAddress;

	if (!Generation->Initialized) {

		Generation = PushStruct(Memory, voxworld_generation_state);

		VoxelChunksGenerationInit(Generation, Memory, VoxelThreadQueueSize, Input);

		Generation->Initialized = 1;
	}

	voxel_atlas_id VoxelAtlasID = GetFirstVoxelAtlas(RenderState->AssetSystem, GameAsset_MyVoxelAtlas);
	voxel_atlas_info* VoxelAtlas = GetVoxelAtlasFromID(RenderState->AssetSystem, VoxelAtlasID);
	
	int CamChunkIndexX;
	int CamChunkIndexY;
	int CamChunkIndexZ;

	GetVoxelChunkPosForCamera(CameraPos, &CamChunkIndexX, &CamChunkIndexY, &CamChunkIndexZ);

#if 0
	int testviewdist = 40;
	int MinCellX = -testviewdist;
	int MaxCellX = testviewdist;
	int MinCellZ = -testviewdist;
	int MaxCellZ = testviewdist;
	int MinCellY = 0;
	int MaxCellY = Generation->ChunksHeightCount - 1;
#else
	int MinCellX = CamChunkIndexX - Generation->ChunksViewDistance;
	int MaxCellX = CamChunkIndexX + Generation->ChunksViewDistance;
	int MinCellZ = CamChunkIndexZ - Generation->ChunksViewDistance;
	int MaxCellZ = CamChunkIndexZ + Generation->ChunksViewDistance;
	int MinCellY = 0;
	int MaxCellY = Generation->ChunksHeightCount - 1;
#endif

	BEGIN_TIMING("VoxelCellsWalkaround");
	BEGIN_TIMING("Preparing");
	voxel_cell_walkaround_threadwork_data CellWalkaroundDatas[4];

	v4 FrustumPlanes[6];

#if 1
	mat4 PVM = RenderState->CameraSetup.ProjectionViewMatrix;
#else
	mat4 PVM = PerspectiveProjection(
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		45.0f,
		2000.0f,
		0.1f);
#endif
	PVM = Transpose(PVM);

	BEGIN_TIMING("Frustum culling precomp");
	//NOTE(dima): Left plane
	FrustumPlanes[0].A = PVM.E[3] + PVM.E[0];
	FrustumPlanes[0].B = PVM.E[7] + PVM.E[4];
	FrustumPlanes[0].C = PVM.E[11] + PVM.E[8];
	FrustumPlanes[0].D = PVM.E[15] + PVM.E[12];

	//NOTE(dima): Right plane
	FrustumPlanes[1].A = PVM.E[3] - PVM.E[0];
	FrustumPlanes[1].B = PVM.E[7] - PVM.E[4];
	FrustumPlanes[1].C = PVM.E[11] - PVM.E[8];
	FrustumPlanes[1].D = PVM.E[15] - PVM.E[12];

	//NOTE(dima): Bottom plane
	FrustumPlanes[2].A = PVM.E[3] + PVM.E[1];
	FrustumPlanes[2].B = PVM.E[7] + PVM.E[5];
	FrustumPlanes[2].C = PVM.E[11] + PVM.E[9];
	FrustumPlanes[2].D = PVM.E[15] + PVM.E[13];

	//NOTE(dima): Top plane
	FrustumPlanes[3].A = PVM.E[3] - PVM.E[1];
	FrustumPlanes[3].B = PVM.E[7] - PVM.E[5];
	FrustumPlanes[3].C = PVM.E[11] - PVM.E[9];
	FrustumPlanes[3].D = PVM.E[15] - PVM.E[13];

	//NOTE(dima): Near plane
	FrustumPlanes[4].A = PVM.E[3] + PVM.E[2];
	FrustumPlanes[4].B = PVM.E[7] + PVM.E[6];
	FrustumPlanes[4].C = PVM.E[11] + PVM.E[10];
	FrustumPlanes[4].D = PVM.E[15] + PVM.E[14];

	//NOTE(dima): Far plane
	FrustumPlanes[5].A = PVM.E[3] - PVM.E[2];
	FrustumPlanes[5].B = PVM.E[7] - PVM.E[6];
	FrustumPlanes[5].C = PVM.E[11] - PVM.E[10];
	FrustumPlanes[5].D = PVM.E[15] - PVM.E[14];

	for (int PlaneIndex = 0;
		PlaneIndex < 6;
		PlaneIndex++)
	{
		FrustumPlanes[PlaneIndex] = NormalizePlane(FrustumPlanes[PlaneIndex]);
	}

	//NOTE(dima): Precomputing frustum culling results
	stacked_memory FCPMemory = BeginTempStackedMemory(
		&Generation->FCPMemoryBlock,
		Generation->FCPMemoryBlock.MaxSize);

	int ChunksSetsXCount = ((MaxCellX - MinCellX) + CHUNKS_IN_CHUNK_SET_WIDTH) / CHUNKS_IN_CHUNK_SET_WIDTH;
	int ChunksSetsZCount = ((MaxCellZ - MinCellZ) + CHUNKS_IN_CHUNK_SET_WIDTH) / CHUNKS_IN_CHUNK_SET_WIDTH;
	int ChunksSetsYCount = Generation->ChunksHeightCount / CHUNKS_IN_CHUNK_SET_HEIGHT;

	int ArrSize = ChunksSetsXCount * ChunksSetsYCount * ChunksSetsZCount;

	b32* PrecompFrustumCullingArray = PushArray(&FCPMemory, b32, ArrSize);

	v3 ChunkSetCornersP[8];
	//NOTE(dima): Chunk bottom corners
	ChunkSetCornersP[0] = V3(
		0.0f,
		0.0f,
		0.0f);

	ChunkSetCornersP[1] = V3(
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH,
		0.0f,
		0.0f);

	ChunkSetCornersP[2] = V3(
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH,
		0.0f,
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH);

	ChunkSetCornersP[3] = V3(
		0.0f,
		0.0f,
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH);

	//NOTE(dima): Chunk top corners
	ChunkSetCornersP[4] = V3(
		0.0f,
		(float)VOXEL_CHUNK_HEIGHT * (float)CHUNKS_IN_CHUNK_SET_HEIGHT,
		0.0f);

	ChunkSetCornersP[5] = V3(
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH,
		(float)VOXEL_CHUNK_HEIGHT * (float)CHUNKS_IN_CHUNK_SET_HEIGHT,
		0.0f);

	ChunkSetCornersP[6] = V3(
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH,
		(float)VOXEL_CHUNK_HEIGHT * (float)CHUNKS_IN_CHUNK_SET_HEIGHT,
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH);

	ChunkSetCornersP[7] = V3(
		0.0f,
		(float)VOXEL_CHUNK_HEIGHT * (float)CHUNKS_IN_CHUNK_SET_HEIGHT,
		(float)VOXEL_CHUNK_WIDTH * (float)CHUNKS_IN_CHUNK_SET_WIDTH);

	int ChunkSetIndexX = 0;
	int ChunkSetIndexY = 0;
	int ChunkSetIndexZ = 0;

	for (int CellY = 0,
		ChunkSetIndexY = 0; 
		CellY < Generation->ChunksHeightCount; 
		CellY+= CHUNKS_IN_CHUNK_SET_HEIGHT,
		ChunkSetIndexY++) 
	{
		for (int CellZ = MinCellZ,
			ChunkSetIndexZ = 0; 
			CellZ <= MaxCellZ; 
			CellZ += CHUNKS_IN_CHUNK_SET_WIDTH,
			ChunkSetIndexZ++) 
		{
			for (int CellX = MinCellX,
				ChunkSetIndexX = 0;
				CellX <= MaxCellX; 
				CellX += CHUNKS_IN_CHUNK_SET_WIDTH,
				ChunkSetIndexX++) 
			{
				int ResultIndex = ChunkSetIndexX +
					ChunkSetIndexZ * ChunksSetsXCount +
					ChunkSetIndexY * ChunksSetsXCount * ChunksSetsZCount;

				int ChunkCullTest = 0;

				v3 ChunkSetOffset = V3(
					(float)VOXEL_CHUNK_WIDTH * (float)(MinCellX + ChunkSetIndexX * (float)CHUNKS_IN_CHUNK_SET_WIDTH),
					(float)VOXEL_CHUNK_HEIGHT * (float)(MinCellY + ChunkSetIndexY * (float)CHUNKS_IN_CHUNK_SET_HEIGHT),
					(float)VOXEL_CHUNK_WIDTH * (float)(MinCellZ + ChunkSetIndexZ * (float)CHUNKS_IN_CHUNK_SET_WIDTH));

				v3 TestPts[8];
				for (int PIndex = 0;
					PIndex < 8;
					PIndex++)
				{
					TestPts[PIndex] = ChunkSetCornersP[PIndex] + ChunkSetOffset;
				}

				//RENDERPushVolumeOutline(RenderState, TestPts[0], TestPts[6], V3(1.0f, 1.0f, 1.0f), 1.0f);

				for (int PlaneIndex = 0;
					PlaneIndex < 6;
					PlaneIndex++)
				{
					int PointTestRes = 1;
					for (int TestPIndex = 0;
						TestPIndex < 8;
						TestPIndex++)
					{
						PointTestRes &= (PlanePointTest(
							FrustumPlanes[PlaneIndex],
							TestPts[TestPIndex]) < 0.0f);
					}

					ChunkCullTest |= PointTestRes;
				}

				PrecompFrustumCullingArray[ResultIndex] = (ChunkCullTest == 0);
			}
		}
	}
	END_TIMING();

	for (int CellDataIndex = 0;
		CellDataIndex < 4;
		CellDataIndex++)
	{
		voxel_cell_walkaround_threadwork_data* CellData = CellWalkaroundDatas + CellDataIndex;

		*CellData = {};

		CellData->Generation = Generation;
		CellData->VoxelAtlas = VoxelAtlas;

		CellData->Threadwork = BeginThreadworkData(&Generation->CellWalkaroundSet);

		Assert(CellData->Threadwork);

		CellData->FirstRenderEntry = 0;

		CellData->FCPrecomp = PrecompFrustumCullingArray;
	}

	CellWalkaroundDatas[0].MinX = MinCellX;
	CellWalkaroundDatas[0].MaxX = CamChunkIndexX - 1;
	CellWalkaroundDatas[0].MinZ = MinCellZ;
	CellWalkaroundDatas[0].MaxZ = CamChunkIndexZ - 1;

	CellWalkaroundDatas[1].MinX = MinCellX;
	CellWalkaroundDatas[1].MaxX = CamChunkIndexX - 1;
	CellWalkaroundDatas[1].MinZ = CamChunkIndexZ;
	CellWalkaroundDatas[1].MaxZ = MaxCellZ;
	
	CellWalkaroundDatas[2].MinX = CamChunkIndexX;
	CellWalkaroundDatas[2].MaxX = MaxCellX;
	CellWalkaroundDatas[2].MinZ = MinCellZ;
	CellWalkaroundDatas[2].MaxZ = CamChunkIndexZ - 1;

	CellWalkaroundDatas[3].MinX = CamChunkIndexX;
	CellWalkaroundDatas[3].MaxX = MaxCellX;
	CellWalkaroundDatas[3].MinZ = CamChunkIndexZ;
	CellWalkaroundDatas[3].MaxZ = MaxCellZ;

	for (int DataIndex = 0;
		DataIndex < 4;
		DataIndex++)
	{
		CellWalkaroundDatas[DataIndex].MinY = MinCellY;
		CellWalkaroundDatas[DataIndex].MaxY = MaxCellY;

		CellWalkaroundDatas[DataIndex].TotalMinX = MinCellX;
		CellWalkaroundDatas[DataIndex].TotalMaxX = MaxCellX;
		CellWalkaroundDatas[DataIndex].TotalMinY = MinCellY;
		CellWalkaroundDatas[DataIndex].TotalMaxY = MaxCellY;
		CellWalkaroundDatas[DataIndex].TotalMinZ = MinCellZ;
		CellWalkaroundDatas[DataIndex].TotalMaxZ = MaxCellZ;

		CellWalkaroundDatas[DataIndex].ChunksSetsXCount = ChunksSetsXCount;
		CellWalkaroundDatas[DataIndex].ChunksSetsYCount = ChunksSetsYCount;
		CellWalkaroundDatas[DataIndex].ChunksSetsZCount = ChunksSetsZCount;
	}
	END_TIMING();

	BEGIN_TIMING("Performing");
	for (int CellDataIndex = 0;
		CellDataIndex < 4;
		CellDataIndex++)
	{
		voxel_cell_walkaround_threadwork_data* CellData = CellWalkaroundDatas + CellDataIndex;

		PlatformApi.AddThreadworkEntry(
			PlatformApi.HighPriorityQueue,
			CellData,
			VoxelCellWalkaroundThreadwork);
	}

	PlatformApi.CompleteThreadWorks(PlatformApi.HighPriorityQueue);
	END_TIMING();

	BEGIN_TIMING("Finishing");

	EndTempStackedMemory(&Generation->FCPMemoryBlock, &FCPMemory);

	for (int CellDataIndex = 0;
		CellDataIndex < 4;
		CellDataIndex++)
	{
		voxel_cell_walkaround_threadwork_data* CellData = CellWalkaroundDatas + CellDataIndex;

		voxel_cell_render_entry* RenderEntryAt = CellData->FirstRenderEntry;
		for (RenderEntryAt;
			RenderEntryAt;
			RenderEntryAt = RenderEntryAt->Next)
		{
			RENDERPushVoxelMesh(
				RenderState,
				RenderEntryAt->MeshInfo,
				RenderEntryAt->Pos,
				&CellData->VoxelAtlas->Bitmap);
		}

		EndThreadworkData(CellData->Threadwork, &Generation->CellWalkaroundSet);

		Generation->TrianglesLoaded += CellData->TrianglesLoaded;
		Generation->TrianglesPushed += CellData->TrianglesPushed;
		Generation->ChunksPushed += CellData->ChunksPushed;
		Generation->ChunksLoaded += CellData->ChunksLoaded;
	}

	if (Generation->MeshRegenTimeCounter > MESH_REGEN_FREQUENCY) {
		Generation->MeshRegenTimeCounter -= MESH_REGEN_FREQUENCY;
	}
	Generation->MeshRegenTimeCounter += Input->DeltaTime;

	END_TIMING();
	END_TIMING();
	
	BEGIN_TIMING("VoxelListWalkaround");
	for (voxworld_table_entry* At = Generation->WorkTableEntrySentinel->NextBro;
		At != Generation->WorkTableEntrySentinel;)
	{
		voxel_chunk_info* NeededChunk = At->ValueChunk;

		voxworld_table_entry* NextTableEntry = At->NextBro;

#if 1
		PlatformApi.ReadBarrier();
		//NOTE(dima): Chunk is out of range and should be deallocated
		if (NeededChunk->State == VoxelChunkState_Ready)
		{
			if (NeededChunk->IndexX < MinCellX || NeededChunk->IndexX > MaxCellX ||
				NeededChunk->IndexZ < MinCellZ || NeededChunk->IndexZ > MaxCellZ)
			{
				//NOTE(dima): Loaded chunk is out of range and we should deallocate it

				threadwork_data* UnloadThreadwork = BeginThreadworkData(&Generation->GenSet);

				Assert(UnloadThreadwork);

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

				VoxelDeleteFromTable(
					UnloadData->Generation,
					NeededChunk->IndexX,
					NeededChunk->IndexY,
					NeededChunk->IndexZ);
			}
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
