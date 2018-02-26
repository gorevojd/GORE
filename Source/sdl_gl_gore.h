#ifndef SDL_GORE_GL_H_INCLUDED
#define SDL_GORE_GL_H_INCLUDED

//#define GL_GLEXT_PROTOTYPES
#define NO_SDL_GLEXT
#include <SDL_opengl.h>

extern void SDLLoadOpenGLFunctions();

#define SDL_OPENGL_FUNCTION_DECLARE(name) static PFN_##name* name = 0

typedef GLuint GLAPIENTRY PFN_glCreateShader(GLenum shaderType);
typedef void GLAPIENTRY PFN_glAttachShader(GLuint program, GLuint shader);
typedef void GLAPIENTRY PFN_glCompileShader(GLuint shader);
typedef void GLAPIENTRY PFN_glDeleteShader(GLuint shader);
typedef void GLAPIENTRY PFN_glDetachShader(GLuint program, GLuint shader);
typedef void GLAPIENTRY PFN_glShaderSource(GLuint shader, GLsizei count, const char **string, const GLint *length);
typedef void GLAPIENTRY PFN_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void GLAPIENTRY PFN_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef GLuint GLAPIENTRY PFN_glCreateProgram(void);
typedef void GLAPIENTRY PFN_glDeleteProgram(GLuint program);
typedef void GLAPIENTRY PFN_glLinkProgram(GLuint program);
typedef void GLAPIENTRY PFN_glValidateProgram(GLuint program);
typedef void GLAPIENTRY PFN_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, char *infoLog);
typedef void GLAPIENTRY PFN_glUseProgram(GLuint program);

typedef void GLAPIENTRY PFN_glVertexAttribPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	const GLvoid * pointer);

typedef void GLAPIENTRY PFN_glVertexAttribIPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	const GLvoid * pointer);

typedef void GLAPIENTRY PFN_glVertexAttribLPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	const GLvoid * pointer);

typedef GLint GLAPIENTRY PFN_glGetAttribLocation(GLuint program, const char *name);
typedef void GLAPIENTRY PFN_glEnableVertexAttribArray(GLuint index);
typedef void GLAPIENTRY PFN_glDisableVertexAttribArray(GLuint index);
typedef void GLAPIENTRY PFN_glEnableVertexArrayAttrib(GLuint vaobj, GLuint index);
typedef void GLAPIENTRY PFN_glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);


typedef void GLAPIENTRY PFN_glDrawArrays(
	GLenum mode,
	GLint first,
	GLsizei count);

typedef void GLAPIENTRY PFN_glDrawElements(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const GLvoid * indices);

typedef void GLAPIENTRY PFN_glMultiDrawElements(
	GLenum mode,
	const GLsizei * count,
	GLenum type,
	const GLvoid * const * indices,
	GLsizei drawcount);

typedef void GLAPIENTRY PFN_glGetUniformfv(GLuint program, GLint location, GLfloat *params);
typedef void GLAPIENTRY PFN_glGetUniformiv(GLuint program, GLint location, GLint *params);
typedef void GLAPIENTRY PFN_glGetUniformuiv(GLuint program, GLint location, GLuint *params);
typedef void GLAPIENTRY PFN_glGetUniformdv(GLuint program, GLint location, GLdouble *params);
typedef void GLAPIENTRY PFN_glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
typedef void GLAPIENTRY PFN_glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params);
typedef void GLAPIENTRY PFN_glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
typedef void GLAPIENTRY PFN_glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);

