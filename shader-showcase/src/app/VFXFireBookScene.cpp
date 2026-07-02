#include "VFXFireBookScene.h"
#include "render/BookMeshRenderer.h"
#include "render/BookParticleSystem.h"
#include "render/DissolvePostProcess.h"
#include "render/AudioPlayer.h"
#include "app/Application.h"
#include "imgui.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

// ---- VFXRTPool implementation ----
TextureHandle VFXRTPool::Acquire(int w, int h) {
    for (auto& e : entries) {
        if (!e.inUse && e.width == w && e.height == h) {
            e.inUse = true;
            return e.handle;
        }
    }
    int texW = w > 0 ? w : 1920;
    int texH = h > 0 ? h : 1080;
    TextureHandle handle = backend->CreateTexture(texW, texH, TextureFormat::RGBA8, nullptr);
    entries.push_back({handle, texW, texH, true});
    return handle;
}

void VFXRTPool::Release(TextureHandle handle) {
    for (auto& e : entries) {
        if (e.handle.id == handle.id) {
            e.inUse = false;
            return;
        }
    }
}

void VFXRTPool::Clear() {
    for (auto& e : entries) {
        if (e.handle.id != 0) {
            backend->DestroyTexture(e.handle);
        }
    }
    entries.clear();
}

VFXFireBookScene::VFXFireBookScene() = default;
VFXFireBookScene::~VFXFireBookScene() = default;

void VFXFireBookScene::OnEnter() {
    printf("[VFXFireBook] OnEnter\n");

    m_rtPool.backend = m_backend;

    // Load SPIR-V shaders
    auto readSPIRV = [](const std::string& path) -> std::vector<uint32_t> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        size_t size = file.tellg();
        file.seekg(0);
        std::vector<uint32_t> data(size / 4);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    };

    // Book shaders
    auto bookVertData = readSPIRV("shaders/vfx_fire/book.vert.spv");
    auto bookFragData = readSPIRV("shaders/vfx_fire/book.frag.spv");
    if (!bookVertData.empty() && !bookFragData.empty()) {
        m_bookVert = m_backend->CreateVertexShader(bookVertData.data(), bookVertData.size());
        m_bookFrag = m_backend->CreateFragmentShader(bookFragData.data(), bookFragData.size());
    }
    printf("[VFXFireBook] Book shaders loaded (vert=%u, frag=%u)\n", m_bookVert.id, m_bookFrag.id);

    // Particle shaders
    auto particleVertData = readSPIRV("shaders/vfx_fire/particle.vert.spv");
    auto particleFragData = readSPIRV("shaders/vfx_fire/particle.frag.spv");
    if (!particleVertData.empty() && !particleFragData.empty()) {
        m_particleVert = m_backend->CreateVertexShader(particleVertData.data(), particleVertData.size());
        m_particleFrag = m_backend->CreateFragmentShader(particleFragData.data(), particleFragData.size());
    }

    // 1. Book renderer
    m_bookRenderer = std::make_unique<BookMeshRenderer>();
    if (!m_bookRenderer->Load("assets/models/book/book_combined.bin")) {
        printf("[VFXFireBook] Failed to load book mesh!\n");
        return;
    }
    if (!m_bookRenderer->LoadTextures(m_backend)) {
        printf("[VFXFireBook] Failed to load book textures!\n");
        return;
    }

    // 2. Dissolve post-process
    m_dissolve = std::make_unique<DissolvePostProcess>();
    if (!m_dissolve->Init(m_backend)) {
        printf("[VFXFireBook] Failed to init dissolve!\n");
        return;
    }

    // 3. Paper particles
    ParticleConfig paperConfig;
    paperConfig.capacity = 32;
    paperConfig.spawnRate = 16.0f;
    paperConfig.spawnCenter[0] = -0.0087f; paperConfig.spawnCenter[1] = 1.4f; paperConfig.spawnCenter[2] = -0.0185f;
    paperConfig.spawnSize[0] = 3.0f; paperConfig.spawnSize[1] = 4.0f; paperConfig.spawnSize[2] = 3.1f;
    paperConfig.velBase[0] = 0.8f; paperConfig.velBase[1] = 0.4f; paperConfig.velBase[2] = -0.1f;
    paperConfig.velRange[0] = 0.1f; paperConfig.velRange[1] = 1.2f; paperConfig.velRange[2] = 1.5f;
    paperConfig.lifeMin = 1.0f; paperConfig.lifeMax = 3.0f;
    paperConfig.sizeMin = 0.5f; paperConfig.sizeMax = 1.0f;
    paperConfig.angularVelMin = 0.2f; paperConfig.angularVelMax = 2.0f;
    paperConfig.gravity[0] = 0; paperConfig.gravity[1] = -0.5f; paperConfig.gravity[2] = 0;
    m_paperParticles = std::make_unique<BookParticleSystem>();
    m_paperParticles->Init(paperConfig);

    // 4. Smoke particles
    ParticleConfig smokeConfig;
    smokeConfig.capacity = 5000;
    smokeConfig.spawnRate = 160.0f;
    smokeConfig.spawnCenter[0] = 0.0164f; smokeConfig.spawnCenter[1] = -0.1317f; smokeConfig.spawnCenter[2] = -0.0315f;
    smokeConfig.spawnSize[0] = 0.5f; smokeConfig.spawnSize[1] = 0.046f; smokeConfig.spawnSize[2] = 0.7f;
    smokeConfig.velBase[0] = -0.2f; smokeConfig.velBase[1] = 0.1f; smokeConfig.velBase[2] = -0.2f;
    smokeConfig.velRange[0] = 0.2f; smokeConfig.velRange[1] = 0.3f; smokeConfig.velRange[2] = 0.2f;
    smokeConfig.lifeMin = 1.0f; smokeConfig.lifeMax = 3.0f;
    smokeConfig.sizeMin = 0.5f; smokeConfig.sizeMax = 0.7f;
    smokeConfig.gravity[0] = 0; smokeConfig.gravity[1] = 0.05f; smokeConfig.gravity[2] = 0;
    m_smokeParticles = std::make_unique<BookParticleSystem>();
    m_smokeParticles->Init(smokeConfig);

    // 5. Audio (optional - will fail silently if no audio file)
    m_audio = std::make_unique<AudioPlayer>();
    m_audio->Init("assets/audio/fire_sound.wav");
    m_audio->Play();

    printf("[VFXFireBook] OnEnter complete\n");
}

