# Awesome-Unity-Shader → ShaderShowcase 移植实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 Awesome-Unity-Shader 16 卷 67 个 Shader 移植到 ShaderShowcase，新增 62 个效果卡片 + 3D 渲染管线。

**Architecture:** 双管线共存 — `fullscreen.vert`(现有后处理) + `mesh3d.vert`(新增3D)。3D 管线新增球体/立方体几何体、方向光、旋转摄像机、160 字节 UBO(前48字节兼容现有)。纹理从项目11直接复制。

**Tech Stack:** C++17 / GLSL 460 / Vulkan / CMake / glslangValidator / Python(PPM验证)

---

## 文件结构总览

| 文件 | 操作 | 职责 |
|------|------|------|
| `shaders/common/mesh3d.vert` | 新建 | 3D 顶点着色器 — 接收 position/normal/uv, 输出 clip pos + world normal + eyeDir + lightDir |
| `shaders/common/mesh3d.vert.spv` | 编译产物 | SPIR-V |
| `src/render/VulkanBackend.h` | 修改 | 新增 `DrawMesh` 方法声明 |
| `src/render/VulkanBackend.cpp` | 修改 | 新增 `DrawMesh` 实现(vertex buffer + index buffer + 160字节 UBO) |
| `src/app/CoverFlowScene.h` | 修改 | 新增 3D 几何体成员 + MVP 矩阵 + Light 参数 |
| `src/app/CoverFlowScene.cpp` | 修改 | 新增 62 个 CARD 宏 + 几何体生成 + 3D 渲染分支 |
| `src/app/EffectDetailScene.h` | 修改 | 新增 3D 效果专用成员 |
| `src/app/EffectDetailScene.cpp` | 修改 | 新增 3D 效果全屏渲染分支 |
| `shaders/CMakeLists.txt` | 修改 | mesh3d.vert 编译规则 |
| `shaders/effects/aus_v01_first_shader/` 等 62 个 | 新建 | 每效果含 .frag + effect.json (+ CMake SPV 规则) |
| `assets/textures/matcap/` | 新建目录 | 17 张 MatCap 贴图(从项目11复制) |
| `assets/textures/ScreenWaterDrop.png` | 复制 | 水幕法线贴图 |

---

### Task 1: 3D 基础设施 — mesh3d.vert

**Files:**
- Create: `shaders/common/mesh3d.vert`

- [ ] **Step 1: 创建 3D 顶点着色器**

```glsl
#version 460
// mesh3d.vert — 3D object vertex shader for AUS port
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

layout(location=0) out vec3 vWorldPos;
layout(location=1) out vec3 vWorldNormal;
layout(location=2) out vec2 vUV;
layout(location=3) out vec3 vEyeDir;
layout(location=4) out vec3 vLightDir;

layout(std140, binding=1) uniform Params {
    float uParamFloat0, uParamFloat1, uParamFloat2, uParamFloat3, uParamFloat4, uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP;          // offset 40
    mat4 uModelView;    // offset 104
    vec3 uLightDir;     // offset 168
    float _pad0;
    vec3 uLightColor;   // offset 176
    float _pad1;
    vec3 uEyePos;       // offset 192
    float _pad2;
};

void main() {
    vec4 worldPos = vec4(aPos, 1.0);
    gl_Position = uMVP * worldPos;
    vWorldPos = aPos;
    vWorldNormal = mat3(uModelView) * aNormal;
    vUV = aUV;
    vEyeDir = normalize(uEyePos - aPos);
    vLightDir = normalize(uLightDir);
}
```

- [ ] **Step 2: 编译为 SPIR-V**

```ps1
glslangValidator -V shaders/common/mesh3d.vert -o shaders/common/mesh3d.vert.spv
# Expected: no output (success)
```

- [ ] **Step 3: 同步 SPV 到 build 目录**

```ps1
copy shaders\common\mesh3d.vert.spv build\shaders\common\mesh3d.vert.spv
```

---

### Task 2: 3D 基础设施 — DrawMesh (VulkanBackend)

**Files:**
- Modify: `src/render/VulkanBackend.h` (新增方法声明)
- Modify: `src/render/VulkanBackend.cpp` (新增实现)

- [ ] **Step 1: 在 VulkanBackend.h 中新增 DrawMesh 声明**

在 `DrawFullscreenQuad` 声明后插入：
```cpp
void DrawMesh(ShaderHandle vert, ShaderHandle frag, 
              const ShaderParams& params,
              const float* vertexData, size_t vertexCount, size_t vertexStride,
              const uint32_t* indexData, size_t indexCount);
```

- [ ] **Step 2: 在 VulkanBackend.cpp 中实现 DrawMesh**

