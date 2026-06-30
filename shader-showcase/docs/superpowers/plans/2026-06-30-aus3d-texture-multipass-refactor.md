# AUS3D 纹理+多Pass重构 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 引入纹理系统和多Pass渲染能力，重构14个AUS3D效果使其接近参考项目的Unity ShaderLab实现方式

**Architecture:** 分三个阶段实施。阶段1扩展VulkanBackend（RT已存在BeginRenderToTexture/EndRenderToTexture，需新增纹理文件加载、辅助纹理绑定、管线缓存键修正）；阶段2改造AUS3DScene（Pass序列、纹理管理器、渲染循环）；阶段3逐个迁移14个着色器（5个后处理型+9个球体纹理型）

**Tech Stack:** C++17, Vulkan 1.3, GLSL 460, stb_image, glslangValidator

**预检：** 现有 `BeginRenderToTexture`/`EndRenderToTexture` 已实现FBO渲染，`CreateRenderPassForFormat` 已实现RT格式管理，`VulkanTexture` 已有 `framebuffer`/`renderPass` 字段。核心缺口：管线缓存键不含renderPass、无辅助纹理binding、无文件纹理加载。

---

### Task 1: 管线缓存键加入renderPass — 修复多RT格式支持

**Files:**
- Modify: `src/render/VulkanBackend.cpp:1280-1283`

**问题：** 当前 `CreatePipeline` 缓存键仅用 `(vertShader << 32 | fragShader)` + blendEnable/useVertexInput 的XOR。当同一shader用于不同RT格式（如swapchain RGBA8 和 FBO RGBA8）时，缓存键相同但renderPass不兼容，导致Vulkan验证层报错。

**修复：** 将 `renderPass` 指针地址加入缓存键。

- [ ] **Step 1: 修改缓存键计算**

```cpp
// Replace lines 1280-1283 in VulkanBackend.cpp:
// OLD:
// uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
// cacheKey ^= (desc.useVertexInput ? (1ULL << 60) : 0);
// cacheKey ^= (desc.blendEnable ? (1ULL << 59) : 0);

// NEW:
uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
cacheKey ^= (desc.useVertexInput ? (1ULL << 60) : 0);
cacheKey ^= (desc.blendEnable ? (1ULL << 59) : 0);
// Include renderPass in cache key since different render passes have different formats
cacheKey ^= (uint64_t(renderPass) >> 3);  // pointer as hash (shift to drop alignment bits)
```

- [ ] **Step 2: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

Expected: 编译成功，无错误

- [ ] **Step 3: 运行现有测试确认无回归**

```powershell
taskkill /f /im ShaderShowcase.exe 2>$null
$env:AUTO_TEST_AUS3D='1'
cd e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release
.\ShaderShowcase.exe
```

Expected: 47个效果全部正常截图，无Vulkan验证层错误

- [ ] **Step 4: Commit**

```bash
git add src/render/VulkanBackend.cpp
git commit -m "fix: 管线缓存键加入renderPass以支持多RT格式"
```

---

### Task 2: 新增CreateTextureFromFile — 从PNG/JPG加载纹理

**Files:**
- Modify: `src/render/IRenderBackend.h` (新增虚函数声明)
- Modify: `src/render/VulkanBackend.h` (新增方法声明)
- Modify: `src/render/VulkanBackend.cpp` (实现)

**依赖：** stb_image.h (已有，项目已使用)

- [ ] **Step 1: 在IRenderBackend.h添加虚函数声明**

在 `CreateTexture` 声明后添加：

```cpp
// 从文件加载纹理 (PNG/JPG/BMP等)
virtual TextureHandle CreateTextureFromFile(const std::string& path) = 0;
// 从内存像素数据创建纹理 (RGBA8)
virtual TextureHandle CreateTextureFromData(int width, int height, const uint8_t* rgbaData) = 0;
```

- [ ] **Step 2: 在VulkanBackend.h添加声明**

```cpp
TextureHandle CreateTextureFromFile(const std::string& path) override;
TextureHandle CreateTextureFromData(int width, int height, const uint8_t* rgbaData) override;
```

- [ ] **Step 3: 在VulkanBackend.cpp实现CreateTextureFromFile**

在 `CreateTexture` 函数之后添加：

