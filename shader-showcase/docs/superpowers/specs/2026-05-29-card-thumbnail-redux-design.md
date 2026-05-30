# Card Thumbnail Redux — 卡片缩略图动态渲染 & 镜像修复

## 问题描述

1. **卡片框图片镜像**：CoverFlow 卡片框显示的缩略图是上下颠倒的，而详情页显示正确。根因：图片加载时 `stbi_set_flip_vertically_on_load(true)` 适配 OpenGL 底部原点坐标，但 ImGui `AddImageRounded` 使用顶部原点 UV `(0,0)-(1,1)`，导致纹理上下翻转。

2. **卡片框静态缩略图**：目前 18 张缩略图在 `main.cpp` 中启动时一次性预渲染到 256×144 FBO，之后永不更新。需要改为每帧实时渲染，使具有时间性参数的效果（noise/vhs/glitch/kaleidoscope/water_ripple 等）在封面流中展示动画；卡片框应动态展示其对应详情页效果的缩略动图。

## 预期行为

- CoverFlow 中所有可见卡片（选中卡片前后各 3 张，共约 7 张）**每帧实时渲染**效果 shader
- 时间性效果在缩略图中可见动画（噪点闪烁、VHS 条纹滚动等）
- 卡片框图片方向与详情页一致，不再上下颠倒
- 性能开销可控：7 张 256×144 小纹理的渲染对现代 GPU 可忽略

## 架构方案

**方案 A：CoverFlowScene 内建实时渲染（选中）**

将缩略图渲染逻辑从 `main.cpp` 迁移到 `CoverFlowScene` 内部。CoverFlowScene 持有每张卡片的 shader 句柄 + 缩略图 FBO 纹理 + 输入纹理，在 `OnRender()` 中遍历可见卡片，每帧跑一次效果 shader 到缩略图 FBO。

## 修改清单

### 修改 1：镜像修复（1 行）

**文件**：`src/app/CoverFlowScene.cpp` 第 411 行

```
修改前:
dl->AddImageRounded(m_thumbIds[i],
    ImVec2(x0, y0), ImVec2(x1, y1),
    ImVec2(0, 0), ImVec2(1, 1),
    IM_COL32(255, 255, 255, ai), r);

修改后:
dl->AddImageRounded(m_thumbIds[i],
    ImVec2(x0, y0), ImVec2(x1, y1),
    ImVec2(0, 1), ImVec2(1, 0),   // UV 翻转，修正 OpenGL→ImGui 坐标系
    IM_COL32(255, 255, 255, ai), r);
```

### 修改 2：动态缩略图渲染

#### 2.1 CoverFlowScene.h — 新增数据结构

```cpp
// 新增结构体：每张卡片的缩略图渲染状态
struct CardThumbnailState {
    ShaderHandle fragShader;   // 效果 shader（每卡独立）
    TextureHandle thumbTex;    // 256×144 缩略图 FBO 纹理
};

// 新增成员变量（private 区域）
ShaderHandle m_sharedVertShader;                      // 所有卡共用的 fullscreen.vert
std::vector<CardThumbnailState> m_thumbnailStates;   // 18 张卡的渲染状态
int m_thumbWidth  = 256;
int m_thumbHeight = 144;
bool m_thumbInitialized = false;

std::string m_shaderDir;         // shader 根目录（延迟缓存）
std::string m_testImageBaseDir;  // 测试图片目录
std::vector<std::string> m_effectIds;    // 效果 ID 列表
std::vector<std::string> m_testImgNames; // 测试图片文件名列表

// 新增方法
void InitializeThumbnails();     // 加载 shader + 创建缩略图纹理
void RenderVisibleThumbnails();  // 每帧渲染可见卡片缩略图
```

#### 2.2 CoverFlowScene.cpp — 初始化（OnEnter）

`OnEnter()` 调用 `InitializeThumbnails()`：

