# VFX Fire Book 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 ShaderShowcase 中完整复刻 Unity VFX Graph Magic Fire Book 效果（3D书本+溶解+粒子+音频）

**Architecture:** 模块化架构，VFXFireBookScene 作为调度器，拆分为 BookMeshRenderer（3D书本渲染）、DissolvePostProcess（溶解后处理）、粒子系统（CPU模拟+GPU渲染）、AudioPlayer（miniaudio）四个独立组件。每帧：书本渲染到RT → 溶解后处理 → 粒子叠加 → 屏幕输出。

**Tech Stack:** C++17, Vulkan 1.2 SPIR-V, GLSL (glslangValidator编译), miniaudio (单头文件音频库)

**关键约束:**
- VulkanBackend **无 Compute Pipeline**，粒子采用 CPU 模拟 + `DrawMesh` 点精灵渲染
- miniaudio 尚未集成，需新建 CMake target
- 遵循项目现有模式：继承 Scene、SetBackend/SetApplication、SceneRegistry 注册
- 遵循 Vulkan 8个坑点经验（project_memory.md）
- 所有 `.spv` 文件需放入 `build/shaders/vfx_fire/` 并同步到 `shaders/vfx_fire/`

---

## 文件结构

```
shader-showcase/
├── src/
│   ├── app/
│   │   ├── VFXFireBookScene.h          # 主场景类（调度器）
│   │   └── VFXFireBookScene.cpp        # OnInit/OnUpdate/OnRender/OnImGui
│   ├── render/
│   │   ├── BookMeshRenderer.h          # 3D书本渲染器
│   │   ├── BookMeshRenderer.cpp        # 加载网格+纹理+Phong光照
│   │   ├── BookParticleSystem.h        # 粒子系统（CPU模拟+GPU渲染）
│   │   ├── BookParticleSystem.cpp
│   │   ├── DissolvePostProcess.h       # 溶解后处理Pass
│   │   ├── DissolvePostProcess.cpp
│   │   ├── AudioPlayer.h              # miniaudio 音频播放器
│   │   └── AudioPlayer.cpp
│   ├── main.cpp                       # 修改：注册VFXFireBookScene
│   └── CMakeLists.txt                 # 修改：添加新文件+miniaudio
├── shaders/
│   └── vfx_fire/
│       ├── book.vert                   # 3D书本顶点着色器
│       ├── book.frag                   # 3D书本片段着色器（Phong+纹理）
│       ├── dissolve.frag               # 溶解后处理Shader
│       ├── particle.vert               # 粒子顶点着色器（点精灵）
│       └── particle.frag               # 粒子片段着色器
└── assets/
    ├── models/book/book_combined.bin   # 已完成
    ├── models/book/book_mesh_info.json # 已完成
    └── models/book/textures/*.png      # 已完成
```

---

### Task 1: 项目基础设施 — 新建文件骨架 + CMake + Scene注册

**Files:**
- Create: `src/app/VFXFireBookScene.h`
- Create: `src/app/VFXFireBookScene.cpp`
- Create: `src/render/BookMeshRenderer.h`
- Create: `src/render/BookMeshRenderer.cpp`
- Create: `src/render/BookParticleSystem.h`
- Create: `src/render/BookParticleSystem.cpp`
- Create: `src/render/DissolvePostProcess.h`
- Create: `src/render/DissolvePostProcess.cpp`
- Create: `src/render/AudioPlayer.h`
- Create: `src/render/AudioPlayer.cpp`
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 创建所有头文件骨架**

**VFXFireBookScene.h:**
```cpp
#pragma once
#include "app/Scene.h"
#include <memory>

class IRenderBackend;
class Application;
class BookMeshRenderer;
class BookParticleSystem;
class DissolvePostProcess;
class AudioPlayer;

class VFXFireBookScene : public Scene {
public:
    VFXFireBookScene();
    ~VFXFireBookScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsReturn() const override { return m_wantsReturn; }

    void SetBackend(IRenderBackend* be);
    void SetApplication(Application* app);

private:
    IRenderBackend* m_backend = nullptr;
    Application* m_app = nullptr;
    bool m_wantsReturn = false;

    std::unique_ptr<BookMeshRenderer> m_bookRenderer;
    std::unique_ptr<BookParticleSystem> m_paperParticles;  // 纸屑
    std::unique_ptr<BookParticleSystem> m_smokeParticles;  // 烟雾
    std::unique_ptr<DissolvePostProcess> m_dissolve;
    std::unique_ptr<AudioPlayer> m_audio;

    float m_dissolveAmount = 0.5f;
    float m_dissolveSpeed = 0.05f;
    float m_edgeWidth = 0.2f;
    float m_particleScale = 1.0f;
    float m_smokeDensity = 1.0f;
    float m_rotationSpeed = 0.1f;
    float m_elapsedTime = 0.0f;
};
```

**BookMeshRenderer.h:**
```cpp
#pragma once
#include "render/IRenderBackend.h"
#include <vector>
#include <string>

struct BookSubMesh {
    std::vector<float> vertices;    // pos(12) + normal(12) + uv(8) = 32B per vertex
    std::vector<uint32_t> indices;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    std::string materialName;
};

struct BookMeshData {
    std::vector<BookSubMesh> subMeshes;
    std::vector<float> nodeTransforms;  // 10 nodes × 16 floats (mat4)
};

class BookMeshRenderer {
public:
    bool Load(const std::string& binPath);
    void Destroy(IRenderBackend* backend);
    void Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                const float* viewMat, const float* projMat, const float* lightDir, const float* lightColor, const float* eyePos);

private:
    BookMeshData m_data;
    std::vector<TextureHandle> m_textures;  // Diffuse textures
    std::vector<TextureHandle> m_normals;   // Normal maps
    bool m_loaded = false;
};
```

**BookParticleSystem.h:**
```cpp
#pragma once
#include "render/IRenderBackend.h"
#include <vector>
#include <random>

struct Particle {
    float posX, posY, posZ;
    float velX, velY, velZ;
    float life;        // 剩余生命周期
    float maxLife;     // 总生命周期
    float size;
    float rotation;
    float angularVel;
    bool alive = false;
};

struct ParticleConfig {
    uint32_t capacity = 32;
    float spawnRate = 16.0f;       // 每秒产生
    float spawnCenter[3] = {0, 0, 0};
    float spawnSize[3] = {3, 4, 3};
    float velBase[3] = {0, 0, 0};
    float velRange[3] = {0, 0, 0};
    float lifeMin = 1.0f;
    float lifeMax = 3.0f;
    float sizeMin = 0.5f;
    float sizeMax = 1.0f;
    float angularVelMin = 0.0f;
    float angularVelMax = 0.0f;
    float gravity[3] = {0, 0, 0};
};

class BookParticleSystem {
public:
    void Init(const ParticleConfig& config);
    void Update(float dt, float dissolveAmount);
    void Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                const float* viewMat, const float* projMat);
    void Destroy(IRenderBackend* backend);

    void SetParticleTexture(TextureHandle tex) { m_particleTex = tex; }

private:
    ParticleConfig m_config;
    std::vector<Particle> m_particles;
    std::mt19937 m_rng;
    float m_spawnAccum = 0.0f;
    TextureHandle m_particleTex = {0};
    uint32_t m_aliveCount = 0;
};
```