```cpp
TextureHandle VulkanBackend::CreateTextureFromFile(const std::string& path) {
    int width, height, channels;
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!pixels) {
        fprintf(stderr, "[Vulkan] Failed to load texture: %s\n", path.c_str());
        return {0};
    }
    
    TextureHandle handle = CreateTexture(width, height, TextureFormat::RGBA8, pixels);
    stbi_image_free(pixels);
    
    if (handle.id != 0) {
        fprintf(stdout, "[Vulkan] Loaded texture from file: %s (%dx%d)\n", path.c_str(), width, height);
    }
    return handle;
}

TextureHandle VulkanBackend::CreateTextureFromData(int width, int height, const uint8_t* rgbaData) {
    return CreateTexture(width, height, TextureFormat::RGBA8, rgbaData);
}
```

- [ ] **Step 4: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

Expected: 编译成功

- [ ] **Step 5: Commit**

```bash
git add src/render/IRenderBackend.h src/render/VulkanBackend.h src/render/VulkanBackend.cpp
git commit -m "feat: 新增CreateTextureFromFile和CreateTextureFromData接口"
```

---

### Task 3: 新增辅助纹理绑定 (binding=2) — 支持uAuxTex

**Files:**
- Modify: `src/render/VulkanBackend.cpp` — `CreateDescriptorSetLayout` 和 `DrawFullscreenQuad` 描述符更新部分

**当前：** 描述符布局只有 binding=0 (uInputTex) 和 binding=1 (UBO)。ShaderParams 已有 `inputTextures` 向量，需要扩展支持第二个纹理。

- [ ] **Step 1: 扩展ShaderParams添加auxTextures**

先检查 `ShaderParams` 定义位置：

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase
rg "struct ShaderParams" src/
```

- [ ] **Step 2: 修改描述符布局添加binding=2**

在 `CreateDescriptorSetLayout` 中，在 binding=1 之后添加：

```cpp
// binding=2: uAuxTex (辅助纹理，如水滴图、Ramp纹理)
VkDescriptorSetLayoutBinding auxTexBinding{};
auxTexBinding.binding = 2;
auxTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
auxTexBinding.descriptorCount = 1;
auxTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
auxTexBinding.pImmutableSamplers = nullptr;

