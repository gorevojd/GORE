#include "gore_opengl.h"

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
	Result.SurfMatDiffLocation = glGetUniformLocation(Result.Program.Handle, "Material.Diffuse");
	Result.SurfMatSpecLocation = glGetUniformLocation(Result.Program.Handle, "Material.Specular");
	Result.SurfMatEmisLocation = glGetUniformLocation(Result.Program.Handle, "Material.Emissive");

	return(Result);
}

void OpenGLUniformSurfaceMaterial(gl_wtf_shader* Shader, surface_material Mat) {
	glUniform1f(Shader->SurfMatShineLocation, Mat.Shine);
	glUniform1i(Shader->SurfMatDiffLocation, 0);
	glUniform1i(Shader->SurfMatSpecLocation, 0);
	glUniform1i(Shader->SurfMatEmisLocation, 0);
}

inline void OpenGLUseProgramBegin(gl_program* Program) {
	glUseProgram(Program->Handle);
}

inline void OpenGLUseProgramEnd(gl_program* Program) {
	glUseProgram(0);
}

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

void OpenGLRenderBitmap(bitmap_info* Buffer, v2 P, v2 Dim, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 Rect = Rect2MinDim(P, Dim);

	if (!Buffer->TextureHandle) {
		OpenGLAllocateTexture(Buffer);
	}

	//glActiveTexture(GL_TEXTURE0);
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

void OpenGLRenderCube(gl_state* GLState, gl_wtf_shader* Shader, v3 Pos) {

	surface_material TempSurfMat = LITCreateSurfaceMaterial(32.0f);

	glUseProgram(Shader->Program.Handle);
	glBindVertexArray(GLState->CubeVAO);

	OpenGLUniformSurfaceMaterial(Shader, TempSurfMat);

	v3 ViewPos = V3(0.0f, 0.0f, 10.0f);
	v3 ViewDir = V3(0.0f, 0.0f, -1.0f);

	mat4 ModelMatrix = Identity();
	ModelMatrix = Translate(ModelMatrix, Pos);
	//ModelMatrix = Translate(ModelMatrix, V3(0.0f, 0.0f, -0.001f));
	glUniformMatrix4fv(Shader->ModelMatrixLocation, 1, GL_TRUE, ModelMatrix.E);

	//NOTE(dima): Rendering
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	//glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glUseProgram(0);
}

void OpenGLTempRenderPlane(gl_state* GLState, gl_wtf_shader* Shader) {
	
	glUseProgram(Shader->Program.Handle);
	glBindVertexArray(GLState->PlaneVAO);

	v3 ViewPos = V3(0.0f, 0.0f, 10.0f);
	v3 ViewDir = V3(0.0f, 0.0f, -1.0f);

	surface_material TempSurfMat = LITCreateSurfaceMaterial(32.0f);
	OpenGLUniformSurfaceMaterial(Shader, TempSurfMat);

	mat4 ModelMatrix = Identity();
	//ModelMatrix = Translate(ModelMatrix, V3(0.0f, 0.0f, -0.001f));
	float ScaleValue = 100.0f;
	ModelMatrix = ModelMatrix * ScalingMatrix(V3(ScaleValue, ScaleValue, ScaleValue));

	glUniformMatrix4fv(Shader->ModelMatrixLocation, 1, GL_TRUE, ModelMatrix.E);

	//NOTE(dima): Rendering
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
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

					u32 OneVertexSize = sizeof(vertex_info);
					u32 ComponentCount = sizeof(vertex_info) / 4;

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
				glBindVertexArray((GLuint)MeshInfo->Handle);
				glUniformMatrix4fv(GLState->WtfShader.ModelMatrixLocation, 1, GL_TRUE, EntryMesh->TransformMatrix.E);
				glDrawElements(GL_TRIANGLES, MeshInfo->IndicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glDisable(GL_DEPTH_TEST);

				glUseProgram(0);
			}break;

			case RenderEntry_Test: {
#if 0
				glEnable(GL_DEPTH_TEST);
				for (int i = -5; i < 5; i++) {
					for (int j = -5; j < 5; j++) {
						for (int k = -5; k < 5; k++) {

							if (i != 0 && j != 0 && k != 0) {
								OpenGLRenderCube(
									GLState,
									&GLState->WtfShader, 
									V3(i * 10, j * 10, k * 10));
							}
						}
					}
				}

				OpenGLTempRenderPlane(GLState, &GLState->WtfShader);

				glDisable(GL_DEPTH_TEST);
#endif
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


#if 1
	gl_wtf_shader* Shader = &State->WtfShader;
	GLfloat PlaneVertices[] = {
		//P N UV C
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f, 0.0f,	0.0f, 1.0f,		0.2f, 0.5f, 0.18f,
		0.5f, 0.0f, -0.5f,		0.0f, 1.0f, 0.0f,	1.0f, 1.0f,		0.2f, 0.5f, 0.18f,
		0.5f, 0.0f, 0.5f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f,		0.2f, 0.5f, 0.18f,
		-0.5f, 0.0f, 0.5f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f,		0.2f, 0.5f, 0.18f,
	};

	GLuint PlaneIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	GLuint PlaneEBO, PlaneVBO;

	glGenVertexArrays(1, &State->PlaneVAO);
	glGenBuffers(1, &PlaneVBO);
	glGenBuffers(1, &PlaneEBO);

	glBindVertexArray(State->PlaneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, PlaneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PlaneVertices), PlaneVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PlaneEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(PlaneIndices), PlaneIndices, GL_STATIC_DRAW);

	if (OpenGLArrayIsValid(Shader->PositionIndex)) {
		glEnableVertexAttribArray(Shader->PositionIndex);
		glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)0);
	}

	if (OpenGLArrayIsValid(Shader->UVIndex)) {
		glEnableVertexAttribArray(Shader->UVIndex);
		glVertexAttribPointer(Shader->UVIndex, 2, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	}

	if (OpenGLArrayIsValid(Shader->NormalIndex)) {
		glEnableVertexAttribArray(Shader->NormalIndex);
		glVertexAttribPointer(Shader->NormalIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	}

	if (OpenGLArrayIsValid(Shader->ColorIndex)) {
		glEnableVertexAttribArray(Shader->ColorIndex);
		glVertexAttribPointer(Shader->ColorIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float CubeVertices[] = {
		/*P N UV C*/
		//NOTE(Dima): Front side
		-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Top side
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Right side
		0.5f, 0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Left side
		-0.5f, 0.5f, -0.5f,		-1.0f, 0.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Back side
		0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		//NOTE(Dima): Down side
		-0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		0.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 1.0f,		1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f, 1.0f,
	};

	u32 CubeIndices[] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	GLuint CubeEBO, CubeVBO;

	glGenVertexArrays(1, &State->CubeVAO);
	glGenBuffers(1, &CubeVBO);
	glGenBuffers(1, &CubeEBO);

	glBindVertexArray(State->CubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), CubeIndices, GL_STATIC_DRAW);

	if (OpenGLArrayIsValid(Shader->PositionIndex)) {
		glEnableVertexAttribArray(Shader->PositionIndex);
		glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)0);
	}

	if (OpenGLArrayIsValid(Shader->UVIndex)) {
		glEnableVertexAttribArray(Shader->UVIndex);
		glVertexAttribPointer(Shader->UVIndex, 2, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	}

	if (OpenGLArrayIsValid(Shader->NormalIndex)) {
		glEnableVertexAttribArray(Shader->NormalIndex);
		glVertexAttribPointer(Shader->NormalIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	}

	if (OpenGLArrayIsValid(Shader->ColorIndex)) {
		glEnableVertexAttribArray(Shader->ColorIndex);
		glVertexAttribPointer(Shader->ColorIndex, 3, GL_FLOAT, 0, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#endif

}