在 `DrawFullscreenQuad` 实现后新增（约 40 行）：
```cpp
void VulkanBackend::DrawMesh(ShaderHandle vert, ShaderHandle frag,
                              const ShaderParams& params,
                              const float* vertexData, size_t vertexCount, size_t vertexStride,
                              const uint32_t* indexData, size_t indexCount)
{
    if (!m_isRecording) return;
    
    PipelineDesc desc;
    desc.vertShader = vert;
    desc.fragShader = frag;
    desc.width = params.viewportWidth;
    desc.height = params.viewportHeight;
    desc.blendEnable = false;
    
    // Create pipeline with vertex input
    PipelineHandle pipeHandle = CreatePipeline(desc);
    if (pipeHandle.id == 0) return;
    
    BindPipeline(pipeHandle);
    
    // Set viewport
    VkViewport vp{};
    vp.x = 0; vp.y = 0;
    vp.width = (float)params.viewportWidth;
    vp.height = (float)params.viewportHeight;
    vp.minDepth = 0; vp.maxDepth = 1;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &vp);
    
    VkRect2D sc{};
    sc.extent = {(uint32_t)params.viewportWidth, (uint32_t)params.viewportHeight};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &sc);
    
    // Upload vertex buffer (temporary staging via host-coherent buffer)
    size_t vbSize = vertexCount * vertexStride;
    // ... vertex buffer creation + upload + bind ...
    // Upload index buffer
    size_t ibSize = indexCount * sizeof(uint32_t);
    // ... index buffer creation + upload + bind ...
    
    // Update UBO with 208 bytes (6 floats + res+time + MVP + MV + LightDir + LightColor + EyePos)
    const size_t UBO_SIZE = 208;
    uint8_t uboData[UBO_SIZE] = {};
    for (size_t i = 0; i < std::min(params.uniformFloats.size(), size_t(6)); i++) {
        float v = params.uniformFloats[i];
        memcpy(uboData + i*4, &v, sizeof(float));
    }
    // uResolution + uTime + uFrameCount at offset 24..39
    { float r[2] = {(float)params.viewportWidth, (float)params.viewportHeight}; memcpy(uboData+24, r, 8); }
    { memcpy(uboData+32, &params.time, 4); float fc=(float)params.frameCount; memcpy(uboData+36, &fc, 4); }
    // MVP at offset 40..103 (mat4 = 16 floats * 4 bytes = 64)
    if (!params.mvp.empty()) memcpy(uboData+40, params.mvp.data(), 64);
    // ModelView at 104..167
    if (!params.modelView.empty()) memcpy(uboData+104, params.modelView.data(), 64);
    // LightDir/Color/EyePos
    if (!params.lightDir.empty()) memcpy(uboData+168, params.lightDir.data(), 12);
    if (!params.lightColor.empty()) memcpy(uboData+176, params.lightColor.data(), 12);
    if (!params.eyePos.empty()) memcpy(uboData+192, params.eyePos.data(), 12);
    
    void* mapped = nullptr;
    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end()) {
        vkMapMemory(m_device, pipeIt->second->uboMemory, 0, UBO_SIZE, 0, &mapped);
        memcpy(mapped, uboData, UBO_SIZE);
        vkUnmapMemory(m_device, pipeIt->second->uboMemory);
    }
    
    // Bind textures + UBO descriptor set
    // ... (same pattern as DrawFullscreenQuad, but with UBO_SIZE=208) ...
    
    vkCmdDrawIndexed(m_commandBuffer, (uint32_t)indexCount, 1, 0, 0, 0);
}
```

- [ ] **Step 3: 在 ShaderParams 结构体中新增 3D 字段**

在 `src/render/IRenderBackend.h` 的 `ShaderParams` 中新增：
```cpp
std::vector<float> mvp;        // mat4 MVP
std::vector<float> modelView;  // mat4 ModelView
std::vector<float> lightDir;   // vec3
std::vector<float> lightColor; // vec3
std::vector<float> eyePos;     // vec3
```

---

### Task 3: CoverFlowScene — 几何体生成 + CARD 注册

**Files:**
- Modify: `src/app/CoverFlowScene.h`
- Modify: `src/app/CoverFlowScene.cpp`

- [ ] **Step 1: CoverFlowScene.h 新增成员**

在 `CoverFlowScene` 类中新增：
```cpp
// 3D geometry
bool m_is3DEffect(int index) const;
void GenerateSphereMesh(std::vector<float>& vertices, std::vector<uint32_t>& indices);
void GenerateCubeMesh(std::vector<float>& vertices, std::vector<uint32_t>& indices);

// Per-card: which mesh to use (0=sphere, 1=cube)
std::vector<int> m_meshType;  // index by card, 0=sphere 1=cube

// 3D scene state
mat4 m_modelViewProj;
vec3 m_lightDir;
vec3 m_lightColor;
vec3 m_eyePos;
float m_cameraYaw;
float m_cameraPitch;
float m_cameraDist;
bool m_isDragging3D;
glm::vec2 m_lastMouse3D;
```

