#include "geometrika.h"

inline int GetCelluralMachineCellsCount(cellural_machine* Machine) {
	int Result = Machine->CellsXCount * Machine->CellsYCount;

	return(Result);
}

void InitCelluralMachine(
	cellural_machine* Machine,
	stacked_memory* MemoryStack,
	int RenderPixelWidth,
	int RenderPixelHeight)
{
	//NOTE(dima): Reserving pixels for borders 
	int BorderWidth = CELLURAL_CELL_PIXEL_WIDTH;

	int NonBorderPixelWidth = (RenderPixelWidth - 2 * BorderWidth);
	int NonBorderPixelHeight = (RenderPixelHeight - 2 * BorderWidth);

	Machine->CellsXCount = NonBorderPixelWidth / CELLURAL_CELL_PIXEL_WIDTH;
	Machine->CellsYCount = NonBorderPixelHeight / CELLURAL_CELL_PIXEL_WIDTH;
	int CellsCount = Machine->CellsXCount * Machine->CellsYCount;

	int AdditionalXOffset = (NonBorderPixelWidth % CELLURAL_CELL_PIXEL_WIDTH) / 2;
	int AdditionalYOffset = (NonBorderPixelHeight % CELLURAL_CELL_PIXEL_WIDTH) / 2;

	//NOTE(dima): Counting starting offsets for borders 
	Machine->StartOffsetX = BorderWidth + AdditionalXOffset;
	Machine->StartOffsetY = BorderWidth + AdditionalYOffset;

	//NOTE(dima): Initializing bitmap 
	Machine->Bitmap.Width = RenderPixelWidth;
	Machine->Bitmap.Height = RenderPixelHeight;
	//NOTE(dima): +4 here will allocate few more pixels than we actually need for SIMD
	Machine->Bitmap.Pixels = PushArray(MemoryStack, u8, (Machine->Bitmap.Width * Machine->Bitmap.Height + 4) * 4); 
	Machine->Bitmap.Pitch = Machine->Bitmap.Width * 4;
	Machine->Bitmap.WidthOverHeight = (float)Machine->Bitmap.Width / (float)Machine->Bitmap.Height;
	Machine->Bitmap.TextureHandle = 0;

	//NOTE(dima): Allocating needed arrays 
	Machine->Colors = PushArray(MemoryStack, u16, CellsCount);
	Machine->Alphas = PushArray(MemoryStack, u8, CellsCount);
	Machine->Types = PushArray(MemoryStack, u8, CellsCount);
	Machine->LifeStart = PushArray(MemoryStack, float, CellsCount);
	Machine->LifeLenSeconds = PushArray(MemoryStack, u8, CellsCount);

	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex++)
	{
		Machine->Colors[CellIndex] = 0;
		Machine->Alphas[CellIndex] = 1.0f;
		Machine->Types[CellIndex] = 0;
	}

	Machine->TimeCounterForSpawning = 0.0f;
}

void UpdateCelluralMachine(
	cellural_machine* Machine, 
	render_stack* RenderState, 
	input_system* Input,
	random_state* Random) 
{

	u16 RandomColors[] = {
		PackRGB16(V3(1.0f, 0.0f, 0.0f)),
		PackRGB16(V3(0.0f, 1.0f, 0.0f)),
		PackRGB16(V3(0.0f, 0.0f, 1.0f)),
		PackRGB16(V3(1.0f, 1.0f, 0.0f)),
		PackRGB16(V3(1.0f, 0.0f, 1.0f)),
		PackRGB16(V3(0.0f, 1.0f, 1.0f)),
		PackRGB16(V3(1.0f, 1.0f, 1.0f)),
	};

	int CellsCount = GetCelluralMachineCellsCount(Machine);

	//NOTE(dima): Spawning cells
#if 0
	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex++)
	{
		int CellX = CellIndex % Machine->CellsXCount;
		int CellY = CellIndex / Machine->CellsXCount;

		Machine->Colors[CellIndex] = RandomColors[ColorIndex % ArrayCount(RandomColors)];
		Machine->Alphas[CellIndex] = 255;
		Machine->Types[CellIndex] = MachineCell_Solid;

		ColorIndex++;
	}
#else
	float TimeToSpawn = 0.03f;
	if (Machine->TimeCounterForSpawning > TimeToSpawn) {

		for (int Index = 0; Index < 3; Index++) {
			u32 NextRandomInt = GetNextRandomInt(Random);

			int XIndex = (NextRandomInt & 0xFFFF) % Machine->CellsXCount;
			int YIndex = ((NextRandomInt >> 16) & 0xFFFF) % Machine->CellsYCount;

			int CellIndex = YIndex * Machine->CellsXCount + XIndex;

			Machine->Colors[CellIndex] = RandomColors[Machine->ColorIncIndex % ArrayCount(RandomColors)];
			Machine->Alphas[CellIndex] = 255;
			Machine->Types[CellIndex] = MachineCell_Fadeout;

			Machine->LifeStart[CellIndex] = Input->Time;
			Machine->LifeLenSeconds[CellIndex] = 4 + NextRandomInt & 7;

			Machine->ColorIncIndex++;
		}

		//Machine->TimeCounterForSpawning -= TimeToSpawn;
		Machine->TimeCounterForSpawning = 0.0f;
	}

	Machine->TimeCounterForSpawning += Input->DeltaTime;
