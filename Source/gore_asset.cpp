#include "gore_asset.h"

#include <stdio.h>

struct asset_load_font_asset_work {

};

PLATFORM_THREADWORK_CALLBACK(ASSETLoadFontAssetWork) {
	asset_load_font_asset_work* Work = (asset_load_font_asset_work*)Data;

	
}

void ASSETLoadFontAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = ASSETGetByID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)Asset->State, 
		GameAssetState_InProgress, 
		GameAssetState_Unloaded) == GameAssetState_Unloaded) 
	{

	}
	else {

	}
}

void ASSETLoadBitmapAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = ASSETGetByID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)Asset->State,
		GameAssetState_InProgress,
		GameAssetState_Unloaded) == GameAssetState_Unloaded)
	{

	}
	else {

	}
}

void ASSETLoadModelAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = ASSETGetByID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)Asset->State,
		GameAssetState_InProgress,
		GameAssetState_Unloaded) == GameAssetState_Unloaded)
	{

	}
	else {

	}
}

void ASSETLoadMeshAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = ASSETGetByID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)Asset->State,
		GameAssetState_InProgress,
		GameAssetState_Unloaded) == GameAssetState_Unloaded)
	{

	}
	else {

	}
}

void ASSETLoadSoundAsset(asset_system* System, u32 Id, b32 Immediate) {
	game_asset* Asset = ASSETGetByID(System, Id);

	if (PlatformApi.AtomicCAS_U32(
		(platform_atomic_type_u32*)Asset->State,
		GameAssetState_InProgress,
		GameAssetState_Unloaded) == GameAssetState_Unloaded)
	{

	}
	else {

	}
}


inline game_asset* ASSETRequestFirstAsset(asset_system* System, u32 GroupID) {
	u32 TargetAssetIndex = System->AssetGroups[GroupID].FirstAssetIndex;
	game_asset* Result = &System->Assets[TargetAssetIndex];

	return(Result);
}

bitmap_info* ASSETRequestFirstBitmap(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = ASSETRequestFirstAsset(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Bitmap);

	bitmap_info* Result = FirstAsset->Bitmap;

	return(Result);
}

sound_info* ASSETRequestFirstSound(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = ASSETRequestFirstAsset(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Sound);

	sound_info* Result = FirstAsset->Sound;

	return(Result);
}

font_info* ASSETRequestFirstFont(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = ASSETRequestFirstAsset(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Font);

	font_info* Result = FirstAsset->Font;

	return(Result);
}

model_info* ASSETRequestFirstModel(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = ASSETRequestFirstAsset(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Model);

	model_info* Result = FirstAsset->Model;

	return(Result);
}

mesh_info* ASSETRequestFirstMesh(asset_system* System, u32 GroupID) {
	game_asset* FirstAsset = ASSETRequestFirstAsset(System, GroupID);

	Assert(FirstAsset->Type == AssetType_Mesh);

	mesh_info* Result = FirstAsset->Mesh;

	return(Result);
}

struct added_asset {
	game_asset* Asset;
	game_asset_source* Source;
};

static added_asset AddAsset(asset_system* System, u32 AssetType) {
	added_asset Result = {};

	Assert(System->CurrentGroup != 0);
	game_asset_group* Group = System->CurrentGroup;

	Group->GroupAssetCount++;

	u32 AssetIndex = System->AssetCount;
	Result.Asset = System->Assets + AssetIndex;
	Result.Asset->Type = AssetType;
	Result.Source = System->AssetSources + AssetIndex;
	System->AssetTypes[AssetIndex] = AssetType;

	++System->AssetCount;

	return(Result);
}

static void BeginAssetGroup(asset_system* System, u32 AssetID) {
	System->CurrentGroup = System->AssetGroups + AssetID;

	game_asset_group* Group = System->CurrentGroup;

	Group->FirstAssetIndex = System->AssetCount;
	Group->GroupAssetCount = 0;
}

