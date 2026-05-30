#include "render/OpenGLBackend.h"
#include "render/gl_core_46.h"
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

// ImGui headers
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// ============================================================================
// Uniform buffer layout matching SPIR-V shaders (std140)
// ============================================================================
#pragma pack(push, 1)
struct UniformData {
    float uParamFloat0;       // offset 0
    float uParamFloat1;       // offset 4
    float uParamFloat2;       // offset 8
    float uParamFloat3;       // offset 12
    float uParamFloat4;       // offset 16
    float uParamFloat5;       // offset 20
    float uResolution[2];     // offset 24 (vec2, 8-byte aligned)
    float uTime;              // offset 32
    float uFrameCount;        // offset 36
    // Padding to 48 bytes (std140 rounds up to vec4 = 16-byte boundary)
    float padding[3];         // offset 40, 3 floats = 12 bytes
}; // total: 48 bytes
#pragma pack(pop)

// ============================================================================
// Fullscreen quad vertex data
// ============================================================================
static const float kQuadVertices[] = {
    // Position (2D)    // UV coords
    -1.0f, -1.0f,       0.0f, 0.0f,
     1.0f, -1.0f,       1.0f, 0.0f,
     1.0f,  1.0f,       1.0f, 1.0f,
    -1.0f,  1.0f,       0.0f, 1.0f
};

static const unsigned int kQuadIndices[] = {
    0, 1, 2,
    0, 2, 3
};

static constexpr int kFullscreenQuadVertexCount = 6;

// ============================================================================
// Helper macros
// ============================================================================
#define GL_CHECK(x) do { x; GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "[OpenGL] Error %d at %s:%d\n", err, __FILE__, __LINE__); } } while(0)

// ============================================================================
// Lifecycle
// ============================================================================
bool OpenGLBackend::Init(GLFWwindow* window) {
    m_window = window;
    if (!m_window) {
        fprintf(stderr, "[OpenGL] Invalid window handle\n");
        return false;
    }

    // Make context current
    glfwMakeContextCurrent(m_window);

    // Load OpenGL function pointers via glad
    if (!LoadGL46Functions(m_window)) {
        fprintf(stderr, "[OpenGL] Failed to initialize GLAD\n");
        return false;
    }

    // Get initial framebuffer size
    glfwGetFramebufferSize(m_window, &m_width, &m_height);

    // Get the default FBO (the one GLFW created for us)
    GLint defaultFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFBO);
    m_defaultFBO = static_cast<GLuint>(defaultFBO);
    m_currentFBO = m_defaultFBO;

    // Create temp UBO for per-draw uniform data (fallback path)
    glGenBuffers(1, &m_tempUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Setup fullscreen quad VAO/VBO
    SetupQuadVAO();

    // Set default OpenGL state
    BindDefaultState();

    printf("[OpenGL] Initialized - GL Version: %s\n", (const char*)glGetString(GL_VERSION));
    return true;
}

void OpenGLBackend::Shutdown() {
    // Cleanup shaders
    for (auto& [id, shader] : m_shaders) {
        if (shader != 0) {
            glDeleteShader(shader);
        }
    }
    m_shaders.clear();

    // Cleanup programs
    for (auto& [key, program] : m_programCache) {
        if (program != 0) {
            glDeleteProgram(program);
        }
    }
    m_programCache.clear();

    // Cleanup framebuffers
    for (auto& [id, fbo] : m_framebuffers) {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }
    m_framebuffers.clear();

    // Cleanup textures
    for (auto& [id, tex] : m_textures) {
        if (tex != 0) {
            glDeleteTextures(1, &tex);
        }
    }
    m_textures.clear();
    m_textureFormats.clear();
    m_texWidths.clear();
    m_texHeights.clear();

    // Cleanup quad
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }

    // Cleanup UBO
    if (m_tempUBO != 0) {
        glDeleteBuffers(1, &m_tempUBO);
        m_tempUBO = 0;
    }

    printf("[OpenGL] Shutdown\n");
}

void OpenGLBackend::BeginFrame() {
    // Clear the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::EndFrame() {
    // Present to screen
    glfwSwapBuffers(m_window);
}

void OpenGLBackend::WaitIdle() {
    // OpenGL is immediate mode, but glFinish ensures all commands complete
    glFinish();
}

// ============================================================================
// Viewport
// ============================================================================
void OpenGLBackend::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void OpenGLBackend::GetFramebufferSize(int& width, int& height) {
    width = m_width;
    height = m_height;
}

