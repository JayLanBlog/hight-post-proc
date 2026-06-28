# Shader 效果展示项目 — 设计规格书

> 日期: 2026-05-25 | 状态: 待审核

---

## 1. 项目概述

基于现有 ReShade `.fx` Shader 库（70 个效果文件），构建一个跨后端（OpenGL + Vulkan）的实时 Shader 效果展示应用。用户可以通过 CoverFlow 风格的总览页浏览效果列表，点击进入详情页全屏体验，并通过 ImGui 面板实时调试 Shader 参数。

---

## 2. 核心需求

| 编号 | 需求 | 优先级 |
|------|------|--------|
| R1 | CoverFlow 风格总览页，横向滑动浏览效果卡片，模糊背景模拟预览 | P0 |
| R2 | 每个卡片显示效果标题 + 简短简介 | P0 |
| R3 | 点击卡片进入详情页，全屏渲染 Shader 效果 | P0 |
| R4 | 详情页底部半透明悬浮简介条 | P0 |
| R5 | 按 Tab 键唤出 ImGui 调参面板，实时调节效果参数 | P0 |
| R6 | 同时支持 OpenGL 4.6+ 和 Vulkan 1.2+ 后端，运行时切换 | P0 |
| R7 | SPIR-V 统一 Shader：编写一次 GLSL，编译为 SPIR-V，双端共用 | P0 |
| R8 | 支持三种输入源：内置测试图/视频、用户加载图片/视频、实时屏幕捕获 | P1 |
| R9 | 初期精选 15-20 个核心效果，每类 1-3 个代表性 Shader | P0 |

---

## 3. 精选效果列表（18 个）

### 3.1 Bloom 泛光
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **ArcaneBloom** | ArcaneBloom.fx | 高度可配泛光，多级降采样管线，Dirt纹理、色温、时域滤波 |
| **NeoBloom** | NeoBloom.fx | 新一代泛光，MIP层级卷积模糊，亮丽光晕效果 |

### 3.2 色彩分级
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **LUT** | LUT.fx | Marty McFly 3D色彩查找表，电影级色调映射 |
| **ColorLab** | ColorLab.fx | CIE L\*a\*b\* 色彩空间调整，科学色彩操作 |
| **Technicolor** | PD80_04_Technicolor.fx | 经典 Technicolor 三色带胶片模拟 |
| **Daltonize** | Daltonize.fx | 色盲矫正，LMS色彩空间偏移，辅助无障碍体验 |
| **ColorTemperature** | PD80_04_Color_Temperature.fx | 基于Tanner Helland算法的色温调整 |

### 3.3 色调映射
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **AdaptiveTonemapper** | AdaptiveTonemapper.fx | 自适应曝光色调映射，Reinhard/Filmic/ACES三种算子 |

### 3.4 CRT 复古显示
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **CRT_Lottes** | CRT_Lottes.fx | Timothy Lottes CRT模拟，扫描线、色罩、几何变形 |
| **CRT_Yeetron** | CRT_Yeetron.fx | 复古像素化滤镜，低分辨率CRT风格 |

### 3.5 色差 Chromatic Aberration
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **FlexibleCA** | FlexibleCA.fx | 双模式色差，Translate平移 / Scale缩放，三通道独立控制 |

### 3.6 景深 & 光晕
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **FocalDOF** | FocalDOF.fx | 基于焦点的景深，平滑时域过渡，可配焦深 |
| **HexLensFlare** | HexLensFlare.fx | 六边形镜头光晕，四色渐变，亮度阈值触发 |

### 3.7 胶片 & 锐化
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **FilmGrain** | PD80_06_Film_Grain.fx | Stefan Gustavson算法胶片颗粒，可配置噪声统计参数 |
| **Unsharp** | Unsharp.fx | 经典反锐化掩模，Gaussian模糊基底 |

### 3.8 风格化 & 其他
| 效果 | 源文件 | 简介 |
|------|--------|------|
| **RetroFog** | RetroFog.fx | 复古雾效，深度距离雾 + Bayer抖动量化，Doom风格 |
| **Dither** | Dither.fx | Bayer有序抖动，低位深色彩模拟，量化/相乘模式 |
| **ArtisticVignette** | ArtisticVignette.fx | 艺术晕影，8种混合模式，5种形状 |

---

## 4. 架构设计

### 4.1 分层架构

