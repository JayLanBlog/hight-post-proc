#pragma once
#include "Scene.h"
#include "render/IRenderBackend.h"
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <cstdlib>

class Application;

struct AUS3DPass {
    std::string fragShader;             // SPIR-V路径
    int targetWidth = 0;                // 0=全屏, N=降采样宽度
    int targetHeight = 0;               // 0=全屏, N=降采样高度
    bool isOutput = false;              // 最后一个Pass输出到屏幕
    ShaderHandle fragShaderHandle = {0}; // 懒加载缓存,避免每帧重建
};

struct AUS3DEffect {
    std::string name;
    std::string description;
    std::string fragShaderPath;
    ShaderHandle   fragShader = {0};
    std::vector<float> defaultValues;
    std::vector<std::string> paramLabels;
    std::vector<float> paramMin;
    std::vector<float> paramMax;
    std::vector<AUS3DPass> passes;        // 新增: Pass序列 (空=旧路径单Pass)
    std::vector<std::string> auxTextures; // 新增: 辅助纹理路径
    bool use3DGeometry = true;            // 新增: 是否保留光追球体
    bool blendSrcAlpha = false;           // 新增: Blend SrcAlpha SrcAlpha (vs 默认SrcAlpha OneMinusSrcAlpha)
};

class TextureManager {
public:
    TextureManager(IRenderBackend* backend) : m_backend(backend) {}
    
    ~TextureManager() {
        for (auto& [key, handle] : m_cache) {
            if (handle.id != 0) {
                m_backend->DestroyTexture(handle);
            }
        }
        m_cache.clear();
    }
    
    TextureHandle LoadTexture(const std::string& path) {
        auto it = m_cache.find(path);
        if (it != m_cache.end()) return it->second;
        TextureHandle tex = m_backend->CreateTextureFromFile(path);
        if (tex.id != 0) {
            m_cache[path] = tex;
        }
        return tex;
    }
    
    TextureHandle GenerateRampTexture(int bands) {
        std::string key = "ramp_" + std::to_string(bands);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;
        std::vector<uint8_t> data(256 * 4);
        for (int i = 0; i < 256; i++) {
            float t = i / 255.0f;
            int band = int(t * bands);
            float v = float(band) / float(bands - 1);
            uint8_t c = uint8_t(v * 255);
            data[i * 4 + 0] = c;
            data[i * 4 + 1] = c;
            data[i * 4 + 2] = c;
            data[i * 4 + 3] = 255;
        }
        TextureHandle tex = m_backend->CreateTextureFromData(256, 1, data.data());
        if (tex.id != 0) { m_cache[key] = tex; }
        return tex;
    }
    
    TextureHandle GenerateNoiseTexture(int size) {
        std::string key = "noise_" + std::to_string(size);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;
        std::vector<uint8_t> data(size * size * 4);
        for (int i = 0; i < size * size; i++) {
            uint8_t v = uint8_t(rand() % 256);
            data[i * 4 + 0] = v;
            data[i * 4 + 1] = v;
            data[i * 4 + 2] = v;
            data[i * 4 + 3] = 255;
        }
        TextureHandle tex = m_backend->CreateTextureFromData(size, size, data.data());
        if (tex.id != 0) { m_cache[key] = tex; }
        return tex;
    }
    
private:
    IRenderBackend* m_backend;
    std::map<std::string, TextureHandle> m_cache;
};

struct RTPool {
    struct Entry {
        TextureHandle handle;
        int width, height;
        bool inUse = false;
    };
    std::vector<Entry> entries;
    IRenderBackend* backend = nullptr;
    
    TextureHandle Acquire(int w, int h);
    void Release(TextureHandle handle);
    void Clear();
};

class AUS3DScene : public Scene {
public:
    AUS3DScene();
    ~AUS3DScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsReturn() const override { return m_wantsReturn; }

    void SetBackend(IRenderBackend* be) { m_backend = be; }
    void SetApplication(Application* app) { m_app = app; }

private:
    void LoadShaders();
    void CompileEffect(int index);
    void NavigateTo(int index);

    IRenderBackend* m_backend = nullptr;
    Application*    m_app = nullptr;

    ShaderHandle m_sharedVert = {0};

    std::vector<AUS3DEffect> m_effects;
    int m_currentIndex = 0;
    int m_totalEffects = 0;

    // Camera orbit
    float m_camTheta  = 0.8f;
    float m_camPhi    = 0.6f;
    float m_camRadius = 3.0f;
    bool  m_dragging  = false;
    float m_dragStartX = 0, m_dragStartY = 0;
    float m_dragStartTheta = 0, m_dragStartPhi = 0;

    bool m_wantsReturn = false;

    float m_fpsDisplay = 0;
    std::chrono::high_resolution_clock::time_point m_fpsLastTime{};
    int m_fpsFrameCount = 0;
    float m_elapsedTime = 0.0f;
    uint32_t m_totalFrameCount = 0;

    TextureHandle m_defaultTex = {0};
    TextureHandle m_sceneBgTex = {0};  // 后处理输入用的风景图片

    std::unique_ptr<TextureManager> m_texManager;
    RTPool m_rtPool;
};
