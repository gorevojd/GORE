#include "geometrika.h"

void GEOMKAUpdateAndRender(geometrika_state* State, asset_system* AssetSystem, render_state* RenderStack, input_system* Input) {
	if (!State->IsInitialized) {

		State->Camera = GAMECreateCamera();
		State->CapturingMouse = 1;

		State->IsInitialized = 1;
	}

	float MouseSpeed = 0.1f;

	if (State->CapturingMouse) {
		float DeltaXAngle = -(Input->MouseP.x - Input->CenterP.x) * DEG_TO_RAD * MouseSpeed;
		float DeltaYAngle = -(Input->MouseP.y - Input->CenterP.y) * DEG_TO_RAD * MouseSpeed;

		GAMEUpdateCameraVectors(&State->Camera, DeltaYAngle, DeltaXAngle, 0.0f);
		
		PlatformApi.PlaceCursorAtCenter();
	}

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
	if (ButtonIsDown(Input, KeyType_LShift)) {
		CameraSpeed *= 10.0f;
	}
	MoveVector = MoveVector * CameraSpeed * Input->DeltaTime;
	State->Camera.Position += State->Camera.Front * MoveVector.z;
	State->Camera.Position -= State->Camera.Left * MoveVector.x;
	State->Camera.Position += State->Camera.Up * MoveVector.y;

	if (ButtonWentDown(Input, KeyType_Backquote)) {
		State->CapturingMouse = !State->CapturingMouse;
	}

	if (ButtonIsDown(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 0;
	}
	if (ButtonWentUp(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 1;
	}

	mesh_info* CubeInfo = ASSETRequestFirstMesh(AssetSystem, GameAsset_Cube);

	for (int i = -5; i < 5; i++) {
		for (int j = -5; j < 5; j++) {
			for (int k = -5; k < 5; k++) {

				if (i != 0 && j != 0 && k != 0) {
					mat4 Transform = TranslationMatrix(V3(i * 10, j * 10, k * 10));

					RENDERPushMesh(RenderStack, CubeInfo, Transform);
				}
			}
		}
	}


	game_camera_setup CameraSetup = GAMECameraSetup(
		State->Camera,
		RenderStack->RenderWidth,
		RenderStack->RenderHeight,
		CameraProjection_Perspective);

	RENDERSetCameraSetup(RenderStack, CameraSetup);
}