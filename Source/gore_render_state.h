#ifndef GORE_RENDER_STACK_H_INCLUDED
#define GORE_RENDER_STACK_H_INCLUDED

#include "gore_asset.h"
#include "gore_input.h"
#include "gore_game_common.h"
#include "gore_lighting.h"
#include "gore_lpterrain.h"

struct render_stack {
	struct render_state* ParentRenderState;

	char Name[32];

	render_stack* Next;
	render_stack* Prev;

	stacked_memory Data;
	stacked_memory InitStack;

	b32 CameraSetupIsSet;
	game_camera_setup* CameraSetup;

	u32 EntryCount;
};

inline b32 RenderStackIsEmpty(render_stack* Stack) {
	b32 Result = (Stack->EntryCount == 0);

	return(Result);
}

struct render_state {
	stacked_memory* RenderMemory;

	render_stack Sentinel;

	struct {
		render_stack* Main;
		render_stack* GUI;
		render_stack* LpterMain;
		render_stack* LpterWater;
	}NamedStacks;

	asset_system* AssetSystem;
	input_system* InputSystem;

	int RenderWidth;
	int RenderHeight;

	mesh_id LowPolyCylMeshID;
};

enum render_entry_type {
	RenderEntry_None = 0,

	//NOTE(dima): Bitmap is the screenspace bitmap
	RenderEntry_Bitmap,
	//NOTE(dima): Sprite is the bitmap that would be rendered in worldspace
	RenderEntry_Sprite,
	RenderEntry_Clear,
	RenderEntry_Gradient,
	RenderEntry_Rectangle,
	RenderEntry_Mesh,
	RenderEntry_VoxelMesh,

	RenderEntry_LpterMesh,
	RenderEntry_LpterWaterMesh,

	RenderEntry_GUI_Glyph,
	RenderEntry_GUI_BeginText,
	RenderEntry_GUI_EndText,

	RenderEntry_Test,
};

struct render_stack_entry_bitmap {
	bitmap_info* Bitmap;
	v2 P;
	v2 Dim;
	v4 ModulationColor;
};


enum sprite_type {
	SpriteType_Rectangle,
	SpriteType_Circle,
};

struct render_stack_entry_sprite {
	b32 BitmapIsSet;
	bitmap_info* Bitmap;

	union {
		struct {
			rect2 SpriteRect;
		}Rectangle;

		struct {
			v2 At;
			float Radius;
		}Circle;
	};

	v4 ModulationColor;
	u32 SpriteType;
	b32 MirrorUVsHorizontally;
};

struct render_stack_entry_clear {
	v3 Color;
};

struct render_stack_entry_gradient {
	v3 Color;
};

struct render_stack_entry_rectangle {
	v4 ModulationColor;
	v2 P;
	v2 Dim;
};

struct render_stack_entry_glyph {
	int Codepoint;

	v2 P;
	v2 Dim;

	v4 ModulationColor;
};

struct render_stack_entry_mesh {
	mesh_info* MeshInfo;

	mat4 TransformMatrix;
	surface_material Material;
};

struct render_stack_entry_lpter_mesh {
	lpter_mesh* Mesh;

	v3 P;
};

struct render_stack_entry_voxel_mesh {
	voxel_mesh_info* MeshInfo;

	bitmap_info* VoxelAtlasBitmap;

	v3 P;
};

struct render_stack_entry_lpter_water_mesh {
	lpter_water* WaterMesh;

	v3 P;
};

struct render_stack_entry_lighting {

};

struct render_stack_entry_voxel_lighting {
	v3 FogColor;
};

struct render_stack_entry_begin_text {
	font_info* FontInfo;
};

struct render_stack_entry_end_text {

};

struct render_stack_entry_header {
	u32 Type;
	u32 SizeOfEntryType;
};

inline void* RENDERPushToStack(render_stack* Stack, u32 Size) {
	void* Result = 0;

	void* MemPushed = PushSomeMemory(&Stack->Data, Size);
	if (MemPushed) {
		Result = MemPushed;
	}
	else {
		Assert(!"Stack was corrupted");
	}

	return(Result);
}