VkDescriptorSetLayoutBinding bindings[3] = { inputTexBinding, uboBinding, auxTexBinding };
layoutInfo.bindingCount = 3;
layoutInfo.pBindings = bindings;
```

- [ ] **Step 3: 修改DrawFullscreenQuad描述符更新**

在现有的 texture(0) + UBO(1) 写入之后，添加 binding=2 的写入：

```cpp
// binding=2: auxiliary texture
if (params.auxTextures.size() >= 1) {
    auto auxIt = m_textures.find(params.auxTextures[0].id);
    if (auxIt != m_textures.end()) {
        VkDescriptorImageInfo auxImageInfo{};
        auxImageInfo.sampler = auxIt->second->sampler;
        auxImageInfo.imageView = auxIt->second->view;
        auxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet auxWrite{};
        auxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        auxWrite.dstSet = pipeIt->second->descSet;
        auxWrite.dstBinding = 2;
        auxWrite.dstArrayElement = 0;
        auxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        auxWrite.descriptorCount = 1;
        auxWrite.pImageInfo = &auxImageInfo;
        writes.push_back(auxWrite);
    }
}
```

- [ ] **Step 4: 在ShaderParams中添加auxTextures字段**

找到 `ShaderParams` 结构体（可能在 `src/render/BackendType.h` 或 `IRenderBackend.h`），添加：

```cpp
std::vector<TextureHandle> auxTextures;  // binding=2: auxiliary textures
```

- [ ] **Step 5: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

- [ ] **Step 6: Commit**

```bash
git add src/render/VulkanBackend.cpp src/render/BackendType.h
git commit -m "feat: 新增binding=2辅助纹理支持(uAuxTex)"
```

---

### Task 4: DrawToScreen — 简化带纹理输入的屏幕渲染

**Files:**
- Modify: `src/render/IRenderBackend.h` (新增声明)
- Modify: `src/render/VulkanBackend.h` (新增声明)
- Modify: `src/render/VulkanBackend.cpp` (实现)

**目的：** 对多Pass效果，最后一个Pass需要将中间RT作为`uInputTex`渲染到屏幕。当前 `DrawFullscreenQuad` 已经支持 `inputTextures`，但需要在 swapchain render pass 中渲染。简化调用接口。

- [ ] **Step 1: 在IRenderBackend.h添加声明**

```cpp
// 渲染到屏幕：将inputTex作为uInputTex采样，全屏输出
virtual void DrawToScreen(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params, TextureHandle inputTex) = 0;
```

- [ ] **Step 2: 在VulkanBackend.h添加声明**

```cpp
void DrawToScreen(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params, TextureHandle inputTex) override;
```

- [ ] **Step 3: 在VulkanBackend.cpp实现**

```cpp
void VulkanBackend::DrawToScreen(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params, TextureHandle inputTex) {
    if (!m_isRecording) return;
    
    // Ensure we're rendering to swapchain (not FBO)
    if (m_isRenderToTexture) {
        EndRenderToTexture();
    }
    
    ShaderParams screenParams = params;
    screenParams.inputTextures = {inputTex};
    DrawFullscreenQuad(vert, frag, screenParams);
}
```

- [ ] **Step 4: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

- [ ] **Step 5: Commit**

```bash
git add src/render/IRenderBackend.h src/render/VulkanBackend.h src/render/VulkanBackend.cpp
git commit -m "feat: 新增DrawToScreen简化接口"
```

---

### Task 5: AUS3DScene — Pass序列和纹理管理器

**Files:**
- Modify: `src/app/AUS3DScene.h` (新增结构体和成员)
- Modify: `src/app/AUS3DScene.cpp` (新增TextureManager、改造BuildEffects、改造OnRender)

这是最大的改动任务。分步实施：

- [ ] **Step 1: 在AUS3DScene.h添加AUS3DPass结构体**

```cpp
struct AUS3DPass {
    std::string fragShader;   // SPIR-V路径
    int targetWidth = 0;      // 0=全屏, N=降采样宽度
    int targetHeight = 0;     // 0=全屏, N=降采样高度
    bool isOutput = false;    // 最后一个Pass输出到屏幕
};
```

- [ ] **Step 2: 扩展AUS3DEffect结构体**

```cpp
struct AUS3DEffect {
    // ... 现有字段保持不变 ...
    std::vector<AUS3DPass> passes;        // 新增: Pass序列
    std::vector<std::string> auxTextures; // 新增: 辅助纹理路径
    bool use3DGeometry = true;            // 新增: 是否保留光追球体
};
```

- [ ] **Step 3: 添加TextureManager类**

```cpp
class TextureManager {
public:
    TextureManager(IRenderBackend* backend) : m_backend(backend) {}
    
    TextureHandle LoadTexture(const std::string& path) {
        auto it = m_cache.find(path);
        if (it != m_cache.end()) return it->second;
        TextureHandle tex = m_backend->CreateTextureFromFile(path);
        m_cache[path] = tex;
        return tex;
    }
    
