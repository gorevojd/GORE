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

	int StartHeight = 100;

	v3 ChunkPos = GetPosForVoxelChunk(Chunk);

	for (int j = 0; j < VOXEL_CHUNK_WIDTH; j++) {
		for (int i = 0; i < VOXEL_CHUNK_WIDTH; i++) {

			float NoiseScale1 = 3.0f;
			float NoiseScale2 = 25.0f;
			float NoiseScale3 = 160.0f;

			float Noise1 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / 64.0f,
				(float)ChunkPos.y / 64.0f,
				(float)(ChunkPos.z + j) / 64.0f, 0, 0, 0);

			float Noise2 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / 128.0f,
				(float)ChunkPos.y / 128.0f,
				(float)(ChunkPos.z + j) / 128.0f, 0, 0, 0);

			float Noise3 = stb_perlin_noise3(
				(float)(ChunkPos.x + i) / 256.0f,
				(float)ChunkPos.y / 256.0f,
				(float)(ChunkPos.z + j) / 256.0f, 0, 0, 0);

			float RandHeight = (Noise1 * NoiseScale1 + Noise2 * NoiseScale2 + Noise3 * NoiseScale3) + StartHeight;

			Chunk->Voxels[GET_VOXEL_INDEX(i, j, (u32)RandHeight)] = VoxelMaterial_GrassyGround;

			BuildColumn(Chunk, i, j, 0, (u32)RandHeight - 1, VoxelMaterial_Ground);
		}
	}
}

#include "stb_sprintf.h"

inline void GetVoxelChunkPosForCamera(
	v3 CamPos, 
	int* IDChunkX, 
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

	int ResZ;
	int CamPosZ = (int)CamPos.z;
	if (CamPos.z >= 0.0f) {
		ResZ = CamPosZ / VOXEL_CHUNK_WIDTH;
	}
	else {
		ResZ = (CamPosZ / VOXEL_CHUNK_WIDTH) - 1;
	}

	*IDChunkX = ResX;
	*IDChunkX = ResZ;
}

static voxworld_threadwork* VoxelAllocateThreadwork(
	stacked_memory* Memory, 
	u32 ThreadworkMemorySize) 
{
	voxworld_threadwork* Result = PushStruct(Memory, voxworld_threadwork);

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
	std::mutex* ThreadworksMutex,
	int* FreeWorkCount) 
{
	voxworld_threadwork* Result = 0;

	ThreadworksMutex->lock();

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

	ThreadworksMutex->unlock();

	return(Result);
}

static void VoxelEndThreadwork(
	voxworld_threadwork* Threadwork,
	voxworld_threadwork* FreeSentinel,
	std::mutex* ThreadworksMutex,
	int* FreeWorkCount)
{
	ThreadworksMutex->lock();

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

	ThreadworksMutex->unlock();
}

static void VoxelInsertToTable(voxworld_generation_state* Generation, voxel_chunk_info* Info) 
{
	char KeyStr[64];
	stbsp_sprintf(KeyStr, "%d|%d|%d", Info->IndexX, Info->IndexY, Info->IndexZ);
	u32 Key = StringHashFNV(KeyStr);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	voxworld_table_entry** FirstEntry = &Generation->HashTable[InTableIndex];

	voxworld_table_entry* PrevEntry = 0;

	if (*FirstEntry) {
		PrevEntry = *FirstEntry;
	}

	if (PrevEntry) {
		while (PrevEntry->Next) {
			PrevEntry = PrevEntry->Next;
		}
	}

	/*
		NOTE(dima): Now that the prev element found we
		can create slot to insert it at the new place
	*/
	voxworld_table_entry* NewEntry = PushStruct(Generation->TotalMemory, voxworld_table_entry);

	NewEntry->ValueChunk = Info;
	NewEntry->Next = 0;
	NewEntry->Key = Key;

	if (PrevEntry) {
		PrevEntry->Next = NewEntry;
	}
	else {
		*FirstEntry = NewEntry;
	}
}

