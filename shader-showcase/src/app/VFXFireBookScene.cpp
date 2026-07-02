#include "app/VFXFireBookScene.h"
#include "render/BookMeshRenderer.h"
#include "render/BookParticleSystem.h"
#include "render/DissolvePostProcess.h"
#include "render/AudioPlayer.h"
#include "app/Application.h"

VFXFireBookScene::VFXFireBookScene() = default;
VFXFireBookScene::~VFXFireBookScene() = default;

void VFXFireBookScene::SetBackend(IRenderBackend* be) { m_backend = be; }
void VFXFireBookScene::SetApplication(Application* app) { m_app = app; }

void VFXFireBookScene::OnEnter() {}

void VFXFireBookScene::OnExit() {
    if (m_bookRenderer) { m_bookRenderer->Destroy(m_backend); m_bookRenderer.reset(); }
    if (m_dissolve) { m_dissolve->Destroy(m_backend); m_dissolve.reset(); }
    if (m_paperParticles) { m_paperParticles->Destroy(m_backend); m_paperParticles.reset(); }
    if (m_smokeParticles) { m_smokeParticles->Destroy(m_backend); m_smokeParticles.reset(); }
    if (m_audio) { m_audio->Destroy(); m_audio.reset(); }
}

void VFXFireBookScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    m_dissolveAmount += m_dissolveSpeed * dt;
    if (m_dissolveAmount > 1.0f) m_dissolveAmount = 0.0f;
}

void VFXFireBookScene::OnRender(IRenderBackend* backend) {
    backend->Clear(0.02f, 0.02f, 0.04f, 1.0f);
}

void VFXFireBookScene::OnImGui() {}