#include "gore_lpterrain.h"

//#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

inline float LpterGetHeight(lpter_terrain* Terrain, int X, int Y) {
	float Result = Terrain->Heights[Y * LPTER_VERTS_PER_SIDE + X];

	return(Result);
}

inline lpter_vertex LpterCreateVertex(v3 Pos, v3 Color) {
	lpter_vertex Result = {};

	Result.P = Pos;
	Result.Color = PackRGBA(V4(Color, 1.0f));

	return(Result);
}

inline void LpterWriteNormal(lpter_mesh* Mesh, v3 Normal, int QuadNormalIndex, int QuadX, int QuadY) {
	Mesh->Normals[QuadY * LPTER_QUADS_PER_SIDE * 2 + QuadX * 2 + QuadNormalIndex] = Normal;
}

inline void LpterWritePolygon(lpter_mesh* Mesh, v3 PolyVerts[4], v3 PolyColors[4], int QuadX, int QuadY) {
	v3 Normal1 = Normalize(Cross(PolyVerts[2] - PolyVerts[0], PolyVerts[1] - PolyVerts[0]));
	v3 Normal2 = Normalize(Cross(PolyVerts[3] - PolyVerts[0], PolyVerts[2] - PolyVerts[0]));

	LpterWriteNormal(Mesh, Normal1, 0, QuadX, QuadY);
	LpterWriteNormal(Mesh, Normal2, 1, QuadX, QuadY);

	lpter_vertex Tri1[3];
	Tri1[0] = LpterCreateVertex(PolyVerts[0], PolyColors[0]);
	Tri1[1] = LpterCreateVertex(PolyVerts[1], PolyColors[1]);
	Tri1[2] = LpterCreateVertex(PolyVerts[2], PolyColors[2]);

	lpter_vertex Tri2[3];
	Tri2[0] = LpterCreateVertex(PolyVerts[0], PolyColors[0]);
	Tri2[1] = LpterCreateVertex(PolyVerts[2], PolyColors[2]);
	Tri2[2] = LpterCreateVertex(PolyVerts[3], PolyColors[3]);

	lpter_vertex* WriteToArray = Mesh->Verts + QuadY * LPTER_QUADS_PER_SIDE * 6 + QuadX * 6;

	WriteToArray[0] = Tri1[0];
	WriteToArray[1] = Tri1[1];
	WriteToArray[2] = Tri1[2];

	WriteToArray[3] = Tri2[0];
	WriteToArray[4] = Tri2[1];
	WriteToArray[5] = Tri2[2];

	Mesh->VertsCount += 6;
}

struct lpter_biome {
	float MinHg;
	float MaxHg;

	v3 Color;

	char* Name;
};

inline lpter_biome LpterBiome(float MinHeight, float MaxHeight, v3 Color, char* DEBUGName) {
	lpter_biome Biome = {};

	Biome.MinHg = MinHeight;
	Biome.MaxHg = MaxHeight;
	Biome.Color = Color;
	Biome.Name = DEBUGName;

	return(Biome);
}