// ============================================================================
// Shaders
// ============================================================================
ShaderHandle OpenGLBackend::CreateVertexShader(const uint32_t* spirv, size_t size) {
    if (spirv == nullptr || size == 0) {
        fprintf(stderr, "[OpenGL] Empty SPIR-V data for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    // Load SPIR-V binary
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv, static_cast<GLsizei>(size * sizeof(uint32_t)));

    // Specialize with default entry point "main"
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader SPIR-V specialization failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}

ShaderHandle OpenGLBackend::CreateFragmentShader(const uint32_t* spirv, size_t size) {
    if (spirv == nullptr || size == 0) {
        fprintf(stderr, "[OpenGL] Empty SPIR-V data for fragment shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create fragment shader object\n");
        return INVALID_SHADER;
    }

    // Load SPIR-V binary
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv, static_cast<GLsizei>(size * sizeof(uint32_t)));

    // Specialize with default entry point "main"
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Fragment shader SPIR-V specialization failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}

ShaderHandle OpenGLBackend::CreateVertexShaderFromGLSL(const std::string& glslSource) {
    if (glslSource.empty()) {
        fprintf(stderr, "[OpenGL] Empty GLSL source for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    const char* source = glslSource.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}

ShaderHandle OpenGLBackend::CreateFragmentShaderFromGLSL(const std::string& glslSource) {
    if (glslSource.empty()) {
        fprintf(stderr, "[OpenGL] Empty GLSL source for fragment shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create fragment shader object\n");
        return INVALID_SHADER;
    }

    const char* source = glslSource.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Fragment shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}

void OpenGLBackend::DestroyShader(ShaderHandle handle) {
    if (handle.id == 0) return;

    auto it = m_shaders.find(handle.id);
    if (it != m_shaders.end()) {
        // Remove any cached programs that use this shader
        std::vector<uint64_t> keysToRemove;
        for (auto& [key, program] : m_programCache) {
            uint32_t vsId = static_cast<uint32_t>(key >> 32);
            uint32_t fsId = static_cast<uint32_t>(key & 0xFFFFFFFF);
            if (vsId == handle.id || fsId == handle.id) {
                glDeleteProgram(program);
                keysToRemove.push_back(key);
            }
        }
        for (auto key : keysToRemove) {
            m_programCache.erase(key);
        }

        glDeleteShader(it->second);
        m_shaders.erase(it);
    }
}

// ============================================================================
// Textures
// ============================================================================
TextureHandle OpenGLBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    GLuint texture;
    glGenTextures(1, &texture);
    if (texture == 0) {
        fprintf(stderr, "[OpenGL] Failed to create texture\n");
        return INVALID_TEXTURE;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate texture storage
    GLint internalFormat = GLInternalFormat(format);
    GLenum glFormat = GLFormat(format);
    GLenum glType = GLType(format);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, glType, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t id = m_nextTextureId++;
    m_textures[id] = texture;
    m_textureFormats[id] = format;
    m_texWidths[id] = width;
    m_texHeights[id] = height;

    return TextureHandle{id};
}

void OpenGLBackend::DestroyTexture(TextureHandle handle) {
    if (handle.id == 0) return;

    auto it = m_textures.find(handle.id);
    if (it != m_textures.end()) {
        // Remove associated framebuffer if exists
        auto fboIt = m_framebuffers.find(handle.id);
        if (fboIt != m_framebuffers.end()) {
            glDeleteFramebuffers(1, &fboIt->second);
            m_framebuffers.erase(fboIt);
        }

        glDeleteTextures(1, &it->second);
        m_textures.erase(it);
        m_textureFormats.erase(handle.id);
        m_texWidths.erase(handle.id);
        m_texHeights.erase(handle.id);
    }
}

void OpenGLBackend::UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) {
    if (handle.id == 0 || !data) return;

    GLuint tex = GetGLTexture(handle);
    if (tex == 0) return;

    auto it = m_textureFormats.find(handle.id);
    if (it == m_textureFormats.end()) return;

    TextureFormat format = it->second;
    GLenum glFormat = GLFormat(format);
    GLenum glType = GLType(format);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, glFormat, glType, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Update stored dimensions if updating full texture
    if (x == 0 && y == 0) {
        m_texWidths[handle.id] = width;
        m_texHeights[handle.id] = height;
    }
}

// ============================================================================
// ImGui helpers
// ============================================================================
void* OpenGLBackend::GetImTextureID(TextureHandle handle) {
    GLuint tex = GetGLTexture(handle);
    return reinterpret_cast<void*>(static_cast<uintptr_t>(tex));
}

void OpenGLBackend::SaveScreenshot(const char* path) const {
    if (!path) return;

    std::vector<uint8_t> pixels(m_width * m_height * 3);
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip vertically (OpenGL reads bottom-to-top)
    std::vector<uint8_t> flipped(m_width * m_height * 3);
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width * 3; ++x) {
            flipped[y * m_width * 3 + x] = pixels[(m_height - 1 - y) * m_width * 3 + x];
        }
    }

    // Write PPM file
    FILE* fp = fopen(path, "wb");
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", m_width, m_height);
        fwrite(flipped.data(), 1, m_width * m_height * 3, fp);
        fclose(fp);
        printf("[OpenGL] Screenshot saved to %s\n", path);
    } else {
        fprintf(stderr, "[OpenGL] Failed to open file for screenshot: %s\n", path);
    }
}

