#include "render/DissolvePostProcess.h"
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>

bool DissolvePostProcess::Init(IRenderBackend* backend) {
    if (!backend) {
        printf("[Dissolve] Init failed: null backend\n");
        return false;
    }

    // Load SPIR-V shaders
    auto readSPIRV = [](const std::string& path) -> std::vector<uint32_t> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        size_t size = file.tellg();
        if (size % 4 != 0) {
            printf("[Dissolve] SPIR-V file size not aligned: %s\n", path.c_str());
            return {};
        }
        file.seekg(0);
        std::vector<uint32_t> data(size / 4);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    };

    auto vertData = readSPIRV("shaders/common/fullscreen.vert.spv");
    auto fragData = readSPIRV("shaders/vfx_fire/dissolve.frag.spv");
    if (vertData.empty() || fragData.empty()) {
        printf("[Dissolve] Failed to read SPIR-V files\n");
        return false;
    }

    m_vertShader = backend->CreateVertexShader(vertData.data(), vertData.size());
    m_fragShader = backend->CreateFragmentShader(fragData.data(), fragData.size());
    if (m_vertShader.id == 0 || m_fragShader.id == 0) {
        printf("[Dissolve] Failed to create shaders\n");
        Destroy(backend);
        return false;
    }

    // Create procedural noise texture (512x512 R8)
    // Use fixed seed for deterministic noise across runs
    srand(42);
    std::vector<uint8_t> noiseData(512 * 512);
    for (int i = 0; i < 512 * 512; i++) {
        noiseData[i] = (uint8_t)(rand() % 256);
    }
    m_noiseTex = backend->CreateTexture(512, 512, TextureFormat::R8, noiseData.data());
    if (m_noiseTex.id == 0) {
        printf("[Dissolve] Failed to create noise texture\n");
        Destroy(backend);
        return false;
    }

    // Pre-create pipeline (cache warm-up; DrawFullscreenQuad creates its own)
    PipelineDesc desc;
    desc.vertShader = m_vertShader;
    desc.fragShader = m_fragShader;
    desc.width = 0;
    desc.height = 0;
    desc.blendEnable = false;
    desc.useVertexInput = false;
    m_pipeline = backend->CreatePipeline(desc);
    if (m_pipeline.id == 0) {
        printf("[Dissolve] Failed to create pipeline\n");
        Destroy(backend);
        return false;
    }

    m_initialized = true;
    printf("[Dissolve] Initialized successfully\n");
    return true;
}

void DissolvePostProcess::Apply(IRenderBackend* backend, TextureHandle inputRT, TextureHandle outputRT,
                                 float dissolveAmount, float edgeWidth,
                                 float noiseScaleX, float noiseScaleY,
                                 float noiseOffsetX, float noiseOffsetY) {
    if (!m_initialized) {
        printf("[Dissolve] Apply called but not initialized\n");
        return;
    }
    if (!backend) {
        printf("[Dissolve] Apply called with null backend\n");
        return;
    }

    ShaderParams params;
    params.inputTextures = {inputRT};
    params.auxTextures = {m_noiseTex};
    // P0-P5顺序: dissolveAmount, edgeWidth, noiseScaleX, noiseScaleY, noiseOffsetX, noiseOffsetY
    params.uniformFloats = {dissolveAmount, edgeWidth, noiseScaleX, noiseScaleY, noiseOffsetX, noiseOffsetY};

    // DrawFullscreenQuad handles pipeline creation and binding internally
    if (outputRT.id != 0) {
        backend->BeginRenderToTexture(outputRT);
    }
    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
    if (outputRT.id != 0) {
        backend->EndRenderToTexture();
    }
}

void DissolvePostProcess::Destroy(IRenderBackend* backend) {
    if (!backend) return;
    if (m_pipeline.id) { backend->DestroyPipeline(m_pipeline); m_pipeline = {0}; }
    if (m_vertShader.id) { backend->DestroyShader(m_vertShader); m_vertShader = {0}; }
    if (m_fragShader.id) { backend->DestroyShader(m_fragShader); m_fragShader = {0}; }
    if (m_noiseTex.id) { backend->DestroyTexture(m_noiseTex); m_noiseTex = {0}; }
    m_initialized = false;
}