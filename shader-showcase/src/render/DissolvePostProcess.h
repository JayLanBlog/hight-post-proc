#pragma once
#include "render/IRenderBackend.h"
#include <string>

class DissolvePostProcess {
public:
    bool Init(IRenderBackend* backend);
    void Apply(IRenderBackend* backend, TextureHandle inputRT, TextureHandle outputRT,
               float dissolveAmount, float edgeWidth, float noiseScaleX, float noiseScaleY,
               float noiseOffsetX, float noiseOffsetY);
    void Destroy(IRenderBackend* backend);

private:
    ShaderHandle m_vertShader = {0};
    ShaderHandle m_fragShader = {0};
    TextureHandle m_noiseTex = {0};
    PipelineHandle m_pipeline = {0};
};