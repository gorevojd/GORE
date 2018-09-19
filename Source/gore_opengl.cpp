#include "gore_opengl.h"

enum texture_allocation_flag {
	TextureAllocation_NearestFiltering = 0x01,
	TextureAllocation_GenerateMipmaps = 0x02,
	TextureAllocation_EnableAnisotropic = 0x04,
};

GLuint OpenGLAllocateTexture(
	bitmap_info* Buffer, 
	u32 TextureAllocationFlag,
	gl_state* GLState) 
{
	GLuint TextureHandle;
	glGenTextures(1, &TextureHandle);

	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	if (TextureAllocationFlag & TextureAllocation_EnableAnisotropic) {
		if (GLState->AnisotropicFilteringSupported) {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GLState->AnisotropicLevel);
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (TextureAllocationFlag & TextureAllocation_NearestFiltering) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		Buffer->Width,
		Buffer->Height,
		0,
		GL_ABGR_EXT,
		GL_UNSIGNED_BYTE,
		Buffer->Pixels);


	glGenerateMipmap(GL_TEXTURE_2D);

	Buffer->TextureHandle = (void*)TextureHandle;

	glBindTexture(GL_TEXTURE_2D, 0);

	return(TextureHandle);
}

void OpenGLDeallocateTexture(bitmap_info* Buffer) {
	GLuint TextureHandle = (GLuint)Buffer->TextureHandle;

	glDeleteTextures(1, &TextureHandle);
	Buffer->TextureHandle = (void*)0;
}

