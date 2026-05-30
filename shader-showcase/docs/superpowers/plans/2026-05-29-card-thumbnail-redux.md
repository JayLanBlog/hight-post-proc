# Card Thumbnail Redux Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复 CoverFlow 卡片缩略图镜像 + 将缩略图从未静态预渲染改为每帧实时渲染效果 shader

**Architecture:** 将缩略图渲染逻辑从 main.cpp 迁移到 CoverFlowScene 内部。CoverFlowScene 持有每张卡片的 shader 句柄 + 256×144 FBO 缩略图纹理，每帧 OnRender 中遍历可见卡片（±3）跑效果 shader 更新缩略图。main.cpp 精简 80+ 行。

**Tech Stack:** C++17, ImGui, OpenGL 4.6 (SPIR-V), stb_image

---

## File Structure

| 文件 | 变更类型 | 职责 |
|------|---------|------|
| `src/app/CoverFlowScene.h` | 修改 | 新增 CardThumbnailState, 新成员变量和新方法声明 |
| `src/app/CoverFlowScene.cpp` | 修改 | 实现 InitializeThumbnails/RenderVisibleThumbnails，镜像修复 |
| `src/app/EffectDetailScene.cpp` | 修改 | GetNextScene 中移除 SetThumbnails，传递 testImageBaseDir |
| `src/app/CoverFlowState.h` | 修改 | 新增 testImageBaseDir 字段 |
| `src/main.cpp` | 修改 | 移除缩略图预渲染代码，添加 SetTestImageBaseDir 调用 |

---

### Task 1: CoverFlowScene.h — 新增数据结构和声明

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.h`

- [ ] **Step 1: 在头文件 private 区域新增 CardThumbnailState 结构体和成员**

在 `#include "render/IRenderBackend.h"` 之后，`class CoverFlowScene : public Scene` 之前，插入新结构体：

```cpp
/// Per-card thumbnail real-time render state
struct CardThumbnailState {
    ShaderHandle fragShader;   // 效果 shader（每卡独立）
    TextureHandle thumbTex;    // 256×144 缩略图 FBO 纹理
};
```

在 CoverFlowScene 类 public 区域，`SetThumbnails` 之后新增：

```cpp
    /// Set test image base directory (for thumbnail input textures)
    void SetTestImageBaseDir(const std::string& dir) { m_testImageBaseDir = dir; }
```

在 CoverFlowScene 类 private 区域，`std::vector<void*> m_thumbIds;` 之后新增：

```cpp

    // ---- Dynamic thumbnail rendering ----
    ShaderHandle m_sharedVertShader = INVALID_SHADER;       // 所有卡共用的 fullscreen.vert
    std::vector<CardThumbnailState> m_thumbnailStates;      // 18 张卡的渲染状态
    int m_thumbWidth  = 256;
    int m_thumbHeight = 144;
    bool m_thumbInitialized = false;
    std::string m_testImageBaseDir;                         // 测试图片目录
    float m_thumbElapsedTime = 0.0f;                        // 缩略图累计时间
    uint32_t m_thumbFrameCount = 0;                         // 缩略图帧计数

    void InitializeThumbnails();     // 加载 shader + 创建缩略图纹理
    void RenderVisibleThumbnails();  // 每帧渲染可见卡片缩略图
```

### Task 2: CoverFlowScene.cpp — InitializeThumbnails()

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.cpp`

- [ ] **Step 1: 实现 InitializeThumbnails()**

在文件末尾（`GetState` 函数之后，`OpenVideoFile` 之前）插入：

```cpp
// ============================================================================
// Dynamic Thumbnail Rendering
// ============================================================================