static v3 LpterGetTransitionedColor(
	lpter_biome* Biomes,
	int BiomesCount,
	float NormHeight)
{
	for (int BiomeIndex = 0;
		BiomeIndex < BiomesCount;
		BiomeIndex++)
	{
		lpter_biome* Biome = Biomes + BiomeIndex;

		if (NormHeight >= Biome->MinHg &&
			NormHeight <= Biome->MaxHg)
		{
			return(Biome->Color);
		}
	}

	lpter_biome* MaxBiome = 0;
	lpter_biome* MinBiome = 0;

	float MaxBiomeHeight = -999999.0f;
	float MinBiomeHeight = 999999.0f;

	for (int BiomeIndex = 0;
		BiomeIndex < BiomesCount;
		BiomeIndex++)
	{
		lpter_biome* Biome = Biomes + BiomeIndex;

		if (Biome->MaxHg > MaxBiomeHeight) {
			MaxBiome = Biome;
			MaxBiomeHeight = Biome->MaxHg;
		}

		if (Biome->MinHg < MinBiomeHeight) {
			MinBiome = Biome;
			MinBiomeHeight = Biome->MinHg;
		}
	}

	lpter_biome* CloserLeftBiome = 0;
	lpter_biome* CloserRightBiome = 0;

	float CloserLeftMax = -999999.0f;
	float CloserRightMin = 999999.0f;
	
	for (int BiomeIndex = 0;
		BiomeIndex < BiomesCount;
		BiomeIndex++)
	{
		lpter_biome* Biome = Biomes + BiomeIndex;

		if (Biome->MaxHg > CloserLeftMax && 
			NormHeight > Biome->MaxHg) 
		{
			CloserLeftMax = Biome->MaxHg;
			CloserLeftBiome = Biome;
		}

		if (Biome->MinHg < CloserRightMin && 
			NormHeight < Biome->MinHg) 
		{
			CloserRightMin = Biome->MinHg;
			CloserRightBiome = Biome;
		}
	}

	float MinLerpRange = 0.0f;
	float MaxLerpRange = 1.0f;
	v3 MinLerpColor;
	v3 MaxLerpColor;
	if (CloserLeftBiome) {
		MinLerpRange = CloserLeftBiome->MaxHg;
		MinLerpColor = CloserLeftBiome->Color;
	}
	else {
		return(MinBiome->Color);
	}
	if (CloserRightBiome) {
		MaxLerpRange = CloserRightBiome->MinHg;
		MaxLerpColor = CloserRightBiome->Color;
	}
	else {
		return(MaxBiome->Color);
	}

	float Delta = (NormHeight - MinLerpRange) / (MaxLerpRange - MinLerpRange);
	v3 Result = Lerp(MinLerpColor, MaxLerpColor, Delta);

	return(Result);
}

void LpterGenerateMesh(lpter_mesh* Mesh, lpter_terrain* Terrain) {
	Mesh->VertsCount = 0;

	float PerVertexOffset = (float)LPTER_TERRAIN_METERS / (float)LPTER_QUADS_PER_SIDE;

	v3 GroundColor = V3(0.1f, 0.33f, 0.1f);
	v3 RockColor = V3(0.6f, 0.6f, 0.65f);
	v3 SandColor = V3(0.65, 0.56f, 0.31f);

	lpter_biome Biomes[] = {
		LpterBiome(0.0f, 0.5f, SandColor, "Sand"),
		LpterBiome(0.6f, 0.7f, GroundColor, "Ground"),
		LpterBiome(0.8f, 1.0f, RockColor, "Rock")
	};

	for (int QuadY = 0;
		QuadY < LPTER_QUADS_PER_SIDE;
		QuadY++)
	{
		float QuadYOffset = (float)QuadY * PerVertexOffset;

		for (int QuadX = 0;
			QuadX < LPTER_QUADS_PER_SIDE;
			QuadX++)
		{
			float QuadXOffset = (float)QuadX * PerVertexOffset;

			float Heights[4];
			Heights[0] = LpterGetHeight(Terrain, QuadX, QuadY);
			Heights[1] = LpterGetHeight(Terrain, QuadX + 1, QuadY);
			Heights[2] = LpterGetHeight(Terrain, QuadX + 1, QuadY + 1);
			Heights[3] = LpterGetHeight(Terrain, QuadX, QuadY + 1);

			v3 VertPss[4];
			VertPss[0] = V3(QuadXOffset, Heights[0], QuadYOffset);
			VertPss[1] = V3(QuadXOffset + PerVertexOffset, Heights[1], QuadYOffset);
			VertPss[2] = V3(QuadXOffset + PerVertexOffset, Heights[2], QuadYOffset + PerVertexOffset);
			VertPss[3] = V3(QuadXOffset, Heights[3], QuadYOffset + PerVertexOffset);

			v3 VertColors[4];
			for (int ColorIndex = 0;
				ColorIndex < 4;
				ColorIndex++)
			{
				float Hg = Heights[ColorIndex] * Terrain->OneOverHeightMultiplier;

				float NormHeight = Hg * 0.5f + 0.5f;

				VertColors[ColorIndex] = LpterGetTransitionedColor(Biomes, ArrayCount(Biomes), NormHeight);

				//float t = Clamp01((Heights[ColorIndex] * Terrain->OneOverHeightMultiplier - (-1.0f)) / 2.0f);
				//VertColors[ColorIndex] = Hadamard(VertColors[ColorIndex], Lerp(V3(0.4f, 0.4f, 0.4f), V3(1.0f, 1.0f, 1.0f), t));
			}

			LpterWritePolygon(Mesh, VertPss, VertColors, QuadX, QuadY);
		}
	}
}