- [ ] **Step 2: 实现球体几何体生成函数**

```cpp
void CoverFlowScene::GenerateSphereMesh(std::vector<float>& vertices, std::vector<uint32_t>& indices) {
    const int latSegs = 64, lonSegs = 32;
    const float radius = 1.0f;
    vertices.reserve((latSegs+1)*(lonSegs+1)*8); // pos(3)+normal(3)+uv(2)=8 floats
    indices.reserve(latSegs*lonSegs*6);
    
    for (int j = 0; j <= lonSegs; ++j) {
        float theta = j * 3.14159265f / lonSegs;
        float sinT = sin(theta), cosT = cos(theta);
        for (int i = 0; i <= latSegs; ++i) {
            float phi = i * 2.0f * 3.14159265f / latSegs;
            float sinP = sin(phi), cosP = cos(phi);
            float nx = cosP * sinT, ny = cosT, nz = sinP * sinT;
            vertices.push_back(nx * radius); // pos
            vertices.push_back(ny * radius);
            vertices.push_back(nz * radius);
            vertices.push_back(nx);           // normal
            vertices.push_back(ny);
            vertices.push_back(nz);
            vertices.push_back((float)i / latSegs); // uv.x
            vertices.push_back((float)j / lonSegs); // uv.y
        }
    }
    for (int j = 0; j < lonSegs; ++j) {
        for (int i = 0; i < latSegs; ++i) {
            uint32_t a = j*(latSegs+1)+i, b = a+1;
            uint32_t c = (j+1)*(latSegs+1)+i, d = c+1;
            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }
    }
}
```

- [ ] **Step 3: 实现立方体几何体生成函数**

```cpp
void CoverFlowScene::GenerateCubeMesh(std::vector<float>& vertices, std::vector<uint32_t>& indices) {
    // 6 faces × 4 vertices × 8 floats = 192 floats
    float faces[6][4][8] = { /* front,back,top,bottom,right,left */
        // Front (z=1)
        {{-1,-1,1, 0,0,1, 0,0},{1,-1,1, 0,0,1, 1,0},{1,1,1, 0,0,1, 1,1},{-1,1,1, 0,0,1, 0,1}},
        // Back (z=-1)
        {{1,-1,-1, 0,0,-1, 0,0},{-1,-1,-1, 0,0,-1, 1,0},{-1,1,-1, 0,0,-1, 1,1},{1,1,-1, 0,0,-1, 0,1}},
        // Top (y=1)
        {{-1,1,1, 0,1,0, 0,0},{1,1,1, 0,1,0, 1,0},{1,1,-1, 0,1,0, 1,1},{-1,1,-1, 0,1,0, 0,1}},
        // Bottom (y=-1)
        {{-1,-1,-1, 0,-1,0, 0,0},{1,-1,-1, 0,-1,0, 1,0},{1,-1,1, 0,-1,0, 1,1},{-1,-1,1, 0,-1,0, 0,1}},
        // Right (x=1)
        {{1,-1,1, 1,0,0, 0,0},{1,-1,-1, 1,0,0, 1,0},{1,1,-1, 1,0,0, 1,1},{1,1,1, 1,0,0, 0,1}},
        // Left (x=-1)
        {{-1,-1,-1, -1,0,0, 0,0},{-1,-1,1, -1,0,0, 1,0},{-1,1,1, -1,0,0, 1,1},{-1,1,-1, -1,0,0, 0,1}},
    };
    vertices.clear(); indices.clear();
    for (int f = 0; f < 6; ++f) {
        uint32_t base = (uint32_t)vertices.size()/8;
        for (int v = 0; v < 4; ++v)
            for (int c = 0; c < 8; ++c) vertices.push_back(faces[f][v][c]);
        indices.push_back(base); indices.push_back(base+1); indices.push_back(base+2);
        indices.push_back(base); indices.push_back(base+2); indices.push_back(base+3);
    }
}
```

- [ ] **Step 4: RegisterCards 中新增 62 个 CARD 宏**

在现有 CARD 宏列表末尾（`"xpl_sharpen_v3"` 之后）追加：

