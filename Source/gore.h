#ifndef GORE_H_INCLUDED
#define GORE_H_INCLUDED

#include "gore_engine.h"
#include "gore_game_mode.h"
#include "gore_game_common.h"

#include "gore_debug.h"

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

struct gore_player {

	v2 P;
	v2 Dim;
	v2 Align;

	v2 Velocity;
	v2 Gravity;
	v2 InitJumpVelocity;
	int JumpCounter;

	float Speed;
	bitmap_id PlayerBitmapID;

	b32 FacingLeft;

	float Health;
	float MaxHealth;
};

enum gore_flying_weapon_type {
	FlyingWeapon_Knife,
	FlyingWeapon_Bottle,

	FlyingWeapon_Count,
};

struct gore_flying_weapon_data {
	v2 Dim;
	v2 Align;
	float Speed;
	float Range;
};

inline gore_flying_weapon_data GoreFlWeapon(
	v2 Dim,
	v2 Align,
	float Speed,
	float Range) 
{
	gore_flying_weapon_data Result = {};

	Result.Align = Align;
	Result.Dim = Dim;
	Result.Speed = Speed;
	Result.Range = Range;

	return(Result);
}

struct gore_flying_weapon {
	v2 P;
	v2 dP;

	v2 Dim;
	v2 Align;

	b32 IsActive;
	float TimeLived;

	float TimeToLive;

	int FlWeaponDatabaseIndex;
};

enum entity_type {
	Entity_Player,
	Entity_Wall,
	Entity_FlyingWeapon,
};

struct gore_state {
	b32 IsInitialized;
	stacked_memory* GameModeMemory;

	float ScreenWidth;
	float ScreenHeight;
	float OneOverScreenWidth;
	float OneOverScreenHeight;

	int ViewPlayerIndex;

	int PlayerCount;
	gore_player* Players;

	int WallCount;
	gore_wall* Walls;

	//NOTE(dima): Flying weapons database
	gore_flying_weapon_data FlyingWeaponDatabase[FlyingWeapon_Count];

	//NOTE(dima): Flying entities
	int FlyingQueueCurrent;
	int FlyingQueueCount;
	gore_flying_weapon* FlyingQueue;

	float PixelsPerMeter;

	float CurrentCameraScaling;
	game_camera Camera;
	game_camera_setup CameraSetup;
};

void UpdateGore(game_mode_state* GameModeState, engine_systems* EngineSystems);

#endif