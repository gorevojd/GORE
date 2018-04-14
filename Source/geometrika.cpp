#include "geometrika.h"

void GEOMKAUpdateAndRender(geometrika_state* State, asset_system* AssetSystem, render_state* RenderStack, input_system* Input) {
	if (!State->IsInitialized) {

		State->Camera = GAMECreateCamera();
		State->CapturingMouse = 1;

		State->CubeMat = LITCreateSurfaceMaterial(32.0f);
		State->PlaneMat = LITCreateSurfaceMaterial(16.0f);

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
	mesh_info* PlaneInfo = ASSETRequestFirstMesh(AssetSystem, GameAsset_Plane);
	mesh_info* SphereMesh = ASSETRequestFirstMesh(AssetSystem, GameAsset_Sphere);

#if 1
	for (int i = -5; i < 5; i++) {
		for (int j = -5; j < 5; j++) {
			for (int k = -5; k < 5; k++) {

				if (i != 0 && j != 0 && k != 0) {
					mat4 Transform = TranslationMatrix(V3(i * 10, j * 10, k * 10));

					RENDERPushMesh(RenderStack, CubeInfo, Transform, &State->CubeMat);
				}
			}
		}
	}
	
	v3 SpherePos1 = V3(10.0f * Sin(Input->Time), 15.0f, 5.0f * Cos(Input->Time));
	mat4 SphereMat1 = TranslationMatrix(SpherePos1) * ScalingMatrix(V3(5.0f, 5.0f, 5.0f));

	v3 SpherePos2 = V3(5.0f * Sin(Input->Time * 0.5f), 3.0f, 5.0f * Cos(Input->Time * 0.5f));
	mat4 SphereMat2 = TranslationMatrix(SpherePos2) * ScalingMatrix(V3(3.0f, 3.0f, 3.0f));

	RENDERPushMesh(RenderStack, SphereMesh, SphereMat1, &State->CubeMat);
	RENDERPushMesh(RenderStack, SphereMesh, SphereMat2, &State->CubeMat);

	RENDERPushMesh(RenderStack, PlaneInfo, ScalingMatrix(V3(100, 100, 100)), &State->PlaneMat);
#endif

	game_camera_setup CameraSetup = GAMECameraSetup(
		State->Camera,
		RenderStack->RenderWidth,
		RenderStack->RenderHeight,
		CameraProjection_Perspective);

	RENDERSetCameraSetup(RenderStack, CameraSetup);
}