#include "shader/ShaderLoader.h"
#include <fstream>
#include <string>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

std::vector<uint32_t> ShaderLoader::LoadSPIRV(const std::string& filepath) {
    std::vector<uint32_t> result;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        fprintf(stderr, "[ShaderLoader] Cannot open SPIR-V file: %s\n", filepath.c_str());
        return result;
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        fprintf(stderr, "[ShaderLoader] Empty SPIR-V file: %s\n", filepath.c_str());
        return result;
    }

    if (size % sizeof(uint32_t) != 0) {
        fprintf(stderr, "[ShaderLoader] SPIR-V file size not multiple of 4: %s\n", filepath.c_str());
        return result;
    }

    result.resize(static_cast<size_t>(size) / sizeof(uint32_t));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(result.data()), size);
    file.close();

    printf("[ShaderLoader] Loaded SPIR-V: %s (%zu bytes, %zu words)\n",
           filepath.c_str(), static_cast<size_t>(size), result.size());
    return result;
}

std::string ShaderLoader::FindShaderDir() {
#ifdef _WIN32
    // Try to locate shader output relative to the executable
    char exePathBuf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        std::string exeDir(exePathBuf, len);
        size_t slash = exeDir.find_last_of("\\/");
        if (slash != std::string::npos)
            exeDir = exeDir.substr(0, slash);

        // Try: exe_dir/../../shaders (typical for build/bin/Release -> build/shaders)
        std::string candidate = exeDir + "/../../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        // Try: exe_dir/../shaders (typical for build/bin -> build/shaders)
        candidate = exeDir + "/../shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        // Try: exe_dir/shaders
        candidate = exeDir + "/shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;
    }
#else
    char exePathBuf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePathBuf, sizeof(exePathBuf) - 1);
    if (len > 0) {
        exePathBuf[len] = '\0';
        std::string exeDir(exePathBuf);
        size_t slash = exeDir.find_last_of('/');
        if (slash != std::string::npos)
            exeDir = exeDir.substr(0, slash);

        std::string candidate = exeDir + "/../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        candidate = exeDir + "/shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;
    }
#endif

    fprintf(stderr, "[ShaderLoader] Could not find shader directory\n");
    return "shaders"; // fallback
}