```cpp
// ===== Phase 1: 后处理屏幕特效 (5个) =====
CARD("aus_v08_motion_blur",    "径向模糊",     "AUS 后处理",
     "Vol.08 径向模糊屏幕特效 — 中心→边缘采样密度递增",
     "effects/aus_v08_motion_blur/aus_v08_motion_blur.frag.spv");

CARD("aus_v09_water_drop",     "水幕特效",     "AUS 后处理",
     "Vol.09 屏幕水幕特效 — 法线贴图uv偏移 + 折射模拟",
     "effects/aus_v09_water_drop/aus_v09_water_drop.frag.spv");

CARD("aus_v10_oil_paint",      "油画特效",     "AUS 后处理",
     "Vol.10 屏幕油画特效 — Kuwahara 滤波网格采样",
     "effects/aus_v10_oil_paint/aus_v10_oil_paint.frag.spv");

CARD("aus_v15_gaussian_blur",  "高斯模糊",     "AUS 后处理",
     "Vol.15 屏幕高斯模糊 — 单Pass水平+垂直混合采样",
     "effects/aus_v15_gaussian_blur/aus_v15_gaussian_blur.frag.spv");

// ===== Phase 2: Vol.01-04 3D物体着色器 (19个) =====
CARD("aus_v01_rim_bump",       "凹凸+边缘光",   "AUS 3D物体",
     "Vol.01 凹凸纹理显示+自选边缘颜色和强度",
     "effects/aus_v01_rim_bump/aus_v01_rim_bump.frag.spv", 1);

CARD("aus_v02_solid_color",    "基础单色",     "AUS 3D物体",
     "Vol.02 基础单色Shader",
     "effects/aus_v02_solid_color/aus_v02_solid_color.frag.spv", 1);

CARD("aus_v02_simple_light",   "材质+光照",     "AUS 3D物体",
     "Vol.02 材质颜色设置+开启光照",
     "effects/aus_v02_simple_light/aus_v02_simple_light.frag.spv", 1);

CARD("aus_v02_lambert",        "可调漫反射",    "AUS 3D物体",
     "Vol.02 简单的可调漫反射光照(Lambert)",
     "effects/aus_v02_lambert/aus_v02_lambert.frag.spv", 1);

CARD("aus_v02_light_full_beta","完备光照Beta",  "AUS 3D物体",
     "Vol.02 光照材质完备beta版",
     "effects/aus_v02_light_full_beta/aus_v02_light_full_beta.frag.spv", 1);

CARD("aus_v02_texture_load",   "纹理载入",     "AUS 3D物体",
     "Vol.02 简单的纹理载入Shader",
     "effects/aus_v02_texture_load/aus_v02_texture_load.frag.spv", 1);

CARD("aus_v02_light_full",     "完备光照正式",  "AUS 3D物体",
     "Vol.02 光照材质完备正式版(纹理+漫反射+镜面)",
     "effects/aus_v02_light_full/aus_v02_light_full.frag.spv", 1);

CARD("aus_v03_alpha_blend",    "Alpha纹理混合", "AUS 3D物体",
     "Vol.03 Alpha纹理混合",
     "effects/aus_v03_alpha_blend/aus_v03_alpha_blend.frag.spv", 1);

CARD("aus_v03_alpha_emissive", "Alpha+自发光",  "AUS 3D物体",
     "Vol.03 纹理Alpha与自发光混合",
     "effects/aus_v03_alpha_emissive/aus_v03_alpha_emissive.frag.spv", 1);

CARD("aus_v03_alpha_emiss_tint","可调色混合",   "AUS 3D物体",
     "Vol.03 纹理Alpha与自发光混合可调色版",
     "effects/aus_v03_alpha_emiss_tint/aus_v03_alpha_emiss_tint.frag.spv", 1);

CARD("aus_v03_vertex_alpha",   "顶点光照+Alpha", "AUS 3D物体",
     "Vol.03 顶点光照+纹理Alpha自发光混合",
     "effects/aus_v03_vertex_alpha/aus_v03_vertex_alpha.frag.spv", 1);

CARD("aus_v03_vertex_emiss",   "顶点光照+自发光", "AUS 3D物体",
     "Vol.03 顶点光照+自发光混合+纹理混合",
     "effects/aus_v03_vertex_emiss/aus_v03_vertex_emiss.frag.spv", 1);

CARD("aus_v04_cull_backface",  "剔除背面",     "AUS 3D物体",
     "Vol.04 用剔除操作渲染对象背面",
     "effects/aus_v04_cull_backface/aus_v04_cull_backface.frag.spv", 1);

CARD("aus_v04_cull_glass",     "剔除玻璃效果",  "AUS 3D物体",
     "Vol.04 用剔除实现玻璃效果",
     "effects/aus_v04_cull_glass/aus_v04_cull_glass.frag.spv", 1);

CARD("aus_v04_alpha_test",     "Alpha测试",   "AUS 3D物体",
     "Vol.04 基本Alpha测试",
     "effects/aus_v04_alpha_test/aus_v04_alpha_test.frag.spv", 1);

CARD("aus_v04_transparent",    "顶点光+透明",  "AUS 3D物体",
     "Vol.04 顶点光照+可调透明度",
     "effects/aus_v04_transparent/aus_v04_transparent.frag.spv", 1);

CARD("aus_v04_vegetation",     "植被Shader",   "AUS 3D物体",
     "Vol.04 简单的植被Shader(Alpha裁切+双面)",
     "effects/aus_v04_vegetation/aus_v04_vegetation.frag.spv", 1);

CARD("aus_v04_fog",            "基本雾效",     "AUS 3D物体",
     "Vol.04 基本雾效 — 远处衰减到雾色",
     "effects/aus_v04_fog/aus_v04_fog.frag.spv", 1);

CARD("aus_v05_texture_blend",  "Blend纹理载入", "AUS 3D物体",
     "Vol.05 混合操作纹理载入",
     "effects/aus_v05_texture_blend/aus_v05_texture_blend.frag.spv", 1);
```

