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

/*
	NOTE(dima):
		Raycasting implemented

		Raycasting was implemented here only for
		rectangular shapes. 

		1) It works through 
		first using Minkowski difference to build
		rectangle across which we will test 
		our point. 

		2) Then linear equations of wall lines 
		are built

		3) Then hit point is found

		4) At the end of the algorighm closest hit 
		point is found

	IMPORTANT(dima): Need to optimize this because
		test happens across all the walls and all 
		the lines that walls contain. For every player
		the same line equations are calculated. 
		Maybe I should precalculate line equations
		for static walls. But there will be some 
		problems with the minkovski difference...

		Maybe we should SIMD this!!!
*/

//NOTE(dima): Now those only just for player :(((
b32 GoreRaycast2DWallsForDimensionalEntity(
	gore_state* GoreState,
	v2 EntityDim, 
	v2 EntityTopLeftAlign,
	gore_ray2d* Ray, 
	gore_raycast_hit* OutHitData) 
{

	b32 AtLeastOneHitHappened = 0;
	gore_raycast_hit ClosestHit;
	float MinDistanceToHit = 99999999.0f;

	for (int WallIndex = 0; WallIndex < GoreState->WallCount; WallIndex++) {

		gore_wall* Wall = GoreState->Walls + WallIndex;

		rect2 WallRect = GoreGetEntityRect(Wall->At, Wall->Dim, Wall->TopLeftAlign);

		gore_raycast_hit HitDatas[4];

		rect2 MinkovskiRect = WallRect;

		//NOTE(dima): Minkovski based collision detection
		v2 LeftRightAddition = V2(EntityDim.x * EntityTopLeftAlign.x, EntityDim.x * (1.0f - EntityTopLeftAlign.x));
		v2 TopBottomAddition = V2(EntityDim.y * EntityTopLeftAlign.y, EntityDim.y * (1.0f - EntityTopLeftAlign.y));

		MinkovskiRect.Min.x -= LeftRightAddition.x;
		MinkovskiRect.Max.x += LeftRightAddition.y;
		MinkovskiRect.Min.y -= TopBottomAddition.x;
		MinkovskiRect.Max.y += TopBottomAddition.y;

		//NOTE(dima): Theese values should be precalculated for static elements
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

				//NOTE(dima): We need to check if ray is before the line
				if (HitData->DistanseFromRayOrigin >= 0.0f) {

					//NOTE(dima): Checking if ray is long enough to hit the line
					b32 RayHitsTheLine = HitData->DistanseFromRayOrigin * HitData->DistanseFromRayOrigin < Ray->SqLen;
					if (RayHitsTheLine) {

						v2 HitPP1 = Edge->Min - HitData->HitPoint;
						v2 HitPP2 = Edge->Max - HitData->HitPoint;

						//HitPP1 = Normalize(HitPP1);
						//HitPP2 = Normalize(HitPP2);

						//NOTE(dima): If hit point is between edge min and max points
						if (Dot(HitPP1, HitPP2) <= 0.0f) {
							HitData->HitHappened = 1;
							AtLeastOneHitHappened = 1;
						}
					}
				}
			}
		}

		for (int HitIndex = 0; HitIndex < 4; HitIndex++) {
			gore_raycast_hit* HitData = HitDatas + HitIndex;

			if (HitData->HitHappened) {
				if (HitData->DistanseFromRayOrigin < MinDistanceToHit) {
					ClosestHit = *HitData;
					MinDistanceToHit = HitData->DistanseFromRayOrigin;
				}
			}
		}
	}

	if (AtLeastOneHitHappened && OutHitData) {
		*OutHitData = ClosestHit;
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

		GoreState->ViewPlayerIndex = 0;

		//NOTE(dima): Players
		float TimeForHalfJump = 0.35f;
		float JumpHeight = 4.0f;
		float PlayersInitSpeed = 10.0f;

		GoreState->PlayerCount = 3;
		GoreState->Players = PushArray(GoreState->GameModeMemory, gore_player, GoreState->PlayerCount);
		
		GoreState->Players[0].P = V2(0.0f, 4.0f);
		GoreState->Players[0].Velocity = {};
		GoreState->Players[0].Dim = V2(0.7f, 3.0f / 2.0f * 0.7f);
		GoreState->Players[0].Align = V2(0.5f, 1.0f);
		GoreState->Players[0].Velocity = V2(0.0f, 0.0f);
		GoreState->Players[0].FacingLeft = 1;
		GoreState->Players[0].Health = 0.75f;
		GoreState->Players[0].MaxHealth = 1.0f;
		GoreState->Players[0].Gravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		GoreState->Players[0].InitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);
		GoreState->Players[0].Speed = PlayersInitSpeed;
		GoreState->Players[0].PlayerBitmapID = GetAssetByBestFloatTag(
			EngineSystems->AssetSystem,
			GameAsset_Lilboy,
			GameAssetTag_Lilboy,
			0, AssetType_Bitmap);

		GoreState->Players[1].P = V2(3.0f, 0.0f);
		GoreState->Players[1].Velocity = {};
		GoreState->Players[1].Dim = V2(0.7f, 3.0f / 2.0f * 0.7f);
		GoreState->Players[1].Align = V2(0.5f, 1.0f);
		GoreState->Players[1].Velocity = V2(0.0f, 0.0f);
		GoreState->Players[1].FacingLeft = 1;
		GoreState->Players[1].Health = 0.75f;
		GoreState->Players[1].MaxHealth = 1.0f;
		GoreState->Players[1].Gravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		GoreState->Players[1].InitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);
		GoreState->Players[1].Speed = PlayersInitSpeed;
		GoreState->Players[1].PlayerBitmapID = GetAssetByBestFloatTag(
			EngineSystems->AssetSystem,
			GameAsset_Lilboy,
			GameAssetTag_Lilboy,
			1, AssetType_Bitmap);

		GoreState->Players[2].P = V2(5.0f, 0.0f);
		GoreState->Players[2].Velocity = {};
		GoreState->Players[2].Dim = V2(0.7f, 3.0f / 2.0f * 0.7f);
		GoreState->Players[2].Align = V2(0.5f, 1.0f);
		GoreState->Players[2].Velocity = V2(0.0f, 0.0f);
		GoreState->Players[2].FacingLeft = 1;
		GoreState->Players[2].Health = 0.75f;
		GoreState->Players[2].MaxHealth = 1.0f;
		GoreState->Players[2].Gravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		GoreState->Players[2].InitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);
		GoreState->Players[2].Speed = PlayersInitSpeed;
		GoreState->Players[2].PlayerBitmapID = GetAssetByBestFloatTag(
			EngineSystems->AssetSystem,
			GameAsset_Lilboy,
			GameAssetTag_Lilboy,
			2, AssetType_Bitmap);

		//NOTE(dima): Walls
		GoreState->WallCount = 3;
		GoreState->Walls = PushArray(GoreState->GameModeMemory, gore_wall, GoreState->WallCount);

		GoreState->Walls[0].At = V2(0.0f, 6.5f);
		GoreState->Walls[0].Dim = V2(3.0f, 3.0f);
		GoreState->Walls[0].TopLeftAlign = V2(0.5f, 0.0f);
		GoreState->Walls[0].IsDynamic = 0;

		GoreState->Walls[1].At = V2(4.0f, 6.5f);
		GoreState->Walls[1].Dim = V2(3.0f, 1.0f);
		GoreState->Walls[1].TopLeftAlign = V2(0.5f, 0.0f);
		GoreState->Walls[1].IsDynamic = 0;

		GoreState->Walls[2].At = V2(4.0f, 3.0f);
		GoreState->Walls[2].Dim = V2(7.0f, 1.0f);
		GoreState->Walls[2].TopLeftAlign = V2(0.5f, 0.0f);
		GoreState->Walls[2].IsDynamic = 0;

		//NOTE(dima): Flying
		GoreState->FlyingDim = V2(0.5f, 0.2f);
		GoreState->FlyingAlign = V2(0.5f, 0.5f);
		GoreState->FlyingTimeToLive = 2.0f;

		GoreState->IsInitialized = 1;
	}

	gore_player* CameraFollowEntity = &GoreState->Players[GoreState->ViewPlayerIndex];
	v3 InitCameraOffset = V3(0.0f, 1.5f, 5.0f);

	float dt = EngineSystems->InputSystem->DeltaTime;

	GoreState->Camera.Position = Lerp(
		GoreState->Camera.Position,
		V3(CameraFollowEntity->P.x, CameraFollowEntity->P.y, 0.0f) + InitCameraOffset,
		dt * 3.0f);

	if (ButtonWentDown(EngineSystems->InputSystem, KeyType_E)) {
		GoreState->ViewPlayerIndex = (GoreState->ViewPlayerIndex + 1) % GoreState->PlayerCount;
	}

	//GoreState->Camera.Position = V3(GoreState->PlayerP.x, GoreState->PlayerP.y, 0.0f) + InitCameraOffset;


	//NOTE(dima): Updating player entities
	for (int PlayerIndex = 0;
		PlayerIndex < GoreState->PlayerCount;
		PlayerIndex++)
	{
		gore_player* Player = GoreState->Players + PlayerIndex;

		b32 PlayerShouldJump = 0;

		//NOTE(dima): Clearing velocity before input processing
		Player->Velocity.x = 0.0f;

		if (Player == CameraFollowEntity) {
			if (ButtonIsDown(EngineSystems->InputSystem, KeyType_A) || 
				ButtonIsDown(EngineSystems->InputSystem, KeyType_Left)) 
			{
				Player->Velocity.x = Player->Speed;
				Player->FacingLeft = 1;
			}

			if (ButtonIsDown(EngineSystems->InputSystem, KeyType_D) || 
				ButtonIsDown(EngineSystems->InputSystem, KeyType_Right)) 
			{
				Player->Velocity.x = -Player->Speed;
				Player->FacingLeft = 0;
			}
		
			PlayerShouldJump =
				ButtonWentDown(EngineSystems->InputSystem, KeyType_Space) ||
				ButtonWentDown(EngineSystems->InputSystem, KeyType_Up);
		}

		
		v2 PlayerDeltaP = Player->Velocity * dt + Player->Gravity * dt * dt * 0.5f;

		v2 NextSupposedPlayerPosition = Player->P + PlayerDeltaP;
		v2 NextSupposedHorizontalP = V2(NextSupposedPlayerPosition.x, Player->P.y);
		v2 NextSupposedVerticalP = V2(Player->P.x, NextSupposedPlayerPosition.y);

		//NOTE(dima): Collision detection code
		rect2 PlayerRect = GoreGetEntityRect(Player->P, Player->Dim, Player->Align);

		//NOTE(dima): Minkovski based collision detection
		v2 LeftRightAddition = V2(Player->Dim.x * Player->Align.x, Player->Dim.x * (1.0f - Player->Align.x));
		v2 TopBottomAddition = V2(Player->Dim.y * Player->Align.y, Player->Dim.y * (1.0f - Player->Align.y));

		float MinkovskiOffsetEpsilon = 0.001f;

		/*
			NOTE(dima): So now we have minkovski rect and 
			the point that we can test it against with.

			We can check point at discrete positions and we
			also can cast a ray and test it against 4 edges
			of rectangle. Maybe i should imlement both
		*/

		gore_ray2d MoveRay = GoreRay2D(Player->P, NextSupposedPlayerPosition);
		gore_raycast_hit MoveRayHit;

		gore_ray2d MoveHorizontalRay = GoreRay2D(Player->P, NextSupposedHorizontalP);
		gore_raycast_hit HorzRayHit;

		gore_ray2d MoveVerticalRay = GoreRay2D(Player->P, NextSupposedVerticalP);
		gore_raycast_hit VertRayHit;

		gore_ray2d JumpRay = GoreRay2D(Player->P + V2(0.0f, 0.1f), V2(0.0f, -1.0f), 0.3f);
		gore_raycast_hit JumpRayHit;

		b32 MoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
			GoreState,
			Player->Dim, Player->Align,
			&MoveRay, &MoveRayHit);

		b32 HorzMoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
			GoreState,
			Player->Dim, Player->Align,
			&MoveHorizontalRay, &HorzRayHit);

		b32 VertMoveHitHappened = GoreRaycast2DWallsForDimensionalEntity(
			GoreState,
			Player->Dim, Player->Align,
			&MoveVerticalRay, &VertRayHit);

