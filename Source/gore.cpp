#include "gore.h"

void GorePushRectEntity(
	render_stack* Stack, 
	bitmap_info* Bitmap, 
	float BitmapHeight, 
	v2 P, v2 Dim, 
	v2 AlignFromTopLeft, 
	b32 FacingLeft,
	v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	if (Bitmap) {
		Dim = V2(Bitmap->WidthOverHeight * BitmapHeight, BitmapHeight);
	}

	v2 TargetRectMin = V2(
		P.x - AlignFromTopLeft.x * Dim.x,
		P.y - (1.0f - AlignFromTopLeft.y) * Dim.y);

	rect2 ResultEntityRect = Rect2MinDim(TargetRectMin, Dim);

	RENDERPushRectSprite(Stack, Bitmap, ResultEntityRect, FacingLeft, ModulationColor);
}

v2 UnprojectScreenToWorldspace(gore_state* GoreState, v2 ScreenP) {
	/*
		NOTE(dima): Old code that was rewritten
		The reason is that PixelsToXUnits and PixelsToYUnits are the same

		float XUnitsPerScreen = GoreState->ScreenWidth * GoreState->CurrentCameraScaling * 2.0f / GoreState->ScreenHeight;
		float YUnitsPerScreen = GoreState->CurrentCameraScaling * 2.0f;

		float PixelsToXUnits = XUnitsPerScreen / GoreState->ScreenWidth;
		float PixelsToYUnits = YUnitsPerScreen / GoreState->ScreenHeight;
	*/

	float PixelsToUnits = GoreState->CurrentCameraScaling * 2.0f * GoreState->OneOverScreenHeight;
	
	v2 Result = ScreenP * PixelsToUnits;

	return(Result);
}

//v2 ProjectWorldScpaceToScreen(gore_state* GoreState, v2 WorldspacePoint) {
//
//}

void UpdateGore(game_mode_state* GameModeState, engine_systems* EngineSystems) {
	gore_state* GoreState = (gore_state*)GameModeState->GameModeMemory.BaseAddress;

	if (!GoreState->IsInitialized) {

		PushStruct(&GameModeState->GameModeMemory, gore_state);
		GoreState->GameModeMemory = &GameModeState->GameModeMemory;

		//NOTE(dima): Here i precalculate some values
		GoreState->ScreenWidth = EngineSystems->RenderState->RenderWidth;
		GoreState->ScreenHeight = EngineSystems->RenderState->RenderHeight;
		GoreState->OneOverScreenWidth = 1.0f / GoreState->ScreenWidth;
		GoreState->OneOverScreenHeight = 1.0f / GoreState->ScreenHeight;

		GoreState->CurrentCameraScaling = 6.0f;
		GoreState->Camera = GAMECreateCamera();
		GoreState->CameraSetup = {};

		GoreState->PixelsPerMeter = 100.0f;

		GoreState->PlayerBitmapID = GetFirstBitmap(EngineSystems->AssetSystem, GameAsset_Lilboy);
		
		GoreState->PlayerP = {};
		GoreState->PlayerVelocity = {};
		GoreState->PlayerDim = V2(0.7f, 3.0f / 2.0f * 0.7f);
		GoreState->PlayerVelocity = V2(0.0f, 0.0f);
		GoreState->PlayerFacingLeft = 1;
		GoreState->PlayerHealth = 0.75f;
		GoreState->PlayerMaxHealth = 1.0f;

		float TimeForHalfJump = 0.35f;
		float JumpHeight = 4.0f;
		GoreState->Gravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		GoreState->InitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);

		GoreState->IsInitialized = 1;
	}

	v3 InitCameraOffset = V3(0.0f, 1.5f, 5.0f);

	GoreState->Camera.Position = Lerp(
		GoreState->Camera.Position,
		V3(GoreState->PlayerP.x, GoreState->PlayerP.y, 0.0f) + InitCameraOffset,
		EngineSystems->InputSystem->DeltaTime * 3.0f);


	float dt = EngineSystems->InputSystem->DeltaTime;

	float Speed = 10.0f;
	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_A) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Left)) 
	{
		GoreState->PlayerP.x += Speed * dt;
		GoreState->PlayerFacingLeft = 1;
	}

	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_D) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Right)) 
	{
		GoreState->PlayerP.x -= Speed * dt;
		GoreState->PlayerFacingLeft = 0;
	}

	GoreState->PlayerP += GoreState->PlayerVelocity * dt + GoreState->Gravity * dt * dt * 0.5f;

	GoreState->PlayerVelocity += GoreState->Gravity * dt;

	if (GoreState->PlayerP.y < 0.0f) 
	{
		GoreState->PlayerP.y = 0.0f;
		GoreState->PlayerVelocity.y = 0.0f;
	}

	if (ButtonWentDown(EngineSystems->InputSystem, KeyType_Space) || 
		ButtonWentDown(EngineSystems->InputSystem, KeyType_Up)) 
	{
		GoreState->PlayerVelocity = GoreState->InitJumpVelocity;
	}

	GAMEUpdateCameraVectorsBasedOnUpAndFront(
		&GoreState->Camera, 
		V3(0.0f, 0.0f, -1.0f), 
		V3(0.0f, 1.0f, 0.0f));

	GoreState->CameraSetup = GAMECameraSetup(
		GoreState->Camera,
		EngineSystems->RenderState->RenderWidth,
		EngineSystems->RenderState->RenderHeight,
		CameraProjection_GoreCustom,
		0.1f, 100.0f, 45.0f, 
		GoreState->CurrentCameraScaling);

	render_stack* RenderStack = EngineSystems->RenderState->NamedStacks.Main;

	RENDERSetCameraSetup(RenderStack, &GoreState->CameraSetup);

	//RENDERPushClear(RenderStack, V3(1.0f, 1.0f, 1.0f));

	bitmap_info* PlayerBitmap = GetBitmapFromID(EngineSystems->AssetSystem, GoreState->PlayerBitmapID);
	//bitmap_info* PlayerBitmap = 0;

	//NOTE(dima): Pushing player bitmap
