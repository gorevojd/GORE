#ifndef ASSET_BUILDER_TOOL_H_INCLUDED
#define ASSET_BUILDER_TOOL_H_INCLUDED

#include "gore_asset_types.h"
#include "gore_math.h"
#include "gore_types.h"

#include "gore_asset_identifiers.h"
#include "gore_file_formats.h"
#include "gore_asset_common.h"

struct game_asset_group {
	u32 FirstAssetIndex;
	u32 GroupAssetCount;
};

#define MAX_TAGS_PER_ASSET 4
struct game_asset_tag {
	u32 Type;

	union {
		float Value_Float;
		int Value_Int;
	};
};

struct game_asset {
	u32 ID;

	u32 Type;

	game_asset_tag Tags[MAX_TAGS_PER_ASSET];
	int TagCount;

	union {
		bitmap_info* Bitmap;
		font_info* Font;
		sound_info* Sound;
		model_info* Model;
		mesh_info* Mesh;
		glyph_info* FontGlyph;
	};
};


/*
	NOTE(dima): Asset sources
*/

struct game_asset_source_bitmap {
	char* Path;

	bitmap_info* BitmapInfo;
};

struct game_asset_source_mesh {
	mesh_info* MeshInfo;
};

struct game_asset_source_model {
	model_info* ModelInfo;
};

struct game_asset_source_sound {
	char* Path;
};

struct game_asset_source_font {
	font_info* FontInfo;
};

struct game_asset_source_fontglyph {
	glyph_info* Glyph;
};

struct game_asset_source {
	union {
		game_asset_source_bitmap BitmapSource;
		game_asset_source_sound SoundSource;
		game_asset_source_font FontSource;
		game_asset_source_fontglyph FontGlyphSource;
		game_asset_source_model ModelSource;
		game_asset_source_mesh MeshSource;
	};
};


//NOTE(dima): Assets freeareas
#define FREEAREA_SLOTS_COUNT 4
struct game_asset_freearea {
	void* Pointers[FREEAREA_SLOTS_COUNT];
	int SetCount;
};


//NOTE(dima): Asset system
#define TEMP_STORED_ASSET_COUNT 2048
struct asset_system {
	u32 AssetTypes[TEMP_STORED_ASSET_COUNT];
	game_asset Assets[TEMP_STORED_ASSET_COUNT];
	game_asset_source AssetSources[TEMP_STORED_ASSET_COUNT];
	game_asset_freearea AssetFreeareas[TEMP_STORED_ASSET_COUNT];
	gass_header FileHeaders[TEMP_STORED_ASSET_COUNT];

	game_asset_group AssetGroups[GameAsset_Count];

	u32 AssetCount;
	game_asset_group* CurrentGroup;
	game_asset* PrevAssetPointer;
};

enum asset_load_font_from_image_flag {
	AssetLoadFontFromImage_None = 0,
	AssetLoadFontFromImage_InitLowercaseWithUppercase = 1,
};

enum asset_load_font_flags {
	AssetLoadFontFlag_None = 0,
	AssetLoadFontFlag_BakeOffsetShadows = 1,
};



font_info LoadFontInfoWithSTB(char* FontName, float Height = 14.0f, u32 Flags = 0);
font_info LoadFontInfoFromImage(char* ImagePath, int Height, int OneCharPixelWidth, int OneCharPixelHeight, u32 Flags);


#endif