#include "asset_builder_tool.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static void BeginAssetGroup(asset_system* System, u32 GroupID) {
	System->CurrentGroup = System->AssetGroups + GroupID;
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

struct added_asset {
	game_asset* Asset;
	game_asset_source* Source;
	game_asset_freearea* Freearea;
	gass_header* FileHeader;
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
	Result.Freearea = System->AssetFreeareas + AssetIndex;
	Result.FileHeader = System->FileHeaders + AssetIndex;
	Result.FileHeader->AssetType = AssetType;

	Result.Asset->ID = System->AssetCount;
	++System->AssetCount;

	System->PrevAssetPointer = Result.Asset;

	return(Result);
}

static void AddFreeareaToAsset(asset_system* System, game_asset* Asset, void* Pointer) {
	game_asset_freearea* Free = System->AssetFreeareas + Asset->ID;

	int TargetFreeAreaIndex = Free->SetCount++;
	Assert(TargetFreeAreaIndex < FREEAREA_SLOTS_COUNT);

	Free->Pointers[TargetFreeAreaIndex] = Pointer;
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

static void AddEmptyTag(asset_system* System, u32 TagType) {
	game_asset_tag* Tag = AddTag(System, TagType);

	if (Tag) {
		Tag->Value_Int = 1;
	}
}

static bitmap_id AddBitmapAsset(asset_system* System, char* Path) {
	added_asset Added = AddAsset(System, AssetType_Bitmap);


	gass_header* FileHeader = Added.FileHeader;
	game_asset_source* Source = Added.Source;

	Source->BitmapSource.Path = Path;
	Source->BitmapSource.BitmapInfo = 0;

	bitmap_id Result = Added.Asset->ID;

	return(Result);
}

static bitmap_id AddBitmapAssetManual(asset_system* System, bitmap_info* Bitmap) {
	added_asset Added = AddAsset(System, AssetType_Bitmap);

	gass_header* FileHeader = Added.FileHeader;

	game_asset_source* Source = Added.Source;
	Source->BitmapSource.BitmapInfo = Bitmap;

	bitmap_id Result = Added.Asset->ID;

	return(Result);
}

static sound_id AddSoundAsset(asset_system* System, char* Path) {
	added_asset Added = AddAsset(System, AssetType_Sound);

	game_asset_source* Source = Added.Source;
	Source->SoundSource.Path = Path;

	sound_id Result = Added.Asset->ID;

	return(Result);
}

static model_id AddModelAsset(asset_system* System, model_info* Model) {
	added_asset Added = AddAsset(System, AssetType_Model);

	game_asset_source* Source = Added.Source;
	Source->ModelSource.ModelInfo = Model;

	model_id Result = Added.Asset->ID;

	return(Result);
}

static mesh_id AddMeshAsset(asset_system* System, mesh_info* Mesh) {
	added_asset Added = AddAsset(System, AssetType_Mesh);

	game_asset_source* Source = Added.Source;
	Source->MeshSource.MeshInfo = Mesh;

	mesh_id Result = Added.Asset->ID;

	return(Result);
}

static font_id AddFontAsset(
	asset_system* System,
	font_info* FontInfo)
{
	added_asset Added = AddAsset(System, AssetType_Font);

	game_asset_source* Source = Added.Source;
	Source->FontSource.FontInfo = FontInfo;
	Added.Asset->Font = FontInfo;

	gass_header* Header = Added.FileHeader;
	Header->Font.AscenderHeight = FontInfo->AscenderHeight;
	Header->Font.DescenderHeight = FontInfo->DescenderHeight;
	Header->Font.LineGap = FontInfo->LineGap;
	Header->Font.GlyphsCount = FontInfo->GlyphsCount;
	Header->Font.MaxGlyphsCount = MAX_FONT_INFO_GLYPH_COUNT;
	Header->Font.AtlasBitmapHeight = FontInfo->FontAtlasImage.Height;
	Header->Font.AtlasBitmapWidth = FontInfo->FontAtlasImage.Width;
	Header->Font.FirstGlyphID = FontInfo->Reserved;

	Header->Font.LineOffsetToMapping = sizeof(gass_header);
	Header->Font.LineOffsetToKerningPairs =
		Header->Font.LineOffsetToMapping +
		sizeof(int) * Header->Font.MaxGlyphsCount;
	Header->Font.LineOffsetToAtlasBitmapPixels =
		Header->Font.LineOffsetToKerningPairs +
		sizeof(float) * Header->Font.GlyphsCount * Header->Font.GlyphsCount;

	AddFreeareaToAsset(System, Added.Asset, FontInfo->KerningPairs);
	AddFreeareaToAsset(System, Added.Asset, FontInfo->FontAtlasImage.Pixels);

	font_id Result = Added.Asset->ID;
	return(Result);
}

static u32 AddFontGlyphAsset(
	asset_system* System,
	glyph_info* GlyphInfo)
{
	added_asset Added = AddAsset(System, AssetType_FontGlyph);

	game_asset_source* Source = Added.Source;
	Source->FontGlyphSource.Glyph = GlyphInfo;

	gass_header* Header = Added.FileHeader;
	Header->Glyph.Codepoint = GlyphInfo->Codepoint;
	Header->Glyph.BitmapWidth = GlyphInfo->Bitmap.Width;
	Header->Glyph.BitmapHeight = GlyphInfo->Bitmap.Height;
	Header->Glyph.XOffset = GlyphInfo->XOffset;
	Header->Glyph.YOffset = GlyphInfo->YOffset;
	Header->Glyph.Advance = GlyphInfo->Advance;
	Header->Glyph.LeftBearingX = GlyphInfo->LeftBearingX;

	Header->Glyph.AtlasMinUV_x = GlyphInfo->AtlasMinUV.x;
	Header->Glyph.AtlasMinUV_y = GlyphInfo->AtlasMinUV.y;
	Header->Glyph.AtlasMaxUV_x = GlyphInfo->AtlasMaxUV.x;
	Header->Glyph.AtlasMaxUV_y = GlyphInfo->AtlasMaxUV.y;

	AddFreeareaToAsset(System, Added.Asset, GlyphInfo->Bitmap.Pixels);

	font_glyph_id Result = Added.Asset->ID;
	return(Result);
}

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

		Glyph->Bitmap = AllocateBitmap(Glyph->Width, Glyph->Height);

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

				CopyBitmapData(&Glyph->Bitmap, &SrcGlyph->Bitmap);
			}
		}
	}

	//NOTE(dima): Processing kerning
	u32 KerningOneRowSize = sizeof(float) * Result.GlyphsCount;
	Result.KerningPairs = (float*)malloc(KerningOneRowSize * KerningOneRowSize);

	for (int FirstGlyphIndex = 0; FirstGlyphIndex < Result.GlyphsCount; FirstGlyphIndex++) {
		for (int SecondGlyphIndex = 0; SecondGlyphIndex < Result.GlyphsCount; SecondGlyphIndex++) {
			u32 KerningIndex = SecondGlyphIndex * Result.GlyphsCount + FirstGlyphIndex;

			Result.KerningPairs[KerningIndex] = 0;
		}
	}

	//NOTE(dima): Building font atlas
	Result.FontAtlasImage = AllocateBitmap(AtlasWidth, AtlasHeight);

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
	DeallocateBitmap(&FontImage);

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
		Glyph->Bitmap = AllocateBitmap(Glyph->Width, Glyph->Height);
		Glyph->Advance = Advance * Scale;
		Glyph->LeftBearingX = LeftBearingX * Scale;
		Glyph->XOffset = XOffset;
		Glyph->YOffset = YOffset;
		Glyph->Codepoint = Codepoint;

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
	u32 KerningOneRowSize = sizeof(float) * Result.GlyphsCount;
	Result.KerningPairs = (float*)malloc(KerningOneRowSize * KerningOneRowSize);

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
	Result.FontAtlasImage = AllocateBitmap(AtlasWidth, AtlasHeight);

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


