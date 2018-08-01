#ifndef GORE_RENDER_STACK_H_INCLUDED
#define GORE_RENDER_STACK_H_INCLUDED

#include "gore_asset.h"
#include "gore_game_common.h"
#include "gore_lighting.h"

struct render_state {
	platform_mutex RenderPushMutex;
	stacked_memory Data;
	stacked_memory* InitStack;

	asset_system* AssetSystem;

	int RenderWidth;
	int RenderHeight;

	u32 EntryCount;

	mesh_id LowPolyCylMeshID;

	game_camera_setup CameraSetup;
};

enum render_entry_type {
	RenderEntry_None = 0,

	RenderEntry_Bitmap,
	RenderEntry_Clear,
	RenderEntry_Gradient,
	RenderEntry_Rectangle,
	RenderEntry_Mesh,
	RenderEntry_VoxelMesh,

	RenderEntry_Lighting,
	RenderEntry_VoxelLighting,

	RenderEntry_Glyph,
	RenderEntry_BeginText,
	RenderEntry_EndText,

	RenderEntry_Test,
};

struct render_stack_entry_bitmap {
	bitmap_info* Bitmap;
	v2 P;
	v2 Dim;
	v4 ModulationColor;
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

struct render_stack_entry_voxel_mesh {
	voxel_mesh_info* MeshInfo;

	bitmap_info* VoxelAtlasBitmap;

	v3 P;
};

struct render_stack_entry_lighting {

};

struct render_stack_entry_voxel_lighting {

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

inline void* RENDERPushToStack(render_state* Stack, u32 Size) {
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

inline void* RENDERPushEntryToStack(render_state* Stack, u32 SizeOfType, u32 TypeEnum) {
	render_stack_entry_header* Header =
		(render_stack_entry_header*)RENDERPushToStack(Stack, sizeof(render_stack_entry_header));

	Stack->EntryCount++;
	Header->Type = TypeEnum;
	Header->SizeOfEntryType = SizeOfType;
	void* EntryData = RENDERPushToStack(Stack, SizeOfType);

	return(EntryData);
}
#define PUSH_RENDER_ENTRY(Stack, type, entry_type_enum)	(type *)(RENDERPushEntryToStack(Stack, sizeof(type), entry_type_enum))

inline void RENDERPushBitmap(render_state* Stack, bitmap_info* Bitmap, v2 P, float Height, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_bitmap* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_bitmap, RenderEntry_Bitmap);

	Entry->P = P;
	Entry->Dim = V2(Bitmap->WidthOverHeight * Height, Height);
	Entry->ModulationColor = ModulationColor;

	Entry->Bitmap = Bitmap;
}

inline void RENDERPushRect(render_state* Stack, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderEntry_Rectangle);

	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}


inline void RENDERPushRect(render_state* Stack, rect2 Rect, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_rectangle* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_rectangle, RenderEntry_Rectangle);

	Entry->P = Rect.Min;
	Entry->Dim = Rect.Max - Rect.Min;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERPushRectOutline(render_state* Stack, v2 P, v2 Dim, int PixelWidth, v4 ModulationColor = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), ModulationColor);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), ModulationColor);
}

inline void RENDERPushRectOutline(render_state* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	v2 WidthQuad = V2(PixelWidth, PixelWidth);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y - PixelWidth), V2(Dim.x + 2.0f * PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x - PixelWidth, P.y), V2(PixelWidth, Dim.y + PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + Dim.y), V2(Dim.x + PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x, P.y), V2(PixelWidth, Dim.y), Color);
}

inline void RENDERPushRectInnerOutline(render_state* Stack, rect2 Rect, int PixelWidth, v4 Color = V4(0.0f, 0.0f, 0.0f, 1.0f)) {
	v2 Dim = GetRectDim(Rect);
	v2 P = Rect.Min;

	RENDERPushRect(Stack, V2(P.x, P.y), V2(Dim.x, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x, P.y + PixelWidth), V2(PixelWidth, Dim.y - PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + PixelWidth, P.y + Dim.y - PixelWidth), V2(Dim.x - PixelWidth, PixelWidth), Color);
	RENDERPushRect(Stack, V2(P.x + Dim.x - PixelWidth, P.y + PixelWidth), V2(PixelWidth, Dim.y - 2 * PixelWidth), Color);
}

inline void RENDERPushClear(render_state* Stack, v3 Clear) {
	render_stack_entry_clear* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_clear, RenderEntry_Clear);

	Entry->Color = Clear;
}


