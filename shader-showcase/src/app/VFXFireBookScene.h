#pragma once
#include "Scene.h"
#include "render/BackendType.h"
#include "render/IRenderBackend.h"
#include <memory>
#include <vector>

class IRenderBackend;
class Application;
class BookMeshRenderer;
class BookParticleSystem;
class DissolvePostProcess;
class AudioPlayer;

struct VFXRTPool {
    struct Entry {
        TextureHandle handle;
        int width, height;
        bool inUse = false;
    };
    std::vector<Entry> entries;
    IRenderBackend* backend = nullptr;

    TextureHandle Acquire(int w, int h);
    void Release(TextureHandle handle);
    void Clear();
};

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
    ShaderHandle m_particleVert = {0};
    ShaderHandle m_particleFrag = {0};      // smoke particles (sequence-frame)
    ShaderHandle m_paperFrag = {0};          // paper debris particles (dissolve)
    ShaderHandle m_smokeFrag = {0};          // smoke particles (sequence-frame)

    TextureHandle m_smokeTex = {0};          // RealisticSmoke02_6x6.png
    TextureHandle m_noiseTex = {0};          // procedural dissolve noise (reuse DissolvePostProcess)
    TextureHandle m_paperTex = {0};          // paper page texture for debris

    VFXRTPool m_rtPool;

    // Lighting (matches Unity directional light: white, intensity 2)
    float m_lightDir[3] = {0.0f, -1.0f, 0.0f};
    float m_lightColor[3] = {2.0f, 2.0f, 2.0f};

    // Camera orbit
    float m_camTheta = 0.8f;
    float m_camPhi = 0.6f;
    float m_camRadius = 50.0f;
    float m_camTarget[3] = {0, 0, 0};
    float m_camFov = 66.0f * 3.14159265f / 180.0f;  // Unity FOV=66°
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