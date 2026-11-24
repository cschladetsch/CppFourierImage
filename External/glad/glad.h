/*
 * Minimal OpenGL 3.3 Core loader in the style of GLAD.
 *
 * Every GL function this project uses is called through a function pointer
 * that gladLoadGLLoader() fills in at runtime (e.g. with glfwGetProcAddress).
 * This is required on Windows, where opengl32.lib only exports OpenGL 1.1.
 *
 * To use a new GL function, add it in four places: the typedef, the extern
 * pointer and the #define below, and the LOAD() line in glad.c.
 */

#ifndef __glad_h_
#define __glad_h_

/* Stop the system GL header being included after this one. */
#ifndef __gl_h_
#define __gl_h_
#endif
#ifndef __GL_H__
#define __GL_H__
#endif

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

/* Load all OpenGL function pointers. Returns 1 on success, 0 if any are missing. */
GLAPI int gladLoadGLLoader(GLADloadproc load);

/* Function pointer types */
typedef void   (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
typedef void   (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void   (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void   (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void   (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);
typedef void   (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);

typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
typedef void   (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void   (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);

typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void   (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);

typedef void   (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void   (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void   (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void   (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);

typedef void   (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void   (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void   (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void   (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void   (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);

typedef void   (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void   (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void   (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void   (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void   (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void   (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);

typedef void   (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void   (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void   (APIENTRYP PFNGLLINEWIDTHPROC)(GLfloat width);

typedef void   (APIENTRYP PFNGLGETINTEGERVPROC)(GLenum pname, GLint *data);

typedef GLint  (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void   (APIENTRYP PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void   (APIENTRYP PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void   (APIENTRYP PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void   (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* Function pointers, filled in by gladLoadGLLoader() */
GLAPI PFNGLCLEARPROC glad_glClear;
GLAPI PFNGLCLEARCOLORPROC glad_glClearColor;
GLAPI PFNGLVIEWPORTPROC glad_glViewport;
GLAPI PFNGLENABLEPROC glad_glEnable;
GLAPI PFNGLDISABLEPROC glad_glDisable;
GLAPI PFNGLBLENDFUNCPROC glad_glBlendFunc;

GLAPI PFNGLCREATESHADERPROC glad_glCreateShader;
GLAPI PFNGLSHADERSOURCEPROC glad_glShaderSource;
GLAPI PFNGLCOMPILESHADERPROC glad_glCompileShader;
GLAPI PFNGLGETSHADERIVPROC glad_glGetShaderiv;
GLAPI PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
GLAPI PFNGLDELETESHADERPROC glad_glDeleteShader;

GLAPI PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
GLAPI PFNGLATTACHSHADERPROC glad_glAttachShader;
GLAPI PFNGLLINKPROGRAMPROC glad_glLinkProgram;
GLAPI PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
GLAPI PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
GLAPI PFNGLUSEPROGRAMPROC glad_glUseProgram;
GLAPI PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;

GLAPI PFNGLGENBUFFERSPROC glad_glGenBuffers;
GLAPI PFNGLBINDBUFFERPROC glad_glBindBuffer;
GLAPI PFNGLBUFFERDATAPROC glad_glBufferData;
GLAPI PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
GLAPI PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;

GLAPI PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
GLAPI PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
GLAPI PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
GLAPI PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;

GLAPI PFNGLGENTEXTURESPROC glad_glGenTextures;
GLAPI PFNGLBINDTEXTUREPROC glad_glBindTexture;
GLAPI PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
GLAPI PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
GLAPI PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
GLAPI PFNGLACTIVETEXTUREPROC glad_glActiveTexture;

GLAPI PFNGLDRAWARRAYSPROC glad_glDrawArrays;
GLAPI PFNGLDRAWELEMENTSPROC glad_glDrawElements;
GLAPI PFNGLLINEWIDTHPROC glad_glLineWidth;

GLAPI PFNGLGETINTEGERVPROC glad_glGetIntegerv;

GLAPI PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
GLAPI PFNGLUNIFORM1IPROC glad_glUniform1i;
GLAPI PFNGLUNIFORM1FPROC glad_glUniform1f;
GLAPI PFNGLUNIFORM4FPROC glad_glUniform4f;
GLAPI PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;

/* Route the usual GL names through the loaded pointers */
#define glClear glad_glClear
#define glClearColor glad_glClearColor
#define glViewport glad_glViewport
#define glEnable glad_glEnable
#define glDisable glad_glDisable
#define glBlendFunc glad_glBlendFunc

#define glCreateShader glad_glCreateShader
#define glShaderSource glad_glShaderSource
#define glCompileShader glad_glCompileShader
#define glGetShaderiv glad_glGetShaderiv
#define glGetShaderInfoLog glad_glGetShaderInfoLog
#define glDeleteShader glad_glDeleteShader

#define glCreateProgram glad_glCreateProgram
#define glAttachShader glad_glAttachShader
#define glLinkProgram glad_glLinkProgram
#define glGetProgramiv glad_glGetProgramiv
#define glGetProgramInfoLog glad_glGetProgramInfoLog
#define glUseProgram glad_glUseProgram
#define glDeleteProgram glad_glDeleteProgram

#define glGenBuffers glad_glGenBuffers
#define glBindBuffer glad_glBindBuffer
#define glBufferData glad_glBufferData
#define glBufferSubData glad_glBufferSubData
#define glDeleteBuffers glad_glDeleteBuffers

#define glGenVertexArrays glad_glGenVertexArrays
#define glBindVertexArray glad_glBindVertexArray
#define glDeleteVertexArrays glad_glDeleteVertexArrays
#define glVertexAttribPointer glad_glVertexAttribPointer
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray

#define glGenTextures glad_glGenTextures
#define glBindTexture glad_glBindTexture
#define glTexImage2D glad_glTexImage2D
#define glTexParameteri glad_glTexParameteri
#define glDeleteTextures glad_glDeleteTextures
#define glActiveTexture glad_glActiveTexture

#define glDrawArrays glad_glDrawArrays
#define glDrawElements glad_glDrawElements
#define glLineWidth glad_glLineWidth

#define glGetIntegerv glad_glGetIntegerv

#define glGetUniformLocation glad_glGetUniformLocation
#define glUniform1i glad_glUniform1i
#define glUniform1f glad_glUniform1f
#define glUniform4f glad_glUniform4f
#define glUniformMatrix4fv glad_glUniformMatrix4fv

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
