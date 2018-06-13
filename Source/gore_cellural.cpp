#include "gore_cellural.h"

#include <stdlib.h>

/* 
	NOTE(dima): 
		I'v allocated 1 byte per cell here.
		If i want I can actually do 1 bit per 1 cell, 
		but i think it might be useful at some time 
		to use other states than black or white.
*/

struct cellural_stack_entry {
	cellural_stack_entry* Next;

	int X;
	int Y;
};

struct cellural_stack {
	cellural_stack_entry* Head;
	int Count;
};

void InitCelluralStack(cellural_stack* Stack) {
	Stack->Count = 0;
	Stack->Head = 0;
}

inline void CelluralStackPush(cellural_stack* Stack, cellural_stack_entry* Entry) {
	Entry->Next = Stack->Head;
	Stack->Head = Entry;
	Stack->Count++;
}

inline b32 CelluralStackIsEmpty(cellural_stack* Stack) {
	b32 Result = (Stack->Count == 0) && (Stack->Head == 0);

	return(Result);
}

inline cellural_stack_entry* CelluralStackPeek(cellural_stack* Stack) {
	cellural_stack_entry* Result = Stack->Head;

	return(Result);
}

inline void CelluralStackPop(cellural_stack* Stack) {
	if (Stack->Head) {
		cellural_stack_entry* Popped = Stack->Head;
		Stack->Head = Stack->Head->Next;
		Stack->Count--;

		free(Popped);
	}
}

void FreeCelluralStack(cellural_stack* Stack) {
	while (Stack->Head) {
		CelluralStackPop(Stack);
	}
}

inline cellural_t* GetCelluralCell(cellural_buffer* Buffer, int X, int Y) {
	cellural_t* Result = 0;

	if ((X < Buffer->InitX || X >= (Buffer->InitX + Buffer->Width)) ||
		(Y < Buffer->InitY || Y >= (Buffer->InitY + Buffer->Height)))
	{
		Result = 0;
	}
	else {
		Result = (cellural_t*)((u8*)Buffer->Buf + Y * Buffer->Pitch + X * sizeof(cellural_t));
	}

	return(Result);
}

inline b32 CellIsAtEdge(cellural_buffer* Buffer, int X, int Y) {
	b32 Result =
		(X == Buffer->InitX) || (Y == Buffer->InitY) ||
		(X == (Buffer->InitX + Buffer->Width - 1)) ||
		(Y == (Buffer->InitY + Buffer->Height - 1));

	return(Result);
}

cellural_buffer AllocateCelluralBuffer(i32 Width, i32 Height) {
	cellural_buffer Buffer;
	Buffer.Buf = (u8*)calloc(Width * Height, sizeof(cellural_t));
	Buffer.Width = Width;
	Buffer.Height = Height;
	Buffer.Pitch = Width * sizeof(cellural_t);
	
	Buffer.InitX = 0;
	Buffer.InitY = 0;

	return(Buffer);
}

void DeallocateCelluralBuffer(cellural_buffer* Buffer) {
	if (Buffer->Buf) {
		free(Buffer->Buf);
	}
	Buffer->Width = 0;
	Buffer->Height = 0;
}

enum cellural_jump_direction {
	CellJumpDirection_Left,
	CellJumpDirection_Right,
	CellJumpDirection_Top,
	CellJumpDirection_Bottom,

	CellJumpDirection_Count,
};

enum try_get_next_cell_result_type {
	TryGetNextCell_OK,
	TryGetNextCell_DeadEnd,
	TryGetNextCell_CantJump,
};