static void EndAssetGroup(asset_system* System) {
	Assert(System->CurrentGroup);
	game_asset_group* Group = System->CurrentGroup;

	System->CurrentGroup = 0;
}

static void AddBitmapAsset(asset_system* System, char* Path) {
	added_asset Added = AddAsset(System, AssetType_Bitmap);

	game_asset_source* Source = Added.Source;
	Source->BitmapSource.Path = Path;
}

static void AddSoundAsset(asset_system* System, char* Path) {
	added_asset Added = AddAsset(System, AssetType_Sound);

	game_asset_source* Source = Added.Source;
	Source->SoundSource.Path = Path;


}

static void AddModelAsset(asset_system* System, model_info* Model) {
	added_asset Added = AddAsset(System, AssetType_Model);

	game_asset_source* Source = Added.Source;
	Source->ModelSource.ModelInfo = Model;
}

static void AddMeshAsset(asset_system* System, mesh_info* Mesh) {
	added_asset Added = AddAsset(System, AssetType_Mesh);

	game_asset_source* Source = Added.Source;
	Source->MeshSource.MeshInfo = Mesh;
}

static void AddFontAsset(
	asset_system* System, 
	char* Path, 
	int Height,
	b32 LoadFromImage, 
	int OneCharWidth, 
	int OneCharHeight, 
	u32 Flags) 
{
	added_asset Added = AddAsset(System, AssetType_Font);

	game_asset_source* Source = Added.Source;
	Source->FontSource.Path = Path;
	Source->FontSource.Height = Height;
	Source->FontSource.LoadFromImage = LoadFromImage;
	Source->FontSource.OneCharWidth = OneCharWidth;
	Source->FontSource.OneCharHeight = OneCharHeight;
	Source->FontSource.Flags = Flags;
	Source->FontSource.FontInfo = 0;
}

static void AddFontAssetManual(
	asset_system* System, 
	font_info* FontInfo) 
{
	added_asset Added = AddAsset(System, AssetType_Font);

	game_asset_source* Source = Added.Source;
	Source->FontSource.FontInfo = FontInfo;
}