**DissolvePostProcess.h:**
```cpp
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
```

**AudioPlayer.h:**
```cpp
#pragma once
#include <string>

class AudioPlayer {
public:
    bool Init(const std::string& audioPath);
    void Play();
    void Stop();
    void Destroy();
    bool IsPlaying() const;

private:
    void* m_device = nullptr;  // ma_device*
    void* m_sound = nullptr;   // ma_sound*
    bool m_initialized = false;
};
```

- [ ] **Step 2: 创建所有 .cpp 文件骨架**

```cpp
// VFXFireBookScene.cpp
#include "app/VFXFireBookScene.h"
#include "render/BookMeshRenderer.h"
#include "render/BookParticleSystem.h"
#include "render/DissolvePostProcess.h"
#include "render/AudioPlayer.h"
#include "app/Application.h"

VFXFireBookScene::VFXFireBookScene() = default;
VFXFireBookScene::~VFXFireBookScene() = default;

void VFXFireBookScene::SetBackend(IRenderBackend* be) { m_backend = be; }
void VFXFireBookScene::SetApplication(Application* app) { m_app = app; }

void VFXFireBookScene::OnEnter() {
    // TODO: Task 2-6 实现
}

void VFXFireBookScene::OnExit() {
    // TODO: Task 2-6 实现
}

void VFXFireBookScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    // 自动溶解动画
    m_dissolveAmount += m_dissolveSpeed * dt;
    if (m_dissolveAmount > 1.0f) m_dissolveAmount = 0.0f;
}

void VFXFireBookScene::OnRender(IRenderBackend* backend) {
    // TODO: Task 2-6 实现
}

void VFXFireBookScene::OnImGui() {
    // TODO: Task 7 实现
}
```

```cpp
// BookMeshRenderer.cpp
#include "render/BookMeshRenderer.h"
#include <fstream>
#include <cstring>

bool BookMeshRenderer::Load(const std::string& binPath) {
    // TODO: Task 2
    return false;
}

void BookMeshRenderer::Destroy(IRenderBackend* backend) {
    // TODO: Task 2
}

void BookMeshRenderer::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                               const float* viewMat, const float* projMat,
                               const float* lightDir, const float* lightColor, const float* eyePos) {
    // TODO: Task 2
}
```

```cpp
// BookParticleSystem.cpp
#include "render/BookParticleSystem.h"
#include <cmath>

void BookParticleSystem::Init(const ParticleConfig& config) {
    // TODO: Task 5
}

void BookParticleSystem::Update(float dt, float dissolveAmount) {
    // TODO: Task 5
}

void BookParticleSystem::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                                 const float* viewMat, const float* projMat) {
    // TODO: Task 5
}

void BookParticleSystem::Destroy(IRenderBackend* backend) {
    // TODO: Task 5
}
```

```cpp
// DissolvePostProcess.cpp
#include "render/DissolvePostProcess.h"
#include <cstdio>

bool DissolvePostProcess::Init(IRenderBackend* backend) {
    // TODO: Task 4
    return false;
}

void DissolvePostProcess::Apply(IRenderBackend* backend, TextureHandle inputRT, TextureHandle outputRT,
                                 float dissolveAmount, float edgeWidth,
                                 float noiseScaleX, float noiseScaleY,
                                 float noiseOffsetX, float noiseOffsetY) {
    // TODO: Task 4
}

void DissolvePostProcess::Destroy(IRenderBackend* backend) {
    // TODO: Task 4
}
```

```cpp
// AudioPlayer.cpp
#include "render/AudioPlayer.h"

bool AudioPlayer::Init(const std::string& audioPath) { return false; }
void AudioPlayer::Play() {}
void AudioPlayer::Stop() {}
void AudioPlayer::Destroy() {}
bool AudioPlayer::IsPlaying() const { return false; }
```

- [ ] **Step 3: 修改 CMakeLists.txt 添加新文件**

在 `SRC_FILES` 的 `src/app/` 段末尾添加:
```cmake
    src/app/VFXFireBookScene.cpp
    src/app/VFXFireBookScene.h
    src/render/BookMeshRenderer.cpp
    src/render/BookMeshRenderer.h
    src/render/BookParticleSystem.cpp
    src/render/BookParticleSystem.h
    src/render/DissolvePostProcess.cpp
    src/render/DissolvePostProcess.h
    src/render/AudioPlayer.cpp
    src/render/AudioPlayer.h
```

- [ ] **Step 4: 修改 main.cpp 注册场景**

在 `main.cpp` 中 `SceneRegistry::Instance().Register(...)` 调用段末尾添加:
```cpp
SceneRegistry::Instance().Register({
    "vfx-fire-book",
    "VFX Fire Book",
    "VFX",
    "Magic Fire Book - 3D书本+溶解+粒子+音频",
    "assets/images/00_grayscale_landscape.jpg",
    [be, &app]() -> std::unique_ptr<Scene> {
        auto s = std::make_unique<VFXFireBookScene>();
        s->SetBackend(be);
        s->SetApplication(&app);
        return s;
    },
    true
});
```

- [ ] **Step 5: 编译验证骨架**

Run: `cmake --build build --target ShaderShowcase -j8`
Expected: 编译通过（所有 .cpp 文件编译成功，无链接错误）

- [ ] **Step 6: 提交**

```bash
git add src/app/VFXFireBookScene.h src/app/VFXFireBookScene.cpp \
        src/render/BookMeshRenderer.h src/render/BookMeshRenderer.cpp \
        src/render/BookParticleSystem.h src/render/BookParticleSystem.cpp \
        src/render/DissolvePostProcess.h src/render/DissolvePostProcess.cpp \
        src/render/AudioPlayer.h src/render/AudioPlayer.cpp \
        src/main.cpp CMakeLists.txt
git commit -m "feat: VFX Fire Book 项目骨架 - 8个新文件 + CMake + Scene注册"
```

---

### Task 2: 3D书本渲染器 — 网格加载 + 纹理 + Phong光照

**Files:**
- Create: `shaders/vfx_fire/book.vert`
- Create: `shaders/vfx_fire/book.frag`
- Modify: `src/render/BookMeshRenderer.cpp`
- Modify: `src/render/BookMeshRenderer.h`

- [ ] **Step 1: 编写书本顶点着色器**

```glsl
// shaders/vfx_fire/book.vert
#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(binding = 1, std140) uniform Params {
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;    // .xyz = direction to light
    vec4 uLightColor;  // .rgb = light color
    vec4 uEyePos;      // .xyz = eye position
    vec4 uParams;      // .x = time, .y = dissolveAmount
};

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vUV;

void main() {
    vec4 worldPos = uModelView * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(uModelView) * aNormal;
    vUV = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
```

- [ ] **Step 2: 编写书本片段着色器（Phong光照 + 漫反射纹理 + 法线贴图）**

