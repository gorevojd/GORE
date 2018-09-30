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
			int Diff = Tag->Value_Float - TagValue;
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

	Result.Asset->ID = System->AssetCount;
	++System->AssetCount;

	System->PrevAssetPointer = Result.Asset;

	return(Result);
}

static game_asset_tag* AddTag(asset_system* System, u32 TagType) {
	game_asset_tag* Result = 0;

	if (System->PrevAssetPointer) {
		/*
			NOTE(dima): First we should check if tag with 
			the same type alredy exist.. Just for sure...
		*/
		Result = FindTagInAsset(System->PrevAssetPointer, TagType);

		if (!Result) {
			if (System->PrevAssetPointer->TagCount < MAX_TAGS_PER_ASSET) {
				//NOTE(dima): Getting tag and incrementing tag count
				Result = System->PrevAssetPointer->Tags + System->PrevAssetPointer->TagCount++;
				Result->Type = TagType;
			}
		}
	}

	return(Result);
}

static void AddFloatTag(asset_system* System, u32 TagType, float TagValue) {
	game_asset_tag* Tag = AddTag(System, TagType);

	if (Tag) {
		Tag->Value_Float = TagValue;
	}
}

static void AddIntTag(asset_system* System, u32 TagType, int TagValue) {
	game_asset_tag* Tag = AddTag(System, TagType);

	if (Tag) {
		Tag->Value_Int = TagValue;
	}
}

static void BeginAssetGroup(asset_system* System, u32 AssetID) {
	System->CurrentGroup = System->AssetGroups + AssetID;
	System->PrevAssetPointer = 0;

	game_asset_group* Group = System->CurrentGroup;

	Group->FirstAssetIndex = System->AssetCount;
	Group->GroupAssetCount = 0;
}

static void EndAssetGroup(asset_system* System) {
	Assert(System->CurrentGroup);
	game_asset_group* Group = System->CurrentGroup;

	System->CurrentGroup = 0;
	System->PrevAssetPointer = 0;
}

static void AddBitmapAsset(asset_system* System, char* Path) {
	added_asset Added = AddAsset(System, AssetType_Bitmap);

	game_asset_source* Source = Added.Source;
	Source->BitmapSource.Path = Path;
	Source->BitmapSource.BitmapInfo = 0;
}