static u32 TryGetNextLabyCell(int* GenX, int* GenY, cellural_buffer* Buffer, random_state* RandomState) {

	cellural_t* CurCell = GetCelluralCell(Buffer, *GenX, *GenY);

	cellural_t* LeftCell = GetCelluralCell(Buffer, *GenX - 2, *GenY);
	cellural_t* RightCell = GetCelluralCell(Buffer, *GenX + 2, *GenY);
	cellural_t* TopCell = GetCelluralCell(Buffer, *GenX, *GenY - 2);
	cellural_t* BottomCell = GetCelluralCell(Buffer, *GenX, *GenY + 2);

	cellural_t* PreLeftCell = GetCelluralCell(Buffer, *GenX - 1, *GenY);
	cellural_t* PreRightCell = GetCelluralCell(Buffer, *GenX + 1, *GenY);
	cellural_t* PreTopCell = GetCelluralCell(Buffer, *GenX, *GenY - 1);
	cellural_t* PreBottomCell = GetCelluralCell(Buffer, *GenX, *GenY + 1);

	b32 CantJumpLeft = 1;
	b32 CantJumpRight = 1;
	b32 CantJumpTop = 1;
	b32 CantJumpBottom = 1;

	if (CurCell) {
		*CurCell = 1;
	}

	if (LeftCell) {
		CantJumpLeft = (*LeftCell != 0);
		//CantJumpLeft = 0;
	}
	if (RightCell) {
		CantJumpRight = (*RightCell != 0);
		//CantJumpRight = 0;
	}
	if (TopCell) {
		CantJumpTop = (*TopCell != 0);
		//CantJumpTop = 0;
	}
	if (BottomCell) {
		CantJumpBottom = (*BottomCell != 0);
		//CantJumpBottom = 0;
	}

	if (CantJumpLeft &&
		CantJumpRight &&
		CantJumpTop &&
		CantJumpBottom)
	{
		//NOTE(dima): Can't make a jump anymore!!!
		return(TryGetNextCell_DeadEnd);
	}

	u32 RandomDirection = XORShift32(RandomState) & 3;
	cellural_t* RandomDirectionCell = 0;
	int NewX, NewY;

	cellural_t *DstCell = 0;
	cellural_t *PreDstCell = 0;


	/*
		NOTE(dima): What i did here is that i
		found first random valid direction that
		i can go to. Instead of just peeking random 
		direction and hope that i can go there
	*/
	u32 CanJumpDirections[CellJumpDirection_Count];
	CanJumpDirections[CellJumpDirection_Left] = !CantJumpLeft;
	CanJumpDirections[CellJumpDirection_Right] = !CantJumpRight;
	CanJumpDirections[CellJumpDirection_Bottom] = !CantJumpBottom;
	CanJumpDirections[CellJumpDirection_Top] = !CantJumpTop;

	u32 TmpDirection = RandomDirection;
	for (int TmpIndex = TmpDirection;; TmpIndex++) {

		if (CanJumpDirections[TmpIndex]) {
			RandomDirection = TmpIndex;
			break;
		}

		if (TmpIndex == 3) {
			TmpIndex = -1;
		}
	}

	switch (RandomDirection) {
		case CellJumpDirection_Left: {
			RandomDirectionCell = LeftCell;

			NewX = *GenX - 2;
			NewY = *GenY;

			DstCell = LeftCell;
			PreDstCell = PreLeftCell;
		}break;

		case CellJumpDirection_Right: {
			RandomDirectionCell = RightCell;

			NewX = *GenX + 2;
			NewY = *GenY;

			DstCell = RightCell;
			PreDstCell = PreRightCell;
		}break;

		case CellJumpDirection_Top: {
			RandomDirectionCell = TopCell;

			NewX = *GenX;
			NewY = *GenY - 2;

			DstCell = TopCell;
			PreDstCell = PreTopCell;
		}break;

		case CellJumpDirection_Bottom: {
			RandomDirectionCell = BottomCell;

			NewX = *GenX;
			NewY = *GenY + 2;

			DstCell = BottomCell;
			PreDstCell = PreBottomCell;
		}break;
	}

	*GenX = NewX;
	*GenY = NewY;
		
	if (DstCell) {
		*DstCell = 1;
	}

	if (PreDstCell) {
		*PreDstCell = 1;
	}

	return(TryGetNextCell_OK);
}