GLuint OpenGLLoadProgramFromSource(char* VertexSource, char* FragmentSource) {
	char InfoLog[1024];
	int Success;
	
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Program;

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexSource, 0);
	glCompileShader(VertexShader);
	
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(VertexShader, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentSource, 0);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	glGetProgramiv(Program, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(Program, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	return(Program);
}

gl_program OpenGLLoadShader(char* VertexPath, char* FragmentPath) {
	gl_program Result = {};

	platform_read_file_result VertexFile = PlatformApi.ReadFile(VertexPath);
	platform_read_file_result FragmentFile = PlatformApi.ReadFile(FragmentPath);

	Result.Handle = OpenGLLoadProgramFromSource(
		(char*)VertexFile.Data, 
		(char*)FragmentFile.Data);

	PlatformApi.FreeFileMemory(&VertexFile);
	PlatformApi.FreeFileMemory(&FragmentFile);

	return(Result);
}

static void OpenGLCleanupShader(gl_program* Program) {
	glDeleteProgram(Program->Handle);
}

#define GLGET_ATTRIB(name) glGetAttribLocation(Result.Program.Handle, name)
#define GLGET_UNIFORM(name) glGetUniformLocation(Result.Program.Handle, name)

gl_fxaa_shader OpenGLLoadFXAAShader() {
	gl_fxaa_shader Result = {};

	char* VertexPath = "../Data/Shaders/FXAAShader.vert";
	char* FragmentPath = "../Data/Shaders/FXAAShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PosIndex = GLGET_ATTRIB("inPos");
	Result.TexCoordIndex = GLGET_ATTRIB("inTexCoords");

	Result.TextureLocation = GLGET_UNIFORM("FramebufferTexture");
	Result.TextureSizeLocation = GLGET_UNIFORM("FramebufferSize");

	return(Result);
}

gl_screen_shader OpenGLLoadScreenShader() {
	gl_screen_shader Result = {};

	char* VertexPath = "../Data/Shaders/ScreenShader.vert";
	char* FragmentPath = "../Data/Shaders/ScreenShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PosIndex = GLGET_ATTRIB("aPos");
	Result.TexIndex = GLGET_ATTRIB("aTexCoords");

	Result.ScreenTextureLocation = GLGET_UNIFORM("screenTexture");

	return(Result);
}

gl_wtf_shader OpenGLLoadWtfShader() {
	gl_wtf_shader Result = {};
	
	char* VertexPath = "../Data/Shaders/WtfShader.vert";
	char* FragmentPath = "../Data/Shaders/WtfShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PositionIndex = GLGET_ATTRIB("Position");
	Result.NormalIndex = GLGET_ATTRIB("Normal");
	Result.UVIndex = GLGET_ATTRIB("UV");
	Result.ColorIndex = GLGET_ATTRIB("Color");
	Result.TangentIndex = GLGET_ATTRIB("Tangent");

	Result.ModelMatrixLocation = GLGET_UNIFORM("Model");
	Result.ViewMatrixLocation = GLGET_UNIFORM("View");
	Result.ProjectionMatrixLocation = GLGET_UNIFORM("Projection");
	Result.CameraPLocation = GLGET_UNIFORM("CameraP");

	Result.SurfMatShineLocation = GLGET_UNIFORM("Material.Shine");
	Result.SurfMatColorLocation = GLGET_UNIFORM("Material.Color");

	Result.SurfMatDiffLocation = GLGET_UNIFORM("Material.Diffuse");
	Result.SurfMatSpecLocation = GLGET_UNIFORM("Material.Specular");
	Result.SurfMatEmisLocation = GLGET_UNIFORM("Material.Emissive");

	Result.SurfMatHasDiffLocation = GLGET_UNIFORM("Material.HasDiffuse");
	Result.SurfMatHasSpecLocation = GLGET_UNIFORM("Material.HasSpecular");
	Result.SurfMatHasEmisLocation = GLGET_UNIFORM("Material.HasEmissive");

	return(Result);
}

gl_lpter_shader OpenGLLoadLpterShader() {
	gl_lpter_shader Result = {};

	char* VertexPath = "../Data/Shaders/LpterShader.vert";
	char* FragmentPath = "../Data/Shaders/LpterShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PositionIndex = GLGET_ATTRIB("inPos");
	Result.NormalIndex = GLGET_ATTRIB("inNorm");
	Result.ColorIndex = GLGET_ATTRIB("inColor");

	Result.ModelMatrixLocation = GLGET_UNIFORM("Model");
	Result.ViewMatrixLocation = GLGET_UNIFORM("View");
	Result.ProjectionMatrixLocation = GLGET_UNIFORM("Projection");
	Result.NormalsBufferLocation = GLGET_UNIFORM("NormalsBuffer");
	Result.CameraPLocation = GLGET_UNIFORM("CameraP");
	Result.FogColorLocation = GLGET_UNIFORM("FogColor");

	Result.DirDirectionLocation = GLGET_UNIFORM("DirLight.Direction");
	Result.DirDiffuseLocation = GLGET_UNIFORM("DirLight.Diffuse");
	Result.DirAmbientLocation = GLGET_UNIFORM("DirLight.Ambient");

	glUseProgram(Result.Program.Handle);
	glUniform3f(Result.DirDirectionLocation, 0.5f, -0.5f, 0.5f);
	glUniform3f(Result.DirAmbientLocation, 0.05f, 0.05f, 0.05f);
	glUniform3f(Result.DirDiffuseLocation, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);

	return(Result);
}

gl_lpter_water_shader OpenGLLoadLpterWaterShader() {
	gl_lpter_water_shader Result = {};

	char* VertexPath = "../Data/Shaders/LpterWater.vert";
	char* FragmentPath = "../Data/Shaders/LpterWater.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PositionIndex = GLGET_ATTRIB("inPosition");
	Result.OffsetsToOthersIndex = GLGET_ATTRIB("inOffsetsToOtherPoints");

	Result.GlobalTimeLocation = GLGET_UNIFORM("GlobalTime");
	Result.WaterLevelLocation = GLGET_UNIFORM("WaterLevel");
	Result.CameraPLocation = GLGET_UNIFORM("CameraP");

	Result.ProjectionMatrixLocation = GLGET_UNIFORM("Projection");
	Result.ViewMatrixLocation = GLGET_UNIFORM("View");
	Result.ModelMatrixLocation = GLGET_UNIFORM("Model");
	Result.PerVertexOfffsetLocation = GLGET_UNIFORM("PerVertexOffset");

	Result.DirDiffuseLocation = GLGET_UNIFORM("DirLight.Diffuse");
	Result.DirDirectionLocation = GLGET_UNIFORM("DirLight.Direction");

	v3 VoxDirDir = V3(0.5f, -0.5f, 0.5f);
	v3 VoxDirDif = V3(1.0f, 1.0f, 1.0f);

	Result.Program.Use();
	glUniform3f(
		Result.DirDirectionLocation,
		VoxDirDir.x,
		VoxDirDir.y,
		VoxDirDir.z);
	glUniform3f(
		Result.DirDiffuseLocation,
		VoxDirDif.x,
		VoxDirDif.y,
		VoxDirDif.z);
	glUseProgram(0);

	return(Result);
}

gl_voxel_shader OpenGLLoadVoxelShader() {
	gl_voxel_shader Result = {};

	char* VertexPath = "../Data/Shaders/VoxelShader.vert";
	char* FragmentPath = "../Data/Shaders/VoxelShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.VertexDataIndex = GLGET_ATTRIB("VertexData");

	Result.ModelMatrixLocation = GLGET_UNIFORM("Model");
	Result.ViewMatrixLocation = GLGET_UNIFORM("View");
	Result.ProjectionMatrixLocation = GLGET_UNIFORM("Projection");
	Result.CameraPLocation = GLGET_UNIFORM("CameraP");
	Result.FogColorLocation = GLGET_UNIFORM("FogColor");

	Result.DiffuseMapLocation = GLGET_UNIFORM("DiffuseMap");

	Result.DirDirectionLocation = GLGET_UNIFORM("DirLight.Direction");
	Result.DirDiffuseLocation = GLGET_UNIFORM("DirLight.Diffuse");
	Result.DirAmbientLocation = GLGET_UNIFORM("DirLight.Ambient");

	v3 VoxDirDir = V3(0.5f, -0.5f, 0.5f);
	v3 VoxDirAmb = V3(0.1f, 0.1f, 0.1f);
	v3 VoxDirDif = V3(1.0f, 1.0f, 1.0f);
	v3 FogColor = V3(0.5f, 0.5f, 0.5f);

	Result.Program.Use();
	glUniform3f(
		Result.FogColorLocation,
		FogColor.r,
		FogColor.g,
		FogColor.b);
	glUniform3f(
		Result.DirDirectionLocation,
		VoxDirDir.x,
		VoxDirDir.y,
		VoxDirDir.z);
	glUniform3f(
		Result.DirDiffuseLocation,
		VoxDirDif.x,
		VoxDirDif.y,
		VoxDirDif.z);
	glUniform3f(
		Result.DirAmbientLocation,
		VoxDirAmb.x,
		VoxDirAmb.y,
		VoxDirAmb.z);
	glUseProgram(0);

	return(Result);
}


void OpenGLUniformSurfaceMaterial(gl_state* GLState, render_stack* Stack, gl_wtf_shader* Shader, surface_material* Mat) {
	glUniform1f(Shader->SurfMatShineLocation, Mat->Shine);
	glUniform3f(Shader->SurfMatColorLocation, Mat->Color.x, Mat->Color.y, Mat->Color.z);

	b32 HasDiffuseBitmap = (Mat->Diffuse || Mat->DiffuseInfo);
	b32 HasSpecularBitmap = (Mat->Specular || Mat->SpecularInfo);
	b32 HasEmissiveBitmap = (Mat->Emissive || Mat->EmissiveInfo);

	glUniform1i(Shader->SurfMatHasDiffLocation, HasDiffuseBitmap ? 1 : 0);
	if (HasDiffuseBitmap) {
		bitmap_info* Info = Mat->DiffuseInfo;
		if (Mat->Diffuse) {
			Info = GetBitmapFromID(Stack->ParentRenderState->AssetSystem, Mat->Diffuse);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatDiffLocation, 0);
	}

	glUniform1i(Shader->SurfMatHasSpecLocation, HasSpecularBitmap ? 1 : 0);
	if (HasSpecularBitmap) {
		bitmap_info* Info = Mat->SpecularInfo;
		if (Mat->Specular) {
			Info = GetBitmapFromID(Stack->ParentRenderState->AssetSystem, Mat->Specular);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatSpecLocation, 1);
	}

	glUniform1i(Shader->SurfMatHasEmisLocation, HasEmissiveBitmap ? 1 : 0);
	if (HasEmissiveBitmap) {
		bitmap_info* Info = Mat->EmissiveInfo;
		
		if (Mat->Emissive) {
			Info = GetBitmapFromID(Stack->ParentRenderState->AssetSystem, Mat->Emissive);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatEmisLocation, 2);
	}
}

inline void OpenGLUseProgramBegin(gl_program* Program) {
	glUseProgram(Program->Handle);
}

inline void OpenGLUseProgramEnd() {
	glUseProgram(0);
}

void OpenGLRenderBitmap(gl_state* GLState, bitmap_info* Buffer, v2 P, v2 Dim, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 Rect = Rect2MinDim(P, Dim);

	if (!Buffer->TextureHandle) {
		OpenGLAllocateTexture(Buffer, 0, GLState);
	}

	glBindTexture(GL_TEXTURE_2D, (size_t)Buffer->TextureHandle);
	glBegin(GL_TRIANGLES);

	glColor4f(Color.r, Color.g, Color.b, Color.a);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Rect.Min.x, Rect.Min.y);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(Rect.Max.x, Rect.Min.y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Rect.Max.x, Rect.Max.y);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Rect.Min.x, Rect.Min.y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Rect.Max.x, Rect.Max.y);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(Rect.Min.x, Rect.Max.y);

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderRectangle(rect2 Rect, v4 Color) {
	glBegin(GL_TRIANGLES);

	glColor4f(Color.r, Color.g, Color.b, Color.a);

	glVertex2f(Rect.Min.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Max.y);

	glVertex2f(Rect.Min.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Max.y);
	glVertex2f(Rect.Min.x, Rect.Max.y);

	glEnd();
}