// ============================================================================
// Fullscreen quad
// ============================================================================
void OpenGLBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    GLuint vs = GetGLShader(vert);
    GLuint fs = GetGLShader(frag);

    if (vs == 0 || fs == 0) {
        fprintf(stderr, "[OpenGL] Invalid shader handles in DrawFullscreenQuad vs=%u fs=%u\n", vs, fs);
        return;
    }

    GLuint program = GetOrCreateProgram(vs, fs);
    if (program == 0) {
        fprintf(stderr, "[OpenGL] Failed to create shader program\n");
        return;
    }

    glUseProgram(program);

    // Reset ALL GL state that ImGui may have changed
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
    glViewport(0, 0, m_width, m_height);

    // Bind input textures (up to 8)
    for (size_t i = 0; i < params.inputTextures.size() && i < 8; ++i) {
        GLuint tex = GetGLTexture(params.inputTextures[i]);
        if (tex != 0) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
            glBindTexture(GL_TEXTURE_2D, tex);

            // Try to set sampler uniform
            char uniformName[32];
            snprintf(uniformName, sizeof(uniformName), "uTexture%d", static_cast<int>(i));
            GLint loc = glGetUniformLocation(program, uniformName);
            if (loc >= 0) {
                glUniform1i(loc, static_cast<GLint>(i));
            }

            // Also try "uInputTex" for single-texture shaders
            if (i == 0) {
                loc = glGetUniformLocation(program, "uInputTex");
                if (loc >= 0) {
                    glUniform1i(loc, 0);
                }
            }
        }
    }

    // Fill uniform data structure
    UniformData data = {};
    if (params.uniformFloats.size() > 0) data.uParamFloat0 = params.uniformFloats[0];
    if (params.uniformFloats.size() > 1) data.uParamFloat1 = params.uniformFloats[1];
    if (params.uniformFloats.size() > 2) data.uParamFloat2 = params.uniformFloats[2];
    if (params.uniformFloats.size() > 3) data.uParamFloat3 = params.uniformFloats[3];
    if (params.uniformFloats.size() > 4) data.uParamFloat4 = params.uniformFloats[4];
    if (params.uniformFloats.size() > 5) data.uParamFloat5 = params.uniformFloats[5];

    data.uResolution[0] = static_cast<float>(params.viewportWidth);
    data.uResolution[1] = static_cast<float>(params.viewportHeight);
    data.uTime = params.time;
    data.uFrameCount = static_cast<float>(params.frameCount);

    // ---- Check if program has UBO ----
    GLint nb=0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);

    if (nb > 0) {
        // ---- UBO path (SPIR-V or NVIDIA-baked) ----
        // Ensure the "Params" uniform block is bound to binding point 1
        GLuint blockIndex = glGetUniformBlockIndex(program, "Params");
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, blockIndex, 1);
        }

        const size_t UBO_SIZE = 48;
        uint8_t ubo[UBO_SIZE] = {};
        for (size_t i = 0; i < params.uniformFloats.size() && i < 6; ++i) {
            float v = params.uniformFloats[i];
            memcpy(ubo + i * 4, &v, sizeof(float));
        }
        { float r[2]={(float)params.viewportWidth,(float)params.viewportHeight}; memcpy(ubo+24,r,8); }
        { memcpy(ubo+32,&params.time,4); float fc=(float)params.frameCount; memcpy(ubo+36,&fc,4); }
        glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
        glBufferData(GL_UNIFORM_BUFFER, UBO_SIZE, ubo, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_tempUBO);
    } else {
        // ---- Individual uniform path (GLSL) ----
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0); // unbind any UBO from previous draw
        GLint loc;

        // Helper: try bare name first, then Params. prefix (named block uniforms)
        auto getLoc = [&](const char* name) -> GLint {
            GLint l = glGetUniformLocation(program, name);
            if (l >= 0) return l;
            char buf[64];
            snprintf(buf, sizeof(buf), "Params.%s", name);
            return glGetUniformLocation(program, buf);
        };

        if (params.uniformFloats.size() > 0) {
            loc = getLoc("uParamFloat0");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[0]);
        }
        if (params.uniformFloats.size() > 1) {
            loc = getLoc("uParamFloat1");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[1]);
        }
        if (params.uniformFloats.size() > 2) {
            loc = getLoc("uParamFloat2");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[2]);
        }
        if (params.uniformFloats.size() > 3) {
            loc = getLoc("uParamFloat3");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[3]);
        }
        if (params.uniformFloats.size() > 4) {
            loc = getLoc("uParamFloat4");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[4]);
        }
        if (params.uniformFloats.size() > 5) {
            loc = getLoc("uParamFloat5");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[5]);
        }
        loc = getLoc("uResolution");
        if (loc >= 0) glUniform2f(loc, (float)params.viewportWidth, (float)params.viewportHeight);
        loc = getLoc("uTime");
        if (loc >= 0) glUniform1f(loc, params.time);
        loc = getLoc("uFrameCount");
        if (loc >= 0) glUniform1ui(loc, params.frameCount);
    }

    // Draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, kFullscreenQuadVertexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // Cleanup
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}

