#pragma once

#include "shader/EffectMetadata.h"

#include <vector>
#include <cstdint>

class DebugPanel {
public:
    void SetParams(const std::vector<ShaderParam>& params);
    void SetUniformValues(std::vector<float>& uniformFloats, std::vector<int32_t>& uniformInts);
    void Render(bool* pOpen);

private:
    std::vector<ShaderParam> m_params;
    std::vector<float>       m_floatValues;
    std::vector<int32_t>     m_intValues;
    int m_floatOffset = 0;
    int m_intOffset   = 0;
};
