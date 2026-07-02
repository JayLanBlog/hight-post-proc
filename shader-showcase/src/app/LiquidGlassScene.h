#pragma once
#include "Scene.h"
#include "render/IRenderBackend.h"
#include <vector>
#include <string>
#include <chrono>

class Application;

class LiquidGlassScene : public Scene {
public:
    LiquidGlassScene();
    ~LiquidGlassScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsReturn() const override { return m_wantsReturn; }

    void SetBackend(IRenderBackend* be) { m_backend = be; }
    void SetApplication(Application* app) { m_app = app; }

private:
    IRenderBackend* m_backend = nullptr;
    Application* m_app = nullptr;

    ShaderHandle m_sharedVert = {0};
    ShaderHandle m_bgShader = {0};
    ShaderHandle m_blurShader = {0};
    ShaderHandle m_glassShader = {0};

    // 渲染目标
    TextureHandle m_rtA = {0};  // 背景 (全分辨率)
    TextureHandle m_rtB = {0};  // 模糊乒乓
    TextureHandle m_rtC = {0};  // 模糊结果

    // 背景纹理
    std::vector<TextureHandle> m_bgTextures;
    int m_currentBgIndex = 0;
    std::vector<std::string> m_bgNames;
    std::vector<std::pair<int,int>> m_bgTexSizes;  // (width, height)

    // 参数 (与参考项目默认值完全一致)
    float m_powerFactor = 3.0f;
    float m_a = 0.7f, m_b = 2.3f, m_c = 5.2f, m_d = 6.9f;
    float m_fPower = 1.0f;
    float m_noise = 0.06f;
    float m_blurRadius = 2.0f;
    int m_blurIters = 1;
    float m_blurDownscale = 0.5f;
    float m_glowWeight = 0.25f;
    float m_glowBias = 0.0f;
    float m_glowEdge0 = 0.5f;
    float m_glowEdge1 = -0.5f;
    // 玻璃尺寸 (参考: ortho=15.0, quad=3.5x3.5)
    // orthoLeft=-13.333, orthoRight=13.333, orthoBottom=-7.5, orthoTop=7.5
    // NDC_half = quad_half / ortho_half = 1.75/13.333=0.1313(X), 1.75/7.5=0.2333(Y)
    // scale = 1/NDC_half = 7.619(X), 4.286(Y)
    float m_glassScaleX = 7.62f;
    float m_glassScaleY = 4.29f;

    bool m_wantsReturn = false;
    float m_elapsedTime = 0.0f;
    uint32_t m_frameCount = 0;
    float m_fpsDisplay = 0.0f;
    std::chrono::high_resolution_clock::time_point m_fpsLastTime{};
    int m_fpsFrameCount = 0;

    int m_lastRTSizeW = 0, m_lastRTSizeH = 0;

    void CreateRTs(int w, int h);
    void DestroyRTs();
};