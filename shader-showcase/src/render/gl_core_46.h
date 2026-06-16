#pragma once

// Minimal OpenGL 4.6 Core function loader for Windows.
// Provides all GL 4.6 types, constants, and function declarations
// that are missing from the Windows SDK's <gl/GL.h> (OpenGL 1.1 only).

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <gl/GL.h>

// ---- Missing OpenGL types (Windows gl/GL.h only has GL 1.1 types) ----------

#ifndef GL_GLEXT_VERSION  // Only define if not already provided
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char      GLchar;
#endif

// ---- Missing APIENTRY for GLFW compatibility --------------------------------
// GLFW's glfw3.h defines APIENTRY as __stdcall; minwindef.h also defines it.
// We need it for our function pointer types.

// ---- Missing OpenGL constants (not in Windows gl/GL.h) ----------------------

#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_TEXTURE0                       0x84C0
#define GL_CLAMP_TO_EDGE                 0x812F
#define GL_FRAMEBUFFER                    0x8D40
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_SHADER_BINARY_FORMAT_SPIR_V    0x9551
#define GL_RGBA8                          0x8058
#define GL_RGBA32F                        0x8814
#define GL_R8                             0x8229
#define GL_RED                            0x1903
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_INVALID_INDEX                  0xFFFFFFFFu
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_BLOCKS           0x8A36
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS          0x8A43
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES   0x8A44
#define GL_UNIFORM_OFFSET                         0x1006
#define GL_UNIFORM_SIZE                           0x1004
#define GL_UNIFORM_TYPE                           0x1005
#define GL_FLOAT_VEC2                             0x8B50
#define GL_DYNAMIC_DRAW                           0x88E8
#define GL_ELEMENT_ARRAY_BUFFER                   0x8893

// ---- Our own function pointer typedefs -------------------------------------