static inline b32 CellHasNotVisitedMembers(int GenX, int GenY, cellural_buffer* Buffer) {
	b32 Result = 0;

	cellural_t* LeftCell = GetCelluralCell(Buffer, GenX - 2, GenY);
	cellural_t* RightCell = GetCelluralCell(Buffer, GenX + 2, GenY);
	cellural_t* TopCell = GetCelluralCell(Buffer, GenX, GenY - 2);
	cellural_t* BottomCell = GetCelluralCell(Buffer, GenX, GenY + 2);

	if (LeftCell) {
		Result |= (*LeftCell == 0);
	}
	if (RightCell) {
		Result |= (*RightCell == 0);
	}
	if (TopCell) {
		Result |= (*TopCell == 0);
	}
	if (BottomCell) {
		Result |= (*BottomCell == 0);
	}

	return(Result);
}

static void CelluralStackPushCellCond(cellural_stack* Stack, int X, int Y, cellural_buffer* Buffer) {
	if (CellHasNotVisitedMembers(X, Y, Buffer)) {
		cellural_stack_entry* Entry = (cellural_stack_entry*)malloc(sizeof(cellural_stack_entry));
		Entry->Next = 0;
		Entry->X = X;
		Entry->Y = Y;

		CelluralStackPush(Stack, Entry);
	}
}

static b32 CelluralLabyIsGenerated(cellural_buffer* Buffer) {

	for (int Y = Buffer->InitY; Y < Buffer->Height; Y++) {
		for (int X = Buffer->InitX; X < Buffer->Width; X++) {
			if ((X & 1) && (Y & 1)) {
				cellural_t* Cell = GetCelluralCell(Buffer, X, Y);
				if (!*Cell) {
					return(0);
				}
			}
		}
	}

	return(1);
}


/*
	NOTE(dima): 
		Cache parameters are used here to give 
		possibility not to rerun from the beginning
*/

static b32 CelluralLabyFindValidCoord(
	cellural_buffer* Buffer, 
	int* GenX, int* GenY,
	int* Cache) 
{
	int StartX = Buffer->InitX;
	int StartY = Buffer->InitY;

	if (Cache) {
		StartX = *Cache % Buffer->Width;
		StartY = *Cache / Buffer->Width;
	}

	for (int Y = StartY; Y < Buffer->Height; Y++) {
		for (int X = StartX; X < Buffer->Width; X++) {
			if ((X & 1) && (Y & 1)) {
				cellural_t* Cell = GetCelluralCell(Buffer, X, Y);
				if (!*Cell) {
					*GenX = X;
					*GenY = Y;

					if (Cache) {
						*Cache = X + Y * Buffer->Width;
					}

					return(1);
				}
			}
		}
	}

	return(0);
}

static cellural_buffer CelluralCreateChildrenBuffer(cellural_buffer* Buffer, int X, int Y, int Width, int Height) {
	cellural_buffer Result = {};

	Result.Buf = Buffer->Buf;
	Result.Pitch = Buffer->Pitch;
	Result.Width = Width;
	Result.Height = Height;

	Result.InitX = X;
	Result.InitY = Y;

	//NOTE(dima): If target width is bigger than existing
	if (X + Width > Buffer->InitX + Buffer->Width) {
		Result.Width = Buffer->InitX + Buffer->Width - X;
	}

	//NOTE(dima): If target height is bigger than existing
	if (Y + Height > Buffer->InitY + Buffer->Height) {
		Result.Height = Buffer->InitY + Buffer->Height - Y;
	}

	return(Result);
}

static void CelluralClearBuffer(cellural_buffer* Buffer) {
	for (int Y = Buffer->InitY; Y < Buffer->Height; Y++) {
		cellural_t* Out = GetCelluralCell(Buffer, Buffer->InitX, Y);
		for (int X = Buffer->InitX; X < Buffer->Width; X++) {
			*Out++ = 0;
		}
	}
}