#if 1
		/*
			NOTE(dima): Player movement physics was done through the
			raycasting to be sure in reliability of movement and etc..

			I've implement it by raycasting move vector, horizontal
			displacement vector and vertical displacement vectors
			through the world to find if move in this frame will hit
			any wall.

			I should think about collisions that would be dynamic and
			non-walls. Current implementation will handle dynamic moving
			walls collisions. But only walls! I want more advanced system
			if i would have to implement enemies to be the walls like in
			mario or braid or etc..
		*/

		v2 PlayerTargetP = NextSupposedPlayerPosition;
		if (HorzMoveHitHappened) {
			v2 HitWithOffset = HorzRayHit.HitPoint + HorzRayHit.HitNormal * MinkovskiOffsetEpsilon;

			PlayerTargetP.x = HitWithOffset.x;

			Player->Velocity.x = 0.0f;
		}

		if (VertMoveHitHappened) {
			v2 HitWithOffset = VertRayHit.HitPoint + VertRayHit.HitNormal * MinkovskiOffsetEpsilon;

			PlayerTargetP.y = HitWithOffset.y;

			Player->Velocity.y = 0.0f;
		}

		if (!VertMoveHitHappened && !HorzMoveHitHappened && MoveHitHappened) {
			v2 HitWithOffset = MoveRayHit.HitPoint + MoveRayHit.HitNormal * MinkovskiOffsetEpsilon;

			PlayerTargetP = HitWithOffset;
			Player->Velocity = V2(0.0f, 0.0f);
		}

		PlayerDeltaP = PlayerTargetP - Player->P;