void InitAssetFile(asset_system* Assets) {
	//NOTE(dima): Reserving first asset to make it NULL asset
	Assets->AssetCount = 1;
	Assets->PrevAssetPointer = 0;

	//NOTE(dima): Clearing asset groups
	for (int AssetGroupIndex = 0;
		AssetGroupIndex < GameAsset_Count;
		AssetGroupIndex++)
	{
		game_asset_group* Group = Assets->AssetGroups + AssetGroupIndex;

		Group->FirstAssetIndex = 0;
		Group->GroupAssetCount = 0;
	}

	//NOTE(dima): Clearing free areas
	for (int FreeAreaIndex = 0;
		FreeAreaIndex < TEMP_STORED_ASSET_COUNT;
		FreeAreaIndex++) 
	{
		game_asset_freearea* Free = Assets->AssetFreeareas + FreeAreaIndex;

		*Free = {};

		Free->SetCount = 0;
		for (int PointerIndex = 0; PointerIndex < FREEAREA_SLOTS_COUNT; PointerIndex++) {
			Free->Pointers[PointerIndex] = 0;
		}
	}
}

void WriteAssetFile(asset_system* Assets, char* FileName) {
	FILE* fp = fopen(FileName, "wb");

	u32 AssetsLinesOffsetsSize = sizeof(u32) * (Assets->AssetCount - 1);
	u32* AssetsLinesOffsets = (u32*)malloc(AssetsLinesOffsetsSize);

	u32 AssetFileBytesWritten = 0;
	u32 AssetLinesBytesWritten = 0;
	if (fp) {

		//NOTE(dima): Writing asset file header
		asset_file_header FileHeader = {};

		FileHeader.Version = ASSET_FILE_VERSION;
		FileHeader.AssetGroupsCount = GameAsset_Count;
		FileHeader.AssetsCount = Assets->AssetCount - 1;

		FileHeader.AssetFileHeader[0] = 'G';
		FileHeader.AssetFileHeader[1] = 'A';
		FileHeader.AssetFileHeader[2] = 'S';
		FileHeader.AssetFileHeader[3] = 'S';

		fwrite(&FileHeader, sizeof(asset_file_header), 1, fp);
		AssetFileBytesWritten += sizeof(asset_file_header);
		
		//NOTE(dima): Writing asset groups after asset file header
		for (int GroupIndex = 0;
			GroupIndex < FileHeader.AssetGroupsCount;
			GroupIndex++)
		{
			asset_file_asset_group Group;
			Group.FirstAssetIndex = Assets->AssetGroups[GroupIndex].FirstAssetIndex;
			Group.GroupAssetCount = Assets->AssetGroups[GroupIndex].GroupAssetCount;

			fwrite(&Group, sizeof(asset_file_asset_group), 1, fp);
			AssetFileBytesWritten += sizeof(asset_file_asset_group);
		}

		for (int AssetIndex = 1;
			AssetIndex < Assets->AssetCount;
			AssetIndex++)
		{
			//NOTE(dima): Setting asset line offset
			AssetsLinesOffsets[AssetIndex - 1] = ftell(fp);

			game_asset* Asset = Assets->Assets + AssetIndex;
			game_asset_source* Source = Assets->AssetSources + AssetIndex;
			game_asset_freearea* Free = Assets->AssetFreeareas + AssetIndex;
			gass_header* Header = Assets->FileHeaders + AssetIndex;

			u32 HeaderByteSize = sizeof(gass_header);
			u32 TagsByteSize = Asset->TagCount * sizeof(gass_tag);
			u32 DataByteSize = 0;

			/*
				NOTE(dima): Loading assets and setting assets
				headers data byte size.
			*/

			switch (Asset->Type) {
				case AssetType_Bitmap: {
					b32 BitmapAllocatedHere = 0;
					if (!Source->BitmapSource.BitmapInfo) 
					{
						Asset->Bitmap = (bitmap_info*)malloc(sizeof(bitmap_info));
						*Asset->Bitmap = LoadIMG(Source->BitmapSource.Path);

						BitmapAllocatedHere = 1;
					}
					else {
						Asset->Bitmap = Source->BitmapSource.BitmapInfo;
					}
					AddFreeareaToAsset(Assets, Asset, Asset->Bitmap->Pixels);

					if (BitmapAllocatedHere) {
						AddFreeareaToAsset(Assets, Asset, Asset->Bitmap);
					}

					//NOTE(dima): Setting asset header
					Header->Bitmap.Width = Asset->Bitmap->Width;
					Header->Bitmap.Height = Asset->Bitmap->Height;

					//NOTE(dima): Set data size
					DataByteSize = Asset->Bitmap->Width * Asset->Bitmap->Height * 4;
				}break;

				case AssetType_Font: {
					Asset->Font = Source->FontSource.FontInfo;

					/*
					NOTE(dima): This needs to be set here because
					glyphs are added after fonts are added and 
					first glyph index is remembered in Font->Reserved
					variable
					*/
					Header->Font.FirstGlyphID = Asset->Font->Reserved;

					u32 SizeOfMapping = sizeof(int) * MAX_FONT_INFO_GLYPH_COUNT;
					u32 SizeOfKerning = sizeof(float) * Asset->Font->GlyphsCount * Asset->Font->GlyphsCount;
					u32 SizeOfAtlasBitmap = Asset->Font->FontAtlasImage.Width * Asset->Font->FontAtlasImage.Height * 4;

					DataByteSize = SizeOfMapping + SizeOfKerning + SizeOfAtlasBitmap;
				}break;

				case AssetType_FontGlyph: {
					Asset->FontGlyph = Source->FontGlyphSource.Glyph;

					u32 GlyphBitmapSize =
						Asset->FontGlyph->Bitmap.Width *
						Asset->FontGlyph->Bitmap.Height * 4;

					DataByteSize = GlyphBitmapSize;
				}break;

				case AssetType_Mesh: {
					Asset->Mesh = Source->MeshSource.MeshInfo;


				}break;
			}

			//NOTE(dima): Forming and writing header
			Header->Pitch = HeaderByteSize + DataByteSize + TagsByteSize;
			Header->LineFirstTagOffset = HeaderByteSize + DataByteSize;
			Header->TagCount = Asset->TagCount;
			Header->AssetType = Asset->Type;
			Header->AssetTotalDataSize = DataByteSize;
			Header->AssetTotalTagsSize = TagsByteSize;

			STRONG_ASSERT(Header->AssetTotalTagsSize == Asset->TagCount * sizeof(gass_tag));

			fwrite(Header, sizeof(gass_header), 1, fp);

			//NOTE(dima): Writing asset data
			switch (Asset->Type) {
				case AssetType_Bitmap: {
					//NOTE(dima): Writing bitmap pixel data
					fwrite(Asset->Bitmap->Pixels, DataByteSize, 1, fp);
				}break;

				case AssetType_Font: {
					//NOTE(dima): Writing mapping data
					fwrite(
						Asset->Font->CodepointToGlyphMapping,
						sizeof(int) * MAX_FONT_INFO_GLYPH_COUNT,
						1, fp);

					//NOTE(dima): Writing kerning pairs
					fwrite(
						Asset->Font->KerningPairs,
						sizeof(float) * Asset->Font->GlyphsCount * Asset->Font->GlyphsCount,
						1, fp);

					//NOTE(dima): Writing atlas bitmap pixel data
					fwrite(
						Asset->Font->FontAtlasImage.Pixels,
						Asset->Font->FontAtlasImage.Width * Asset->Font->FontAtlasImage.Height * 4,
						1, fp);
				}break;

				case AssetType_FontGlyph: {
					//NOTE(dima):
					fwrite(Asset->FontGlyph->Bitmap.Pixels, DataByteSize, 1, fp);
				}break;

				case AssetType_Mesh: {

				}break;
			}

			//NOTE(dima): Forming tags
			gass_tag WriteTags[MAX_TAGS_PER_ASSET];

			for (int AssetTagIndex = 0;
				AssetTagIndex < Asset->TagCount;
				AssetTagIndex++)
			{
				game_asset_tag* From = Asset->Tags + AssetTagIndex;
				gass_tag* To = WriteTags + AssetTagIndex;

				To->Type = From->Type;
				To->Value_Float = From->Value_Float;
			}
			fwrite(WriteTags, TagsByteSize, 1, fp);

			//NOTE(dima): Freeing freareas
			for (int FreeIndex = 0; FreeIndex < Free->SetCount; FreeIndex++) {
				free(Free->Pointers[FreeIndex]);
			}

			//NOTE(dima): Incrementing file written data size
			AssetFileBytesWritten += Header->Pitch;
			AssetLinesBytesWritten += Header->Pitch;
		}

		fclose(fp);
	}
	else {
		INVALID_CODE_PATH;
	}


	//NOTE(dima): Reading file contents
	void* FileData = 0;
	fp = fopen(FileName, "rb");
	if (fp) {
		//NOTE(dima): Getting file size
		fseek(fp, 0, SEEK_END);
		u32 FileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		STRONG_ASSERT(FileSize == AssetFileBytesWritten);

		//NOTE(dima): Reading file contents
		FileData = malloc(FileSize);
		fread(FileData, FileSize, 1, fp);

		fclose(fp);
	}
	else {
		INVALID_CODE_PATH;
	}


	//NOTE(dima): Inserting asset lines offsets after groups
	fp = fopen(FileName, "wb");
	if (fp) {
		asset_file_header* Header = (asset_file_header*)FileData;

		u32 GroupsByteSize = Header->AssetGroupsCount * sizeof(asset_file_asset_group);
		u32 LinesOffsetsSize = Header->AssetsCount * sizeof(u32);
		u32 AssetsLinesByteSize = AssetLinesBytesWritten;

		u32 GroupsByteOffset = sizeof(asset_file_header);
		u32 LinesOffsetsByteOffset = GroupsByteOffset + GroupsByteSize;
		u32 AssetLinesByteOffset = LinesOffsetsByteOffset + LinesOffsetsSize;
		
		Header->GroupsByteOffset = GroupsByteOffset;
		Header->LinesOffsetsByteOffset = LinesOffsetsByteOffset;
		Header->AssetLinesByteOffset = AssetLinesByteOffset;

		//NOTE(dima): Rewriting header
		fwrite(Header, sizeof(asset_file_header), 1, fp);

		//NOTE(dima): Rewriting groups
		STRONG_ASSERT(GroupsByteOffset == ftell(fp));
		fwrite((u8*)FileData + GroupsByteOffset, GroupsByteSize, 1, fp);

		//NOTE(dima): Writing asset lines offsets
		STRONG_ASSERT(LinesOffsetsByteOffset == ftell(fp));
		fwrite(AssetsLinesOffsets, AssetsLinesOffsetsSize, 1, fp);

		//NOTE(dima): Rewriting asset data lines
		STRONG_ASSERT(AssetLinesByteOffset == ftell(fp));
		fwrite(
			(u8*)FileData + GroupsByteOffset + GroupsByteSize, 
			AssetLinesBytesWritten, 1, fp);
	}
	else {
		INVALID_CODE_PATH;
	}

	if (FileData) {
		free(FileData);
	}

	free(AssetsLinesOffsets);
}

