#ifndef ASSET_BUILDER_TOOL_H_INCLUDED
#define ASSET_BUILDER_TOOL_H_INCLUDED

#include "gore_types.h"
#include "gore_math.h"
#include "gore_asset_types.h"

#include "gore_asset_identifiers.h"


struct game_asset_group {

	u32 FirstAssetIndex;
	u32 GroupAssetCount;
};

#define MAX_TAGS_PER_ASSET 4
struct game_asset_tag {
	u32 Type;

	float Value_Float;
	int Value_Int;
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
		voxel_atlas_info* VoxelAtlas;
	};

	/*
	TODO(dima): Its important here is that it's done through the union.
	If you imagine that we store only bitmaps that have a little
	memory to deal with(sizeof(bitmap_info)), than everything is ok.
	BUT THATS NOT. We will always have memory as big as the biggest
	element in the union.. I will need to make better memory management
	on assets code as soon as i can...
	*/

	union {
		bitmap_info Bitmap_;
		font_info Font_;
		sound_info Sound_;
		model_info Model_;
		mesh_info Mesh_;
		voxel_atlas_info VoxelAtlas_;
	};
};

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
	char* Path;

	int Height;

	b32 LoadFromImage;
	int OneCharWidth;
	int OneCharHeight;

	u32 Flags;

	font_info* FontInfo;
};

struct game_asset_source_voxel_atlas {
	voxel_atlas_info* Info;
};

struct game_asset_source {
	union {
		game_asset_source_bitmap BitmapSource;
		game_asset_source_sound SoundSource;
		game_asset_source_font FontSource;
		game_asset_source_model ModelSource;
		game_asset_source_mesh MeshSource;
		game_asset_source_voxel_atlas VoxelAtlasSource;
	};
};

#define TEMP_STORED_ASSET_COUNT 2048
struct asset_system {
	u32 AssetTypes[TEMP_STORED_ASSET_COUNT];
	game_asset Assets[TEMP_STORED_ASSET_COUNT];
	game_asset_source AssetSources[TEMP_STORED_ASSET_COUNT];

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

enum load_mesh_vertex_layout {
	MeshVertexLayout_PUV,
	MeshVertexLayout_PUVN,
	MeshVertexLayout_PNUV,
	MeshVertexLayout_PUVNC,
	MeshVertexLayout_PNUVC,
};

font_info LoadFontInfoWithSTB(char* FontName, float Height = 14.0f, u32 Flags = 0);
font_info LoadFontInfoFromImage(char* ImagePath, int Height, int OneCharPixelWidth, int OneCharPixelHeight, u32 Flags);
bitmap_info LoadIMG(char* Path);
mesh_info LoadMeshFromVertices(
	float* Verts, u32 VertsCount,
	u32* Indices, u32 IndicesCount,
	u32 VertexLayout,
	b32 CalculateNormals = 0,
	b32 CalculateTangents = 0);

bitmap_info AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
void CopyRGBABuffer(bitmap_info* Dst, bitmap_info* Src);
void DeallocateRGBABuffer(bitmap_info* Buffer);

#endif