void VFXFireBookScene::OnExit() {
    if (m_audio) { m_audio->Destroy(); m_audio.reset(); }
    if (m_smokeParticles) { m_smokeParticles->Destroy(m_backend); m_smokeParticles.reset(); }
    if (m_paperParticles) { m_paperParticles->Destroy(m_backend); m_paperParticles.reset(); }
    if (m_dissolve) { m_dissolve->Destroy(m_backend); m_dissolve.reset(); }
    if (m_bookRenderer) { m_bookRenderer->Destroy(m_backend); m_bookRenderer.reset(); }
    m_rtPool.Clear();
    if (m_bookVert.id) { m_backend->DestroyShader(m_bookVert); m_bookVert = {0}; }
    if (m_bookFrag.id) { m_backend->DestroyShader(m_bookFrag); m_bookFrag = {0}; }
    if (m_particleVert.id) { m_backend->DestroyShader(m_particleVert); m_particleVert = {0}; }
    if (m_particleFrag.id) { m_backend->DestroyShader(m_particleFrag); m_particleFrag = {0}; }
}

void VFXFireBookScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    m_dissolveAmount += m_dissolveSpeed * dt;
    if (m_dissolveAmount > 1.0f) m_dissolveAmount -= 1.0f;

    if (m_paperParticles) m_paperParticles->Update(dt, m_dissolveAmount);
    if (m_smokeParticles) m_smokeParticles->Update(dt, m_dissolveAmount);
}

void VFXFireBookScene::OnRender(IRenderBackend* backend) {
    int w, h;
    backend->GetFramebufferSize(w, h);

    float viewMat[16], projMat[16];
    BuildViewMatrix(viewMat);
    BuildProjMatrix(projMat, w, h);

    float lightDir[3] = {0.0f, -1.0f, 0.0f};
    float lightColor[3] = {2.0f, 2.0f, 2.0f};

    // Stage 1: Render 3D book to RT
    TextureHandle bookRT = m_rtPool.Acquire(w, h);
    backend->BeginRenderToTexture(bookRT);
    backend->Clear(0.0f, 0.0f, 0.0f, 0.0f);
    if (m_bookVert.id != 0 && m_bookFrag.id != 0) {
        m_bookRenderer->Render(backend, m_bookVert, m_bookFrag, viewMat, projMat, lightDir, lightColor);
    }
    backend->EndRenderToTexture();

    // Stage 2: Dissolve post-process (bookRT → screen)
    backend->Clear(0.02f, 0.02f, 0.04f, 1.0f);
    if (m_dissolve) {
        m_dissolve->Apply(backend, bookRT, {0}, m_dissolveAmount, m_edgeWidth,
                          4.0f, 8.0f, 0.0f, m_elapsedTime * 0.5f);
    }

    // Stage 3: Render particles (blend onto screen)
    if (m_particleVert.id != 0 && m_particleFrag.id != 0) {
        if (m_paperParticles) m_paperParticles->Render(backend, m_particleVert, m_particleFrag, viewMat, projMat);
        if (m_smokeParticles) m_smokeParticles->Render(backend, m_particleVert, m_particleFrag, viewMat, projMat);
    }

    m_rtPool.Release(bookRT);
}

