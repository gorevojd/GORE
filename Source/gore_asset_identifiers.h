#ifndef GORE_ASSET_IDENTIFIERS_H_INCLUDED
#define GORE_ASSET_IDENTIFIERS_H_INCLUDED

enum game_asset_tag_id {
	GameAssetTag_None,

	GameAssetTag_Font_Debug,
	GameAssetTag_Font_Golden,
	GameAssetTag_Font_MainMenuFont,

	GameAssetTag_Lilboy,

	GameAssetTag_LOD,

	GameAssetTag_Count,
};

enum game_asset_group_id {
	GameAsset_Lilboy,
	GameAsset_Knife,
	GameAsset_Bottle,

	GameAsset_OblivonMemeImage,
	GameAsset_PotImage,
	GameAsset_AlphaImage,
	GameAsset_Checkerboard,
	GameAsset_ContainerDiffImage,
	GameAsset_ContainerSpecImage,
	GameAsset_VoxelAtlasBitmap,

	GameAsset_Cube,
	GameAsset_Plane,
	GameAsset_Sphere,
	GameAsset_Cylynder,

	GameAsset_FontGlyph,
	GameAsset_Font,

	GameAsset_Count,
};

#endif