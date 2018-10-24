#ifndef GORE_FILE_FORMATS_H_INCLUDED
#define GORE_FILE_FORMATS_H_INCLUDED

#include "gore_types.h"

#define ASSET_FILE_VERSION 1

#pragma pack(push, 1)

/*
	//NOTE(dima): asset data is stored in the next pattern

	(Asset file header)
	(Asset1 header) | (Asset1 data) | (Asset1 tags) -> Line. 
	(Asset2 header) | (Asset2 data) | (Asset2 tags)
	(Asset3 header) | (Asset3 data) | (Asset3 tags)
	(Asset4 header) | (Asset4 data) | (Asset4 tags)
	(Asset5 header) | (Asset5 data) | (Asset5 tags)
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

//Bitmap pixels should be stored right after this header
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
};

struct gass_font {
	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	u32 FirstGlyphID;
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

	u32 LineFirstTagOffset;
	u32 TagCount;

	u32 GroupIndex;

	u32 Pitch;

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