void VFXFireBookScene::OnImGui() {
    ImGui::Begin("VFX Fire Book Controls");
    ImGui::SliderFloat("Dissolve Amount", &m_dissolveAmount, 0.0f, 1.0f);
    ImGui::SliderFloat("Dissolve Speed", &m_dissolveSpeed, 0.0f, 0.5f);
    ImGui::SliderFloat("Edge Width", &m_edgeWidth, 0.0f, 0.5f);
    ImGui::SliderFloat("Particle Scale", &m_particleScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Smoke Density", &m_smokeDensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Rotation Speed", &m_rotationSpeed, 0.0f, 2.0f);
    if (ImGui::Button("Reset")) {
        m_dissolveAmount = 0.5f;
        m_dissolveSpeed = 0.05f;
        m_edgeWidth = 0.2f;
        m_particleScale = 1.0f;
        m_smokeDensity = 1.0f;
        m_rotationSpeed = 0.1f;
    }
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Paper Particles: %d", m_paperParticles ? m_paperParticles->GetAliveCount() : 0);
    ImGui::Text("Smoke Particles: %d", m_smokeParticles ? m_smokeParticles->GetAliveCount() : 0);
    ImGui::End();
}

void VFXFireBookScene::BuildViewMatrix(float* out) {
    float cx = m_camRadius * sin(m_camPhi) * cos(m_camTheta);
    float cy = m_camRadius * cos(m_camPhi);
    float cz = m_camRadius * sin(m_camPhi) * sin(m_camTheta);
    float eyeX = m_camTarget[0] + cx;
    float eyeY = m_camTarget[1] + cy;
    float eyeZ = m_camTarget[2] + cz;

    float fwd[3] = {m_camTarget[0] - eyeX, m_camTarget[1] - eyeY, m_camTarget[2] - eyeZ};
    float len = sqrt(fwd[0]*fwd[0] + fwd[1]*fwd[1] + fwd[2]*fwd[2]);
    fwd[0] /= len; fwd[1] /= len; fwd[2] /= len;

    float up[3] = {0, 1, 0};
    float right[3] = {
        up[1]*fwd[2] - up[2]*fwd[1],
        up[2]*fwd[0] - up[0]*fwd[2],
        up[0]*fwd[1] - up[1]*fwd[0]
    };
    float rLen = sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
    right[0] /= rLen; right[1] /= rLen; right[2] /= rLen;

    float realUp[3] = {
        fwd[1]*right[2] - fwd[2]*right[1],
        fwd[2]*right[0] - fwd[0]*right[2],
        fwd[0]*right[1] - fwd[1]*right[0]
    };

    out[0] = right[0]; out[1] = realUp[0]; out[2] = -fwd[0]; out[3] = 0;
    out[4] = right[1]; out[5] = realUp[1]; out[6] = -fwd[1]; out[7] = 0;
    out[8] = right[2]; out[9] = realUp[2]; out[10] = -fwd[2]; out[11] = 0;
    out[12] = -(right[0]*eyeX + right[1]*eyeY + right[2]*eyeZ);
    out[13] = -(realUp[0]*eyeX + realUp[1]*eyeY + realUp[2]*eyeZ);
    out[14] = (fwd[0]*eyeX + fwd[1]*eyeY + fwd[2]*eyeZ);
    out[15] = 1;
}

void VFXFireBookScene::BuildProjMatrix(float* out, int w, int h) {
    float aspect = (float)w / (float)h;
    float fov = 0.8f;
    float f = 1.0f / tan(fov * 0.5f);
    float n = 0.1f, fFar = 1000.0f;

    std::memset(out, 0, 16 * sizeof(float));
    out[0] = f / aspect;
    out[5] = f;
    out[10] = (fFar + n) / (n - fFar);
    out[11] = -1;
    out[14] = (2.0f * fFar * n) / (n - fFar);
}