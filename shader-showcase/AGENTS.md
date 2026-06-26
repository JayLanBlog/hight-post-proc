# ShaderShowcase — 项目技术认知

> 更新于 2026-06-26 · C++17 / CMake / GLFW / ImGui / OpenGL 4.6+Vulkan

---

## 1. 项目概览

| 项 | 值 |
|---|-----|
| 名称 | ShaderShowcase |
| 类型 | 桌面 GUI 应用（Shader 效果展示器） |
| 语言 | C++17 |
| 构建系统 | CMake 3.20+ |
| 窗口库 | GLFW 3.4 |
| UI 库 | ImGui (docking branch) |
| 渲染后端 | OpenGL 4.6 / Vulkan (可选) |
| SPIR-V 编译器 | glslangValidator (Vulkan SDK) |
| 平台 | Windows (MSVC) |

---

## 2. 目录结构

```
shader-showcase/
├── src/
│   ├── app/
│   │   ├── Application.cpp/h       # 主循环、渲染后端切换、截图
│   │   ├── CoverFlowScene.cpp/h    # 全屏 Shader 轮播页（浮动 UI）
│   │   ├── CoverFlowState.h        # 状态传递结构体
│   │   ├── EffectDetailScene.cpp/h # 效果详情页（参数调节）
│   │   ├── LanguageManager.cpp/h   # 中/英文名称和描述表
│   │   ├── PageManager.cpp/h       # 页面管理器
│   │   ├── Scene.h                 # Scene 抽象基类
│   │   ├── SceneGalleryScene.cpp/h # 首页陈列页（Hero + Grid）
│   │   └── SceneRegistry.cpp/h     # 场景注册表
│   ├── render/
│   │   ├── IRenderBackend.h        # 渲染后端抽象接口
│   │   ├── BackendType.h           # OpenGL/Vulkan 枚举
│   │   ├── OpenGLBackend.cpp/h     # OpenGL 4.6 后端
│   │   ├── VulkanBackend.cpp/h     # Vulkan 后端
│   │   ├── FullscreenQuad.cpp/h    # 全屏四边形 VAO
│   │   └── gl_core_46.cpp/h        # OpenGL 4.6 扩展加载
│   ├── shader/
│   │   ├── EffectMetadata.cpp/h    # effect.json 解析 + EffectCard 结构
│   │   └── ShaderLoader.cpp/h      # SPIR-V 加载
│   ├── ui/
│   │   ├── DebugPanel.cpp/h        # ImGui 参数面板
│   │   └── PerformancePanel.cpp/h  # 性能面板 (FPS)
│   ├── input/
│   │   ├── ScreenCapture.cpp/h     # Windows DXGI 屏幕捕获
│   │   └── VideoPlayer.cpp/h       # FFmpeg 视频播放
│   ├── main.cpp                    # 入口 + 场景注册
│   └── stb_impl.cpp                # stb_image 编译单元
├── shaders/
│   ├── CMakeLists.txt              # SPIR-V 编译规则
│   ├── common/
│   │   ├── fullscreen.vert         # 全屏 quad (OpenGL VAO)
│   │   └── fullscreen_vk.vert      # 全屏 quad (Vulkan VertexIndex)
│   └── effects/                    # 35 个效果
│       └── {id}/
│           ├── {id}.frag           # GLSL 450/460 源码
│           ├── {id}.frag.spv       # SPIR-V 编译产物
│           └── effect.json         # 元数据 (中英文名称/分类/参数)
├── assets/
│   ├── images/                     # 18 张测试图片 (00~17.jpg)
│   ├── videos/                     # 17 个演示视频 (00~17.mp4)
│   └── *.jpg                       # 通用测试图 (abstract/nature/portrait)
├── external/
│   ├── imgui/                      # ImGui docking
│   └── stb/                        # stb_image.h
├── docs/superpowers/
│   ├── specs/                      # 设计文档
│   └── plans/                      # 实施计划
├── CMakeLists.txt
└── AGENTS.md
```

---

## 3. 架构概览