void WriteFonts() 
{
	asset_system System_ = {};
	asset_system* System = &System_;
	InitAssetFile(System);

	//NOTE(dima): Fonts
	font_info GoldenFontInfo = LoadFontInfoFromImage("../Data/Fonts/NewFontAtlas.png", 15, 8, 8, 0);
	font_info DebugFontInfo = LoadFontInfoWithSTB("../Data/Fonts/LiberationMono-Bold.ttf", 18, AssetLoadFontFlag_BakeOffsetShadows);
	font_info UbuntuFontInfo = LoadFontInfoWithSTB("../Data/Fonts/UbuntuMono-B.ttf", 18, AssetLoadFontFlag_BakeOffsetShadows);
	font_info AntiqueOliveFontInfo = LoadFontInfoWithSTB("../Data/Fonts/aqct.ttf", 30, AssetLoadFontFlag_BakeOffsetShadows);

	BeginAssetGroup(System, GameAsset_Font);
	AddFontAsset(System, &UbuntuFontInfo);
	AddEmptyTag(System, GameAssetTag_Font_Debug);
	AddFontAsset(System, &AntiqueOliveFontInfo);
	AddEmptyTag(System, GameAssetTag_Font_MainMenuFont);
	AddFontAsset(System, &GoldenFontInfo);
	AddEmptyTag(System, GameAssetTag_Font_Golden);
	AddFontAsset(System, &DebugFontInfo);
	EndAssetGroup(System);

	font_info Fonts[] = {
		GoldenFontInfo, 
		DebugFontInfo,
		UbuntuFontInfo,
		AntiqueOliveFontInfo,
	};

	u32 FirstBitmapIDs[ArrayCount(Fonts)];

	BeginAssetGroup(System, GameAsset_FontGlyph);
	for (int FontIndex = 0;
		FontIndex < ArrayCount(Fonts);
		FontIndex++)
	{
		font_info* Font = Fonts + FontIndex;

		for (int GlyphIndex = 0;
			GlyphIndex < Font->GlyphsCount;
			GlyphIndex++)
		{
			u32 AddedGlyphID = AddFontGlyphAsset(System, &Font->Glyphs[GlyphIndex]);

			if (GlyphIndex == 0) {
				Font->Reserved = AddedGlyphID;
			}
		}
	}
	EndAssetGroup(System);

	WriteAssetFile(System, "../Data/Fonts.gass");
}