// ============================================================================
// Cards (3D card rendering)
// ============================================================================
void OpenGLBackend::DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) {
    if (cards.empty() || !viewMat || !projMat) return;

    // Build a simple card shader if needed (hardcoded GLSL)
    static GLuint cardProgram = 0;
    if (cardProgram == 0) {
        const char* cardVS = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec2 aTexCoord;
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProj;
            out vec2 vTexCoord;
            out float vOpacity;
            void main() {
                gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
                vTexCoord = aTexCoord;
            }
        )";
        const char* cardFS = R"(
            #version 460 core
            in vec2 vTexCoord;
            uniform sampler2D uCardTexture;
            uniform float uOpacity;
            out vec4 FragColor;
            void main() {
                vec4 texColor = texture(uCardTexture, vTexCoord);
                FragColor = vec4(texColor.rgb, texColor.a * uOpacity);
            }
        )";

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &cardVS, nullptr);
        glCompileShader(vs);
        GLint success = 0;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(vs, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card vertex shader failed: %s\n", log);
            glDeleteShader(vs);
            return;
        }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &cardFS, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(fs, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card fragment shader failed: %s\n", log);
            glDeleteShader(vs);
            glDeleteShader(fs);
            return;
        }

        cardProgram = glCreateProgram();
        glAttachShader(cardProgram, vs);
        glAttachShader(cardProgram, fs);
        glLinkProgram(cardProgram);
        glGetProgramiv(cardProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(cardProgram, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card program link failed: %s\n", log);
            glDeleteProgram(cardProgram);
            cardProgram = 0;
            glDeleteShader(vs);
            glDeleteShader(fs);
            return;
        }
        glDetachShader(cardProgram, vs);
        glDetachShader(cardProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    glUseProgram(cardProgram);

    // Set view and projection matrices
    GLint viewLoc = glGetUniformLocation(cardProgram, "uView");
    GLint projLoc = glGetUniformLocation(cardProgram, "uProj");
    if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);
    if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMat);

    // Card quad vertices (position + UV)
    static const float cardVerts[] = {
        // pos x, y, z    uv u, v
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
    };
    static const unsigned int cardIdx[] = { 0, 1, 2, 0, 2, 3 };

    // Create a temporary VAO for card rendering
    GLuint cardVAO, cardVBO, cardEBO;
    glGenVertexArrays(1, &cardVAO);
    glGenBuffers(1, &cardVBO);
    glGenBuffers(1, &cardEBO);

    glBindVertexArray(cardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cardVerts), cardVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cardEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cardIdx), cardIdx, GL_STATIC_DRAW);

    // Position attribute (location 0): vec3
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV attribute (location 1): vec2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Set texture sampler
    GLint texLoc = glGetUniformLocation(cardProgram, "uCardTexture");
    if (texLoc >= 0) glUniform1i(texLoc, 0);

    GLint modelLoc = glGetUniformLocation(cardProgram, "uModel");
    GLint opacityLoc = glGetUniformLocation(cardProgram, "uOpacity");

    for (const auto& card : cards) {
        GLuint tex = GetGLTexture(card.texture);
        if (tex == 0) continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // Build model matrix: translate * rotateY * scale
        float model[16] = {};

        // Start with identity
        model[0] = model[5] = model[10] = model[15] = 1.0f;

        // Apply scale
        model[0]  *= card.scaleX;
        model[5]  *= card.scaleY;

        // Apply rotation around Y axis
        float cosR = std::cos(card.rotationY);
        float sinR = std::sin(card.rotationY);
        float rotMat[16] = {};
        rotMat[0]  =  cosR;
        rotMat[2]  =  sinR;
        rotMat[5]  =  1.0f;
        rotMat[8]  = -sinR;
        rotMat[10] =  cosR;
        rotMat[15] =  1.0f;

        // Multiply model = rotMat * model
        float result[16] = {};
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                for (int k = 0; k < 4; ++k) {
                    result[r * 4 + c] += rotMat[r * 4 + k] * model[k * 4 + c];
                }
            }
        }

        // Apply translation
        result[12] += card.posX;
        result[13] += card.posY;
        result[14] += card.posZ;

        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, result);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, card.opacity);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    // Cleanup temporary card geometry
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &cardVAO);
    glDeleteBuffers(1, &cardVBO);
    glDeleteBuffers(1, &cardEBO);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}