```
main.cpp
  └─ Application (主循环)
       ├─ GLFW 窗口 (新建/退役，支持 Vulkan↔OpenGL 切换)
       ├─ 渲染后端 (OpenGL / Vulkan)
       │    ├─ IRenderBackend (抽象接口)
       │    ├─ OpenGLBackend
       │    │    ├─ std140 UBO "Params" (binding=1, 32B: 6×float + vec2 + float + float)
       │    │    └─ Fallback uniform path: uParamFloat0~uParamFloat5 + uResolution/uTime/uFrameCount
       │    └─ VulkanBackend
       │         └─ push_constant 风格 (原生 SPIR-V push_constant)
       └─ Scene 系统
            ├─ SceneGalleryScene  (首页 — Hero 大卡 + Grid 网格)
            ├─ CoverFlowScene     (全屏轮播 — 当前开发焦点)
            └─ EffectDetailScene  (详情页 — 参数调节 + 对比)
```

**场景切换**: `Scene::OnUpdate` 返回 `WantsReturn/WantsExit` → `Application` 调度 `SetScene()`。

---

## 4. 35 个 Shader 效果

### 4.1 原有经典效果 (18 个)

| # | ID | 中文名 | 分类 | 动态 |
|---|-----|--------|------|:---:|
| 1 | simple_test | 灰度测试 | 色彩调整 | |
| 2 | bloom | 泛光 | 模糊效果 | |
| 3 | blur | 高斯模糊 | 模糊效果 | |
| 4 | sharpen | 锐化 | 图像处理 | |
| 5 | edge_detect | 边缘检测 | 边缘检测 | |
| 6 | emboss | 浮雕 | 边缘检测 | |
| 7 | pixelate | 像素化 | 像素化效果 | |
| 8 | vignette | 暗角 | 暗角效果 | |
| 9 | chromatic | 色差 | 色彩调整 | |
| 10 | color_grade | 调色 | 色彩调整 | |
| 11 | noise | 噪声生成器 | 故障效果 | ✓ |
| 12 | kaleidoscope | 万花筒 | 图像处理 | ✓ |
| 13 | glitch | 故障艺术 | 故障效果 | ✓ |
| 14 | toon | 卡通着色 | 色彩调整 | |
| 15 | vhs | VHS复古 | 故障效果 | ✓ |
| 16 | crt | CRT显示器 | 故障效果 | ✓ |
| 17 | water_ripple | 水波纹 | 图像处理 | ✓ |
| 18 | lens_distort | 镜头畸变 | 图像处理 | |

### 4.2 XPL Glitch 移植效果 (17 个)

| # | ID | 中文名 | 参数数 | 动态 |
|---|-----|--------|:---:|:---:|
| 19 | xpl_glitch_screen_jump | 屏幕跳跃 | 1 | ✓ |
| 20 | xpl_glitch_screen_shake | 屏幕震动 | 1 | ✓ |
| 21 | xpl_glitch_scan_line_jitter | 扫描线抖动 | 2 | ✓ |
| 22 | xpl_glitch_rgb_split_v4 | RGB分离 V4 | 2 | ✓ |
| 23 | xpl_glitch_rgb_split_v2 | RGB分离 V2 | 2 | ✓ |
| 24 | xpl_glitch_analog_noise | 模拟信号噪声 | 3 | ✓ |
| 25 | xpl_glitch_image_block_v3 | 画面块错位 V3 | 2 | ✓ |
| 26 | xpl_glitch_image_block_v4 | 画面块错位 V4 | 4 | ✓ |
| 27 | xpl_glitch_tile_jitter | 瓦片抖动 | 4 | ✓ |
| 28 | xpl_glitch_rgb_split_v3 | RGB分离 V3 | 3 | ✓ |
| 29 | xpl_glitch_digital_stripe | 数字条纹 | 5 | ✓ |
| 30 | xpl_glitch_image_block_v2 | 画面块错位 V2 | 7 | ✓ |
| 31 | xpl_glitch_image_block_v1 | 画面块错位 V1 | 10 | ✓ |
| 32 | xpl_glitch_rgb_split_v1 | RGB分离 V1 | 6 | ✓ |
| 33 | xpl_glitch_rgb_split_v5 | RGB分离 V5 | 2 | ✓ |
| 34 | xpl_glitch_line_block | 行块错位 | 6 | ✓ |
| 35 | xpl_glitch_wave_jitter | 波浪抖动 | 4 | ✓ |