void CoverFlowScene::InitializeThumbnails()
{
    if (m_thumbInitialized || !m_backend) return;
    if (m_cards.empty()) return;

    std::string shaderDir = ShaderLoader::FindShaderDir();
    
    // Load shared vertex shader (all cards use the same fullscreen.vert)
    auto vertSpv = ShaderLoader::LoadSPIRV(shaderDir + "/common/fullscreen.vert.spv");
    if (vertSpv.empty()) {
        fprintf(stderr, "[CoverFlowScene] Cannot load vertex shader for thumbnails\n");
        return;
    }
    m_sharedVertShader = m_backend->CreateVertexShader(vertSpv);
    if (m_sharedVertShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[CoverFlowScene] Cannot create vertex shader for thumbnails\n");
        return;
    }

    // Pre-allocate m_thumbIds to match card count
    m_thumbIds.resize(m_cards.size(), nullptr);

    // For each card: load frag shader + create thumbnail FBO texture
    for (int i = 0; i < (int)m_cards.size(); i++) {
        CardThumbnailState state;
        state.fragShader = INVALID_SHADER;
        state.thumbTex = INVALID_TEXTURE;

        // Load fragment SPIR-V
        auto fragSpv = ShaderLoader::LoadSPIRV(m_cards[i].fragSpirvPath);
        if (!fragSpv.empty()) {
            state.fragShader = m_backend->CreateFragmentShader(fragSpv);
        }
        // Note: if fragSpv is empty, state.fragShader stays INVALID — card shows gray placeholder

        // Create 256×144 thumbnail FBO texture
        state.thumbTex = m_backend->CreateTexture(m_thumbWidth, m_thumbHeight, TextureFormat::RGBA8, nullptr);

        m_thumbnailStates.push_back(state);
    }

    m_thumbInitialized = true;
    printf("[CoverFlowScene] Thumbnails initialized: %zu cards, %dx%d\n",
           m_thumbnailStates.size(), m_thumbWidth, m_thumbHeight);
}
```

- [ ] **Step 2: 在 OnEnter() 末尾添加初始化调用**

在 `CoverFlowScene::OnEnter()` 的 `printf` 之后，函数末尾 `}` 之前插入：

```cpp
    InitializeThumbnails();
```

修改后的 OnEnter() 完整代码：

```cpp
void CoverFlowScene::OnEnter()
{
    m_fpsLastTime    = std::chrono::high_resolution_clock::now();
    m_fpsFrameCount  = 0;
    m_fpsDisplay     = 0.0f;

    printf("[CoverFlowScene] Entered with %zu cards, selected=%d\n",
           m_cards.size(), m_selectedIndex);
    InitializeThumbnails();
}
```

### Task 3: CoverFlowScene.cpp — RenderVisibleThumbnails()

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.cpp`

- [ ] **Step 1: 实现 RenderVisibleThumbnails()**

在 `InitializeThumbnails()` 之后继续插入：

```cpp
void CoverFlowScene::RenderVisibleThumbnails()
{
    if (!m_thumbInitialized || !m_backend) return;

    auto* glBackend = dynamic_cast<OpenGLBackend*>(m_backend);
    if (!glBackend) return;

    const int vr = 3;  // visible range

    for (int i = m_selectedIndex - vr; i <= m_selectedIndex + vr; i++) {
        if (i < 0 || i >= (int)m_thumbnailStates.size()) continue;
        const auto& state = m_thumbnailStates[i];
        if (state.fragShader.id == INVALID_SHADER.id) continue;
        if (state.thumbTex.id == INVALID_TEXTURE.id) continue;

        // Pick input texture for this card
        TextureHandle input = m_inputTex;
        if (i < (int)m_inputTexCache.size() && m_inputTexCache[i].id != INVALID_TEXTURE.id) {
            input = m_inputTexCache[i];
        }

        m_backend->BeginRenderToTexture(state.thumbTex);
        ShaderParams params;
        params.inputTextures.push_back(input);
        params.uniformFloats  = {4.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        params.viewportWidth  = m_thumbWidth;
        params.viewportHeight = m_thumbHeight;
        params.time           = m_thumbElapsedTime;
        params.frameCount     = m_thumbFrameCount;
        m_backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
        m_backend->EndRenderToTexture();

        // Update ImTextureID for this card
        m_thumbIds[i] = glBackend->GetImTextureID(state.thumbTex);
    }
}
```

- [ ] **Step 2: 在 OnRender() 中添加缩略图渲染调用**

修改 `CoverFlowScene::OnRender`：

```cpp
void CoverFlowScene::OnRender(IRenderBackend* /*backend*/)
{
    // Update thumbnail time/frame counters
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    m_thumbElapsedTime += dt;
    m_thumbFrameCount++;

    // Render visible card thumbnails every frame
    RenderVisibleThumbnails();
}
```

- [ ] **Step 3: 添加 ShaderLoader.h 以及 Chrono header 的 include**

在 CoverFlowScene.cpp 顶部已有 include 区域检查并确保有 `<chrono>`（已有，在 `.h` 中的 FPS counter 用到）。确保有 `#include "shader/ShaderLoader.h"`（检查第 5 行已存在）。

### Task 4: CoverFlowScene.cpp — 镜像修复

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.cpp`

- [ ] **Step 1: 翻转卡片纹理 UV 坐标**

将第 411 行的 UV 参数从 `ImVec2(0,0), ImVec2(1,1)` 改为 `ImVec2(0,1), ImVec2(1,0)`：

```cpp
// 修改前:
            dl->AddImageRounded(m_thumbIds[i],
                ImVec2(x0, y0), ImVec2(x1, y1),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(255, 255, 255, ai), r);

