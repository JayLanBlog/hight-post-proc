#include "gl_core_46.h"

#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>

// Helper macro so we don't mistype the name string
#define LOAD_PROC(name)                                           \
    do {                                                          \
        name = (decltype(name))glfwGetProcAddress(#name);         \
        if (!name) {                                              \
            fprintf(stderr, "[GL Loader] Failed to load: %s\n",   \
                    #name);                                       \
            return false;                                         \
        }                                                         \
    } while (0)

// ---- GL 1.3 ----------------------------------------------------------------
PFN_glActiveTexture glActiveTexture = nullptr;

// ---- GL 1.5 ----------------------------------------------------------------
PFN_glGenBuffers    glGenBuffers    = nullptr;
PFN_glBindBuffer    glBindBuffer    = nullptr;
PFN_glBufferData    glBufferData    = nullptr;
PFN_glDeleteBuffers glDeleteBuffers = nullptr;

// ---- GL 2.0 ----------------------------------------------------------------
PFN_glCreateShader             glCreateShader             = nullptr;
PFN_glShaderSource             glShaderSource             = nullptr;
PFN_glCompileShader            glCompileShader            = nullptr;
PFN_glGetShaderiv              glGetShaderiv              = nullptr;
PFN_glGetShaderInfoLog         glGetShaderInfoLog         = nullptr;
PFN_glDeleteShader             glDeleteShader             = nullptr;
PFN_glCreateProgram            glCreateProgram            = nullptr;
PFN_glAttachShader             glAttachShader             = nullptr;
PFN_glLinkProgram              glLinkProgram              = nullptr;
PFN_glGetProgramiv             glGetProgramiv             = nullptr;
PFN_glGetProgramInfoLog        glGetProgramInfoLog        = nullptr;
PFN_glUseProgram               glUseProgram               = nullptr;
PFN_glDeleteProgram            glDeleteProgram            = nullptr;
PFN_glEnableVertexAttribArray  glEnableVertexAttribArray  = nullptr;
PFN_glVertexAttribPointer      glVertexAttribPointer      = nullptr;
PFN_glDisableVertexAttribArray glDisableVertexAttribArray = nullptr;
PFN_glGetUniformLocation       glGetUniformLocation       = nullptr;
PFN_glUniform1i                glUniform1i                = nullptr;
PFN_glUniform1f                glUniform1f                = nullptr;
PFN_glUniform2f                glUniform2f                = nullptr;
PFN_glUniform1ui               glUniform1ui               = nullptr;

// ---- GL 3.0 ----------------------------------------------------------------
PFN_glGenVertexArrays        glGenVertexArrays        = nullptr;
PFN_glBindVertexArray        glBindVertexArray        = nullptr;
PFN_glDeleteVertexArrays     glDeleteVertexArrays     = nullptr;
PFN_glGenFramebuffers        glGenFramebuffers        = nullptr;
PFN_glBindFramebuffer        glBindFramebuffer        = nullptr;
PFN_glFramebufferTexture2D   glFramebufferTexture2D   = nullptr;
PFN_glCheckFramebufferStatus glCheckFramebufferStatus = nullptr;
PFN_glDeleteFramebuffers     glDeleteFramebuffers     = nullptr;
PFN_glGetStringi             glGetStringi             = nullptr;

// ---- GL 4.1 (ARB_gl_spirv) -------------------------------------------------
PFN_glShaderBinary     glShaderBinary     = nullptr;
PFN_glSpecializeShader glSpecializeShader = nullptr;

// ---- GL 3.1 (UBO) ----------------------------------------------------------
PFN_glGetUniformBlockIndex     glGetUniformBlockIndex     = nullptr;
PFN_glUniformBlockBinding      glUniformBlockBinding      = nullptr;
PFN_glBindBufferBase           glBindBufferBase           = nullptr;
PFN_glGetActiveUniformBlockiv  glGetActiveUniformBlockiv  = nullptr;
PFN_glGetActiveUniformsiv       glGetActiveUniformsiv       = nullptr;
PFN_glGetActiveUniform         glGetActiveUniform         = nullptr;
PFN_glBufferSubData            glBufferSubData            = nullptr;
PFN_glDetachShader             glDetachShader             = nullptr;

// ---- GL 2.0 (matrix uniforms) -----------------------------------------------
PFN_glUniformMatrix4fv        glUniformMatrix4fv        = nullptr;

// ---- GL 3.0 (blit) -----------------------------------------------------------
PFN_glBlitFramebuffer         glBlitFramebuffer         = nullptr;

// ---- Loader ----------------------------------------------------------------
bool LoadGL46Functions(GLFWwindow* window) {
    (void)window;

    // GL 1.3
    LOAD_PROC(glActiveTexture);

    // GL 1.5
    LOAD_PROC(glGenBuffers);
    LOAD_PROC(glBindBuffer);
    LOAD_PROC(glBufferData);
    LOAD_PROC(glDeleteBuffers);

    // GL 2.0
    LOAD_PROC(glCreateShader);
    LOAD_PROC(glShaderSource);
    LOAD_PROC(glCompileShader);
    LOAD_PROC(glGetShaderiv);
    LOAD_PROC(glGetShaderInfoLog);
    LOAD_PROC(glDeleteShader);
    LOAD_PROC(glCreateProgram);
    LOAD_PROC(glAttachShader);
    LOAD_PROC(glLinkProgram);
    LOAD_PROC(glGetProgramiv);
    LOAD_PROC(glGetProgramInfoLog);
    LOAD_PROC(glUseProgram);
    LOAD_PROC(glDeleteProgram);
    LOAD_PROC(glEnableVertexAttribArray);
    LOAD_PROC(glVertexAttribPointer);
    LOAD_PROC(glDisableVertexAttribArray);
    LOAD_PROC(glGetUniformLocation);
    LOAD_PROC(glUniform1i);
    LOAD_PROC(glUniform1f);
    LOAD_PROC(glUniform2f);
    LOAD_PROC(glUniform1ui);

    // GL 3.0
    LOAD_PROC(glGenVertexArrays);
    LOAD_PROC(glBindVertexArray);
    LOAD_PROC(glDeleteVertexArrays);
    LOAD_PROC(glGenFramebuffers);
    LOAD_PROC(glBindFramebuffer);
    LOAD_PROC(glFramebufferTexture2D);
    LOAD_PROC(glCheckFramebufferStatus);
    LOAD_PROC(glDeleteFramebuffers);
    LOAD_PROC(glGetStringi);

    // GL 4.1 (ARB_gl_spirv)
    LOAD_PROC(glShaderBinary);
    LOAD_PROC(glSpecializeShader);

    // GL 3.1 (UBO)
    LOAD_PROC(glGetUniformBlockIndex);
    LOAD_PROC(glUniformBlockBinding);
    LOAD_PROC(glBindBufferBase);
    LOAD_PROC(glGetActiveUniformBlockiv);
    LOAD_PROC(glGetActiveUniformsiv);
    LOAD_PROC(glGetActiveUniform);

    // GL 1.5 additional
    LOAD_PROC(glBufferSubData);

    // GL 2.0 additional
    LOAD_PROC(glDetachShader);

    // GL 2.0 (matrix uniforms)
    LOAD_PROC(glUniformMatrix4fv);

    // GL 3.0 (blit)
    LOAD_PROC(glBlitFramebuffer);

    printf("[GL Loader] All OpenGL 4.6 function pointers loaded.\n");
    return true;
}
