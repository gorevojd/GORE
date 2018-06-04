#ifndef GORE_VOXEL_SHARED_H_INCLUDED
#define GORE_VOXEL_SHARED_H_INCLUDED

#include "gore_math.h"

union voxel_face_tex_coords_set {
	struct {
		v2 T0;
		v2 T1;
		v2 T2;
		v2 T3;
	};
	v2 T[4];
};

inline voxel_face_tex_coords_set
GetFaceTexCoordSetForTextureIndex(u32 TextureIndex, u32 OneTextureWidth, u32 AtlasWidth)
{
	float OneTexUVDelta = (float)OneTextureWidth / (float)AtlasWidth;

	int TexturesPerWidth = AtlasWidth / OneTextureWidth;

	int XIndex = TextureIndex % TexturesPerWidth;
	int YIndex = TextureIndex / TexturesPerWidth;

	voxel_face_tex_coords_set Result = {};

	Result.T0 = V2((float)XIndex * OneTexUVDelta, (float)YIndex * OneTexUVDelta);
	Result.T1 = V2((float)(XIndex + 1) * OneTexUVDelta, (float)YIndex * OneTexUVDelta);
	Result.T2 = V2((float)(XIndex + 1) * OneTexUVDelta, (float)(YIndex + 1) * OneTexUVDelta);
	Result.T3 = V2((float)XIndex * OneTexUVDelta, (float)(YIndex + 1) * OneTexUVDelta);

	return(Result);
}

#endif