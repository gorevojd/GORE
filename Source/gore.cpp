#include "gore.h"

inline rect2 GoreGetEntityRect(v2 P, v2 Dim, v2 AlignFromTopLeft) {
	v2 ResultMin = V2(
		P.x - AlignFromTopLeft.x * Dim.x,
		P.y - (1.0f - AlignFromTopLeft.y) * Dim.y);

	rect2 Result = Rect2MinDim(ResultMin, Dim);

	return(Result);
}

void GorePushRectEntity(
	render_stack* Stack,
	v2 At, v2 Dim,
	v2 AlignFromTopLeft,
	b32 FacingLeft,
	v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	rect2 EntityRect = GoreGetEntityRect(At, Dim, AlignFromTopLeft);

	RENDERPushRectSprite(Stack, 0, EntityRect, FacingLeft, ModulationColor);
}

void GorePushRectEntity(
	render_stack* Stack,
	rect2 EntityRect,
	b32 FacingLeft,
	v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	RENDERPushRectSprite(Stack, 0, EntityRect, FacingLeft, ModulationColor);
}

void GorePushBitmapEntity(
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

	rect2 ResultEntityRect = GoreGetEntityRect(P, Dim, AlignFromTopLeft);

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

//NOTE(dima): Now those only just for player :(((
b32 GoreRaycast2DWallsForDimensionalEntity(
	gore_state* GoreState,
	v2 EntityDim, 
	v2 EntityTopLeftAlign,
	gore_ray2d* Ray, 
	gore_raycast_hit* OutHitData) 
{

	b32 AtLeastOneHitHappened = 0;
	gore_raycast_hit* ClosestHit = 0;
	float MinDistanceToHit = 99999999.0f;

	for (int WallIndex = 0; WallIndex < GoreState->WallCount; WallIndex++) {

		gore_wall* Wall = GoreState->Walls + WallIndex;

		rect2 WallRect = GoreGetEntityRect(Wall->At, Wall->Dim, Wall->TopLeftAlign);

		//NOTE(dima): Theese values should be precalculated for static elements
		gore_raycast_hit HitDatas[4];

		rect2 MinkovskiRect = WallRect;

		//NOTE(dima): Minkovski based collision detection
		v2 LeftRightAddition = V2(EntityDim.x * EntityTopLeftAlign.x, EntityDim.x * (1.0f - EntityTopLeftAlign.x));
		v2 TopBottomAddition = V2(EntityDim.y * EntityTopLeftAlign.y, EntityDim.y * (1.0f - EntityTopLeftAlign.y));

		MinkovskiRect.Min.x -= LeftRightAddition.x;
		MinkovskiRect.Max.x += LeftRightAddition.y;
		MinkovskiRect.Min.y -= TopBottomAddition.x;
		MinkovskiRect.Max.y += TopBottomAddition.y;

		gore_edge MinkovskiRectEdges[4];
		MinkovskiRectEdges[0] = GoreEdge(MinkovskiRect.Min, V2(MinkovskiRect.Max.x, MinkovskiRect.Min.y));
		MinkovskiRectEdges[1] = GoreEdge(V2(MinkovskiRect.Max.x, MinkovskiRect.Min.y), MinkovskiRect.Max);
		MinkovskiRectEdges[2] = GoreEdge(MinkovskiRect.Max, V2(MinkovskiRect.Min.x, MinkovskiRect.Max.y));
		MinkovskiRectEdges[3] = GoreEdge(V2(MinkovskiRect.Min.x, MinkovskiRect.Max.y), MinkovskiRect.Min);

		for (int EdgeIndex = 0; EdgeIndex < 4; EdgeIndex++)
		{
			gore_edge* Edge = MinkovskiRectEdges + EdgeIndex;
			gore_raycast_hit* HitData = HitDatas + EdgeIndex;

			HitData->HitHappened = 0;

			v2 EdgeN = V2(Edge->EdgeLineEqation.A, Edge->EdgeLineEqation.B);

			float ON = Dot(Ray->Origin, EdgeN);
			float RN = Dot(Ray->Direction, EdgeN);

			//NOTE(dima): If ray and line normal are not parallel
			if (Abs(RN) > 0.0001f) {

				HitData->DistanseFromRayOrigin = (-Edge->EdgeLineEqation.C - ON) / RN;
				HitData->HitNormal = EdgeN;
				HitData->HitPoint = Ray->Origin + Ray->Direction * HitData->DistanseFromRayOrigin;

				b32 RayHitsTheLine = HitData->DistanseFromRayOrigin * HitData->DistanseFromRayOrigin < Ray->SqLen;
				if (RayHitsTheLine) {

					v2 HitPP1 = Edge->Min - HitData->HitPoint;
					v2 HitPP2 = Edge->Max - HitData->HitPoint;

					//NOTE(dima): If hit point is between edge min and max points
					if (Dot(HitPP1, HitPP2) < 0.0f) {
						HitData->HitHappened = 1;
						AtLeastOneHitHappened = 1;
					}

				}
			}
		}

		for (int HitIndex = 0; HitIndex < 4; HitIndex++) {
			gore_raycast_hit* HitData = HitDatas + HitIndex;

			if (HitData->HitHappened) {
				if (HitData->DistanseFromRayOrigin < MinDistanceToHit) {
					ClosestHit = HitData;
					MinDistanceToHit = HitData->DistanseFromRayOrigin;
				}
			}
		}


	}

	if (AtLeastOneHitHappened && OutHitData) {
		*OutHitData = *ClosestHit;
	}

	return(AtLeastOneHitHappened);
}

void VisualizeRay(render_stack* Stack, gore_ray2d* Ray, v4 Color) {
	int AnchorCount = 10;

	v2 Dim = V2(0.05f, 0.05f);

	for (int  AnchorIndex = 0; AnchorIndex <= AnchorCount; AnchorIndex++)
	{
		float Percentage = AnchorIndex / (float)(AnchorCount + 1);

		v2 TargetP = Ray->Origin + Ray->Direction * Ray->Len * Percentage ;

		GorePushRectEntity(
			Stack,
			GoreGetEntityRect(
				TargetP,
				Dim,
				V2(0.5f, 0.5f)), 1,
			Color);
	}
}

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
		
		GoreState->PlayerP = V2(0.0f, 4.0f);
		GoreState->PlayerVelocity = {};
		GoreState->PlayerDim = V2(0.7f, 3.0f / 2.0f * 0.7f);
		GoreState->PlayerVelocity = V2(0.0f, 0.0f);
		GoreState->PlayerFacingLeft = 1;
		GoreState->PlayerHealth = 0.75f;
		GoreState->PlayerMaxHealth = 1.0f;

		GoreState->WallCount = 1;
		GoreState->Walls = PushArray(GoreState->GameModeMemory, gore_wall, GoreState->WallCount);

		GoreState->Walls[0].At = V2(0.0f, 5.5f);
		GoreState->Walls[0].Dim = V2(3.0f, 3.0f);
		GoreState->Walls[0].TopLeftAlign = V2(0.5f, 0.0f);
		GoreState->Walls[0].IsDynamic = 0;


		float TimeForHalfJump = 0.35f;
		float JumpHeight = 4.0f;
		GoreState->PlayerGravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		GoreState->PlayerInitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);

		GoreState->IsInitialized = 1;
	}

	v3 InitCameraOffset = V3(0.0f, 1.5f, 5.0f);

	GoreState->Camera.Position = Lerp(
		GoreState->Camera.Position,
		V3(GoreState->PlayerP.x, GoreState->PlayerP.y, 0.0f) + InitCameraOffset,
		EngineSystems->InputSystem->DeltaTime * 3.0f);

	//GoreState->Camera.Position = V3(GoreState->PlayerP.x, GoreState->PlayerP.y, 0.0f) + InitCameraOffset;

	float dt = EngineSystems->InputSystem->DeltaTime;

	float Speed = 10.0f;
	float PlayerHorizontalDelta = 0.0f;
	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_A) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Left)) 
	{
		PlayerHorizontalDelta = Speed * dt;
		GoreState->PlayerFacingLeft = 1;
	}

	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_D) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Right)) 
	{
		PlayerHorizontalDelta = -Speed * dt;
		GoreState->PlayerFacingLeft = 0;
	}

	v2 PlayerDeltaP = GoreState->PlayerVelocity * dt + GoreState->PlayerGravity * dt * dt * 0.5f + V2(PlayerHorizontalDelta, 0.0f);

	v2 NextSupposedPlayerPosition = GoreState->PlayerP + PlayerDeltaP;
	v2 NextSupposedHorizontalP = V2(NextSupposedPlayerPosition.x, GoreState->PlayerP.y);
	v2 NextSupposedVerticalP = V2(GoreState->PlayerP.x, NextSupposedPlayerPosition.y);

	//NOTE(dima): Collision detection code
	rect2 PlayerRect = GoreGetEntityRect(GoreState->PlayerP, GoreState->PlayerDim, V2(0.5f, 1.0f));
	rect2 WallRect = GoreGetEntityRect(GoreState->Walls[0].At, GoreState->Walls[0].Dim, GoreState->Walls[0].TopLeftAlign);

	v4 WallColor = V4(0.0f, 1.0f, 0.0f, 1.0f);

	v2 PlayerAlign = V2(0.5f, 1.0f);

	//NOTE(dima): Minkovski based collision detection
	v2 LeftRightAddition = V2(GoreState->PlayerDim.x * PlayerAlign.x, GoreState->PlayerDim.x * (1.0f - PlayerAlign.x));
	v2 TopBottomAddition = V2(GoreState->PlayerDim.y * PlayerAlign.y, GoreState->PlayerDim.y * (1.0f - PlayerAlign.y));

	rect2 MinkovskiRect = WallRect;

	MinkovskiRect.Min.x -= LeftRightAddition.x;
	MinkovskiRect.Max.x += LeftRightAddition.y;
	MinkovskiRect.Min.y -= TopBottomAddition.x;
	MinkovskiRect.Max.y += TopBottomAddition.y;

	float MinkovskiOffsetEpsilon = 0.01f;

	/*
		NOTE(dima): So now we have minkovski rect and 
		the point that we can test it against with.

		We can check point at discrete positions and we
		also can cast a ray and test it against 4 edges
		of rectangle. Maybe i should imlement both
	*/

	gore_ray2d MoveRay = GoreRay2D(GoreState->PlayerP, NextSupposedPlayerPosition);
	gore_raycast_hit MoveRayHit;

	gore_ray2d MoveHorizontalRay = GoreRay2D(GoreState->PlayerP, NextSupposedHorizontalP);
	gore_raycast_hit HorzRayHit;

	gore_ray2d MoveVerticalRay = GoreRay2D(GoreState->PlayerP, NextSupposedVerticalP);
	gore_raycast_hit VertRayHit;

	b32 MoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
		GoreState,
		GoreState->PlayerDim, PlayerAlign,
		&MoveRay, &MoveRayHit);

	b32 HorzMoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
		GoreState,
		GoreState->PlayerDim, PlayerAlign,
		&MoveHorizontalRay, &HorzRayHit);

	b32 VertMoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
		GoreState,
		GoreState->PlayerDim, PlayerAlign,
		&MoveVerticalRay, &VertRayHit);