static void CelluralGenerateLabyDepthFirst(cellural_buffer* Buffer, random_state* RandomState) {
	cellural_stack CellStack;
	InitCelluralStack(&CellStack);

	int GenX = Buffer->InitX + 1;
	int GenY = Buffer->InitY + 1;

	CelluralStackPushCellCond(&CellStack, GenX, GenY, Buffer);

	while (!CelluralStackIsEmpty(&CellStack)) {
		if (TryGetNextLabyCell(&GenX, &GenY, Buffer, RandomState) == TryGetNextCell_DeadEnd) {
			cellural_stack_entry* Entry = CelluralStackPeek(&CellStack);

			GenX = Entry->X;
			GenY = Entry->Y;

			if (!CellHasNotVisitedMembers(GenX, GenY, Buffer)) {
				CelluralStackPop(&CellStack);
			}
		}
		else {
			CelluralStackPushCellCond(&CellStack, GenX, GenY, Buffer);
		}
	}

	FreeCelluralStack(&CellStack);
}

void CelluralGenerateLaby(cellural_buffer* Buffer, random_state* RandomState) {
	//NOTE(dima): Clearing just for safe
	CelluralClearBuffer(Buffer);

	for (int Y = 0; Y < Buffer->Height; Y++) {
		for (int X = 0; X < Buffer->Width; X++) {

		}
	}

	//NOTE(dima): Generation
	CelluralGenerateLabyDepthFirst(Buffer, RandomState);
}

static void CelluralGenerateSquadLaby(cellural_buffer* Buffer, random_state* RandomState) {
	int HalfWidth = Buffer->Width / 2;
	int HalfHeight = Buffer->Height / 2;

	//NOTE(dima): Clearing just for safe
	for (int Y = Buffer->InitY; Y < Buffer->Height; Y++) {
		cellural_t* Out = GetCelluralCell(Buffer, Buffer->InitX, Y);
		for (int X = Buffer->InitX; X < Buffer->Width; X++) {
			*Out++ = 0;
		}
	}

	//NOTE(dima): Creating children buffers
	cellural_buffer Buf1 = CelluralCreateChildrenBuffer(Buffer, 0, 0, HalfWidth, HalfHeight);
	cellural_buffer Buf2 = CelluralCreateChildrenBuffer(Buffer, HalfWidth, 0, HalfWidth, HalfHeight);
	cellural_buffer Buf3 = CelluralCreateChildrenBuffer(Buffer, 0, HalfHeight, HalfWidth, HalfHeight);
	cellural_buffer Buf4 = CelluralCreateChildrenBuffer(Buffer, HalfWidth, HalfHeight, HalfWidth, HalfHeight);

	//NOTE(dima): Clearing just for safe
	CelluralClearBuffer(Buffer);

	//NOTE(dima): Generation
	CelluralGenerateLabyDepthFirst(&Buf1, RandomState);
	CelluralGenerateLabyDepthFirst(&Buf2, RandomState);
	CelluralGenerateLabyDepthFirst(&Buf3, RandomState);
	CelluralGenerateLabyDepthFirst(&Buf4, RandomState);
}

