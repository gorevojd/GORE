#include "row.h"

void ROWPushRectEntity(
	render_stack* Stack, 
	bitmap_info* Bitmap, 
	float BitmapHeight, 
	v2 P, v2 Dim, 
	v2 AlignFromTopLeft, 
	float PixelsPerMeter, 
	b32 FacingLeft)
{
	int ScreenPixelCenterX = Stack->ParentRenderState->RenderWidth / 2;
	int ScreenPixelCenterY = Stack->ParentRenderState->RenderHeight / 2;
	v2 ScreenCenter = V2(-ScreenPixelCenterX, ScreenPixelCenterY / 2);
	
	P *= PixelsPerMeter;
	P += ScreenCenter;

	if (Bitmap) {
		Dim = V2(Bitmap->WidthOverHeight * BitmapHeight, BitmapHeight);
	}
	Dim *= PixelsPerMeter;

	v2 TargetRectMin = V2(
		P.x - AlignFromTopLeft.x * Dim.x,
		P.y - (1.0f - AlignFromTopLeft.y) * Dim.y);

	rect2 ResultEntityRect = Rect2MinDim(TargetRectMin, Dim);

	RENDERPushRectSprite(Stack, Bitmap, ResultEntityRect, FacingLeft);
}

void ROWPushCircleEntity(render_stack* Stack, v2 P, float Radius, float PixelsPerMeter) {

}

void UpdateROW(game_mode_state* GameModeState, engine_systems* EngineSystems) {
	row_state* ROWState = (row_state*)GameModeState->GameModeMemory.BaseAddress;

	if (!ROWState->IsInitialized) {

		PushStruct(&GameModeState->GameModeMemory, row_state);
		ROWState->GameModeMemory = &GameModeState->GameModeMemory;

		ROWState->Camera = GAMECreateCamera();
		ROWState->CameraSetup = {};

		ROWState->PixelsPerMeter = 100.0f;

		ROWState->PlayerBitmapID = GetFirstBitmap(EngineSystems->AssetSystem, GameAsset_Lilboy);
		
		ROWState->PlayerP = {};
		ROWState->PlayerVelocity = {};;
		ROWState->PlayerDim = V2(1.0f, 3.0f / 2.0f);
		ROWState->PlayerVelocity = V2(0.0f, 0.0f);
		ROWState->PlayerFacingLeft = 1;

		float TimeForHalfJump = 0.35f;
		float JumpHeight = 4.0f;
		ROWState->Gravity = V2(0.0f, -2.0f * JumpHeight / (TimeForHalfJump * TimeForHalfJump));
		ROWState->InitJumpVelocity = V2(0.0f, 2.0f * JumpHeight / TimeForHalfJump);

		ROWState->IsInitialized = 1;
	}

	ROWState->Camera.Position.z = 5;

	float dt = EngineSystems->InputSystem->DeltaTime;

	float Speed = 10.0f;
	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_A) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Left)) 
	{
		ROWState->PlayerP.x += Speed * dt;
		ROWState->PlayerFacingLeft = 1;
	}

	if (ButtonIsDown(EngineSystems->InputSystem, KeyType_D) || 
		ButtonIsDown(EngineSystems->InputSystem, KeyType_Right)) 
	{
		ROWState->PlayerP.x -= Speed * dt;
		ROWState->PlayerFacingLeft = 0;
	}

	ROWState->PlayerP += ROWState->PlayerVelocity * dt + ROWState->Gravity * dt * dt * 0.5f;

	ROWState->PlayerVelocity += ROWState->Gravity * dt;

	if (ROWState->PlayerP.y < 0.0f) 
	{
		ROWState->PlayerP.y = 0.0f;
		ROWState->PlayerVelocity.y = 0.0f;
	}

	if (ButtonWentDown(EngineSystems->InputSystem, KeyType_Space) || 
		ButtonWentDown(EngineSystems->InputSystem, KeyType_Up)) 
	{
		ROWState->PlayerVelocity = ROWState->InitJumpVelocity;
	}

	GAMEUpdateCameraVectorsBasedOnUpAndFront(
		&ROWState->Camera, 
		V3(0.0f, 0.0f, -1.0f), 
		V3(0.0f, 1.0f, 0.0f));

	ROWState->CameraSetup = GAMECameraSetup(
		ROWState->Camera,
		EngineSystems->RenderState->RenderWidth,
		EngineSystems->RenderState->RenderHeight,
		CameraProjection_Orthographic,
		0.1f, 100.0f);

	render_stack* RenderStack = EngineSystems->RenderState->NamedStacks.Main;

	RENDERSetCameraSetup(RenderStack, &ROWState->CameraSetup);

	//RENDERPushClear(RenderStack, V3(1.0f, 1.0f, 1.0f));

	bitmap_info* PlayerBitmap = GetBitmapFromID(EngineSystems->AssetSystem, ROWState->PlayerBitmapID);
	//bitmap_info* PlayerBitmap = 0;

	ROWPushRectEntity(
		RenderStack, 
		PlayerBitmap,
		ROWState->PlayerDim.y * 1.5f,
		ROWState->PlayerP, 
		ROWState->PlayerDim, 
		V2(0.5f, 1.0f), 
		ROWState->PixelsPerMeter,
		ROWState->PlayerFacingLeft);
	
	//NOTE(dima): Floor
	ROWPushRectEntity(
		RenderStack,
		0,
		0.0f,
		V2(ROWState->PlayerP.x, 0.0f),
		V2(100.0f, 1.0f),
		V2(0.5f, 0.0f),
		ROWState->PixelsPerMeter,
		ROWState->PlayerFacingLeft);
}