// test_shaders.cpp - Automated shader validation test
// Uses the project's existing OpenGLBackend to test all SPIR-V shaders
#include "render/IRenderBackend.h"
#include "render/OpenGLBackend.h"
#include "shader/ShaderLoader.h"
#include "shader/EffectMetadata.h"

#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    printf("=== Shader Showcase - Automated SPIR-V + OpenGL Test ===\n\n");

    // Find shader dir
    std::string shaderDir = ShaderLoader::FindShaderDir();
    if (shaderDir == "shaders") {
        printf("FATAL: Cannot find shader directory\n");
        return 1;
    }
    printf("Shader dir: %s\n\n", shaderDir.c_str());

    // Init GLFW
    if (!glfwInit()) { printf("FATAL: glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "Test", nullptr, nullptr);
    if (!window) { printf("FATAL: Cannot create GL 4.6 context\n"); glfwTerminate(); return 1; }

    // Init backend
    OpenGLBackend backend;
    if (!backend.Init(window)) {
        printf("FATAL: Backend init failed\n");
        glfwDestroyWindow(window); glfwTerminate();
        return 1;
    }
    printf("Backend: %s\n\n", backend.GetName());

    // Load vertex shader
    auto vertSPV = ShaderLoader::LoadSPIRV(shaderDir + "/common/fullscreen.vert.spv");
    if (vertSPV.empty()) { printf("FATAL: Cannot load fullscreen.vert.spv\n"); return 1; }

    ShaderHandle vert = backend.CreateVertexShader(vertSPV);
    if (vert.id == INVALID_SHADER.id) { printf("FATAL: Vertex shader creation failed\n"); return 1; }
    printf("[PASS] fullscreen.vert\n");

    // Test all effects
    const char* effects[] = {
        "simple_test","bloom","blur","sharpen","edge_detect",
        "emboss","pixelate","vignette","chromatic","color_grade",
        "noise","kaleidoscope","glitch","toon","vhs",
        "crt","water_ripple","lens_distort"
    };
    int N = (int)(sizeof(effects)/sizeof(effects[0]));
    int pass = 0, fail = 0;

    printf("\n--- Testing %d effects ---\n\n", N);
    for (int i = 0; i < N; i++) {
        std::string fragPath = shaderDir + "/effects/" + effects[i] + "/" + effects[i] + ".frag.spv";
        auto fragSPV = ShaderLoader::LoadSPIRV(fragPath);
        if (fragSPV.empty()) {
            printf("[FAIL] %-20s SPIR-V file not found\n", effects[i]);
            fail++; continue;
        }

        ShaderHandle frag = backend.CreateFragmentShader(fragSPV);
        if (frag.id == INVALID_SHADER.id) {
            printf("[FAIL] %-20s fragment shader creation failed\n", effects[i]);
            fail++; continue;
        }

        // Test link by doing a full DrawFullscreenQuad (will create temp program internally)
        // We need a valid texture - create a 1x1 white pixel
        uint8_t pixel[4] = {255, 255, 255, 255};
        TextureHandle tex = backend.CreateTexture(1, 1, TextureFormat::RGBA8, pixel);

        ShaderParams params;
        params.inputTextures.push_back(tex);
        params.viewportWidth = 1;
        params.viewportHeight = 1;
        params.time = 0.0f;
        params.frameCount = 0;
        // Add some default float params
        params.uniformFloats.resize(6, 0.5f);

        backend.BeginFrame();
        backend.DrawFullscreenQuad(vert, frag, params);
        backend.EndFrame();

        // If we got here without crash/link error, it passed
        printf("[PASS] %-20s\n", effects[i]);
        pass++;

        backend.DestroyShader(frag);
        backend.DestroyTexture(tex);
    }

    backend.DestroyShader(vert);
    backend.Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    printf("\n=== Results: %d/%d passed ===\n", pass, N);
    return fail > 0 ? 1 : 0;
}
