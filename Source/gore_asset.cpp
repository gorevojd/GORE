#include "gore_asset.h"
#include "gore_voxshared.h"

#include <stdio.h>

struct asset_load_font_asset_work {

};

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
	Result = GetByAssetID(System, ResultAssetIndex);

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
			int Diff = Tag->Value_Int - TagValue;
			//NOTE(dima): Getting absolute value of the diff
			if (Diff < 0) {
				Diff = -Diff;
			}

			if (Diff < BestMatchDiff) {
				BestMatchAssetIndex = AssetIndex;
				BestMatchDiff = Diff;
			}
		}
	}

	int ResultAssetIndex = Group->FirstAssetIndex + BestMatchAssetIndex;
	Result = GetByAssetID(System, ResultAssetIndex);

	return(Result);
}

u32 GetAssetByBestFloatTag(asset_system* System, u32 GroupID, u32 TagType, float TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestFloatTagInternal(System, GroupID, TagType, TagValue);

	u32 Result = 0;
	if (Asset) {
		Assert(Asset->Type == AssetType);
		Result = Asset->ID;
	}

	return(Result);
}

u32 GetAssetByBestIntTag(asset_system* System, u32 GroupID, u32 TagType, int TagValue, u32 AssetType) {
	game_asset* Asset = GetAssetByBestIntTagInternal(System, GroupID, TagType, TagValue);

	u32 Result = 0;
	if (Asset) {
		Assert(Asset->Type == AssetType);
		Result = Asset->ID;
	}

	return(Result);
}