void WriteBitmaps() {
	asset_system System_ = {};
	asset_system* System = &System_;
	InitAssetFile(System);

	//NOTE(dima): Player asset
	BeginAssetGroup(System, GameAsset_Lilboy);

	AddBitmapAsset(System, "../Data/Images/Game/lilboy.png");
	AddFloatTag(System, GameAssetTag_Lilboy, 0.0f);

	AddBitmapAsset(System, "../Data/Images/Game/lilboyhoodie.png");
	AddFloatTag(System, GameAssetTag_Lilboy, 1.0f);

	AddBitmapAsset(System, "../Data/Images/Game/lilboyhair.png");
	AddFloatTag(System, GameAssetTag_Lilboy, 2.0f);

	AddBitmapAsset(System, "../Data/Images/Game/zsmile.png");
	AddBitmapAsset(System, "../Data/Images/Game/hellboy.png");
	AddBitmapAsset(System, "../Data/Images/Game/wtf.png");
	EndAssetGroup(System);

	//NOTE(dima): Knife
	BeginAssetGroup(System, GameAsset_Knife);
	AddBitmapAsset(System, "../Data/Images/Game/Knife.png");
	EndAssetGroup(System);

	//NOTE(dima): Bottle
	BeginAssetGroup(System, GameAsset_Bottle);
	AddBitmapAsset(System, "../Data/Images/Game/cola_bottle.png");
	EndAssetGroup(System);

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

	BeginAssetGroup(System, GameAsset_VoxelAtlasBitmap);
	AddBitmapAsset(System, "../Data/Images/VoxelAtlas/VoxelAtlas.png");
	EndAssetGroup(System);

	bitmap_info Checker1 = GenerateCheckerboardBitmap(512, 64, V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));

	BeginAssetGroup(System, GameAsset_Checkerboard);
	AddBitmapAssetManual(System, &Checker1);
	EndAssetGroup(System);

	WriteAssetFile(System, "../data/Bitmaps.gass");
}