#if 1
	if (MoveHitHappened || HorzMoveHitHappened || VertMoveHitHappened) 
	{
		v2 PlayerTargetP;

		v2 HitWithOffset = MoveRayHit.HitPoint + MoveRayHit.HitNormal * MinkovskiOffsetEpsilon * 10;


#if 0
		if (HorzMoveHitHappened) {
			PlayerTargetP = V2(HitWithOffset.x, NextSupposedPlayerPosition.y);

			GoreState->PlayerVelocity.x = 0.0f;
		}
		else if(VertMoveHitHappened){
			PlayerTargetP = V2(NextSupposedPlayerPosition.x, HitWithOffset.y);

			GoreState->PlayerVelocity.y = 0.0f;
		}
		else {
			PlayerTargetP = HitWithOffset;

			GoreState->PlayerVelocity = V2(0.0f, 0.0f);
		}
#else
		if (VertMoveHitHappened) {
			PlayerTargetP = V2(NextSupposedPlayerPosition.x, HitWithOffset.y);

			GoreState->PlayerVelocity.y = 0.0f;
		}
		else {
			PlayerTargetP = HitWithOffset;
			GoreState->PlayerVelocity = V2(0.0f, 0.0f);
		}

#endif

		PlayerDeltaP = PlayerTargetP - GoreState->PlayerP;

		//WallColor = V4(1.0f, 0.0f, 0.0f, 1.0f);
	}
