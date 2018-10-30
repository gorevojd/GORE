#ifndef GORE_FONT_H_INCLUDED
#define GORE_FONT_H_INCLUDED

#include "gore_math.h"
#include "gore_render_state.h"
#include "gore_asset_types.h"

enum font_print_operation_type {
	FontPrint_PrintText,
	FontPrint_GetTextSize,
};

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

rect2 PrintText(render_stack* Stack, font_info* FontInfo, char* Text, v2 P, float Scale, v4 Color);
v2 GetTextSize(render_stack* Stack, font_info* FontInfo, char* Text, float Scale);
rect2 PrintTextCenteredInRect(
	render_stack* Stack,
	font_info* FontInfo,
	char* Text,
	rect2 Rect,
	float Scale,
	v4 Color);

#endif