注意：CARD 宏需扩展以支持 mesh 类型。修改 CARD 宏定义：
```cpp
#define CARD(id, name, cat, desc, frag, mesh) \
    add(id, name, cat, desc, frag, mesh)

// Backward compat: old cards use mesh=0 (fullscreen quad)
#define CARD_2D(id, name, cat, desc, frag) \
    CARD(id, name, cat, desc, frag, -1)
```

- [ ] **Step 5: Modify add() lambda to handle meshType**

```cpp
auto add = [&](const char* id, const char* name, const char* category,
               const char* desc, const char* fragRelPath, int meshType) {
    EffectCard c;
    c.id = id; c.name = name; c.category = category; c.description = desc;
    c.vertSpirvPath = (meshType >= 0) ? (shaderDir + "/common/mesh3d.vert.spv") : vertPath;
    c.fragSpirvPath = shaderDir + "/" + fragRelPath;
    c.passes = 1;
    m_cards.push_back(std::move(c));
    m_meshType.push_back(meshType);
};
```

所有现有 CARD 调用改为 `add(id, name, cat, desc, frag, -1)` 即保持 fullscreen.vert。

- [ ] **Step 6: 修改 render logic — 区分 2D/3D**

在 `CoverFlowScene::Render()` 中，选择效果时：
```cpp
if (m_meshType[m_selectedIndex] >= 0) {
    // 3D effect — draw mesh
    backend->DrawMesh(m_mesh3dVertShader, state.fragShader, params,
                      m_sphereVertices.data(), m_sphereVertices.size()/8, 8,
                      m_sphereIndices.data(), m_sphereIndices.size());
} else {
    // 2D post-processing — draw fullscreen quad
    backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
}
```

---

### Task 4: 纹理资源复制

**Files:**
- Create: `assets/textures/matcap/` (目录)

- [ ] **Step 1: 复制 MatCap 纹理**

```ps1
mkdir assets\textures\matcap -Force
copy 'e:\AI\test\Awesome-Unity-Shader\Volume 16 基于MatCap实现高真实感车漆Shader\MatCap Textrues\*.jpg' assets\textures\matcap\
copy 'e:\AI\test\Awesome-Unity-Shader\Volume 16 基于MatCap实现高真实感车漆Shader\MatCap Textrues\*.png' assets\textures\matcap\
```

- [ ] **Step 2: 复制水幕法线贴图**

```ps1
copy 'e:\AI\test\Awesome-Unity-Shader\Volume 09 屏幕水幕特效Shader&Standard Shader\ScreenWaterDropEffect\Resources\ScreenWaterDrop.png' assets\textures\
```

---

### Task 5: Phase 1 后处理特效 — 2个shader (径向模糊 + 水幕 + 油画 + 高斯模糊)

每个效果独立为子任务。

#### Task 5a: 径向模糊 MotionBlur

**Files:**
- Create: `shaders/effects/aus_v08_motion_blur/aus_v08_motion_blur.frag`
- Create: `shaders/effects/aus_v08_motion_blur/effect.json`

- [ ] **Step 1: 编写径向模糊 shader**

```glsl
#version 460
// Radial Motion Blur — from Awesome-Unity-Shader Vol.08
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    int samples = int(clamp(uParamFloat0, 2, 20));
    float strength = clamp(uParamFloat1, 0.01, 0.2);
    vec2 center = vec2(0.5 + uParamFloat2 * 0.2, 0.5 + uParamFloat3 * 0.2);
    vec4 sum = vec4(0);
    for (int i = 0; i < samples; i++) {
        float t = float(i) / float(samples-1);
        vec2 uv = mix(vUV, center, t * strength);
        sum += texture(uInputTex, uv);
    }
    outColor = sum / float(samples);
}
```