void OpenGLSetScreenspace(int Width, int Height) {
	
	float a = 2.0f / (float)Width;
	float b = -2.0f / (float)Height;

	float c = 2.0f / 10000.0f;

	float ProjMatrix[] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		-1.0f, 1.0f, -1.0f, 1.0f
	};

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(ProjMatrix);
}

static void OpenGLUniformCameraSetup(
	GLuint ProgramHandle,
	GLuint ViewLocation,
	GLuint ProjLocation,
	GLuint CameraPLocation,
	game_camera_setup* CameraSetup)
{
	glUseProgram(ProgramHandle);

	glUniformMatrix4fv(ProjLocation, 1, GL_TRUE, CameraSetup->ProjectionMatrix.E);
	glUniformMatrix4fv(ViewLocation, 1, GL_TRUE, CameraSetup->ViewMatrix.E);
	glUniform3f(
		CameraPLocation,
		CameraSetup->Camera.Position.x,
		CameraSetup->Camera.Position.y,
		CameraSetup->Camera.Position.z);

	glUseProgram(0);
}

static void OpenGLRenderStackToOutput(gl_state* GLState, render_stack* Stack) {
	font_info* CurrentFontInfo = 0;

	//NOTE(dima): Iteration through render stack
	u8* At = (u8*)Stack->Data.BaseAddress;
	u8* StackEnd = (u8*)Stack->Data.BaseAddress + Stack->Data.Used;

	while (At < StackEnd) {
		render_stack_entry_header* Header = (render_stack_entry_header*)At;

		u32 SizeOfEntryType = Header->SizeOfEntryType;

		At += sizeof(render_stack_entry_header);
		switch (Header->Type) {
			case(RenderEntry_Bitmap): {
				render_stack_entry_bitmap* EntryBitmap = (render_stack_entry_bitmap*)At;

				OpenGLRenderBitmap(GLState, EntryBitmap->Bitmap, EntryBitmap->P, EntryBitmap->Dim, EntryBitmap->ModulationColor);
			}break;

			case(RenderEntry_Clear): {
				render_stack_entry_clear* EntryClear = (render_stack_entry_clear*)At;

				glClearColor(
					EntryClear->Color.r,
					EntryClear->Color.g,
					EntryClear->Color.b,
					1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}break;

			case(RenderEntry_Gradient): {
				render_stack_entry_gradient* EntryGrad = (render_stack_entry_gradient*)At;

				//TODO(DIMA): 
			}break;

			case(RenderEntry_Rectangle): {
				render_stack_entry_rectangle* EntryRect = (render_stack_entry_rectangle*)At;

				OpenGLRenderRectangle(Rect2MinDim(EntryRect->P, EntryRect->Dim), EntryRect->ModulationColor);
			}break;

			case RenderEntry_GUI_Glyph: {
				render_stack_entry_glyph* EntryGlyph = (render_stack_entry_glyph*)At;

				if (CurrentFontInfo) {
					glyph_info* Glyph = &CurrentFontInfo->Glyphs[CurrentFontInfo->CodepointToGlyphMapping[EntryGlyph->Codepoint]];

					v4 Color = EntryGlyph->ModulationColor;
					rect2 Rect = Rect2MinDim(EntryGlyph->P, EntryGlyph->Dim);
					v2 MinUV = Glyph->AtlasMinUV;
					v2 MaxUV = Glyph->AtlasMaxUV;

					glColor4f(Color.r, Color.g, Color.b, Color.a);

					float Depth = 10000.0f;

					glTexCoord2f(MinUV.x, MinUV.y);
					glVertex3f(Rect.Min.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MinUV.y);
					glVertex3f(Rect.Max.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MaxUV.y);
					glVertex3f(Rect.Max.x, Rect.Max.y, Depth);

					glTexCoord2f(MinUV.x, MinUV.y);
					glVertex3f(Rect.Min.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MaxUV.y);
					glVertex3f(Rect.Max.x, Rect.Max.y, Depth);
					glTexCoord2f(MinUV.x, MaxUV.y);
					glVertex3f(Rect.Min.x, Rect.Max.y, Depth);
				}
			}break;

			case RenderEntry_GUI_BeginText: {
				render_stack_entry_begin_text* EntryBeginText = (render_stack_entry_begin_text*)At;

				CurrentFontInfo = EntryBeginText->FontInfo;
				bitmap_info* Buffer = &CurrentFontInfo->FontAtlasImage;

				if (!Buffer->TextureHandle) {
					OpenGLAllocateTexture(Buffer, 0, GLState);
				}
				glBindTexture(GL_TEXTURE_2D, (GLuint)Buffer->TextureHandle);
				glBegin(GL_TRIANGLES);

			}break;

			case RenderEntry_GUI_EndText: {
				render_stack_entry_end_text* EntryEndText = (render_stack_entry_end_text*)At;

				CurrentFontInfo = 0;

				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);
			}break;

			case RenderEntry_Mesh: {
				render_stack_entry_mesh* EntryMesh = (render_stack_entry_mesh*)At;

				mesh_info* MeshInfo = EntryMesh->MeshInfo;

				gl_wtf_shader* Shader = &GLState->WtfShader;

				if (!MeshInfo->Handle) {
					GLuint EBO, VBO, VAO;

					glGenVertexArrays(1, &VAO);
					glGenBuffers(1, &VBO);
					glGenBuffers(1, &EBO);

					/*
					NOTE(dima): We need to be accurate here
					because of the alignment..
					*/
					u32 OneVertexSize = sizeof(vertex_info);

					glBindVertexArray(VAO);

					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferData(
						GL_ARRAY_BUFFER,
						MeshInfo->VerticesCount * OneVertexSize,
						MeshInfo->Vertices,
						GL_STATIC_DRAW);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
					glBufferData(
						GL_ELEMENT_ARRAY_BUFFER,
						MeshInfo->IndicesCount * sizeof(u32),
						MeshInfo->Indices,
						GL_STATIC_DRAW);

					if (OpenGLArrayIsValid(Shader->PositionIndex)) {
						glEnableVertexAttribArray(Shader->PositionIndex);
						u32 POffset = offsetof(vertex_info, P);
						glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)POffset);
					}

					if (OpenGLArrayIsValid(Shader->NormalIndex)) {
						glEnableVertexAttribArray(Shader->NormalIndex);
						u32 NOffset = offsetof(vertex_info, N);
						glVertexAttribPointer(Shader->NormalIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)NOffset);
					}

					if (OpenGLArrayIsValid(Shader->UVIndex)) {
						glEnableVertexAttribArray(Shader->UVIndex);
						u32 UVOffset = offsetof(vertex_info, UV);
						glVertexAttribPointer(Shader->UVIndex, 2, GL_FLOAT, 0, OneVertexSize, (void*)UVOffset);
					}

					if (OpenGLArrayIsValid(Shader->TangentIndex)) {
						glEnableVertexAttribArray(Shader->TangentIndex);
						u32 TOffset = offsetof(vertex_info, T);
						glVertexAttribPointer(Shader->TangentIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)TOffset);
					}

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);

					MeshInfo->Handle = (void*)VAO;
				}
				glEnable(GL_DEPTH_TEST);
				glUseProgram(GLState->WtfShader.Program.Handle);

				OpenGLUniformSurfaceMaterial(GLState, Stack, &GLState->WtfShader, &EntryMesh->Material);

				glBindVertexArray((GLuint)MeshInfo->Handle);
				glUniformMatrix4fv(GLState->WtfShader.ModelMatrixLocation, 1, GL_TRUE, EntryMesh->TransformMatrix.E);
				glDrawElements(GL_TRIANGLES, MeshInfo->IndicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				glUseProgram(0);
				glDisable(GL_DEPTH_TEST);
			}break;

			case RenderEntry_VoxelMesh: {
				render_stack_entry_voxel_mesh* EntryVoxelMesh = (render_stack_entry_voxel_mesh*)At;

				gl_voxel_shader* Shader = &GLState->VoxelShader;
				voxel_mesh_info* Mesh = EntryVoxelMesh->MeshInfo;

				mat4 ModelTransform = Translate(Identity(), EntryVoxelMesh->P);

				if (Mesh->State == VoxelMeshState_Generated) {

					u32 TextureToBind = 0;
					if (EntryVoxelMesh->VoxelAtlasBitmap) {
						if (EntryVoxelMesh->VoxelAtlasBitmap->TextureHandle) {
							TextureToBind = (u32)EntryVoxelMesh->VoxelAtlasBitmap->TextureHandle;
						}
						else {
							TextureToBind = OpenGLAllocateTexture(
								EntryVoxelMesh->VoxelAtlasBitmap,
								TextureAllocation_NearestFiltering | TextureAllocation_EnableAnisotropic,
								GLState);
						}
					}

					if (!Mesh->MeshHandle) {
						BeginMutexAccess(&Mesh->MeshUseMutex);
						GLuint MeshVAO;
						GLuint MeshVBO;

						glGenVertexArrays(1, &MeshVAO);
						glGenBuffers(1, &MeshVBO);

						glBindVertexArray(MeshVAO);

						glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
						glBufferData(GL_ARRAY_BUFFER,
							Mesh->VerticesCount * sizeof(u32),
							&Mesh->Vertices[0], GL_DYNAMIC_DRAW);

						if (OpenGLArrayIsValid(Shader->VertexDataIndex)) {
							glEnableVertexAttribArray(Shader->VertexDataIndex);
							glVertexAttribPointer(Shader->VertexDataIndex, 1, GL_FLOAT, GL_FALSE, 4, 0);
						}

						glBindBuffer(GL_ARRAY_BUFFER, 0);
						glBindVertexArray(0);

						Mesh->MeshHandle = (void*)MeshVAO;
						Mesh->MeshHandle2 = (void*)MeshVBO;
						EndMutexAccess(&Mesh->MeshUseMutex);
					}

					glEnable(GL_DEPTH_TEST);
					glUseProgram((u32)Shader->Program.Handle);

					_glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, TextureToBind);
					glUniform1i(Shader->DiffuseMapLocation, 0);

					glUniformMatrix4fv(
						Shader->ModelMatrixLocation, 1, GL_TRUE, ModelTransform.E);

					glBindVertexArray((u32)Mesh->MeshHandle);
					glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
					glBindVertexArray(0);

					glUseProgram(0);
					glDisable(GL_DEPTH_TEST);
				}
			}break;

			case RenderEntry_LpterWaterMesh: {
				render_stack_entry_lpter_water_mesh* Entry = (render_stack_entry_lpter_water_mesh*)At;

				lpter_water* Water = Entry->WaterMesh;
				gl_lpter_water_shader* WaterShader = &GLState->LpterWaterShader;

				mat4 ModelTransform = Translate(Identity(), Entry->P);

				if (!Water->MeshHandle1) {
					GLuint MeshVAO;
					GLuint MeshVBO;

					glGenVertexArrays(1, &MeshVAO);
					glGenBuffers(1, &MeshVBO);

					glBindVertexArray(MeshVAO);
					glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
					glBufferData(GL_ARRAY_BUFFER, sizeof(Water->Vertices), Water->Vertices, GL_DYNAMIC_DRAW);

					if (OpenGLArrayIsValid(WaterShader->PositionIndex)) {
						glEnableVertexAttribArray(WaterShader->PositionIndex);
						glVertexAttribPointer(
							WaterShader->PositionIndex,
							2, GL_FLOAT,
							0,
							sizeof(lpter_water_vertex),
							(void*)offsetof(lpter_water_vertex, VertexXZ));
					}

					if (OpenGLArrayIsValid(WaterShader->OffsetsToOthersIndex)) {
						glEnableVertexAttribArray(WaterShader->OffsetsToOthersIndex);
						glVertexAttribIPointer(
							WaterShader->OffsetsToOthersIndex,
							4, GL_BYTE,
							sizeof(lpter_water_vertex),
							(void*)offsetof(lpter_water_vertex, OffsetsToOtherVerts));
					}

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);

					Water->MeshHandle1 = (void*)MeshVAO;
					Water->MeshHandle2 = (void*)MeshVBO;
				}

				glEnable(GL_DEPTH_TEST);
				WaterShader->Program.Use();

				glUniformMatrix4fv(WaterShader->ModelMatrixLocation,
					1, GL_TRUE, ModelTransform.E);

				glUniform1f(WaterShader->WaterLevelLocation, Water->WaterLevel);
				glUniform1f(WaterShader->PerVertexOfffsetLocation, Water->PerVertexOffset);

				glBindVertexArray((u32)Water->MeshHandle1);
				glDrawArrays(GL_TRIANGLES, 0, Water->VerticesCount);
				glBindVertexArray(0);

				glUseProgram(0);
				glDisable(GL_DEPTH_TEST);
			}break;

			case RenderEntry_LpterMesh: {
#if 1
				render_stack_entry_lpter_mesh* EntryMesh = (render_stack_entry_lpter_mesh*)At;

				lpter_mesh* Mesh = EntryMesh->Mesh;
				gl_lpter_shader* Shader = &GLState->LpterShader;

				mat4 ModelTransform = Translate(Identity(), EntryMesh->P);

				if (!Mesh->MeshHandle0) {
					GLuint MeshVAO;
					GLuint MeshVBO;
					GLuint TexNormBO;

					glGenVertexArrays(1, &MeshVAO);
					glGenBuffers(1, &MeshVBO);
					glGenBuffers(1, &TexNormBO);

					glBindVertexArray(MeshVAO);

					glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
					glBufferData(GL_ARRAY_BUFFER,
						Mesh->VertsCount * sizeof(lpter_vertex),
						&Mesh->Verts[0], GL_DYNAMIC_DRAW);
					glBindBuffer(GL_TEXTURE_BUFFER, TexNormBO);
					glBufferData(GL_TEXTURE_BUFFER, sizeof(Mesh->Normals), &Mesh->Normals[0], GL_STATIC_DRAW);

					GLuint NormalsBufTexture;
					glGenTextures(1, &NormalsBufTexture);
					_glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_BUFFER, NormalsBufTexture);
					glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TexNormBO);

					if (OpenGLArrayIsValid(Shader->PositionIndex)) {
						glEnableVertexAttribArray(Shader->PositionIndex);
						glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(lpter_vertex), (void*)offsetof(lpter_vertex, P));
					}

					if (OpenGLArrayIsValid(Shader->ColorIndex)) {
						glEnableVertexAttribArray(Shader->ColorIndex);
						glVertexAttribIPointer(Shader->ColorIndex, 1, GL_UNSIGNED_INT, sizeof(lpter_vertex), (void*)offsetof(lpter_vertex, Color));
					}

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);

					Mesh->MeshHandle0 = (void*)MeshVAO;
					Mesh->MeshHandle1 = (void*)MeshVBO;
					Mesh->MeshHandleTexBuf = (void*)TexNormBO;
					Mesh->NormTexHandle = (void*)NormalsBufTexture;
				}

				glEnable(GL_DEPTH_TEST);
				Shader->Program.Use();

				glUniformMatrix4fv(
					Shader->ModelMatrixLocation, 1, GL_TRUE, ModelTransform.E);

				_glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_BUFFER, (u32)Mesh->NormTexHandle);
				glUniform1i(Shader->NormalsBufferLocation, 0);

				glBindVertexArray((u32)Mesh->MeshHandle0);
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays(GL_TRIANGLES, 0, Mesh->VertsCount);
				glBindVertexArray(0);

				glUseProgram(0);
				glDisable(GL_DEPTH_TEST);
