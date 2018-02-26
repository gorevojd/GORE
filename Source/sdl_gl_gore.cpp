#include "sdl_gl_gore.h"

#include <SDL_video.h>

#define SDL_GORE_GL_ID_TO_STRING(id) #id

#define SDL_OPENGL_GET_FUNCTION(name) name = (PFN_##name*)SDL_GL_GetProcAddress(SDL_GORE_GL_ID_TO_STRING(name))

void SDLLoadOpenGLFunctions() {
	SDL_GL_LoadLibrary(0);

	SDL_OPENGL_GET_FUNCTION(glCreateShader);
	SDL_OPENGL_GET_FUNCTION(glAttachShader);
	SDL_OPENGL_GET_FUNCTION(glCompileShader);
	SDL_OPENGL_GET_FUNCTION(glDeleteShader);
	SDL_OPENGL_GET_FUNCTION(glDetachShader);
	SDL_OPENGL_GET_FUNCTION(glShaderSource);
	SDL_OPENGL_GET_FUNCTION(glGetShaderiv);
	SDL_OPENGL_GET_FUNCTION(glGetProgramiv);
	SDL_OPENGL_GET_FUNCTION(glCreateProgram);
	SDL_OPENGL_GET_FUNCTION(glDeleteProgram);
	SDL_OPENGL_GET_FUNCTION(glLinkProgram);
	SDL_OPENGL_GET_FUNCTION(glValidateProgram);
	SDL_OPENGL_GET_FUNCTION(glGetProgramInfoLog);
	SDL_OPENGL_GET_FUNCTION(glUseProgram);
	SDL_OPENGL_GET_FUNCTION(glVertexAttribPointer);
	SDL_OPENGL_GET_FUNCTION(glVertexAttribIPointer);
	SDL_OPENGL_GET_FUNCTION(glVertexAttribLPointer);
	SDL_OPENGL_GET_FUNCTION(glGetAttribLocation);
	SDL_OPENGL_GET_FUNCTION(glEnableVertexAttribArray);
	SDL_OPENGL_GET_FUNCTION(glDisableVertexAttribArray);
	SDL_OPENGL_GET_FUNCTION(glEnableVertexArrayAttrib);
	SDL_OPENGL_GET_FUNCTION(glDisableVertexArrayAttrib);
#if 0
	SDL_OPENGL_GET_FUNCTION(glDrawArrays);
	SDL_OPENGL_GET_FUNCTION(glDrawElements);
#endif
	SDL_OPENGL_GET_FUNCTION(glMultiDrawElements);
	SDL_OPENGL_GET_FUNCTION(glGetUniformfv);
	SDL_OPENGL_GET_FUNCTION(glGetUniformiv);
	SDL_OPENGL_GET_FUNCTION(glGetUniformuiv);
	SDL_OPENGL_GET_FUNCTION(glGetUniformdv);
	SDL_OPENGL_GET_FUNCTION(glGetnUniformfv);
	SDL_OPENGL_GET_FUNCTION(glGetnUniformiv);
	SDL_OPENGL_GET_FUNCTION(glGetnUniformuiv);
	SDL_OPENGL_GET_FUNCTION(glGetnUniformdv);
	SDL_OPENGL_GET_FUNCTION(glUniform1f);
	SDL_OPENGL_GET_FUNCTION(glUniform2f);
	SDL_OPENGL_GET_FUNCTION(glUniform3f);
	SDL_OPENGL_GET_FUNCTION(glUniform4f);
	SDL_OPENGL_GET_FUNCTION(glUniform1i);
	SDL_OPENGL_GET_FUNCTION(glUniform2i);
	SDL_OPENGL_GET_FUNCTION(glUniform3i);
	SDL_OPENGL_GET_FUNCTION(glUniform4i);
	SDL_OPENGL_GET_FUNCTION(glUniform1ui);
	SDL_OPENGL_GET_FUNCTION(glUniform2ui);
	SDL_OPENGL_GET_FUNCTION(glUniform3ui);
	SDL_OPENGL_GET_FUNCTION(glUniform4ui);
	SDL_OPENGL_GET_FUNCTION(glUniform1fv);
	SDL_OPENGL_GET_FUNCTION(glUniform2fv);
	SDL_OPENGL_GET_FUNCTION(glUniform3fv);
	SDL_OPENGL_GET_FUNCTION(glUniform4fv);
	SDL_OPENGL_GET_FUNCTION(glUniform1iv);
	SDL_OPENGL_GET_FUNCTION(glUniform2iv);
	SDL_OPENGL_GET_FUNCTION(glUniform3iv);
	SDL_OPENGL_GET_FUNCTION(glUniform4iv);
	SDL_OPENGL_GET_FUNCTION(glUniform1uiv);
	SDL_OPENGL_GET_FUNCTION(glUniform2uiv);
	SDL_OPENGL_GET_FUNCTION(glUniform3uiv);
	SDL_OPENGL_GET_FUNCTION(glUniform4uiv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix2fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix3fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix4fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix2x3fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix3x2fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix2x4fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix4x2fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix3x4fv);
	SDL_OPENGL_GET_FUNCTION(glUniformMatrix4x3fv);

	SDL_GL_UnloadLibrary();
}