- [ ] **Step 2: 编写 effect.json**

```json
{
  "name": "径向模糊",
  "category": "AUS 后处理",
  "description": "Vol.08 径向模糊屏幕特效",
  "passes": 1,
  "params": [
    {"name":"uParamFloat0","label":"采样数","type":"Float","min":2,"max":20,"default":8,"ui_type":"slider"},
    {"name":"uParamFloat1","label":"强度","type":"Float","min":0.01,"max":0.2,"default":0.08,"ui_type":"slider"},
    {"name":"uParamFloat2","label":"中心X偏移","type":"Float","min":-1,"max":1,"default":0,"ui_type":"slider"},
    {"name":"uParamFloat3","label":"中心Y偏移","type":"Float","min":-1,"max":1,"default":0,"ui_type":"slider"}
  ]
}
```

- [ ] **Step 3: 编译 + 同步**

```ps1
glslangValidator -V shaders\effects\aus_v08_motion_blur\aus_v08_motion_blur.frag -o shaders\effects\aus_v08_motion_blur\aus_v08_motion_blur.frag.spv
copy shaders\effects\aus_v08_motion_blur\aus_v08_motion_blur.frag.spv build\shaders\effects\aus_v08_motion_blur\aus_v08_motion_blur.frag.spv
```

#### Task 5b: 水幕特效 WaterDrop

**Files:**
- Create: `shaders/effects/aus_v09_water_drop/aus_v09_water_drop.frag`
- Create: `shaders/effects/aus_v09_water_drop/effect.json`

- [ ] **Step 1: 编写水幕特效 shader**

```glsl
#version 460
// Screen Water Drop — from Awesome-Unity-Shader Vol.09
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(binding=2) uniform sampler2D uTex1;  // water normal map
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    float scale = clamp(uParamFloat0, 0.1, 5.0);
    float speed = clamp(uParamFloat1, 0, 2.0);
    float distort = clamp(uParamFloat2, 0, 0.2);
    vec2 uv = vUV + vec2(sin(vUV.y*50.0+uTime*speed), cos(vUV.x*50.0+uTime*speed))*0.002;
    vec4 normalTex = texture(uTex1, uv * scale + uTime * speed * 0.1);
    vec2 offset = (normalTex.rg - 0.5) * distort;
    outColor = texture(uInputTex, vUV + offset);
    outColor = mix(outColor, vec4(0.7,0.85,1.0,1.0), normalTex.b * 0.3);
}
```

- [ ] **Step 2: 编写 effect.json**

```json
{
  "name": "水幕特效",
  "category": "AUS 后处理",
  "description": "Vol.09 屏幕水幕特效 — 法线贴图uv偏移+折射模拟",
  "passes": 1,
  "params": [
    {"name":"uParamFloat0","label":"缩放","type":"Float","min":0.1,"max":5,"default":2,"ui_type":"slider"},
    {"name":"uParamFloat1","label":"速度","type":"Float","min":0,"max":2,"default":1,"ui_type":"slider"},
    {"name":"uParamFloat2","label":"扭曲强度","type":"Float","min":0,"max":0.2,"default":0.05,"ui_type":"slider"}
  ],
  "textures": [
    {"binding":2,"name":"uTex1","path":"assets/textures/ScreenWaterDrop.png","label":"水纹法线"}
  ]
}
```

- [ ] **Step 3: 编译 + 同步**

```ps1
glslangValidator -V shaders\effects\aus_v09_water_drop\aus_v09_water_drop.frag -o shaders\effects\aus_v09_water_drop\aus_v09_water_drop.frag.spv
copy shaders\effects\aus_v09_water_drop\aus_v09_water_drop.frag.spv build\shaders\effects\aus_v09_water_drop\aus_v09_water_drop.frag.spv
```

#### Task 5c: 油画特效 OilPaint

**Files:**
- Create: `shaders/effects/aus_v10_oil_paint/aus_v10_oil_paint.frag`
- Create: `shaders/effects/aus_v10_oil_paint/effect.json`

- [ ] **Step 1: 编写油画特效 shader**