#endif
			}break;

			case RenderEntry_Test: {

			}break;

			default: {
				Assert(!"Invalid entry type");
			}break;
		}

		At += Header->SizeOfEntryType;
	}

}

void OpenGLRenderStateToOutput(gl_state* GLState, render_state* RenderState, game_settings* GameSettings) {
	FUNCTION_TIMING();

	glEnable(GL_MULTISAMPLE);

	GLuint LastWriteFBO;
	GLuint LastReadFBO;

#if 0
	if (IsGUIRenderStack) {
		glBindFramebuffer(GL_FRAMEBUFFER, GLState->FramebufferGUI.FBO);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		int ClearingValues[4] = { 0, 0, 0, 0 };
		glClearBufferiv(GL_COLOR, 0, ClearingValues);
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);

	}
	else {
		LastWriteFBO = GLState->FramebufferInitial.FBO;
		glBindFramebuffer(GL_FRAMEBUFFER, LastWriteFBO);
	
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		//glEnable(GL_CULL_FACE);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		OpenGLSetScreenspace(RenderState->RenderWidth, RenderState->RenderHeight);
	
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
#else
	LastWriteFBO = GLState->FramebufferInitial.FBO;
	glBindFramebuffer(GL_FRAMEBUFFER, LastWriteFBO);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLSetScreenspace(RenderState->RenderWidth, RenderState->RenderHeight);

	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
 
	game_camera_setup* CameraSetup = &RenderState->CameraSetup;

	OpenGLUniformCameraSetup(
		GLState->WtfShader.Program.Handle,
		GLState->WtfShader.ViewMatrixLocation,
		GLState->WtfShader.ProjectionMatrixLocation,
		GLState->WtfShader.CameraPLocation,
		CameraSetup);

	OpenGLUniformCameraSetup(
		GLState->VoxelShader.Program.Handle,
		GLState->VoxelShader.ViewMatrixLocation,
		GLState->VoxelShader.ProjectionMatrixLocation,
		GLState->VoxelShader.CameraPLocation,
		CameraSetup);

	OpenGLUniformCameraSetup(
		GLState->LpterShader.Program.Handle,
		GLState->LpterShader.ViewMatrixLocation,
		GLState->LpterShader.ProjectionMatrixLocation,
		GLState->VoxelShader.CameraPLocation,
		CameraSetup);

	OpenGLUniformCameraSetup(
		GLState->LpterWaterShader.Program.Handle,
		GLState->LpterWaterShader.ViewMatrixLocation,
		GLState->LpterWaterShader.ProjectionMatrixLocation,
		GLState->LpterWaterShader.CameraPLocation,
		CameraSetup);
	glUniform1f(
		GLState->LpterWaterShader.GlobalTimeLocation,
		RenderState->InputSystem->Time);
	glUseProgram(0);

	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//NOTE(dima): Resolving multisampled buffer to temp buffer
	if (AntialiasingIsMSAA(GLState->AntialiasingType)) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, LastWriteFBO);
		LastWriteFBO = GLState->FramebufferResolved.FBO;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LastWriteFBO);

		glBlitFramebuffer(
			0, 0,
			RenderState->RenderWidth,
			RenderState->RenderHeight,
			0, 0,
			RenderState->RenderWidth,
			RenderState->RenderHeight,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			GL_NEAREST);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, LastWriteFBO);
	LastWriteFBO = GLState->FramebufferPFX.FBO;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LastWriteFBO);

	glBlitFramebuffer(
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);

	//NOTE(dima): FXAA antialiasing
	GLState->FXAAEnabled = GameSettings->Named.FXAAEnabledSetting->BoolValue;
	if (GLState->FXAAEnabled) {
		BEGIN_TIMING("FXAA");
		glBindFramebuffer(GL_FRAMEBUFFER, GLState->FramebufferPFX.FBO);

		OpenGLUseProgramBegin(&GLState->FXAAShader.Program);
		glBindVertexArray(GLState->ScreenQuadVAO);
		_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GLState->FramebufferPFX.Texture);
		glUniform1i(GLState->FXAAShader.TextureLocation, 0);
		glUniform2f(
			GLState->FXAAShader.TextureSizeLocation,
			RenderState->RenderWidth,
			RenderState->RenderHeight);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		OpenGLUseProgramEnd();
		END_TIMING();
	}

	//NOTE(dima): Finalizing screen shader
	BEGIN_TIMING("Final shader postprocess");

	glBindFramebuffer(GL_FRAMEBUFFER, GLState->FramebufferPFX.FBO);

	OpenGLUseProgramBegin(&GLState->ScreenShader.Program);
	glBindVertexArray(GLState->ScreenQuadVAO);
	_glActiveTexture(GL_TEXTURE0);
	glUniform1i(GLState->ScreenShader.ScreenTextureLocation, 0);
	glBindTexture(GL_TEXTURE_2D, GLState->FramebufferPFX.Texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	OpenGLUseProgramEnd();
	END_TIMING();

	//NOTE(dima): Blitting to default framebuffer
	BEGIN_TIMING("Blitting to default FBO");
	glBindFramebuffer(GL_READ_FRAMEBUFFER, LastWriteFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);
	END_TIMING();
	
		//NOTE(dima): Render stack is GUI render stack

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//GLState->ScreenShader.Program.Use();
		//glBindVertexArray(GLState->ScreenQuadVAO);
		//_glActiveTexture(GL_TEXTURE0);
		//glUniform1i(GLState->ScreenShader.ScreenTextureLocation, 0);
		//glBindTexture(GL_TEXTURE_2D, GLState->FramebufferGUI.Texture);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//glBindVertexArray(0);
		//glUseProgram(0);
}


