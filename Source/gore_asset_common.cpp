#include "gore_asset_common.h"

bitmap_info AllocateBitmap(u32 Width, u32 Height) {
	bitmap_info Result = {};

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;

	Result.Align.x = 0.0f;
	Result.Align.y = 0.0f;
	Result.WidthOverHeight = (float)Width / (float)Height;

	u32 MemoryForBufRequired = Width * Height * 4;
	Result.Pixels = (u8*)calloc(MemoryForBufRequired, 1);

	return(Result);
}

void CopyBitmap(bitmap_info* Dst, bitmap_info* Src) {
	Assert(Dst->Width == Src->Width);
	Assert(Dst->Height == Src->Height);

	u32* DestOut = (u32*)Dst->Pixels;
	u32* ScrPix = (u32*)Src->Pixels;
	for (int j = 0; j < Src->Height; j++) {
		for (int i = 0; i < Src->Width; i++) {
			*DestOut++ = *ScrPix++;
		}
	}
}

void DeallocateBitmap(bitmap_info* Buffer) {
	if (Buffer->Pixels) {
		free(Buffer->Pixels);
	}
}

bitmap_info LoadIMG(char* Path) {
	bitmap_info Result = {};

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

	stbi_image_free(ImageMemory);

	return(Result);
}

bitmap_info GenerateCheckerboardBitmap(
	int Width,
	int CellCountPerWidth,
	v3 FirstColor = V3(1.0f, 1.0f, 1.0f),
	v3 SecondColor = V3(0.0f, 0.0f, 0.0f))
{
	bitmap_info Result = {};

	if (CellCountPerWidth <= 1) {
		CellCountPerWidth = 2;
	}

	int OneCellWidth = Width / CellCountPerWidth;

	/*
	NOTE(dima): Result color has alpha of 1.
	So color don't need to be premultiplied
	*/
	u32 FirstColorPacked = PackRGBA(V4(FirstColor, 1.0f));
	u32 SecondColorPacked = PackRGBA(V4(SecondColor, 1.0f));

	Result.Pixels = (u8*)malloc(Width * Width * 4);
	Result.Pitch = Width * 4;
	Result.Width = Width;
	Result.Height = Width;
	Result.TextureHandle = 0;
	Result.WidthOverHeight = 1.0f;

	int X, Y;
	for (Y = 0; Y < Width; Y++) {

		int VertIndex = Y / OneCellWidth;

		for (X = 0; X < Width; X++) {
			int HorzIndex = X / OneCellWidth;

			u32 CellColor = ((HorzIndex + VertIndex) & 1) ? SecondColorPacked : FirstColorPacked;

			u32* Out = (u32*)Result.Pixels + Width * Y + X;

			*Out = CellColor;
		}
	}

	return(Result);
}