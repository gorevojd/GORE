#include "gore_opengl.h"

GLuint OpenGLLoadProgramFromSource(char* VertexPath, char* FragmentPath) {
	char InfoLog[512];
	int Success;
	
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Program;

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexPath, 0);
	glCompileShader(VertexShader);
	
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(VertexShader, sizeof(InfoLog), 0, InfoLog);
		//TODO(dima): Logging
	}

	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentPath, 0);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), 0, InfoLog);
		//TODO(dima): Logging
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	glGetProgramiv(Program, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(Program, sizeof(InfoLog), 0, InfoLog);
		//TODO(dima): Logging
	}

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	return(Program);
}

gl_program OpenGLLoadShader(char* VertexPath, char* FragmentPath) {
	gl_program Result = {};

	Result.Handle = OpenGLLoadProgramFromSource(VertexPath, FragmentPath);

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

	Result.ModelMatrixLocation = glGetUniform

	return(Result);
}

inline void OpenGLUseProgramBegin(gl_program* Program) {
	glUseProgram(Program->Handle);
}

inline void OpenGLUseProgramEnd(gl_program* Program) {
	glUseProgram(0);
}

GLuint OpenGLAllocateTexture(rgba_buffer* Buffer) {
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

void OpenGLDeallocateTexture(rgba_buffer* Buffer) {
	GLuint TextureHandle = (GLuint)Buffer->TextureHandle;

	glDeleteTextures(1, &TextureHandle);
	Buffer->TextureHandle = (void*)0;
}

void OpenGLRenderBitmap(rgba_buffer* Buffer, v2 P, v2 Dim, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

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

	float ProjMatrix[] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, 1.0f, 0,
		-1.0f, 1.0f, 0, 1.0f
	};

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(ProjMatrix);
}

void OpenGLRenderCube(gl_wtf_shader* Shader, v3 Pos) {
	
	GLfloat CubeVertices[] = {
		/*P N UV*/
		//NOTE(Dima): Front side
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		//NOTE(Dima): Top side
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		//NOTE(Dima): Right side
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//NOTE(Dima): Left side
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		//NOTE(Dima): Back side
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		//NOTE(Dima): Down side
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f
	};

	GLuint CubeIndices[] = {
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

	GLuint VAO, EBO, VBO;
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), CubeIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(Shader->PositionIndex);
	glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, 0, 7 * sizeof(GLfloat), (void*)0);
	
	glEnableVertexAttribArray(Shader->UVIndex);
	glVertexAttribPointer(Shader->UVIndex, 2, GL_FLOAT, 0, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLuint)));

	glEnableVertexAttribArray(Shader->NormalIndex);
	glVertexAttribPointer(Shader->NormalIndex, 3, GL_FLOAT, 0, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLuint)));

	glBindVertexArray(0);

	//NOTE(dima): Rendering
	glUseProgram(Shader->Program.Handle);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void OpenGLRenderStackToOutput(gl_state* State, render_stack* Stack) {
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLSetScreenspace(Stack->WindowWidth, Stack->WindowHeight);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//NOTE(dima): Iteration through render stack
	u8* At = Stack->Data.BaseAddress;
	u8* StackEnd = Stack->Data.BaseAddress + Stack->Data.Used;

	while (At < StackEnd) {
		render_stack_entry_header* Header = (render_stack_entry_header*)At;

		u32 SizeOfEntryType = Header->SizeOfEntryType;

		At += sizeof(render_stack_entry_header);
		switch (Header->Type) {
			case(RenderStackEntry_Bitmap): {
				render_stack_entry_bitmap* EntryBitmap = (render_stack_entry_bitmap*)At;

				OpenGLRenderBitmap(EntryBitmap->Bitmap, EntryBitmap->P, EntryBitmap->Dim, EntryBitmap->ModulationColor);
			}break;

			case(RenderStackEntry_Clear): {
				render_stack_entry_clear* EntryClear = (render_stack_entry_clear*)At;

				glClearColor(
					EntryClear->Color.r,
					EntryClear->Color.g,
					EntryClear->Color.b,
					1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}break;

			case(RenderStackEntry_Gradient): {
				render_stack_entry_gradient* EntryGrad = (render_stack_entry_gradient*)At;
					
				//TODO(DIMA): 
			}break;

			case(RenderStackEntry_Rectangle): {
				render_stack_entry_rectangle* EntryRect = (render_stack_entry_rectangle*)At;

				OpenGLRenderRectangle(Rect2MinDim(EntryRect->P, EntryRect->Dim), EntryRect->ModulationColor);
			}break;

			case RenderStackEntry_Glyph: {
				render_stack_entry_glyph* EntryGlyph = (render_stack_entry_glyph*)At;

				glyph_info* Glyph = &EntryGlyph->FontInfo->Glyphs[EntryGlyph->FontInfo->CodepointToGlyphMapping[EntryGlyph->Codepoint]];

				v4 Color = EntryGlyph->ModulationColor;
				rect2 Rect = Rect2MinDim(EntryGlyph->P, EntryGlyph->Dim);
				v2 MinUV = Glyph->AtlasMinUV;
				v2 MaxUV = Glyph->AtlasMaxUV;

				glColor4f(Color.r, Color.g, Color.b, Color.a);

				glTexCoord2f(MinUV.x, MinUV.y);
				glVertex2f(Rect.Min.x, Rect.Min.y);
				glTexCoord2f(MaxUV.x, MinUV.y);
				glVertex2f(Rect.Max.x, Rect.Min.y);
				glTexCoord2f(MaxUV.x, MaxUV.y);
				glVertex2f(Rect.Max.x, Rect.Max.y);

				glTexCoord2f(MinUV.x, MinUV.y);
				glVertex2f(Rect.Min.x, Rect.Min.y);
				glTexCoord2f(MaxUV.x, MaxUV.y);
				glVertex2f(Rect.Max.x, Rect.Max.y);
				glTexCoord2f(MinUV.x, MaxUV.y);
				glVertex2f(Rect.Min.x, Rect.Max.y);

			}break;

			case RenderStackEntry_BeginText: {
				render_stack_entry_begin_text* EntryBeginText = (render_stack_entry_begin_text*)At;

				rgba_buffer* Buffer = &EntryBeginText->FontInfo->FontAtlasImage;

				if (!Buffer->TextureHandle) {
					OpenGLAllocateTexture(Buffer);
				}

				glBindTexture(GL_TEXTURE_2D, (GLuint)Buffer->TextureHandle);
				glBegin(GL_TRIANGLES);

			}break;

			case RenderStackEntry_EndText: {
				render_stack_entry_end_text* EntryEndText = (render_stack_entry_end_text*)At;

				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);
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