1. 查找 shader 目录路径
2. 加载 `common/fullscreen.vert.spv` 作为共享顶点 shader
3. 遍历 18 个效果，加载各效果 `.frag.spv`
4. 创建各效果 256×144 FBO 纹理
5. 设置 `m_thumbInitialized = true`

#### 2.3 CoverFlowScene.cpp — 每帧渲染（OnRender）

`OnRender()` 中调用 `RenderVisibleThumbnails()`：

```
for i in [selectedIndex-3 .. selectedIndex+3]:
    if i < 0 or i >= 18: continue
    // 绑定缩略图 FBO
    backend->BeginRenderToTexture(m_thumbnailStates[i].thumbTex)
    // 设定 shader params（帧计数 + 默认参数）
    ShaderParams params
    params.inputTextures = [对应输入纹理]
    params.uniformFloats = [默认值 4.0, 1.0, 1.0, 1.0, 1.0, 1.0]
    params.time = 累计时间
    params.frameCount = 当前帧数
    // 渲染
    backend->DrawFullscreenQuad(vertShader, fragShader, params)
    backend->EndRenderToTexture()
    
    // 刷新 ImTextureID
    m_thumbIds[i] = gl->GetImTextureID(m_thumbnailStates[i].thumbTex)
```

**关键**：帧计数 `frameCount` 必须递增，时间性效果才能产生动画。

#### 2.4 CoverFlowScene.cpp — OnImGui 修改

移除对 `m_thumbIds` 数组的外部依赖。`m_thumbIds` 现在由 CoverFlowScene 内部管理，OnRender 中刷新后 OnImGui 自动使用最新纹理。

`SetThumbnails()` 方法保留但标记为 deprecated（不再需要外部调用）。

#### 2.5 main.cpp — 精简

移除以下代码块（约 80 行）：

- `main.cpp:82-108` — `effectList`/`testImageList` 数组
- `main.cpp:110-189` — 预渲染缩略图循环（加载 SPIR-V、创建 FBO、DrawFullscreenQuad）
- `main.cpp:196` — `SetThumbnails(thumbImIds)` 调用

保留内容：

- 输入纹理缓存 `inputTexCache` 创建（每张效果仍需独立的输入纹理，通过 `SetInputTexCache()` 传入）
- CoverFlowScene 创建 + `SetInputTexture` / `SetInputTexCache` / `SetBackend`

**main.cpp 与 CoverFlowScene 的新交互**：

新增方法来传递初始化所需数据：
```cpp
coverFlow->SetEffectData(effectIds, testImgNames, shaderDir, testImageBaseDir);
```

### 修改 3：EffectDetailScene 参数回传（可选增强）

从详情页返回 CoverFlow 时，将用户调整后的参数值传递给 CoverFlowScene，使缩略图也反映最新参数。**本次不实现**，留作后续优化。默认使用标准参数值渲染缩略图。

### 资源统计

- 18 个 `ShaderHandle`（frag shader）
- 1 个共享 VS `ShaderHandle`
- 18 个 256×144 `RGBA8` 纹理 = 18×256×144×4 = ~2.66 MB VRAM
- 每帧 7 次 `BeginRenderToTexture`/`EndRenderToTexture` + `DrawFullscreenQuad`
- 每帧 7 次 `GetImTextureID` 调用

## 不改动

- `OpenGLBackend` / `IRenderBackend` 接口
- `EffectDetailScene` 逻辑
- 输入纹理缓存机制（18 张独立输入纹理保持不变）
- `CoverFlowState` 序列化/反序列化
- auto-test / screenshot 模式

## 风险

- **shader 编译错误**：某个 shader 编译失败不应阻塞其他卡片。用 try-catch 或返回值检查，失败卡片显示灰色占位。
- **初始化时机**：`InitializeThumbnails()` 依赖 backend 已就绪，需在 `OnEnter()` 或首帧 `OnRender()` 中调用，确保 GL 上下文已创建。
- **性能**：7 次 FBO 绑定/解绑在 Release 模式下开销极小（<1ms），但需要实测验证。
