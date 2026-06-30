#pragma once

#include "BackendType.h"

#include <cstdint>
#include <vector>
#include <string>

// Forward declaration
struct GLFWwindow;

// Pipeline description for creating pipelines
struct PipelineDesc {
    ShaderHandle vertShader;
    ShaderHandle fragShader;
    int width;
    int height;
    bool blendEnable = true;
    bool useVertexInput = false;  // true for DrawMesh (3D), false for DrawFullscreenQuad
};

// ---------------------------------------------------------------------------
// ShaderParams — data passed per fullscreen-quad draw
// ---------------------------------------------------------------------------
struct ShaderParams {
    std::vector<TextureHandle> inputTextures;
    std::vector<TextureHandle> auxTextures;  // binding=2: auxiliary textures
    std::vector<float>         uniformFloats;
    std::vector<int32_t>       uniformInts;
    int   viewportWidth  = 1280;
    int   viewportHeight = 720;
    float time           = 0.0f;
    uint32_t frameCount  = 0;

    // 3D pipeline extensions
    std::vector<float> mvp;           // mat4 (16 floats) -- model-view-projection
    std::vector<float> modelView;     // mat4 (16 floats) -- model-view (for normal transform)
    std::vector<float> lightDir;      // vec3 (3 floats)
    std::vector<float> lightColor;    // vec3 (3 floats)
    std::vector<float> eyePos;        // vec3 (3 floats)

    bool blendEnable = false;      // enables alpha blending (for transparent effects)
};

// ---------------------------------------------------------------------------
// IRenderBackend — pure virtual render backend interface
// ---------------------------------------------------------------------------
class IRenderBackend {
public:
    // ---- nested CardDrawInfo -----------------------------------------------
    struct CardDrawInfo {
        TextureHandle texture;
        float posX, posY, posZ;
        float scaleX, scaleY;
        float rotationY;
        float opacity;
    };

    virtual ~IRenderBackend() = default;

    // ---- Lifecycle ---------------------------------------------------------
    virtual bool Init(GLFWwindow* window) = 0;
    virtual void Shutdown()               = 0;
    virtual void BeginFrame()             = 0;
    virtual void EndFrame()               = 0;
    virtual void WaitIdle()               = 0;

    // ---- Viewport ----------------------------------------------------------
    virtual void Resize(int width, int height)              = 0;
    virtual void GetFramebufferSize(int& width, int& height) = 0;

    // ---- Shaders -----------------------------------------------------------
    virtual ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size)   = 0;
    virtual ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) = 0;
    virtual ShaderHandle CreateVertexShaderFromGLSL(const std::string& source)    = 0;
    virtual ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source)  = 0;
    virtual void         DestroyShader(ShaderHandle handle)                        = 0;

    // ---- Textures ----------------------------------------------------------
    virtual TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) = 0;
    virtual TextureHandle CreateTextureFromFile(const std::string& path) = 0;
    virtual TextureHandle CreateTextureFromData(int width, int height, const uint8_t* rgbaData) = 0;
    virtual void          UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) = 0;
    virtual void          DestroyTexture(TextureHandle handle)                                        = 0;
    virtual void*         GetImTextureID(TextureHandle handle)                                        = 0;

    // ---- Pipelines ---------------------------------------------------------
    virtual PipelineHandle CreatePipeline(const PipelineDesc& desc) = 0;
    virtual void           DestroyPipeline(PipelineHandle handle)  = 0;
    virtual void           BindPipeline(PipelineHandle handle)     = 0;

    // ---- Fullscreen quad ---------------------------------------------------
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) = 0;

    // ---- 3D Mesh drawing ----------------------------------------------------
    virtual void DrawMesh(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params,
                          const float* vertexData, size_t vertexCount, size_t vertexStride,
                          const uint32_t* indexData, size_t indexCount) = 0;

    // ---- Cards (3D card rendering) -----------------------------------------
    virtual void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) = 0;

    // ---- Blit --------------------------------------------------------------
    virtual void BlitToScreen(TextureHandle src) = 0;

    // ---- Render targets ----------------------------------------------------
    virtual void BeginRenderToTexture(TextureHandle target) = 0;
    virtual void EndRenderToTexture()                       = 0;

    // ---- Utility -----------------------------------------------------------
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void Clear(float r, float g, float b, float a)        = 0;
    virtual bool SaveScreenshot(const char* path)                 = 0;  // returns true on success

    // ---- ImGui -------------------------------------------------------------
    virtual void ImGuiInit(GLFWwindow* window) = 0;
    virtual void ImGuiNewFrame()               = 0;
    virtual void ImGuiRender()                 = 0;
    virtual void ImGuiShutdown()               = 0;

    // ---- Query -------------------------------------------------------------
    virtual BackendType GetType()                     const = 0;
    virtual const char* GetName()                     const = 0;
    virtual int         GetMaxTextureSize()           const = 0;
};
