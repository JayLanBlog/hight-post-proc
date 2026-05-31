# 🎨 Shader Showcase — 实时后处理着色器特效展示

**Shader Showcase** 是一个基于 C++ 的桌面应用程序，以 **CoverFlow 3D 卡片翻转流** 交互界面展示 **18 种图像后处理着色器特效**。支持加载图片/视频素材，实时应用特效并交互调参。

---

## ✨ 功能特性

- **18 种后处理特效** — 覆盖泛光、模糊、锐化、边缘检测、浮雕、像素化、暗角、色差、调色、噪声、万花筒、故障艺术、卡通着色、VHS复古、CRT、水波纹、镜头畸变等
- **CoverFlow 3D 交互** — 鼠标拖拽浏览特效卡片，点击卡片进入详情页
- **实时参数调节** — 每个特效支持 2-4 个可调参数（滑块/开关），效果即时可见
- **双后端渲染** — 同时支持 **OpenGL 4.6** 和 **Vulkan 1.2**，运行时一键切换
- **统一 SPIR-V 着色器** — 两个后端共用同一套预编译 SPIR-V 着色器
- **中英双语 UI** — 界面支持中文/英文实时切换
- **视频素材支持** — 可加载 MP4 视频作为特效输入源
- **截图保存** — 按 F12 或 Ctrl+S 可保存当前画面为 PNG
- **自动化测试** — 支持无人值守自动轮播所有特效并截图

---

## 📸 界面预览

### CoverFlow 主界面

![CoverFlow 主界面](test_screenshots/01_startup.png)

*3D CoverFlow 卡片流，鼠标拖拽浏览 18 种特效*

### 特效详情页

![特效详情页](test_screenshots/08_detail_opened.png)

*点击卡片进入详情页，左侧原始素材，右侧实时特效渲染，底部可调参数*

### 参数调试面板

![参数面板](test_screenshots/10_debug_panel.png)

*每个特效提供独立参数面板，滑块实时调节效果*

---

## 🎯 特效一览

### 🧪 简单 (Simple)

| 特效 | 说明 |
|------|------|
| **灰度测试** (Grayscale) | 基础灰度转换，用于测试框架 |

### 💡 光照 (Lighting)

| 特效 | 说明 |
|------|------|
| **泛光** (Bloom) | 提取高亮区域叠加模糊，产生梦幻光晕效果 |

| ![Bloom](screenshots/detail_01.png) |
|:---:|
| *泛光效果 — 城市夜景* |

### 🔧 滤镜 (Filter)

| 特效 | 说明 |
|------|------|
| **高斯模糊** (Gaussian Blur) | 可调模糊半径的高斯卷积 |
| **锐化** (Sharpen) | 提取边缘信息增强画面清晰度 |
| **边缘检测** (Edge Detection) | Sobel 算子边缘提取，可调阈值 |

### 🎨 风格化 (Stylize)

| 特效 | 说明 |
|------|------|
| **浮雕** (Emboss) | 金属浮雕效果的定向光照模拟 |
| **像素化** (Pixelate) | 可调控马赛克块大小的像素风格 |
| **故障艺术** (Glitch Art) | 随机块位移和色彩撕裂的数字故障效果 |
| **卡通着色** (Toon Shading) | 色阶量化 + 边缘描边的赛璐珞风格 |

### 🌈 色彩 (Color)

| 特效 | 说明 |
|------|------|
| **暗角** (Vignette) | 可调强度和范围的四角渐变暗化 |
| **调色** (Color Grading) | LUT 色彩查找表，专业级电影调色 |

### 🔄 扭曲 (Distort)

| 特效 | 说明 |
|------|------|
| **色差** (Chromatic Aberration) | RGB 通道分离偏移，模拟镜头色差 |
| **万花筒** (Kaleidoscope) | 可调扇区数和旋转角度的径向对称 |
| **水波纹** (Water Ripple) | 基于法线贴图的水波位移动画 |
| **镜头畸变** (Lens Distortion) | 桶形/枕形畸变校正和模拟 |

### 🧬 程序化 (Procedural)

| 特效 | 说明 |
|------|------|
| **噪声生成** (Noise) | 可调频率和振幅的 Perlin 噪声纹理 |

### 📼 复古 (Retro)

| 特效 | 说明 |
|------|------|
| **VHS 复古** (VHS Retro) | 模拟 VHS 磁带播放的噪点、扫线和颜色偏移 |
| **CRT 显示器** (CRT Monitor) | CRT 荧幕的扫描线、RGB 荧光点和边缘扭曲 |

---

## 🛠 技术架构

