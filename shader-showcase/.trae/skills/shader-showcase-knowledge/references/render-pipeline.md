# 渲染管线

## 全屏渲染流程

```
1. OnRender() 调用
   ├─ 获取 m_thumbnailStates[selectedIndex].fragShader
   ├─ 创建/调整 m_immersiveTex FBO (RGBA8, 窗口大小)
   ├─ 选择输入纹理 (m_inputTexCache[i] 优先)
   ├─ 从 m_cards[i].params 构建 uniformFloats[] / uniformInts[]
   ├─ backend->BeginRenderToTexture(m_immersiveTex)
   ├─ backend->DrawFullscreenQuad(vertShader, fragShader, shaderParams)
   └─ backend->EndRenderToTexture()
        └─ GetImTextureID() → m_immersiveImTexID

2. OnImGui() 显示
   ├─ Layer 1: ##ImmersiveBg (NoInputs 窗口)
   │    └─ bgdl->AddImage(m_immersiveImTexID, 0,0, w,h, UV=0,1→1,0)
   ├─ Layer 2: ForegroundDrawList
   │    └─ 分类标签、分段控件、效果名、描述、页码、提示
   └─ Layer 3: ##ImmersiveUI (透明窗口)
        └─ InvisibleButton (分类点击 + 全屏点击进入)
```

## ShaderParams 结构

```cpp
struct ShaderParams {
    std::vector<TextureHandle> inputTextures;  // 输入纹理列表
    std::vector<float>        uniformFloats;   // float uniform 值
    std::vector<int32_t>      uniformInts;     // int/bool uniform 值
    int  viewportWidth, viewportHeight;
    float time;         // uTime
    uint32_t frameCount; // uFrameCount
};
```

## effect.json 格式

```json
{
  "name": "效果名称",
  "name_cn": "中文名称",
  "category": "分类",
  "category_cn": "中文分类",
  "description": "描述",
  "description_cn": "中文描述",
  "params": [
    {
      "name": "uParamFloat0",
      "label": "强度",
      "type": "float",
      "min": 0.0,
      "max": 1.0,
      "default": 0.5,
      "ui": "slider"
    }
  ]
}
```

支持的 ParamType: Float, Int, Bool, Float2, Float3, Float4, Color
支持的 uiType: slider, drag, combo, color, checkbox

## 对比模式 (EffectDetailScene)

```
RenderCompareView():
  ├─ RenderFullscreenEffect() → m_effectTex FBO
  └─ OnImGui():
       ├─ AddImage(原图, 全屏)
       ├─ AddImage(效果图, splitX 右侧裁剪)
       ├─ 分割线 + 拖拽手柄 (蓝色圆圈 + ◀▶ 三角形)
       ├─ "Before"/"After" 标签 (阴影描边)
       └─ 鼠标拖拽交互 (ResizeEW 光标)
```

<!-- layer:L2 -->
## L2: GLSL std140 UBO 参数补全规则

> 来源：2026-06-26 XPL Glitch 迁移，17 个 shader 渲染无效。C++ 端向 UBO 写入固定 6 个 float 参数，shader 端参数少于 6 时 std140 偏移错位。

**根因**：`OpenGLBackend::DrawFullscreenQuad()` 固定写 48 字节 UBO（6 float + vec2 + float + float）。GLSL 端若只声明 2 个 float 参数，`uResolution` 出现在 byte 8 而非预期的 byte 24。

**修复**：不足 6 个时用 `_pad0.._padN` 补到 6 个 float：

```glsl
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // byte 0
    float uParamFloat1;  // byte 4
    float _pad0;         // byte 8   ← 占位，不可省略
    float _pad1;         // byte 12
    float _pad2;         // byte 16
    float _pad3;         // byte 20
    vec2  uResolution;   // byte 24  ← 必须对齐
    float uTime;         // byte 32
    float uFrameCount;   // byte 36
};
```