```
┌──────────────────────────────────────────┐
│           应用层 (Application)            │
│  ┌──────────┐  ┌──────────┐  ┌────────┐  │
│  │ CoverFlow│  │ 详情展示  │  │ 窗口管理│  │
│  │ 总览场景  │  │ 效果场景  │  │ Input   │  │
│  └──────────┘  └──────────┘  └────────┘  │
├──────────────────────────────────────────┤
│        渲染抽象层 (Render Backend)        │
│  ┌──────────────────┐  ┌──────────────┐  │
│  │  OpenGL Backend   │  │ Vulkan Backend│  │
│  │  GL 4.6 + SPIR-V │  │ Vk 1.2 +SPIR-V│  │
│  └────────┬─────────┘  └──────┬───────┘  │
│           └─────────┬──────────┘          │
│                     ▼                      │
│           ┌──────────────────┐            │
│           │  SPIR-V Shader   │            │
│           │   统一二进制格式   │            │
│           └──────────────────┘            │
├──────────────────────────────────────────┤
│         资源层 (Resource)                  │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌─────────┐ │
│  │内置资源│ │用户加载│ │屏幕捕获│ │纹理管理  │ │
│  └──────┘ └──────┘ └──────┘ └─────────┘ │
├──────────────────────────────────────────┤
│         UI 层 (ImGui)                     │
│  ┌──────────────────────────────────┐    │
│  │  调参面板 (Slider/Color/Combo...)  │    │
│  └──────────────────────────────────┘    │
└──────────────────────────────────────────┘
```

### 4.2 渲染后端接口 (IRenderBackend)

```cpp
class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;
    
    // 生命周期
    virtual bool Init(GLFWwindow* window) = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Resize(int w, int h) = 0;
    
    // Shader
    virtual ShaderHandle CreateShader(const std::vector<uint8_t>& spirv) = 0;
    virtual void DestroyShader(ShaderHandle) = 0;
    
    // 纹理
    virtual TextureHandle CreateTexture(int w, int h, const void* data, TextureFormat fmt) = 0;
    virtual void DestroyTexture(TextureHandle) = 0;
    virtual void UpdateTexture(TextureHandle, const void* data) = 0;
    
    // 渲染
    virtual void DrawFullscreenQuad(ShaderHandle, const ShaderParams&) = 0;
    virtual void DrawCoverFlowCards(/* card data */) = 0;
    
    // ImGui
    virtual void ImGuiNewFrame() = 0;
    virtual void ImGuiRender() = 0;
    
    // 查询
    virtual BackendType GetType() const = 0;  // OpenGL or Vulkan
    virtual const char* GetName() const = 0;
};

// 两种实现
class OpenGLBackend : public IRenderBackend { /* ... */ };
class VulkanBackend : public IRenderBackend { /* ... */ };
```

### 4.3 场景管理 (Scene)

```
App
 ├── CoverFlowScene     (总览页)
 │    ├── CardList       (效果卡片列表，含标题/简介/类别/SPIR-V路径)
 │    ├── CoverFlowRenderer (横向3D卡片排列渲染)
 │    └── BlurBackground (模糊背景) [可选优化]
 │
 └── EffectDetailScene  (详情页)
      ├── ShaderRunner   (加载SPIR-V、设置uniform、全屏quad渲染)
      ├── InputTexture   (当前输入源纹理)
      └── DebugPanel     (ImGui调参面板，动态生成控件)
```

### 4.4 CoverFlow 渲染方案

CoverFlow 使用 3D 透视变换在 GPU 端绘制：

- 每张卡片是一个带纹理的矩形面片（Quad）
- 当前选中卡片位于屏幕中央，scale=1.0，完全清晰
- 左右相邻卡片依次缩小并向外旋转，带透视投影
- 卡片纹理为目标 Shader 效果在当前测试图上的静态截图（预生成缩略图）
- 背景使用当前选中卡片的截图做高斯模糊
- 滑动动画：smoothstep 缓动插值 position/scale/opacity

### 4.5 数据流

```
[内置资源] ──┐
[用户文件] ──┼──→ InputTexture ──→ SPIR-V Shader ──→ FS Quad ──→ Screen
[屏幕捕获] ──┘                                    ▲
                                                   │
                            [ImGui DebugPanel] ──→ Uniform Params
```

### 4.6 输入源管理

