#ifndef GORE_ASSET_H
#define GORE_ASSET_H

#include "gore_game_layer.h"

#define MAX_FONT_INFO_GLYPH_COUNT 256

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
};

struct font_info {
	int CodepointToGlyphMapping[MAX_FONT_INFO_GLYPH_COUNT];

	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	int GlyphsCount;
	int* KerningPairs;
	glyph_info Glyphs[MAX_FONT_INFO_GLYPH_COUNT];
};

extern font_info LoadFontInfoWithSTB(char* FontName, float Height = 14.0f);
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
#endif