```
shader-showcase/
├── src/
│   ├── app/                     # 应用层
│   │   ├── Application.cpp/h    # 主循环、场景管理、后端切换
│   │   ├── CoverFlowScene.cpp/h # CoverFlow 3D 交互场景
│   │   ├── EffectDetailScene    # 特效详情页场景
│   │   └── LanguageManager      # 中英双语管理
│   ├── render/                  # 渲染后端
│   │   ├── IRenderBackend.h     # 统一抽象接口
│   │   ├── OpenGLBackend        # OpenGL 4.6 实现 (GLAD + SPIR-V)
│   │   └── VulkanBackend        # Vulkan 1.2 实现 (完整管线)
│   ├── shader/                  # 着色器管理
│   │   ├── ShaderLoader.cpp     # SPIR-V 加载器
│   │   └── EffectMetadata.cpp   # effect.json 元数据解析
│   ├── input/                   # 输入源
│   │   ├── VideoPlayer.cpp      # FFmpeg 视频解码
│   │   └── ScreenCapture.cpp    # D3D11 桌面捕获
│   └── ui/                      # UI 组件
│       └── PerformancePanel     # FPS/性能显示面板
├── shaders/
│   ├── common/                  # 公共顶点着色器
│   └── effects/<name>/          # 各特效着色器 + effect.json
├── assets/
│   ├── images/                  # 按特效命名的测试图片 (18张)
│   └── videos/                  # 测试视频素材 (18个)
├── external/
│   ├── imgui/                   # ImGui docking 分支
│   └── stb/                     # stb_image.h
├── test/                        # 自动化测试 (pytest)
├── test_screenshots/            # 测试截图
└── docs/                        # 设计文档
```

### 技术栈

| 技术 | 说明 |
|------|------|
| **语言** | C++17 (严格模式) |
| **窗口管理** | GLFW 3.4 |
| **渲染后端** | OpenGL 4.6 / Vulkan 1.2 (运行时切换) |
| **着色器** | GLSL → SPIR-V (glslangValidator) |
| **GUI** | ImGui (docking 分支) |
| **图片加载** | stb_image |
| **视频解码** | FFmpeg (子进程管道) |
| **构建系统** | CMake 3.20+ (Visual Studio 2022) |

### 双后端动态切换

程序支持在 OpenGL 和 Vulkan 之间**实时热切换**（Ctrl+1 / Ctrl+2），无需重启：

- **OpenGL 4.6** — 通过 `ARB_gl_spirv` 扩展直接加载 SPIR-V，兼容性好
- **Vulkan 1.2** — 完整的 DescriptorSet/Pipeline/Swapchain 管理，性能更优

两个后端共用同一套 SPIR-V 着色器，确保渲染结果一致。

---

## 📦 构建指南

### 依赖要求

- **Visual Studio 2022** (含 C++ 桌面开发负载)
- **CMake** 3.20+ 
- **Vulkan SDK** 1.3+ (可选，仅 Vulkan 后端需要)
- **Python 3.10+** (可选，自动化测试用)
- **FFmpeg** (可选，视频播放功能需要)

### 准备外部依赖

克隆项目后，需手动放置以下依赖：

```powershell
# 1. ImGui (docking 分支)
cd shader-showcase/external
git clone -b docking https://github.com/ocornut/imgui.git

# 2. stb_image.h
mkdir shader-showcase/external/stb
# 下载 stb_image.h 放至 external/stb/stb_image.h
# https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

### 构建步骤

```powershell
# 配置 (Visual Studio 2022)
cd shader-showcase
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译
cmake --build . --config Release

# 运行
.\bin\Release\ShaderShowcase.exe
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `USE_OPENGL` | ON | 启用 OpenGL 后端 |
| `USE_VULKAN` | ON | 启用 Vulkan 后端 (需要 Vulkan SDK) |

---

## ⌨ 操作指南

### 键盘快捷键

| 按键 | 功能 |
|------|------|
| `←` / `→` | 左右切换特效卡片 |
| `Enter` | 打开当前选中的特效详情 |
| `Esc` | 从详情页返回 CoverFlow 主界面 |
| `F1` ~ `F8` | 快速选择第 1-8 个特效 |
| `F9` | 切换渲染后端 (OpenGL ↔ Vulkan) |
| `F12` / `Ctrl+S` | 保存当前画面截图 |
| `Ctrl+O` | 加载外部图片文件 |
| `Ctrl+V` | 切换到视频素材输入 |
| `Ctrl+←` / `Ctrl+→` | 切换输入图片 (详情页) |
| `点击封面图按钮` | 切换 UI 语言 (中文 ↔ English) |

### 鼠标操作

- **拖拽卡片** — 左右拖拽浏览 CoverFlow
- **点击卡片** — 打开特效详情页
- **滚轮** — 在详情页缩放查看

---

## 🧪 自动化测试

项目内置自动化测试框架，支持无人值守轮播和截图：

```powershell
# 设置环境变量进入测试模式
$env:AUTO_TEST = "1"            # 自动轮播模式
$env:AUTO_TEST_CARDS = "1"      # 卡片截图模式
$env:AUTO_TEST_DETAILS = "1"    # 详情页截图模式
$env:AUTO_TEST_UI = "1"         # UI 截图模式

# 运行
.\build\bin\Release\ShaderShowcase.exe
```

也可使用 Python 测试框架：

```powershell
cd shader-showcase/test
pip install -r requirements.txt
pytest test_automated.py -v
```

---

## 📄 许可证

MIT License

---

## 🙏 致谢

- [GLFW](https://www.glfw.org/) — 跨平台窗口和输入管理
- [ImGui](https://github.com/ocornut/imgui) — 即时模式 GUI
- [stb](https://github.com/nothings/stb) — 单头文件图片加载库
- [GLAD](https://glad.dav1d.de/) — OpenGL 函数指针加载器
- [Vulkan SDK](https://vulkan.lunarg.com/) — Vulkan API