u32 GetAssetByTag(asset_system* System, u32 GroupID, u32 TagType, u32 AssetType) {
	u32 ResultAssetIndex = 0;

	game_asset_group* Group = System->AssetGroups + GroupID;

	for (int AssetIndex = 0;
		AssetIndex < Group->GroupAssetCount;
		AssetIndex++)
	{
		int ExactAssetIndex = Group->FirstAssetIndex + AssetIndex;
		game_asset* Asset = System->Assets + ExactAssetIndex;

		game_asset_tag* Tag = FindTagInAsset(Asset, TagType);

		if (Tag) {
			ResultAssetIndex = ExactAssetIndex;
			break;
		}
	}

	game_asset* ResultAsset = GetByAssetID(System, ResultAssetIndex);
	if (ResultAsset) {
		Assert(ResultAsset->Type == AssetType);
	}

	return(ResultAssetIndex);
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

static game_asset* AddGameAsset(asset_system* System) {
	game_asset* Result = System->Assets + System->AssetCount;

	Result->ID = System->AssetCount++;

	return(Result);
}

static void AssetAllocateBitmap(
	bitmap_info* Bitmap, 
	void* BitmapDataMemory, 
	int Width, int Height) 
{
	Bitmap->Width = Width;
	Bitmap->Height = Height;
	Bitmap->WidthOverHeight = (float)Width / (float)Height;
	Bitmap->Pitch = Width * 4;
	Bitmap->Pixels = (u8*)BitmapDataMemory;
	Bitmap->TextureHandle = 0;
}

void* AssetRequestMemory(asset_system* System, u32 MemorySize) {
	void* Result = malloc(MemorySize);

	return(Result);
}

void AssetReleaseMemory(asset_system* System) {

}

inline void InsertAssetBefore(game_asset* ToInsert, game_asset* Before) {
	ToInsert->Next = Before;
	ToInsert->Prev = Before->Prev;

	ToInsert->Next->Prev = ToInsert;
	ToInsert->Prev->Next = ToInsert;
}

game_asset_pool_block AssetInitAssetPoolBlock(asset_system* System, u32 PullAssetCount) {
	game_asset_pool_block Result = {};

	Result.UseAssetsCount = 0;

	//NOTE(dima): INitialization of UseSentinel
	Result.MaxBlockAssetCount = PullAssetCount;
	Result.BlockAssets = PushArray(System->AssetSystemMemory, game_asset, PullAssetCount);

	Result.UseSentinel = {};
	Result.UseSentinel.Next = &Result.UseSentinel;
	Result.UseSentinel.Prev = &Result.UseSentinel;

	//NOTE(dima): Initialization of FreeSentinel
	Result.FreeSentinel = {};
	Result.FreeSentinel.Next = &Result.FreeSentinel;
	Result.FreeSentinel.Prev = &Result.FreeSentinel;

	//NOTE(dima): Inserting all of the assets into freelist
	for (int AssetIndex = 0;
		AssetIndex < PullAssetCount;
		AssetIndex++)
	{
		InsertAssetBefore(&Result.BlockAssets[AssetIndex], &Result.FreeSentinel);
	}

	return(Result);
}

inline asset_memory_entry* AssetAllocateMemoryEntry(asset_system* AssetSystem) {
	asset_memory_entry* Result = 0;

	if (AssetSystem->FirstFreeMemoryEntry.NextAllocEntry !=
		&AssetSystem->FirstFreeMemoryEntry) 
	{
		Result = AssetSystem->FirstFreeMemoryEntry.NextAllocEntry;

		Result->NextAllocEntry = AssetSystem->FirstUseMemoryEntry.NextAllocEntry;
		Result->PrevAllocEntry = &AssetSystem->FirstUseMemoryEntry;

		Result->NextAllocEntry->PrevAllocEntry = Result;
		Result->PrevAllocEntry->NextAllocEntry = Result;
	}
	else {
		//NOTE(dima): Allocate new entry

	}

	return(Result);
}

void SplitMemoryEntry(asset_memory_entry* ToSplit, u32 SplitOffset) {
	/*
		NOTE(dima): If equal then second block will be 0 bytes,
		so we dont need equal and the sign is <
	*/
	Assert(SplitOffset < ToSplit->DataSize);


}

void ASSETSInit(asset_system* System, stacked_memory* AssetSystemMemory) {

	System->AssetSystemMemory = AssetSystemMemory;
	System->TempMemoryForFileLoading = SplitStackedMemory(System->AssetSystemMemory, KILOBYTES(500));

	System->AssetPoolBlockSentinel = {};
	System->AssetPoolBlockSentinel.NextBlock = &System->AssetPoolBlockSentinel;
	System->AssetPoolBlockSentinel.PrevBlock = &System->AssetPoolBlockSentinel;

	//NOTE(dima): Initializing sentinel memory entries
	System->FirstFreeMemoryEntry = {};
	System->FirstFreeMemoryEntry.NextAllocEntry = &System->FirstFreeMemoryEntry;
	System->FirstFreeMemoryEntry.PrevAllocEntry = &System->FirstFreeMemoryEntry;

	System->FirstUseMemoryEntry = {};
	System->FirstUseMemoryEntry.NextAllocEntry = &System->FirstUseMemoryEntry;
	System->FirstUseMemoryEntry.PrevAllocEntry = &System->FirstUseMemoryEntry;

	System->Assets[0] = {};
	System->AssetCount = 1;

	//NOTE(dima): INitializing groups
	for (int GroupIndex = 0; GroupIndex < GameAsset_Count; GroupIndex++) {
		game_asset_group* Group = System->AssetGroups + GroupIndex;

		Group->FirstAssetIndex = 0;
		Group->GroupAssetCount = 0;
	}
	 
	//NOTE(dima): integrating file groups to asset system
	platform_file_group FileGroup = PlatformApi.OpenAllFilesOfTypeBegin("../Data/", FileType_Asset);
	for (platform_file_entry* File = FileGroup.FirstFileEntry;
		File != 0;
		File = File->Next) 
	{
		stacked_memory TempMemory = BeginTempStackedMemory(
			&System->TempMemoryForFileLoading,
			System->TempMemoryForFileLoading.MaxSize);

		u32 CurrentOffset;
		u32 StartOffset = sizeof(asset_file_header);

		//asset_file_header* FileHeader = (asset_file_header*)File->Data;
		asset_file_header FileHeader;

		PlatformApi.ReadDataFromFileEntry(File, &FileHeader, 0, sizeof(asset_file_header));

		b32 HeaderIsEqual =
			FileHeader.AssetFileHeader[0] == 'G' &&
			FileHeader.AssetFileHeader[1] == 'A' &&
			FileHeader.AssetFileHeader[2] == 'S' &&
			FileHeader.AssetFileHeader[3] == 'S';

		Assert(HeaderIsEqual);
		Assert(FileHeader.Version >= ASSET_FILE_VERSION);
		Assert(FileHeader.AssetGroupsCount == GameAsset_Count);

		//NOTE(dima): Reading file asset groups
		game_asset_group* FileGroups = PushArray(
			&TempMemory, game_asset_group, GameAsset_Count);
		asset_file_asset_group* BufferForReadFileGroups = PushArray(
			&TempMemory, asset_file_asset_group, FileHeader.AssetGroupsCount);

		PlatformApi.ReadDataFromFileEntry(
			File, 
			BufferForReadFileGroups, 
			StartOffset, 
			sizeof(asset_file_asset_group) * FileHeader.AssetGroupsCount);

		//NOTE(dima): Clearing target groups
		for (int GroupIndex = 0;
			GroupIndex < GameAsset_Count;
			GroupIndex++)
		{
			game_asset_group* Grp = &FileGroups[GroupIndex];

			Grp->FirstAssetIndex = 0;
			Grp->GroupAssetCount = 0;
		}

		//NOTE(dima): COpying data to target groups
		for (int GroupIndex = 0;
			GroupIndex < FileHeader.AssetGroupsCount;
			GroupIndex++)
		{
			game_asset_group* Grp = &FileGroups[GroupIndex];
			asset_file_asset_group* RdGroup = &BufferForReadFileGroups[GroupIndex];

			Grp->FirstAssetIndex = RdGroup->FirstAssetIndex;
			Grp->GroupAssetCount = RdGroup->GroupAssetCount;
		}
		StartOffset += sizeof(asset_file_asset_group) * FileHeader.AssetGroupsCount;

		//NOTE(dima): Reading file asset lines offsets
		u32 FileAssetCount = FileHeader.AssetsCount;
		u32* AssetLinesOffsets = PushArray(&TempMemory, u32, FileAssetCount);

		PlatformApi.ReadDataFromFileEntry(
			File,
			AssetLinesOffsets,
			FileHeader.LinesOffsetsByteOffset,
			FileAssetCount * sizeof(u32));

		gass_tag* HelperTags = PushArray(&TempMemory, gass_tag, 64);

		for (int FileGroupIndex = 0;
			FileGroupIndex < FileHeader.AssetGroupsCount;
			FileGroupIndex++)
		{
			game_asset_group* FileGroup = FileGroups + FileGroupIndex;

			for (int ToGroupIndex = 0;
				ToGroupIndex < GameAsset_Count;
				ToGroupIndex++)
			{
				game_asset_group* ToGroup = System->AssetGroups + ToGroupIndex;
				
				if ((ToGroupIndex == FileGroupIndex) && (FileGroup->GroupAssetCount)) {

					if (ToGroup->GroupAssetCount == 0) {
						ToGroup->FirstAssetIndex = System->AssetCount;
					}

					/*
						NOTE(dima): Subtract 1 here because we don't write zero asset
						to asset lines offset in the asset packer
					*/
					u32 FirstAssetIndex = FileGroup->FirstAssetIndex - 1;
					u32 OnePastLastAssetIndex = FirstAssetIndex + FileGroup->GroupAssetCount;

					for (int FileAssetIndex = FirstAssetIndex;
						FileAssetIndex < OnePastLastAssetIndex;
						FileAssetIndex++)
					{
						gass_header GASS_;
						PlatformApi.ReadDataFromFileEntry(
							File,
							&GASS_,
							AssetLinesOffsets[FileAssetIndex],
							sizeof(gass_header));

						gass_header* GASS = &GASS_;

						game_asset* Asset = AddGameAsset(System);

						Asset->Type = GASS->AssetType;
						Asset->State = GameAssetState_Loaded;
						CopyStrings(Asset->AssetFilePath, File->FileName);


						//NOTE(dima): Incrementing target group asset count
						ToGroup->GroupAssetCount++;

						//NOTE(dima): Reading asset tags
						u32 TagsMemorySize = GASS->AssetTotalTagsSize;

						Asset->Tags = 0;
						Asset->TagCount = GASS->TagCount;

						if (Asset->TagCount) {
							Asset->Tags = PushArray(
								System->AssetSystemMemory,
								game_asset_tag,
								Asset->TagCount);

							u32 TagMemoryRequired = Asset->TagCount * sizeof(game_asset_tag);
							Assert(GASS->AssetTotalTagsSize == TagMemoryRequired);

							PlatformApi.ReadDataFromFileEntry(
								File,
								HelperTags,
								AssetLinesOffsets[FileAssetIndex] + GASS->LineFirstTagOffset,
								GASS->AssetTotalTagsSize);

							for (int TagIndex = 0;
								TagIndex < Asset->TagCount;
								TagIndex++)
							{
								gass_tag* ReadTag = HelperTags + TagIndex;
								game_asset_tag* ToTag = Asset->Tags + TagIndex;

								ToTag->Type = ReadTag->Type;
								ToTag->Value_Int = ReadTag->Value_Int;
							}
						}

						u32 AssetDataOffsetInFile = AssetLinesOffsets[FileAssetIndex] + GASS->LineDataOffset;
						Asset->AssetDataOffsetInFile = AssetDataOffsetInFile;
						Asset->AssetDataSize = GASS->AssetTotalDataSize;
						Asset->AssetData = 0;

						switch (Asset->Type) {
							case AssetType_Bitmap: {
								bitmap_info* TargetBitmap = &Asset->Bitmap;
								*TargetBitmap = {};

								AssetAllocateBitmap(
									TargetBitmap,
									0,
									GASS->Bitmap.Width,
									GASS->Bitmap.Height);

								u32 DataMemSize = GASS->AssetTotalDataSize;
								void* DataMem = AssetRequestMemory(System, DataMemSize);

								//NOTE(dima): Reading data from file
								PlatformApi.ReadDataFromFileEntry(
									File,
									DataMem,
									AssetDataOffsetInFile,
									DataMemSize);

								AssetAllocateBitmap(
									TargetBitmap,
									DataMem,
									GASS->Bitmap.Width,
									GASS->Bitmap.Height);
							}break;

							case AssetType_Font: {
								font_info* Font = &Asset->Font;
								*Font = {};

								Font->AscenderHeight = GASS->Font.AscenderHeight;
								Font->DescenderHeight = GASS->Font.DescenderHeight;
								Font->LineGap = GASS->Font.LineGap;
								Font->GlyphsCount = GASS->Font.GlyphsCount;
								Font->MaxGlyphsCount = GASS->Font.MaxGlyphsCount;

								AssetAllocateBitmap(
									&Font->FontAtlasImage,
									0,
									GASS->Font.AtlasBitmapWidth,
									GASS->Font.AtlasBitmapHeight);

								u32 DataMemSize =
									GASS->AssetTotalDataSize +
									sizeof(u32) * GASS->Font.GlyphsCount;
								void* DataMem = AssetRequestMemory(System, DataMemSize);

								//NOTE(dima): Reading data from file
								PlatformApi.ReadDataFromFileEntry(
									File,
									DataMem,
									AssetDataOffsetInFile,
									DataMemSize);

								u32 OffsetToMapping = -sizeof(gass_header) + GASS->Font.LineOffsetToMapping;
								u32 OffsetToKerning = -sizeof(gass_header) + GASS->Font.LineOffsetToKerningPairs;
								u32 OffsetToAtlasss = -sizeof(gass_header) + GASS->Font.LineOffsetToAtlasBitmapPixels;

								Font->CodepointToGlyphMapping = (int*)((u8*)DataMem + OffsetToMapping);
								Font->KerningPairs = (float*)((u8*)DataMem + OffsetToKerning);
								void* AtlasBitmapPixels = (u8*)DataMem + OffsetToAtlasss;
								Font->GlyphIDs = (u32*)((u8*)DataMem + GASS->AssetTotalDataSize);

								//NOTE(dima): Restoring asset id's
								for (int GlyphIndex = 0;
									GlyphIndex < GASS->Font.GlyphsCount;
									GlyphIndex++)
								{
									Font->GlyphIDs[GlyphIndex] = ToGroup->FirstAssetIndex - 1 + GASS->Font.FirstGlyphID + GlyphIndex;
								}

								AssetAllocateBitmap(
									&Font->FontAtlasImage,
									AtlasBitmapPixels,
									GASS->Font.AtlasBitmapWidth,
									GASS->Font.AtlasBitmapHeight);
							}break;

							case AssetType_FontGlyph: {
								glyph_info* Glyph = &Asset->Glyph;

								u32 DataMemSize = GASS->AssetTotalDataSize;
								void* DataMem = AssetRequestMemory(System, DataMemSize);

								AssetAllocateBitmap(
									&Glyph->Bitmap,
									0,
									GASS->Glyph.BitmapWidth,
									GASS->Glyph.BitmapHeight);

								//NOTE(dima): Reading data from file
								PlatformApi.ReadDataFromFileEntry(
									File,
									DataMem,
									AssetDataOffsetInFile,
									DataMemSize);

								Glyph->Codepoint = GASS->Glyph.Codepoint;
								Glyph->Advance = GASS->Glyph.Advance;
								Glyph->XOffset = GASS->Glyph.XOffset;
								Glyph->YOffset = GASS->Glyph.YOffset;
								Glyph->LeftBearingX = GASS->Glyph.LeftBearingX;
								Glyph->Width = GASS->Glyph.BitmapWidth;
								Glyph->Height = GASS->Glyph.BitmapHeight;

								float MinUVx;
								float MinUVy;
								float MaxUVx;
								float MaxUVy;

#if 1
								MinUVx = GASS->Glyph.AtlasMinUV_x;
								MinUVy = GASS->Glyph.AtlasMinUV_y;
								MaxUVx = GASS->Glyph.AtlasMaxUV_x;
								MaxUVy = GASS->Glyph.AtlasMaxUV_y;
#else
								MinUVx = From0xFFFFTo01(GASS->Glyph.AtlasMinUV16x16y & 0xFFFF);
								MinUVy = From0xFFFFTo01((GASS->Glyph.AtlasMinUV16x16y >> 16) & 0xFFFF);
								MaxUVx = From0xFFFFTo01(GASS->Glyph.AtlasMaxUV16x16y & 0xFFFF);
								MaxUVy = From0xFFFFTo01((GASS->Glyph.AtlasMaxUV16x16y >> 16) & 0xFFFF);
#endif

								Glyph->AtlasMinUV = V2(MinUVx, MinUVy);
								Glyph->AtlasMaxUV = V2(MaxUVx, MaxUVy);

								AssetAllocateBitmap(
									&Glyph->Bitmap,
									DataMem,
									GASS->Glyph.BitmapWidth,
									GASS->Glyph.BitmapHeight);
							}break;

							case AssetType_Mesh: {
								mesh_info* TargetMesh = &Asset->Mesh;

								u32 DataMemSize = GASS->AssetTotalDataSize;
								void* DataMem = AssetRequestMemory(System, DataMemSize);

								//NOTE(dima): Reading data from file
								PlatformApi.ReadDataFromFileEntry(
									File,
									DataMem,
									AssetDataOffsetInFile,
									DataMemSize);

								void* VerticesData = DataMem;
								void* IndicesData = (u8*)DataMem + GASS->Mesh.VertexTypeSize * GASS->Mesh.VerticesCount;

								TargetMesh->IndicesCount = GASS->Mesh.IndicesCount;
								TargetMesh->Indices = (u32*)IndicesData;

								TargetMesh->Handle = 0;

								TargetMesh->VerticesCount = GASS->Mesh.VerticesCount;
								switch (GASS->Mesh.MeshType) {
									case GassMeshType_Simple: {
										TargetMesh->MeshType = MeshType_Simple;
										TargetMesh->Vertices = (vertex_info*)VerticesData;

									}break;

									case GassMeshType_Skinned: {
										TargetMesh->MeshType = MeshType_Skinned;
										TargetMesh->SkinnedVertices = (skinned_vertex_info*)VerticesData;
									}break;
								}

							}break;
						}
					}
				}
			}
		}

		EndTempStackedMemory(&System->TempMemoryForFileLoading, &TempMemory);
	}
	PlatformApi.OpenAllFilesOfTypeEnd(&FileGroup);
}
