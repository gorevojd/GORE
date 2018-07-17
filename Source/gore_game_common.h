#ifndef GORE_GAME_COMMON_H_INCLUDED
#define GORE_GAME_COMMON_H_INCLUDED

#include "gore_math.h"

struct game_camera {

	float Pitch;
	float Yaw;
	float Roll;

	v3 Front;
	v3 Left;
	v3 Up;

	v3 Position;
	v3 dPosition;
};

struct game_camera_setup {
	game_camera Camera;

	mat4 ProjectionMatrix;
	mat4 ViewMatrix;

	mat4 ProjectionViewMatrix;
};

enum camera_projection_type {
	CameraProjection_None,

	CameraProjection_InfiniteOrthographic,
	CameraProjection_Orthographic,
	CameraProjection_Perspective,
};

inline game_camera GAMECreateCamera() {
	game_camera Result = {};

	Result.Front = V3(0.0f, 0.0f, 1.0f);
	Result.Left = V3(1.0f, 0.0f, 0.0f);
	Result.Up = V3(0.0f, 1.0f, 0.0f);
	Result.Position = V3(0.0f, 0.0f, 0.0f);
	Result.dPosition = V3(0.0f, 0.0f, 0.0f);

	return(Result);
}

inline mat4 GAMEGetCameraTransform(game_camera* Camera) {
	mat4 Result = LookAt(Camera->Position, Camera->Position + Camera->Front, V3(0.0f, 1.0f, 0.0f));

	return(Result);
}

extern void GAMEUpdateCameraVectors(
	game_camera* Camera,
	float DeltaPitch = 0.0f,
	float DeltaYaw = 0.0f,
	float DeltaRoll = 0.0f,
	v3 WorldUp = V3(0.0f, 1.0f, 0.0f));

extern game_camera_setup GAMECameraSetup(
	game_camera Camera,
	u32 Width,
	u32 Height,
	u32 ProjectionType = CameraProjection_InfiniteOrthographic,
	float Far = 1000.0f,
	float Near = 0.1f,
	float FieldOfView = 45.0f);

#endif