inline void RENDERPushMesh(render_state* State, mesh_info* Mesh, mat4 TransformMatrix, surface_material Material) {
	render_stack_entry_mesh* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_mesh, RenderEntry_Mesh);

	Entry->MeshInfo = Mesh;
	Entry->TransformMatrix = TransformMatrix;
	Entry->Material = Material;
}

inline void RENDERPushMesh(render_state* State, mesh_id MeshID, mat4 TransformMatrix, surface_material Material) {
	mesh_info* MeshInfo = GetMeshFromID(State->AssetSystem, MeshID);

	if (MeshInfo) {
		RENDERPushMesh(State, MeshInfo, TransformMatrix, Material);
	}
	else {
		//TODO(dima): Load mesh asset
	}
}

inline void RENDERPushVoxelMesh(render_state* State, voxel_mesh_info* Mesh, v3 P, bitmap_info* VoxelAtlasBitmap) {
	render_stack_entry_voxel_mesh* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_voxel_mesh, RenderEntry_VoxelMesh);

	Entry->MeshInfo = Mesh;
	Entry->P = P;
	Entry->VoxelAtlasBitmap = VoxelAtlasBitmap;
}

inline void RENDERPushVolumeOutline(render_state* State, v3 Min, v3 Max, v3 Color, float Diameter) {
	v3 Diff = Max - Min;

	surface_material OutlineMat = LITCreateSurfaceMaterial(1.0f, Color);

	mat4 InitTran = TranslationMatrix(V3(0.0f, 0.5f, 1.0f));
	mat4 VertTran = ScalingMatrix(V3(Diameter, Diff.y, Diameter)) * InitTran;

	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, 0.0f)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, Diff.z)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, 0.0f, Diff.z)), OutlineMat);

	VertTran = ScalingMatrix(V3(Diameter, Diff.x, Diameter)) * InitTran;
	mat4 RotationMat = RotationZ(-GORE_PI / 2.0f);
	VertTran = RotationMat * VertTran;
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, 0.0f)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, Diff.z)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, 0.0f, Diff.z)), OutlineMat);


	VertTran = ScalingMatrix(V3(Diameter, Diff.x, Diameter)) * InitTran;
	RotationMat = RotationX(GORE_PI / 2.0f);
	VertTran = RotationMat * VertTran;
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, 0.0f, 0.0f)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(Diff.x, Diff.y, 0.0f)), OutlineMat);
	RENDERPushMesh(State, State->LowPolyCylMeshID, Translate(VertTran, Min + V3(0.0f, Diff.y, 0.0f)), OutlineMat);

}

inline void RENDERPushLighting(render_state* State) {
	render_stack_entry_lighting* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_lighting, RenderEntry_Lighting);


}

inline void RENDERPushVoxelLighting(render_state* State) {
	render_stack_entry_voxel_lighting* Entry = PUSH_RENDER_ENTRY(State, render_stack_entry_voxel_lighting, RenderEntry_VoxelLighting);

}

inline void RENDERPushGradient(render_state* Stack, v3 Color) {
	render_stack_entry_gradient* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_gradient, RenderEntry_Gradient);

	Entry->Color = Color;
}

inline void RENDERPushBeginText(render_state* Stack, font_info* FontInfo) {
	render_stack_entry_begin_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_begin_text, RenderEntry_BeginText);

	Entry->FontInfo = FontInfo;
}

inline void RENDERPushEndText(render_state* Stack) {
	render_stack_entry_end_text* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_end_text, RenderEntry_EndText);
}

inline void RENDERPushGlyph(render_state* Stack, int Codepoint, v2 P, v2 Dim, v4 ModulationColor = V4(1.0f, 1.0f, 1.0f, 1.0f)) {
	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderEntry_Glyph);

	Entry->Codepoint = Codepoint;
	Entry->P = P;
	Entry->Dim = Dim;
	Entry->ModulationColor = ModulationColor;
}

inline void RENDERSetCameraSetup(render_state* State, game_camera_setup Setup) {
	State->CameraSetup = Setup;
}

inline void RENDERPushTest(render_state* Stack) {
	render_stack_entry_glyph* Entry = PUSH_RENDER_ENTRY(Stack, render_stack_entry_glyph, RenderEntry_Test);
}

extern render_state RENDERBeginStack(stacked_memory* RenderMemory, int WindowWidth, int WindowHeight, asset_system* AssetSystem);
extern void RENDEREndStack(render_state* Stack);

#endif