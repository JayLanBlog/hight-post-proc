#include "render/BookMeshRenderer.h"
#include <fstream>
#include <cstring>
#include <cstdio>

bool BookMeshRenderer::Load(const std::string& binPath) {
    std::ifstream file(binPath, std::ios::binary);
    if (!file) return false;

    uint32_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    if (meshCount == 0 || meshCount > 10) {
        printf("[BookMesh] Invalid meshCount: %u\n", meshCount);
        return false;
    }

    m_data.subMeshes.resize(meshCount);
    m_data.nodeTransforms.resize(meshCount * 16, 0.0f);

    float transforms[10][3] = {
        {-6.288f, 0.030f, 0.396f}, {-6.288f, 0.484f, 0.396f},
        {0.0f, 0.030f, 0.396f},    {0.0f, 0.484f, 0.396f},
        {20.655f, 3.051f, 15.910f},{20.655f, 3.051f, 15.910f},
        {-26.943f, 3.051f, 15.910f},{-26.943f, 3.051f, 15.910f},
        {-3.579f, -0.322f, -1.702f},{0.0f, 0.0f, 0.0f}
    };

    for (uint32_t mi = 0; mi < meshCount; mi++) {
        uint32_t vc, ic;
        uint8_t hasUV, hasNormal;
        file.read(reinterpret_cast<char*>(&vc), 4);
        file.read(reinterpret_cast<char*>(&ic), 4);
        file.read(reinterpret_cast<char*>(&hasUV), 1);
        file.read(reinterpret_cast<char*>(&hasNormal), 1);
        file.seekg(2, std::ios::cur);

        uint32_t partCount;
        file.read(reinterpret_cast<char*>(&partCount), 4);
        for (uint32_t pi = 0; pi < partCount; pi++) {
            uint32_t firstIdx, numIdx, nameLen;
            file.read(reinterpret_cast<char*>(&firstIdx), 4);
            file.read(reinterpret_cast<char*>(&numIdx), 4);
            file.read(reinterpret_cast<char*>(&nameLen), 4);
            file.seekg(nameLen, std::ios::cur);
        }

        auto& sm = m_data.subMeshes[mi];
        sm.vertexCount = vc;
        sm.indexCount = ic;
        sm.vertices.resize(vc * 8);
        sm.indices.resize(ic);

        file.read(reinterpret_cast<char*>(sm.vertices.data()), vc * 8 * sizeof(float));
        file.read(reinterpret_cast<char*>(sm.indices.data()), ic * sizeof(uint32_t));

        float* t = &m_data.nodeTransforms[mi * 16];
        std::memset(t, 0, 16 * sizeof(float));
        t[0] = t[5] = t[10] = t[15] = 1.0f;
        t[12] = transforms[mi][0];
        t[13] = transforms[mi][1];
        t[14] = transforms[mi][2];
    }

    m_loaded = true;
    return true;
}

void BookMeshRenderer::Destroy(IRenderBackend* backend) {
    for (auto& tex : m_textures) backend->DestroyTexture(tex);
    for (auto& tex : m_normals) backend->DestroyTexture(tex);
    m_textures.clear();
    m_normals.clear();
    m_loaded = false;
}

bool BookMeshRenderer::LoadTextures(IRenderBackend* backend) {
    // 清理旧纹理
    for (auto& tex : m_textures) backend->DestroyTexture(tex);
    for (auto& tex : m_normals) backend->DestroyTexture(tex);
    m_textures.clear();
    m_normals.clear();

    const char* texPaths[] = {
        "assets/models/book/textures/Book Texture 1.jpg",
        "assets/models/book/textures/Book Texture 2.jpg",
        "assets/models/book/textures/Book Texture 3.jpg",
        "assets/models/book/textures/Book Page Left.jpg",
    };
    const char* normPaths[] = {
        "assets/models/book/textures/Book Texture 1 Normal Map.jpg",
        "assets/models/book/textures/Book Texture 2 Normal Map.jpg",
        "assets/models/book/textures/Book Texture 3 Normal Map.jpg",
        "assets/models/book/textures/Book Texture 1 Normal Map.jpg",  // 页面无独立法线贴图，复用
    };

    for (int i = 0; i < 4; i++) {
        auto tex = backend->CreateTextureFromFile(texPaths[i]);
        if (tex.id == 0) {
            printf("[BookMesh] Failed to load texture: %s\n", texPaths[i]);
            // 清理已加载的纹理
            for (auto& t : m_textures) backend->DestroyTexture(t);
            for (auto& t : m_normals) backend->DestroyTexture(t);
            m_textures.clear();
            m_normals.clear();
            return false;
        }
        m_textures.push_back(tex);

        auto norm = backend->CreateTextureFromFile(normPaths[i]);
        if (norm.id == 0) {
            printf("[BookMesh] Failed to load normal: %s\n", normPaths[i]);
            for (auto& t : m_textures) backend->DestroyTexture(t);
            for (auto& t : m_normals) backend->DestroyTexture(t);
            m_textures.clear();
            m_normals.clear();
            return false;
        }
        m_normals.push_back(norm);
    }
    printf("[BookMesh] Loaded %d textures\n", (int)m_textures.size());
    return true;
}