#else
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
			else if (MinkovskiVerticalTest) {
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
		//NOTE(dima): Player movement finalizing
		Player->P += PlayerDeltaP;
		Player->Velocity += Player->Gravity * dt;

#if 1
		if (Player->P.y < 0.0f)
		{
			Player->P.y = 0.0f;
			Player->Velocity.y = 0.0f;
		}
#endif
		//NOTE(dima): Flying updating
		if (MouseButtonWentDown(EngineSystems->InputSystem, MouseButton_Left)) {
			GoreState->FlyingTimeLived = 0.0f;
			GoreState->FlyingAt = Player->P + V2(0.0f, 0.5f);

			float InitialSpeed = 5.0f;
			v2 ThrowDirection = Player->FacingLeft ? V2(1.0f, 0.0f) : V2(-1.0f, 0.0f);
			GoreState->FlyingVelocity = V2(Player->Velocity.x, 0.0f) + ThrowDirection * InitialSpeed;
		}

		//NOTE(dima): Jump
		if (PlayerShouldJump) {

			b32 JumpRayHitsFloor = GoreRaycast2DWallsForDimensionalEntity(
				GoreState, Player->Dim,
				Player->Align, &JumpRay,
				&JumpRayHit);

			if (JumpRayHitsFloor || Player->P.y < 0.1f) {
				Player->JumpCounter = 0;
			}

			if (Player->JumpCounter < 1) {
				Player->Velocity.y = Player->InitJumpVelocity.y;

				Player->JumpCounter++;
			}
		}

		Player->Health = Sin(EngineSystems->InputSystem->Time) * 0.5f + 0.5f;
	}

	//NOTE(dima): Flying updating
	GoreState->FlyingAt = GoreState->FlyingAt + GoreState->FlyingVelocity * dt;
	GoreState->FlyingTimeLived += dt;

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

	//RENDERPushClear(RenderStack, V3(0.2f, 0.2f, 0.2f));

	//bitmap_info* PlayerBitmap = 0;

	
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

	//NOTE(dima): Walls
	for (int WallIndex = 0; WallIndex < GoreState->WallCount; WallIndex++) {
		gore_wall* Wall = GoreState->Walls + WallIndex;

		rect2 CurWallRect = GoreGetEntityRect(
			Wall->At, 
			Wall->Dim, 
			Wall->TopLeftAlign);

		GorePushRectEntity(RenderStack, CurWallRect, 1, V4(0.5f, 0.8f, 0.3f, 1.0f));
	}

	//NOTE(dima): Players
	for (int PlayerIndex = 0;
		PlayerIndex < GoreState->PlayerCount;
		PlayerIndex++) 
	{
		gore_player* Player = GoreState->Players + PlayerIndex;

		bitmap_info* PlayerBitmap = GetBitmapFromID(EngineSystems->AssetSystem, Player->PlayerBitmapID);

		//NOTE(dima): Pushing player bitmap
#if 1
		GorePushRectEntity(
			RenderStack,
			Player->P,
			Player->Dim,
			V2(0.5f, 1.0f),
			Player->FacingLeft);
#endif

		GorePushBitmapEntity(
			RenderStack,
			PlayerBitmap,
			Player->Dim.y * 2.2f,
			Player->P,
			Player->Dim,
			V2(0.5f, 1.0f),
			Player->FacingLeft);


		float HealthPercentage = Player->Health / Player->MaxHealth;
		HealthPercentage = Clamp01(HealthPercentage);

		v2 HealthHUDOffset = V2(0.0f, 1.5f);
		v2 MaxHealthHUDDim = V2(1.0f, 0.2f);
		v2 HealthHUDDim = V2(MaxHealthHUDDim.x * HealthPercentage, MaxHealthHUDDim.y);

		v4 HealthColor = Lerp(V4(1.0f, 0.1f, 0.1f, 1.0f), V4(0.1f, 1.0f, 0.1f, 1.0f), HealthPercentage);

		GorePushRectEntity(
			RenderStack,
			Player->P + HealthHUDOffset,
			MaxHealthHUDDim,
			V2(0.5f, 0.5f),
			1,
			V4(0.15f, 0.15f, 0.15f, 1.0f));

		GorePushRectEntity(
			RenderStack,
			Player->P + HealthHUDOffset,
			HealthHUDDim,
			V2(0.5f, 0.5f),
			1,
			HealthColor);

		float HealthBorderDim = 0.02f;
		v4 HealthBorderColor = V4(0.0f, 0.0f, 0.0f, 1.0f);

		rect2 TopRect = GoreGetEntityRect(
			Player->P + HealthHUDOffset + V2(0.0f, MaxHealthHUDDim.y * 0.5f),
			V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
			V2(0.5f, 1.0f));
		rect2 BottomRect = GoreGetEntityRect(
			Player->P + HealthHUDOffset + V2(0.0f, -MaxHealthHUDDim.y * 0.5f),
			V2(MaxHealthHUDDim.x + 2.0f * HealthBorderDim, HealthBorderDim),
			V2(0.5f, 0.0f));
		rect2 LeftRect = GoreGetEntityRect(
			Player->P + HealthHUDOffset + V2(MaxHealthHUDDim.x * 0.5f, 0.0f),
			V2(HealthBorderDim, MaxHealthHUDDim.y),
			V2(0.0f, 0.5f));
		rect2 RightRect = GoreGetEntityRect(
			Player->P + HealthHUDOffset + V2(-MaxHealthHUDDim.x * 0.5f, 0.0f),
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

	}

	//NOTE(dima): Flying entity rendering
	GorePushRectEntity(
		RenderStack,
		GoreState->FlyingAt,
		GoreState->FlyingDim,
		GoreState->FlyingAlign,
		1,
		V4(1.0f, 1.0f, 1.0f, 1.0f));
	

#if 0
	//NOTE(dima): Center anchor
	GorePushRectEntity(
		RenderStack,
		GoreGetEntityRect(GoreState->PlayerP,
		V2(0.1f, 0.1f),
		V2(0.5f, 0.5f)), 1,
		V4(1.0f, 0.4f, 0.0f, 1.0f));
#endif

	//NOTE(dima): Floor
	for (int i = -5; i <= 5; i++) {
		GorePushRectEntity(
			RenderStack,
			V2((float)i, 0.0f),
			V2(1.0f, 1.0f),
			V2(0.5f, 0.0f),
			0,
			GetColor(EngineSystems->ColorsState, Color_Red + i + 5));
	}
}