#pragma once
#include "Scene.h"
#include "render/BackendType.h"
#include <memory>

class IRenderBackend;
class Application;
class BookMeshRenderer;
class BookParticleSystem;
class DissolvePostProcess;
class AudioPlayer;

class VFXFireBookScene : public Scene {
public:
    VFXFireBookScene();
    ~VFXFireBookScene() override;

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
    bool m_wantsReturn = false;

    std::unique_ptr<BookMeshRenderer> m_bookRenderer;
    std::unique_ptr<BookParticleSystem> m_paperParticles;
    std::unique_ptr<BookParticleSystem> m_smokeParticles;
    std::unique_ptr<DissolvePostProcess> m_dissolve;
    std::unique_ptr<AudioPlayer> m_audio;

    ShaderHandle m_bookVert = {0};
    ShaderHandle m_bookFrag = {0};

    // Camera orbit
    float m_camTheta = 0.8f;
    float m_camPhi = 0.6f;
    float m_camRadius = 50.0f;
    float m_camTarget[3] = {0, 0, 0};
    bool m_dragging = false;
    float m_dragStartX = 0, m_dragStartY = 0;
    float m_dragStartTheta = 0, m_dragStartPhi = 0;

    void BuildViewMatrix(float* out);
    void BuildProjMatrix(float* out, int w, int h);

    float m_dissolveAmount = 0.5f;
    float m_dissolveSpeed = 0.05f;
    float m_edgeWidth = 0.2f;
    float m_particleScale = 1.0f;
    float m_smokeDensity = 1.0f;
    float m_rotationSpeed = 0.1f;
    float m_elapsedTime = 0.0f;
};