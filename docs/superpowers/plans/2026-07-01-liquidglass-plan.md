# LiquidGlass 启动页场景实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 ShaderShowcase 中新增 LiquidGlass 玻璃效果场景，一比一复刻 `e:\AI\test\LiquidGlass` 的渲染效果

**Architecture:** 新建 `LiquidGlassScene` 继承 `Scene`，3 Stage 渲染管线（背景→13-tap 高斯模糊→合成），3 个 Vulkan GLSL 460 Shader 编译为 SPIR-V，11 张背景纹理，ImGui 参数面板

**Tech Stack:** C++17, Vulkan, GLSL 460 SPIR-V, ImGui, glslangValidator

---

## 文件结构

| 文件 | 操作 | 职责 |
|------|------|------|
| `assets/textures/liquid_glass/` | 新建目录 | 11 张背景纹理 |
| `shaders/liquid_glass/lg_bg.frag` | 新建 | 纹理直通 Shader |
| `shaders/liquid_glass/lg_blur.frag` | 新建 | 13-tap 高斯模糊 Shader |
| `shaders/liquid_glass/lg_glass.frag` | 新建 | LiquidGlass 核心 Shader |
| `src/app/LiquidGlassScene.h` | 新建 | 场景类声明 |
| `src/app/LiquidGlassScene.cpp` | 新建 | 场景实现 |
| `shaders/CMakeLists.txt` | 修改 | 添加 liquid_glass/ SPIR-V 编译 |
| `CMakeLists.txt` | 修改 | 添加源文件 |
| `src/main.cpp` | 修改 | 注册场景卡片 |

---

### Task 1: 拷贝背景纹理

**Files:**
- Create: `assets/textures/liquid_glass/` (11 个文件)

- [ ] **Step 1: 创建目录并拷贝纹理**

```powershell
$src = 'e:\AI\test\LiquidGlass\assets\textures'
$dst = 'e:\AI\graph\hight-post-proc\shader-showcase\assets\textures\liquid_glass'
New-Item -ItemType Directory -Force -Path $dst
Copy-Item "$src\background-cubes.jpg" $dst
Copy-Item "$src\background-spring.png" $dst
Copy-Item "$src\background-summer.png" $dst
Copy-Item "$src\background-autumn.png" $dst
Copy-Item "$src\background-winter.png" $dst
Copy-Item "$src\Seasonal Landscape 1.png" $dst
Copy-Item "$src\Seasonal Landscape 2.png" $dst
Copy-Item "$src\Newspaper.png" $dst
Copy-Item "$src\Cartoon Cottage.png" $dst
Copy-Item "$src\anime.png" $dst
Copy-Item "$src\background-progress-bar.jpg" $dst
```

- [ ] **Step 2: 验证**

Run: `Get-ChildItem $dst | Select-Object Name,Length | Format-Table -AutoSize`

Expected: 11 个文件，大小均 > 0

- [ ] **Step 3: Commit**

```bash
git add assets/textures/liquid_glass/
git commit -m "feat: 拷贝LiquidGlass 11张背景纹理"
```

---

### Task 2: 创建 lg_bg.frag (纹理直通)

**Files:**
- Create: `shaders/liquid_glass/lg_bg.frag`

- [ ] **Step 1: 创建文件**

```glsl
#version 460
// LiquidGlass: 背景纹理直通 — 渲染背景到RT

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;

void main() {
    outColor = texture(uInputTex, vUV);
}
```

- [ ] **Step 2: Commit**

```bash
git add shaders/liquid_glass/lg_bg.frag
git commit -m "feat: LiquidGlass 背景纹理直通 shader"
```

---

### Task 3: 创建 lg_blur.frag (13-tap 高斯模糊)

**Files:**
- Create: `shaders/liquid_glass/lg_blur.frag`

- [ ] **Step 1: 创建文件，与 Blur.glsl 的 blur13 完全一致**