static void OpenGLInitFramebuffer(
	opengl_framebuffer* Framebuffer,
	int RenderWidth,
	int RenderHeight, 
	b32 AllocateDepthStencilAttachment,
	b32 EnableMultisampling = 0,
	b32 NumberOfSamples = 0, 
	u32 DepthStencilAttachmentType = 0)
{
	Framebuffer->DepthStencilAttachmentType = DepthStencilAttachmentType;

	//NOTE(dima): Initializing internal framebuffer
	glGenFramebuffers(1, &Framebuffer->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO);

	glGenTextures(1, &Framebuffer->Texture);
	if (EnableMultisampling) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer->Texture);
		glTexImage2DMultisample(
			GL_TEXTURE_2D_MULTISAMPLE,
			NumberOfSamples,
			GL_RGBA8,
			RenderWidth,
			RenderHeight,
			GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D_MULTISAMPLE,
			Framebuffer->Texture,
			0);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, Framebuffer->Texture);
		glTexImage2D(
			GL_TEXTURE_2D, 0,
			GL_RGBA,
			RenderWidth,
			RenderHeight,
			0,
			GL_ABGR_EXT,
			GL_UNSIGNED_BYTE,
			0);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			Framebuffer->Texture,
			0);
	}

	if (AllocateDepthStencilAttachment) {
		switch (DepthStencilAttachmentType) {
			case OpenGL_DS_Renderbuffer: {
				glGenRenderbuffers(1, &Framebuffer->DepthStencilRBO);
				glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer->DepthStencilRBO);
				if (EnableMultisampling) {
					glRenderbufferStorageMultisample(
						GL_RENDERBUFFER,
						NumberOfSamples,
						GL_DEPTH24_STENCIL8,
						RenderWidth,
						RenderHeight);
				}
				else {
					glRenderbufferStorage(
						GL_RENDERBUFFER,
						GL_DEPTH24_STENCIL8,
						RenderWidth,
						RenderHeight);
				}
				glBindRenderbuffer(GL_RENDERBUFFER, 0);

				glFramebufferRenderbuffer(
					GL_FRAMEBUFFER,
					GL_DEPTH_STENCIL_ATTACHMENT,
					GL_RENDERBUFFER,
					Framebuffer->DepthStencilRBO);
			}break;

			case OpenGL_DS_Texture: {
				if (EnableMultisampling) {
					glGenTextures(1, &Framebuffer->DepthStencilTexture);
					glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer->DepthStencilTexture);

					glTexImage2DMultisample(
						GL_TEXTURE_2D_MULTISAMPLE,
						NumberOfSamples,
						GL_DEPTH24_STENCIL8,
						RenderWidth,
						RenderHeight,
						GL_TRUE);

					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_DEPTH_STENCIL_ATTACHMENT,
						GL_TEXTURE_2D_MULTISAMPLE,
						Framebuffer->DepthStencilTexture,
						0);
				}
				else {
					glGenTextures(1, &Framebuffer->DepthStencilTexture);
					glBindTexture(GL_TEXTURE_2D, Framebuffer->DepthStencilTexture);

					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_DEPTH24_STENCIL8,
						RenderWidth,
						RenderHeight,
						0,
						GL_DEPTH_STENCIL,
						GL_UNSIGNED_INT_24_8,
						0);

					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_DEPTH_STENCIL_ATTACHMENT,
						GL_TEXTURE_2D,
						Framebuffer->DepthStencilTexture,
						0);
				}
			}break;
		}

	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Assert(!"Framebuffer should be complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void OpenGLInitMultisampleFramebuffer(
	opengl_framebuffer* Framebuffer,
	int RenderWidth,
	int RenderHeight,
	int NumberOfSamples)
{
	OpenGLInitFramebuffer(
		Framebuffer,
		RenderWidth,
		RenderHeight,
		true,
		true,
		NumberOfSamples);
}

