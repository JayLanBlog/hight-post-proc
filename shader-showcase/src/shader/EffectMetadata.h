#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class ParamType { Float, Int, Bool, Float2, Float3, Float4, Color };

struct ShaderParam {
    std::string name;
    std::string label;
    ParamType type = ParamType::Float;
    float minVal = 0.0f, maxVal = 1.0f;
    float defaultVal[4] = {0,0,0,0};
    std::string uiType; // slider, drag, combo, color, checkbox
    std::vector<std::string> comboOptions;
};

struct EffectCard {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string thumbnailPath;
    std::string vertSpirvPath;  // path to vertex .spv
    std::string fragSpirvPath;  // path to fragment .spv
    int passes = 1;
    std::vector<ShaderParam> params;
};

struct UniformBinding {
    int location = -1;
    ParamType type = ParamType::Float;
    float currentValue[4] = {0,0,0,0};
};

// Parse an effect.json file and return a populated EffectCard.
EffectCard LoadEffectFromJson(const std::string& filepath);
