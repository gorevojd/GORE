#ifndef GORE_FILE_FORMATS_H_INCLUDED
#define GORE_FILE_FORMATS_H_INCLUDED

#include "gore_types.h"

#define ASSET_FILE_VERSION 1

#pragma pack(push, 1)

/*
	//NOTE(dima): asset data is stored in the next pattern

	type: asset_file_header
	{
		(Asset file header)
	}

	type: asset_file_asset_group
	{
		(Asset group0)
		(Asset group1)
		(Asset group2)
		..............
		(Asset groupN)									N = AssetFileHeader->AssetGroupsCount
	}

	type: unsigned integer(4 bytes)
	{
		(Asset1 line byte offset)
		(Asset2 line byte offset)
		(Asset3 line byte offset)
		.........................
		(AssetN line byte offset)						N = AssetFileHeader->AssetsCount;
	}


	(Asset1 header) | (Asset1 data) | (Asset1 tags) -> Line. 
	(Asset2 header) | (Asset2 data) | (Asset2 tags)
	(Asset3 header) | (Asset3 data) | (Asset3 tags)
	...............................................
	(AssetN header) | (AssetN data) | (AssetN tags)


	Byte size of the Line is Pitch. Pitch stored in asset header
*/

struct gass_tag {
	u32 Type;

	union {
		float Value_Float;
		int Value_Int;
	};
};

/*
	NOTE(dima): 
		Bitmap pixels should be stored right after this header

		Bitmap data byte size is equal to Width * Height * 4(bytes per pixel)
*/
struct gass_bitmap {
	u32 Width;
	u32 Height;
};

//Glyph bitmap pixels should be stored right after this header
struct gass_font_glyph {
	int Codepoint;

	u32 BitmapWidth;
	u32 BitmapHeight;

	float XOffset;
	float YOffset;
	float Advance;
	float LeftBearingX;

	float AtlasMinUV_x;
	float AtlasMinUV_y;
	float AtlasMaxUV_x;
	float AtlasMaxUV_y;

	u32 AtlasMinUV16x16y;
	u32 AtlasMaxUV16x16y;
};

/*
	NOTE(dima):

	Asset data for fonts stored next way:
	1) (sizeof(int) * MaxGlyphCount)				bytes of mapping data
	2) (sizeof(float) * GlyphCount * GlyphCount)	bytes of kerning data
	3) (AtlasWidth * AtlasHeight * 4)				bytes of atlas bitmap pixel data
	
*/
struct gass_font {
	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	u32 FirstGlyphID;

	int GlyphsCount;

	int MapRowCount;
	int MapLastRowIndex;

	u32 AtlasBitmapWidth;
	u32 AtlasBitmapHeight;

	u32 LineOffsetToMapping;
	u32 LineOffsetToKerningPairs;
	u32 LineOffsetToAtlasBitmapPixels;
};

enum gass_mesh_type {
	GassMeshType_Simple,
	GassMeshType_Skinned,
};


/*
	NOTE(dima):
	
	Asset data for fonts stored next way:
	1) VertexTypeSize * VerticesCount				bytes of vertices data
	2) sizeof(u32) * IndicesCount					bytes of indices data
*/
struct gass_mesh {
	u32 MeshType;

	u32 VertexTypeSize;
	u32 IndexTypeSize;

	u32 VerticesCount;
	u32 IndicesCount;

	u32 LineOffsetToVertices;
	u32 LineOffsetToIndices;
};

struct gass_header {
	u32 AssetType;

	u32 LineDataOffset;
	u32 LineFirstTagOffset;
	u32 TagCount;

	u32 AssetTotalDataSize;
	u32 AssetTotalTagsSize;

	u32 Pitch;

	union {
		gass_bitmap Bitmap;
		gass_font Font;
		gass_font_glyph Glyph;
		gass_mesh Mesh;
	};
};

inline u32 GASSGetLineOffsetForData() {
	u32 Result = sizeof(gass_header);

	return(Result);
}

struct asset_file_asset_group {
	u32 FirstAssetIndex;
	u32 GroupAssetCount;
};

struct asset_file_header{
	u8 AssetFileHeader[4];

	u32 Version;

	/*
		NOTE(dima): I store asset groups count here
		because later i will want to make sure that
		the asset file that i load is actual and 
		wont contain new assets group types that we dont't 
		have in our game...
	*/
	u32 AssetGroupsCount;

	u32 AssetsCount;

	u32 GroupsByteOffset;
	u32 LinesOffsetsByteOffset;
	u32 AssetLinesByteOffset;
};
#pragma pack(pop)

#endif