static voxel_chunk_info* VoxelFindChunk(
	voxworld_generation_state* Generation,
	int X, int Y, int Z) 
{
	voxel_chunk_info* Result = 0;

	char KeyStr[64];
	stbsp_sprintf(KeyStr, "%d|%d|%d", X, Y, Z);
	u32 Key = StringHashFNV(KeyStr);
	u32 InTableIndex = Key % VOXWORLD_TABLE_SIZE;

	voxworld_table_entry* FirstEntry = Generation->HashTable[InTableIndex];
	
	voxworld_table_entry* At = FirstEntry;

	while (At != 0) {
		if (At->Key == Key) {
			char AtChunkStr[64];
			stbsp_sprintf(
				AtChunkStr, "%d|%d|%d", 
				At->ValueChunk->IndexX,
				At->ValueChunk->IndexY,
				At->ValueChunk->IndexZ);

			//NOTE(dima): Important to have additional check here!!!
			//NOTE(dima): Because of hash function might overlap with others chunks
			if (StringsAreEqual(AtChunkStr, KeyStr)) {
				Result = At->ValueChunk;
				break;
			}
		}

		At = At->Next;
	}

	return(Result);
}

struct generate_voxel_chunk_data {
	//NOTE(dima): Used to store temporary generation work data while generating chunk
	voxworld_threadwork* GenThreadwork;

	voxworld_generation_state* Generation;

	voxel_chunk_info* Chunk;
	voxel_atlas_info* VoxelAtlasInfo;
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

		//TODO(dima): Better memory management here
		//WorkChunk->MeshInfo.Vertices = (u32*)malloc(65536 * 6 * 6 * 4);
		WorkChunk->MeshInfo.Vertices = (u32*)malloc(65536 * 5);
		VoxmeshGenerate(&WorkChunk->MeshInfo, WorkChunk, GenData->VoxelAtlasInfo);
		WorkChunk->MeshInfo.Vertices = (u32*)realloc(
			WorkChunk->MeshInfo.Vertices,
			WorkChunk->MeshInfo.VerticesCount * 4);

		PLATFORM_COMPILER_BARRIER();

		WorkChunk->State = VoxelChunkState_Ready;

		VoxelEndThreadwork(
			GenData->GenThreadwork,
			GenData->Generation->GenFreeSentinel,
			&GenData->Generation->GenMutex,
			&GenData->Generation->FreeGenThreadworksCount);
	}
	else if (WorkChunk->State == VoxelChunkState_InProcess) {
		while (WorkChunk->State == VoxelChunkState_InProcess) {

		}
	}
	else {

	}
}

void VoxelChunksGenerationInit(
	voxworld_generation_state* Generation,
	stacked_memory* Memory,
	int ChunksViewDistanceCount)
{
	int TotalChunksSideCount = (ChunksViewDistanceCount * 2 + 1);
	int TotalChunksCount = TotalChunksSideCount * TotalChunksSideCount;

	Generation->ChunksSideCount = TotalChunksSideCount;
	Generation->ChunksCount = TotalChunksCount;

	Generation->TotalMemory = Memory;

	/*
		NOTE(dima): Initialization of work threadworks.
		They are used to store loaded chunk data;
	*/
	Generation->WorkUseSentinel = VoxelAllocateThreadwork(Generation->TotalMemory, 0);
	Generation->WorkFreeSentinel = VoxelAllocateThreadwork(Generation->TotalMemory, 0);

	Generation->FreeWorkThreadworksCount = TotalChunksCount;
	Generation->TotalWorkThreadworksCount = TotalChunksCount;

	for (int NewWorkIndex = 0;
		NewWorkIndex < TotalChunksCount;
		NewWorkIndex++) 
	{
		voxworld_threadwork* NewThreadwork = 
			VoxelAllocateThreadwork(Generation->TotalMemory, KILOBYTES(70));
		VoxelInsertThreadworkAfter(NewThreadwork, Generation->WorkFreeSentinel);
	}

	/*
		NOTE(dima): Initialization of generation threadworks.
		They are used to store temporary data( for chunk
		generation threads.
	*/
	int GenThreadworksCount = 2000;

	Generation->GenUseSentinel = VoxelAllocateThreadwork(Generation->TotalMemory, 0);
	Generation->GenFreeSentinel = VoxelAllocateThreadwork(Generation->TotalMemory, 0);

	Generation->FreeGenThreadworksCount = GenThreadworksCount;
	Generation->TotalGenThreadworksCount = GenThreadworksCount;

	for (int GenThreadworkIndex = 0;
		GenThreadworkIndex < GenThreadworksCount;
		GenThreadworkIndex++)
	{
		//NOTE(dima): 16 bytes added here because of the alignment problems that may arrive
		voxworld_threadwork* NewThreadwork =
			VoxelAllocateThreadwork(Generation->TotalMemory, sizeof(generate_voxel_chunk_data) + 16);
		VoxelInsertThreadworkAfter(NewThreadwork, Generation->GenFreeSentinel);
	}

	//NOTE(dima): Initializing of world chunks hash table
	for (int EntryIndex = 0;
		EntryIndex < VOXWORLD_TABLE_SIZE;
		EntryIndex++)
	{
		Generation->HashTable[EntryIndex] = 0;
	}
}