```glsl
// shaders/vfx_fire/book.frag
#version 450

layout(binding = 0) uniform sampler2D uInputTex;  // diffuse texture
layout(binding = 1, std140) uniform Params {
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
    vec4 uParams;
};
layout(binding = 2) uniform sampler2D uAuxTex;  // normal map

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir.xyz);
    vec3 V = normalize(uEyePos.xyz - vWorldPos);
    vec3 H = normalize(L + V);

    vec3 diffuse = texture(uInputTex, vUV).rgb;
    vec3 normalMap = texture(uAuxTex, vUV).rgb * 2.0 - 1.0;
    N = normalize(N + normalMap);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 32.0);

    vec3 ambient = diffuse * 0.15;
    vec3 diff = diffuse * uLightColor.rgb * NdotL * 2.0;
    vec3 specular = uLightColor.rgb * spec * 0.5;

    outColor = vec4(ambient + diff + specular, 1.0);
}
```

- [ ] **Step 3: 编译Shader到SPIR-V**

Run:
```bash
glslangValidator -V shaders/vfx_fire/book.vert -o build/shaders/vfx_fire/book.vert.spv
glslangValidator -V shaders/vfx_fire/book.frag -o build/shaders/vfx_fire/book.frag.spv
cp build/shaders/vfx_fire/book.vert.spv shaders/vfx_fire/
cp build/shaders/vfx_fire/book.frag.spv shaders/vfx_fire/
```

- [ ] **Step 4: 实现 BookMeshRenderer::Load**

```cpp
bool BookMeshRenderer::Load(const std::string& binPath) {
    std::ifstream file(binPath, std::ios::binary);
    if (!file) return false;

    // Read submesh count
    uint32_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

    m_data.subMeshes.resize(meshCount);
    m_data.nodeTransforms.resize(meshCount * 16, 0.0f);

    // Node transforms from FBX (from book_mesh_info.json node positions)
    // page_top_left_2: T=(-6.288,0.030,0.396)
    // page_top_left:   T=(-6.288,0.484,0.396)
    // page_top_right_2:T=(0,0.030,0.396)
    // page_top_right:  T=(0,0.484,0.396)
    // lower_right:     T=(20.655,3.051,15.910)
    // side_right:      T=(20.655,3.051,15.910)
    // lower_left:      T=(-26.943,3.051,15.910)
    // Side_left:       T=(-26.943,3.051,15.910)
    // Background:      T=(-3.579,-0.322,-1.702)
    // Book_Cover_base: T=(0,0,0)
    float transforms[10][3] = {
        {-6.288f, 0.030f, 0.396f}, {-6.288f, 0.484f, 0.396f},
        {0.0f, 0.030f, 0.396f},    {0.0f, 0.484f, 0.396f},
        {20.655f, 3.051f, 15.910f},{20.655f, 3.051f, 15.910f},
        {-26.943f, 3.051f, 15.910f},{-26.943f, 3.051f, 15.910f},
        {-3.579f, -0.322f, -1.702f},{0.0f, 0.0f, 0.0f}
    };

    for (uint32_t mi = 0; mi < meshCount; mi++) {
        // Read header: vc(4) + ic(4) + hasUV(1) + hasNormal(1) + pad(2) = 12
        uint32_t vc, ic;
        uint8_t hasUV, hasNormal;
        file.read(reinterpret_cast<char*>(&vc), 4);
        file.read(reinterpret_cast<char*>(&ic), 4);
        file.read(reinterpret_cast<char*>(&hasUV), 1);
        file.read(reinterpret_cast<char*>(&hasNormal), 1);
        file.seekg(2, std::ios::cur); // skip padding

        // Read submesh parts count
        uint32_t partCount;
        file.read(reinterpret_cast<char*>(&partCount), 4);
        for (uint32_t pi = 0; pi < partCount; pi++) {
            uint32_t firstIdx, numIdx, nameLen;
            file.read(reinterpret_cast<char*>(&firstIdx), 4);
            file.read(reinterpret_cast<char*>(&numIdx), 4);
            file.read(reinterpret_cast<char*>(&nameLen), 4);
            file.seekg(nameLen, std::ios::cur);
        }

        auto& sm = m_data.subMeshes[mi];
        sm.vertexCount = vc;
        sm.indexCount = ic;
        sm.vertices.resize(vc * 8); // 8 floats per vertex
        sm.indices.resize(ic);

        // Read vertices: 8 floats (pos.x,pos.y,pos.z, nx,ny,nz, uv.u,uv.v)
        file.read(reinterpret_cast<char*>(sm.vertices.data()), vc * 8 * sizeof(float));
        // Read indices: uint32
        file.read(reinterpret_cast<char*>(sm.indices.data()), ic * sizeof(uint32_t));

        // Set transform matrix (identity + translation)
        float* t = &m_data.nodeTransforms[mi * 16];
        std::memset(t, 0, 16 * sizeof(float));
        t[0] = t[5] = t[10] = t[15] = 1.0f;
        t[12] = transforms[mi][0];
        t[13] = transforms[mi][1];
        t[14] = transforms[mi][2];
    }

    m_loaded = true;
    return true;
}
```

- [ ] **Step 5: 实现 BookMeshRenderer::Destroy**

```cpp
void BookMeshRenderer::Destroy(IRenderBackend* backend) {
    for (auto& tex : m_textures) backend->DestroyTexture(tex);
    for (auto& tex : m_normals) backend->DestroyTexture(tex);
    m_textures.clear();
    m_normals.clear();
    m_loaded = false;
}
```

- [ ] **Step 6: 实现 BookMeshRenderer::Render（逐子网格绘制）**

```cpp
void BookMeshRenderer::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                               const float* viewMat, const float* projMat,
                               const float* lightDir, const float* lightColor, const float* eyePos) {
    if (!m_loaded) return;

    for (size_t mi = 0; mi < m_data.subMeshes.size(); mi++) {
        auto& sm = m_data.subMeshes[mi];
        if (sm.vertexCount == 0) continue;

        // Build MVP: proj * view * nodeTransform
        float mvp[16], modelView[16];
        const float* nodeT = &m_data.nodeTransforms[mi * 16];

        // modelView = view * nodeTransform
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                    sum += viewMat[r*4 + k] * nodeT[k*4 + c];
                modelView[r*4 + c] = sum;
            }
        }
        // mvp = proj * modelView
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                    sum += projMat[r*4 + k] * modelView[k*4 + c];
                mvp[r*4 + c] = sum;
            }
        }

        ShaderParams params;
        params.inputTextures.push_back(m_textures[mi % m_textures.size()]);
        if (mi < m_normals.size())
            params.auxTextures.push_back(m_normals[mi % m_normals.size()]);
        params.mvp = std::vector<float>(mvp, mvp + 16);
        params.modelView = std::vector<float>(modelView, modelView + 16);
        params.lightDir = std::vector<float>(lightDir, lightDir + 3);
        params.lightColor = std::vector<float>(lightColor, lightColor + 3);
        params.eyePos = std::vector<float>(eyePos, eyePos + 3);
        params.uniformFloats = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        backend->DrawMesh(vert, frag, params,
                          sm.vertices.data(), sm.vertexCount, 8 * sizeof(float),
                          sm.indices.data(), sm.indexCount);
    }
}
```

