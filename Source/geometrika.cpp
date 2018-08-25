#include "geometrika.h"

void GEOMKAUpdateAndRender(stacked_memory* GameMemoryBlock, asset_system* AssetSystem, render_state* RenderStack, input_system* Input) {
	geometrika_state* State = (geometrika_state*)GameMemoryBlock->BaseAddress;

	if (!State->IsInitialized) {

		PushStruct(GameMemoryBlock, geometrika_state);

		State->Camera = GAMECreateCamera();
		State->CapturingMouse = 1;
		State->CameraAutoMove = 0;

		State->CubeMat = LITCreateSurfaceMaterial(32.0f, V3(0.9f, 0.1f, 0.1f));
		State->PlaneMat = LITCreateSurfaceMaterial(16.0f, V3(0.1f, 0.1f, 0.9f));

		State->PlaneMat.Diffuse = GetFirstBitmap(AssetSystem, GameAsset_Checkerboard);

		State->CubeMat.Diffuse = GetFirstBitmap(AssetSystem, GameAsset_ContainerDiffImage);
		State->CubeMat.Specular = GetFirstBitmap(AssetSystem, GameAsset_ContainerSpecImage);

		for (int X = 0; X < LPTER_CHUNKS_SIDE_COUNT; X++) {
			for (int Y = 0; Y < LPTER_CHUNKS_SIDE_COUNT; Y++) {
				int CurrentIndex = X * LPTER_CHUNKS_SIDE_COUNT + Y;
				LpterGenerateTerrain(&State->Terrain[CurrentIndex], X, Y);
				LpterGenerateWater(&State->Water[CurrentIndex], X, Y, LPTER_WATER_LEVEL);
			}
		}
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
	v3 RawMoveVector = V3(0.0f, 0.0f, 0.0f);
	if (ButtonIsDown(Input, KeyType_A)) {
		RawMoveVector += V3(1.0f, 0.0f, 0.0f);
	}
	if (ButtonIsDown(Input, KeyType_D)) {
		RawMoveVector -= V3(1.0f, 0.0f, 0.0f);
	}
	if (ButtonIsDown(Input, KeyType_W)) {
		RawMoveVector += V3(0.0f, 0.0f, 1.0f);
	}
	if (ButtonIsDown(Input, KeyType_S)) {
		RawMoveVector -= V3(0.0f, 0.0f, 1.0f);
	}

	RawMoveVector = NOZ(RawMoveVector);

	float CameraSpeed = 20.0f;
	if (ButtonIsDown(Input, KeyType_LShift)) {
		CameraSpeed *= 12.0f;
	}
	if (ButtonIsDown(Input, KeyType_Space)) {
		CameraSpeed *= 7.0f;

		if (ButtonIsDown(Input, KeyType_E)) {
			CameraSpeed *= 5.0f;
		}
	}
	if (ButtonWentDown(Input, KeyType_Q)) {
		State->CameraAutoMove = !State->CameraAutoMove;
	}

	RawMoveVector = RawMoveVector * CameraSpeed;
#if 0
	RawMoveVector *= Input->DeltaTime;
	State->Camera.Position += State->Camera.Front * RawMoveVector.z;
	State->Camera.Position -= State->Camera.Left * RawMoveVector.x;
	State->Camera.Position += State->Camera.Up * RawMoveVector.y;
#else

	v3 MoveVector = {};
	MoveVector += State->Camera.Front * RawMoveVector.z;
	MoveVector += (-State->Camera.Left * RawMoveVector.x);
	MoveVector += State->Camera.Up * RawMoveVector.y;

	if (State->CameraAutoMove) {
		MoveVector = NOZ(V3(1.0f, 0.0f, 1.0f)) * 500.0f;
	}

	State->Camera.Position +=
		State->Camera.dPosition * Input->DeltaTime * 1.2f +
		MoveVector * Input->DeltaTime * Input->DeltaTime * 0.5f;

	State->Camera.dPosition = (State->Camera.dPosition + MoveVector * Input->DeltaTime) * (1.0f - Input->DeltaTime * 5.0f);

#endif

	game_camera_setup CameraSetup = GAMECameraSetup(
		State->Camera,
		RenderStack->RenderWidth,
		RenderStack->RenderHeight,
		CameraProjection_Perspective,
		2000.0f);

	RENDERSetCameraSetup(RenderStack, CameraSetup);

	if (ButtonWentDown(Input, KeyType_Backquote)) {
		State->CapturingMouse = !State->CapturingMouse;
	}

	if (ButtonIsDown(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 0;
	}
	if (ButtonWentUp(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 1;
	}

	mesh_id CubeID = GetFirstMesh(AssetSystem, GameAsset_Cube);
	mesh_id PlaneID = GetFirstMesh(AssetSystem, GameAsset_Plane);
	mesh_id CylID = GetFirstMesh(AssetSystem, GameAsset_Cylynder);

	mesh_id SphereID = GetAssetByBestFloatTag(AssetSystem, GameAsset_Sphere, GameAssetTag_LOD, 0.0f, AssetType_Mesh);

#if 0
	for (int i = 0; i < ArrayCount(State->Terrain); i++) {
		v3 TerrainOffset = LpterGetTerrainOffset(&State->Terrain[i]);
		RENDERPushLpterMesh(RenderStack, &State->Terrain[i].Mesh, TerrainOffset);
		RENDERPushLpterWaterMesh(RenderStack, &State->Water[i], TerrainOffset);
	}
#endif

#if 0
	v4 FrustumPlanes[6];

	mat4 TempProjection = PerspectiveProjection(
		RenderStack->RenderWidth,
		RenderStack->RenderHeight,
		45.0f,
		1000.0f,
		0.1f);

	mat4 PVM = TempProjection;
	PVM = Transpose(PVM);

	//NOTE(dima): Left plane
	FrustumPlanes[0].A = PVM.E[3] + PVM.E[0];
	FrustumPlanes[0].B = PVM.E[7] + PVM.E[4];
	FrustumPlanes[0].C = PVM.E[11] + PVM.E[8];
	FrustumPlanes[0].D = PVM.E[15] + PVM.E[12];

	//NOTE(dima): Right plane
	FrustumPlanes[1].A = PVM.E[3] - PVM.E[0];
	FrustumPlanes[1].B = PVM.E[7] - PVM.E[4];
	FrustumPlanes[1].C = PVM.E[11] - PVM.E[8];
	FrustumPlanes[1].D = PVM.E[15] - PVM.E[12];

	//NOTE(dima): Bottom plane
	FrustumPlanes[2].A = PVM.E[3] + PVM.E[1];
	FrustumPlanes[2].B = PVM.E[7] + PVM.E[5];
	FrustumPlanes[2].C = PVM.E[11] + PVM.E[9];
	FrustumPlanes[2].D = PVM.E[15] + PVM.E[13];

	//NOTE(dima): Top plane
	FrustumPlanes[3].A = PVM.E[3] - PVM.E[1];
	FrustumPlanes[3].B = PVM.E[7] - PVM.E[5];
	FrustumPlanes[3].C = PVM.E[11] - PVM.E[9];
	FrustumPlanes[3].D = PVM.E[15] - PVM.E[13];

	//NOTE(dima): Near plane
	FrustumPlanes[4].A = PVM.E[3] + PVM.E[2];
	FrustumPlanes[4].B = PVM.E[7] + PVM.E[6];
	FrustumPlanes[4].C = PVM.E[11] + PVM.E[10];
	FrustumPlanes[4].D = PVM.E[15] + PVM.E[14];

	//NOTE(dima): Far plane
	FrustumPlanes[5].A = PVM.E[3] - PVM.E[2];
	FrustumPlanes[5].B = PVM.E[7] - PVM.E[6];
	FrustumPlanes[5].C = PVM.E[11] - PVM.E[10];
	FrustumPlanes[5].D = PVM.E[15] - PVM.E[14];

	for (int PlaneIndex = 0;
		PlaneIndex < 6;
		PlaneIndex++)
	{
		FrustumPlanes[PlaneIndex] = NormalizePlane(FrustumPlanes[PlaneIndex]);
	}

	for (int i = -10; i < 10; i++) {
		for (int j = -10; j < 10; j++) {
			for (int k = -10; k < 10; k++) {

				if (i != 0 && j != 0 && k != 0) {
					v3 CubePos = V3(i * 5, j * 5, k * 5);

					int CullTestRes = 1;
					for (int PlaneIndex = 0;
						PlaneIndex < 6;
						PlaneIndex++)
					{
						CullTestRes &= (PlanePointTest(FrustumPlanes[PlaneIndex], CubePos) > 0.0f);
					}

					if (CullTestRes > 0) {
						mat4 Transform = TranslationMatrix(CubePos);

						RENDERPushMesh(RenderStack, CubeID, Transform, &State->CubeMat);
					}
				}
			}
		}
	}
#endif
	
	v3 SpherePos1 = V3(10.0f * Sin(Input->Time), 15.0f, 5.0f * Cos(Input->Time));
	mat4 SphereMat1 = TranslationMatrix(SpherePos1) * ScalingMatrix(V3(5.0f, 5.0f, 5.0f));

	v3 SpherePos2 = V3(5.0f * Sin(Input->Time * 0.5f), 3.0f, 5.0f * Cos(Input->Time * 0.5f));
	mat4 SphereMat2 = TranslationMatrix(SpherePos2) * ScalingMatrix(V3(3.0f, 3.0f, 3.0f));

	v3 CylPos1 = V3(1.0f, 6.0f, 20.0f);
	
	static mat4 CylTranMat = TranslationMatrix(CylPos1);
	mat4 CylMat1 = 
		CylTranMat * 
		RotationX(Input->Time) *
		ScalingMatrix(V3(2.0f, 10.0f, 2.0f));
	CylTranMat = Translate(CylTranMat, V3(1.0f, 0.0f, 1.0f) * Input->DeltaTime * 3);
	

	v3 CubePos = V3(-5.0f, 2.0f, 3.0f);
	mat4 CubeMat = TranslationMatrix(CubePos) * RotationX(Input->Time) * RotationY(Input->Time) *  ScalingMatrix(V3(2.0f, 2.0f, 2.0f));

	RENDERPushMesh(RenderStack, SphereID, SphereMat1, State->CubeMat);
	//RENDERPushMesh(RenderStack, SphereID, SphereMat2, &State->CubeMat);
	RENDERPushMesh(RenderStack, CubeID, CubeMat, State->CubeMat);

	RENDERPushMesh(RenderStack, CylID, CylMat1, State->CubeMat);

	RENDERPushMesh(RenderStack, PlaneID, ScalingMatrix(V3(100, 100, 100)), State->PlaneMat);
}