static void AddBitmapAssetManual(asset_system* System, bitmap_info* Bitmap) {
	added_asset Added = AddAsset(System, AssetType_Bitmap);

	game_asset_source* Source = Added.Source;
	Source->BitmapSource.BitmapInfo = Bitmap;
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

static void AddVoxelAtlasAsset(asset_system* System, voxel_atlas_info* Info) {
	added_asset Added = AddAsset(System, AssetType_VoxelAtlas);

	game_asset_source* Source = Added.Source;
	Source->VoxelAtlasSource.Info = Info;
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

inline vertex_info LoadMeshVertex(float* Pos, float* Tex, float* Norm) {
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

				*ToLoad = LoadMeshVertex(Positions, TexCoords, 0);
			}break;

			case MeshVertexLayout_PUVN: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PNUV: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PUVNC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 3;
				float* Normals = VertexAt + 5;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
			}break;

			case MeshVertexLayout_PNUVC: {
				float* Positions = VertexAt;
				float* TexCoords = VertexAt + 6;
				float* Normals = VertexAt + 3;
				float* Colors = VertexAt + 8;

				*ToLoad = LoadMeshVertex(Positions, TexCoords, Normals);
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

inline void ASSETGenerateWriteVertex(float* Vertex, v3 P, v2 UV) {
	//NOTE(dima): Writing position
	*Vertex = P.x;
	*(Vertex + 1) = P.y;
	*(Vertex + 2) = P.z;


	//NOTE(dima): Writing texture coordinate
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
				ASSETGenerateWriteVertex(V0, P0, P0uv);
				ASSETGenerateWriteVertex(V1, C0, C0uv);
				ASSETGenerateWriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else if (VertAt == Rings) {
				ASSETGenerateWriteVertex(V0, P1, P1uv);
				ASSETGenerateWriteVertex(V1, P0, P0uv);
				ASSETGenerateWriteVertex(V2, C1, C1uv);

				u32* Ind = Indices + IndexAt;
				Ind[0] = VertexAt;
				Ind[1] = VertexAt + 1;
				Ind[2] = VertexAt + 2;

				IndexAt += 3;
				VertexAt += 3;
			}
			else {
				ASSETGenerateWriteVertex(V0, P1, P1uv);
				ASSETGenerateWriteVertex(V1, P0, P0uv);
				ASSETGenerateWriteVertex(V2, C0, C0uv);
				ASSETGenerateWriteVertex(V3, C1, C1uv);

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

mesh_info ASSETGenerateCylynder(float Height, float Radius, int SidesCount) {
	mesh_info Result = {};

	SidesCount = Max(3, SidesCount);

	int VerticesCount = SidesCount * 4 + SidesCount * 2 * 3;
	int IndicesCount = SidesCount * 6 + SidesCount * 2 * 3;

	float Angle = GORE_TWO_PI / (float)SidesCount;

	int IndexAt = 0;
	int VertexAt = 0;

	//NOTE(dima): 5 floats per vertex
	float* Vertices = (float*)malloc(sizeof(float) * 5 * VerticesCount);
	u32* Indices = (u32*)malloc(sizeof(u32) * IndicesCount);

	//NOTE(dima): Building top triangle fans
	float TopY = Height * 0.5f;
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		float TopY = Height * 0.5f;

		v3 CurrP;
		v3 PrevP;
		v3 Center = V3(0.0f, 0.0f, 0.0f);

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = TopY;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = TopY;
		PrevP.z = Sin(PrevAngle) * Radius;

		v2 CurrUV = V2(0.0f, 0.0f);
		v2 PrevUV = V2(0.0f, 0.0f);
		v2 CentUV = V2(0.0f, 0.0f);

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;

		Center.y = TopY;
		ASSETGenerateWriteVertex(V0, PrevP, PrevUV);
		ASSETGenerateWriteVertex(V1, CurrP, CurrUV);
		ASSETGenerateWriteVertex(V2, Center, CentUV);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;

		VertexAt += 3;
		IndexAt += 3;
	}

	//NOTE(dima): Building bottom triangle fans
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		float BotY = -Height * 0.5f;

		v3 CurrP;
		v3 PrevP;
		v3 Center = V3(0.0f, 0.0f, 0.0f);

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = BotY;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = BotY;
		PrevP.z = Sin(PrevAngle) * Radius;

		v2 CurrUV = V2(0.0f, 0.0f);
		v2 PrevUV = V2(0.0f, 0.0f);
		v2 CentUV = V2(0.0f, 0.0f);

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;

		Center.y = BotY;
		ASSETGenerateWriteVertex(V0, CurrP, CurrUV);
		ASSETGenerateWriteVertex(V1, PrevP, PrevUV);
		ASSETGenerateWriteVertex(V2, Center, CentUV);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;

		VertexAt += 3;
		IndexAt += 3;
	}

	//NOTE(dima): Building sides
	for (int Index = 1;
		Index <= SidesCount;
		Index++)
	{
		float CurrAngle = (float)Index * Angle;
		float PrevAngle = (float)(Index - 1) * Angle;

		v3 CurrP;
		v3 PrevP;

		CurrP.x = Cos(CurrAngle) * Radius;
		CurrP.y = 0.0f;
		CurrP.z = Sin(CurrAngle) * Radius;

		PrevP.x = Cos(PrevAngle) * Radius;
		PrevP.y = 0.0f;
		PrevP.z = Sin(PrevAngle) * Radius;

		v3 TopC, TopP;
		v3 BotC, BotP;

		v2 TopCuv = V2(0.0f, 0.0f);
		v2 TopPuv = V2(0.0f, 0.0f);
		v2 BotCuv = V2(0.0f, 0.0f);
		v2 BotPuv = V2(0.0f, 0.0f);

		TopC = CurrP; 
		BotC = CurrP;
		TopP = PrevP; 
		BotP = PrevP;

		TopC.y = Height * 0.5f;
		TopP.y = Height * 0.5f;
		BotC.y = -Height * 0.5f;
		BotP.y = -Height * 0.5f;

		float* V0 = Vertices + VertexAt * 5;
		float* V1 = Vertices + (VertexAt + 1) * 5;
		float* V2 = Vertices + (VertexAt + 2) * 5;
		float* V3 = Vertices + (VertexAt + 3) * 5;

		ASSETGenerateWriteVertex(V0, TopC, TopCuv);
		ASSETGenerateWriteVertex(V1, TopP, TopPuv);
		ASSETGenerateWriteVertex(V2, BotP, BotPuv);
		ASSETGenerateWriteVertex(V3, BotC, BotCuv);

		u32* Ind = Indices + IndexAt;
		Ind[0] = VertexAt;
		Ind[1] = VertexAt + 1;
		Ind[2] = VertexAt + 2;
		Ind[3] = VertexAt;
		Ind[4] = VertexAt + 2;
		Ind[5] = VertexAt + 3;

		VertexAt += 4;
		IndexAt += 6;
	}

	Result = LoadMeshFromVertices(Vertices, VerticesCount, Indices, IndicesCount, MeshVertexLayout_PUV, 1, 1);

	free(Vertices);
	free(Indices);

	return(Result);
}

voxel_atlas_info* LoadVoxelAtlas(char* FileName, u32 OneTextureWidth)
{
	voxel_atlas_info* Atlas = (voxel_atlas_info*)malloc(sizeof(voxel_atlas_info));
	Atlas->Materials = (voxel_tex_coords_set*)malloc(sizeof(voxel_tex_coords_set) * VoxelMaterial_Count);

	Atlas->Bitmap = LoadIMG(FileName);
	
	int AtlasWidth = Atlas->Bitmap.Width;

	/*AtlasWidth must be multiple of OneTextureWidth*/
	Assert((AtlasWidth & (OneTextureWidth - 1)) == 0);

	u32 TexturesByWidth = AtlasWidth / OneTextureWidth;
	Atlas->MaxTexturesCount = TexturesByWidth * TexturesByWidth;
	Atlas->TexturesCount = 0;

	Atlas->AtlasWidth = AtlasWidth;
	Atlas->OneTextureWidth = OneTextureWidth;


	for (int MaterialIndex = 0;
		MaterialIndex < VoxelMaterial_Count;
		MaterialIndex++)
	{
		for (int i = 0;
			i < VoxelFaceTypeIndex_Count;
			i++)
		{
			Atlas->Materials[MaterialIndex].Sets[i] = 0;
		}
	}

	return(Atlas);
}

void DescribeVoxelAtlasTexture(
	voxel_atlas_info* Atlas,
	u32 MaterialType,
	voxel_face_type_index FaceTypeIndex,
	int CurrTextureIndex)
{
	u32 TexturesByWidth = Atlas->AtlasWidth / Atlas->OneTextureWidth;

	Assert(CurrTextureIndex < Atlas->MaxTexturesCount);

	voxel_tex_coords_set* MatTexSet = &Atlas->Materials[MaterialType];

	switch (FaceTypeIndex) {
		case(VoxelFaceTypeIndex_Top): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Top] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Bottom): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Bottom] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Left): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Left] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Right): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Right] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Front): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Front] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Back): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Back] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_All): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Bottom] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Top] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Left] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Right] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Front] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Back] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_Side): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Left] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Right] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Front] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Back] = CurrTextureIndex;
		}break;

		case(VoxelFaceTypeIndex_TopBottom): {
			MatTexSet->Sets[VoxelFaceTypeIndex_Bottom] = CurrTextureIndex;
			MatTexSet->Sets[VoxelFaceTypeIndex_Top] = CurrTextureIndex;
		}break;

		default: {
			Assert(!"Invalid code path");
		}break;
	}
}