```glsl
#version 460
// LiquidGlass: 13-tap 可分离高斯模糊 (σ≈1.0)
// 与 Blur.glsl blur13() 完全一致
// 水平: P0=radius, P1=0; 垂直: P0=0, P1=radius

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params {
    float P0;       // 方向 X (已乘以 u_radius)
    float P1;       // 方向 Y (已乘以 u_radius)
    vec2 uRes;      // RT 分辨率
    float uTime;
    float uFC;
};

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;
    color += texture(image, uv) * 0.1964825501511404;
    color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
    color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
    return color;
}

void main() {
    outColor = blur13(uInputTex, vUV, uRes, vec2(P0, P1));
}
```

- [ ] **Step 2: Commit**

```bash
git add shaders/liquid_glass/lg_blur.frag
git commit -m "feat: LiquidGlass 13-tap 高斯模糊 shader"
```

---

### Task 4: 创建 lg_glass.frag (LiquidGlass 核心)

**Files:**
- Create: `shaders/liquid_glass/lg_glass.frag`

- [ ] **Step 1: 创建文件，与 BatchRenderer2D.glsl 的 LiquidGlass() 完全一致**

```glsl
#version 460
// LiquidGlass 核心: 超椭圆 SDF + 折射 + 噪声 + 发光
// 与 BatchRenderer2D.glsl LiquidGlass() 完全一致
// P2 < 1.5 → LiquidGlass squircle; P2 >= 1.5 → 背景直通

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params {
    float P0;       // u_powerFactor (超椭圆指数)
    float P1;       // u_fPower (折射强度曲线)
    float P2;       // 模式: 1.0=玻璃, 2.0=背景直通
    float P3;       // u_noise
    float P4;       // u_glowWeight
    float P5;       // u_glowBias
    vec2 uRes; float uTime; float uFC;
    mat4 m0;        // m0[0]=vec4(u_a,u_b,u_c,u_d), m0[1].xy=(u_glowEdge0,u_glowEdge1)
    mat4 m1;
    vec3 uLightDir; float _p0;
    vec3 uLightColor; float _p1;
    vec3 uEyePos; float _p2;
};

#define M_E 2.718281828459045

float sdSuperellipse(vec2 p, float n, float r) {
    vec2 p_abs = abs(p);
    float num = pow(p_abs.x, n) + pow(p_abs.y, n) - pow(r, n);
    float den = n * sqrt(pow(p_abs.x, 2.0 * n - 2.0) + pow(p_abs.y, 2.0 * n - 2.0)) + 0.00001;
    return num / den;
}

float rand2(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float Glow() {
    return sin(atan(vUV.y * 2.0 - 1.0, vUV.x * 2.0 - 1.0) - 0.5);
}

void main() {
    // 背景直通模式 (P2 >= 1.5)
    if (P2 > 1.5) {
        outColor = texture(uInputTex, vUV);
        return;
    }

    float u_a = m0[0].x;
    float u_b = m0[0].y;
    float u_c = m0[0].z;
    float u_d = m0[0].w;
    float u_glowEdge0 = m0[1].x;
    float u_glowEdge1 = m0[1].y;

    vec2 p = (vUV - 0.5) * 2.0;
    float d = sdSuperellipse(p, P0, 1.0);

    if (d > 0.0)
        discard;

    float dist = -d;
    // f(x) = 1 - u_b * (u_c * e)^(-u_d * x - u_a)
    float refr = 1.0 - u_b * pow(u_c * M_E, -u_d * dist - u_a);
    vec2 sampleP = p * pow(refr, P1);

    vec2 coord = sampleP * 0.5 + 0.5;

    if (max(coord.x, coord.y) > 1.0 || min(coord.x, coord.y) < 0.0) {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    vec4 noise = vec4(vec3(rand2(gl_FragCoord.xy * 1e-3) - 0.5), 0.0);
    vec4 color = texture(uInputTex, coord) + noise * P3;
    float mul = Glow() * P4 * smoothstep(u_glowEdge0, u_glowEdge1, dist) + 1.0 + P5;
    outColor = color * vec4(vec3(mul), 1.0);
}
```

- [ ] **Step 2: Commit**

```bash
git add shaders/liquid_glass/lg_glass.frag
git commit -m "feat: LiquidGlass 核心 shader (超椭圆SDF+折射+噪声+发光)"
```

---

### Task 5: 更新 CMake 构建系统

**Files:**
- Modify: `shaders/CMakeLists.txt`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 修改 `shaders/CMakeLists.txt`，在文件末尾添加 liquid_glass 编译**