inline void* RENDERPushEntryToStack(render_stack* Stack, u32 SizeOfType, u32 TypeEnum) {
	render_stack_entry_header* Header =
		(render_stack_entry_header*)RENDERPushToStack(Stack, sizeof(render_stack_entry_header));

	Stack->EntryCount++;
	Header->Type = TypeEnum;
	Header->SizeOfEntryType = SizeOfType;
	void* EntryData = RENDERPushToStack(Stack, SizeOfType);

	return(EntryData);
}
#define PUSH_RENDER_ENTRY(Stack, type, entry_type_enum)	(type *)(RENDERPushEntryToStack(Stack, sizeof(type), entry_type_enum))

inline void RENDERPushBitmap(render_stack* Stack, bitmap_info* Bitmap, v2 P, float Height, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_bitmap* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_bitmap, RenderEntry_Bitmap);

	Entry->P = P;
	Entry->Dim = V2(Bitmap->WidthOverHeight * Height, Height);
	Entry->ModulationColor = ModulationColor;

	Entry->Bitmap = Bitmap;
}

inline void RENDERPushRectSprite(render_stack* Stack, bitmap_info* Bitmap, rect2 Rect, b32 FacingLeft, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_sprite* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_sprite, RenderEntry_Sprite);

	Entry->SpriteType = SpriteType_Rectangle;

	Entry->Bitmap = Bitmap;
	Entry->BitmapIsSet = Bitmap ? 1 : 0;
	Entry->ModulationColor = ModulationColor;

	Entry->Rectangle.SpriteRect = Rect;

	Entry->MirrorUVsHorizontally = FacingLeft ? 1 : 0;
}

inline void RENDERPushCircleSprite(render_stack* Stack, bitmap_info* Bitmap, v2 At, float Radius, b32 FacingLeft, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_sprite* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_sprite, RenderEntry_Sprite);
	
	Entry->SpriteType = SpriteType_Circle;

	Entry->Bitmap = Bitmap;
	Entry->BitmapIsSet = Bitmap ? 1 : 0;
	Entry->ModulationColor = ModulationColor;

	Entry->Circle.At = At;
	Entry->Circle.Radius = Radius;

	Entry->MirrorUVsHorizontally = FacingLeft ? 1 : 0;
}

inline void RENDERPushRect(render_stack* Stack, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderEntry_Rectangle);

	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}


inline void RENDERPushRect(render_stack* Stack, rect2 Rect, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderEntry_Rectangle);

	Entry->P = Rect.Min;
	Entry->Dim = Rect.Max - Rect.Min;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERPushRectOutline(render_stack* Stack, v2 P, v2 Dim, int PixelWidth, v4 ModulationColor = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), ModulationColor);
}

inline void RENDERPushRectOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

inline void RENDERPushRectInnerOutline(render_stack* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	RENDERPushRect(Stack, V2(P.x, P.y), V2(Dim.x, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + PixelWidth), V2(PixelWidth, Dim.y - PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + PixelWidth, P.y + Dim.y - PixelWidth), V2(Dim.x - PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x - PixelWidth, P.y + PixelWidth), V2(PixelWidth, Dim.y - 2 * PixelWidth), Color);
}

inline void RENDERPushClear(render_stack* Stack, v3 Clear) {
	render_stack_entry_clear* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_clear, RenderEntry_Clear);

	Entry->Color = Clear;
}


inline void RENDERPushMesh(render_stack* State, mesh_info* Mesh, mat4 TransformMatrix, surface_material Material) {
	render_stack_entry_mesh* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_mesh, RenderEntry_Mesh);

	Entry->MeshInfo = Mesh;
	Entry->TransformMatrix = TransformMatrix;
	Entry->Material = Material;
}

inline void RENDERPushMesh(render_stack* State, mesh_id MeshID, mat4 TransformMatrix, surface_material Material) {
	mesh_info* MeshInfo = GetMeshFromID(State->ParentRenderState->AssetSystem, MeshID);

	if (MeshInfo) {
		RENDERPushMesh(State, MeshInfo, TransformMatrix, Material);
	}
	else {
		//TODO(dima): Load mesh asset
	}
}

inline void RENDERPushLpterMesh(render_stack* State, lpter_mesh* Mesh, v3 P) {
	render_stack_entry_lpter_mesh* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_lpter_mesh, RenderEntry_LpterMesh);

	Entry->Mesh = Mesh;
	Entry->P = P;
}

