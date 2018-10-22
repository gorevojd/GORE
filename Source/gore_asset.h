#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_types.h"
#include "gore_asset_common.h"
#include "gore_game_settings.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 sound_id;
typedef u32 model_id;
typedef u32 mesh_id;
typedef u32 voxel_atlas_id;

enum game_asset_state {
	GameAssetState_Unloaded,

	GameAssetState_InProgress,
	GameAssetState_Loaded,
};

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

#define TEMP_STORED_ASSET_COUNT 2048
struct asset_system {
	stacked_memory* MemAllocPointer;

	game_asset Assets[TEMP_STORED_ASSET_COUNT];
	game_asset_group AssetGroups[GameAsset_Count];
};

void LoadAsset(asset_system* System, u32 Id, b32 Immediate);

bitmap_id GetFirstBitmap(asset_system* System, u32 GroupID);
sound_id GetFirstSound(asset_system* System, u32 GroupID);
font_id GetFirstFont(asset_system* System, u32 GroupID);
model_id GetFirstModel(asset_system* System, u32 GroupID);
mesh_id GetFirstMesh(asset_system* System, u32 GroupID);
voxel_atlas_id GetFirstVoxelAtlas(asset_system* System, u32 GroupID);

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType);
u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType);

void ASSETSInit(asset_system* System);

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

inline voxel_atlas_info* GetVoxelAtlasFromID(asset_system* System, voxel_atlas_id ID) {
	game_asset* Asset = GetByAssetIDCheck(System, ID, AssetType_VoxelAtlas);

	voxel_atlas_info* Result = 0;
	if (Asset) {
		Result = Asset->VoxelAtlas;
	}

	return(Result);
}

#endif