/*	
	NOTE(dima): Code below is temporary here.
	It will be removed from here when i will 
	create asset packer
*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct data_buffer {
	u8* Data;
	u64 Size;
};

data_buffer ReadFileToDataBuffer(char* FileName) {
	data_buffer Result = {};

	FILE* fp = fopen(FileName, "rb");
	if (fp) {
		fseek(fp, 0, 2);
		u64 FileSize = ftell(fp);
		fseek(fp, 0, 0);

		Result.Size = FileSize;
		Result.Data = (u8*)calloc(FileSize, 1);

		fread(Result.Data, 1, FileSize, fp);

		fclose(fp);
	}

	return(Result);
}

void FreeDataBuffer(data_buffer* DataBuffer) {
	if (DataBuffer->Data) {
		free(DataBuffer->Data);
	}
}


#ifndef GET_ALIGN_OFFSET
#define GET_ALIGN_OFFSET(val, align) (((align) - ((size_t)val & ((align) - 1))) & (Align - 1))
#endif

bitmap_info AllocateRGBABuffer(u32 Width, u32 Height, u32 Align) {
	bitmap_info Result = {};

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;

	Result.Align.x = 0.0f;
	Result.Align.y = 0.0f;
	Result.WidthOverHeight = (float)Width / (float)Height;

	u32 MemoryForBufRequired = Width * Height * 4;
	u32 AlignedMemoryBufSize = MemoryForBufRequired + Align;
	Result.Pixels = (u8*)calloc(AlignedMemoryBufSize, 1);

	u32 AlignOffset = GET_ALIGN_OFFSET(Result.Pixels, Align);
	Result.Pixels += AlignOffset;

	return(Result);
}

void CopyRGBABuffer(bitmap_info* Dst, bitmap_info* Src) {
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

void DeallocateRGBABuffer(bitmap_info* Buffer) {
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



font_info LoadFontInfoFromImage(
	char* ImagePath, 
	int Height,
	int OneCharPixelWidth, 
	int OneCharPixelHeight,
	u32 Flags) 
{
	font_info Result = {};
	
	float Scale = (float)Height / (float)OneCharPixelHeight;

	int TargetCharWidth = (int)((float)OneCharPixelWidth * Scale);
	int TargetCharHeight = (int)Height;

	float OneOverSrcWidth = 1.0f / (float)OneCharPixelWidth;
	float OneOverSrcHeight = 1.0f / (float)OneCharPixelHeight;

	float OneOverTargetWidth = 1.0f / (float)TargetCharWidth;
	float OneOverTargetHeight = 1.0f / (float)TargetCharHeight;

	u32 AtlasHeight = 0;
	u32 AtlasWidth = 0;

	//NOTE(dima): Loading font atlas image
	bitmap_info FontImage = LoadIMG(ImagePath);

	Result.AscenderHeight = TargetCharHeight;
	Result.DescenderHeight = 0;
	Result.LineGap = 0;

	//NOTE(dima): Initializing all codepoints
	for (int i = ' '; i <= '~'; i++) {
		Result.CodepointToGlyphMapping[i] = Result.GlyphsCount;
		glyph_info* Glyph = &Result.Glyphs[Result.GlyphsCount++];

		Glyph->Advance = TargetCharWidth;

		Glyph->AtlasMinUV = V2(0.0f, 0.0f);
		Glyph->AtlasMaxUV = V2(0.0f, 0.0f);
		Glyph->LeftBearingX = 0;
		Glyph->Codepoint = i;

		Glyph->Width = TargetCharWidth + 2;
		Glyph->Height = TargetCharHeight + 2;

		Glyph->XOffset = 0.0f;
		Glyph->YOffset = -TargetCharHeight;

		Glyph->Bitmap = AllocateRGBABuffer(Glyph->Width, Glyph->Height);

		//NOTE(dima): Initializing every single bitmap to empty
		u32* DstPixel = (u32*)Glyph->Bitmap.Pixels;
		for (int y = 0; y < Glyph->Height; y++) {
			for (int x = 0; x < Glyph->Width; x++) {
				DstPixel = 0;
			}
		}

		//NOTE(dima): Increasing atlas size
		AtlasWidth += Glyph->Width;
		AtlasHeight = Max(AtlasHeight, Glyph->Height);
	}

	//NOTE(dima): Loading chars
	int PixelAtX = 0;
	int CodePoint = ' ';
	int BigACodepointIndex = 0;
	for (PixelAtX; PixelAtX + OneCharPixelWidth <= FontImage.Width; PixelAtX += OneCharPixelWidth, CodePoint++) {

		if (CodePoint == 'A') {
			BigACodepointIndex = CodePoint;
		}

		glyph_info* Glyph = &Result.Glyphs[Result.CodepointToGlyphMapping[CodePoint]];

		for (int y = 1; y < TargetCharHeight + 1; y++) {
			
			int SrcY = (int)((float)(y - 1) * OneOverTargetHeight * (float)OneCharPixelHeight);
			
			for (int x = 1; x < TargetCharWidth + 1; x++) {
				int SrcX = (int)((float)(x - 1) * OneOverTargetWidth * (float)OneCharPixelWidth);

				u32* SrcPixel = (u32*)FontImage.Pixels + SrcY * FontImage.Width + PixelAtX + SrcX;
				u32* TargetPixel = (u32*)Glyph->Bitmap.Pixels + y * Glyph->Width + x;

				*TargetPixel = *SrcPixel;
			}
		}
	}

	//NOTE(dima): If lowercase letters wasn't loaded then init them with uppercase
	if (Flags & AssetLoadFontFromImage_InitLowercaseWithUppercase) {
		if (CodePoint > 'Z') {
			int TempCodepointIndex = 0;
			for (TempCodepointIndex = 'a'; TempCodepointIndex < 'z'; TempCodepointIndex++) {
				glyph_info* SrcGlyph = &Result.Glyphs[Result.CodepointToGlyphMapping[TempCodepointIndex - 'a' + 'A']];
				glyph_info* Glyph = &Result.Glyphs[Result.CodepointToGlyphMapping[TempCodepointIndex]];
				CopyRGBABuffer(&Glyph->Bitmap, &SrcGlyph->Bitmap);
			}
		}
	}

	//NOTE(dima): Processing kerning
	u32 KerningOneRowSize = sizeof(int) * Result.GlyphsCount;
	Result.KerningPairs = (int*)malloc(KerningOneRowSize * KerningOneRowSize);

	for (int FirstGlyphIndex = 0; FirstGlyphIndex < Result.GlyphsCount; FirstGlyphIndex++) {
		for (int SecondGlyphIndex = 0; SecondGlyphIndex < Result.GlyphsCount; SecondGlyphIndex++) {
			u32 KerningIndex = SecondGlyphIndex * Result.GlyphsCount + FirstGlyphIndex;

			Result.KerningPairs[KerningIndex] = 0;
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

	//NOTE(dima): Freing font image
	DeallocateRGBABuffer(&FontImage);

	return(Result);
}

font_info LoadFontInfoWithSTB(char* FontName, float Height, u32 Flags) {
	font_info Result = {};
	stbtt_fontinfo FontInfo;

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

		int ShadowOffset = 0;
		if (Flags & AssetLoadFontFlag_BakeOffsetShadows) {
			ShadowOffset = 2;
		}

		Glyph->Width = CharWidth + 2 + ShadowOffset;
		Glyph->Height = CharHeight + 2 + ShadowOffset;
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

		//NOTE(dima): Clearing the image bytes
		u32* Pixel = (u32*)Glyph->Bitmap.Pixels;
		for (int Y = 0; Y < Glyph->Height; Y++) {
			for (int X = 0; X < Glyph->Width; X++) {
				*Pixel++ = 0;
			}
		}
		

		u32 SrcX = 0;
		u32 SrcY = 0;

		//NOTE(dima): First - render shadow if needed
		if (Flags & AssetLoadFontFlag_BakeOffsetShadows) {
			for (int Y = 1 + ShadowOffset; Y < Glyph->Height - 1; Y++) {
				SrcX = 0;
				u32* Pixel = (u32*)Glyph->Bitmap.Pixels + Glyph->Bitmap.Width * Y;
				for (int X = 1 + ShadowOffset; X < Glyph->Width - 1; X++) {
					u32* Out = (u32*)(Glyph->Bitmap.Pixels + Y * Glyph->Bitmap.Pitch + X * 4);

					u8 Grayscale = *((u8*)Bitmap + SrcY * CharWidth + SrcX);
					float GrayscaleFloat = (float)(Grayscale + 0.5f);
					float Grayscale01 = GrayscaleFloat / 255.0f;

					v4 ResultColor = V4(0.0f, 0.0f, 0.0f, Grayscale01);

					/*Alpha premultiplication*/
					ResultColor.r *= ResultColor.a;
					ResultColor.g *= ResultColor.a;
					ResultColor.b *= ResultColor.a;

					u32 ColorValue = PackRGBA(ResultColor);
					*Out = ColorValue;

					SrcX++;
				}
				SrcY++;
			}
		}

		SrcX = 0;
		SrcY = 0;

		//NOTE(dima): Then render actual glyph
		for (int Y = 1; Y < Glyph->Height - 1 - ShadowOffset; Y++) {
			SrcX = 0;
			for (int X = 1; X < Glyph->Width - 1 - ShadowOffset; X++) {

				u32* Out = (u32*)Glyph->Bitmap.Pixels + Y * Glyph->Width + X;
				v4 DstInitColor = UnpackRGBA(*Out);

				u8 Grayscale = *((u8*)Bitmap + SrcY * CharWidth + SrcX);
				float GrayscaleFloat = (float)(Grayscale + 0.5f);
				float Grayscale01 = GrayscaleFloat / 255.0f;

				v4 ResultColor = V4(1.0f, 1.0f, 1.0f, Grayscale01);

				//NOTE(dima): alpha premultiplication
				ResultColor.r *= ResultColor.a;
				ResultColor.g *= ResultColor.a;
				ResultColor.b *= ResultColor.a;

				//NOTE(dima): Calculating blend alpha value
				float BlendAlpha = ResultColor.a;
				
				ResultColor.x = ResultColor.x + DstInitColor.x * (1.0f - BlendAlpha);
				ResultColor.y = ResultColor.y + DstInitColor.y * (1.0f - BlendAlpha);
				ResultColor.z = ResultColor.z + DstInitColor.z * (1.0f - BlendAlpha);
				ResultColor.a = ResultColor.a + DstInitColor.a - ResultColor.a * DstInitColor.a;

				u32 ColorValue = PackRGBA(ResultColor);
				*Out = ColorValue;

				SrcX++;
			}

			SrcY++;
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

enum load_mesh_vertex_layout {
	MeshVertexLayout_PUV,
	MeshVertexLayout_PUVN,
	MeshVertexLayout_PNUV,
	MeshVertexLayout_PUVNC,
	MeshVertexLayout_PNUVC,
};

inline vertex_info LoadMeshVertex(float* Pos, float* Tex, float* Norm, float* Color) {
	vertex_info Result = {};

	if (Pos) {
		Result.P.x = *Pos;
		Result.P.y = *(Pos + 1);
		Result.P.z = *(Pos + 2);
	}

	if (Tex) {
		Result.UV.x = *Tex;
		Result.UV.y = *(Tex + 1);
	}

	if (Norm) {
		Result.N.x = *Norm;
		Result.N.y = *(Norm + 1);
		Result.N.z = *(Norm + 2);
	}

	if (Color) {
		Result.C.x = *Color;
		Result.C.y = *(Color + 1);
		Result.C.z = *(Color + 2);
	}

	return(Result);
}

mesh_info LoadMeshFromVertices(
	float* Verts, u32 VertsCount,
	u32* Indices, u32 IndicesCount,
	u32 VertexLayout,
	b32 CalculateNormals,
	b32 CalculateTangents) 
{
	mesh_info Result = {};

	Result.Handle = 0;
	Result.IndicesCount = IndicesCount;
	Result.Indices = (u32*)malloc(IndicesCount * sizeof(u32));
	for (int IndexIndex = 0;
		IndexIndex < IndicesCount;
		IndexIndex++)
	{
		Result.Indices[IndexIndex] = Indices[IndexIndex];
	}

	u32 Increment = 0;
	switch (VertexLayout) {
		case MeshVertexLayout_PUV: {
			Increment = 5;
			Result.MeshType = MeshType_Simple;
		}break;

		case MeshVertexLayout_PNUV:
		case MeshVertexLayout_PUVN: {
			Increment = 8;
			Result.MeshType = MeshType_Simple;
		}break;

		case MeshVertexLayout_PUVNC:
		case MeshVertexLayout_PNUVC: {
			Increment = 11;
			Result.MeshType = MeshType_Simple;
		}break;
	}

	Result.VerticesCount = VertsCount;
	Result.Vertices = (vertex_info*)malloc(sizeof(vertex_info) * VertsCount);

	float *VertexAt = Verts;
	for (int VertexIndex = 0;
		VertexIndex < VertsCount;
		VertexIndex++)
	{
		
		vertex_info* ToLoad = Result.Vertices + VertexIndex;

		switch (VertexLayout) {
			case MeshVertexLayout_PUV: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, 0, 0);
				(*ToLoad).C = V3(1.0f, 1.0f, 1.0f);
			}break;

			case MeshVertexLayout_PUVN: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals, 0);
				(*ToLoad).C = V3(1.0f, 1.0f, 1.0f);
			}break;

			case MeshVertexLayout_PNUV: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals, 0);
				(*ToLoad).C = V3(1.0f, 1.0f, 1.0f);
			}break;

			case MeshVertexLayout_PUVNC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals, Colors);
			}break;

			case MeshVertexLayout_PNUVC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals, Colors);
			}break;
		}
		
		VertexAt += Increment;
	}

	if (CalculateTangents || CalculateNormals) {
		for (int Index = 0;
			Index < Result.IndicesCount;
			Index += 3)
		{
			int Index0 = Result.Indices[Index];
			int Index1 = Result.Indices[Index + 1];
			int Index2 = Result.Indices[Index + 2];

			v3 P0 = Result.Vertices[Index0].P;
			v3 P1 = Result.Vertices[Index1].P;
			v3 P2 = Result.Vertices[Index2].P;

			v2 Tex0 = Result.Vertices[Index0].UV;
			v2 Tex1 = Result.Vertices[Index1].UV;
			v2 Tex2 = Result.Vertices[Index2].UV;

			v3 Edge1 = P1 - P0;
			v3 Edge2 = P2 - P0;

			if (CalculateTangents) {
				v2 DeltaTex1 = Tex1 - Tex0;
				v2 DeltaTex2 = Tex2 - Tex0;

				float InvDet = 1.0f / (DeltaTex1.x * DeltaTex2.y - DeltaTex2.x * DeltaTex1.y);

				v3 T = InvDet * (DeltaTex2.y * Edge1 - DeltaTex1.y * Edge2);
				v3 B = InvDet * (DeltaTex1.x * Edge2 - DeltaTex2.x * Edge1);

				T = Normalize(T);
				/*
					NOTE(dima): bitangent calculation is implemented
					but not used...
				*/
				B = Normalize(B);

				//NOTE(dima): Setting the calculating tangent to the vertex;
				Result.Vertices[Index0].T = T;
				Result.Vertices[Index1].T = T;
				Result.Vertices[Index2].T = T;
			}

			//NOTE(dima): Normals calculation and setting
			if (CalculateNormals) {
				v3 TriNormal = Normalize(Cross(Edge2, Edge1));

				Result.Vertices[Index0].N = TriNormal;
				Result.Vertices[Index1].N = TriNormal;
				Result.Vertices[Index2].N = TriNormal;
			}
		}
	}

	return(Result);
}

