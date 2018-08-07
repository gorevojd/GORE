#ifndef GORE_GAME_SETTINGS_H_DEFINED
#define GORE_GAME_SETTINGS_H_DEFINED

#include "gore_types.h"

enum texture_anisotropic_filter_level {
	AnisoLevel_1x,
	AnisoLevel_2x,
	AnisoLevel_4x,
	AnisoLevel_8x,
	AnisoLevel_16x,
};

enum antialiasing_type {
	AA_None,
	AA_MSAA_1x,
	AA_MSAA_2x,
	AA_MSAA_4x,
	AA_MSAA_8x,
	AA_MSAA_16x,
	AA_FXAA,
};


inline float GetAnisoLevelBasedOnParams(u32 TextureAnisotropicFilterLevelType, float MaxAnisoLevel) {
	float Result = 1.0f;
	switch (TextureAnisotropicFilterLevelType) {

		case AnisoLevel_1x: {
			Result = Min(1.0f, MaxAnisoLevel);
		}break;

		case AnisoLevel_2x: {
			Result = Min(2.0f, MaxAnisoLevel);
		}break;

		case AnisoLevel_4x: {
			Result = Min(4.0f, MaxAnisoLevel);
		}break;

		case AnisoLevel_8x: {
			Result = Min(8.0f, MaxAnisoLevel);
		}break;

		case AnisoLevel_16x: {
			Result = Min(16.0f, MaxAnisoLevel);
		}break;
	}

	return(Result);
}

inline b32 AntialiasingIsMSAA(u32 AntialiasingType) {
	b32 Result = 0;

	if (AntialiasingType == AA_MSAA_1x ||
		AntialiasingType == AA_MSAA_2x ||
		AntialiasingType == AA_MSAA_4x ||
		AntialiasingType == AA_MSAA_8x ||
		AntialiasingType == AA_MSAA_16x)
	{
		Result = 1;
	}

	return(Result);
}

inline int GetMSAALevel(u32 AntialiasingType) { 
	Assert(AntialiasingIsMSAA(AntialiasingType));

	int Result = 1;

	switch (AntialiasingType) {
		case AA_MSAA_1x:{
			Result = 1;
		}break;

		case AA_MSAA_2x: {
			Result = 2;
		}break;

		case AA_MSAA_4x: {
			Result = 4;
		}break;

		case AA_MSAA_8x: {
			Result = 8;
		}break;

		case AA_MSAA_16x: {
			Result = 16;
		}break;
	}

	return(Result);
}

struct game_settings{
	texture_anisotropic_filter_level AnisotropicFilterLevel;
	antialiasing_type AntialiasingType;
};

#endif