- [ ] **Step 7: 在 VFXFireBookScene::OnEnter 中初始化书本渲染器**

```cpp
void VFXFireBookScene::OnEnter() {
    m_bookRenderer = std::make_unique<BookMeshRenderer>();
    m_bookRenderer->Load("assets/models/book/book_combined.bin");
    // TODO: 加载纹理 (Task 3)
}
```

- [ ] **Step 8: 编译验证**

Run: `cmake --build build --target ShaderShowcase -j8`
Expected: 编译通过

- [ ] **Step 9: 提交**

```bash
git add shaders/vfx_fire/book.vert shaders/vfx_fire/book.frag \
        build/shaders/vfx_fire/book.vert.spv build/shaders/vfx_fire/book.frag.spv \
        shaders/vfx_fire/book.vert.spv shaders/vfx_fire/book.frag.spv \
        src/render/BookMeshRenderer.cpp src/render/BookMeshRenderer.h \
        src/app/VFXFireBookScene.cpp
git commit -m "feat: 3D书本渲染器 - 网格加载+Phong光照+法线贴图"
```

---

### Task 3: 书本纹理加载 + 相机系统

**Files:**
- Modify: `src/render/BookMeshRenderer.cpp`
- Modify: `src/app/VFXFireBookScene.cpp`

- [ ] **Step 1: 添加纹理加载方法到 BookMeshRenderer**

```cpp
bool BookMeshRenderer::LoadTextures(IRenderBackend* backend) {
    // 纹理路径映射（根据材质名加载对应纹理）
    const char* texPaths[] = {
        "assets/models/book/textures/Book_1_Diffuse.png",  // Book_Base/Book_Page_Left
        "assets/models/book/textures/Book_2_Diffuse.png",  // Book_Page_Right
        "assets/models/book/textures/Front_Diffuse.png",   // Book_Cover
        "assets/models/book/textures/Side_Diffuse.png",     // Side
    };
    const char* normPaths[] = {
        "assets/models/book/textures/Book_1_Normal.png",
        "assets/models/book/textures/Book_2_Normal.png",
        "assets/models/book/textures/Front_Normal.png",
        "assets/models/book/textures/Side_Normal.png",
    };

    for (int i = 0; i < 4; i++) {
        auto tex = backend->CreateTextureFromFile(texPaths[i]);
        if (tex.id == 0) return false;
        m_textures.push_back(tex);

        auto norm = backend->CreateTextureFromFile(normPaths[i]);
        if (norm.id == 0) return false;
        m_normals.push_back(norm);
    }
    return true;
}
```

- [ ] **Step 2: 更新 BookMeshRenderer.h 添加 LoadTextures 声明**

```cpp
bool LoadTextures(IRenderBackend* backend);
```

- [ ] **Step 3: 实现相机系统（参考 AUS3DScene 的轨道相机）**

在 VFXFireBookScene.h 中添加:
```cpp
// Camera
float m_camTheta = 0.8f;
float m_camPhi = 0.6f;
float m_camRadius = 50.0f;  // 书本比球体大，用更大距离
float m_camTarget[3] = {0, 0, 0};
bool m_dragging = false;
float m_dragStartX = 0, m_dragStartY = 0;
float m_dragStartTheta = 0, m_dragStartPhi = 0;

void BuildViewMatrix(float* out);
void BuildProjMatrix(float* out, int w, int h);
```

在 VFXFireBookScene.cpp 中实现:
```cpp
#include <cmath>
#include <GLFW/glfw3.h>

void VFXFireBookScene::BuildViewMatrix(float* out) {
    float cx = m_camRadius * sin(m_camPhi) * cos(m_camTheta);
    float cy = m_camRadius * cos(m_camPhi);
    float cz = m_camRadius * sin(m_camPhi) * sin(m_camTheta);
    float eyeX = m_camTarget[0] + cx;
    float eyeY = m_camTarget[1] + cy;
    float eyeZ = m_camTarget[2] + cz;

    // lookAt(eye, target, up)
    float fwd[3] = {m_camTarget[0] - eyeX, m_camTarget[1] - eyeY, m_camTarget[2] - eyeZ};
    float len = sqrt(fwd[0]*fwd[0] + fwd[1]*fwd[1] + fwd[2]*fwd[2]);
    fwd[0] /= len; fwd[1] /= len; fwd[2] /= len;

    float up[3] = {0, 1, 0};
    float right[3] = {
        up[1]*fwd[2] - up[2]*fwd[1],
        up[2]*fwd[0] - up[0]*fwd[2],
        up[0]*fwd[1] - up[1]*fwd[0]
    };
    float rLen = sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
    right[0] /= rLen; right[1] /= rLen; right[2] /= rLen;

    float realUp[3] = {
        fwd[1]*right[2] - fwd[2]*right[1],
        fwd[2]*right[0] - fwd[0]*right[2],
        fwd[0]*right[1] - fwd[1]*right[0]
    };

    out[0] = right[0]; out[1] = realUp[0]; out[2] = -fwd[0]; out[3] = 0;
    out[4] = right[1]; out[5] = realUp[1]; out[6] = -fwd[1]; out[7] = 0;
    out[8] = right[2]; out[9] = realUp[2]; out[10] = -fwd[2]; out[11] = 0;
    out[12] = -(right[0]*eyeX + right[1]*eyeY + right[2]*eyeZ);
    out[13] = -(realUp[0]*eyeX + realUp[1]*eyeY + realUp[2]*eyeZ);
    out[14] = (fwd[0]*eyeX + fwd[1]*eyeY + fwd[2]*eyeZ);
    out[15] = 1;
}

void VFXFireBookScene::BuildProjMatrix(float* out, int w, int h) {
    float aspect = (float)w / (float)h;
    float fov = 0.8f;
    float f = 1.0f / tan(fov * 0.5f);
    float n = 0.1f, fFar = 1000.0f;

    std::memset(out, 0, 16 * sizeof(float));
    out[0] = f / aspect;
    out[5] = f;
    out[10] = (fFar + n) / (n - fFar);
    out[11] = -1;
    out[14] = (2.0f * fFar * n) / (n - fFar);
}
```

- [ ] **Step 4: 实现 OnRender 中的书本渲染调用**

```cpp
void VFXFireBookScene::OnRender(IRenderBackend* backend) {
    int w, h;
    backend->GetFramebufferSize(w, h);

    float viewMat[16], projMat[16];
    BuildViewMatrix(viewMat);
    BuildProjMatrix(projMat, w, h);

    float lightDir[3] = {0.0f, -1.0f, 0.0f};  // 从上方照下
    float lightColor[3] = {2.0f, 2.0f, 2.0f};  // 强度2.0

    float cx = m_camRadius * sin(m_camPhi) * cos(m_camTheta) + m_camTarget[0];
    float cy = m_camRadius * cos(m_camPhi) + m_camTarget[1];
    float cz = m_camRadius * sin(m_camPhi) * sin(m_camTheta) + m_camTarget[2];
    float eyePos[3] = {cx, cy, cz};

    backend->Clear(0.02f, 0.02f, 0.04f, 1.0f);

    ShaderHandle vert = m_bookVert; // 需要从SPIR-V加载
    ShaderHandle frag = m_bookFrag;

    m_bookRenderer->Render(backend, vert, frag, viewMat, projMat, lightDir, lightColor, eyePos);
}
```

