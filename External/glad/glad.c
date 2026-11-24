/*
 * Minimal OpenGL 3.3 Core loader in the style of GLAD.
 * See glad.h for how to add a function.
 */

#include "glad.h"
#include <stdio.h>

PFNGLCLEARPROC glad_glClear;
PFNGLCLEARCOLORPROC glad_glClearColor;
PFNGLVIEWPORTPROC glad_glViewport;
PFNGLENABLEPROC glad_glEnable;
PFNGLDISABLEPROC glad_glDisable;
PFNGLBLENDFUNCPROC glad_glBlendFunc;

PFNGLCREATESHADERPROC glad_glCreateShader;
PFNGLSHADERSOURCEPROC glad_glShaderSource;
PFNGLCOMPILESHADERPROC glad_glCompileShader;
PFNGLGETSHADERIVPROC glad_glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
PFNGLDELETESHADERPROC glad_glDeleteShader;

PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
PFNGLATTACHSHADERPROC glad_glAttachShader;
PFNGLLINKPROGRAMPROC glad_glLinkProgram;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glad_glUseProgram;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;

PFNGLGENBUFFERSPROC glad_glGenBuffers;
PFNGLBINDBUFFERPROC glad_glBindBuffer;
PFNGLBUFFERDATAPROC glad_glBufferData;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;

PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;

PFNGLGENTEXTURESPROC glad_glGenTextures;
PFNGLBINDTEXTUREPROC glad_glBindTexture;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture;

PFNGLDRAWARRAYSPROC glad_glDrawArrays;
PFNGLDRAWELEMENTSPROC glad_glDrawElements;
PFNGLLINEWIDTHPROC glad_glLineWidth;

PFNGLGETINTEGERVPROC glad_glGetIntegerv;

PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
PFNGLUNIFORM1IPROC glad_glUniform1i;
PFNGLUNIFORM1FPROC glad_glUniform1f;
PFNGLUNIFORM4FPROC glad_glUniform4f;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;

int gladLoadGLLoader(GLADloadproc load) {
    int ok = 1;

    if (!load) {
        return 0;
    }

#define LOAD(type, name)                                              \
    do {                                                              \
        glad_##name = (type)load(#name);                              \
        if (!glad_##name) {                                           \
            fprintf(stderr, "GLAD: failed to load %s\n", #name);      \
            ok = 0;                                                   \
        }                                                             \
    } while (0)

    LOAD(PFNGLCLEARPROC, glClear);
    LOAD(PFNGLCLEARCOLORPROC, glClearColor);
    LOAD(PFNGLVIEWPORTPROC, glViewport);
    LOAD(PFNGLENABLEPROC, glEnable);
    LOAD(PFNGLDISABLEPROC, glDisable);
    LOAD(PFNGLBLENDFUNCPROC, glBlendFunc);

    LOAD(PFNGLCREATESHADERPROC, glCreateShader);
    LOAD(PFNGLSHADERSOURCEPROC, glShaderSource);
    LOAD(PFNGLCOMPILESHADERPROC, glCompileShader);
    LOAD(PFNGLGETSHADERIVPROC, glGetShaderiv);
    LOAD(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
    LOAD(PFNGLDELETESHADERPROC, glDeleteShader);

    LOAD(PFNGLCREATEPROGRAMPROC, glCreateProgram);
    LOAD(PFNGLATTACHSHADERPROC, glAttachShader);
    LOAD(PFNGLLINKPROGRAMPROC, glLinkProgram);
    LOAD(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
    LOAD(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
    LOAD(PFNGLUSEPROGRAMPROC, glUseProgram);
    LOAD(PFNGLDELETEPROGRAMPROC, glDeleteProgram);

    LOAD(PFNGLGENBUFFERSPROC, glGenBuffers);
    LOAD(PFNGLBINDBUFFERPROC, glBindBuffer);
    LOAD(PFNGLBUFFERDATAPROC, glBufferData);
    LOAD(PFNGLBUFFERSUBDATAPROC, glBufferSubData);
    LOAD(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);

    LOAD(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    LOAD(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
    LOAD(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);
    LOAD(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    LOAD(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);

    LOAD(PFNGLGENTEXTURESPROC, glGenTextures);
    LOAD(PFNGLBINDTEXTUREPROC, glBindTexture);
    LOAD(PFNGLTEXIMAGE2DPROC, glTexImage2D);
    LOAD(PFNGLTEXPARAMETERIPROC, glTexParameteri);
    LOAD(PFNGLDELETETEXTURESPROC, glDeleteTextures);
    LOAD(PFNGLACTIVETEXTUREPROC, glActiveTexture);

    LOAD(PFNGLDRAWARRAYSPROC, glDrawArrays);
    LOAD(PFNGLDRAWELEMENTSPROC, glDrawElements);
    LOAD(PFNGLLINEWIDTHPROC, glLineWidth);

    LOAD(PFNGLGETINTEGERVPROC, glGetIntegerv);

    LOAD(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
    LOAD(PFNGLUNIFORM1IPROC, glUniform1i);
    LOAD(PFNGLUNIFORM1FPROC, glUniform1f);
    LOAD(PFNGLUNIFORM4FPROC, glUniform4f);
    LOAD(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);

#undef LOAD

    return ok;
}
