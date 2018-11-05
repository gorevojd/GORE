#include "gore_font.h"

static rect2 PrintTextInternal(
	render_stack* RenderStack, 
	font_info* FontInfo, 
	u32 FontPrintOperationType, 
	char* Text, 
	v2 P, float Scale, 
	v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) 
{
	FUNCTION_TIMING();

	asset_system* AssetSystem = RenderStack->ParentRenderState->AssetSystem;

	rect2 TextRect = {};

	b32 IsPrint = (FontPrintOperationType == FontPrint_PrintText);
	b32 IsGetSize = (FontPrintOperationType == FontPrint_GetTextSize);

	v2 CurrentP = P;

	char* At = Text;

	RENDERPushBeginText(RenderStack, FontInfo);

	float MaxRowPX = 0.0f;
	float CurGlyphAdvance = 0.0f;

	while (*At) {
		b32 CharIsValid = (*At >= ' ') && (*At <= '~');
		int GlyphIndex = 0;
		if (CharIsValid) {
			GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		}

		u32 GlyphID = FontInfo->GlyphIDs[GlyphIndex];
		glyph_info* Glyph = GetGlyphFromID(AssetSystem, GlyphID);
		if (Glyph) {
			CurGlyphAdvance = Glyph->Advance;

			float BitmapScale = Glyph->Height * Scale;
		
			if (IsPrint && CharIsValid)
			{
				float BitmapMinY = CurrentP.y + (Glyph->YOffset - 1.0f) * Scale;
				float BitmapMinX = CurrentP.x + (Glyph->XOffset - 1.0f) * Scale;

				v2 BitmapDim = { Glyph->Bitmap.WidthOverHeight * BitmapScale, BitmapScale };

				//RENDERPushGlyph(Stack, *At, { BitmapMinX + 2, BitmapMinY + 2}, BitmapDim, V4(0.0f, 0.0f, 0.0f, 1.0f));
				bitmap_info* GlyphBitmap = 0;
#if 0
				GlyphBitmap = &Glyph->Bitmap;
#endif

				RENDERPushGlyph(
					RenderStack, 
					*At, 
					{ BitmapMinX, BitmapMinY }, 
					BitmapDim, 
					Glyph->AtlasMinUV,
					Glyph->AtlasMaxUV,
					GlyphBitmap,
					Color);
			}

			if (*At == '\t') {
				CurGlyphAdvance = Glyph->Advance * 2.0f;
			}

			float Kerning = 0.0f;
			char NextChar = *(At + 1);
			if (NextChar >= ' ' && NextChar <= '~') {
				Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
			}

			CurrentP.x += (CurGlyphAdvance + Kerning) * Scale;

		}
		++At;
	}

	if (CurrentP.x > MaxRowPX) {
		MaxRowPX = CurrentP.x;
	}

	RENDERPushEndText(RenderStack);

	TextRect.Min.x = P.x;
	TextRect.Min.y = P.y - FontInfo->AscenderHeight * Scale;
	TextRect.Max.x = MaxRowPX;
	TextRect.Max.y = CurrentP.y - FontInfo->DescenderHeight * Scale;

	return(TextRect);
}

rect2 PrintText(render_stack* Stack, font_info* FontInfo, char* Text, v2 P, float Scale, v4 Color) {
	rect2 Result = PrintTextInternal(Stack, FontInfo, FontPrint_PrintText, Text, P, Scale, Color);

	return(Result);
}

v2 GetTextSize(render_stack* Stack, font_info* FontInfo, char* Text, float Scale) {
	rect2 TextRc = PrintTextInternal(Stack, FontInfo, FontPrint_GetTextSize, Text, {}, Scale);

	v2 Result = GetRectDim(TextRc);
	return(Result);
}

rect2 PrintTextCenteredInRect(
	render_stack* Stack, 
	font_info* FontInfo, 
	char* Text, 
	rect2 Rect,
	float Scale, 
	v4 Color) 
{
	v2 TextDim = GetTextSize(Stack, FontInfo, Text, Scale);

	v2 RectDim = GetRectDim(Rect);

	v2 At;
	At.x = Rect.Min.x + (RectDim.x - TextDim.x) * 0.5f;
	At.y = Rect.Min.y + (RectDim.y + TextDim.y) * 0.5f;

	rect2 TextRc = PrintText(Stack, FontInfo, Text, At, Scale, Color);

	return(TextRc);
}