typedef void GLAPIENTRY PFN_glUniform1f(GLint location, GLfloat v0);
typedef void GLAPIENTRY PFN_glUniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void GLAPIENTRY PFN_glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void GLAPIENTRY PFN_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void GLAPIENTRY PFN_glUniform1i(GLint location, GLint v0);
typedef void GLAPIENTRY PFN_glUniform2i(GLint location, GLint v0, GLint v1);
typedef void GLAPIENTRY PFN_glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void GLAPIENTRY PFN_glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void GLAPIENTRY PFN_glUniform1ui(GLint location, GLuint v0);
typedef void GLAPIENTRY PFN_glUniform2ui(GLint location, GLuint v0, GLuint v1);
typedef void GLAPIENTRY PFN_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void GLAPIENTRY PFN_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void GLAPIENTRY PFN_glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniform1iv(GLint location, GLsizei count, const GLint *value);
typedef void GLAPIENTRY PFN_glUniform2iv(GLint location, GLsizei count, const GLint *value);
typedef void GLAPIENTRY PFN_glUniform3iv(GLint location, GLsizei count, const GLint *value);
typedef void GLAPIENTRY PFN_glUniform4iv(GLint location, GLsizei count, const GLint *value);
typedef void GLAPIENTRY PFN_glUniform1uiv(GLint location, GLsizei count, const GLuint *value);
typedef void GLAPIENTRY PFN_glUniform2uiv(GLint location, GLsizei count, const GLuint *value);
typedef void GLAPIENTRY PFN_glUniform3uiv(GLint location, GLsizei count, const GLuint *value);
typedef void GLAPIENTRY PFN_glUniform4uiv(GLint location, GLsizei count, const GLuint *value);
typedef void GLAPIENTRY PFN_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GLAPIENTRY PFN_glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);



SDL_OPENGL_FUNCTION_DECLARE(glCreateShader);
SDL_OPENGL_FUNCTION_DECLARE(glAttachShader);
SDL_OPENGL_FUNCTION_DECLARE(glCompileShader);
SDL_OPENGL_FUNCTION_DECLARE(glDeleteShader);
SDL_OPENGL_FUNCTION_DECLARE(glDetachShader);
SDL_OPENGL_FUNCTION_DECLARE(glShaderSource);
SDL_OPENGL_FUNCTION_DECLARE(glGetShaderiv);
SDL_OPENGL_FUNCTION_DECLARE(glGetProgramiv);
SDL_OPENGL_FUNCTION_DECLARE(glCreateProgram);
SDL_OPENGL_FUNCTION_DECLARE(glDeleteProgram);
SDL_OPENGL_FUNCTION_DECLARE(glLinkProgram);
SDL_OPENGL_FUNCTION_DECLARE(glValidateProgram);
SDL_OPENGL_FUNCTION_DECLARE(glGetProgramInfoLog);
SDL_OPENGL_FUNCTION_DECLARE(glUseProgram);
SDL_OPENGL_FUNCTION_DECLARE(glVertexAttribPointer);
SDL_OPENGL_FUNCTION_DECLARE(glVertexAttribIPointer);
SDL_OPENGL_FUNCTION_DECLARE(glVertexAttribLPointer);
SDL_OPENGL_FUNCTION_DECLARE(glGetAttribLocation);
SDL_OPENGL_FUNCTION_DECLARE(glEnableVertexAttribArray);
SDL_OPENGL_FUNCTION_DECLARE(glDisableVertexAttribArray);
SDL_OPENGL_FUNCTION_DECLARE(glEnableVertexArrayAttrib);
SDL_OPENGL_FUNCTION_DECLARE(glDisableVertexArrayAttrib);
#if 0
SDL_OPENGL_FUNCTION_DECLARE(glDrawArrays);
SDL_OPENGL_FUNCTION_DECLARE(glDrawElements);
#endif
SDL_OPENGL_FUNCTION_DECLARE(glMultiDrawElements);
SDL_OPENGL_FUNCTION_DECLARE(glGetUniformfv);
SDL_OPENGL_FUNCTION_DECLARE(glGetUniformiv);
SDL_OPENGL_FUNCTION_DECLARE(glGetUniformuiv);
SDL_OPENGL_FUNCTION_DECLARE(glGetUniformdv);
SDL_OPENGL_FUNCTION_DECLARE(glGetnUniformfv);
SDL_OPENGL_FUNCTION_DECLARE(glGetnUniformiv);
SDL_OPENGL_FUNCTION_DECLARE(glGetnUniformuiv);
SDL_OPENGL_FUNCTION_DECLARE(glGetnUniformdv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1f);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2f);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3f);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4f);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1i);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2i);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3i);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4i);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1ui);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2ui);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3ui);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4ui);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1iv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2iv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3iv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4iv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform1uiv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform2uiv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform3uiv);
SDL_OPENGL_FUNCTION_DECLARE(glUniform4uiv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix2fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix3fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix4fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix2x3fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix3x2fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix2x4fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix4x2fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix3x4fv);
SDL_OPENGL_FUNCTION_DECLARE(glUniformMatrix4x3fv);


#endif