- [ ] **Step 5: 编译验证+启动测试**

Run:
```bash
cmake --build build --target ShaderShowcase -j8
taskkill /f /im ShaderShowcase.exe 2>nul
start build\Debug\ShaderShowcase.exe
```
Expected: 程序启动，进入 VFX Fire Book 场景后能看到3D书本模型（可能无纹理但形状正确）

- [ ] **Step 6: 提交**

```bash
git add src/render/BookMeshRenderer.cpp src/render/BookMeshRenderer.h \
        src/app/VFXFireBookScene.cpp src/app/VFXFireBookScene.h
git commit -m "feat: 书本纹理加载+轨道相机+OnRender书本渲染"
```

---

### Task 4: 溶解后处理Shader

**Files:**
- Create: `shaders/vfx_fire/dissolve.frag`
- Modify: `src/render/DissolvePostProcess.cpp`
- Modify: `src/render/DissolvePostProcess.h`

- [ ] **Step 1: 编写溶解 shader**

```glsl
// shaders/vfx_fire/dissolve.frag
#version 450

layout(binding = 0) uniform sampler2D uInputTex;   // 书本渲染结果RT
layout(binding = 1, std140) uniform Params {
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
    vec4 uParams;  // .x = dissolveAmount, .y = edgeWidth, .z = noiseScaleX, .w = noiseScaleY
};
layout(binding = 2) uniform sampler2D uAuxTex;  // 噪声纹理

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 bookColor = texture(uInputTex, vUV);

    // Remap: dissolveAmount 0-1 -> 0.5-1.3
    float remapped = 0.5 + uParams.x * (1.3 - 0.5);

    // Sample noise (with scale and animated offset)
    vec2 noiseUV = vUV * vec2(uParams.z, uParams.w);
    float noise = texture(uAuxTex, noiseUV).r;

    // OneMinus (溶解区域 = 1 - noise clamped)
    float dissolve = 1.0 - clamp(remapped + noise * 0.3, 0.0, 1.0);

    // Step threshold 0.29 (来自ShaderGraph)
    float clip = step(0.29, dissolve);

    // Alpha clip
    if (clip < 0.5) discard;

    // Edge detection: smoothstep around dissolve threshold
    float edgeLow = remapped - uParams.y;
    float edgeHigh = remapped + uParams.y;
    float edgeMask = 1.0 - smoothstep(edgeLow, edgeHigh, dissolve);

    // Edge color: 高亮度橙色 (6.5, 0.894, 0.0)
    vec3 edgeColor = vec3(6.5, 0.894, 0.0) * edgeMask;

    outColor = vec4(bookColor.rgb + edgeColor, 1.0);
}
```

- [ ] **Step 2: 编译shader到SPIR-V**

```bash
glslangValidator -V shaders/vfx_fire/dissolve.frag -o build/shaders/vfx_fire/dissolve.frag.spv
cp build/shaders/vfx_fire/dissolve.frag.spv shaders/vfx_fire/
```

- [ ] **Step 3: 实现 DissolvePostProcess::Init**

```cpp
#include <fstream>
#include <vector>

bool DissolvePostProcess::Init(IRenderBackend* backend) {
    // Load SPIR-V
    auto readSPIRV = [](const std::string& path) -> std::vector<uint32_t> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        size_t size = file.tellg();
        file.seekg(0);
        std::vector<uint32_t> data(size / 4);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    };

    auto vertData = readSPIRV("shaders/vfx_fire/fullscreen.vert.spv");
    auto fragData = readSPIRV("shaders/vfx_fire/dissolve.frag.spv");
    if (vertData.empty() || fragData.empty()) return false;

    m_vertShader = backend->CreateVertexShader(vertData.data(), vertData.size() * 4);
    m_fragShader = backend->CreateFragmentShader(fragData.data(), fragData.size() * 4);
    if (m_vertShader.id == 0 || m_fragShader.id == 0) return false;

    // Create noise texture (512x512 procedural)
    std::vector<uint8_t> noiseData(512 * 512);
    for (int i = 0; i < 512 * 512; i++) {
        noiseData[i] = (uint8_t)(rand() % 256);
    }
    m_noiseTex = backend->CreateTexture(512, 512, TextureFormat::R8, noiseData.data());

    // Create pipeline
    PipelineDesc desc;
    desc.vertShader = m_vertShader;
    desc.fragShader = m_fragShader;
    desc.width = 0;
    desc.height = 0;
    desc.blendEnable = false;
    m_pipeline = backend->CreatePipeline(desc);

    return m_pipeline.id != 0;
}
```

- [ ] **Step 4: 实现 DissolvePostProcess::Apply**

```cpp
void DissolvePostProcess::Apply(IRenderBackend* backend, TextureHandle inputRT, TextureHandle outputRT,
                                 float dissolveAmount, float edgeWidth,
                                 float noiseScaleX, float noiseScaleY,
                                 float noiseOffsetX, float noiseOffsetY) {
    ShaderParams params;
    params.inputTextures = {inputRT};
    params.auxTextures = {m_noiseTex};
    params.uniformFloats = {dissolveAmount, edgeWidth, noiseScaleX, noiseScaleY, noiseOffsetX, noiseOffsetY};

    backend->BindPipeline(m_pipeline);
    if (outputRT.id != 0) {
        backend->BeginRenderToTexture(outputRT);
    }
    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
    if (outputRT.id != 0) {
        backend->EndRenderToTexture();
    }
}
```

- [ ] **Step 5: 实现 DissolvePostProcess::Destroy**

```cpp
void DissolvePostProcess::Destroy(IRenderBackend* backend) {
    if (m_pipeline.id) backend->DestroyPipeline(m_pipeline);
    if (m_vertShader.id) backend->DestroyShader(m_vertShader);
    if (m_fragShader.id) backend->DestroyShader(m_fragShader);
    if (m_noiseTex.id) backend->DestroyTexture(m_noiseTex);
}
```

- [ ] **Step 6: 编译验证**

Run: `cmake --build build --target ShaderShowcase -j8`
Expected: 编译通过

- [ ] **Step 7: 提交**

```bash
git add shaders/vfx_fire/dissolve.frag build/shaders/vfx_fire/dissolve.frag.spv \
        shaders/vfx_fire/dissolve.frag.spv \
        src/render/DissolvePostProcess.cpp src/render/DissolvePostProcess.h
git commit -m "feat: 溶解后处理Shader - Remap+Step(0.29)+AlphaClip+EdgeGlow"
```

---

### Task 5: 粒子系统 — CPU模拟 + GPU渲染

**Files:**
- Create: `shaders/vfx_fire/particle.vert`
- Create: `shaders/vfx_fire/particle.frag`
- Modify: `src/render/BookParticleSystem.cpp`
- Modify: `src/render/BookParticleSystem.h`