在 `shaders/CMakeLists.txt` 末尾追加：

```cmake
# LiquidGlass shaders
file(GLOB LG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/liquid_glass/*.frag")
foreach(LG_FILE ${LG_SOURCES})
    compile_frag(${LG_FILE})
endforeach()
```

- [ ] **Step 2: 修改 `CMakeLists.txt`，在 SRC_FILES 中添加新文件**

在 `CMakeLists.txt` 的 `SRC_FILES` 列表中添加：

```cmake
    src/app/LiquidGlassScene.h
    src/app/LiquidGlassScene.cpp
```

- [ ] **Step 3: 验证 CMake 配置**

Run: `cd e:\AI\graph\hight-post-proc\shader-showcase\build ; cmake .. -G "Visual Studio 17 2022" -A x64`

Expected: 配置成功，无错误

- [ ] **Step 4: Commit**

```bash
git add shaders/CMakeLists.txt CMakeLists.txt
git commit -m "feat: CMake添加LiquidGlass场景源文件和Shader编译"
```

---

### Task 6: 创建 LiquidGlassScene.h

**Files:**
- Create: `src/app/LiquidGlassScene.h`

- [ ] **Step 1: 创建头文件**

```cpp
#pragma once
#include "Scene.h"
#include "render/IRenderBackend.h"
#include <vector>
#include <string>
#include <chrono>

class Application;

class LiquidGlassScene : public Scene {
public:
    LiquidGlassScene();
    ~LiquidGlassScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsReturn() const override { return m_wantsReturn; }

    void SetBackend(IRenderBackend* be) { m_backend = be; }
    void SetApplication(Application* app) { m_app = app; }

private:
    IRenderBackend* m_backend = nullptr;
    Application* m_app = nullptr;

    ShaderHandle m_sharedVert = {0};
    ShaderHandle m_bgShader = {0};
    ShaderHandle m_blurShader = {0};
    ShaderHandle m_glassShader = {0};

    // 渲染目标
    TextureHandle m_rtA = {0};  // 背景 (全分辨率)
    TextureHandle m_rtB = {0};  // 模糊乒乓
    TextureHandle m_rtC = {0};  // 模糊结果

    // 背景纹理
    std::vector<TextureHandle> m_bgTextures;
    int m_currentBgIndex = 0;
    std::vector<std::string> m_bgNames;

    // 参数 (与参考项目默认值完全一致)
    float m_powerFactor = 3.0f;
    float m_a = 0.7f, m_b = 2.3f, m_c = 5.2f, m_d = 6.9f;
    float m_fPower = 1.0f;
    float m_noise = 0.06f;
    float m_blurRadius = 2.0f;
    int m_blurIters = 1;
    float m_blurDownscale = 0.5f;
    float m_glowWeight = 0.25f;
    float m_glowBias = 0.0f;
    float m_glowEdge0 = 0.5f;
    float m_glowEdge1 = -0.5f;

    bool m_wantsReturn = false;
    float m_elapsedTime = 0.0f;
    uint32_t m_frameCount = 0;
    float m_fpsDisplay = 0.0f;
    std::chrono::high_resolution_clock::time_point m_fpsLastTime{};
    int m_fpsFrameCount = 0;

    int m_lastRTSizeW = 0, m_lastRTSizeH = 0;

    void CreateRTs(int w, int h);
    void DestroyRTs();
};
```

- [ ] **Step 2: Commit**

```bash
git add src/app/LiquidGlassScene.h
git commit -m "feat: LiquidGlassScene 头文件"
```

---

### Task 7: 创建 LiquidGlassScene.cpp

**Files:**
- Create: `src/app/LiquidGlassScene.cpp`

- [ ] **Step 1: 创建文件骨架（includes、ReadSPIRV、构造函数、CreateRTs、DestroyRTs）**

