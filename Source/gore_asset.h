#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_common.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 sound_id;
typedef u32 model_id;

enum game_asset_id {
	GameAsset_Cube,
	GameAsset_Plane,

	GameAsset_DebugFont,
	GameAsset_GoldenFont,

	GameAsset_Count,
};

enum game_asset_state {
	GameAssetState_Unloaded,

	GameAssetState_InProgress,
	GameAssetState_Loaded,
};

struct game_asset {
	platform_atomic_type_u32 State;
	u32 Type;

	union {
		rgba_buffer* Bitmap;
		font_info* Font;
	};

	union {
		rgba_buffer Bitmap_;
		font_info Font_;
	};
};

struct game_asset_info {
	union {
		
	};
};

struct asset_system {
	stacked_memory* MemAllocPointer;

	game_asset Assets[2048];
};

extern void ASSETLoadBitmapAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadFontAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadModelAsset(asset_system* System, u32 Id, b32 Immediate);
extern void ASSETLoadSoundAsset(asset_system* System, u32 Id, b32 Immediate);

inline game_asset* ASSETGetByID(asset_system* System, u32 ID) {
	game_asset* Result = System->Assets + ID;

	return(Result);
}

inline font_info* ASSETRequestFont(asset_system* System, font_id Id) {
	font_info* Result = 0;

	game_asset* Asset = ASSETGetByID(System, Id);
	
	if (Asset->State == GameAssetState_Loaded) {
		Result = Asset->Font;
	}
	else {
		ASSETLoadFontAsset(System, Id, 0);
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


extern font_info LoadFontInfoWithSTB(char* FontName, float Height = 14.0f, u32 Flags = 0);
extern font_info LoadFontInfoFromImage(char* ImagePath, int Height, int OneCharPixelWidth, int OneCharPixelHeight, u32 Flags);
extern rgba_buffer LoadIMG(char* Path);

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


extern rgba_buffer AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
extern void CopyRGBABuffer(rgba_buffer* Dst, rgba_buffer* Src);
extern void DeallocateRGBABuffer(rgba_buffer* Buffer);

#endif