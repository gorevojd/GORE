#ifndef GORE_ASSET_H_INCLUDED
#define GORE_ASSET_H_INCLUDED

#include "gore_platform.h"

#include "gore_asset_common.h"

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