```cpp
#include "LiquidGlassScene.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdint>

static std::vector<uint32_t> ReadSPIRV(const char* relPath) {
    const char* tries[] = {"shaders/", "build/shaders/"};
    for (int t = 0; t < 2; t++) {
        std::string path = tries[t] + std::string(relPath);
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f.is_open()) continue;
        size_t sz = f.tellg(); f.seekg(0);
        std::vector<uint32_t> data((sz+3)/4);
        f.read((char*)data.data(), sz);
        fprintf(stderr,"[LG] ReadSPIRV OK: %s (%zu bytes)\n", path.c_str(), sz);
        return data;
    }
    fprintf(stderr,"[LG] ReadSPIRV failed: %s\n", relPath);
    return {};
}

LiquidGlassScene::LiquidGlassScene() {
    m_fpsLastTime = std::chrono::high_resolution_clock::now();
}

LiquidGlassScene::~LiquidGlassScene() = default;

void LiquidGlassScene::CreateRTs(int w, int h) {
    DestroyRTs();
    int bw = (int)(w * m_blurDownscale);
    int bh = (int)(h * m_blurDownscale);
    m_rtA = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    m_rtB = m_backend->CreateTexture(bw, bh, TextureFormat::RGBA8, nullptr);
    m_rtC = m_backend->CreateTexture(bw, bh, TextureFormat::RGBA8, nullptr);
    m_lastRTSizeW = w; m_lastRTSizeH = h;
    fprintf(stderr, "[LG] RTs created: A=%dx%d, B/C=%dx%d\n", w, h, bw, bh);
}

void LiquidGlassScene::DestroyRTs() {
    if (m_rtA.id) { m_backend->DestroyTexture(m_rtA); m_rtA = {0}; }
    if (m_rtB.id) { m_backend->DestroyTexture(m_rtB); m_rtB = {0}; }
    if (m_rtC.id) { m_backend->DestroyTexture(m_rtC); m_rtC = {0}; }
}
```

- [ ] **Step 2: 添加 OnEnter**

```cpp
void LiquidGlassScene::OnEnter() {
    printf("[LiquidGlass] OnEnter\n");

    // 加载顶点 Shader
    auto vd = ReadSPIRV("common/fullscreen_vk.vert.spv");
    if (!vd.empty()) m_sharedVert = m_backend->CreateVertexShader(vd.data(), vd.size());

    // 加载 Fragment Shaders
    auto bgd = ReadSPIRV("liquid_glass/lg_bg.frag.spv");
    if (!bgd.empty()) m_bgShader = m_backend->CreateFragmentShader(bgd.data(), bgd.size());

    auto bld = ReadSPIRV("liquid_glass/lg_blur.frag.spv");
    if (!bld.empty()) m_blurShader = m_backend->CreateFragmentShader(bld.data(), bld.size());

    auto gld = ReadSPIRV("liquid_glass/lg_glass.frag.spv");
    if (!gld.empty()) m_glassShader = m_backend->CreateFragmentShader(gld.data(), gld.size());

    // 加载背景纹理
    const char* bgFiles[] = {
        "background-cubes.jpg", "background-spring.png", "background-summer.png",
        "background-autumn.png", "background-winter.png",
        "Seasonal Landscape 1.png", "Seasonal Landscape 2.png",
        "Newspaper.png", "Cartoon Cottage.png", "anime.png",
        "background-progress-bar.jpg"
    };
    const char* bgNames[] = {
        "Cubes", "Spring", "Summer", "Autumn", "Winter",
        "Landscape 1", "Landscape 2", "Newspaper", "Cartoon Cottage", "Anime",
        "Progress Bar"
    };
    for (int i = 0; i < 11; i++) {
        std::string path = std::string("assets/textures/liquid_glass/") + bgFiles[i];
        TextureHandle tex = m_backend->CreateTextureFromFile(path);
        if (tex.id != 0) {
            m_bgTextures.push_back(tex);
            m_bgNames.push_back(bgNames[i]);
            printf("[LiquidGlass] Loaded: %s (id=%d)\n", path.c_str(), tex.id);
        } else {
            printf("[LiquidGlass] FAILED: %s\n", path.c_str());
        }
    }
    if (m_bgTextures.empty()) {
        printf("[LiquidGlass] WARNING: No background textures loaded!\n");
    }
}
```

- [ ] **Step 3: 添加 OnExit 和 OnUpdate**