static void DescribeByIndex(
	voxel_atlas_info* Atlas,
	int HorzIndex, int VertIndex,
	u32 MaterialType,
	voxel_face_type_index FaceTypeIndex)
{
	DescribeVoxelAtlasTexture(Atlas, MaterialType, FaceTypeIndex, VertIndex * 16 + HorzIndex);
}


void ASSETSInit(asset_system* System) {

	//NOTE(dima): Reserving first asset to make it NULL asset
	System->AssetCount = 1;
	System->PrevAssetPointer = 0;

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
	font_info UbuntuFontInfo = LoadFontInfoWithSTB("../Data/Fonts/UbuntuMono-B.ttf", 18, AssetLoadFontFlag_BakeOffsetShadows);
	font_info ChurchFontInfo = LoadFontInfoWithSTB("../Data/Fonts/11550.ttf", 30, AssetLoadFontFlag_BakeOffsetShadows);
	font_info AntiqueOliveFontInfo = LoadFontInfoWithSTB("../Data/Fonts/aqct.ttf", 30, AssetLoadFontFlag_BakeOffsetShadows);

	BeginAssetGroup(System, GameAsset_Font);
#if 0
	FontAsset(System, "../Data/Fonts/LiberationMono-Bold.ttf", 18, false, 0, 0, AssetLoadFontFlag_BakeOffsetShadows);
	AddFontAsset(System, "../Data/Fonts/NewFontAtlas.png", 15, true, 8, 8, 0);
#else
	AddFontAssetManual(System, &UbuntuFontInfo);
	AddFontAssetManual(System, &AntiqueOliveFontInfo);
	AddFontAssetManual(System, &DebugFontInfo);
	AddFontAssetManual(System, &GoldenFontInfo);
	AddFontAssetManual(System, &ChurchFontInfo);
#endif
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_MainMenuFont);
	AddFontAssetManual(System, &AntiqueOliveFontInfo);
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

	BeginAssetGroup(System, GameAsset_ContainerDiffImage);
	AddBitmapAsset(System, "../Data/Images/container_diff.png");
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_ContainerSpecImage);
	AddBitmapAsset(System, "../Data/Images/container_spec.png");
	EndAssetGroup(System);

	bitmap_info Checker1 = GenerateCheckerboardBitmap(512, 64, V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));

	BeginAssetGroup(System, GameAsset_Checkerboard);
	AddBitmapAssetManual(System, &Checker1);
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

	mesh_info CylMeshSuperLow = ASSETGenerateCylynder(1.0f, 0.5f, 6);
	mesh_info CylMeshLow = ASSETGenerateCylynder(1.0f, 0.5f, 12);
	mesh_info CylMeshAvg = ASSETGenerateCylynder(1.0f, 0.5f, 24);
	mesh_info CylMeshHig = ASSETGenerateCylynder(1.0f, 0.5f, 48);

	mesh_info SphereMeshSuperHig = ASSETGenerateSphere(160, 80);
	mesh_info SphereMeshHig = ASSETGenerateSphere(80, 40);
	mesh_info SphereMeshAvg = ASSETGenerateSphere(40, 20);
	mesh_info SphereMeshLow = ASSETGenerateSphere(20, 10);
	mesh_info SphereMeshSuperLow = ASSETGenerateSphere(10, 5);

	mesh_info CubeMesh = LoadMeshFromVertices(CubeVertices, 24, CubeIndices, 36, MeshVertexLayout_PNUVC, 0, 1);
	mesh_info PlaneMesh = LoadMeshFromVertices(PlaneVertices, 4, PlaneIndices, 6, MeshVertexLayout_PNUVC, 0, 1);

	BeginAssetGroup(System, GameAsset_Cube);
	AddMeshAsset(System, &CubeMesh);
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_Plane);
	AddMeshAsset(System, &PlaneMesh);
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_Sphere);
	AddMeshAsset(System, &SphereMeshSuperLow);
	AddFloatTag(System, GameAssetTag_LOD, 0.0f);
	AddMeshAsset(System, &SphereMeshLow);
	AddFloatTag(System, GameAssetTag_LOD, 0.25f);
	AddMeshAsset(System, &SphereMeshAvg);
	AddFloatTag(System, GameAssetTag_LOD, 0.5f);
	AddMeshAsset(System, &SphereMeshHig);
	AddFloatTag(System, GameAssetTag_LOD, 0.75f);
	AddMeshAsset(System, &SphereMeshSuperHig);
	AddFloatTag(System, GameAssetTag_LOD, 1.0f);
	EndAssetGroup(System);

	BeginAssetGroup(System, GameAsset_Cylynder);
	AddMeshAsset(System, &CylMeshSuperLow);
	AddFloatTag(System, GameAssetTag_LOD, 0.0f);
	AddMeshAsset(System, &CylMeshLow);
	AddFloatTag(System, GameAssetTag_LOD, 0.25f);
	AddMeshAsset(System, &CylMeshAvg);
	AddFloatTag(System, GameAssetTag_LOD, 0.5f);
	AddMeshAsset(System, &CylMeshHig);
	AddFloatTag(System, GameAssetTag_LOD, 1.0f);
	EndAssetGroup(System);

	voxel_atlas_info* Atlas = LoadVoxelAtlas("../Data/Images/VoxelAtlas/VoxelAtlas.png", 16);
	DescribeByIndex(Atlas, 0, 2, VoxelMaterial_GrassyGround, VoxelFaceTypeIndex_Top);
	DescribeByIndex(Atlas, 1, 2, VoxelMaterial_GrassyGround, VoxelFaceTypeIndex_Side);
	DescribeByIndex(Atlas, 2, 2, VoxelMaterial_GrassyGround, VoxelFaceTypeIndex_Bottom);
	DescribeByIndex(Atlas, 2, 2, VoxelMaterial_Ground, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 3, 1, VoxelMaterial_Tree, VoxelFaceTypeIndex_Side);
	DescribeByIndex(Atlas, 4, 0, VoxelMaterial_Tree, VoxelFaceTypeIndex_TopBottom);
	DescribeByIndex(Atlas, 5, 0, VoxelMaterial_Stone, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 6, 0, VoxelMaterial_Sand, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 7, 0, VoxelMaterial_Leaves, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 8, 0, VoxelMaterial_Birch, VoxelFaceTypeIndex_Side);
	DescribeByIndex(Atlas, 4, 0, VoxelMaterial_Birch, VoxelFaceTypeIndex_TopBottom);
	DescribeByIndex(Atlas, 15, 15, VoxelMaterial_Lava, VoxelFaceTypeIndex_All);

	DescribeByIndex(Atlas, 0, 1, VoxelMaterial_SnowGround, VoxelFaceTypeIndex_Top);
	DescribeByIndex(Atlas, 1, 1, VoxelMaterial_SnowGround, VoxelFaceTypeIndex_Side);
	DescribeByIndex(Atlas, 2, 1, VoxelMaterial_SnowGround, VoxelFaceTypeIndex_Bottom);
	DescribeByIndex(Atlas, 2, 1, VoxelMaterial_WinterGround, VoxelFaceTypeIndex_All);

	DescribeByIndex(Atlas, 0, 2, VoxelMaterial_Brick, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 1, 2, VoxelMaterial_GrassyBigBrick, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 2, 2, VoxelMaterial_DecorateBrick, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 3, 2, VoxelMaterial_BigBrick, VoxelFaceTypeIndex_All);
	DescribeByIndex(Atlas, 4, 2, VoxelMaterial_BookShelf, VoxelFaceTypeIndex_Side);
	DescribeByIndex(Atlas, 3, 3, VoxelMaterial_BookShelf, VoxelFaceTypeIndex_TopBottom);

	DescribeByIndex(Atlas, 0, 15, VoxelMaterial_Secret, VoxelFaceTypeIndex_All);

	BeginAssetGroup(System, GameAsset_MyVoxelAtlas);
	AddVoxelAtlasAsset(System, Atlas);
	EndAssetGroup(System);

#if 1
	for (int AssetIndex = 1;
		AssetIndex < System->AssetCount;
		AssetIndex++)
	{
		game_asset* Asset = System->Assets + AssetIndex;
		game_asset_source* Source = System->AssetSources + AssetIndex;

		Asset->State = GameAssetState_Loaded;

		switch (System->AssetTypes[AssetIndex]) {
			case AssetType_Bitmap: {
				if (!Source->BitmapSource.BitmapInfo) {
					Asset->Bitmap_ = LoadIMG(Source->BitmapSource.Path);
				}
				else {
					Asset->Bitmap_ = *Source->BitmapSource.BitmapInfo;
				}

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

			case AssetType_VoxelAtlas: {
				Asset->VoxelAtlas_ = *Source->VoxelAtlasSource.Info;

				Asset->VoxelAtlas = &Asset->VoxelAtlas_;
			} break;
		}
	}
#endif
}
