#ifndef GORE_FILE_FORMATS_H_INCLUDED
#define GORE_FILE_FORMATS_H_INCLUDED

#include "gore_types.h"

#define ASSET_FILE_VERSION 1
#define ASSET_FILE_HEADER ('G', 'A', 'S', 'S')

#pragma pack(push, 1)
struct gass_bitmap {
	u32 Width;
	u32 Height;

	u32 OffsetToPixelsData;
};

struct gass_font_glyph {
	int Codepoint;

	u32 BitmapWidth;
	u32 BitmapHeight;
	u32 OffsetToBitmapPixels;

	float XOffset;
	float YOffset;
	float Advance;
	float LeftBearingX;

	float AtlasMinUV_x;
	float AtlasMinUV_y;
	float AtlasMaxUV_x;
	float AtlasMaxUV_y;
};

struct gass_font {
	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	int GlyphsCount;

	u32 OffsetToKerningPairs;
	u32 OffsetToGlyphs;

	u32 AtlasBitmapWidth;
	u32 AtlasBitmapHeight;
	u32 OffsetToAtlasBitmapPixels;
};

struct gass_mesh {
	u32 MeshType;

	u32 VerticesCount;
	u32 IndicesCount;

	u32 OffsetToVertices;
	u32 OffsetToIndices;
};

struct gass_header {
	u32 AssetType;

	u32 FirstTagOffset;
	u32 TagCount;

	u32 GroupIndex;

	/*
		NOTE(dima): Asset data is written right
		after asset header. So we need actually
		know how big is it to advance to next asset
	*/
	u32 TotalAssetDataSize;

	union {
		gass_bitmap Bitmap;
		gass_font Font;
		gass_font_glyph Glyph;
	};
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
};
#pragma pack(pop)

#endif