void CelluralGenerateCave(cellural_buffer* Buffer, float FillPercentage, random_state* RandomState) {

	u8* At = Buffer->Buf;
	u8* To = Buffer->Buf;
	for (i32 Y = 0; Y < Buffer->Height; Y++) {
		for (i32 X = 0; X < Buffer->Width; X++) {

			u8 randval = XORShift32(RandomState) % 101;
			b32 ShouldBeFilled = (randval <= FillPercentage);
			if (ShouldBeFilled) {
				*To = 1;
			}
			else {
				*To = 0;
			}

			++To;
			++At;
		}
	}

#if 1
	for (int SmoothIteration = 0;
		SmoothIteration < 10;
		SmoothIteration++) 
	{
		cellural_buffer Temp = AllocateCelluralBuffer(Buffer->Width, Buffer->Height);

		At = Buffer->Buf;
		To = Temp.Buf;
		for (i32 Y = 0; Y < Buffer->Height; Y++) {
			for (i32 X = 0; X < Buffer->Width; X++) {

				if (X == 0 ||
					X == (Buffer->Width - 1) ||
					Y == 0 ||
					Y == (Buffer->Height - 1))
				{
					*To = 0;
					At++;
					To++;
					continue;
				}

				u8* Neighbours[8];
				Neighbours[0] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X - 1;
				Neighbours[1] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X;
				Neighbours[2] = Buffer->Buf + ((Y - 1) * Buffer->Width) + X + 1;
				Neighbours[3] = Buffer->Buf + (Y * Buffer->Width) + X - 1;
				Neighbours[4] = Buffer->Buf + (Y * Buffer->Width) + X + 1;
				Neighbours[5] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X - 1;
				Neighbours[6] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X;
				Neighbours[7] = Buffer->Buf + ((Y + 1) * Buffer->Width) + X + 1;

				int WallCount = 0;
				for (int i = 0; i < 8; i++) {
					if (*Neighbours[i]) {
						WallCount++;
					}
				}

				if (WallCount > 4) {
					*To = 1;
				}
				else if (WallCount < 4) {
					*To = 0;
				}
				else {
					*To = *At;
				}

				++To;
				++At;
			}
		}

#if 0
		At = Temp.Buf;
		To = Buffer->Buf;
		for (i32 Y = 0; Y < Buffer->Height; Y++) {
			for (i32 X = 0; X < Buffer->Width; X++) {
				*To++ = *At++;
			}
		}
#else
		MEMCopy(Buffer->Buf, Temp.Buf, Buffer->Width * Buffer->Height);
#endif

		DeallocateCelluralBuffer(&Temp);
	}
#endif
}

#define CELLURAL_CELL_WIDTH 4
bitmap_info CelluralBufferToBitmap(cellural_buffer* Buffer) {
	bitmap_info Res = AllocateRGBABuffer(
		Buffer->Width * CELLURAL_CELL_WIDTH, 
		Buffer->Height * CELLURAL_CELL_WIDTH);

	u8* At = Buffer->Buf;
	for (i32 Y = 0; Y < Buffer->Height; Y++) {
		for (i32 X = 0; X < Buffer->Width; X++) {

			rect2 DrawRect;
			DrawRect.Min.x = X * CELLURAL_CELL_WIDTH;
			DrawRect.Min.y = Y * CELLURAL_CELL_WIDTH;
			DrawRect.Max.x = (X + 1) * CELLURAL_CELL_WIDTH;
			DrawRect.Max.y = (Y + 1) * CELLURAL_CELL_WIDTH;

			v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f);
			if (*At == 1) {
				Color = V4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			u32 ColorU32 = PackRGBA(Color);

			for (int TargetY = DrawRect.Min.y; TargetY != DrawRect.Max.y; TargetY++) {
				for (int TargetX = DrawRect.Min.x; TargetX != DrawRect.Max.x; TargetX++) {
					u32* OutPixel = (u32*)Res.Pixels + TargetY * Res.Width + TargetX;
					*OutPixel = ColorU32;
				}
			}

			++At;
		}
	}

	return(Res);
}

inline void CelluralWriteVertex(float* Vertex, v3 P, v2 UV) {
	*Vertex = P.x;
	*(Vertex + 1) = P.y;
	*(Vertex + 2) = P.z;

	*(Vertex + 3) = UV.x;
	*(Vertex + 4) = UV.y;
}

