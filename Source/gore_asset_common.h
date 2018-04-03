#ifndef GORE_ASSET_COMMON_H_INCLUDED
#define GORE_ASSET_COMMON_H_INCLUDED

#define MAX_FONT_INFO_GLYPH_COUNT 256

struct rgba_buffer {
	u8* Pixels;

	u32 Width;
	u32 Height;
	v2 Align;

	u32 Pitch;

	float WidthOverHeight;

	void* TextureHandle;
};

struct glyph_info {
	int Codepoint;

	rgba_buffer Bitmap;

	int Width;
	int Height;

	/*Theese are offset from glyph origin to top-left of bitmap*/
	float XOffset;
	float YOffset;
	float Advance;
	float LeftBearingX;

	v2 AtlasMinUV;
	v2 AtlasMaxUV;
};

struct font_info {
	int CodepointToGlyphMapping[MAX_FONT_INFO_GLYPH_COUNT];

	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	int GlyphsCount;
	int* KerningPairs;
	glyph_info Glyphs[MAX_FONT_INFO_GLYPH_COUNT];

	rgba_buffer FontAtlasImage;
};

enum asset_type {
	AssetType_None,

	AssetType_Bitmap,
	AssetType_Sound,
	AssetType_Font,
};

struct asset_header {
	u32 AssetType;


};

#endif