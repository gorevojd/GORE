#ifndef GORE_ASSET_TYPES_H_INCLUDED
#define GORE_ASSET_TYPES_H_INCLUDED

#include "gore_types.h"
#include "gore_math.h"

typedef u32 bitmap_id;
typedef u32 font_id;
typedef u32 font_glyph_id;
typedef u32 sound_id;
typedef u32 model_id;
typedef u32 mesh_id;

enum load_mesh_vertex_layout {
	MeshVertexLayout_PUV,
	MeshVertexLayout_PUVN,
	MeshVertexLayout_PNUV,
	MeshVertexLayout_PUVNC,
	MeshVertexLayout_PNUVC,
};

//NOTE(dima): DO NOT CHANGE ORDER OF THEESE
struct vertex_info {
	v3 P;
	v3 N;
	v2 UV;
	v3 T;
};

#define INFLUENCE_BONE_COUNT 4
//NOTE(dima): DO NOT CHANGE ORDER OF THEESE
struct skinned_vertex_info {
	v3 P;
	v3 N;
	v2 UV;
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

	int MaxGlyphsCount;
	int GlyphsCount;
	float* KerningPairs;
	glyph_info Glyphs[MAX_FONT_INFO_GLYPH_COUNT];

	bitmap_info FontAtlasImage;

	//NOTE(dima): It used in asset packer to store temp First glyph ID
	u32 Reserved;
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

struct voxel_tex_coords_set {
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

typedef u32 voxel_vert_t;
#define VOXEL_VERTEX_SIZE sizeof(voxel_vert_t)

struct voxel_mesh_info {
	void* MeshHandle;
	//NOTE(dima): MeshHandle2 used to store VBO in openGL
	void* MeshHandle2;

	voxel_vert_t* Vertices;
	u32 VerticesCount;
};

enum asset_type {
	AssetType_None,

	AssetType_Bitmap,
	AssetType_Sound,
	AssetType_Font,
	AssetType_FontGlyph,
	AssetType_Model,
	AssetType_Mesh,
};



#endif