// ============================================================================
// Blit
// ============================================================================
void OpenGLBackend::BlitToScreen(TextureHandle texture) {
    GLuint fbo = GetGLFramebuffer(texture);
    if (fbo == 0) {
        fprintf(stderr, "[OpenGL] Invalid texture/FBO in BlitToScreen\n");
        return;
    }

    // Get texture dimensions
    auto wIt = m_texWidths.find(texture.id);
    auto hIt = m_texHeights.find(texture.id);
    int texW = (wIt != m_texWidths.end()) ? wIt->second : m_width;
    int texH = (hIt != m_texHeights.end()) ? hIt->second : m_height;

    // Bind source FBO for reading
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    // Bind default FBO for drawing
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_defaultFBO);
    m_currentFBO = m_defaultFBO;

    // Blit from texture FBO to default FBO
    glBlitFramebuffer(0, 0, texW, texH,
                       0, 0, m_width, m_height,
                       GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Restore state
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

// ============================================================================
// Render targets
// ============================================================================
void OpenGLBackend::BeginRenderToTexture(TextureHandle target) {
    GLuint fbo = GetGLFramebuffer(target);
    if (fbo == 0) {
        fprintf(stderr, "[OpenGL] Failed to get/create framebuffer for texture\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    m_currentFBO = fbo;

    // Get texture dimensions for viewport
    auto wIt = m_texWidths.find(target.id);
    auto hIt = m_texHeights.find(target.id);
    if (wIt != m_texWidths.end() && hIt != m_texHeights.end()) {
        glViewport(0, 0, wIt->second, hIt->second);
    }
}

void OpenGLBackend::EndRenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    m_currentFBO = m_defaultFBO;
    glViewport(0, 0, m_width, m_height);
}

// ============================================================================
// ImGui
// ============================================================================
void OpenGLBackend::ImGuiInit(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Load fonts: default Latin font + Chinese font merge
    // Try Windows system fonts first
    const char* chineseFontPath = nullptr;
    const char* latinFontPath = nullptr;

    // Check for Microsoft YaHei (msyh.ttc) - best CJK coverage on Windows
    if (GetFileAttributesA("C:\\Windows\\Fonts\\msyh.ttc") != INVALID_FILE_ATTRIBUTES) {
        chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }

    if (chineseFontPath) {
        // Load default ImGui font first (covers Latin glyphs)
        io.Fonts->AddFontDefault();

        // Merge Chinese glyphs on top of default font
        ImFontConfig cfg;
        cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());
    }
    // If no Chinese font found, use ImGui default (ProggyClean)

    // Initialize ImGui GLFW backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // Initialize ImGui OpenGL3 backend
    ImGui_ImplOpenGL3_Init("#version 460 core");
}

void OpenGLBackend::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OpenGLBackend::ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLBackend::ImGuiShutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// ============================================================================
// Pipelines (OpenGL doesn't use pipeline objects, but we implement for interface compatibility)
// ============================================================================
PipelineHandle OpenGLBackend::CreatePipeline(const PipelineDesc& desc) {
    // OpenGL doesn't have pipeline objects like Vulkan
    // We just return a handle that combines the shaders
    // The actual program is created lazily in DrawFullscreenQuad
    uint32_t id = (desc.vertShader.id << 16) | desc.fragShader.id;
    return PipelineHandle{id};
}

void OpenGLBackend::DestroyPipeline(PipelineHandle handle) {
    // Nothing to destroy in OpenGL - programs are cached and cleaned up on shader destroy
    (void)handle;
}

void OpenGLBackend::BindPipeline(PipelineHandle handle) {
    // Extract shader IDs from pipeline handle
    uint32_t vsId = handle.id >> 16;
    uint32_t fsId = handle.id & 0xFFFF;

    GLuint vs = GetGLShader({vsId});
    GLuint fs = GetGLShader({fsId});

    if (vs != 0 && fs != 0) {
        GLuint program = GetOrCreateProgram(vs, fs);
        if (program != 0) {
            glUseProgram(program);
        }
    }
}

// ============================================================================
// Utility
// ============================================================================
void OpenGLBackend::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLBackend::Clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ============================================================================
// Query
// ============================================================================
int OpenGLBackend::GetMaxTextureSize() const {
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return maxSize;
}

// ============================================================================
// Internal helpers
// ============================================================================
GLuint OpenGLBackend::GetGLShader(ShaderHandle handle) const {
    if (handle.id == 0) return 0;
    auto it = m_shaders.find(handle.id);
    return (it != m_shaders.end()) ? it->second : 0;
}

GLuint OpenGLBackend::GetGLTexture(TextureHandle handle) const {
    if (handle.id == 0) return 0;
    auto it = m_textures.find(handle.id);
    return (it != m_textures.end()) ? it->second : 0;
}

GLuint OpenGLBackend::GetGLFramebuffer(TextureHandle textureHandle) const {
    if (textureHandle.id == 0) return 0;

    // Check if framebuffer already exists
    auto it = m_framebuffers.find(textureHandle.id);
    if (it != m_framebuffers.end()) {
        return it->second;
    }

    // Create new framebuffer
    GLuint tex = GetGLTexture(textureHandle);
    if (tex == 0) return 0;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    if (fbo == 0) return 0;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[OpenGL] Framebuffer incomplete: 0x%x\n", status);
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);

    // Store in cache (const method needs to modify cache)
    const_cast<OpenGLBackend*>(this)->m_framebuffers[textureHandle.id] = fbo;

    return fbo;
}