inline void CelluralWriteFace(
	v3 Vert1, v3 Vert2, v3 Vert3, v3 Vert4, 
	voxel_face_tex_coords_set* FaceTexSet,
	float* VerticesBase, u32* IndicesBase, 
	u32* IndexAt, u32* VertexAt) 
{

	float* Vertex0 = VerticesBase + *VertexAt * 5;
	float* Vertex1 = VerticesBase + (*VertexAt + 1) * 5;
	float* Vertex2 = VerticesBase + (*VertexAt + 2) * 5;
	float* Vertex3 = VerticesBase + (*VertexAt + 3) * 5;

	CelluralWriteVertex(Vertex0, Vert1, FaceTexSet->T1);
	CelluralWriteVertex(Vertex1, Vert2, FaceTexSet->T0);
	CelluralWriteVertex(Vertex2, Vert3, FaceTexSet->T3);
	CelluralWriteVertex(Vertex3, Vert4, FaceTexSet->T2);

	u32* Ind = IndicesBase + *IndexAt;
	Ind[0] = *VertexAt;
	Ind[1] = *VertexAt + 1;
	Ind[2] = *VertexAt + 2;
	Ind[3] = *VertexAt;
	Ind[4] = *VertexAt + 2;
	Ind[5] = *VertexAt + 3;

	*IndexAt += 6;
	*VertexAt += 4;
}