```glsl
#version 460
// Oil Paint (Kuwahara) — from Awesome-Unity-Shader Vol.10
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    float radius = max(uParamFloat0, 1.0);
    vec2 step = vec2(radius) / uResolution;
    vec4 bestColor = vec4(0); float bestVar = 1e10;
    // Four sub-regions (Kuwahara)
    for (int q = 0; q < 4; q++) {
        vec2 offSign = vec2((q%2==0)?-1.0:1.0, (q/2==0)?-1.0:1.0);
        vec3 sum = vec3(0), sumSq = vec3(0); float cnt = 0;
        for (int dy = 0; dy < int(radius); dy++) {
            for (int dx = 0; dx < int(radius); dx++) {
                vec2 uv = vUV + vec2(dx*offSign.x, dy*offSign.y)*step;
                vec3 col = texture(uInputTex, uv).rgb;
                sum += col; sumSq += col*col; cnt += 1.0;
            }
        }
        vec3 mean = sum / cnt;
        float variance = dot(sumSq/cnt - mean*mean, vec3(1));
        if (variance < bestVar) { bestVar = variance; bestColor = vec4(mean, 1.0); }
    }
    outColor = bestColor;
}
```

- [ ] **Step 2: 编译 + 同步**

```
effect.json 同上格式，uParamFloat0=半径(1~16, default=4)
```

#### Task 5d: 高斯模糊 GaussianBlur

- [ ] **Step 1: 编写单Pass高斯模糊 shader (13-tap)**

```glsl
#version 460
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    float blur = clamp(uParamFloat0, 0.5, 20.0);
    vec2 step = vec2(1.0/uResolution.x, 1.0/uResolution.y);
    vec4 col = vec4(0);
    float weights[7] = float[](0.196, 0.175, 0.132, 0.077, 0.035, 0.012, 0.003);
    for (int i = -6; i <= 6; i++) {
        float w = weights[abs(i)];
        col += texture(uInputTex, vUV + vec2(i*step.x*blur, 0)) * w;
    }
    vec4 colH = col;
    col = vec4(0);
    for (int i = -6; i <= 6; i++) {
        float w = weights[abs(i)];
        col += texture(uInputTex, vUV + vec2(0, i*step.y*blur)) * w;
    }
    outColor = (colH + col) * 0.5;
}
```

---

### Task 6: Phase 2 — Vol.01-04 3D物体着色器 (19个)

每个效果文件结构：
```
shaders/effects/aus_v01_rim_bump/
├── aus_v01_rim_bump.frag
├── aus_v01_rim_bump.frag.spv
└── effect.json
```

核心转换规则：
1. 读原 `.shader` 的 `frag` 函数/片段着色器
2. `_MainTex` → `uInputTex` (binding=0)
3. `_BumpMap` → `uTex1` (binding=2)
4. `_WorldSpaceLightPos0` → `uLightDir` (来自 mesh3d.vert 的 vLightDir)
5. `_LightColor0` → `uLightColor` (来自 mesh3d.vert 的 vLightColor)
6. `_WorldSpaceCameraPos` → `uEyePos` (来自 mesh3d.vert 的 vEyeDir)
7. `UNITY_MATRIX_MVP` → `uMVP`, `UNITY_MATRIX_MV` → `uModelView`
8. Pass Tags (Cull/Blend/ZWrite/ZTest) → 不同效果用不同 PipelineDesc

#### 3D效果子任务规划 (分6批，每批3-4个)

**Batch 1 (Vol.01-02前半):**
- aus_v01_rim_bump — 凹凸+边缘光 (球体+纹理)
- aus_v02_solid_color — 基础单色 (球体, 无纹理)
- aus_v02_simple_light — 材质+光照 (球体)
- aus_v02_lambert — Lambert漫反射 (球体)

**Batch 2 (Vol.02后半+Vol.03前半):**
- aus_v02_light_full_beta — 完备光照Beta (球体+纹理)
- aus_v02_texture_load — 纹理载入 (球体+纹理)
- aus_v02_light_full — 完备光照正式版 (球体+纹理+镜面)
- aus_v03_alpha_blend — Alpha纹理混合 (立方体)

**Batch 3 (Vol.03后半+Vol.04前半):**
- aus_v03_alpha_emissive — Alpha+自发光 (立方体)
- aus_v03_alpha_emiss_tint — 可调色混合 (立方体)
- aus_v03_vertex_alpha — 顶点光照+Alpha (立方体)
- aus_v03_vertex_emiss — 顶点光照+自发光 (立方体)

**Batch 4 (Vol.04后半):**
- aus_v04_cull_backface — 剔除背面 (立方体)
- aus_v04_cull_glass — 剔除玻璃 (立方体)
- aus_v04_alpha_test — Alpha测试 (立方体, Alpha裁切)
- aus_v04_transparent — 透明 (立方体, Blend)
- aus_v04_vegetation — 植被 (立方体, Alpha裁切+双面)
- aus_v04_fog — 雾效 (球体, 距离衰减)

