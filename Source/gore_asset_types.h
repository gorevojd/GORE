#ifndef GORE_ASSET_TYPES_H_INCLUDED
#define GORE_ASSET_TYPES_H_INCLUDED

//NOTE(dima): DO NOT CHANGE ORDER OF THEESE
struct vertex_info {
	v3 P;
	v3 N;
	v2 UV;
	v3 C;
	v3 T;
};

#define INFLUENCE_BONE_COUNT 4
//NOTE(dima): DO NOT CHANGE ORDER OF THEESE
struct skinned_vertex_info {
	v3 P;
	v3 N;
	v2 UV;
	v3 C;
	v3 T;

	u32 Bones[INFLUENCE_BONE_COUNT];
	float Weights[INFLUENCE_BONE_COUNT];
};

enum mesh_type {
	MeshType_Simple,
	MeshType_Skinned,
};

struct mesh_info {
	union {
		vertex_info* Vertices;
		skinned_vertex_info* SkinnedVertices;
	};
	u32 VerticesCount;

	u32 MeshType;

	u32* Indices;
	u32 IndicesCount;

	void* Handle;
};

struct model_info {

};

struct sound_info {

};

struct bitmap_info {
	u8* Pixels;

	u32 Width;
	u32 Height;
	v2 Align;

	u32 Pitch;

	float WidthOverHeight;

	void* TextureHandle;
};

struct glyph_info {
	int Codepoint;

	bitmap_info Bitmap;

	int Width;
	int Height;

	/*Theese are offset from glyph origin to top-left of bitmap*/
	float XOffset;
	float YOffset;
	float Advance;
	float LeftBearingX;

	v2 AtlasMinUV;
	v2 AtlasMaxUV;
};

#define MAX_FONT_INFO_GLYPH_COUNT 256
struct font_info {
	int CodepointToGlyphMapping[MAX_FONT_INFO_GLYPH_COUNT];

	float AscenderHeight;
	float DescenderHeight;
	float LineGap;

	int GlyphsCount;
	int* KerningPairs;
	glyph_info Glyphs[MAX_FONT_INFO_GLYPH_COUNT];

	bitmap_info FontAtlasImage;
};

enum voxel_material_type {
	VoxelMaterial_None,
	VoxelMaterial_Stone,
	VoxelMaterial_Ground,
	VoxelMaterial_Sand,
	VoxelMaterial_GrassyGround,
	VoxelMaterial_Lava,
	VoxelMaterial_SnowGround,
	VoxelMaterial_WinterGround,
	VoxelMaterial_Leaves,
	VoxelMaterial_Brick,
	VoxelMaterial_Logs,
	VoxelMaterial_Birch,
	VoxelMaterial_Tree,

	VoxelMaterial_GrassyBigBrick,
	VoxelMaterial_DecorateBrick,
	VoxelMaterial_BigBrick,
	VoxelMaterial_BookShelf,

	VoxelMaterial_Secret,

	VoxelMaterial_Count,
};

enum voxel_face_type_index {
	VoxelFaceTypeIndex_Top = 0,
	VoxelFaceTypeIndex_Bottom,
	VoxelFaceTypeIndex_Left,
	VoxelFaceTypeIndex_Right,
	VoxelFaceTypeIndex_Front,
	VoxelFaceTypeIndex_Back,

	VoxelFaceTypeIndex_Count,

	VoxelFaceTypeIndex_All,
	VoxelFaceTypeIndex_Side,
	VoxelFaceTypeIndex_TopBottom,
};

struct voxel_tex_coords_set{
	union {
		struct {
			union {
				struct {
					u8 Top;
					u8 Bottom;
				};
				u8 TopBottom;
			};
			union {
				struct {
					u8 Left;
					u8 Right;
					u8 Front;
					u8 Back;
				};
				u8 Side;
			};
		};

		u8 All;
		u8 Sets[VoxelFaceTypeIndex_Count];
	};
};

struct voxel_atlas_info {
	bitmap_info Bitmap;

	int MaxTexturesCount;
	int TexturesCount;

	int AtlasWidth;
	int OneTextureWidth;

	voxel_tex_coords_set* Materials;
	int MaterialsCount;
};

enum asset_type {
	AssetType_None,

	AssetType_Bitmap,
	AssetType_Sound,
	AssetType_Font,
	AssetType_Model,
	AssetType_Mesh,
	AssetType_VoxelAtlas,
};


#endif