```cpp
void LiquidGlassScene::OnExit() {
    printf("[LiquidGlass] OnExit\n");
    DestroyRTs();
    if (m_sharedVert.id) { m_backend->DestroyShader(m_sharedVert); m_sharedVert = {0}; }
    if (m_bgShader.id) { m_backend->DestroyShader(m_bgShader); m_bgShader = {0}; }
    if (m_blurShader.id) { m_backend->DestroyShader(m_blurShader); m_blurShader = {0}; }
    if (m_glassShader.id) { m_backend->DestroyShader(m_glassShader); m_glassShader = {0}; }
    for (auto& t : m_bgTextures) { if (t.id) m_backend->DestroyTexture(t); }
    m_bgTextures.clear();
}

void LiquidGlassScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    m_frameCount++;
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) m_wantsReturn = true;
    // FPS 计算
    m_fpsFrameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    float e = std::chrono::duration<float>(now - m_fpsLastTime).count();
    if (e >= 1.0f) { m_fpsDisplay = m_fpsFrameCount / e; m_fpsFrameCount = 0; m_fpsLastTime = now; }
}
```

- [ ] **Step 4: 添加 OnRender**

```cpp
void LiquidGlassScene::OnRender(IRenderBackend* be) {
    if (!be || !m_sharedVert.id || !m_bgShader.id || !m_blurShader.id || !m_glassShader.id)
        return;
    if (m_bgTextures.empty()) return;

    int fw = 1280, fh = 720;
    be->GetFramebufferSize(fw, fh);

    // 按需重建 RT
    int bw = (int)(fw * m_blurDownscale);
    int bh = (int)(fh * m_blurDownscale);
    if (m_lastRTSizeW != fw || m_lastRTSizeH != fh) {
        CreateRTs(fw, fh);
    }

    TextureHandle bgTex = m_bgTextures[m_currentBgIndex];

    // === Stage 1: 背景渲染 ===
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {bgTex};
        be->BeginRenderToTexture(m_rtA);
        be->DrawFullscreenQuad(m_sharedVert, m_bgShader, p);
        be->EndRenderToTexture();
    }

    // === Stage 2: 高斯模糊 ===
    if (m_blurIters > 0) {
        for (int i = 0; i < m_blurIters; i++) {
            // 水平 pass
            {
                ShaderParams p;
                p.viewportWidth = bw; p.viewportHeight = bh;
                p.inputTextures = {(i == 0) ? m_rtA : m_rtC};
                p.uniformFloats = {m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                be->BeginRenderToTexture(m_rtB);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
            // 垂直 pass
            {
                ShaderParams p;
                p.viewportWidth = bw; p.viewportHeight = bh;
                p.inputTextures = {m_rtB};
                p.uniformFloats = {0.0f, m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f};
                be->BeginRenderToTexture(m_rtC);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
        }
    }

    be->Clear(0.1f, 0.1f, 0.1f, 1.0f);

    // === Stage 3: 合成 ===
    // Pass 3a: 背景全屏 (P2=2.0 → 纹理直通)
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {m_rtA};
        p.uniformFloats = {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f};
        be->DrawFullscreenQuad(m_sharedVert, m_glassShader, p);
    }
    // Pass 3b: squircle 玻璃 (P2=1.0 → LiquidGlass)
    {
        TextureHandle blurTex = (m_blurIters > 0) ? m_rtC : m_rtA;
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {blurTex};
        p.uniformFloats = {m_powerFactor, m_fPower, 1.0f, m_noise, m_glowWeight, m_glowBias};
        p.mvp = {
            m_a, m_b, m_c, m_d,
            m_glowEdge0, m_glowEdge1, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        };
        be->DrawFullscreenQuad(m_sharedVert, m_glassShader, p);
    }
}
```

- [ ] **Step 5: 添加 OnImGui**

