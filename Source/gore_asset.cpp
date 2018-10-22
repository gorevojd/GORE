#include "gore_asset.h"
#include "gore_voxshared.h"

#include <stdio.h>

struct asset_load_font_asset_work {

};

PLATFORM_THREADWORK_CALLBACK(ASSETLoadFontAssetWork) {
	asset_load_font_asset_work* Work = (asset_load_font_asset_work*)Data;

	
}

/*
	NOTE(dima): Thanks to Minsk Gameloft office programming
	teamlead Kirill who found the mistake here xD.
	This is my forever-ever mistake that I had and will 
	always do in atomic operations - forget to put & in front
	of the first parameter. Lol
*/

void LoadAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = GetByAssetID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)&Asset->State, 
		GameAssetState_InProgress, 
		GameAssetState_Unloaded) == GameAssetState_Unloaded) 
	{

	}
	else {

	}
}

inline game_asset_tag* FindTagInAsset(game_asset* Asset, u32 TagType) {
	game_asset_tag* Result = 0;

	for (int TagIndex = 0;
		TagIndex < Asset->TagCount;
		TagIndex++)
	{
		game_asset_tag* Tag = Asset->Tags + TagIndex;
		if (Tag->Type == TagType) {
			Result = Tag;
			break;
		}
	}

	return(Result);
}

static game_asset* GetAssetByBestFloatTagInternal(asset_system* System, u32 GroupID, u32 TagType, float TagValue) {
	FUNCTION_TIMING();

	game_asset* Result = 0;

	game_asset_group* Group = &System->AssetGroups[GroupID];

	//NOTE(dima): Should be added with Group first asset index
	int BestMatchAssetIndex = 0;
	float BestMatchDiff = 1000000000.0f;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			float Diff = Tag->Value_Float - TagValue;
			//NOTE(dima): Getting absolute value of the diff
			if (Diff < 0.0f) {
				Diff = -Diff;
			}

			if (Diff < BestMatchDiff) {
				BestMatchAssetIndex = AssetIndex;
				BestMatchDiff = Diff;
			}
		}
	}

	int ResultAssetIndex = Group->FirstAssetIndex + BestMatchAssetIndex;
	Result = System->Assets + ResultAssetIndex;

	return(Result);
}

static game_asset* GetAssetByBestIntTagInternal(asset_system* System, u32 GroupID, u32 TagType, int TagValue) {
	FUNCTION_TIMING();

	game_asset* Result = 0;

	game_asset_group* Group = &System->AssetGroups[GroupID];

	//NOTE(dima): Should be added with Group first asset index
	int BestMatchAssetIndex = 0;
	int BestMatchDiff = 0x7FFFFFFF;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			int Diff = Tag->Value_Int - TagValue;
			//NOTE(dima): Getting absolute value of the diff
			if (Diff < 0) {
				Diff = -Diff;
			}

			if (Diff < BestMatchDiff) {
				BestMatchAssetIndex = AssetIndex;
				BestMatchDiff = Diff;
			}
		}
	}

	int ResultAssetIndex = Group->FirstAssetIndex + BestMatchAssetIndex;
	Result = System->Assets + ResultAssetIndex;

	return(Result);
}

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestFloatTagInternal(System, GroupID, TagType, TagValue);

	Assert(Asset->Type == AssetType);
	u32 Result = Asset->ID;

	return(Result);
}

u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestIntTagInternal(System, GroupID, TagType, TagValue);

	Assert(Asset->Type == AssetType);
	u32 Result = Asset->ID;

	return(Result);
}

inline game_asset* GetFirstAssetInternal(asset_system* System, u32 GroupID) {
	u32 TargetAssetIndex = System->AssetGroups[GroupID].FirstAssetIndex;
	game_asset* Result = &System->Assets[TargetAssetIndex];

	return(Result);
}

u32 GetFirstAsset(asset_system* System, u32 GroupID, u32 AssetType) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType);

	u32 Result = FirstAsset->ID;

	return(Result);
}

bitmap_id GetFirstBitmap(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Bitmap);

	u32 Result = FirstAsset->ID;

	return(Result);
}

sound_id GetFirstSound(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Sound);

	u32 Result = FirstAsset->ID;

	return(Result);
}

font_id GetFirstFont(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Font);

	u32 Result = FirstAsset->ID;

	return(Result);
}

model_id GetFirstModel(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Model);

	u32 Result = FirstAsset->ID;

	return(Result);
}

mesh_id GetFirstMesh(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Mesh);

	u32 Result = FirstAsset->ID;

	return(Result);
}

voxel_atlas_id GetFirstVoxelAtlas(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = GetFirstAssetInternal(System, GroupID);

	Assert(FirstAsset->Type == AssetType_VoxelAtlas);

	u32 Result = FirstAsset->ID;

	return(Result);
}

void ASSETSInit(asset_system* System) {

	
}
