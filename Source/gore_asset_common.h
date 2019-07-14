#ifndef GORE_ASSET_COMMON_H_INCLUDED
#define GORE_ASSET_COMMON_H_INCLUDED

#include <stdio.h>

#include "gore_math.h"
#include "gore_types.h"
#include "gore_asset_types.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 font_glyph_id;
typedef u32 sound_id;
typedef u32 model_id;
typedef u32 mesh_id;



bitmap_info AssetAllocateBitmapInternal(u32 Width, u32 Height, void* PixelsData);
bitmap_info AssetAllocateBitmap(u32 Width, u32 Height);
void AssetDeallocateBitmap(bitmap_info* Buffer);

void AssetCopyBitmapData(bitmap_info* Dst, bitmap_info* Src);


bitmap_info AssetLoadIMG(char* Path);

bitmap_info AssetGenerateCheckerboardBitmap(
	int Width,
	int CellCountPerWidth,
	v3 FirstColor = V3(1.0f, 1.0f, 1.0f),
	v3 SecondColor = V3(0.0f, 0.0f, 0.0f));

mesh_info AssetLoadMeshFromVertices(
	float* Verts, u32 VertsCount,
	u32* Indices, u32 IndicesCount,
	u32 VertexLayout,
	b32 CalculateNormals = 0,
	b32 CalculateTangents = 0);

mesh_info AssetGenerateSphere(int Segments, int Rings);
mesh_info AssetGenerateCylynder(float Height, float Radius, int SidesCount);

#endif