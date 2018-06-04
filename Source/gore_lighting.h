#ifndef GORE_LIGHTING_H_INCLUDED
#define GORE_LIGHTING_H_INCLUDED

#include "gore_platform.h"

struct surface_material {
	v3 Color;

	bitmap_id Diffuse;
	bitmap_id Specular;
	bitmap_id Emissive;

	bitmap_info* DiffuseInfo;
	bitmap_info* SpecularInfo;
	bitmap_info* EmissiveInfo;

	float Shine;
};

struct point_light {
	v3 Pos;
	float Radius;
	v3 Color;

	float LinearC;
	float QuadraticC;
};

struct dir_light {
	v3 Dir;
	v3 Color;
};

struct lighting_setup {
	point_light Light1;
};

inline surface_material LITCreateSurfaceMaterial(float Shine, v3 Color) {
	surface_material Result = {};

	Result.Shine = Shine;
	Result.Color = Color;

	Result.Diffuse = 0;
	Result.Specular = 0;
	Result.Emissive = 0;

	Result.DiffuseInfo = 0;
	Result.SpecularInfo = 0;
	Result.EmissiveInfo = 0;

	return(Result);
}

inline point_light LITCreatePointLight(v3 Pos, float Radius, v3 Color) {
	point_light Result = {};

	Result.Pos = Pos;
	Result.Radius = Radius;
	Result.Color = Color;

	float OneOverRad = 1.0f / Radius;
	Result.LinearC = 2.0f * OneOverRad;
	Result.QuadraticC = OneOverRad * OneOverRad;

	return(Result);
}

#endif