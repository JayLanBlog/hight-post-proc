#pragma once

#include "BackendType.h"

#include <cstdint>
#include <vector>

// Forward declaration
struct GLFWwindow;

// ---------------------------------------------------------------------------
// ShaderParams — data passed per fullscreen-quad draw
// ---------------------------------------------------------------------------
struct ShaderParams {
    std::vector<TextureHandle> inputTextures;
    std::vector<float>         uniformFloats;
    std::vector<int32_t>       uniformInts;
    int   viewportWidth  = 1280;
    int   viewportHeight = 720;
    float time           = 0.0f;
    uint32_t frameCount  = 0;
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
    virtual ShaderHandle CreateVertexShader(const std::vector<uint32_t>& spirv)   = 0;
    virtual ShaderHandle CreateFragmentShader(const std::vector<uint32_t>& spirv) = 0;
    virtual void         DestroyShader(ShaderHandle handle)                        = 0;

    // ---- Textures ----------------------------------------------------------
    virtual TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) = 0;
    virtual void          DestroyTexture(TextureHandle handle)                                        = 0;
    virtual void          UpdateTexture(TextureHandle handle, int width, int height, const void* data) = 0;

    // ---- Fullscreen quad ---------------------------------------------------
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) = 0;

    // ---- Cards (3D card rendering) -----------------------------------------
    virtual void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) = 0;

    // ---- Blit --------------------------------------------------------------
    virtual void BlitToScreen(TextureHandle texture) = 0;

    // ---- Render targets ----------------------------------------------------
    virtual void BeginRenderToTexture(TextureHandle target) = 0;
    virtual void EndRenderToTexture()                       = 0;

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