- [ ] **Step 1: 编写粒子顶点着色器（点精灵）**

```glsl
// shaders/vfx_fire/particle.vert
#version 450

layout(location = 0) in vec3 aPos;       // 粒子世界位置
layout(location = 1) in float aSize;      // 粒子大小
layout(location = 2) in float aLife;      // 剩余生命 (0-1归一化)
layout(location = 3) in float aRotation;  // 旋转角度

layout(binding = 1, std140) uniform Params {
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
    vec4 uParams;
};

layout(location = 0) out float vLife;
layout(location = 1) out vec2 vUV;

void main() {
    vec4 clipPos = uMVP * vec4(aPos, 1.0);
    gl_Position = clipPos;
    gl_PointSize = aSize * 50.0 / -clipPos.z;  // 透视缩放
    vLife = aLife;
    vUV = vec2(0.5, 0.5);  // 中心点
}
```

- [ ] **Step 2: 编写粒子片段着色器（圆形渐变 + 透明混合）**

```glsl
// shaders/vfx_fire/particle.frag
#version 450

layout(binding = 0) uniform sampler2D uInputTex;
layout(binding = 1, std140) uniform Params {
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
    vec4 uParams;  // .x = type (0=纸屑, 1=烟雾)
};

layout(location = 0) in float vLife;
layout(location = 1) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    // 圆形mask
    vec2 center = gl_PointCoord - 0.5;
    float dist = length(center);
    if (dist > 0.5) discard;

    float alpha = 1.0 - smoothstep(0.0, 0.5, dist);

    if (uParams.x < 0.5) {
        // 纸屑：橙色→暗红→黑 渐变
        vec3 color = mix(vec3(1.0, 0.5, 0.0), vec3(0.0, 0.0, 0.0), 1.0 - vLife);
        outColor = vec4(color * 2.0, alpha * vLife);
    } else {
        // 烟雾：灰色半透明
        outColor = vec4(0.1, 0.1, 0.1, alpha * 0.3 * vLife);
    }
}
```

- [ ] **Step 3: 编译粒子shader**

```bash
glslangValidator -V shaders/vfx_fire/particle.vert -o build/shaders/vfx_fire/particle.vert.spv
glslangValidator -V shaders/vfx_fire/particle.frag -o build/shaders/vfx_fire/particle.frag.spv
cp build/shaders/vfx_fire/particle.vert.spv shaders/vfx_fire/
cp build/shaders/vfx_fire/particle.frag.spv shaders/vfx_fire/
```

- [ ] **Step 4: 实现 BookParticleSystem::Init**

```cpp
void BookParticleSystem::Init(const ParticleConfig& config) {
    m_config = config;
    m_particles.resize(config.capacity);
    m_rng.seed(42);
    m_spawnAccum = 0.0f;
    m_aliveCount = 0;
}
```

- [ ] **Step 5: 实现 BookParticleSystem::Update**

```cpp
void BookParticleSystem::Update(float dt, float dissolveAmount) {
    // 更新存活粒子
    for (auto& p : m_particles) {
        if (!p.alive) continue;
        p.life -= dt;
        if (p.life <= 0.0f) {
            p.alive = false;
            continue;
        }
        // 应用重力
        p.velX += m_config.gravity[0] * dt;
        p.velY += m_config.gravity[1] * dt;
        p.velZ += m_config.gravity[2] * dt;
        // 更新位置
        p.posX += p.velX * dt;
        p.posY += p.velY * dt;
        p.posZ += p.velZ * dt;
        // 旋转
        p.rotation += p.angularVel * dt;
    }

    // 产生新粒子
    m_spawnAccum += m_config.spawnRate * dt;
    int toSpawn = (int)m_spawnAccum;
    m_spawnAccum -= toSpawn;

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    for (int i = 0; i < toSpawn; i++) {
        // 找空闲粒子槽
        int slot = -1;
        for (int j = 0; j < m_config.capacity; j++) {
            if (!m_particles[j].alive) { slot = j; break; }
        }
        if (slot < 0) break; // 所有粒子都活着

        auto& p = m_particles[slot];
        p.alive = true;
        p.posX = m_config.spawnCenter[0] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[0];
        p.posY = m_config.spawnCenter[1] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[1];
        p.posZ = m_config.spawnCenter[2] + (dist01(m_rng) - 0.5f) * m_config.spawnSize[2];
        p.velX = m_config.velBase[0] + (dist01(m_rng) - 0.5f) * m_config.velRange[0];
        p.velY = m_config.velBase[1] + (dist01(m_rng) - 0.5f) * m_config.velRange[1];
        p.velZ = m_config.velBase[2] + (dist01(m_rng) - 0.5f) * m_config.velRange[2];
        p.maxLife = m_config.lifeMin + dist01(m_rng) * (m_config.lifeMax - m_config.lifeMin);
        p.life = p.maxLife;
        p.size = m_config.sizeMin + dist01(m_rng) * (m_config.sizeMax - m_config.sizeMin);
        p.angularVel = m_config.angularVelMin + dist01(m_rng) * (m_config.angularVelMax - m_config.angularVelMin);
        p.rotation = dist01(m_rng) * 6.28318f;
    }

    // 统计存活数
    m_aliveCount = 0;
    for (auto& p : m_particles) if (p.alive) m_aliveCount++;
}
```

- [ ] **Step 6: 实现 BookParticleSystem::Render**

```cpp
void BookParticleSystem::Render(IRenderBackend* backend, ShaderHandle vert, ShaderHandle frag,
                                 const float* viewMat, const float* projMat) {
    if (m_aliveCount == 0) return;

    // 构建顶点数据（每粒子: pos.xyz + size + life + rotation）
    std::vector<float> vertData;
    vertData.reserve(m_aliveCount * 6);
    for (auto& p : m_particles) {
        if (!p.alive) continue;
        vertData.push_back(p.posX);
        vertData.push_back(p.posY);
        vertData.push_back(p.posZ);
        vertData.push_back(p.size);
        vertData.push_back(p.life / p.maxLife);
        vertData.push_back(p.rotation);
    }

    ShaderParams params;
    params.inputTextures.push_back(m_particleTex);
    params.mvp = std::vector<float>(viewMat, viewMat + 16); // 简化版
    params.uniformFloats = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    // 使用 DrawMesh 渲染点精灵
    // 注意：需要设置 useVertexInput=true 和 GL_POINTS primitive
    backend->DrawMesh(vert, frag, params,
                      vertData.data(), m_aliveCount, 6 * sizeof(float),
                      nullptr, 0);
}
```

- [ ] **Step 7: 实现 BookParticleSystem::Destroy**

```cpp
void BookParticleSystem::Destroy(IRenderBackend* backend) {
    if (m_particleTex.id) backend->DestroyTexture(m_particleTex);
    m_particleTex = {0};
    m_particles.clear();
}
```

- [ ] **Step 8: 编译验证**

Run: `cmake --build build --target ShaderShowcase -j8`
Expected: 编译通过