| 输入源 | 实现方式 | 格式支持 |
|--------|----------|----------|
| 内置资源 | CMake 打包进可执行文件或运行时从 assets/ 加载 | PNG/JPG/MP4 |
| 用户加载 | GLFW 拖放回调 + 文件对话框 (nativefiledialog) | PNG/JPG/BMP/MP4 |
| 屏幕捕获 | Windows: DXGI Desktop Duplication API → 共享纹理 | 桌面画面 |

视频解码使用 FFmpeg 或 stb_vorbis + 内置简单解码器。

---

## 5. SPIR-V 编译管线

```
┌──────────────┐    glslangValidator     ┌──────────────┐
│ effect.frag  │ ──────────────────────→ │ effect.frag.spv │
│ effect.vert  │ ──────────────────────→ │ effect.vert.spv │
└──────────────┘                         └──────────────┘
                                                  │
                    ┌─────────────────────────────┘
                    ▼
           ┌───────────────┐
           │ OpenGL 后端    │──── glShaderBinary() ────→ GL Program
           │ Vulkan 后端    │──── vkCreateShaderModule() → VkPipeline
           └───────────────┘
```

- 每个效果由一个顶点 Shader（全屏三角形/Quad）和一个片段 Shader（后期处理逻辑）组成
- 顶点 Shader 所有效果共用（全屏 Pass-through）
- 片段 Shader 从 ReShade `.fx` 手动移植到 GLSL 460，再编译为 SPIR-V
- SPIR-V 文件在 CMake 构建阶段预编译，运行时直接加载二进制

---

## 6. Shader 移植策略

ReShade `.fx` → 标准 GLSL 460 的转换要点：

1. **Uniform 声明**：`.fx` 中的 `uniform float var <ui_type="slider"; ui_min=0; ui_max=1;>;` → GLSL `layout(location=N) uniform float var;` + 元数据 JSON
2. **纹理采样**：`tex2D(sampler, uv)` → `texture(sampler, uv)`
3. **ReShade 内置宏**：`BUFFER_WIDTH`、`BUFFER_HEIGHT`、`BUFFER_COLOR_BIT_DEPTH` 等 → 运行时 uniform 传入
4. **Pass 管线**：多 Pass 效果（如 ArcaneBloom 的下采样 Pass）→ 通过 FBO/RenderPass 链实现
5. **参数元数据**：从 `.fx` 的 ui_label/ui_type/ui_min/ui_max/ui_tooltip 注释提取为 JSON，供 ImGui 动态生成控件

每个效果附带一个 `effect.json` 元数据文件：
```json
{
  "name": "ArcaneBloom",
  "category": "Bloom",
  "description": "高度可配置的泛光效果...",
  "passes": 4,
  "params": [
    { "name": "uIntensity", "type": "slider", "min": 0, "max": 2, "default": 1.0, "label": "强度" },
    { "name": "uThreshold", "type": "slider", "min": 0, "max": 1, "default": 0.5, "label": "阈值" }
  ]
}
```

---

## 7. 平台 & 依赖

| 依赖 | 用途 | 版本要求 |
|------|------|----------|
| GLFW | 窗口管理、输入处理 | 3.3+ |
| OpenGL | 渲染后端 A | 4.6+ (需 SPIR-V 扩展) |
| Vulkan | 渲染后端 B | 1.2+ |
| Dear ImGui | 调参面板 UI | docking 分支 |
| glslangValidator | GLSL → SPIR-V 编译 | 最新 |
| stb_image | 图片加载 | 最新 |
| FFmpeg | 视频解码 | 4.0+ (可选) |
| nativefiledialog | 文件选择对话框 | 最新 |

**构建系统**：CMake 3.20+  
**平台**：Windows（主），结构预留 Linux/macOS 扩展  
**编译器**：MSVC 2022 / Clang 15+

---

## 8. 项目目录结构