**所有 XPL 效果均为动态**（依赖 uTime），归类于 `Glitch Effects`。

---

## 5. Shader 统一参数规范 (CRITICAL)

### 5.1 所有效果使用 std140 UBO 布局

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 fC;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 参数 0
    float uParamFloat1;   // 参数 1
    float uParamFloat2;   // 参数 2
    float uParamFloat3;   // 参数 3
    float uParamFloat4;   // 参数 4
    float uParamFloat5;   // 参数 5
    vec2 uResolution;     // 视口分辨率 (offset 24)
    float uTime;          // 累计时间 (offset 32)
    float uFrameCount;    // 帧计数 (offset 36)
};
```

**关键规则**：
- 即使参数不足 6 个，也必须用 `_pad0.._padN` 补齐到 6 个 float
- `uResolution` 必须对齐到 byte 24，`uTime` 到 byte 32
- Vulkan 后端使用 `push_constant`（由 SPIR-V 编译器自动处理）

### 5.2 effect.json 格式

```json
{
  "name": "中文名称",
  "name_cn": "中文名称",
  "category": "Glitch Effects",
  "category_cn": "故障效果",
  "description": "英文描述",
  "description_cn": "中文描述",
  "params": [
    {
      "name": "uParamFloat0",
      "label": "参数标签",
      "label_cn": "中文标签",
      "type": "float",
      "min": 0, "max": 1,
      "default": 0.5,
      "ui": "slider"
    }
  ]
}
```

---

## 6. 核心数据流

```
输入纹理 (m_inputTex / m_inputTexCache[index])
    │
    ▼
IRenderBackend::DrawFullscreenQuad(vert, frag, ShaderParams)
    │  ShaderParams: inputTextures[], uniformFloats[], uniformInts[],
    │                time, frameCount, viewportW, viewportH
    │
    ├── BeginRenderToTexture(FBO) → 渲染 → EndRenderToTexture()
    │    └─ GetImTextureID() → ImGui::Image() 显示 (UV 翻转 0,1→1,0)
    │
    └── 直接渲染到主 framebuffer (EffectDetailScene 非对比模式)
```

**参数传递**：`CoverFlowScene::OnRender()` 从 `m_cards[i].params[]` 读取默认值构建 `uniformFloats[]`，通过 `DrawFullscreenQuad()` 写入 Params UBO。

---

## 7. 关键设计

### 7.1 左侧竖栏布局 (CoverFlowScene)
- 所有浮动文字左对齐 `w*0.04`，分层排列在左下角
- 效果名称 (h*0.78, 大字号 28pt) → 描述 (单行截断) → 页码点 → 页码数字 → 帮助文字
- 左右箭头在屏幕右边缘 `h*0.78` 同高度

### 7.2 Dynamic/Static 隔离
- `m_dynamicIndices` / `m_staticIndices` 两个索引池
- 所有 XPL Glitch 效果在动态池
- AUTO_TEST_CARDS 模式下绕过隔离，显示全部卡片

### 7.3 阴影文字
`fgShadowText` lambda：4 层黑色对角偏移 (±2,±2) + 垂直偏移 (0,+3) + 前景色

### 7.4 自动化测试
- `AUTO_TEST_CARDS=1`：进入 CoverFlowScene，遍历 35 张卡片截图到 `screenshots/`
- 测试脚本 `tools/test_xpl_glitch.py` 验证文件完整性、JSON 结构、CARD 注册
- 构建输出 `build_and_screenshot.bat`

---

## 8. 编译与运行

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/bin/Release/ShaderShowcase.exe
```

**自动化截图**：`$env:AUTO_TEST_CARDS='1'; .\build\bin\Release\ShaderShowcase.exe`

---

## 9. 当前开发状态

- **35 效果全部就绪**（18 原有 + 17 XPL Glitch）
- **左侧竖栏 UI** 已应用
- **中英文双语** 通过 LanguageManager 硬编码表驱动
- **OpenGL 后端为主**；Vulkan 为可选扩展
- 所有 XPL shader 使用 std140 UBO（非 push_constant），确保 OpenGL 兼容