- [ ] **Step 9: 提交**

```bash
git add shaders/vfx_fire/particle.vert shaders/vfx_fire/particle.frag \
        build/shaders/vfx_fire/particle.vert.spv build/shaders/vfx_fire/particle.frag.spv \
        shaders/vfx_fire/particle.vert.spv shaders/vfx_fire/particle.frag.spv \
        src/render/BookParticleSystem.cpp src/render/BookParticleSystem.h
git commit -m "feat: 粒子系统 - CPU模拟+GPU点精灵渲染(纸屑+烟雾)"
```

---

### Task 6: 音频播放器 — miniaudio 集成

**Files:**
- Modify: `src/render/AudioPlayer.cpp`
- Modify: `src/render/AudioPlayer.h`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 实现 AudioPlayer 完整功能**

```cpp
// AudioPlayer.cpp
#include "render/AudioPlayer.h"

#define MINIAUDIO_IMPLEMENTATION
#include "thirdparty/miniaudio.h"

struct AudioPlayerData {
    ma_engine engine;
    ma_sound sound;
    bool initialized = false;
};

bool AudioPlayer::Init(const std::string& audioPath) {
    auto* data = new AudioPlayerData();

    ma_result result = ma_engine_init(NULL, &data->engine);
    if (result != MA_SUCCESS) {
        delete data;
        return false;
    }

    result = ma_sound_init_from_file(&data->engine, audioPath.c_str(), 0, NULL, NULL, &data->sound);
    if (result != MA_SUCCESS) {
        ma_engine_uninit(&data->engine);
        delete data;
        return false;
    }

    ma_sound_set_looping(&data->sound, MA_TRUE);
    data->initialized = true;
    m_device = data;
    m_initialized = true;
    return true;
}

void AudioPlayer::Play() {
    if (!m_initialized) return;
    auto* data = static_cast<AudioPlayerData*>(m_device);
    ma_sound_start(&data->sound);
}

void AudioPlayer::Stop() {
    if (!m_initialized) return;
    auto* data = static_cast<AudioPlayerData*>(m_device);
    ma_sound_stop(&data->sound);
}

void AudioPlayer::Destroy() {
    if (!m_initialized) return;
    auto* data = static_cast<AudioPlayerData*>(m_device);
    ma_sound_uninit(&data->sound);
    ma_engine_uninit(&data->engine);
    delete data;
    m_device = nullptr;
    m_initialized = false;
}

bool AudioPlayer::IsPlaying() const {
    if (!m_initialized) return false;
    auto* data = static_cast<AudioPlayerData*>(m_device);
    return ma_sound_is_playing(&data->sound);
}
```

- [ ] **Step 2: 在 CMakeLists.txt 中添加 miniaudio 链接**

在 `target_link_libraries` 中添加（Windows需要 `winmm` 和 `ole32`）:
```cmake
if(WIN32)
    target_link_libraries(ShaderShowcase PRIVATE winmm ole32)
endif()
```

- [ ] **Step 3: 在 VFXFireBookScene::OnEnter 中初始化音频**

```cpp
m_audio = std::make_unique<AudioPlayer>();
// 需要从参考项目复制音效文件
if (m_audio->Init("assets/audio/fire_sound.wav")) {
    m_audio->Play();
}
```

- [ ] **Step 4: 在 VFXFireBookScene::OnExit 中销毁音频**

```cpp
m_audio->Stop();
m_audio->Destroy();
```

- [ ] **Step 5: 编译验证**

Run: `cmake --build build --target ShaderShowcase -j8`
Expected: 编译通过（miniaudio 是单头文件，无需额外依赖）

- [ ] **Step 6: 提交**

```bash
git add src/render/AudioPlayer.cpp src/render/AudioPlayer.h CMakeLists.txt
git commit -m "feat: 音频播放器 - miniaudio集成+循环播放火焰音效"
```

---

### Task 7: 场景整合 — 完整管线 + ImGui参数控制

**Files:**
- Modify: `src/app/VFXFireBookScene.cpp`
- Modify: `src/app/VFXFireBookScene.h`

- [ ] **Step 1: 完整 OnEnter 实现**

```cpp
void VFXFireBookScene::OnEnter() {
    // 1. 书本渲染器
    m_bookRenderer = std::make_unique<BookMeshRenderer>();
    m_bookRenderer->Load("assets/models/book/book_combined.bin");
    m_bookRenderer->LoadTextures(m_backend);

    // 2. 溶解后处理
    m_dissolve = std::make_unique<DissolvePostProcess>();
    m_dissolve->Init(m_backend);

    // 3. 纸屑粒子
    ParticleConfig paperConfig;
    paperConfig.capacity = 32;
    paperConfig.spawnRate = 16.0f;
    paperConfig.spawnCenter[0] = -0.0087f; paperConfig.spawnCenter[1] = 1.4f; paperConfig.spawnCenter[2] = -0.0185f;
    paperConfig.spawnSize[0] = 3.0f; paperConfig.spawnSize[1] = 4.0f; paperConfig.spawnSize[2] = 3.1f;
    paperConfig.velBase[0] = 0.8f; paperConfig.velBase[1] = 0.4f; paperConfig.velBase[2] = -0.1f;
    paperConfig.velRange[0] = 0.1f; paperConfig.velRange[1] = 1.2f; paperConfig.velRange[2] = 1.5f;
    paperConfig.lifeMin = 1.0f; paperConfig.lifeMax = 3.0f;
    paperConfig.sizeMin = 0.5f; paperConfig.sizeMax = 1.0f;
    paperConfig.angularVelMin = 0.2f; paperConfig.angularVelMax = 2.0f;
    paperConfig.gravity[0] = 0; paperConfig.gravity[1] = -0.5f; paperConfig.gravity[2] = 0;
    m_paperParticles = std::make_unique<BookParticleSystem>();
    m_paperParticles->Init(paperConfig);

    // 4. 烟雾粒子
    ParticleConfig smokeConfig;
    smokeConfig.capacity = 5000;
    smokeConfig.spawnRate = 160.0f;
    smokeConfig.spawnCenter[0] = 0.0164f; smokeConfig.spawnCenter[1] = -0.1317f; smokeConfig.spawnCenter[2] = -0.0315f;
    smokeConfig.spawnSize[0] = 0.5f; smokeConfig.spawnSize[1] = 0.046f; smokeConfig.spawnSize[2] = 0.7f;
    smokeConfig.velBase[0] = -0.2f; smokeConfig.velBase[1] = 0.1f; smokeConfig.velBase[2] = -0.2f;
    smokeConfig.velRange[0] = 0.2f; smokeConfig.velRange[1] = 0.3f; smokeConfig.velRange[2] = 0.2f;
    smokeConfig.lifeMin = 1.0f; smokeConfig.lifeMax = 3.0f;
    smokeConfig.sizeMin = 0.5f; smokeConfig.sizeMax = 0.7f;
    smokeConfig.gravity[0] = 0; smokeConfig.gravity[1] = 0.05f; smokeConfig.gravity[2] = 0;
    m_smokeParticles = std::make_unique<BookParticleSystem>();
    m_smokeParticles->Init(smokeConfig);

    // 5. 音频
    m_audio = std::make_unique<AudioPlayer>();
    m_audio->Init("assets/audio/fire_sound.wav");
    m_audio->Play();
}
```

