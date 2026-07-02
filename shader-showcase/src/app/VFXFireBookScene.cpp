#include "VFXFireBookScene.h"
#include "render/BookMeshRenderer.h"
#include "render/BookParticleSystem.h"
#include "render/DissolvePostProcess.h"
#include "render/AudioPlayer.h"
#include "app/Application.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

VFXFireBookScene::VFXFireBookScene() = default;
VFXFireBookScene::~VFXFireBookScene() = default;

void VFXFireBookScene::OnEnter() {
    printf("[VFXFireBook] OnEnter\n");

    // Create book renderer and load mesh + textures
    m_bookRenderer = std::make_unique<BookMeshRenderer>();
    if (!m_bookRenderer->Load("assets/models/book/book_combined.bin")) {
        printf("[VFXFireBook] Failed to load book mesh!\n");
        return;
    }
    if (!m_bookRenderer->LoadTextures(m_backend)) {
        printf("[VFXFireBook] Failed to load book textures!\n");
        return;
    }

    // Load shaders
    auto readSPIRV = [](const std::string& path) -> std::vector<uint32_t> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        size_t size = file.tellg();
        file.seekg(0);
        std::vector<uint32_t> data(size / 4);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    };

    auto vertData = readSPIRV("shaders/vfx_fire/book.vert.spv");
    auto fragData = readSPIRV("shaders/vfx_fire/book.frag.spv");
    if (!vertData.empty() && !fragData.empty()) {
        m_bookVert = m_backend->CreateVertexShader(vertData.data(), vertData.size() * 4);
        m_bookFrag = m_backend->CreateFragmentShader(fragData.data(), fragData.size() * 4);
    }
}

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
    int w, h;
    backend->GetFramebufferSize(w, h);

    float viewMat[16], projMat[16];
    BuildViewMatrix(viewMat);
    BuildProjMatrix(projMat, w, h);

    float lightDir[3] = {0.0f, -1.0f, 0.0f};
    float lightColor[3] = {2.0f, 2.0f, 2.0f};

    backend->Clear(0.02f, 0.02f, 0.04f, 1.0f);

    if (m_bookVert.id != 0 && m_bookFrag.id != 0) {
        m_bookRenderer->Render(backend, m_bookVert, m_bookFrag, viewMat, projMat, lightDir, lightColor);
    }
}

void VFXFireBookScene::OnImGui() {}

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