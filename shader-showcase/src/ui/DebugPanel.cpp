#include "ui/DebugPanel.h"

#include <imgui.h>

void DebugPanel::SetParams(const std::vector<ShaderParam>& params)
{
    m_params = params;

    // Reset offsets and re-count
    m_floatOffset = 0;
    m_intOffset   = 0;

    // Count how many float/int slots each param uses
    for (const auto& p : m_params) {
        switch (p.type) {
        case ParamType::Float:  m_floatOffset += 1; break;
        case ParamType::Int:    m_intOffset   += 1; break;
        case ParamType::Bool:   m_intOffset   += 1; break;
        case ParamType::Float2: m_floatOffset += 2; break;
        case ParamType::Float3: m_floatOffset += 3; break;
        case ParamType::Float4: m_floatOffset += 4; break;
        case ParamType::Color:  m_floatOffset += 3; break;
        }
    }

    // Allocate and initialize from defaults
    m_floatValues.assign(m_floatOffset, 0.0f);
    m_intValues.assign(m_intOffset, 0);

    int fIdx = 0;
    int iIdx = 0;
    for (const auto& p : m_params) {
        switch (p.type) {
        case ParamType::Float:
            m_floatValues[fIdx++] = p.defaultVal[0];
            break;
        case ParamType::Int:
            m_intValues[iIdx++] = static_cast<int32_t>(p.defaultVal[0]);
            break;
        case ParamType::Bool:
            m_intValues[iIdx++] = static_cast<int32_t>(p.defaultVal[0] > 0.5f ? 1 : 0);
            break;
        case ParamType::Float2:
            m_floatValues[fIdx++] = p.defaultVal[0];
            m_floatValues[fIdx++] = p.defaultVal[1];
            break;
        case ParamType::Float3:
            m_floatValues[fIdx++] = p.defaultVal[0];
            m_floatValues[fIdx++] = p.defaultVal[1];
            m_floatValues[fIdx++] = p.defaultVal[2];
            break;
        case ParamType::Float4:
            m_floatValues[fIdx++] = p.defaultVal[0];
            m_floatValues[fIdx++] = p.defaultVal[1];
            m_floatValues[fIdx++] = p.defaultVal[2];
            m_floatValues[fIdx++] = p.defaultVal[3];
            break;
        case ParamType::Color:
            m_floatValues[fIdx++] = p.defaultVal[0];
            m_floatValues[fIdx++] = p.defaultVal[1];
            m_floatValues[fIdx++] = p.defaultVal[2];
            break;
        }
    }
}

void DebugPanel::SetUniformValues(std::vector<float>& uniformFloats, std::vector<int32_t>& uniformInts)
{
    // Only update values, preserve the caller's array size
    // (prevents SPIR-V UBO layout mismatch when shader expects more params)
    size_t nf = (std::min)(uniformFloats.size(), m_floatValues.size());
    for (size_t i = 0; i < nf; ++i) {
        uniformFloats[i] = m_floatValues[i];
    }
    size_t ni = (std::min)(uniformInts.size(), m_intValues.size());
    for (size_t i = 0; i < ni; ++i) {
        uniformInts[i] = m_intValues[i];
    }
}

void DebugPanel::Render(bool* pOpen)
{
    (void)pOpen;

    int fIdx = 0;
    int iIdx = 0;

    for (const auto& p : m_params) {
        const char* label = p.label.empty() ? p.name.c_str() : p.label.c_str();

        switch (p.type) {
        case ParamType::Float:
        {
            if (p.uiType == "slider") {
                ImGui::SliderFloat(label, &m_floatValues[fIdx], p.minVal, p.maxVal);
            } else if (p.uiType == "drag") {
                float speed = (p.maxVal - p.minVal) * 0.01f;
                if (speed <= 0.0f) speed = 0.01f;
                ImGui::DragFloat(label, &m_floatValues[fIdx], speed, p.minVal, p.maxVal);
            } else {
                ImGui::SliderFloat(label, &m_floatValues[fIdx], p.minVal, p.maxVal);
            }
            fIdx += 1;
            break;
        }
        case ParamType::Int:
        {
            if (p.uiType == "combo" && !p.comboOptions.empty()) {
                int current = m_intValues[iIdx];
                // Build the items string for ImGui::Combo
                std::string items;
                for (size_t j = 0; j < p.comboOptions.size(); ++j) {
                    if (j > 0) items += '\0';
                    items += p.comboOptions[j];
                }
                items += '\0';
                if (ImGui::Combo(label, &current, items.c_str())) {
                    m_intValues[iIdx] = current;
                }
            } else if (p.uiType == "checkbox") {
                bool b = m_intValues[iIdx] != 0;
                if (ImGui::Checkbox(label, &b)) {
                    m_intValues[iIdx] = b ? 1 : 0;
                }
            } else {
                int imin = static_cast<int>(p.minVal);
                int imax = static_cast<int>(p.maxVal);
                ImGui::SliderInt(label, &m_intValues[iIdx], imin, imax);
            }
            iIdx += 1;
            break;
        }
        case ParamType::Bool:
        {
            bool b = m_intValues[iIdx] != 0;
            if (ImGui::Checkbox(label, &b)) {
                m_intValues[iIdx] = b ? 1 : 0;
            }
            iIdx += 1;
            break;
        }
        case ParamType::Float2:
        {
            ImGui::SliderFloat2(label, &m_floatValues[fIdx], p.minVal, p.maxVal);
            fIdx += 2;
            break;
        }
        case ParamType::Float3:
        {
            if (p.uiType == "color") {
                ImGui::ColorEdit3(label, &m_floatValues[fIdx]);
            } else {
                ImGui::SliderFloat3(label, &m_floatValues[fIdx], p.minVal, p.maxVal);
            }
            fIdx += 3;
            break;
        }
        case ParamType::Float4:
        {
            ImGui::SliderFloat4(label, &m_floatValues[fIdx], p.minVal, p.maxVal);
            fIdx += 4;
            break;
        }
        case ParamType::Color:
        {
            ImGui::ColorEdit3(label, &m_floatValues[fIdx]);
            fIdx += 3;
            break;
        }
        }
    }
}