float LpterNoise(float X, float Z) {
	float Result = 0.0f;

	float Freq = 1.0f;
	float Ampl = 1.0f;

	int Octaves = 6;
	float dFreq = 2.0f;
	//NOTE(dima): [0.0 - 1.0]
	float ddFreq = 0.5f;
	float dAmpl = 0.5f;

	float StartAngle = -GORE_PI / 2.0f / 0.8f;
	float EndAngle = GORE_PI / 2.0f / 0.8f;
	float OneOctaveAngularDisplacement = (EndAngle - StartAngle) / Octaves;

	for (int OctaveIndex = 0;
		OctaveIndex < Octaves;
		OctaveIndex++)
	{
		float StepNoise = stb_perlin_noise3(X * Freq, 0.0f, Z * Freq, 0, 0, 0);
		//Ampl = Cos(StartAngle + OneOctaveAngularDisplacement * OctaveIndex);
		//StepNoise = StepNoise < 0.0f ? StepNoise * 0.5f : StepNoise;


		Result += StepNoise * Ampl;


		Freq = Freq * dFreq;
		//dFreq -= (dFreq - 1.0f) * ddFreq;
		Ampl = Ampl * dAmpl;
		//Ampl *= dAmpl;
	}

	float TestNoiseFreq = 1.0f;
	float TestNoise = stb_perlin_noise3(X * TestNoiseFreq, 0.0f, Z * TestNoiseFreq, 0, 0, 0);

	Result += TestNoise * 0.5f;

	return(Result);
}

void LpterGenerateTerrain(
	lpter_terrain* Terrain,
	int WorldIndexX,
	int WorldIndexY)
{
	Terrain->WorldIndexX = WorldIndexX;
	Terrain->WorldIndexY = WorldIndexY;

	float PerVertexOffset = (float)LPTER_TERRAIN_METERS / (float)LPTER_QUADS_PER_SIDE;

	float HeightMultiplier = 15.0f;

	Terrain->OneOverHeightMultiplier = 1.0f / HeightMultiplier;

	v2 TerrainWorldOffset = V2(
		(float)WorldIndexX * (float)LPTER_TERRAIN_METERS,
		(float)WorldIndexY * (float)LPTER_TERRAIN_METERS);

	int Octaves = 6;
	float Lacunarity = 2.0f;
	float Gain = 0.5f;

	for (int VerticalIndex = 0;
		VerticalIndex < LPTER_VERTS_PER_SIDE;
		VerticalIndex++)
	{
		float VertVerticalOffset = TerrainWorldOffset.y + (float)VerticalIndex * PerVertexOffset;

		for (int HorizontalIndex = 0;
			HorizontalIndex < LPTER_VERTS_PER_SIDE;
			HorizontalIndex++)
		{
			float VertHorzOffset = TerrainWorldOffset.x + (float)HorizontalIndex * PerVertexOffset;

			float* ToSet = Terrain->Heights + LPTER_VERTS_PER_SIDE * VerticalIndex + HorizontalIndex;

#if 0
			float Noise = stb_perlin_fbm_noise3(
				VertHorzOffset / LPTER_TERRAIN_METERS,
				0.0f,
				VertVerticalOffset / LPTER_TERRAIN_METERS,
				Lacunarity,
				Gain,
				Octaves,
				0, 0, 0);
#else
			float Noise = LpterNoise(
				VertHorzOffset / LPTER_TERRAIN_METERS / 0.5f,
				VertVerticalOffset / LPTER_TERRAIN_METERS / 0.5f);
#endif

			*ToSet = Noise * HeightMultiplier;
		}
	}

	LpterGenerateMesh(&Terrain->Mesh, Terrain);
}

