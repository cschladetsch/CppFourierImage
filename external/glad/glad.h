/*
 * Simple GLAD stub for OpenGL 3.3 Core
 * This is a minimal implementation to get the build working
 */

#ifndef __glad_h_
#define __glad_h_

#include <stddef.h>  /* for ptrdiff_t */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
    #ifdef _WIN32
        #define APIENTRY __stdcall
    #else
        #define APIENTRY
    #endif
#endif

#ifndef APIENTRYP
    #define APIENTRYP APIENTRY *
#endif

#ifndef GLAPI
    #define GLAPI extern
#endif

/* OpenGL types */
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned short GLushort;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

/* OpenGL function loader */
typedef void* (* GLADloadproc)(const char *name);

/* Initialize OpenGL function pointers */
GLAPI int gladLoadGLLoader(GLADloadproc load);

/* OpenGL 3.3 Core Profile functions - declare only what we need */
GLAPI void glClear(GLbitfield mask);
GLAPI void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLAPI void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void glEnable(GLenum cap);
GLAPI void glDisable(GLenum cap);
GLAPI void glBlendFunc(GLenum sfactor, GLenum dfactor);

/* Shader functions */
GLAPI GLuint glCreateShader(GLenum type);
GLAPI void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
GLAPI void glCompileShader(GLuint shader);
GLAPI void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
GLAPI void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void glDeleteShader(GLuint shader);

/* Program functions */
GLAPI GLuint glCreateProgram(void);
GLAPI void glAttachShader(GLuint program, GLuint shader);
GLAPI void glLinkProgram(GLuint program);
GLAPI void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
GLAPI void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void glUseProgram(GLuint program);
GLAPI void glDeleteProgram(GLuint program);

/* Buffer functions */
GLAPI void glGenBuffers(GLsizei n, GLuint *buffers);
GLAPI void glBindBuffer(GLenum target, GLuint buffer);
GLAPI void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GLAPI void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GLAPI void glDeleteBuffers(GLsizei n, const GLuint *buffers);

/* Vertex array functions */
GLAPI void glGenVertexArrays(GLsizei n, GLuint *arrays);
GLAPI void glBindVertexArray(GLuint array);
GLAPI void glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
GLAPI void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GLAPI void glEnableVertexAttribArray(GLuint index);

/* Texture functions */
GLAPI void glGenTextures(GLsizei n, GLuint *textures);
GLAPI void glBindTexture(GLenum target, GLuint texture);
GLAPI void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GLAPI void glTexParameteri(GLenum target, GLenum pname, GLint param);
GLAPI void glDeleteTextures(GLsizei n, const GLuint *textures);
GLAPI void glActiveTexture(GLenum texture);

/* Drawing functions */
GLAPI void glDrawArrays(GLenum mode, GLint first, GLsizei count);
GLAPI void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
GLAPI void glLineWidth(GLfloat width);

/* State functions */
GLAPI void glGetIntegerv(GLenum pname, GLint *data);

/* Uniform functions */
GLAPI GLint glGetUniformLocation(GLuint program, const GLchar *name);
GLAPI void glUniform1i(GLint location, GLint v0);
GLAPI void glUniform1f(GLint location, GLfloat v0);
GLAPI void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLAPI void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* Constants */
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_BLEND                          0x0BE2
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_FLOAT                          0x1406
#define GL_RGBA                           0x1908
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE0                       0x84C0
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_TRIANGLES                      0x0004
#define GL_LINE_STRIP                     0x0003
#define GL_LINES                          0x0001
#define GL_UNSIGNED_INT                   0x1405
#define GL_VIEWPORT                       0x0BA2

#ifdef __cplusplus
}
#endif

#endif /* __glad_h_ */