#include "gore_opengl_shader.h"


static GLuint OpenGLLoadProgramFromSource(char* VertexSource, char* FragmentSource) {
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
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentSource, 0);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG("Fragment shader load error");
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

opengl_shader::opengl_shader(char * VertexPath, char * FragmentPath)
{
	platform_read_file_result VertexFile = PlatformApi.ReadFile(VertexPath);
	platform_read_file_result FragmentFile = PlatformApi.ReadFile(FragmentPath);

	this->ProgramHandle = OpenGLLoadProgramFromSource(
		(char*)VertexFile.Data,
		(char*)FragmentFile.Data);

	PlatformApi.FreeFileMemory(&VertexFile);
	PlatformApi.FreeFileMemory(&FragmentFile);
}

opengl_shader::~opengl_shader()
{
	glDeleteProgram(this->ProgramHandle);
}

void opengl_shader::Use()
{
	glUseProgram(this->ProgramHandle);
}

void opengl_shader::SetInt(char * UniformName, int Value)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform1i(Location, Value);
}

void opengl_shader::SetUnsignedInt(char * UniformName, u32 Value)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform1ui(Location, Value);
}

void opengl_shader::SetFloat(char * UniformName, float Value)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform1f(Location, Value);
}

void opengl_shader::SetIV2(char * UniformName, int x, int y)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform2i(Location, x, y);
}

void opengl_shader::SetIV3(char * UniformName, int x, int y, int z)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform3i(Location, x, y, z);
}

void opengl_shader::SetIV4(char * UniformName, int x, int y, int z, int w)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform4i(Location, x, y, z, w);
}

void opengl_shader::SetUV2(char * UniformName, u32 x, u32 y)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform2ui(Location, x, y);
}

void opengl_shader::SetUV3(char * UniformName, u32 x, u32 y, u32 z)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform3ui(Location, x, y, z);
}

void opengl_shader::SetUV4(char * UniformName, u32 x, u32 y, u32 z, u32 w)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform4ui(Location, x, y, z, w);
}

void opengl_shader::SetV2(char * UniformName, v2 Value)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform2f(Location, Value.x, Value.y);
}

void opengl_shader::SetV2(char * UniformName, float x, float y)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform2f(Location, x, y);
}

void opengl_shader::SetV3(char * UniformName, float x, float y, float z)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform3f(Location, x, y, z);
}

void opengl_shader::SetV3(char * UniformName, v3 VectorToSet)
{
	SetV3(UniformName, VectorToSet.x, VectorToSet.y, VectorToSet.z);
}

void opengl_shader::SetV4(char * UniformName, float x, float y, float z, float w)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniform4f(Location, x, y, z, w);
}

void opengl_shader::SetV4(char * UniformName, v4 Value)
{
	SetV4(UniformName, Value.x, Value.y, Value.z, Value.w);
}

void opengl_shader::SetMat4(char * UniformName, float * Elements, b32 Transpose)
{
	int Location = glGetUniformLocation(this->ProgramHandle, UniformName);
	glUniformMatrix4fv(Location, 1, Transpose, Elements);
}