// 修改后:
            dl->AddImageRounded(m_thumbIds[i],
                ImVec2(x0, y0), ImVec2(x1, y1),
                ImVec2(0, 1), ImVec2(1, 0),   // UV flipped: OpenGL (bottom-left origin) -> ImGui (top-left origin)
                IM_COL32(255, 255, 255, ai), r);
```

### Task 5: CoverFlowScene.cpp — 析构函数清理 shader 资源

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.cpp`

- [ ] **Step 1: 修改析构函数以清理缩略图 shader 和纹理**

Replace `CoverFlowScene::~CoverFlowScene() = default;` (line 23) with:

```cpp
CoverFlowScene::~CoverFlowScene()
{
    // Destroy thumbnail resources (shaders + textures created in InitializeThumbnails)
    if (m_backend) {
        if (m_sharedVertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_sharedVertShader);
            m_sharedVertShader = INVALID_SHADER;
        }
        for (auto& state : m_thumbnailStates) {
            if (state.fragShader.id != INVALID_SHADER.id) {
                m_backend->DestroyShader(state.fragShader);
                state.fragShader = INVALID_SHADER;
            }
            if (state.thumbTex.id != INVALID_TEXTURE.id) {
                m_backend->DestroyTexture(state.thumbTex);
                state.thumbTex = INVALID_TEXTURE;
            }
        }
        m_thumbnailStates.clear();
    }
}
```

> **Note:** `m_backend` pointer is valid during scene destruction because backend outlives scenes (it's owned by Application). If transitioning between scenes, the new CoverFlowScene creates fresh shaders in InitializeThumbnails, so there's no shared-state conflict.

### Task 6: CoverFlowState.h — 新增 testImageBaseDir 字段

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowState.h`

- [ ] **Step 1: 在 CoverFlowState 结构体中添加 testImageBaseDir**

在 `autoTestCardIndex` 之后添加：

```cpp
    int                      autoTestCardIndex  = 0;
    std::string              testImageBaseDir;   // for thumbnail initialization on restore
```

修改后的 CoverFlowState.h 关键部分：

```cpp
struct CoverFlowState {
    std::vector<void*>       thumbIds;
    std::vector<TextureHandle> inputTexCache;
    std::vector<std::string> imagePool;
    std::vector<std::string> videoPool;
    int                      currentImageIndex = 0;
    int                      selectedIndex    = 0;
    Application*             app              = nullptr;
    TextureHandle            inputTex         = {0};
    IRenderBackend*          backend          = nullptr;
    bool                     captureActive    = false;
    bool                     autoTest         = false;
    int                      autoTestHoldFrames = 0;
    int                      autoTestCardIndex  = 0;
    std::string              testImageBaseDir;   // NEW: for thumbnail init on restore
};
```

### Task 7: CoverFlowScene.cpp — GetState() 保存 testImageBaseDir

**Files:**
- Modify: `shader-showcase/src/app/CoverFlowScene.cpp`

- [ ] **Step 1: GetState 中添加 testImageBaseDir**

在 `CoverFlowScene::GetState()` 函数的 `s.autoTestCardIndex = m_autoTestCardIndex;` 之后添加：

```cpp
    s.testImageBaseDir = m_testImageBaseDir;
```

### Task 8: EffectDetailScene.cpp — GetNextScene() 移除 SetThumbnails 调用

**Files:**
- Modify: `shader-showcase/src/app/EffectDetailScene.cpp`

- [ ] **Step 1: 移除 SetThumbnails 调用，添加 SetTestImageBaseDir**

将 `EffectDetailScene::GetNextScene()` 中的：

```cpp
        coverFlow->SetThumbnails(m_savedState.thumbIds);
```

替换为：

```cpp
        // Thumbnails are now initialized internally by CoverFlowScene::OnEnter
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);
```

修改后的代码片段：

```cpp
        coverFlow->SetInputTexture(m_savedState.inputTex);
        coverFlow->SetInputTexCache(m_savedState.inputTexCache);
        coverFlow->SetBackend(m_savedState.backend);
        coverFlow->SetApplication(m_savedState.app);
        // Thumbnails are now initialized internally by CoverFlowScene::OnEnter
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);
```

### Task 9: main.cpp — 精简缩略图预渲染代码

**Files:**
- Modify: `shader-showcase/src/main.cpp`

- [ ] **Step 1: 移除 effectList/testImageList 数组和缩略图预渲染循环**

删除以下行（约 82-189）：

```
82:    const char* effectList[] = {
83:        "simple_test","bloom","blur","sharpen","edge_detect",
84:        "emboss","pixelate","vignette","chromatic","color_grade",
85:        "noise","kaleidoscope","glitch","toon","vhs",
86:        "crt","water_ripple","lens_distort"
87:    };
88:    // Corresponding test image filenames (in screenshots/assets/images/)
89:    const char* testImageList[] = { ... };
...
110:    std::vector<void*> thumbImIds;
111:    thumbImIds.reserve(NUM_EFFECTS);
...
189:    }  // end of thumbnail rendering for-loop
```

- [ ] **Step 2: 保留输入纹理缓存创建代码**

在原有位置保留以下代码块（用于创建 per-effect 输入纹理）：

```cpp
        const int NUM_EFFECTS = 18;
        std::vector<TextureHandle> inputTexCache;
        inputTexCache.reserve(NUM_EFFECTS);

        // Test image filenames (one per effect)
        const char* testImageList[] = {
            "00_grayscale_landscape.jpg","01_bloom_citynight.jpg","02_blur_brickwall.jpg",
            "03_sharpen_architecture.jpg","04_edge_building.jpg","05_emboss_metal.jpg",
            "06_pixelate_portrait.jpg","07_vignette_flower.jpg","08_chromatic_leaves.jpg",
            "09_colorgrading_food.jpg","10_noise_sky.jpg","11_kaleidoscope_mandala.jpg",
            "12_glitch_tech.jpg","13_toon_cartoon.jpg","14_vhs_retro.jpg",
            "15_crt_screen.jpg","16_water_lake.jpg","17_lens_wideangle.jpg"
        };

        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string testImgPath = testImageBaseDir + testImageList[i];
            int tiw = 0, tih = 0, tcomp = 0;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* testImgData = stbi_load(testImgPath.c_str(), &tiw, &tih, &tcomp, 4);
            stbi_set_flip_vertically_on_load(false);

            TextureHandle effectInputTex = inputTex;
            if (testImgData) {
                printf("[main] Loaded test image for effect %d: %s (%d x %d)\n", i, testImgPath.c_str(), tiw, tih);
                effectInputTex = backend->CreateTexture(tiw, tih, TextureFormat::RGBA8, testImgData);
                stbi_image_free(testImgData);
            } else {
                printf("[main] Warning: Cannot load test image %s, using default\n", testImgPath.c_str());
            }
            inputTexCache.push_back(effectInputTex);
        }