void WriteMeshPrimitives() {
	asset_system System_ = {};
	asset_system* System = &System_;
	InitAssetFile(System);

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

	mesh_info CylMeshSuperLow = AssetGenerateCylynder(1.0f, 0.5f, 6);
	mesh_info CylMeshLow = AssetGenerateCylynder(1.0f, 0.5f, 12);
	mesh_info CylMeshAvg = AssetGenerateCylynder(1.0f, 0.5f, 24);
	mesh_info CylMeshHig = AssetGenerateCylynder(1.0f, 0.5f, 48);

	mesh_info SphereMeshSuperHig = AssetGenerateSphere(160, 80);
	mesh_info SphereMeshHig = AssetGenerateSphere(80, 40);
	mesh_info SphereMeshAvg = AssetGenerateSphere(40, 20);
	mesh_info SphereMeshLow = AssetGenerateSphere(20, 10);
	mesh_info SphereMeshSuperLow = AssetGenerateSphere(10, 5);

	mesh_info CubeMesh = AssetLoadMeshFromVertices(CubeVertices, 24, CubeIndices, 36, MeshVertexLayout_PNUVC, 0, 1);
	mesh_info PlaneMesh = AssetLoadMeshFromVertices(PlaneVertices, 4, PlaneIndices, 6, MeshVertexLayout_PNUVC, 0, 1);

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

	WriteAssetFile(System, "../Data/MeshPrimitives.gass");
}

int main() {
	
	WriteFonts();
	WriteBitmaps();
	WriteMeshPrimitives();

	system("pause");
	return(0);
}