#endif

#if 0
	rect2 MinkovskiInflatedRect = Rect2MinMax(
		MinkovskiRect.Min - V2(MinkovskiOffsetEpsilon, MinkovskiOffsetEpsilon),
		MinkovskiRect.Max + V2(MinkovskiOffsetEpsilon, MinkovskiOffsetEpsilon));

	b32 MinkovskiTest = PointIsInRectangle(NextSupposedPlayerPosition, MinkovskiRect);
	if (MinkovskiTest) {

		b32 MinkovskiHorizontalTest = PointIsInRectangle(NextSupposedHorizontalP, MinkovskiRect);
		b32 MinkovskiVerticalTest = PointIsInRectangle(NextSupposedVerticalP, MinkovskiRect);

		v2 PlayerTargetP;
		if (MinkovskiHorizontalTest) {
			PlayerTargetP.x = Clamp(
				GoreState->PlayerP.x, MinkovskiInflatedRect.Min.x, MinkovskiInflatedRect.Max.x);
			PlayerTargetP.y = NextSupposedPlayerPosition.y;

			GoreState->PlayerVelocity.x = 0.0f;
		}
		else if (MinkovskiVerticalTest){
			PlayerTargetP.x = NextSupposedPlayerPosition.x;
			PlayerTargetP.y = Clamp(
				GoreState->PlayerP.y, MinkovskiInflatedRect.Min.y, MinkovskiInflatedRect.Max.y);

			GoreState->PlayerVelocity.y = 0.0f;
		}
		else {
			PlayerTargetP.x = Clamp(
				GoreState->PlayerP.x, MinkovskiInflatedRect.Min.x, MinkovskiInflatedRect.Max.x);
			PlayerTargetP.y = Clamp(
				GoreState->PlayerP.y, MinkovskiInflatedRect.Min.y, MinkovskiInflatedRect.Max.y);

			GoreState->PlayerVelocity = V2(0.0f, 0.0f);
		}

		PlayerDeltaP = PlayerTargetP - GoreState->PlayerP;

		WallColor = V4(1.0f, 0.0f, 0.0f, 1.0f);
	}
