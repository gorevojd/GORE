#ifndef GORE_OPENGL_H_INCLUDED

#include "gore_platform.h"
#include "gore_render_state.h"
#include "gore_lighting.h"
#include "gore_voxshared.h"

#include <SDL_opengl.h>

struct gl_program {
	GLuint Handle;
};

struct gl_wtf_shader {
	GLint PositionIndex;
	GLint UVIndex;
	GLint NormalIndex;
	GLint ColorIndex;
	GLint TangentIndex;

	GLint ModelMatrixLocation;
	GLint ViewMatrixLocation;
	GLint ProjectionMatrixLocation;

	GLint CameraPLocation;

	GLint SurfMatShineLocation;
	GLint SurfMatColorLocation;

	GLint SurfMatDiffLocation;
	GLint SurfMatSpecLocation;
	GLint SurfMatEmisLocation;

	GLint SurfMatHasDiffLocation;
	GLint SurfMatHasSpecLocation;
	GLint SurfMatHasEmisLocation;

	gl_program Program;
};

struct gl_voxel_shader {

	GLint VertexDataIndex;

	GLint ModelMatrixLocation;
	GLint ViewMatrixLocation;
	GLint ProjectionMatrixLocation;

	GLint CameraPLocation;

	GLint DiffuseMapLocation;

	GLint DirDirectionLocation;
	GLint DirDiffuseLocation;
	GLint DirAmbientLocation;

	gl_program Program;
};


struct gl_state {
	gl_wtf_shader WtfShader;
	gl_voxel_shader VoxelShader;

	//NOTE(dima): theese are temp values
	GLuint CubeVAO;
	GLuint PlaneVAO;
};

//#define GL_GUI_CHUNK_POLYS_COUNT 4096
//struct gl_gui_polygons_chunk {
//	float PosX[GL_GUI_CHUNK_POLYS_COUNT * 4];
//	float PosY[GL_GUI_CHUNK_POLYS_COUNT * 4];
//
//	float TexU[GL_GUI_CHUNK_POLYS_COUNT * 4];
//	float TexV[GL_GUI_CHUNK_POLYS_COUNT * 4];
//
//	u32 Indices[GL_GUI_CHUNK_POLYS_COUNT * 6];
//
//	
//};

extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM2IPROC glUniform2i;
extern PFNGLUNIFORM3IPROC glUniform3i;
extern PFNGLUNIFORM4IPROC glUniform4i;
extern PFNGLUNIFORM1UIPROC glUniform1ui;
extern PFNGLUNIFORM2UIPROC glUniform2ui;
extern PFNGLUNIFORM3UIPROC glUniform3ui;
extern PFNGLUNIFORM4UIPROC glUniform4ui;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM2IVPROC glUniform2iv;
extern PFNGLUNIFORM3IVPROC glUniform3iv;
extern PFNGLUNIFORM4IVPROC glUniform4iv;
extern PFNGLUNIFORM1UIVPROC glUniform1uiv;
extern PFNGLUNIFORM2UIVPROC glUniform2uiv;
extern PFNGLUNIFORM3UIVPROC glUniform3uiv;
extern PFNGLUNIFORM4UIVPROC glUniform4uiv;
extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;


typedef void (GLAPIENTRY *MYPFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
extern MYPFNGLDRAWELEMENTSPROC _glDrawElements;

typedef void (GLAPIENTRY *MYPFNGLACTIVETEXTURE)(GLenum texture);
extern MYPFNGLACTIVETEXTURE _glActiveTexture;

extern void OpenGLProcessAllocationQueue();
extern void OpenGLRenderStackToOutput(gl_state* State, render_state* Stack);

extern void OpenGLInitState(gl_state* State);

inline b32 OpenGLArrayIsValid(GLint ArrayIndex) {
	b32 Result = 0;

	if (ArrayIndex != -1) {
		Result = true;
	}

	return(Result);
}

#define GORE_OPENGL_H_INCLUDED
#endif