#endif

	//NOTE(dima): Updating cells
	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex++)
	{
		switch (Machine->Types[CellIndex]) {
			case MachineCell_None: {

			}break;

			case MachineCell_Solid: {

			}break;

			case MachineCell_Fadeout: {
				float LifeStart = Machine->LifeStart[CellIndex];
				float LifeEnd = LifeStart + (float)Machine->LifeLenSeconds[CellIndex];

				float t = (Input->Time - LifeStart) / (LifeEnd - LifeStart);

				t = 1.0f - Clamp01(t);

				Machine->Alphas[CellIndex] = From01To255(t);
			}break;
		}
	}


	/*
		NOTE(dima): Forming cellural machine bitmap.

		IMPORTANT(dima): Remember that renderer 
		accepts premultiplied alpha images.
	*/

	v4 ClearColor = V4(0.05f, 0.05f, 0.05f, 1.0f);
#if 1
	//NOTE(dima): Clearing bitmap
	u32 ClearColorPacked = PackRGBA(ClearColor);
	u32* Pixel = (u32*)Machine->Bitmap.Pixels;
	for (int PixelIndex = 0;
		PixelIndex < Machine->Bitmap.Width * Machine->Bitmap.Height;
		PixelIndex++)
	{
		*Pixel++ = ClearColorPacked;
	}

	__m128i mm31 = _mm_set1_epi32(31);
	__m128i mm63 = _mm_set1_epi32(63);

	__m128 mmOneOver31 = _mm_div_ps(_mm_set1_ps(1.0f), _mm_cvtepi32_ps(mm31));
	__m128 mmOneOver63 = _mm_div_ps(_mm_set1_ps(1.0f), _mm_cvtepi32_ps(mm63));

	__m128 mmClearColor_r = _mm_set1_ps(ClearColor.r);
	__m128 mmClearColor_g = _mm_set1_ps(ClearColor.g);
	__m128 mmClearColor_b = _mm_set1_ps(ClearColor.b);
	__m128 mmClearColor_a = _mm_set1_ps(ClearColor.a);

	//NOTE(dima): Rendering cells
	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex += 4)
	{
		__m128i mmCellsXCount = _mm_set1_epi32(Machine->CellsXCount);

		__m128i mmCellIndex = _mm_setr_epi32(
			CellIndex,
			CellIndex + 1,
			CellIndex + 2,
			CellIndex + 3);

		__m128i mmCellY = _mm_cvttps_epi32(_mm_div_ps(_mm_cvtepi32_ps(mmCellIndex), _mm_cvtepi32_ps(mmCellsXCount)));

		__m128 mmYByWidth = _mm_mul_ps(_mm_cvtepi32_ps(mmCellY), _mm_cvtepi32_ps(mmCellsXCount));
		__m128i mmCellX = _mm_cvtps_epi32(_mm_sub_ps(_mm_cvtepi32_ps(mmCellIndex), mmYByWidth));

		//NOTE(dima): Fetching colors from color table
		__m128i mmColors = _mm_setr_epi32(
			Machine->Colors[CellIndex],
			Machine->Colors[CellIndex + 1],
			Machine->Colors[CellIndex + 2],
			Machine->Colors[CellIndex + 3]);

		//NOTE(dima): Color unpacking from RGB16
		__m128 mmCellColorUnpacked_r = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(mmColors, mm31)), mmOneOver31);
		__m128 mmCellColorUnpacked_g = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(mmColors, mm63)), mmOneOver63);
		__m128 mmCellColorUnpacked_b = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(mmColors, mm31)), mmOneOver31);
		__m128 mmCollColorUnpacked_a = _mm_set1_ps(1.0f);

		//NOTE(dima): Fetching alphas values
		__m128 mmAlphas = _mm_setr_ps(
			Machine->Alphas[CellIndex],
			Machine->Alphas[CellIndex + 1],
			Machine->Alphas[CellIndex + 2],
			Machine->Alphas[CellIndex + 3]);
	}

	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex++)
	{
		int CellX = CellIndex % Machine->CellsXCount;
		int CellY = CellIndex / Machine->CellsXCount;

		v3 UnpackedRGB16 = UnpackRGB16(Machine->Colors[CellIndex]);
		v4 CellColor = V4(UnpackedRGB16, 1.0f);

		float CellAlpha = From255To01(Machine->Alphas[CellIndex]);
		v4 BlendedColor = Lerp(ClearColor, CellColor, CellAlpha);

		u32 BlendedColorPacked = PackRGBA(BlendedColor);

		for (int YPixel = Machine->StartOffsetY;
			YPixel < Machine->StartOffsetY + CELLURAL_CELL_PIXEL_WIDTH;
			YPixel++)
		{
			for (int XPixel = Machine->StartOffsetX;
				XPixel < Machine->StartOffsetX + CELLURAL_CELL_PIXEL_WIDTH;
				XPixel++)
			{
				u32* TargetPixel = (u32*)Machine->Bitmap.Pixels + 
					Machine->Bitmap.Width * (YPixel + CellY * CELLURAL_CELL_PIXEL_WIDTH) +
						XPixel + CellX * CELLURAL_CELL_PIXEL_WIDTH;

				//NOTE(dima): Alpha blend
				*TargetPixel = BlendedColorPacked;
			}
		}
	}

	//NOTE(dima): Deallocating bitmap entry to reallocate it in the renderer
	dealloc_queue_entry* DeallocEntry = PlatformRequestDeallocEntry();
	DeallocEntry->EntryType = DeallocQueueEntry_Bitmap;
	DeallocEntry->Data.BitmapData.TextureHandle = Machine->Bitmap.TextureHandle;;
	PlatformInsertDellocEntry(DeallocEntry);

	Machine->Bitmap.TextureHandle = 0;

	//TODO(dima): Sending bitmap to the renderer
	RENDERPushBitmap(RenderState, &Machine->Bitmap, V2(0.0f, 0.0f), Machine->Bitmap.Height);
