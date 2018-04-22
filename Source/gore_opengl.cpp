#include "gore_opengl.h"

GLuint OpenGLAllocateTexture(bitmap_info* Buffer) {
	GLuint TextureHandle;
	glGenTextures(1, &TextureHandle);

	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
		DEBUG_ERROR_LOG("Vert shader load error");
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

gl_wtf_shader OpenGLLoadWtfShader() {
	gl_wtf_shader Result = {};
	
	char* VertexPath = "../Data/Shaders/WtfShader.vert";
	char* FragmentPath = "../Data/Shaders/WtfShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PositionIndex = glGetAttribLocation(Result.Program.Handle, "Position");
	Result.NormalIndex = glGetAttribLocation(Result.Program.Handle, "Normal");
	Result.UVIndex = glGetAttribLocation(Result.Program.Handle, "UV");
	Result.ColorIndex = glGetAttribLocation(Result.Program.Handle, "Color");
	Result.TangentIndex = glGetAttribLocation(Result.Program.Handle, "Tangent");

	Result.ModelMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Model");
	Result.ViewMatrixLocation = glGetUniformLocation(Result.Program.Handle, "View");
	Result.ProjectionMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Projection");
	Result.CameraPLocation = glGetUniformLocation(Result.Program.Handle, "CameraP");

	Result.SurfMatShineLocation = glGetUniformLocation(Result.Program.Handle, "Material.Shine");
	Result.SurfMatColorLocation = glGetUniformLocation(Result.Program.Handle, "Material.Color");

	Result.SurfMatDiffLocation = glGetUniformLocation(Result.Program.Handle, "Material.Diffuse");
	Result.SurfMatSpecLocation = glGetUniformLocation(Result.Program.Handle, "Material.Specular");
	Result.SurfMatEmisLocation = glGetUniformLocation(Result.Program.Handle, "Material.Emissive");

	Result.SurfMatHasDiffLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasDiffuse");
	Result.SurfMatHasSpecLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasSpecular");
	Result.SurfMatHasEmisLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasEmissive");

	return(Result);
}

void OpenGLUniformSurfaceMaterial(render_state* State, gl_wtf_shader* Shader, surface_material* Mat) {
	glUniform1f(Shader->SurfMatShineLocation, Mat->Shine);
	glUniform3f(Shader->SurfMatColorLocation, Mat->Color.x, Mat->Color.y, Mat->Color.z);

	glUniform1i(Shader->SurfMatHasDiffLocation, Mat->Diffuse ? 1 : 0);
	if (Mat->Diffuse) {
		bitmap_info* Info = GetBitmapFromID(State->AssetSystem, Mat->Diffuse);
		if (!Info->TextureHandle) {
			OpenGLAllocateTexture(Info);
		}

		_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		//glUniform1i(Shader->SurfMatDiffLocation, (GLint)Info->TextureHandle);
	}

	glUniform1i(Shader->SurfMatHasSpecLocation, Mat->Specular ? 1 : 0);
	if (Mat->Specular) {
		bitmap_info* Info = GetBitmapFromID(State->AssetSystem, Mat->Specular);
		if (!Info->TextureHandle) {
			OpenGLAllocateTexture(Info);
		}

		_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		//glUniform1i(Shader->SurfMatSpecLocation, (GLuint)Info->TextureHandle);
	}

	glUniform1i(Shader->SurfMatHasEmisLocation, Mat->Emissive ? 1 : 0);
	if (Mat->Emissive) {
		bitmap_info* Info = GetBitmapFromID(State->AssetSystem, Mat->Emissive);
		if (!Info->TextureHandle) {
			OpenGLAllocateTexture(Info);
		}

		_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		//glUniform1i(Shader->SurfMatEmisLocation, (GLuint)Info->TextureHandle);
	}
}

inline void OpenGLUseProgramBegin(gl_program* Program) {
	glUseProgram(Program->Handle);
}

inline void OpenGLUseProgramEnd(gl_program* Program) {
	glUseProgram(0);
}

void OpenGLRenderBitmap(bitmap_info* Buffer, v2 P, v2 Dim, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 Rect = Rect2MinDim(P, Dim);

	if (!Buffer->TextureHandle) {
		OpenGLAllocateTexture(Buffer);
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

void OpenGLRenderStackToOutput(gl_state* GLState, render_state* RenderState) {
	FUNCTION_TIMING();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLSetScreenspace(RenderState->RenderWidth, RenderState->RenderHeight);

	//glClearColor(0.08f, 0.08f, 0.15f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//NOTE(dima): Iteration through render stack
	u8* At = (u8*)RenderState->Data.BaseAddress;
	u8* StackEnd = (u8*)RenderState->Data.BaseAddress + RenderState->Data.Used;

	game_camera_setup* CameraSetup = &RenderState->CameraSetup;
	glUseProgram(GLState->WtfShader.Program.Handle);
	glUniformMatrix4fv(GLState->WtfShader.ProjectionMatrixLocation, 1, GL_TRUE, CameraSetup->ProjectionMatrix.E);
	glUniformMatrix4fv(GLState->WtfShader.ViewMatrixLocation, 1, GL_TRUE, CameraSetup->ViewMatrix.E);
	glUniform3f(GLState->WtfShader.CameraPLocation,
		RenderState->CameraSetup.Camera.Position.x,
		RenderState->CameraSetup.Camera.Position.y,
		RenderState->CameraSetup.Camera.Position.z);
	glUseProgram(0);

	font_info* CurrentFontInfo = 0;

	while (At < StackEnd) {
		render_stack_entry_header* Header = (render_stack_entry_header*)At;

		u32 SizeOfEntryType = Header->SizeOfEntryType;

		At += sizeof(render_stack_entry_header);
		switch (Header->Type) {
			case(RenderEntry_Bitmap): {
				render_stack_entry_bitmap* EntryBitmap = (render_stack_entry_bitmap*)At;

				OpenGLRenderBitmap(EntryBitmap->Bitmap, EntryBitmap->P, EntryBitmap->Dim, EntryBitmap->ModulationColor);
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

			case RenderEntry_Glyph: {
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

			case RenderEntry_BeginText: {
				render_stack_entry_begin_text* EntryBeginText = (render_stack_entry_begin_text*)At;

				CurrentFontInfo = EntryBeginText->FontInfo;
				bitmap_info* Buffer = &CurrentFontInfo->FontAtlasImage;

				if (!Buffer->TextureHandle) {
					OpenGLAllocateTexture(Buffer);
				}
				glBindTexture(GL_TEXTURE_2D, (GLuint)Buffer->TextureHandle);
				glBegin(GL_TRIANGLES);

			}break;

			case RenderEntry_EndText: {
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

					if (OpenGLArrayIsValid(Shader->ColorIndex)) {
						glEnableVertexAttribArray(Shader->ColorIndex);
						u32 COffset = offsetof(vertex_info, C);
						glVertexAttribPointer(Shader->ColorIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)COffset);
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

				glUseProgram(GLState->WtfShader.Program.Handle);

				glEnable(GL_DEPTH_TEST);

				OpenGLUniformSurfaceMaterial(RenderState, &GLState->WtfShader, EntryMesh->Material);

				glBindVertexArray((GLuint)MeshInfo->Handle);
				glUniformMatrix4fv(GLState->WtfShader.ModelMatrixLocation, 1, GL_TRUE, EntryMesh->TransformMatrix.E);
				glDrawElements(GL_TRIANGLES, MeshInfo->IndicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				glDisable(GL_DEPTH_TEST);

				glUseProgram(0);
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

void OpenGLInitState(gl_state* State) {
	*State = {};

	State->WtfShader = OpenGLLoadWtfShader();
}