inline void ASSETGenerateSphere_WriteVertex(float* Vertex, v3 P, v2 UV) {
	*Vertex = P.x;
	*(Vertex + 1) = P.y;
	*(Vertex + 2) = P.z;

	*(Vertex + 3) = UV.x;
	*(Vertex + 4) = UV.y;
}

mesh_info ASSETGenerateSphere(int Segments, int Rings) {
	mesh_info Result = {};

	float Radius = 0.5f;

	Segments = Max(Segments, 3);
	Rings = Max(Rings, 2);

	//NOTE(dima): 2 top and bottom triangle fans + 
	int VerticesCount = (Segments * 3) * 2 + (Segments * (Rings - 2)) * 4;
	int IndicesCount = (Segments * 3) * 2 + (Segments * (Rings - 2)) * 6;

	//NOTE(dima): 5 floats per vertex
	float* Vertices = (float*)malloc(VerticesCount * 5 * sizeof(float));
	u32* Indices = (u32*)malloc(IndicesCount * sizeof(u32));

	float AngleVert = GORE_PI / (float)Rings;
	float AngleHorz = GORE_TWO_PI / (float)Segments;

	int VertexAt = 0;
	int IndexAt = 0;

	for (int VertAt = 1; VertAt <= Rings; VertAt++) {
		float CurrAngleVert = (float)VertAt * AngleVert;
		float PrevAngleVert = (float)(VertAt - 1) * AngleVert;

		float PrevY = Cos(PrevAngleVert) * Radius;
		float CurrY = Cos(CurrAngleVert) * Radius;

		float SinVertPrev = Sin(PrevAngleVert);
		float SinVertCurr = Sin(CurrAngleVert);

		for (int HorzAt = 1; HorzAt <= Segments; HorzAt++) {
			float CurrAngleHorz = (float)HorzAt * AngleHorz;
			float PrevAngleHorz = (float)(HorzAt - 1) * AngleHorz;

			v3 P0, P1, C0, C1;
			v2 P0uv, P1uv, C0uv, C1uv;

			P0.y = PrevY;
			P1.y = PrevY;

			C0.y = CurrY;
			C1.y = CurrY;

			//TODO(dima): handle triangle fan case

			P0.x = Cos(PrevAngleHorz) * SinVertPrev * Radius;
			P1.x = Cos(CurrAngleHorz) * SinVertPrev * Radius;

			P0.z = Sin(PrevAngleHorz) * SinVertPrev * Radius;
			P1.z = Sin(CurrAngleHorz) * SinVertPrev * Radius;

			C0.x = Cos(PrevAngleHorz) * SinVertCurr * Radius;
			C1.x = Cos(CurrAngleHorz) * SinVertCurr * Radius;

			C0.z = Sin(PrevAngleHorz) * SinVertCurr * Radius;
			C1.z = Sin(CurrAngleHorz) * SinVertCurr * Radius;

			P0uv = V2(0.0f, 0.0f);
			P1uv = V2(0.0f, 0.0f);
			C0uv = V2(0.0f, 0.0f);
			C1uv = V2(0.0f, 0.0f);

			float* V0 = Vertices + VertexAt * 5;
			float* V1 = Vertices + (VertexAt + 1) * 5;
			float* V2 = Vertices + (VertexAt + 2) * 5;
			float* V3 = Vertices + (VertexAt + 3) * 5;

			if (VertAt == 1) {
				ASSETGenerateSphere_WriteVertex(V0, P0, P0uv);
				ASSETGenerateSphere_WriteVertex(V1, C0, C0uv);
				ASSETGenerateSphere_WriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else if (VertAt == Rings) {
				ASSETGenerateSphere_WriteVertex(V0, P1, P1uv);
				ASSETGenerateSphere_WriteVertex(V1, P0, P0uv);
				ASSETGenerateSphere_WriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else {
				ASSETGenerateSphere_WriteVertex(V0, P1, P1uv);
				ASSETGenerateSphere_WriteVertex(V1, P0, P0uv);
				ASSETGenerateSphere_WriteVertex(V2, C0, C0uv);
				ASSETGenerateSphere_WriteVertex(V3, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;
				Ind[3] = VertexAt;
				Ind[4] = VertexAt + 2;
				Ind[5] = VertexAt + 3;

				IndexAt += 6;
				VertexAt += 4;
			}
		}
	}

	Result = LoadMeshFromVertices(Vertices, VerticesCount, Indices, IndicesCount, MeshVertexLayout_PUV, 1, 1);

	free(Vertices);
	free(Indices);

	return(Result);
}

void ASSETSInit(asset_system* System, u32 MemorySizeForAssets) {

	//NOTE(dima): Clearing asset groups
	for (int AssetGroupIndex = 0;
		AssetGroupIndex < GameAsset_Count;
		AssetGroupIndex++)
	{
		game_asset_group* Group = System->AssetGroups + AssetGroupIndex;

		Group->FirstAssetIndex = 0;
		Group->GroupAssetCount = 0;
	}

	//NOTE(dima): Fonts
	font_info GoldenFontInfo = LoadFontInfoFromImage("../Data/Fonts/NewFontAtlas.png", 15, 8, 8, 0);
	font_info DebugFontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Bold.ttf", 18, AssetLoadFontFlag_BakeOffsetShadows);

	BeginAssetGroup(System, GameAsset_Font);
#if 0
	AddFontAsset(System, "../Data/Fonts/LiberationMono-Bold.ttf", 18, false, 0, 0, AssetLoadFontFlag_BakeOffsetShadows);
	AddFontAsset(System, "../Data/Fonts/NewFontAtlas.png", 15, true, 8, 8, 0);
#else
	AddFontAssetManual(System, &DebugFontInfo);
	AddFontAssetManual(System, &GoldenFontInfo);
#endif
	EndAssetGroup(System);

	//NOTE(dima): Bitmaps
	BeginAssetGroup(System, GameAsset_AlphaImage);
	AddBitmapAsset(System, "../Data/Images/alpha.png");
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_PotImage);
	AddBitmapAsset(System, "../Data/Images/pot.png");
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_OblivonMemeImage);
	AddBitmapAsset(System, "../Data/Images/image.bmp");
	EndAssetGroup(System);

	float PlaneVertices[] = {
		//P N UV C
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f, 0.0f,	0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.0f, -0.5f,		0.0f, 1.0f, 0.0f,	1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.0f, 0.5f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.0f, 0.5f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
	};

	u32 PlaneIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	float CubeVertices[] = {
		/*P N UV C*/
		//NOTE(Dima): Front side
		-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Top side
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Right side
		0.5f, 0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Left side
		-0.5f, 0.5f, -0.5f,		-1.0f, 0.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Back side
		0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Down side
		-0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
	};

	u32 CubeIndices[] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	mesh_info SphereMeshHig = ASSETGenerateSphere(80, 40);
	mesh_info SphereMeshAvg = ASSETGenerateSphere(40, 20);
	mesh_info SphereMeshLow = ASSETGenerateSphere(20, 10);
	mesh_info CubeMesh = LoadMeshFromVertices(CubeVertices, 24, CubeIndices, 36, MeshVertexLayout_PNUVC, 0, 1);
	mesh_info PlaneMesh = LoadMeshFromVertices(PlaneVertices, 4, PlaneIndices, 6, MeshVertexLayout_PNUVC, 0, 1);

	BeginAssetGroup(System, GameAsset_Cube);
	AddMeshAsset(System, &CubeMesh);
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_Plane);
	AddMeshAsset(System, &PlaneMesh);
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_Sphere);
	AddMeshAsset(System, &SphereMeshAvg);
	AddMeshAsset(System, &SphereMeshHig);
	AddMeshAsset(System, &SphereMeshLow);
	EndAssetGroup(System);

