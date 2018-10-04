#ifndef GORE_ASSET_IDENTIFIERS_H_INCLUDED
#define GORE_ASSET_IDENTIFIERS_H_INCLUDED

enum game_asset_tag_id {
	GameAssetTag_None,

	GameAssetTag_Font_Debug,
	GameAssetTag_Font_Golden,

	GameAssetTag_LOD,
};

enum game_asset_group_id {
	GameAsset_Lilboy,

	GameAsset_OblivonMemeImage,
	GameAsset_PotImage,
	GameAsset_AlphaImage,

	GameAsset_Checkerboard,

	GameAsset_ContainerDiffImage,
	GameAsset_ContainerSpecImage,

	GameAsset_Cube,
	GameAsset_Plane,
	GameAsset_Sphere,
	GameAsset_Cylynder,

	GameAsset_Font,
	GameAsset_MainMenuFont,

	GameAsset_MyVoxelAtlas,

	GameAsset_Count,
};

#endif