typedef void   (APIENTRY *PFN_glActiveTexture)(GLenum);
typedef void   (APIENTRY *PFN_glGenBuffers)(GLsizei, GLuint*);
typedef void   (APIENTRY *PFN_glBindBuffer)(GLenum, GLuint);
typedef void   (APIENTRY *PFN_glBufferData)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void   (APIENTRY *PFN_glDeleteBuffers)(GLsizei, const GLuint*);
typedef GLuint (APIENTRY *PFN_glCreateShader)(GLenum);
typedef void   (APIENTRY *PFN_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef void   (APIENTRY *PFN_glCompileShader)(GLuint);
typedef void   (APIENTRY *PFN_glGetShaderiv)(GLuint, GLenum, GLint*);
typedef void   (APIENTRY *PFN_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (APIENTRY *PFN_glDeleteShader)(GLuint);
typedef GLuint (APIENTRY *PFN_glCreateProgram)(void);
typedef void   (APIENTRY *PFN_glAttachShader)(GLuint, GLuint);
typedef void   (APIENTRY *PFN_glLinkProgram)(GLuint);
typedef void   (APIENTRY *PFN_glGetProgramiv)(GLuint, GLenum, GLint*);
typedef void   (APIENTRY *PFN_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (APIENTRY *PFN_glUseProgram)(GLuint);
typedef void   (APIENTRY *PFN_glDeleteProgram)(GLuint);
typedef void   (APIENTRY *PFN_glEnableVertexAttribArray)(GLuint);
typedef void   (APIENTRY *PFN_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void   (APIENTRY *PFN_glDisableVertexAttribArray)(GLuint);
typedef GLint  (APIENTRY *PFN_glGetUniformLocation)(GLuint, const GLchar*);
typedef void   (APIENTRY *PFN_glUniform1i)(GLint, GLint);
typedef void   (APIENTRY *PFN_glUniform1f)(GLint, GLfloat);
typedef void   (APIENTRY *PFN_glUniform2f)(GLint, GLfloat, GLfloat);
typedef void   (APIENTRY *PFN_glUniform1ui)(GLint, GLuint);
typedef void   (APIENTRY *PFN_glGenVertexArrays)(GLsizei, GLuint*);
typedef void   (APIENTRY *PFN_glBindVertexArray)(GLuint);
typedef void   (APIENTRY *PFN_glDeleteVertexArrays)(GLsizei, const GLuint*);
typedef void   (APIENTRY *PFN_glGenFramebuffers)(GLsizei, GLuint*);
typedef void   (APIENTRY *PFN_glBindFramebuffer)(GLenum, GLuint);
typedef void   (APIENTRY *PFN_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRY *PFN_glCheckFramebufferStatus)(GLenum);
typedef void   (APIENTRY *PFN_glDeleteFramebuffers)(GLsizei, const GLuint*);
typedef const GLubyte* (APIENTRY *PFN_glGetStringi)(GLenum, GLuint);
typedef void   (APIENTRY *PFN_glShaderBinary)(GLsizei, const GLuint*, GLenum, const void*, GLsizei);
typedef void   (APIENTRY *PFN_glSpecializeShader)(GLuint, const GLchar*, GLuint, const GLuint*, const GLuint*);
typedef GLuint (APIENTRY *PFN_glGetUniformBlockIndex)(GLuint, const GLchar*);
typedef void   (APIENTRY *PFN_glUniformBlockBinding)(GLuint, GLuint, GLuint);
typedef void   (APIENTRY *PFN_glBindBufferBase)(GLenum, GLuint, GLuint);
typedef void   (APIENTRY *PFN_glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint*);
typedef void   (APIENTRY *PFN_glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint*, GLenum, GLint*);
typedef void   (APIENTRY *PFN_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
typedef void   (APIENTRY *PFN_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void*);
typedef void   (APIENTRY *PFN_glDetachShader)(GLuint, GLuint);

// ---- Extern declarations --------------------------------------------------

extern PFN_glActiveTexture            glActiveTexture;
extern PFN_glGenBuffers               glGenBuffers;
extern PFN_glBindBuffer               glBindBuffer;
extern PFN_glBufferData               glBufferData;
extern PFN_glDeleteBuffers            glDeleteBuffers;
extern PFN_glCreateShader             glCreateShader;
extern PFN_glShaderSource             glShaderSource;
extern PFN_glCompileShader            glCompileShader;
extern PFN_glGetShaderiv              glGetShaderiv;
extern PFN_glGetShaderInfoLog         glGetShaderInfoLog;
extern PFN_glDeleteShader             glDeleteShader;
extern PFN_glCreateProgram            glCreateProgram;
extern PFN_glAttachShader             glAttachShader;
extern PFN_glLinkProgram              glLinkProgram;
extern PFN_glGetProgramiv             glGetProgramiv;
extern PFN_glGetProgramInfoLog        glGetProgramInfoLog;
extern PFN_glUseProgram               glUseProgram;
extern PFN_glDeleteProgram            glDeleteProgram;
extern PFN_glEnableVertexAttribArray  glEnableVertexAttribArray;
extern PFN_glVertexAttribPointer      glVertexAttribPointer;
extern PFN_glDisableVertexAttribArray glDisableVertexAttribArray;
extern PFN_glGetUniformLocation       glGetUniformLocation;
extern PFN_glUniform1i                glUniform1i;
extern PFN_glUniform1f                glUniform1f;
extern PFN_glUniform2f                glUniform2f;
extern PFN_glUniform1ui               glUniform1ui;
extern PFN_glGenVertexArrays          glGenVertexArrays;
extern PFN_glBindVertexArray          glBindVertexArray;
extern PFN_glDeleteVertexArrays       glDeleteVertexArrays;
extern PFN_glGenFramebuffers          glGenFramebuffers;
extern PFN_glBindFramebuffer          glBindFramebuffer;
extern PFN_glFramebufferTexture2D     glFramebufferTexture2D;
extern PFN_glCheckFramebufferStatus   glCheckFramebufferStatus;
extern PFN_glDeleteFramebuffers       glDeleteFramebuffers;
extern PFN_glGetStringi               glGetStringi;
extern PFN_glShaderBinary             glShaderBinary;
extern PFN_glSpecializeShader         glSpecializeShader;
extern PFN_glGetUniformBlockIndex     glGetUniformBlockIndex;
extern PFN_glUniformBlockBinding      glUniformBlockBinding;
extern PFN_glBindBufferBase           glBindBufferBase;
extern PFN_glGetActiveUniformBlockiv  glGetActiveUniformBlockiv;
extern PFN_glGetActiveUniformsiv       glGetActiveUniformsiv;
extern PFN_glGetActiveUniform         glGetActiveUniform;
extern PFN_glBufferSubData            glBufferSubData;
extern PFN_glDetachShader             glDetachShader;

// ---- Additional GL functions needed by OpenGLBackend -------------------------
#define GL_READ_FRAMEBUFFER                   0x8CA8
#define GL_DRAW_FRAMEBUFFER                  0x8CA9
#define GL_COLOR_BUFFER_BIT                  0x00004000
#define GL_FRAMEBUFFER                       0x8D40
#define GL_INVALID_INDEX                     0xFFFFFFFFu

typedef void   (APIENTRY *PFN_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void   (APIENTRY *PFN_glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
typedef void   (APIENTRY *PFN_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef void   (APIENTRY *PFN_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void   (APIENTRY *PFN_glDisableVertexAttribArray)(GLuint);

extern PFN_glUniformMatrix4fv        glUniformMatrix4fv;
extern PFN_glBlitFramebuffer         glBlitFramebuffer;
extern PFN_glFramebufferTexture2D    glFramebufferTexture2D;
extern PFN_glVertexAttribPointer     glVertexAttribPointer;
extern PFN_glDisableVertexAttribArray glDisableVertexAttribArray;

// ---- Loader ----------------------------------------------------------------
struct GLFWwindow;
bool LoadGL46Functions(GLFWwindow* window);
