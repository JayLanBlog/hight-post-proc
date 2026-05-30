# UI 打磨设计规格书

> 日期：2026-05-26 | 目标：修复 CoverFlow 缩略图、鼠标交互、详情页效果展示

## 1. 问题清单

| # | 问题 | 根因 |
|---|------|------|
| 1 | 卡��没有效果缩略图 | `OnImGui()` 只用 `AddRectFilled` 画纯色矩形，无纹理 |
| 2 | 没有鼠标点击卡片事件 | 直接 `ImDrawList` 绘制，没有交互控件 |
| 3 | 没有鼠标左右拖拽/滑动 | 只处理了 ImGui 键盘事件，无拖拽 |
| 4 | 详情页看不到效果、参数面板未显示 | 输入是 256×256 棋盘格，`m_showDebug` 默认 false |

## 2. 解决方案总览

### 2.1 缩略图渲染策略
- 使用**预渲染**方案：在 CoverFlowScene 初始化时，对每个效果做一次小尺寸（256×144）的离屏渲染（FBO），结果存为纹理
- 在卡片 `OnImGui()` 中用 `AddImageRounded` 将缩略图纹理绘制在卡片上
- 不使用实时渲染：CoverFlow 页面没有效果 shader 加载，避免 GL 状态污染

### 2.2 输入纹理策略
- 用 `stb_image` 加载一张测试图片（从 Unsplash 下载 1920×1080 风景照），替换棋盘格
- 这个图片同时作为 CoverFlow 缩略图渲染和详情页效果渲染的输入

### 2.3 鼠标交互策略
- 每张可见卡片覆盖一个 `InvisibleButton`，检测点击 → 打开详情
- 在窗口级别检测鼠标拖拽（`IsMouseDragging`）→ 左右滑动翻页
- 保留键盘方向键和滚轮翻页

### 2.4 详情页策略
- `m_showDebug` 默认 true，Tab 键切换
- 加载的测试图片作为输入纹理，效果 shader 直接渲染全屏

## 3. 修改文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/main.cpp` | 修改 | stb 加载图片 + 创建 FBO 预渲染 18 个缩略图 |
| `src/app/CoverFlowScene.h` | 修改 | 添加缩略图纹理数组、鼠标拖拽状态 |
| `src/app/CoverFlowScene.cpp` | 修改 | 卡片渲染 AddImage、鼠标点击+拖拽交互 |
| `src/app/EffectDetailScene.h` | 修改 | m_showDebug 改为 true |
| `src/app/EffectDetailScene.cpp` | 修改 | 默认显示参数面板 |
| `assets/` | 新建 | 存放下载的测试图片 |

## 4. 预渲染缩略图流程

```
main.cpp:
  1. stbi_load("assets/test.jpg") → CPU buffer RGBA8
  2. backend→CreateTexture(1920,1080, RGBA8, buffer) → texInput
  3. Load fullscreen.vert.spv → vertShader
  4. For each effect card:
     a. Load effect.frag.spv → fragShader
     b. CreateTexture(256,144, RGBA8, NULL) → texThumb (as render target)
     c. backend→BeginRenderToTexture(texThumb)
     d. backend→DrawFullscreenQuad(vertShader, fragShader, texInput)
     e. backend→EndRenderToTexture()
     f. Destroy fragShader (not needed after thumbnail rendered)
  5. Pass texThumb[18] to CoverFlowScene

CoverFlowScene.cpp:
  每个卡片区域先画缩略图 (AddImageRounded with texThumb[i])，
  再覆盖半透明深色层给文字留可读性，最后画文字。
```

## 5. 测试资源

从 Unsplash 下载免费图片作为测试输入：
- URL: `https://images.unsplash.com/photo-1506905925346-21bda4d32df4?w=1920&q=80`
- 分辨率: 1920×1080 (缩放到 fit)
- 使用 curl/wget 下载到 `assets/test.jpg`
