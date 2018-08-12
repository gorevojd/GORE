#ifndef GORE_LPTERRAIN_H_INCLUDED
#define GORE_LPTERRAIN_H_INCLUDED

#include "gore_platform.h"
#include "gore_colors.h"

#define LPTER_QUADS_PER_SIDE 127
#define LPTER_TERRAIN_METERS 127.0f
#define LPTER_VERTS_PER_SIDE (LPTER_QUADS_PER_SIDE + 1)
#define LPTER_TOTAL_UNIQUE_VERTS (LPTER_VERTS_PER_SIDE * LPTER_VERTS_PER_SIDE)
#define LPTER_TOTAL_MESH_VERTS (LPTER_QUADS_PER_SIDE * LPTER_QUADS_PER_SIDE * 6)
#define LPTER_TOTAL_QUADS (LPTER_QUADS_PER_SIDE * LPTER_QUADS_PER_SIDE)

struct lpter_world {

};

struct lpter_water {
	int WorldIndexX;
	int WorldIndexY;

	//NOTE(dima): Starting heights of water relative to water level
	float StartRelativeHeights[LPTER_TOTAL_UNIQUE_VERTS];
};

struct lpter_terrain {
	int WorldIndexX;
	int WorldIndexY;

	float OneOverHeightMultiplier;

	float Heights[LPTER_TOTAL_UNIQUE_VERTS];
};

struct lpter_vertex {
	v3 P;
	u32 Color;
};

//TODO(dima): Deallocate mesh in opengl code
struct lpter_mesh {
	lpter_vertex Verts[LPTER_TOTAL_MESH_VERTS];
	v3 Normals[LPTER_TOTAL_QUADS * 2];

	int VertsCount;

	void* MeshHandle0;
	void* MeshHandle1;
	void* MeshHandleTexBuf;
	void* NormTexHandle;
};

inline v3 LpterGetTerrainOffset(lpter_terrain* Terrain) {
	v3 Result = {};

	Result.x = (float)Terrain->WorldIndexX * (float)LPTER_TERRAIN_METERS;
	Result.y = 0.0f;
	Result.z = (float)Terrain->WorldIndexY * (float)LPTER_TERRAIN_METERS;

	return(Result);
}

void LpterGenerateTerrain(lpter_terrain* Terrain, int WorldIndexX, int WorldIndexY);
void LpterGenerateMesh(lpter_mesh* Mesh, lpter_terrain* Terrain);

#endif