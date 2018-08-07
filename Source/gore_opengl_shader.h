#ifndef GORE_OPENGL_SHADER_H_INCLUDED
#define GORE_OPENGL_SHADER_H_INCLUDED

#include "gore_math.h"
#include "gore_opengl_common.h"
#include "gore_platform.h"

class opengl_shader {
protected:
	GLuint ProgramHandle;

public:
	opengl_shader(char* VertexPath, char* FragmentPath);

	~opengl_shader();

	void Use();

	void SetInt(char* UniformName, int Value);

	void SetUnsignedInt(char* UniformName, u32 Value);

	void SetFloat(char* UniformName, float Value);

	void SetIV2(char* UniformName, int x, int y);

	void SetIV3(char* UniformName, int x, int y, int z);

	void SetIV4(char* UniformName, int x, int y, int z, int w);

	void SetUV2(char* UniformName, u32 x, u32 y);

	void SetUV3(char* UniformName, u32 x, u32 y, u32 z);

	void SetUV4(char* UniformName, u32 x, u32 y, u32 z, u32 w);

	void SetV2(char* UniformName, v2 Value);

	void SetV2(char* UniformName, float x, float y);

	void SetV3(char* UniformName, float x, float y, float z);

	void SetV3(char* UniformName, v3 VectorToSet);

	void SetV4(char* UniformName, float x, float y, float z, float w);

	void SetV4(char* UniformName, v4 Value);

	void SetMat4(char* UniformName, float* Elements, b32 Transpose);
};

#endif