#endif

	GoreState->PlayerP += PlayerDeltaP;
	GoreState->PlayerVelocity += GoreState->PlayerGravity * dt;

#if 1
	if (GoreState->PlayerP.y < 0.0f) 
	{
		GoreState->PlayerP.y = 0.0f;
		GoreState->PlayerVelocity.y = 0.0f;
	}
#endif

	if (ButtonWentDown(EngineSystems->InputSystem, KeyType_Space) || 
		ButtonWentDown(EngineSystems->InputSystem, KeyType_Up)) 
	{
		GoreState->PlayerVelocity = GoreState->PlayerInitJumpVelocity;
	}

	//NOTE(dima): Camera setup code
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
		GoreState->PlayerP,
		GoreState->PlayerDim,
		V2(0.5f, 1.0f),
		GoreState->PlayerFacingLeft);
#endif

	GorePushBitmapEntity(
		RenderStack, 
		PlayerBitmap,
		GoreState->PlayerDim.y * 2.2f,
		GoreState->PlayerP, 
		GoreState->PlayerDim, 
		V2(0.5f, 1.0f), 
		GoreState->PlayerFacingLeft);

	//Wall
#if 0
	GorePushRectEntity(RenderStack, MinkovskiInflatedRect, 1, GetColor(EngineSystems->ColorsState, Color_PrettyBlue));
	GorePushRectEntity(RenderStack, MinkovskiRect, 1, GetColor(EngineSystems->ColorsState, Color_Purple));
	//NOTE(dima): 
	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(V2(MinkovskiInflatedRect.Min.x, MinkovskiInflatedRect.Max.y),
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.4f, 0.0f, 1.0f));

	//NOTE(dima): 
	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(MinkovskiInflatedRect.Max,
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.4f, 0.0f, 1.0f));
#endif

	GorePushRectEntity(RenderStack, WallRect, 1, WallColor);

	float HealthPercentage = GoreState->PlayerHealth / GoreState->PlayerMaxHealth;
	HealthPercentage = Clamp01(HealthPercentage);

	v2 HealthHUDOffset = V2(0.0f, 1.5f);
	v2 MaxHealthHUDDim = V2(1.0f, 0.2f);
	v2 HealthHUDDim = V2(MaxHealthHUDDim.x * HealthPercentage, MaxHealthHUDDim.y);

	v4 HealthColor = Lerp(V4(1.0f, 0.1f, 0.1f, 1.0f), V4(0.1f, 1.0f, 0.1f, 1.0f), HealthPercentage);

	GorePushRectEntity(
		RenderStack,
		GoreState->PlayerP + HealthHUDOffset,
		MaxHealthHUDDim,
		V2(0.5f, 0.5f),
		1,
		V4(0.15f, 0.15f, 0.15f, 1.0f));

	GorePushRectEntity(
		RenderStack,
		GoreState->PlayerP + HealthHUDOffset,
		HealthHUDDim,
		V2(0.5f, 0.5f),
		1,
		HealthColor);

	float HealthBorderDim = 0.02f;
	v4 HealthBorderColor = V4(0.0f, 0.0f, 0.0f, 1.0f);

	rect2 TopRect = GoreGetEntityRect(
		GoreState->PlayerP + HealthHUDOffset + V2(0.0f, MaxHealthHUDDim.y * 0.5f),
		V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
		V2(0.5f, 1.0f));
	rect2 BottomRect = GoreGetEntityRect(
		GoreState->PlayerP + HealthHUDOffset + V2(0.0f, -MaxHealthHUDDim.y * 0.5f),
		V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
		V2(0.5f, 0.0f));
	rect2 LeftRect = GoreGetEntityRect(
		GoreState->PlayerP + HealthHUDOffset + V2(MaxHealthHUDDim.x * 0.5f, 0.0f),
		V2(HealthBorderDim, MaxHealthHUDDim.y),
		V2(0.0f, 0.5f));
	rect2 RightRect = GoreGetEntityRect(
		GoreState->PlayerP + HealthHUDOffset + V2(-MaxHealthHUDDim.x * 0.5f, 0.0f),
		V2(HealthBorderDim, MaxHealthHUDDim.y),
		V2(1.0f, 0.5f));

	GorePushRectEntity(
		RenderStack,
		TopRect, 1,
		HealthBorderColor);
	GorePushRectEntity(
		RenderStack,
		BottomRect, 1,
		HealthBorderColor);
	GorePushRectEntity(
		RenderStack,
		LeftRect, 1,
		HealthBorderColor);
	GorePushRectEntity(
		RenderStack,
		RightRect, 1,
		HealthBorderColor);

	GoreState->PlayerHealth = Sin(EngineSystems->InputSystem->Time) * 0.5f + 0.5f;