#if 1
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP,
		GoreState->PlayerDim,
		V2(0.5f, 1.0f),
		GoreState->PlayerFacingLeft);
#endif

	GorePushRectEntity(
		RenderStack, 
		PlayerBitmap,
		GoreState->PlayerDim.y * 2.2f,
		GoreState->PlayerP, 
		GoreState->PlayerDim, 
		V2(0.5f, 1.0f), 
		GoreState->PlayerFacingLeft);

	float HealthPercentage = GoreState->PlayerHealth / GoreState->PlayerMaxHealth;
	HealthPercentage = Clamp01(HealthPercentage);

	v2 HealthHUDOffset = V2(0.0f, 1.5f);
	v2 MaxHealthHUDDim = V2(1.0f, 0.2f);
	v2 HealthHUDDim = V2(MaxHealthHUDDim.x * HealthPercentage, MaxHealthHUDDim.y);

	v4 HealthColor = Lerp(V4(1.0f, 0.1f, 0.1f, 1.0f), V4(0.1f, 1.0f, 0.1f, 1.0f), HealthPercentage);

	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset,
		MaxHealthHUDDim,
		V2(0.5f, 0.5f),
		1,
		V4(0.15f, 0.15f, 0.15f, 1.0f));

	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset,
		HealthHUDDim,
		V2(0.5f, 0.5f),
		1,
		HealthColor);

	float HealthBorderDim = 0.02f;
	v4 HealthBorderColor = V4(0.0f, 0.0f, 0.0f, 1.0f);

	//NOTE(dima): Top
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset + V2(0.0f, MaxHealthHUDDim.y * 0.5f),
		V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
		V2(0.5f, 1.0f),
		1,
		HealthBorderColor);

	//NOTE(dima): Bottom
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset + V2(0.0f, -MaxHealthHUDDim.y * 0.5f),
		V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
		V2(0.5f, 0.0f),
		1,
		HealthBorderColor);

	//NOTE(dima): Left
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset + V2(MaxHealthHUDDim.x * 0.5f, 0.0f),
		V2(HealthBorderDim, MaxHealthHUDDim.y),
		V2(0.0f, 0.5f),
		1,
		HealthBorderColor);


	//NOTE(dima): Right
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP + HealthHUDOffset + V2(-MaxHealthHUDDim.x * 0.5f, 0.0f),
		V2(HealthBorderDim, MaxHealthHUDDim.y),
		V2(1.0f, 0.5f),
		1,
		HealthBorderColor);

	GoreState->PlayerHealth = Sin(EngineSystems->InputSystem->Time) * 0.5f + 0.5f;

	//NOTE(dima): Center anchor
	GorePushRectEntity(
		RenderStack,
		0, 0.0f,
		GoreState->PlayerP,
		V2(0.1f, 0.1f),
		V2(0.5f, 0.5f),
		1);

	//NOTE(dima): Floor
	for (int i = -5; i <= 5; i++) {
		GorePushRectEntity(
			RenderStack,
			0,
			0.0f,
			V2((float)i, 0.0f),
			V2(1.0f, 1.0f),
			V2(0.5f, 0.0f),
			GoreState->PlayerFacingLeft,
			GetColor(EngineSystems->ColorsState, Color_Red + i + 5));
	}
}