inline void RENDERPushLpterWaterMesh(render_stack* State, lpter_water* Water, v3 P) {
	render_stack_entry_lpter_water_mesh* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_lpter_water_mesh, RenderEntry_LpterWaterMesh);

	Entry->P = P;
	Entry->WaterMesh = Water;
}

inline void RENDERPushVolumeOutline(render_stack* Stack, v3 Min, v3 Max, v3 Color, float Diameter) {
	v3 Diff = Max - Min;

	surface_material OutlineMat = LITCreateSurfaceMaterial(1.0f, Color);

	mat4 InitTran = TranslationMatrix(V3(0.0f, 0.5f, 0.0f));
	mat4 VertTran = ScalingMatrix(V3(Diameter, Diff.y, Diameter)) * InitTran;

	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, 0.0f)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, Diff.z)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, 0.0f, Diff.z)), OutlineMat);

	VertTran = ScalingMatrix(V3(Diameter, Diff.x, Diameter)) * InitTran;
	mat4 RotationMat = RotationZ(-GORE_PI / 2.0f);
	VertTran = RotationMat * VertTran;
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, 0.0f)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, Diff.z)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, 0.0f, Diff.z)), OutlineMat);


	VertTran = ScalingMatrix(V3(Diameter, Diff.x, Diameter)) * InitTran;
	RotationMat = RotationX(GORE_PI / 2.0f);
	VertTran = RotationMat * VertTran;
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, 0.0f)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, Diff.y, 0.0f)), OutlineMat);
	RENDERPushMesh(Stack, Stack->ParentRenderState->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, 0.0f)), OutlineMat);

}

inline void RENDERPushGradient(render_stack* Stack, v3 Color) {
	render_stack_entry_gradient* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_gradient, RenderEntry_Gradient);

	Entry->Color = Color;
}

inline void RENDERPushBeginText(render_stack* Stack, font_info* FontInfo) {
	render_stack_entry_begin_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_begin_text, RenderEntry_GUI_BeginText);

	Entry->FontInfo = FontInfo;
}

inline void RENDERPushEndText(render_stack* Stack) {
	render_stack_entry_end_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_end_text, RenderEntry_GUI_EndText);
}

inline void RENDERPushGlyph(render_stack* Stack, int Codepoint, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderEntry_GUI_Glyph);

	Entry->Codepoint = Codepoint;
	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERSetCameraSetup(render_stack* State, game_camera_setup* Setup) {
	State->CameraSetup = Setup;
	State->CameraSetupIsSet = 1;
}

inline void RENDERPushTest(render_stack* Stack) {
	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderEntry_Test);
}

/*
	 /$$$$$$$$ /$$   /$$ /$$   /$$  /$$$$$$  /$$$$$$$$ /$$$$$$  /$$$$$$  /$$   /$$  /$$$$$$                   
	| $$_____/| $$  | $$| $$$ | $$ /$$__  $$|__  $$__/|_  $$_/ /$$__  $$| $$$ | $$ /$$__  $$                  
	| $$      | $$  | $$| $$$$| $$| $$  \__/   | $$     | $$  | $$  \ $$| $$$$| $$| $$  \__/                  
	| $$$$$   | $$  | $$| $$ $$ $$| $$         | $$     | $$  | $$  | $$| $$ $$ $$|  $$$$$$                   
	| $$__/   | $$  | $$| $$  $$$$| $$         | $$     | $$  | $$  | $$| $$  $$$$ \____  $$                  
	| $$      | $$  | $$| $$\  $$$| $$    $$   | $$     | $$  | $$  | $$| $$\  $$$ /$$  \ $$                  
	| $$      |  $$$$$$/| $$ \  $$|  $$$$$$/   | $$    /$$$$$$|  $$$$$$/| $$ \  $$|  $$$$$$/       /$$ /$$ /$$
	|__/       \______/ |__/  \__/ \______/    |__/   |______/ \______/ |__/  \__/ \______/       |__/|__/|__/
*/                                                                                                          

render_stack* RenderInitStack(render_state* RenderState, u32 StackByteSize, char* Name);
void RenderBeginFrame(render_state* RenderState);
void RenderEndFrame(render_state* RenderState);
void RenderInitState(
	render_state* Result,
	stacked_memory* RenderMemory,
	int RenderWidth,
	int RenderHeight,
	asset_system* AssetSystem,
	input_system* InputSystem);

#endif