每个 3D 效果 shader 的关键接口 (mesh3d.vert 提供):
```glsl
layout(location=0) in vec3 vWorldPos;
layout(location=1) in vec3 vWorldNormal;
layout(location=2) in vec2 vUV;
layout(location=3) in vec3 vEyeDir;
layout(location=4) in vec3 vLightDir;
```

示例 — aus_v02_lambert.frag:
```glsl
#version 460
layout(location=0) in vec3 vWorldPos;
layout(location=1) in vec3 vWorldNormal;
layout(location=2) in vec2 vUV;
layout(location=3) in vec3 vEyeDir;
layout(location=4) in vec3 vLightDir;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
    mat4 uMVP; mat4 uModelView;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(vLightDir);
    float NdotL = max(dot(N, L), 0.0);
    vec4 texColor = texture(uInputTex, vUV);
    vec3 diffuse = texColor.rgb * uLightColor * NdotL * uParamFloat0;
    outColor = vec4(diffuse, texColor.a);
}
```

---

### Task 7: Phase 3 — Vol.05-07 (15个)

按 Vol.01-04 相同模式移植。每个 3D 效果 = `.frag` + `effect.json`。

**每个效果的 effect.json 模板:**
```json
{
  "name": "效果中文名",
  "category": "AUS 3D物体",
  "description": "描述文字",
  "passes": 1,
  "vertexShader": "common/mesh3d.vert.spv",
  "params": [
    {"name":"uParamFloat0","label":"参数名","type":"Float","min":...,"max":...,"default":...,"ui_type":"slider"}
  ]
}
```

---

### Task 8: Phase 4-6 — Vol.12-14,16 (15个)

同上模式批量移植。Vol.16 MatCap 特殊处理 — 需要额外纹理绑定 (uTex1=MatCap贴图)。

---

### Task 9: 自动化验证扩展

**Files:**
- Modify: `c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\deep_compare.py` (复用)
- Create: `c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\verify_3d.py`

- [ ] **Step 1: 扩展 deep_compare.py 扫描 153 个效果**

```python
# 将 range(91) 改为 range(153)
# 3D 效果额外校验: 颜色不能全是原图2D的采样结果
for idx in range(153):
    # ... existing logic ...
    if idx >= 91:  # 3D effects must differ from 2D reference
        ref_fp = os.path.join(shot, 'card_01.ppm')  # Bloom as 2D baseline
        # Verify 3D card has < 50% overlap with 2D reference
```

- [ ] **Step 2: create verify_3d.py — 快速单个 3D 效果编译+渲染检查**

```python
import os, subprocess
effects_3d = [eid for eid in os.listdir(src) if eid.startswith('aus_v')]
for eid in effects_3d:
    frag = os.path.join(src, eid, f'{eid}.frag')
    spv = frag + '.spv'
    r = subprocess.run(['glslangValidator', '-V', frag, '-o', spv], capture_output=True)
    if r.returncode != 0:
        print(f'FAIL: {eid} — {r.stderr.decode()}')
    else:
        print(f'OK: {eid}')
```

---

### Task 10: Build + Push

- [ ] **Step 1: CMakeLists 中添加 mesh3d.vert 编译规则**

```cmake
# shaders/CMakeLists.txt — add:
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../common/mesh3d.vert.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/common/mesh3d.vert -o ${CMAKE_CURRENT_BINARY_DIR}/../common/mesh3d.vert.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/common/mesh3d.vert
)
```

- [ ] **Step 2: CMake 重建整个项目**

```ps1
cmake --build build --config Release
# Expected: 0 errors
```

- [ ] **Step 3: AUTO_TEST_CARDS 全量截图 (153 张)**

```ps1
$env:AUTO_TEST_CARDS = '1'
.\build\bin\Release\ShaderShowcase.exe
# Wait 120s, verify 153 PPM files exist
```

- [ ] **Step 4: Python 统计验证**

```ps1
python verify_all_153.py
# Expected: 153 OK, 0 FAIL
```

- [ ] **Step 5: Git commit + push**

```ps1
git add -A ; git commit -m "feat: Awesome-Unity-Shader移植 — +62效果 +3D管线" ; git push
```

---

### 总体进度追踪

| Phase | 内容 | 效果数 | 状态 |
|-------|------|--------|------|
| Task 1-4 | 基础设施 (mesh3d.vert, DrawMesh, 纹理, CARD注册) | — | pending |
| Task 5 | 后处理5个 | 5 | pending |
| Task 6 | Vol.01-04 3D | 19 | pending |
| Task 7 | Vol.05-07 3D | 15 | pending |
| Task 8 | Vol.12-14,16 3D | 15 | pending |
| Task 9-10 | 验证+构建+提交 | — | pending |
| **Total** | | **54新增 (扣8重复/模板)** | |