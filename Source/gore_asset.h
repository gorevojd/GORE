#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_types.h"
#include "gore_asset_common.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 sound_id;
typedef u32 model_id;
typedef u32 mesh_id;

enum game_asset_state {
	GameAssetState_Unloaded,

	GameAssetState_InProgress,
	GameAssetState_Loaded,
};

struct game_asset_group {
	u32 FirstAssetIndex;
	u32 GroupAssetCount;
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

struct game_asset_source {
	union {
		game_asset_source_bitmap BitmapSource;
		game_asset_source_sound SoundSource;
		game_asset_source_font FontSource;
		game_asset_source_model ModelSource;
		game_asset_source_mesh MeshSource;
	};
};

#define MAX_TAGS_PER_ASSET 4
struct game_asset_tag {
	u32 Type;

	float Value_Float;
	int Value_Int;
};

struct game_asset {
	platform_atomic_type_u32 State;

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
	};
};

#define TEMP_STORED_ASSET_COUNT 2048
struct asset_system {
	stacked_memory* MemAllocPointer;

	u32 AssetTypes[TEMP_STORED_ASSET_COUNT];
	game_asset Assets[TEMP_STORED_ASSET_COUNT];

	game_asset_group AssetGroups[GameAsset_Count];

#if 1
	u32 AssetCount;
	game_asset_source AssetSources[TEMP_STORED_ASSET_COUNT];
	game_asset_group* CurrentGroup;
	game_asset* PrevAssetPointer;
#endif
};

void LoadAsset(asset_system* System, u32 Id, b32 Immediate);

bitmap_id GetFirstBitmap(asset_system* System, u32 GroupID);
sound_id GetFirstSound(asset_system* System, u32 GroupID);
font_id GetFirstFont(asset_system* System, u32 GroupID);
model_id GetFirstModel(asset_system* System, u32 GroupID);
mesh_id GetFirstMesh(asset_system* System, u32 GroupID);

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType);
u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType);

void ASSETSInit(asset_system* System, u32 MemorySizeForAssets);

inline game_asset* GetByAssetID(asset_system* System, u32 ID) {
	game_asset* Result = System->Assets + ID;

	return(Result);
}

/*
	NOTE(dima): Get asset from global asset structure
	with check if it has been loaded and with the check
	of the type of the asset
*/
inline game_asset* GetByAssetIDCheck(asset_system* System, u32 ID, u32 AssetType) {
	game_asset* Result = 0;

	game_asset* Asset = GetByAssetID(System, ID);
	if ((Asset->State == GameAssetState_Loaded) && 
		(Asset->Type == AssetType)) 
	{
		Result = Asset;
	}

	return(Result);
}

inline bitmap_info* GetBitmapFromID(asset_system* System, bitmap_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Bitmap);

	bitmap_info* Result = 0;
	if (Asset){
		Result = Asset->Bitmap;
	}

	return(Result);
}

inline sound_info* GetSoundFromID(asset_system* System, sound_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Sound);

	sound_info* Result = 0;
	if (Asset) {
		Result = Asset->Sound;
	}

	return(Result);
}

inline font_info* GetFontFromID(asset_system* System, font_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Font);

	font_info* Result = 0;
	if (Asset) {
		Result = Asset->Font;
	}

	return(Result);
}

inline mesh_info* GetMeshFromID(asset_system* System, mesh_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Mesh);

	mesh_info* Result = 0;
	if (Asset) {
		Result = Asset->Mesh;
	}

	return(Result);
}

inline model_info* GetModelFromID(asset_system* System, model_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_Model);

	model_info* Result = 0;
	if (Asset) {
		Result = Asset->Model;
	}

	return(Result);
}

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

inline float GetNextRowAdvance(font_info* Info, float SpacingMultiplier = 1.0f) {
	float Result = (Info->AscenderHeight - Info->DescenderHeight + Info->LineGap) * SpacingMultiplier;

	return(Result);
}

inline float GetKerningForCharPair(font_info* FontInfo, int FirstCodepoint, int SecondCodepoint) {
	float Result = 0;

	int FirstIndex = FontInfo->CodepointToGlyphMapping[FirstCodepoint];
	int SecondIndex = FontInfo->CodepointToGlyphMapping[SecondCodepoint];

	Result = FontInfo->KerningPairs[SecondIndex * FontInfo->GlyphsCount + FirstIndex];

	return(Result);
}


bitmap_info AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
void CopyRGBABuffer(bitmap_info* Dst, bitmap_info* Src);
void DeallocateRGBABuffer(bitmap_info* Buffer);

#endif