static b32 OpenGLExtensionIsSupported(
	char* ExtensionName)
{
	b32 Result = 0;

	int ExtensionsCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionsCount);

	for (int ExtensionIndex = 0;
		ExtensionIndex < ExtensionsCount;
		ExtensionIndex++)
	{
		const GLubyte* Extension = glGetStringi(GL_EXTENSIONS, ExtensionIndex);

		if (StringsAreEqual((char*)Extension, ExtensionName)) {
			Result = 1;
			break;
		}
	}

	return(Result);
}

void OpenGLInitState(
	gl_state* State, 
	int RenderWidth, 
	int RenderHeight,
	game_settings* GameSettings) 
{
	*State = {};

	State->WtfShader = OpenGLLoadWtfShader();
	State->VoxelShader = OpenGLLoadVoxelShader();
	State->ScreenShader = OpenGLLoadScreenShader();
	State->FXAAShader = OpenGLLoadFXAAShader();
	State->LpterShader = OpenGLLoadLpterShader();
	State->LpterWaterShader = OpenGLLoadLpterWaterShader();

	//NOTE(dima): Checking for OpenGL extensions support
	//TODO(dima): Load parameters from game_settings
	State->AnisotropicFilteringSupported = OpenGLExtensionIsSupported("GL_EXT_texture_filter_anisotropic");
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &State->MaxAnisotropicLevel);

	State->AnisotropicLevelType = GameSettings->Named.AnisotropicLevelTypeSetting->IntegerValue;
	State->AntialiasingType = GameSettings->Named.AntialiasingTypeSetting->IntegerValue;
	State->FXAAEnabled = GameSettings->Named.FXAAEnabledSetting->BoolValue;

	State->AnisotropicLevel = GetAnisoLevelBasedOnParams(
		State->AnisotropicLevelType,
		State->MaxAnisotropicLevel);

	if (AntialiasingIsMSAA(State->AntialiasingType)) {
		State->MultisampleLevel = GetMSAALevel(State->AntialiasingType);
	}

	State->MultisamplingSupported =
		OpenGLExtensionIsSupported("GL_ARB_multisample") ||
		OpenGLExtensionIsSupported("GLX_ARB_multisample") ||
		OpenGLExtensionIsSupported("WGL_ARB_multisample");
	glGetIntegerv(GL_MAX_SAMPLES, &State->MaxMultisampleLevel);


	//NOTE(dima): Initializing of screen quad. Pos + Tex