```
shader-showcase/
├── CMakeLists.txt
├── assets/
│   ├── test-images/          # 内置测试图片
│   ├── test-videos/          # 内置测试视频
│   └── thumbnails/           # CoverFlow 缩略图（预生成）
├── shaders/
│   ├── common/
│   │   └── fullscreen.vert   # 共用顶点Shader
│   ├── effects/              # 18个效果的片段Shader
│   │   ├── arcane_bloom/
│   │   │   ├── arcane_bloom.frag
│   │   │   ├── arcane_bloom.json     # 参数元数据
│   │   │   └── arcane_bloom.frag.spv # 编译产物
│   │   └── ...
│   └── ui/                   # CoverFlow UI Shader
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── Application.h/cpp         # 主应用类
│   │   ├── CoverFlowScene.h/cpp       # CoverFlow场景
│   │   └── EffectDetailScene.h/cpp    # 详情场景
│   ├── render/
│   │   ├── IRenderBackend.h           # 渲染后端接口
│   │   ├── OpenGLBackend.h/cpp        # OpenGL实现
│   │   ├── VulkanBackend.h/cpp        # Vulkan实现
│   │   ├── CoverFlowRenderer.h/cpp    # CoverFlow专用渲染器
│   │   └── FullscreenQuad.h/cpp       # 全屏Quad/三角形
│   ├── shader/
│   │   ├── ShaderManager.h/cpp        # Shader加载/SRIR-V管理
│   │   ├── EffectMetadata.h/cpp       # 效果元数据解析
│   │   └── UniformBinder.h/cpp        # Uniform参数绑定
│   ├── input/
│   │   ├── InputSource.h              # 输入源抽象
│   │   ├── BuiltinInput.h/cpp         # 内置图片/视频
│   │   ├── FileInput.h/cpp            # 用户文件加载
│   │   └── ScreenCapture.h/cpp        # 屏幕捕获(DXGI)
│   └── ui/
│       └── DebugPanel.h/cpp           # ImGui 调参面板
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-05-25-shader-showcase-design.md
```

---

## 9. 核心数据结构

```cpp
// 效果卡片（总览页用）
struct EffectCard {
    std::string id;           // 唯一标识
    std::string name;         // 显示名称
    std::string category;     // 分类: Bloom/Color/CRT/Stylized...
    std::string description;  // 简介
    std::string thumbnailPath;// 缩略图路径
    std::string spirvPath;    // SPIR-V 文件路径
};

// Shader参数（调参面板用）
struct ShaderParam {
    std::string name;         // uniform名称
    std::string label;        // UI显示名
    ParamType type;           // float/int/bool/float2/float3/float4/color
    float minVal, maxVal;     // 取值范围
    float defaultVal[4];     // 默认值
    std::string uiType;       // slider/drag/combo/color
    std::vector<std::string> comboOptions; // combo选项
};

// 运行时uniform绑定
struct UniformBinding {
    int location;             // GLSL location
    ParamType type;
    float currentValue[4];
};
```

---

## 10. 页面状态机

```
               启动
                │
                ▼
        ┌──────────────┐
        │  CoverFlow   │  ← 初始状态
        │  总览页       │
        └──────┬───────┘
               │ 点击卡片
               ▼
        ┌──────────────┐      ESC       ┌──────────────┐
        │  详情页       │ ────────────→ │  CoverFlow   │
        │  (全屏效果)   │               │  总览页       │
        │              │               │              │
        │  Tab 切换 ──→ │ 调参面板显隐  │              │
        └──────────────┘               └──────────────┘
```

---

## 11. 未决问题 (待实现阶段解决)

1. 屏幕捕获的跨后端纹理共享（DXGI → OpenGL/Vulkan 互操作）
2. 视频解码器的选择（FFmpeg 较重，是否有更轻量方案）
3. CoverFlow 卡片缩略图是预生成静态截图，还是实时渲染小窗口（性能 vs 动态性权衡）
4. 多 Pass 效果的中间 RenderTarget 管理细节
5. ImGui 与两种后端的集成方式（OpenGL 有官方 binding，Vulkan 也有）

---

## 12. 里程碑

| 阶段 | 内容 | 预期产出 |
|------|------|----------|
| M1: 框架搭建 | GLFW窗口 + 双后端抽象 + 基础全屏Quad + ImGui集成 | 可切换后端，显示空白窗口 + 调参面板 |
| M2: Shader管线 | SPIR-V编译链 + 第一个效果(Simple) + 调试面板 | 可加载一个效果并调参 |
| M3: CoverFlow | CoverFlow渲染 + 卡片UI + 翻页交互 + 场景切换 | 总览页浏览，点击进入详情 |
| M4: 效果扩展 | 18个效果全部移植 + 元数据 + 调参面板 | 所有效果可体验和调试 |
| M5: 输入源 | 内置资源 + 用户加载 + 屏幕捕获 | 三种输入方式可用 |
| M6: 打磨 | 动画优化 + 多后端对比 + 性能调优 | 可发布版本 |
