#ifndef GORE_H_INCLUDED
#define GORE_H_INCLUDED

#include "gore_engine.h"
#include "gore_game_mode.h"
#include "gore_game_common.h"

struct gore_edge {
	v2 Min;
	v2 Max;

	v3 EdgeLineEqation;

	float EdgeLen;
};

struct gore_raycast_hit {
	b32 HitHappened;

	float DistanseFromRayOrigin;

	v2 HitPoint;

	v2 HitNormal;
};

struct gore_ray2d {
	v2 Origin;
	v2 Direction;
	float Len;

	//NOTE(dima): SqLen might be used for some specific calculations
	float SqLen;
};

inline gore_edge GoreEdge(v2 Min, v2 Max) {
	gore_edge Result = {};

	Result.Min = Min;
	Result.Max = Max;

	Result.EdgeLineEqation = LineEquationFrom2Points(Min, Max);
	Result.EdgeLen = Magnitude(Max - Min);

	return(Result);
}

inline gore_ray2d GoreRay2D(v2 Origin, v2 Direction, float Len) {
	gore_ray2d Result = {};

	Result.Origin = Origin;
	Result.Direction = Direction;
	Result.Len = Len;
	Result.SqLen = Len * Len;

	return(Result);
}

inline gore_ray2d GoreRay2D(v2 Min, v2 Max) {
	v2 Diff = Max - Min;

	gore_ray2d Result = {};

	Result.Origin = Min;
	Result.Direction = Normalize(Diff);
	Result.SqLen = Dot(Diff, Diff);
	Result.Len = Sqrt(Result.SqLen);

	return(Result);
}

struct gore_wall {
	v2 At;
	v2 Dim;
	v2 TopLeftAlign;
	b32 IsDynamic;
};

enum entity_type {
	Entity_Player,
	Entity_Wall,
};

struct gore_state {
	b32 IsInitialized;
	stacked_memory* GameModeMemory;

	bitmap_id PlayerBitmapID;

	float ScreenWidth;
	float ScreenHeight;
	float OneOverScreenWidth;
	float OneOverScreenHeight;

	v2 PlayerP;
	v2 PlayerVelocity;
	v2 PlayerDim;
	v2 PlayerGravity;
	v2 PlayerInitJumpVelocity;

	b32 PlayerFacingLeft;
	float PlayerHealth;
	float PlayerMaxHealth;

	int WallCount;
	gore_wall* Walls;

	gore_edge WallEdges[4];

	v2 MasterGravity;

	float PixelsPerMeter;

	float CurrentCameraScaling;
	game_camera Camera;
	game_camera_setup CameraSetup;
};

void UpdateGore(game_mode_state* GameModeState, engine_systems* EngineSystems);

#endif