    TextureHandle GenerateRampTexture(int bands) {
        std::string key = "ramp_" + std::to_string(bands);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;
        // 生成色阶纹理: 256x1 RGBA
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
        m_cache[key] = tex;
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
        m_cache[key] = tex;
        return tex;
    }
    
private:
    IRenderBackend* m_backend;
    std::map<std::string, TextureHandle> m_cache;
};
```

- [ ] **Step 4: 改造OnRender支持多Pass**

```cpp
void AUS3DScene::OnRender() {
    auto& fx = m_effects[m_currentIndex];
    // ... existing camera/params setup ...
    
    if (fx.passes.empty()) {
        // 旧路径：单Pass光追球体
        m_backend->DrawFullscreenQuad(m_sharedVert, fx.fragShader, params);
    } else {
        // 新路径：多Pass序列
        TextureHandle prevRT = {0};
        for (size_t i = 0; i < fx.passes.size(); i++) {
            auto& pass = fx.passes[i];
            ShaderHandle passShader = LoadShader(pass.fragShader);
            
            ShaderParams passParams = params;
            passParams.viewportWidth = pass.targetWidth > 0 ? pass.targetWidth : params.viewportWidth;
            passParams.viewportHeight = pass.targetHeight > 0 ? pass.targetHeight : params.viewportHeight;
            
            if (pass.isOutput) {
                // 最终Pass: 渲染到屏幕
                passParams.inputTextures = prevRT.id ? std::vector<TextureHandle>{prevRT} : std::vector<TextureHandle>{};
                m_backend->DrawToScreen(m_sharedVert, passShader, passParams, prevRT.id ? prevRT : TextureHandle{0});
            } else {
                // 中间Pass: 渲染到RT
                m_backend->BeginRenderToTexture(/* target RT */);
                passParams.inputTextures = prevRT.id ? std::vector<TextureHandle>{prevRT} : std::vector<TextureHandle>{};
                m_backend->DrawFullscreenQuad(m_sharedVert, passShader, passParams);
                m_backend->EndRenderToTexture();
                // prevRT = 当前RT
            }
        }
    }
}
```

> **注意：** Step 4的RT管理需要创建目标RT。当前 `BeginRenderToTexture` 需要已有的 `TextureHandle`。需要在AUS3DScene中维护RT池。具体实现细节见Task 6。

- [ ] **Step 5: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

- [ ] **Step 6: Commit**

```bash
git add src/app/AUS3DScene.h src/app/AUS3DScene.cpp
git commit -m "feat: AUS3DScene新增Pass序列+纹理管理器基础框架"
```

---

### Task 6: RT池和CreateRenderTarget — 多Pass中间缓冲管理

**Files:**
- Modify: `src/app/AUS3DScene.h` (新增RTPool)
- Modify: `src/app/AUS3DScene.cpp` (实现RT池)

**问题：** `BeginRenderToTexture` 需要一个已创建的 `TextureHandle`（带VkImage/FBO）。AUS3DScene需要维护RT池，按需创建/复用。

- [ ] **Step 1: 在AUS3DScene.h添加RTPool**

```cpp
struct RTPool {
    struct Entry {
        TextureHandle handle;
        int width, height;
        bool inUse = false;
    };
    std::vector<Entry> entries;
    IRenderBackend* backend;
    
    TextureHandle Acquire(int w, int h);
    void Release(TextureHandle handle);
    void Clear(); // 销毁所有RT
};
```

- [ ] **Step 2: 实现RTPool::Acquire**

```cpp
TextureHandle RTPool::Acquire(int w, int h) {
    // 找空闲的匹配尺寸RT
    for (auto& e : entries) {
        if (!e.inUse && e.width == w && e.height == h) {
            e.inUse = true;
            return e.handle;
        }
    }
    // 创建新RT
    int texW = w > 0 ? w : 1920;
    int texH = h > 0 ? h : 1080;
    TextureHandle handle = backend->CreateTexture(texW, texH, TextureFormat::RGBA8, nullptr);
    entries.push_back({handle, texW, texH, true});
    return handle;
}

void RTPool::Release(TextureHandle handle) {
    for (auto& e : entries) {
        if (e.handle.id == handle.id) {
            e.inUse = false;
            return;
        }
    }
}
```

- [ ] **Step 3: 改造OnRender使用RT池**

完整的多Pass渲染循环：

```cpp
TextureHandle prevRT = {0};
for (size_t i = 0; i < fx.passes.size(); i++) {
    auto& pass = fx.passes[i];
    ShaderHandle passShader = LoadShader(pass.fragShader);
    
    ShaderParams passParams = params;
    int pw = pass.targetWidth > 0 ? pass.targetWidth : params.viewportWidth;
    int ph = pass.targetHeight > 0 ? pass.targetHeight : params.viewportHeight;
    passParams.viewportWidth = pw;
    passParams.viewportHeight = ph;
    
    if (pass.isOutput) {
        passParams.inputTextures = prevRT.id ? std::vector<TextureHandle>{prevRT} : std::vector<TextureHandle>{};
        m_backend->DrawToScreen(m_sharedVert, passShader, passParams, prevRT);
    } else {
        TextureHandle rt = m_rtPool.Acquire(pw, ph);
        m_backend->BeginRenderToTexture(rt);
        passParams.inputTextures = prevRT.id ? std::vector<TextureHandle>{prevRT} : std::vector<TextureHandle>{};
        m_backend->DrawFullscreenQuad(m_sharedVert, passShader, passParams);
        m_backend->EndRenderToTexture();
        if (prevRT.id) m_rtPool.Release(prevRT);
        prevRT = rt;
    }
}
if (prevRT.id) m_rtPool.Release(prevRT);
```

- [ ] **Step 4: 编译验证**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
```