#else
	RENDERPushClear(RenderState, ClearColor.rgb);

	//NOTE(dima): Rendering cells
	for (int CellIndex = 0;
		CellIndex < CellsCount;
		CellIndex++)
	{
		int CellX = CellIndex % Machine->CellsXCount;
		int CellY = CellIndex / Machine->CellsXCount;

		v3 UnpackedRGB16 = UnpackRGB16(Machine->Colors[CellIndex]);
		v4 CellColor = V4(UnpackedRGB16, From255To01(Machine->Alphas[CellIndex]));

		v2 Min = V2(
			Machine->StartOffsetX + CellX * CELLURAL_CELL_PIXEL_WIDTH,
			Machine->StartOffsetY + CellY * CELLURAL_CELL_PIXEL_WIDTH);

		v2 Dim = V2(CELLURAL_CELL_PIXEL_WIDTH, CELLURAL_CELL_PIXEL_WIDTH);

		RENDERPushRect(RenderState, Min, Dim, CellColor);
	}

#endif
}

void GEOMKAUpdateAndRender(stacked_memory* GameMemoryBlock, engine_systems* EngineSysState) {
	geometrika_state* State = (geometrika_state*)GameMemoryBlock->BaseAddress;

	asset_system* AssetSystem = EngineSysState->AssetSystem;
	input_system* Input = EngineSysState->InputSystem;
	render_state* RenderState = EngineSysState->RenderState;

	render_stack* RenderStack = RenderState->NamedStacks.Main;

	if (!State->IsInitialized) {

		PushStruct(GameMemoryBlock, geometrika_state);

		State->Random = InitRandomStateWithSeed(1111);

		State->Camera = GAMECreateCamera();
		State->CapturingMouse = 1;
		State->CameraAutoMove = 0;

		State->CubeMat = LITCreateSurfaceMaterial(32.0f, V3(0.9f, 0.1f, 0.1f));
		State->PlaneMat = LITCreateSurfaceMaterial(16.0f, V3(0.1f, 0.1f, 0.9f));

		State->PlaneMat.Diffuse = GetFirstBitmap(AssetSystem, GameAsset_Checkerboard);

		State->CubeMat.Diffuse = GetFirstBitmap(AssetSystem, GameAsset_ContainerDiffImage);
		State->CubeMat.Specular = GetFirstBitmap(AssetSystem, GameAsset_ContainerSpecImage);

		State->CelluralMachineMemory = SplitStackedMemory(GameMemoryBlock, MEGABYTES(20));
		InitCelluralMachine(
			&State->CelluralMachine,
			&State->CelluralMachineMemory,
			RenderState->RenderWidth,
			RenderState->RenderHeight);

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

	if (ButtonWentDown(Input, KeyType_Backquote)) {
		State->CapturingMouse = !State->CapturingMouse;
	}

	if (ButtonIsDown(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 0;
	}
	if (ButtonWentUp(Input, KeyType_LCtrl)) {
		State->CapturingMouse = 1;
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
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		CameraProjection_Perspective,
		2000.0f);

	RENDERSetCameraSetup(RenderStack, CameraSetup);

	mesh_id CubeID = GetFirstMesh(AssetSystem, GameAsset_Cube);
	mesh_id PlaneID = GetFirstMesh(AssetSystem, GameAsset_Plane);
	mesh_id CylID = GetFirstMesh(AssetSystem, GameAsset_Cylynder);

	mesh_id SphereID = GetAssetByBestFloatTag(AssetSystem, GameAsset_Sphere, GameAssetTag_LOD, 0.0f, AssetType_Mesh);

#if 1
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


	UpdateCelluralMachine(&State->CelluralMachine, RenderStack, Input, &State->Random);
	//RENDERPushRect(RenderStack, V2(100, 100), V2(200, 200), V4(1.0f, 0.6f, 1.0f, 1.0f));

}