void VoxelChunksGenerationUpdate(
	voxworld_generation_state* Generation,
	render_state* RenderState,
	v3 CameraPos)
{
	FUNCTION_TIMING();

	BEGIN_SECTION("VoxelState");
	char Stat1Str[64];
	char Stat2Str[64];

	stbsp_sprintf(Stat1Str, "Gen tasks: %d free; %d total.",
		Generation->FreeGenThreadworksCount,
		Generation->TotalGenThreadworksCount);

	stbsp_sprintf(Stat2Str, "Work tasks: %d free; %d total.",
		Generation->FreeWorkThreadworksCount,
		Generation->TotalWorkThreadworksCount);

	DEBUG_STACKED_MEM("VoxelState memory", Generation->TotalMemory);
	END_SECTION();

	voxel_atlas_id VoxelAtlasID = GetFirstVoxelAtlas(RenderState->AssetSystem, GameAsset_MyVoxelAtlas);
	voxel_atlas_info* VoxelAtlas = GetVoxelAtlasFromID(RenderState->AssetSystem, VoxelAtlasID);
	
	int CamChunkIndexX;
	int CamChunkIndexZ;

	GetVoxelChunkPosForCamera(CameraPos, &CamChunkIndexX, &CamChunkIndexZ);

	int CellY = 0;

	for (int CellX = -20; CellX < 20; CellX++) {
		for (int CellZ = -20; CellZ < 20; CellZ++) {
			
			voxel_chunk_info* NeededChunk = VoxelFindChunk(Generation, CellX, CellY, CellZ);

			if (NeededChunk) {
				if (NeededChunk->State == VoxelChunkState_Ready) {

					v3 ChunkPos = GetPosForVoxelChunk(NeededChunk);

					RENDERPushVoxelMesh(
						RenderState, 
						&NeededChunk->MeshInfo, 
						ChunkPos, 
						&VoxelAtlas->Bitmap);
				}
			}
			else {
				voxworld_threadwork* Threadwork = VoxelBeginThreadwork(
					Generation->WorkFreeSentinel,
					Generation->WorkUseSentinel,
					&Generation->WorkMutex,
					&Generation->FreeWorkThreadworksCount);

				if (Threadwork) {
					voxworld_threadwork* GenThreadwork = VoxelBeginThreadwork(
						Generation->GenFreeSentinel,
						Generation->GenUseSentinel,
						&Generation->GenMutex,
						&Generation->FreeGenThreadworksCount);

					Assert(GenThreadwork);

					generate_voxel_chunk_data* ChunkGenerationData = PushStruct(
						&GenThreadwork->Memory, 
						generate_voxel_chunk_data);

					ChunkGenerationData->Chunk = PushStruct(
						&Threadwork->Memory, 
						voxel_chunk_info);

					ChunkGenerationData->Chunk->IndexX = CellX;
					ChunkGenerationData->Chunk->IndexY = CellY;
					ChunkGenerationData->Chunk->IndexZ = CellZ;
					ChunkGenerationData->Chunk->State = VoxelChunkState_None;
					ChunkGenerationData->Chunk->Threadwork = Threadwork;

					ChunkGenerationData->VoxelAtlasInfo = VoxelAtlas;
					ChunkGenerationData->Generation = Generation;
					ChunkGenerationData->GenThreadwork = GenThreadwork;

					PlatformApi.AddThreadworkEntry(
						PlatformApi.HighPriorityQueue,
						ChunkGenerationData,
						GenerateVoxelChunkThreadwork);

					VoxelInsertToTable(Generation, ChunkGenerationData->Chunk);
				}
			}
		}
	}
}