- [ ] **Step 5: Commit**

```bash
git add src/app/AUS3DScene.h src/app/AUS3DScene.cpp
git commit -m "feat: RT池实现多Pass中间缓冲管理"
```

---

### Task 7: 后处理型着色器 — 水幕特效迁移 (v09)

**Files:**
- Create: `shaders/aus3d/v09_pass0_drop.frag` (水滴纹理采样)
- Create: `shaders/aus3d/v09_pass1_sphere.frag` (球体渲染)
- Create: `shaders/aus3d/v09_pass2_composite.frag` (合成输出)
- Modify: `src/app/AUS3DScene.cpp` (水幕特效定义改为多Pass)

**参考实现：** `aus_repo/Volume 09/ScreenWaterDropEffect/ScreenWaterDropEffect.shader`

- [ ] **Step 1: 从参考项目复制水滴纹理**

```powershell
mkdir -p e:\AI\graph\hight-post-proc\shader-showcase\assets\textures\aus3d
Copy-Item "e:\AI\graph\hight-post-proc\aus_repo\Volume 09 屏幕水幕特效Shader&Standard Shader\ScreenWaterDropEffect\Resources\ScreenWaterDrop.png" "e:\AI\graph\hight-post-proc\shader-showcase\assets\textures\aus3d\water_drop.png"
```

- [ ] **Step 2: 创建v09_pass0_drop.frag — 水滴纹理采样+UV偏移**

```glsl
#version 460
// Pass0: 采样水滴纹理 → 计算UV偏移量
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex; // 未使用
layout(binding=2) uniform sampler2D uAuxTex;    // 水滴纹理
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float speed = P0 * 0.3;
    float distortion = P1;
    
    // 三层不同缩放/速度的水滴采样
    vec2 uv = vUV;
    vec3 t1 = texture(uAuxTex, vec2(uv.x * 1.15, uv.y * 1.1 + uTime * speed * 0.15)).rgb;
    vec3 t2 = texture(uAuxTex, vec2(uv.x * 1.25 - 0.1, uv.y * 1.2 + uTime * speed * 0.2)).rgb;
    vec3 t3 = texture(uAuxTex, vec2(uv.x * 0.9, uv.y * 1.25 + uTime * speed * 0.032)).rgb;
    
    // 合成UV偏移 (R/G通道作为dx/dy)
    float dx = (t1.r + t2.r + t3.r) / 3.0 - 0.5;
    float dy = (t1.g + t2.g + t3.g) / 3.0 - 0.5;
    
    // 输出偏移量 (编码为颜色)
    outColor = vec4(dx * distortion, dy * distortion, 0.0, 1.0);
}
```

- [ ] **Step 3: 创建v09_pass1_sphere.frag — 偏移UV的球体渲染**

```glsl
#version 460
// Pass1: 用偏移UV渲染球体 + 背景涟漪
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex; // UV偏移纹理(RG=dx,dy)
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}

void main() {
    // 读取UV偏移
    vec2 offset = texture(uInputTex, vUV).rg * 2.0;
    
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    uv += offset; // 应用偏移
    
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;
    if(!hit(eye,rd,1.0,t)){
        // 背景涟漪
        float ripple = sin(vUV.y * 20.0 + uTime) * 0.1 + sin(vUV.x * 15.0 - uTime * 0.7) * 0.1;
        vec3 bg = vec3(0.02,0.04,0.10) + vec3(0.06,0.10,0.25) * abs(ripple);
        outColor = vec4(bg,1.0);
        return;
    }
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    vec3 col=vec3(0.9,0.35,0.15)*ndl;
    outColor=vec4(col*uLightColor,1.0);
}
```

- [ ] **Step 4: 创建v09_pass2_composite.frag — 合成输出**

```glsl
#version 460
// Pass2: 直接输出Pass1结果 (作为最终合成)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    outColor = texture(uInputTex, vUV);
}
```