inline lpter_water_vertex LpterWaterCreateVertex(v2 VertXZ, lpter_i8v4 OffsetsToOtherVerts) {
	lpter_water_vertex Result = {};

	Result.VertexXZ = VertXZ;
	Result.OffsetsToOtherVerts = OffsetsToOtherVerts;

	return(Result);
}

inline void LpterWaterWritePolygon(lpter_water* Water, v2 VertsXZ[4], int QuadX, int QuadY) {
	lpter_water_vertex* TriangleVerts = &Water->Vertices[QuadX * 6 + QuadY * LPTER_WATER_QUADS_PER_SIDE * 6];

	lpter_water_vertex Tri1[3];
	Tri1[0] = LpterWaterCreateVertex(VertsXZ[0], LpterI8V4(1, 1, 1, 0));
	Tri1[1] = LpterWaterCreateVertex(VertsXZ[1], LpterI8V4(-1, 0, 0, 1));
	Tri1[2] = LpterWaterCreateVertex(VertsXZ[2], LpterI8V4(0, -1, -1, -1));

	lpter_water_vertex Tri2[3];
	Tri2[0] = LpterWaterCreateVertex(VertsXZ[0], LpterI8V4(0, 1, 1, 1));
	Tri2[1] = LpterWaterCreateVertex(VertsXZ[2], LpterI8V4(-1, -1, -1, 0));
	Tri2[2] = LpterWaterCreateVertex(VertsXZ[3], LpterI8V4(1, 0, 0, -1));

	TriangleVerts[0] = Tri1[0];
	TriangleVerts[1] = Tri1[1];
	TriangleVerts[2] = Tri1[2];
	TriangleVerts[3] = Tri2[0];
	TriangleVerts[4] = Tri2[1];
	TriangleVerts[5] = Tri2[2];

	Water->VerticesCount += 6;
}

void LpterGenerateWater(
	lpter_water* Water,
	int WorldIndexX,
	int WorldIndexY,
	float WaterLevel) 
{
	Water->WaterLevel = WaterLevel;

	Water->VerticesCount = 0;

	Water->WorldIndexX = WorldIndexX;
	Water->WorldIndexY = WorldIndexY;

	float PerVertexOffset = (float)LPTER_TERRAIN_METERS / (float)LPTER_WATER_QUADS_PER_SIDE;
	Water->PerVertexOffset = PerVertexOffset;

	for (int VertQuadId = 0;
		VertQuadId < LPTER_WATER_QUADS_PER_SIDE;
		VertQuadId++)
	{
		for (int HorzQuadId = 0;
			HorzQuadId < LPTER_WATER_QUADS_PER_SIDE;
			HorzQuadId++)
		{
			float VertQuadOffset = (float)VertQuadId * PerVertexOffset;
			float HorzQuadOffset = (float)HorzQuadId * PerVertexOffset;

			v2 VertsXZ[4];
			VertsXZ[0] = V2(HorzQuadOffset, VertQuadOffset);
			VertsXZ[1] = V2(HorzQuadOffset + PerVertexOffset, VertQuadOffset);
			VertsXZ[2] = V2(HorzQuadOffset + PerVertexOffset, VertQuadOffset + PerVertexOffset);
			VertsXZ[3] = V2(HorzQuadOffset, VertQuadOffset + PerVertexOffset);

			LpterWaterWritePolygon(Water, VertsXZ, HorzQuadId, VertQuadId);
		}
	}
}