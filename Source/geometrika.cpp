#include "geometrika.h"

void GEOMKAUpdateAndRender(geometrika_state* State, render_stack* RenderStack, input_system* Input) {
	if (!State->IsInitialized) {

		State->Camera = GAMECreateCamera();

		State->IsInitialized = 1;
	}

	float DeltaXAngle = -(Input->GlobalMouseX - Input->LastMouseX) * DEG_TO_RAD;
	float DeltaYAngle = -(Input->GlobalMouseY - Input->LastMouseY) * DEG_TO_RAD;

	GAMEUpdateCameraVectors(&State->Camera, DeltaYAngle, DeltaXAngle, 0.0f);

	//NOTE(dima): Camera movement
	v3 MoveVector = V3(0.0f, 0.0f, 0.0f);
	if (ButtonIsDown(Input, KeyType_A)) {
		MoveVector += V3(1.0f, 0.0f, 0.0f);
	}
	if (ButtonIsDown(Input, KeyType_D)) {
		MoveVector -= V3(1.0f, 0.0f, 0.0f);
	}
	if (ButtonIsDown(Input, KeyType_W)) {
		MoveVector += V3(0.0f, 0.0f, 1.0f);
	}
	if (ButtonIsDown(Input, KeyType_S)) {
		MoveVector -= V3(0.0f, 0.0f, 1.0f);
	}
	MoveVector = NOZ(MoveVector);

	float CameraSpeed = 10.0f;
	MoveVector = MoveVector * CameraSpeed * Input->DeltaTime;
	State->Camera.Position += State->Camera.Front * MoveVector.z;
	State->Camera.Position += State->Camera.Left * MoveVector.x;

	game_camera_setup CameraSetup = GAMECameraSetup(
		&State->Camera,
		RenderStack->RenderWidth,
		RenderStack->RenderHeight,
		CameraProjection_Perspective);

	RENDERPushCameraSetup(RenderStack, CameraSetup);
}