- [ ] **Step 5: 更新BuildEffects中水幕特效的定义**

```cpp
// 水幕特效: 3Pass
{
    AUS3DEffect fx;
    fx.name = "水幕特效";
    fx.description = "水滴纹理多层采样+UV偏移折射+背景涟漪";
    fx.use3DGeometry = false;
    fx.passes = {
        {"shaders/aus3d/v09_pass0_drop.frag.spv", 0, 0, false},       // Pass0: 水滴纹理→UV偏移
        {"shaders/aus3d/v09_pass1_sphere.frag.spv", 0, 0, false},     // Pass1: 偏移UV球体渲染
        {"shaders/aus3d/v09_pass2_composite.frag.spv", 0, 0, true},   // Pass2: 输出到屏幕
    };
    fx.auxTextures = {"assets/textures/aus3d/water_drop.png"};
    fx.defaultValues = {1.0f, 1.0f}; // speed, distortion
    fx.paramLabels = {"速度", "扭曲强度"};
    fx.paramMin = {0.0f, 0.0f};
    fx.paramMax = {2.0f, 2.0f};
    m_effects.push_back(fx);
}
```

- [ ] **Step 6: 编译着色器**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\shaders\aus3d
glslangValidator -V v09_pass0_drop.frag -o v09_pass0_drop.frag.spv
glslangValidator -V v09_pass1_sphere.frag -o v09_pass1_sphere.frag.spv
glslangValidator -V v09_pass2_composite.frag -o v09_pass2_composite.frag.spv
Copy-Item *.spv ..\build\shaders\aus3d\
```

- [ ] **Step 7: 构建并测试**

```powershell
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake --build . --config Release
taskkill /f /im ShaderShowcase.exe 2>$null
$env:AUTO_TEST_AUS3D='1'
cd bin\Release
.\ShaderShowcase.exe
```

- [ ] **Step 8: Commit**

```bash
git add shaders/aus3d/v09_pass0_drop.frag shaders/aus3d/v09_pass1_sphere.frag shaders/aus3d/v09_pass2_composite.frag
git add assets/textures/aus3d/water_drop.png
git add src/app/AUS3DScene.cpp
git commit -m "feat: 水幕特效迁移为3Pass水滴纹理+UV偏移 (v09)"
```

---

### Task 8: 后处理型着色器 — 高斯模糊迁移 (v15)

**Files:**
- Create: `shaders/aus3d/v15_pass0_downsample.frag` (降采样)
- Create: `shaders/aus3d/v15_pass1_blur_v.frag` (垂直模糊)
- Create: `shaders/aus3d/v15_pass2_blur_h.frag` (水平模糊)
- Modify: `src/app/AUS3DScene.cpp` (BuildEffects)

**参考实现：** `aus_repo/Volume 15/RapidBlurEffect.shader`

- [ ] **Step 1: 创建v15_pass0_downsample.frag — 降采样1/4**

```glsl
#version 460
// Pass0: 2x2降采样
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    vec2 uv = vUV;
    // 4邻域平均 = 2x降采样
    vec4 c0 = texture(uInputTex, uv + vec2(-0.5, -0.5) * texelSize);
    vec4 c1 = texture(uInputTex, uv + vec2( 0.5, -0.5) * texelSize);
    vec4 c2 = texture(uInputTex, uv + vec2(-0.5,  0.5) * texelSize);
    vec4 c3 = texture(uInputTex, uv + vec2( 0.5,  0.5) * texelSize);
    outColor = (c0 + c1 + c2 + c3) * 0.25;
}
```

- [ ] **Step 2: 创建v15_pass1_blur_v.frag — 7tap垂直模糊**

```glsl
#version 460
// Pass1: 7-tap垂直高斯模糊
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