#if 1
	float ScreenQuadFloats[] = {
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
	};
#else
	float ScreenQuadFloats[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	};
#endif

	glGenVertexArrays(1, &State->ScreenQuadVAO);
	glGenBuffers(1, &State->ScreenQuadVBO);

	glBindVertexArray(State->ScreenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, State->ScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenQuadFloats), ScreenQuadFloats, GL_STATIC_DRAW);
	if (OpenGLArrayIsValid(State->ScreenShader.PosIndex)) {
		glEnableVertexAttribArray(State->ScreenShader.PosIndex);
		glVertexAttribPointer(State->ScreenShader.PosIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
	if (OpenGLArrayIsValid(State->ScreenShader.TexIndex)) {
		glEnableVertexAttribArray(State->ScreenShader.TexIndex);
		glVertexAttribPointer(State->ScreenShader.TexIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

	if (OpenGLArrayIsValid(State->FXAAShader.PosIndex)) {
		glEnableVertexAttribArray(State->FXAAShader.PosIndex);
		glVertexAttribPointer(State->FXAAShader.PosIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
	if (OpenGLArrayIsValid(State->FXAAShader.TexCoordIndex)) {
		glEnableVertexAttribArray(State->FXAAShader.TexCoordIndex);
		glVertexAttribPointer(State->FXAAShader.TexCoordIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}
	glBindVertexArray(0);

	//NOTE(dima): Initialization of framebuffer objects
	if (State->MultisamplingSupported && AntialiasingIsMSAA(State->AntialiasingType)) {
		OpenGLInitMultisampleFramebuffer(&State->FramebufferInitial, RenderWidth, RenderHeight, State->MultisampleLevel);
		OpenGLInitFramebuffer(&State->FramebufferResolved, RenderWidth, RenderHeight, GL_TRUE);
	}
	else {
		OpenGLInitFramebuffer(&State->FramebufferInitial, RenderWidth, RenderHeight, GL_TRUE);
	}
	
	OpenGLInitFramebuffer(&State->FramebufferPFX, RenderWidth, RenderHeight, GL_FALSE);

	OpenGLInitFramebuffer(&State->FramebufferGUI, RenderWidth, RenderHeight, GL_TRUE);
}

static void OpenGLFramebufferCleanup(opengl_framebuffer* Framebuffer) {
	glDeleteTextures(1, &Framebuffer->Texture);
	switch (Framebuffer->DepthStencilAttachmentType) {
		case OpenGL_DS_Renderbuffer: {
			glDeleteRenderbuffers(1, &Framebuffer->DepthStencilRBO);
		}break;

		case OpenGL_DS_Texture: {
			glDeleteTextures(1, &Framebuffer->DepthStencilTexture);
		}break;
	}
	glDeleteFramebuffers(1, &Framebuffer->FBO);
}

void OpenGLCleanup(gl_state* GLState) {
	OpenGLFramebufferCleanup(&GLState->FramebufferGUI);
	OpenGLFramebufferCleanup(&GLState->FramebufferInitial);
	OpenGLFramebufferCleanup(&GLState->FramebufferPFX);
	OpenGLFramebufferCleanup(&GLState->FramebufferResolved);

	OpenGLCleanupShader(&GLState->WtfShader.Program);
	OpenGLCleanupShader(&GLState->VoxelShader.Program);
	OpenGLCleanupShader(&GLState->FXAAShader.Program);
	OpenGLCleanupShader(&GLState->ScreenShader.Program);
	OpenGLCleanupShader(&GLState->LpterShader.Program);
}

void OpenGLProcessAllocationQueue() {

	FUNCTION_TIMING();

	dealloc_queue_entry* FirstEntry = 0;
	dealloc_queue_entry* LastEntry = 0;

	BeginMutexAccess(&PlatformApi.DeallocQueueMutex);
	
	//NOTE(dima): If there is something to allocate...
	if (PlatformApi.FirstUseAllocQueueEntry->Next != PlatformApi.FirstUseAllocQueueEntry) 
	{
		FirstEntry = PlatformApi.FirstUseAllocQueueEntry->Next;
		LastEntry = PlatformApi.FirstUseAllocQueueEntry->Prev;

#if 0
		PlatformApi.FirstUseAllocQueueEntry->Next = PlatformApi.FirstUseAllocQueueEntry;
		PlatformApi.FirstUseAllocQueueEntry->Prev = PlatformApi.FirstUseAllocQueueEntry;

 		//FirstEntry->Prev->Next = PlatformApi.FirstUseAllocQueueEntry;
		//LastEntry->Next->Prev = PlatformApi.FirstUseAllocQueueEntry;
		
		FirstEntry->Prev = PlatformApi.FirstFreeAllocQueueEntry;
		LastEntry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		
		FirstEntry->Prev->Next = FirstEntry;
		LastEntry->Next->Prev = LastEntry;
#endif
	}

	EndMutexAccess(&PlatformApi.DeallocQueueMutex);

	for (dealloc_queue_entry* Entry = FirstEntry;
		Entry;)
	{
		dealloc_queue_entry* NextEntry = Entry->Next;

		switch (Entry->EntryType) {
			case DeallocQueueEntry_Bitmap: {
				GLuint TextureHandle = (GLuint)Entry->Data.BitmapData.TextureHandle;

				glDeleteTextures(1, &TextureHandle);
			}break;

			case DeallocQueueEntry_VoxelMesh: {
				GLuint MeshVBO = (GLuint)Entry->Data.VoxelMeshData.Handle2;
				GLuint MeshVAO = (GLuint)Entry->Data.VoxelMeshData.Handle1;

				glDeleteVertexArrays(1, &MeshVAO);
				glDeleteBuffers(1, &MeshVBO);
			}break;
		}

#if 1
		BeginMutexAccess(&PlatformApi.DeallocQueueMutex);
		
		Entry->Next->Prev = Entry->Prev;
		Entry->Prev->Next = Entry->Next;

		Entry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		Entry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

		Entry->Next->Prev = Entry;
		Entry->Prev->Next = Entry;

		EndMutexAccess(&PlatformApi.DeallocQueueMutex);
#endif

		if (Entry == LastEntry) {
			break;
		}

		Entry = NextEntry;
	}
}
//