#if 0
	//NOTE(dima): Center anchor
	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(GoreState->PlayerP,
		V2(0.1f, 0.1f),
		V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.4f, 0.0f, 1.0f));
#endif

#if 1
	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(
			MinkovskiRect.Min,
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.0f, 1.0f, 1.0f));

	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(
			MinkovskiRect.Max,
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.0f, 1.0f, 1.0f));

	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(
			V2(MinkovskiRect.Max.x, MinkovskiRect.Min.y),
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.0f, 1.0f, 1.0f));

	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(
			V2(MinkovskiRect.Min.x, MinkovskiRect.Max.y),
			V2(0.1f, 0.1f),
			V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.0f, 1.0f, 1.0f));

	if (MoveHitHappened) {
		GorePushRectEntity(
			RenderStack,
			GoreGetEntityRect(
				MoveRayHit.HitPoint,
				V2(0.1f, 0.1f),
				V2(0.5f, 0.5f)), 1,
			V4(1.0f, 1.0f, 0.0f, 1.0f));
	}

#endif

	VisualizeRay(RenderStack, &MoveRay, V4(1.0f, 0.0f, 1.0f, 1.0f));

	//NOTE(dima): Floor
	for (int i = -5; i <= 5; i++) {
		GorePushRectEntity(
			RenderStack,
			V2((float)i, 0.0f),
			V2(1.0f, 1.0f),
			V2(0.5f, 0.0f),
			GoreState->PlayerFacingLeft,
			GetColor(EngineSystems->ColorsState, Color_Red + i + 5));
	}
}