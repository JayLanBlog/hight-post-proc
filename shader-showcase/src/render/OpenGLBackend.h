#pragma once

#include "IRenderBackend.h"

#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>

class OpenGLBackend : public IRenderBackend {
public:
    OpenGLBackend()  = default;
    ~OpenGLBackend() override = default;

    // ---- Lifecycle ---------------------------------------------------------
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // ---- Viewport ----------------------------------------------------------
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) override;

    // ---- Shaders -----------------------------------------------------------
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    /// Create shader from GLSL source (fallback when SPIR-V UBO query fails)
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& glslSource) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& glslSource) override;
    void         DestroyShader(ShaderHandle handle) override;

    // ---- Textures ----------------------------------------------------------
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void          UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void          DestroyTexture(TextureHandle handle) override;
    void*         GetImTextureID(TextureHandle handle) override;

    // ---- Pipelines ---------------------------------------------------------
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void           DestroyPipeline(PipelineHandle handle) override;
    void           BindPipeline(PipelineHandle handle) override;

    /// Save current framebuffer to PPM file for debugging.
    bool SaveScreenshot(const char* path) override;

    // ---- Fullscreen quad ---------------------------------------------------
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;

    // ---- 3D Mesh drawing ----------------------------------------------------
    void DrawMesh(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params,
                  const float* vertexData, size_t vertexCount, size_t vertexStride,
                  const uint32_t* indexData, size_t indexCount) override;

    // ---- Cards (3D card rendering) -----------------------------------------
    void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) override;

    // ---- Blit --------------------------------------------------------------
    void BlitToScreen(TextureHandle src) override;

    // ---- Render targets ----------------------------------------------------
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // ---- Utility -----------------------------------------------------------
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;

    // ---- ImGui -------------------------------------------------------------
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // ---- Query -------------------------------------------------------------
    BackendType GetType()           const override { return BackendType::OpenGL; }
    const char* GetName()           const override { return "OpenGL 4.6 (SPIR-V)"; }
    int         GetMaxTextureSize() const override;

private:
    // ---- Internal helpers --------------------------------------------------
    GLuint GetGLShader(ShaderHandle handle) const;
    GLuint GetGLTexture(TextureHandle handle) const;
    GLuint GetGLFramebuffer(TextureHandle textureHandle) const;

    /// Get or create a linked GL program from vs+fs handles. Cached.
    GLuint GetOrCreateProgram(GLuint vsKey, GLuint fsKey, GLuint vsGL, GLuint fsGL);

    static GLint  GLInternalFormat(TextureFormat fmt);
    static GLenum GLFormat(TextureFormat fmt);
    static GLenum GLType(TextureFormat fmt);

    void SetupQuadVAO();
    void BindDefaultState();

    // ---- Members -----------------------------------------------------------
    GLFWwindow* m_window     = nullptr;
    int         m_width      = 0;
    int         m_height     = 0;

    // Shader pool
    std::unordered_map<uint32_t, GLuint> m_shaders;
    uint32_t m_nextShaderId = 1;

    // Program cache (key = vs_id << 32 | fs_id)
    std::unordered_map<uint64_t, GLuint> m_programCache;

    // Texture pool
    std::unordered_map<uint32_t, GLuint> m_textures;
    std::unordered_map<uint32_t, TextureFormat> m_textureFormats;
    std::unordered_map<uint32_t, int> m_texWidths;   // texture width per id
    std::unordered_map<uint32_t, int> m_texHeights;  // texture height per id
    uint32_t m_nextTextureId = 1;

    // Framebuffer pool (keyed by texture id)
    std::unordered_map<uint32_t, GLuint> m_framebuffers;

    // Temp UBO for per-draw uniform data
    GLuint m_tempUBO = 0;

    // Fullscreen quad
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;

    // Default framebuffer
    GLuint m_defaultFBO = 0;
    GLuint m_currentFBO  = 0;
    int    m_viewportW   = 0;   // active viewport width (FBO or window)
    int    m_viewportH   = 0;   // active viewport height
};
