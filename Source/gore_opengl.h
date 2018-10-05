#ifndef GORE_OPENGL_H_INCLUDED

#include "gore_opengl_common.h"
#include "gore_platform.h"
#include "gore_render_state.h"
#include "gore_lighting.h"
#include "gore_voxshared.h"
#include "gore_lpterrain.h"
#include "gore_game_settings.h"

struct gl_program {
public:
	GLuint Handle;

	int GetAttrib(char* AttribName) {
		int Result = glGetAttribLocation(this->Handle, AttribName);

		return(Result);
	}

	int GetUniform(char* UniformName) {
		int Result = glGetUniformLocation(this->Handle, UniformName);

		return(Result);
	}

	void Use() {
		glUseProgram(this->Handle);
	}
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
	GLint FogColorLocation;

	GLint CameraPLocation;

	GLint DiffuseMapLocation;

	GLint DirDirectionLocation;
	GLint DirDiffuseLocation;
	GLint DirAmbientLocation;

	gl_program Program;
};

struct gl_lpter_shader {
	GLint PositionIndex;
	GLint NormalIndex;
	GLint ColorIndex;

	GLint ModelMatrixLocation;
	GLint ViewMatrixLocation;
	GLint ProjectionMatrixLocation;
	GLint NormalsBufferLocation;
	GLint FogColorLocation;

	GLint CameraPLocation;

	GLint DirDirectionLocation;
	GLint DirDiffuseLocation;
	GLint DirAmbientLocation;

	gl_program Program;
};

struct gl_lpter_water_shader {
	GLint PositionIndex;
	GLint OffsetsToOthersIndex;

	GLint ModelMatrixLocation;
	GLint ViewMatrixLocation;
	GLint ProjectionMatrixLocation;

	GLint WaterLevelLocation;
	GLint GlobalTimeLocation;
	GLint PerVertexOfffsetLocation;
	GLint CameraPLocation;

	GLint DirDirectionLocation;
	GLint DirDiffuseLocation;

	gl_program Program;
};

struct gl_fxaa_shader {
	GLint PosIndex;
	GLint TexCoordIndex;

	GLint TextureLocation;
	GLint TextureSizeLocation;

	gl_program Program;
};

struct gl_screen_shader {
	GLint PosIndex;
	GLint TexIndex;

	GLint ScreenTextureLocation;

	gl_program Program;
};

struct gl_sprite_shader {
	GLint PosIndex;
	GLint TexIndex;

	GLint ViewMatrixLocation;
	GLint ProjectionMatrixLocation;

	GLint Bitmap1IsSetLocation;
	GLint Bitmap1Location;
	GLint ModulationColorLocation;
	GLint CameraPLocation;
	GLint MirrorUVsHorizontallyLocation;

	gl_program Program;
};

enum opengl_framebuffer_depthstencil_attachment_type {
	OpenGL_DS_Renderbuffer,
	OpenGL_DS_Texture,
};

struct opengl_framebuffer {
	GLuint FBO;
	GLuint Texture;

	u32 DepthStencilAttachmentType;
	GLuint DepthStencilRBO;
	GLuint DepthStencilTexture;
};

struct gl_state {
	gl_wtf_shader WtfShader;
	gl_voxel_shader VoxelShader;
	gl_screen_shader ScreenShader;
	gl_fxaa_shader FXAAShader;
	gl_lpter_shader LpterShader;
	gl_lpter_water_shader LpterWaterShader;
	gl_sprite_shader SpriteShader;

	GLuint ScreenQuadVAO;
	GLuint ScreenQuadVBO;

	//NOTE(dima): This framebuffer will be used for storing and rendering GUI
	opengl_framebuffer FramebufferGUI;

	//NOTE(dima): This is the first framebuffer that we will render to
	opengl_framebuffer FramebufferInitial;

	//NOTE(dima): If antialiasing is MSAA then multisampled data will be resolved to this buffer
	opengl_framebuffer FramebufferResolved;

	//NOTE(dima): This framebuffer will be used for posteffects
	opengl_framebuffer FramebufferPFX;
	
	//NOTE(dima): theese are temp values
	GLuint CubeVAO;
	GLuint PlaneVAO;

	//NOTE(dima): Extensions supported
	b32 AnisotropicFilteringSupported;
	float MaxAnisotropicLevel;
	u32 AnisotropicLevelType;
	float AnisotropicLevel;
	b32 FXAAEnabled;

	u32 AntialiasingType;

	b32 MultisamplingSupported;
	int MaxMultisampleLevel;
	int MultisampleLevel;
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

extern void OpenGLProcessAllocationQueue();
extern void OpenGLRenderStateToOutput(
	gl_state* State, 
	render_state* Stack, 
	game_settings* GameSettings);

extern void OpenGLInitState(
	gl_state* State,
	int RenderWidth,
	int RenderHeight,
	game_settings* GameSettings);

inline b32 OpenGLArrayIsValid(GLint ArrayIndex) {
	b32 Result = 0;

	if (ArrayIndex != -1) {
		Result = true;
	}

	return(Result);
}

#define GORE_OPENGL_H_INCLUDED
#endif