#include "gore_asset.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
font_info LoadFontInfoWithSTB(char* FontName, float Height) {
	font_info Result = {};
	stbtt_fontinfo FontInfo;

	float TempKerningPairs[MAX_FONT_INFO_GLYPH_COUNT * MAX_FONT_INFO_GLYPH_COUNT];

	data_buffer FontFileBuffer = ReadFileToDataBuffer(FontName);

	int AscenderHeight;
	int DescenderHeight;
	int LineGap;

	u32 AtlasHeight = 0;
	u32 AtlasWidth = 0;

	stbtt_InitFont(&FontInfo, FontFileBuffer.Data, 0);

	float Scale = stbtt_ScaleForPixelHeight(&FontInfo, Height);

	stbtt_GetFontVMetrics(
		&FontInfo,
		&AscenderHeight,
		&DescenderHeight,
		&LineGap);

	Result.AscenderHeight = (float)AscenderHeight * Scale;
	Result.DescenderHeight = (float)DescenderHeight * Scale;
	Result.LineGap = (float)LineGap * Scale;

	for (char Codepoint = ' '; Codepoint <= '~'; Codepoint++) {

		Result.CodepointToGlyphMapping[Codepoint] = Result.GlyphsCount;
		glyph_info* Glyph = &Result.Glyphs[Result.GlyphsCount++];

		int CharWidth;
		int CharHeight;
		int XOffset;
		int YOffset;
		int Advance;
		int LeftBearingX;

#if 1
		u8* Bitmap = stbtt_GetCodepointBitmap(
			&FontInfo,
			Scale, Scale,
			Codepoint,
			&CharWidth, &CharHeight,
			&XOffset, &YOffset);
#else
		float SubpixelShift = 0.5f;

		u8* Bitmap = stbtt_GetCodepointBitmapSubpixel(
			&FontInfo,
			Scale, Scale,
			SubpixelShift, SubpixelShift,
			Codepoint,
			&CharWidth, &CharHeight,
			&XOffset, &YOffset);
#endif

		stbtt_GetCodepointHMetrics(&FontInfo, Codepoint, &Advance, &LeftBearingX);

		Glyph->Width = CharWidth + 2;
		Glyph->Height = CharHeight + 2;
		Glyph->Bitmap = AllocateRGBABuffer(Glyph->Width, Glyph->Height);
		Glyph->Advance = Advance * Scale;
		Glyph->LeftBearingX = LeftBearingX * Scale;
		Glyph->XOffset = XOffset;
		Glyph->YOffset = YOffset;
		Glyph->Codepoint = Codepoint;

		Glyph->Bitmap.Align.x = 0.0f;  //TODO
		Glyph->Bitmap.Align.y = 0.0f;  //TODO

		AtlasWidth += Glyph->Width;
		AtlasHeight = Max(AtlasHeight, Glyph->Height);

		u32 CurrentX = 0;
		u32 CurrentY = 0;
		for (int Y = 0; Y < Glyph->Height; Y++) {
			CurrentX = 0;
			for (int X = 0; X < Glyph->Width; X++) {

				u32* Out = (u32*)(Glyph->Bitmap.Pixels + Y * Glyph->Bitmap.Pitch + X * 4);

				v4 ResultColor;

				if (((Y == 0) || (Y == (Glyph->Height - 1))) ||
					((X == 0) || (X == (Glyph->Width - 1))))
				{
					ResultColor = V4(1.0f, 1.0f, 1.0f, 0.0f);
				}
				else {
					u8 Grayscale = *((u8*)Bitmap + CurrentY * CharWidth + CurrentX);
					float GrayscaleFloat = (float)(Grayscale + 0.5f);
					float Grayscale01 = GrayscaleFloat / 255.0f;
					ResultColor = V4(1.0f, 1.0f, 1.0f, Grayscale01);
					CurrentX++;
				}

				/*Alpha premultiplication*/
				ResultColor.r *= ResultColor.a;
				ResultColor.g *= ResultColor.a;
				ResultColor.b *= ResultColor.a;

				u32 ColorValue = PackRGBA(ResultColor);
				*Out = ColorValue;
			}

			if ((Y != 0) && (Y != (Glyph->Height - 1))) {
				CurrentY++;
			}
		}

		stbtt_FreeBitmap(Bitmap, 0); /*???*/
	}

	//NOTE(dima): Processing kerning
	u32 KerningOneRowSize = sizeof(int) * Result.GlyphsCount;
	Result.KerningPairs = (int*)malloc(KerningOneRowSize * KerningOneRowSize);

	for (int FirstGlyphIndex = 0; FirstGlyphIndex < Result.GlyphsCount; FirstGlyphIndex++) {
		for (int SecondGlyphIndex = 0; SecondGlyphIndex < Result.GlyphsCount; SecondGlyphIndex++) {
			u32 KerningIndex = SecondGlyphIndex * Result.GlyphsCount + FirstGlyphIndex;

			int FirstCodepoint = Result.Glyphs[FirstGlyphIndex].Codepoint;
			int SecondCodepoint = Result.Glyphs[SecondGlyphIndex].Codepoint;

			int Kern = stbtt_GetGlyphKernAdvance(&FontInfo, FirstCodepoint, SecondCodepoint);

			Result.KerningPairs[KerningIndex] = (float)Kern * Scale;
		}
	}

	//NOTE(dima): Building font atlas
	Result.FontAtlasImage = AllocateRGBABuffer(AtlasWidth, AtlasHeight);

	float OneOverAtlasWidth = 1.0f / (float)AtlasWidth;
	float OneOverAtlasHeight = 1.0f / (float)AtlasHeight;

	u32 AtWidth = 0;
	for (int GlyphIndex = 0; GlyphIndex < Result.GlyphsCount; GlyphIndex++) {
		glyph_info* Glyph = &Result.Glyphs[GlyphIndex];

		for (int YIndex = 0; YIndex < Glyph->Height; YIndex++) {
			u32* At = (u32*)Glyph->Bitmap.Pixels + YIndex * Glyph->Width;
			u32* To = (u32*)Result.FontAtlasImage.Pixels + YIndex * AtlasWidth + AtWidth;
			for (int XIndex = 0; XIndex < Glyph->Width; XIndex++) {
				*To++ = *At++;
			}
		}

		Glyph->AtlasMinUV = V2((float)AtWidth * OneOverAtlasWidth, 0.0f);
		Glyph->AtlasMaxUV = V2((float)(AtWidth + Glyph->Width) * OneOverAtlasWidth, Glyph->Height * OneOverAtlasHeight);

		AtWidth += Glyph->Width;
	}

	FreeDataBuffer(&FontFileBuffer);

	return(Result);
}



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
rgba_buffer LoadIMG(char* Path) {
	rgba_buffer Result = {};

	int Width;
	int Height;
	int Channels;
	u8* ImageMemory = stbi_load(Path, &Width, &Height, &Channels, 4);

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;
	Result.WidthOverHeight = (float)Width / (float)Height;

	u32 RawImageSize = Width * Height * 4;
	u32 PixelsCount = Width * Height;
	Result.Pixels = (u8*)malloc(RawImageSize);

	for (u32 PixelIndex = 0;
		PixelIndex < PixelsCount;
		PixelIndex++)
	{
		u32 Value = *((u32*)ImageMemory + PixelIndex);

		u32 Color =
			((Value >> 24) & 0x000000FF) |
			((Value & 0xFF) << 24) |
			((Value & 0xFF00) << 8) |
			((Value & 0x00FF0000) >> 8);

		v4 Col = UnpackRGBA(Color);
		Col.r *= Col.a;
		Col.g *= Col.a;
		Col.b *= Col.a;
		Color = PackRGBA(Col);

		*((u32*)Result.Pixels + PixelIndex) = Color;
	}
#if 0
	else if (Channels == 3) {
		for (u32 PixelIndex = 0;
			PixelIndex < PixelsCount;
			PixelIndex++)
		{
			u8* Out = (u8*)((u32*)(Result.Pixels + PixelIndex));
			u8* Src = ImageMemory + PixelIndex * 3;

			for (int i = 0; i < 3; i++) {
				Out[i] = Src[i];
			}
			Out[3] = 0xFF;
		}
	}
#endif

	stbi_image_free(ImageMemory);

	return(Result);
}