void BookMeshRenderer::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                               const float* viewMat, const float* projMat,
                               const float* lightDir, const float* lightColor,
                               float elapsedTime) {
    if (!m_loaded) return;

    // --- Idle page-flutter animation (matches Unity Idle.anim) ---
    // 0.25s triangle wave: z-rotation 0° → -6.324° → 0°
    float animT = fmodf(elapsedTime, 0.25f) / 0.25f; // 0..1 cycle
    float t = animT < 0.333f ? animT / 0.333f :
              animT < 0.666f ? 1.0f - (animT - 0.333f) / 0.333f : 0.0f;
    float angle = t * (-6.324f * 3.14159265f / 180.0f); // radians

    // Build Z-rotation matrix for left-page submeshes
    float rz[16] = {
        cosf(angle), -sinf(angle), 0, 0,
        sinf(angle),  cosf(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // SubMesh indices that belong to left page (X-negative in transforms table)
    const bool leftPageIndices[10] = {true, false, true, false, false, false, true, true, false, false};

    for (size_t mi = 0; mi < m_data.subMeshes.size(); mi++) {
        auto& sm = m_data.subMeshes[mi];
        if (sm.vertexCount == 0) continue;

        float mvp[16], modelView[16];
        const float* nodeT = &m_data.nodeTransforms[mi * 16];

        // Apply Z-rotation animation to left-page submeshes
        float animatedNodeT[16];
        if (mi < 10 && leftPageIndices[mi]) {
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++) {
                    float sum = 0;
                    for (int k = 0; k < 4; k++) sum += rz[r*4 + k] * nodeT[k*4 + c];
                    animatedNodeT[r*4 + c] = sum;
                }
            nodeT = animatedNodeT;
        }

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                    sum += viewMat[r*4 + k] * nodeT[k*4 + c];
                modelView[r*4 + c] = sum;
            }
        }
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                    sum += projMat[r*4 + k] * modelView[k*4 + c];
                mvp[r*4 + c] = sum;
            }
        }

        ShaderParams params;
        // Use a default white texture if textures not loaded yet
        if (m_textures.empty()) {
            params.inputTextures.push_back(m_defaultTex);
        } else {
            params.inputTextures.push_back(m_textures[mi % m_textures.size()]);
        }
        if (!m_normals.empty()) {
            params.auxTextures.push_back(m_normals[mi % m_normals.size()]);
        }
        params.mvp = std::vector<float>(mvp, mvp + 16);
        params.modelView = std::vector<float>(modelView, modelView + 16);

        // Transform lightDir to view space (rotate only, no translation)
        float viewLightDir[3] = {
            viewMat[0]*lightDir[0] + viewMat[1]*lightDir[1] + viewMat[2]*lightDir[2],
            viewMat[4]*lightDir[0] + viewMat[5]*lightDir[1] + viewMat[6]*lightDir[2],
            viewMat[8]*lightDir[0] + viewMat[9]*lightDir[1] + viewMat[10]*lightDir[2]
        };
        float viewEyePos[3] = {0.0f, 0.0f, 0.0f};  // In view space, eye is at origin

        // Pass these to the shader instead of the world-space ones
        params.lightDir = std::vector<float>(viewLightDir, viewLightDir + 3);
        params.lightColor = std::vector<float>(lightColor, lightColor + 3);
        params.eyePos = std::vector<float>(viewEyePos, viewEyePos + 3);
        params.uniformFloats = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        backend->DrawMesh(vert, frag, params,
                          sm.vertices.data(), sm.vertexCount, 8 * sizeof(float),
                          sm.indices.data(), sm.indexCount);
    }
}