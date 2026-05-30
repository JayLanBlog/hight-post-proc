#pragma once
#include <vector>
#include <cstdint>
#include <string>

class ShaderLoader {
public:
    static std::vector<uint32_t> LoadSPIRV(const std::string& filepath);
    static std::string FindShaderDir();
};