```

- [ ] **Step 3: 移除 SetThumbnails 和 thumbImIds 相关代码**

删除：
```cpp
coverFlow->SetThumbnails(thumbImIds);
```

以及删除 `std::vector<void*> thumbImIds;` 声明行和 `thumbImIds.push_back(imId);` 等引用。

- [ ] **Step 4: 添加 SetTestImageBaseDir 调用**

在 `coverFlow->SetInputTexCache(inputTexCache);` 之后添加：

```cpp
        coverFlow->SetTestImageBaseDir(testImageBaseDir);
```

- [ ] **Step 5: 删除不再需要的变量**

删除：
- `vertShader` 创建和销毁（vertSpv 加载、CreateVertexShader、DestroyShader，约 4 行）
- `thumbImIds` 向量

保留：
- `inputTex` 创建（作为默认回退纹理）
- `inputTexCache` 创建

### Task 10: 编译验证

**Files:**
- 无新建文件

- [ ] **Step 1: 编译项目**

```bash
cmake --build shader-showcase/build --config Release --target ShaderShowcase
```

期望：编译成功，无错误。

- [ ] **Step 2: 运行验证**

运行 `ShaderShowcase.exe`，验证：
- CoverFlow 卡片缩略图方向正确（不再上下颠倒）
- 时间性效果（glitch/vhs/noise）在卡片缩略图中可见动画
- 点击卡片进入详情页正常
- 从详情页 ESC 返回 CoverFlow 正常，缩略图继续渲染

### Task 11: Commit

- [ ] **Step 1: 提交所有修改**

```bash
git add shader-showcase/src/app/CoverFlowScene.h \
        shader-showcase/src/app/CoverFlowScene.cpp \
        shader-showcase/src/app/CoverFlowState.h \
        shader-showcase/src/app/EffectDetailScene.cpp \
        shader-showcase/src/main.cpp
git commit -m "feat: dynamic card thumbnails + mirror fix

- Fix UV flip: ImGui AddImageRounded uses top-left origin, OpenGL uses bottom-left
- Move thumbnail rendering from main.cpp into CoverFlowScene (InitializeThumbnails + RenderVisibleThumbnails)
- All visible cards (±3) rendered with effect shaders every frame
- Time-dependent effects (noise/vhs/glitch) now show animation in card thumbnails
- Simplify main.cpp: remove ~80 lines of pre-rendering code
- Pass testImageBaseDir through CoverFlowState for scene restore"
```