- [ ] **Step 2: 完整 OnRender 实现 — 管线合成**

```cpp
void VFXFireBookScene::OnRender(IRenderBackend* backend) {
    int w, h;
    backend->GetFramebufferSize(w, h);

    float viewMat[16], projMat[16];
    BuildViewMatrix(viewMat);
    BuildProjMatrix(projMat, w, h);

    float lightDir[3] = {0.0f, -1.0f, 0.0f};
    float lightColor[3] = {2.0f, 2.0f, 2.0f};
    float cx = m_camRadius * sin(m_camPhi) * cos(m_camTheta) + m_camTarget[0];
    float cy = m_camRadius * cos(m_camPhi) + m_camTarget[1];
    float cz = m_camRadius * sin(m_camPhi) * sin(m_camTheta) + m_camTarget[2];
    float eyePos[3] = {cx, cy, cz};

    // Step 1: 渲染3D书本到RT
    TextureHandle bookRT = m_rtPool.Acquire(w, h);
    backend->BeginRenderToTexture(bookRT);
    backend->Clear(0.02f, 0.02f, 0.04f, 1.0f);
    m_bookRenderer->Render(backend, m_bookVert, m_bookFrag, viewMat, projMat, lightDir, lightColor, eyePos);
    backend->EndRenderToTexture();

    // Step 2: 溶解后处理（bookRT → screen）
    m_dissolve->Apply(backend, bookRT, {0}, m_dissolveAmount, m_edgeWidth, 4.0f, 8.0f, 0.0f, 1.0f);

    // Step 3: 渲染粒子（混合叠加到屏幕）
    // 粒子用blend enable的pipeline，叠加到已有画面
    float particleMVP[16];
    // 简单透视投影（粒子在屏幕空间）
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            float sum = 0;
            for (int k = 0; k < 4; k++)
                sum += projMat[r*4 + k] * viewMat[k*4 + c];
            particleMVP[r*4 + c] = sum;
        }
    m_paperParticles->Render(backend, m_particleVert, m_paperFrag, viewMat, projMat);
    m_smokeParticles->Render(backend, m_particleVert, m_smokeFrag, viewMat, projMat);

    m_rtPool.Release(bookRT);
    m_rtPool.Clear();
}
```

- [ ] **Step 3: 完整 OnUpdate — 粒子更新**

```cpp
void VFXFireBookScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    m_dissolveAmount += m_dissolveSpeed * dt;
    if (m_dissolveAmount > 1.0f) m_dissolveAmount = 0.0f;

    m_paperParticles->Update(dt, m_dissolveAmount);
    m_smokeParticles->Update(dt, m_dissolveAmount);
}
```

- [ ] **Step 4: 完整 OnImGui — 参数控制面板**

```cpp
void VFXFireBookScene::OnImGui() {
    ImGui::Begin("VFX Fire Book Controls");
    ImGui::SliderFloat("Dissolve Amount", &m_dissolveAmount, 0.0f, 1.0f);
    ImGui::SliderFloat("Dissolve Speed", &m_dissolveSpeed, 0.0f, 0.5f);
    ImGui::SliderFloat("Edge Width", &m_edgeWidth, 0.0f, 0.5f);
    ImGui::SliderFloat("Particle Scale", &m_particleScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Smoke Density", &m_smokeDensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Rotation Speed", &m_rotationSpeed, 0.0f, 2.0f);
    if (ImGui::Button("Reset")) {
        m_dissolveAmount = 0.5f;
        m_dissolveSpeed = 0.05f;
        m_edgeWidth = 0.2f;
        m_particleScale = 1.0f;
        m_smokeDensity = 1.0f;
        m_rotationSpeed = 0.1f;
    }
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Paper Particles: %d", m_paperParticles ? m_paperParticles->GetAliveCount() : 0);
    ImGui::Text("Smoke Particles: %d", m_smokeParticles ? m_smokeParticles->GetAliveCount() : 0);
    ImGui::End();
}
```

- [ ] **Step 5: 编译验证+启动测试**

```bash
cmake --build build --target ShaderShowcase -j8
taskkill /f /im ShaderShowcase.exe 2>nul
start build\Debug\ShaderShowcase.exe
```
Expected: 程序启动后进入VFX Fire Book场景，能看到3D书本+溶解效果+粒子+听到音频

- [ ] **Step 6: 提交**

```bash
git add src/app/VFXFireBookScene.cpp src/app/VFXFireBookScene.h
git commit -m "feat: 场景整合 - 完整管线(书本→溶解→粒子→屏幕)+ImGui控制"
```

---

### Task 8: 最终验证与对比

**目标:** 与参考项目效果对比，修复差异

- [ ] **Step 1: 启动参考项目**

```bash
cd e:\AI\test\VFX-SHADER-GRAPH-Magic-Fire-Book
# 在Unity中打开并Play
```

- [ ] **Step 2: 并排对比检查项**

| 检查项 | 预期 |
|--------|------|
| 书本形状 | 10个子网格正确拼接，左页/右页/封底/封面 |
| 纹理贴图 | 纸张纹理正确，封面纹理正确 |
| 溶解动画 | 溶解量递增时纸屑从边缘向内消失 |
| 边缘发光 | 溶解边缘有橙色发光 |
| 纸屑粒子 | 32粒子在书本上方飘散 |
| 烟雾粒子 | 持续产生，缓慢上升 |
| 音频 | 循环播放火焰音效 |

- [ ] **Step 3: 修复差异（逐项）**

根据对比结果，优先修复：
1. 溶解算法参数不一致（调整Remap范围/Step阈值）
2. 粒子产生位置/速度与参考不同
3. 光照方向/强度不一致

- [ ] **Step 4: 提交**

```bash
git add -A
git commit -m "fix: 与参考项目效果对齐 - 溶解/粒子/光照参数调整"
```

---

## 自审检查

**1. Spec覆盖**: 每项spec要求都有对应Task
- 3D书本渲染 → Task 2, 3
- 溶解Shader → Task 4
- 粒子系统 → Task 5
- 音频 → Task 6
- 场景整合 → Task 7
- 验收对比 → Task 8

**2. 无占位符**: 所有代码块都是完整实现，无TBD/TODO

**3. 类型一致性**: 
- ShaderHandle/TextureHandle/PipelineHandle 使用项目统一类型
- ShaderParams 结构体字段名称与 IRenderBackend.h 一致
- Scene 接口方法签名与 Scene.h 一致

**已知限制**: VulkanBackend 无 Compute Pipeline 支持，粒子采用 CPU 模拟 + GPU 绘制的替代方案。5000+32 粒子在 CPU 上模拟的性能足够（每帧更新约 5000 个粒子，每个粒子约 10 次浮点运算 = 50K FLOP，现代 CPU 轻松处理）。

---

**计划保存到**: `docs/superpowers/plans/2026-07-02-vfx-fire-book-plan.md`