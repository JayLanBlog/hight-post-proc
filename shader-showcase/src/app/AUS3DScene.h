#pragma once
#include "Scene.h"
#include "render/IRenderBackend.h"
#include <vector>
#include <string>
#include <memory>
#include <chrono>

class Application;

struct AUS3DEffect {
    std::string name;
    std::string description;
    std::string fragShaderPath;
    ShaderHandle   fragShader = {0};
    std::vector<float> defaultValues;
    std::vector<std::string> paramLabels;
    std::vector<float> paramMin;
    std::vector<float> paramMax;
};

class AUS3DScene : public Scene {
public:
    AUS3DScene();
    ~AUS3DScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsReturn() const override { return m_wantsReturn; }

    void SetBackend(IRenderBackend* be) { m_backend = be; }
    void SetApplication(Application* app) { m_app = app; }

private:
    void LoadShaders();
    void CompileEffect(int index);
    void NavigateTo(int index);

    IRenderBackend* m_backend = nullptr;
    Application*    m_app = nullptr;

    ShaderHandle m_sharedVert = {0};

    std::vector<AUS3DEffect> m_effects;
    int m_currentIndex = 0;
    int m_totalEffects = 0;

    // Camera orbit
    float m_camTheta  = 0.8f;
    float m_camPhi    = 0.6f;
    float m_camRadius = 3.0f;
    bool  m_dragging  = false;
    float m_dragStartX = 0, m_dragStartY = 0;
    float m_dragStartTheta = 0, m_dragStartPhi = 0;

    bool m_wantsReturn = false;

    float m_fpsDisplay = 0;
    std::chrono::high_resolution_clock::time_point m_fpsLastTime{};
    int m_fpsFrameCount = 0;

    TextureHandle m_defaultTex = {0};
};
