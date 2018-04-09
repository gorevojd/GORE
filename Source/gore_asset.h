#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_types.h"
#include "gore_asset_common.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 sound_id;
typedef u32 model_id;


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

	//font_info* FontInfo;
};

struct game_asset_source {
	union {
		game_asset_source_bitmap BitmapSource;
		game_asset_source_model ModelSource;
		game_asset_source_sound SoundSource;
		game_asset_source_font FontSource;
	};
};

struct game_asset {
	platform_atomic_type_u32 State;

	u32 Type;

	union {
		bitmap_info* Bitmap;
		font_info* Font;
		sound_info* Sound;
		model_info* Model;
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
#endif
};

extern void ASSETLoadBitmapAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadFontAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadModelAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadSoundAsset(asset_system* System, u32 Id, b32 Immediate);

extern bitmap_info* ASSETRequestFirstBitmap(asset_system* System, u32 AssetID);
extern sound_info* ASSETRequestFirstSound(asset_system* System, u32 AssetID);
extern font_info* ASSETRequestFirstFont(asset_system* System, u32 AssetID);
extern model_info* ASSETRequestFirstModel(asset_system* System, u32 AssetID);

extern void ASSETSInit(asset_system* System, u32 MemorySizeForAssets);

inline game_asset* ASSETGetByID(asset_system* System, u32 ID) {
	game_asset* Result = System->Assets + ID;

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


extern font_info LoadFontInfoWithSTB(char* FontName, float Height = 14.0f, u32 Flags = 0);
extern font_info LoadFontInfoFromImage(char* ImagePath, int Height, int OneCharPixelWidth, int OneCharPixelHeight, u32 Flags);
extern bitmap_info LoadIMG(char* Path);

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


extern bitmap_info AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
extern void CopyRGBABuffer(bitmap_info* Dst, bitmap_info* Src);
extern void DeallocateRGBABuffer(bitmap_info* Buffer);

#endif