```cpp
void LiquidGlassScene::OnImGui() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("LiquidGlass", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("FPS: %.1f", m_fpsDisplay);

    // 背景选择
    if (!m_bgNames.empty()) {
        if (ImGui::BeginCombo("Background", m_bgNames[m_currentBgIndex].c_str())) {
            for (int i = 0; i < (int)m_bgNames.size(); i++) {
                bool sel = (i == m_currentBgIndex);
                if (ImGui::Selectable(m_bgNames[i].c_str(), sel))
                    m_currentBgIndex = i;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    if (ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Power", &m_powerFactor, 1.001f, 6.0f);
    }

    if (ImGui::CollapsingHeader("Blur & Noise")) {
        ImGui::SliderInt("Blur Iters", &m_blurIters, 0, 10);
        ImGui::SliderFloat("Blur Radius", &m_blurRadius, 0.0f, 10.0f);
        if (ImGui::SliderFloat("Blur Downscale", &m_blurDownscale, 0.1f, 1.0f)) {
            // 改变 downscale 时标记 RT 需重建
            m_lastRTSizeW = 0;
        }
        ImGui::SliderFloat("Noise", &m_noise, 0.0f, 0.3f);
    }

    if (ImGui::CollapsingHeader("Refraction")) {
        ImGui::Text("f(x) = 1 - b*(c*e)^(-d*x - a)");
        ImGui::SliderFloat("f(x) Power", &m_fPower, -1.5f, 6.0f);
        ImGui::SliderFloat("a", &m_a, 0.0f, 5.0f);
        ImGui::SliderFloat("b", &m_b, 0.0f, 6.0f);
        ImGui::SliderFloat("c", &m_c, 0.0f, 6.0f);
        ImGui::SliderFloat("d", &m_d, 0.0f, 10.0f);
    }

    if (ImGui::CollapsingHeader("Glow")) {
        ImGui::SliderFloat("Weight", &m_glowWeight, -1.0f, 1.0f);
        ImGui::SliderFloat("Bias", &m_glowBias, -1.0f, 1.0f);
        ImGui::SliderFloat("Edge0", &m_glowEdge0, -1.0f, 1.0f);
        ImGui::SliderFloat("Edge1", &m_glowEdge1, -1.0f, 1.0f);
    }

    ImGui::End();
}
```

- [ ] **Step 6: Commit**

```bash
git add src/app/LiquidGlassScene.cpp
git commit -m "feat: LiquidGlassScene 完整实现 (3Stage渲染管线+ImGui面板)"
```

---

### Task 8: 注册场景到 main.cpp

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: 添加 include 和注册代码**

在 `src/main.cpp` 顶部添加 include：

```cpp
#include "app/LiquidGlassScene.h"
```

在 `SetFrameCallback` lambda 中，AUS3DScene 注册之后添加：

```cpp
SceneRegistry::Instance().Register({
    "liquid-glass",
    "Liquid Glass",
    "特效",
    "Apple Liquid Glass 风格玻璃效果 — 超椭圆SDF折射+高斯模糊+发光",
    "assets/images/thumb_liquid_glass.jpg",
    [be, &app]() -> std::unique_ptr<Scene> {
        auto s = std::make_unique<LiquidGlassScene>();
        s->SetBackend(be);
        s->SetApplication(&app);
        return s;
    },
    true
});
```

**注意**: 工厂 lambda 必须按值捕获 `[be, &app]`，`be` 是指针值捕获安全，`app` 是全局变量引用捕获安全。

- [ ] **Step 2: Commit**

```bash
git add src/main.cpp
git commit -m "feat: 注册LiquidGlass场景到Gallery"
```

---

### Task 9: 构建并验证

- [ ] **Step 1: 关闭现有进程并构建**

```powershell
taskkill /F /IM ShaderShowcase.exe 2>$null
Start-Sleep -Seconds 1
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release 2>&1 | Select-Object -Last 15
```

Expected: 构建成功，无错误

- [ ] **Step 2: 运行验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release
Start-Process -FilePath '.\ShaderShowcase.exe'
```

Expected: 程序启动，Gallery 中出现 "Liquid Glass" 卡片

- [ ] **Step 3: 手动验证清单**

- [ ] Gallery 中可见 "Liquid Glass" 卡片
- [ ] 点击进入 LiquidGlass 场景
- [ ] 超椭圆 squircle 正确渲染，边缘有折射效果
- [ ] 背景模糊可见（13-tap 高斯）
- [ ] 噪声和发光效果正确叠加
- [ ] ImGui 面板所有参数可调节并即时生效
- [ ] 背景切换下拉菜单正常工作
- [ ] ESC 返回 Gallery
- [ ] 无明显性能问题（FPS > 30）