#include "render/BookMeshRenderer.h"
#include <fstream>
#include <cstring>

bool BookMeshRenderer::Load(const std::string& binPath) { return false; }
void BookMeshRenderer::Destroy(IRenderBackend* backend) {}
bool BookMeshRenderer::LoadTextures(IRenderBackend* backend) { return false; }

void BookMeshRenderer::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                               const float* viewMat, const float* projMat,
                               const float* lightDir, const float* lightColor, const float* eyePos) {}