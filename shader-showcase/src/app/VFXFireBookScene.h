#pragma once
#include "app/Scene.h"
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

    void SetBackend(IRenderBackend* be);
    void SetApplication(Application* app);

private:
    IRenderBackend* m_backend = nullptr;
    Application* m_app = nullptr;
    bool m_wantsReturn = false;

    std::unique_ptr<BookMeshRenderer> m_bookRenderer;
    std::unique_ptr<BookParticleSystem> m_paperParticles;
    std::unique_ptr<BookParticleSystem> m_smokeParticles;
    std::unique_ptr<DissolvePostProcess> m_dissolve;
    std::unique_ptr<AudioPlayer> m_audio;

    float m_dissolveAmount = 0.5f;
    float m_dissolveSpeed = 0.05f;
    float m_edgeWidth = 0.2f;
    float m_particleScale = 1.0f;
    float m_smokeDensity = 1.0f;
    float m_rotationSpeed = 0.1f;
    float m_elapsedTime = 0.0f;
};