mesh_info CelluralBufferToMesh(cellural_buffer* Buffer, int Height, voxel_atlas_info* Atlas) {

	mesh_info Result = {};

	float* TempVerts = (float*)malloc(1000000 * sizeof(float) * 5);
	u32* TempIndices = (u32*)malloc(1500000 * sizeof(u32));

	v3 UpOffsetVector = V3(0.0f, 1.0f, 0.0f);

	u32 IndexAt = 0;
	u32 VertexAt = 0;

	u8* At = Buffer->Buf;
	for (i32 Y = 0; Y < Buffer->Height; Y++) {
		for (i32 X = 0; X < Buffer->Width; X++) {

			float CurHeight = 0.0f;

			u8 FloorCellIndex = Atlas->Materials[VoxelMaterial_BigBrick].Top;
			//u8 WallCellIndex = Atlas->Materials[VoxelMaterial_Secret].Side;
			u8 WallCellIndex = Atlas->Materials[VoxelMaterial_GrassyBigBrick].Side;
			u8 TopCellIndex = Atlas->Materials[VoxelMaterial_GrassyBigBrick].Top;

			voxel_face_tex_coords_set TexSet = GetFaceTexCoordSetForTextureIndex(
				FloorCellIndex,
				Atlas->OneTextureWidth,
				Atlas->AtlasWidth);

			cellural_t* Cell = GetCelluralCell(Buffer, X, Y);
			if (!*Cell) {
				CurHeight = Height;
				TexSet = GetFaceTexCoordSetForTextureIndex(
					TopCellIndex,
					Atlas->OneTextureWidth,
					Atlas->AtlasWidth);
				
				cellural_t* LeftCell = GetCelluralCell(Buffer, X - 1, Y);
				cellural_t* RightCell = GetCelluralCell(Buffer, X + 1, Y);
				cellural_t* TopCell = GetCelluralCell(Buffer, X, Y - 1);
				cellural_t* BottomCell = GetCelluralCell(Buffer, X, Y + 1);

				b32 LeftCellShouldBeWritten = 
					(LeftCell && *LeftCell) || (X == 0);

				b32 RightCellShouldBeWritten = 
					(RightCell && *RightCell) || (X == Buffer->Width - 1);

				b32 TopCellShouldBeWritten = 
					(TopCell && *TopCell) || (Y == 0);

				b32 BottomCellShouldBeWritten = 
					(BottomCell && *BottomCell) || (Y == Buffer->Height - 1);

				if (LeftCellShouldBeWritten) {
					for (int HeightIndex = 0;
						HeightIndex < Height;
						HeightIndex++)
					{
						v3 First = V3(X, HeightIndex, Y);
						v3 Second = V3(X, HeightIndex, Y + 1);

						v3 Vert1 = First + UpOffsetVector;
						v3 Vert2 = Second + UpOffsetVector;
						v3 Vert3 = Second;
						v3 Vert4 = First;

						voxel_face_tex_coords_set LeftTexSet = GetFaceTexCoordSetForTextureIndex(
							WallCellIndex,
							Atlas->OneTextureWidth,
							Atlas->AtlasWidth);

						CelluralWriteFace(Vert1, Vert2, Vert3, Vert4, &LeftTexSet, TempVerts, TempIndices, &IndexAt, &VertexAt);
					}
				}

				if (RightCellShouldBeWritten) {
					for (int HeightIndex = 0;
						HeightIndex < Height;
						HeightIndex++)
					{

						v3 First = V3(X + 1, HeightIndex, Y);
						v3 Second = V3(X + 1, HeightIndex, Y + 1);

						v3 Vert1 = Second + UpOffsetVector;
						v3 Vert2 = First + UpOffsetVector;
						v3 Vert3 = First;
						v3 Vert4 = Second;

						voxel_face_tex_coords_set RightTexSet = GetFaceTexCoordSetForTextureIndex(
							WallCellIndex,
							Atlas->OneTextureWidth,
							Atlas->AtlasWidth);

						CelluralWriteFace(Vert1, Vert2, Vert3, Vert4, &RightTexSet, TempVerts, TempIndices, &IndexAt, &VertexAt);
					}
				}

				if (TopCellShouldBeWritten) {
					for (int HeightIndex = 0;
						HeightIndex < Height;
						HeightIndex++)
					{
						v3 First = V3(X, HeightIndex, Y);
						v3 Second = V3(X + 1, HeightIndex, Y);

						v3 Vert1 = Second + UpOffsetVector;
						v3 Vert2 = First + UpOffsetVector;
						v3 Vert3 = First;
						v3 Vert4 = Second;

						voxel_face_tex_coords_set TopTexSet = GetFaceTexCoordSetForTextureIndex(
							WallCellIndex,
							Atlas->OneTextureWidth,
							Atlas->AtlasWidth);

						CelluralWriteFace(Vert1, Vert2, Vert3, Vert4, &TopTexSet, TempVerts, TempIndices, &IndexAt, &VertexAt);
					}
				}

				if (BottomCellShouldBeWritten) {
					for (int HeightIndex = 0;
						HeightIndex < Height;
						HeightIndex++)
					{
						v3 First = V3(X, HeightIndex, Y + 1);
						v3 Second = V3(X + 1, HeightIndex, Y + 1);

						v3 Vert1 = First + UpOffsetVector;
						v3 Vert2 = Second + UpOffsetVector;
						v3 Vert3 = Second;
						v3 Vert4 = First;

						voxel_face_tex_coords_set BottomTexSet = GetFaceTexCoordSetForTextureIndex(
							WallCellIndex,
							Atlas->OneTextureWidth,
							Atlas->AtlasWidth);

						CelluralWriteFace(Vert1, Vert2, Vert3, Vert4, &BottomTexSet, TempVerts, TempIndices, &IndexAt, &VertexAt);
					}
				}
			}

			v3 Vert1 = V3(X, CurHeight, Y);
			v3 Vert2 = V3(X + 1, CurHeight, Y);
			v3 Vert3 = V3(X + 1, CurHeight, Y + 1);
			v3 Vert4 = V3(X, CurHeight, Y + 1);

			CelluralWriteFace(Vert1, Vert2, Vert3, Vert4, &TexSet, TempVerts, TempIndices, &IndexAt, &VertexAt);

			++At;
		}
	}

	Result = LoadMeshFromVertices(TempVerts, VertexAt, TempIndices, IndexAt, MeshVertexLayout_PUV, 1, 1);

	free(TempVerts);
	free(TempIndices);

	return(Result);
}