GLuint OpenGLBackend::GetOrCreateProgram(GLuint vs, GLuint fs) {
    uint64_t key = (static_cast<uint64_t>(vs) << 32) | fs;

    auto it = m_programCache.find(key);
    if (it != m_programCache.end()) {
        return it->second;
    }

    GLuint program = glCreateProgram();
    if (program == 0) return 0;

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Shader program linking failed: %s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    // Detach shaders after linking (they can be deleted if no longer needed)
    glDetachShader(program, vs);
    glDetachShader(program, fs);

    // Explicitly bind "Params" uniform block to binding point 1
    // (GLSL layout(binding=1) may not be honored by all drivers)
    GLint nb = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);
    if (nb > 0) {
        GLuint blockIndex = glGetUniformBlockIndex(program, "Params");
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, blockIndex, 1);
        }
    }

    m_programCache[key] = program;
    return program;
}

GLint OpenGLBackend::GLInternalFormat(TextureFormat fmt) {
    switch (fmt) {
        case TextureFormat::RGBA8:   return GL_RGBA8;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::R8:      return GL_R8;
        default:                     return GL_RGBA8;
    }
}

GLenum OpenGLBackend::GLFormat(TextureFormat fmt) {
    switch (fmt) {
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA32F:
            return GL_RGBA;
        case TextureFormat::R8:
            return GL_RED;
        default:
            return GL_RGBA;
    }
}

GLenum OpenGLBackend::GLType(TextureFormat fmt) {
    switch (fmt) {
        case TextureFormat::RGBA32F:
            return GL_FLOAT;
        case TextureFormat::RGBA8:
        case TextureFormat::R8:
        default:
            return GL_UNSIGNED_BYTE;
    }
}

void OpenGLBackend::SetupQuadVAO() {
    // Create VAO
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    // Create VBO with interleaved vertex data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // UV attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Create EBO for indexed drawing
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices), kQuadIndices, GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);

    // Store VBO handle (EBO is owned by VAO)
    m_quadVBO = vbo;
}

void OpenGLBackend::BindDefaultState() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, m_width, m_height);
}
