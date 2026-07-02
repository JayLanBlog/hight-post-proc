#pragma once
#include "render/IRenderBackend.h"
#include <vector>
#include <string>

struct BookSubMesh {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    std::string materialName;
};

struct BookMeshData {
    std::vector<BookSubMesh> subMeshes;
    std::vector<float> nodeTransforms;
};

class BookMeshRenderer {
public:
    bool Load(const std::string& binPath);
    void Destroy(IRenderBackend* backend);
    void Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                const float* viewMat, const float* projMat, const float* lightDir, const float* lightColor);
    bool LoadTextures(IRenderBackend* backend);

private:
    BookMeshData m_data;
    std::vector<TextureHandle> m_textures;
    std::vector<TextureHandle> m_normals;
    TextureHandle m_defaultTex = {0};
    bool m_loaded = false;
};