const float weights[7] = {0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205};

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    vec4 col = vec4(0.0);
    for (int i = 0; i < 7; i++) {
        float offset = float(i - 3) * P0 * 2.0; // P0控制模糊强度
        col += texture(uInputTex, vUV + vec2(0.0, offset) * texelSize) * weights[i];
    }
    outColor = col;
}
```

- [ ] **Step 3: 创建v15_pass2_blur_h.frag — 7tap水平模糊**

与v15_pass1_blur_v.frag相同，仅偏移方向改为水平：

```glsl
col += texture(uInputTex, vUV + vec2(offset, 0.0) * texelSize) * weights[i];
```

- [ ] **Step 4: 更新BuildEffects**

```cpp
fx.passes = {
    {"shaders/aus3d/v15_pass0_downsample.frag.spv", 960, 540, false},
    {"shaders/aus3d/v15_pass1_blur_v.frag.spv", 960, 540, false},
    {"shaders/aus3d/v15_pass2_blur_h.frag.spv", 0, 0, true},
};
```

- [ ] **Step 5: 编译/构建/测试/Commit** (同Task 7步骤6-8)

---

### Task 9: 后处理型着色器 — 径向模糊 (v08)、像素化 (v11)、油画 (v10)

**Files:**
- Create: `shaders/aus3d/v08_post_radial.frag` (径向模糊后处理)
- Create: `shaders/aus3d/v11_post_pixelate.frag` (像素化后处理)
- Create: `shaders/aus3d/v10_post_oil.frag` (油画后处理)
- Modify: `src/app/AUS3DScene.cpp`

这三个都是单Pass后处理，直接采样 `uInputTex`（球体渲染结果）。

- [ ] **Step 1: v08_post_radial.frag — 径向模糊**

```glsl
#version 460
// 径向模糊：以中心为原点，10次迭代缩放UV
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec2 center = vec2(0.5);
    vec2 uv = vUV - center;
    float intensity = P0 * 0.1;
    float iterations = P1 * 20.0 + 5.0;
    
    vec4 col = vec4(0.0);
    float scale = 1.0;
    for (int i = 0; i < 20; i++) {
        if (float(i) >= iterations) break;
        col += texture(uInputTex, uv * scale + center);
        scale = 1.0 + float(i) * intensity;
    }
    outColor = col / iterations;
}
```

- [ ] **Step 2: v11_post_pixelate.frag — 像素化**

```glsl
#version 460
// 像素化：UV坐标ceil取整
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float pixelSize = 1.0 / (P0 * 80.0 + 10.0); // 10-90像素块
    float ratio = uRes.x / uRes.y;
    float px = pixelSize * ceil(vUV.x / pixelSize);
    float py = pixelSize * ratio * ceil(vUV.y / (pixelSize * ratio));
    outColor = texture(uInputTex, vec2(px, py));
}
```

- [ ] **Step 3: v10_post_oil.frag — 油画特效**

```glsl
#version 460
// 油画特效：邻域颜色统计
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float radius = P0 * 5.0 + 1.0; // 1-6像素半径
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    
    vec4 avgColor = vec4(0.0);
    float count = 0.0;
    for (float dy = -radius; dy <= radius; dy += 1.0) {
        for (float dx = -radius; dx <= radius; dx += 1.0) {
            vec2 offset = vec2(dx, dy) * texelSize;
            avgColor += texture(uInputTex, vUV + offset);
            count += 1.0;
        }
    }
    outColor = avgColor / count;
}
```

- [ ] **Step 4: 更新BuildEffects中三个效果的定义** (单Pass, use3DGeometry=true, 但后处理采样场景渲染结果)

- [ ] **Step 5: 编译/构建/测试/Commit**

---

### Task 10: 球体+纹理型 — 卡通渐变 (v07) + Ramp纹理

**Files:**
- Modify: `shaders/aus3d/v07_toon.frag`
- Modify: `src/app/AUS3DScene.cpp`

参考实现使用 `tex2D(_Ramp, float2(NdotL, NdotL))` 替代色阶if/else。

- [ ] **Step 1: 重写v07_toon.frag**

```glsl
#version 460
// Vol.07 Toon: Ramp纹理卡通渐变
// binding=2: Ramp纹理 (程序化生成)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=2) uniform sampler2D uAuxTex; // Ramp纹理
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
bool hit(vec3 ro,vec3 rd,float r,out float t){
    float b=dot(ro,rd),c=dot(ro,ro)-r*r,h=b*b-c;
    if(h<0.0)return false;h=sqrt(h);t=-b-h;return t>0.001;
}
void main(){
    vec3 eye=uEyePos,fwd=normalize(-eye),rt=normalize(cross(fwd,vec3(0,1,0))),up=cross(rt,fwd);
    float ar=uRes.x/uRes.y;vec2 uv=(vUV-0.5)*2.0;uv.x*=ar;
    vec3 rd=normalize(fwd+uv.x*rt*0.55+uv.y*up*0.55);
    float t;if(!hit(eye,rd,1.0,t)){outColor=vec4(0.02,0.02,0.04,1);return;}
    vec3 P=eye+rd*t;vec3 N=normalize(P);vec3 L=normalize(uLightDir);
    float ndl=dot(N,L)*0.5+0.5;
    // Ramp纹理采样替代色阶
    vec3 rampColor = texture(uAuxTex, vec2(ndl, 0.5)).rgb;
    outColor = vec4(rampColor * uLightColor, 1.0);
}
```

- [ ] **Step 2: 更新BuildEffects — 卡通渐变使用Ramp纹理**

```cpp
fx.auxTextures = {"ramp_5"}; // 5色阶渐变
// 在OnEnter时调用 m_texManager.GenerateRampTexture(5)
```

- [ ] **Step 3: 编译/构建/测试/Commit**

---

### Task 11: 球体+纹理型 — 玻璃v2/v3 (v05) Cubemap + 凹凸/细节/Alpha纹理 (v01/v03/v06)

**Files:**
- Modify: `shaders/aus3d/v05_glass_v2.frag`, `v05_glass_v3.frag`
- Modify: `shaders/aus3d/v01_bump_rim.frag`, `v03_alpha.frag`, `v03_mix.frag`
- Modify: `shaders/aus3d/v06_detail.frag`, `v06_rim_detail.frag`
- Modify: `shaders/aus3d/v16_carpaint.frag`
- Modify: `src/app/AUS3DScene.cpp`

- [ ] **Step 1: 玻璃v2/v3 — 程序化Cubemap (天空色渐变)**

由于Vulkan不直接支持samplerCube（需要6面纹理），使用程序化天空色函数替代Cubemap采样。当前实现已有程序化天空色，本步保持现有实现，仅添加 `use3DGeometry=true` 标记。

- [ ] **Step 2: 凹凸纹理 — 程序化噪声纹理采样**

```glsl
// v01_bump_rim.frag中添加:
// binding=2: 噪声纹理
layout(binding=2) uniform sampler2D uAuxTex;
// 在球体命中后:
vec3 bump = texture(uAuxTex, uv * 3.0).rgb * 2.0 - 1.0;
N = normalize(N + bump * 0.1);
```

- [ ] **Step 3: Alpha混合纹理 — 纹理Alpha通道**

```glsl
// v03_alpha.frag中添加:
layout(binding=2) uniform sampler2D uAuxTex;
// alpha = texture(uAuxTex, uv).a * P0;
```

- [ ] **Step 4: 细节纹理 — 叠加层**

```glsl
// v06_detail.frag中添加:
layout(binding=2) uniform sampler2D uAuxTex;
// col *= texture(uAuxTex, uv * 5.0).rgb * 2.0;
```

- [ ] **Step 5: MatCap车漆 — MatCap纹理采样**

从参考项目复制 `CarPaint-MatCap.png`，用 `texture(uAuxTex, vec2(NdotV * 0.5 + 0.5, NdotL * 0.5 + 0.5))` 替代公式。

- [ ] **Step 6: 编译/构建/测试/Commit**

---

### Task 12: 最终验证 — 全部47效果截图对比

**Files:**
- 无新建文件，运行全面测试

- [ ] **Step 1: 运行完整自动化测试**

```powershell
taskkill /f /im ShaderShowcase.exe 2>$null
$env:AUTO_TEST_AUS3D='1'
cd e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release
.\ShaderShowcase.exe
```

- [ ] **Step 2: 运行综合分析脚本**

```powershell
cd c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96
python comprehensive_analysis.py
```

Expected: 所有效果通过，无新增问题

- [ ] **Step 3: 运行深度分析确认水幕特效**

```powershell
python water_detail.py
```

- [ ] **Step 4: Commit 最终版本**

```bash
git add -A
git commit -m "feat: 14个效果纹理+多Pass重构完成，全部通过验证"
```