**通用规则**：C/C++ 端 UBO buffer 和 GLSL layout 的字节偏移必须逐字段匹配。不匹配时不报错，只会静默读到错误数据。Vulkan push_constant 无此约束——切换后端时必须检查。

<!-- layer:L3 -->
## L3: SPIR-V push_constant 与 OpenGL UBO 的不兼容性

> 来源：2026-06-26，XPL shader 用 `layout(push_constant) uniform PC {...}`，OpenGL/NVIDIA 驱动将其转为 UBO 但块名是 `pc`，引擎的 `OpenGLBackend` 只向 `Params` UBO（binding=1）写入数据。

**症状**：画面对比无效果/黑屏，程序无报错。

**根因链**：
1. `glslangValidator` 编译 `push_constant` → SPIR-V 中的 `PushConstant` 装饰
2. 运行时 NVIDIA GL 驱动将其转为**独立 UBO**，块名 `pc`
3. 引擎的 `OpenGLBackend::DrawFullscreenQuad()` 通过 `glGetUniformBlockIndex(program, "Params")` 查找 binding=1 的块
4. `"Params"` 查不到（只有 `"pc"`），数据从未到达 shader

**修复**：OpenGL 路径统一用 `layout(std140, binding=1) uniform Params { ... }`。不依赖驱动自动桥接。

**跨后端教训**：
- Vulkan 路径：可用 `push_constant`（SPIR-V 原生支持）
- OpenGL 路径：必须用 `std140` UBO `Params`
- 双后端共用 shader 源码时，必须保证 GLSL 端兼容 OpenGL

<!-- layer:L4 -->
## L4: LanguageManager 是显示名称的唯一真实源

> 来源：2026-06-26，改了 CARD 宏和 effect.json 后界面仍显示英文。

**教训**：`CoverFlowScene` 和 `EffectDetailScene` 的显示文本从 `LanguageManager::CardName(id)` / `CardDesc(id)` 的硬编码查找表读取，不是从 CARD 宏或 effect.json。

新增效果时需同步更新三处：
1. `shaders/effects/{id}/effect.json` — 参数元数据（供 DetailScene DebugPanel）
2. `CoverFlowScene.cpp` CARD 宏 — 效果注册（内部索引/分类）
3. **`LanguageManager.cpp` CardName/CardDesc 表** — 实际显示文本（中英双语）

<!-- layer:L4 -->
## L4: AUTO_TEST_CARDS 需三层绕过

自动化截图模式需同时绕过：
1. **场景路由**（main.cpp 直启 CoverFlowScene 而非 SceneGalleryScene）
2. **Dynamic/Static 隔离**（`m_filteredIndices` 强行填充全部 35 张卡片）
3. **Vulkan 后端**（`Application::Run()` 中检测到 AUTO_TEST_CARDS 强切 OpenGL，因为 `SaveScreenshot` 仅支持 OpenGL）

<!-- layer:L4 -->
## L4: 启动时 _chdir 到项目根目录

> 来源：2026-06-26，SceneGalleryScene Hero/Grid 缩略图全部黑色。stbi_load 用相对路径找不到文件。

**根因**：exe 在 `build/bin/Release/`，缩略图路径写的是 `assets/images/00_grayscale_landscape.jpg`，依赖 CWD 恰好是项目根。

**修复**：`main()` 入口通过 exe 路径反查包含 `assets/images/00_grayscale_landscape.jpg` 的目录并 `_chdir()`：

```cpp
for (const char* up : {"../../..", "../..", "..", "."}) {
    if (fopen((exeDir + "/" + up + "/assets/images/00_grayscale_landscape.jpg").c_str(), "rb")) {
        _chdir(exeDir + "/" + up); break;
    }
}
```

**通用原则**：桌面应用不应假设用户从哪个目录启动，入口处主动设置工作目录。类似方案适用于任何需要从代码读取相对路径资源的桌面程序。