#if 1
	for (int AssetIndex = 0;
		AssetIndex < System->AssetCount;
		AssetIndex++)
	{
		game_asset* Asset = System->Assets + AssetIndex;
		game_asset_source* Source = System->AssetSources + AssetIndex;


		switch (System->AssetTypes[AssetIndex]) {
			case AssetType_Bitmap: {
				Asset->Bitmap_ = LoadIMG(Source->BitmapSource.Path);
				Asset->Bitmap = &Asset->Bitmap_;
			}break;

			case AssetType_Font: {

				if (!Source->FontSource.FontInfo) {
					if (Source->FontSource.LoadFromImage) {
						Asset->Font_ = LoadFontInfoFromImage(
							Source->FontSource.Path,
							Source->FontSource.Height,
							Source->FontSource.OneCharWidth,
							Source->FontSource.OneCharHeight,
							Source->FontSource.Flags);
					}
					else {
						Asset->Font_ = LoadFontInfoWithSTB(
							Source->FontSource.Path,
							Source->FontSource.Height,
							Source->FontSource.Flags);
					}
				}
				else {
					Asset->Font_ = *Source->FontSource.FontInfo;
				}

				Asset->Font = &Asset->Font_;
			}break;

			case AssetType_Mesh: {
				Asset->Mesh_ = *Source->MeshSource.MeshInfo;

				Asset->Mesh = &Asset->Mesh_;
			}break;
		}
	}
#endif
}
