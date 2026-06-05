# Shader Showcase 源码详解

> 本文档对 Shader Showcase 项目的前三章核心源码进行逐行级深度解析，涵盖程序入口、应用核心框架和封面流场景三大模块。

---

## 目录

- [1. 程序入口 — main.cpp](#1-程序入口--maincpp)
  - [1.1 资源定位 FindAssetDir](#11-资源定位-findassetdir)
  - [1.2 主函数 main](#12-主函数-main)
- [2. 应用核心 — Application](#2-应用核心--application)
  - [2.1 头文件 Application.h（类定义）](#21-头文件-applicationh类定义)
  - [2.2 构造与析构](#22-构造与析构)
  - [2.3 Run — 应用启动入口](#23-run--应用启动入口)
  - [2.4 InitBackend — 后端初始化与窗口管理](#24-initbackend--后端初始化与窗口管理)
  - [2.5 MainLoop — 主循环帧处理](#25-mainloop--主循环帧处理)
  - [2.6 Shutdown — 资源清理](#26-shutdown--资源清理)
  - [2.7 场景管理 SetScene / SwitchBackend](#27-场景管理-setscene--switchbackend)
  - [2.8 回调函数 KeyCallback / DropCallback / FramebufferSizeCallback](#28-回调函数-keycallback--dropcallback--framebuffersizecallback)
- [3. 封面流场景 — CoverFlowScene](#3-封面流场景--coverflowscene)
  - [3.1 头文件 CoverFlowScene.h](#31-头文件-coverflowsceneh)
  - [3.2 卡片注册 RegisterCards（CARD宏）](#32-卡片注册-registercardscard宏)
  - [3.3 场景生命周期 OnEnter / OnExit](#33-场景生命周期-onenter--onexit)
  - [3.4 帧更新 OnUpdate](#34-帧更新-onupdate)
  - [3.5 渲染 OnRender + RenderVisibleThumbnails](#35-渲染-onrender--rendervisiblethumbnails)
  - [3.6 UI 绘制 OnImGui](#36-ui-绘制-onimgui)
  - [3.7 场景跳转 OpenSelectedEffect / GetNextScene](#37-场景跳转-openselectedeffect--getnextscene)
  - [3.8 状态保存与恢复 GetState](#38-状态保存与恢复-getstate)
  - [3.9 辅助功能 LoadImageFromFile / CycleImage / ToggleScreenCapture](#39-辅助功能-loadimagefromfile--cycleimage--togglescreencapture)

---

## 1. 程序入口 — main.cpp

`main.cpp` 是整个 Shader Showcase 程序的入口文件，负责资源目录定位、自动测试模式检测、输入纹理加载与缓存、以及 CoverFlowScene 的创建和初始化。

### 1.1 资源定位 FindAssetDir

```cpp
static std::string FindAssetDir() {
#ifdef _WIN32
    char buf[MAX_PATH];                                    // Windows平台最大路径长度缓冲区
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH); // 获取当前可执行文件的完整路径
    if (len > 0 && len < MAX_PATH) {                      // 确保路径获取成功且未溢出
        std::string exePath(buf, (size_t)len);            // 将C字符串转换为std::string
        auto sl = exePath.find_last_of("\\/");            // 查找最后一个路径分隔符位置
        if (sl != std::string::npos)                      // 如果找到了分隔符
            exePath = exePath.substr(0, sl);              // 截取可执行文件所在的目录路径
        for (const char* rel : {"../../../assets", "../../assets", "../assets", "assets"}) {
            // 依次尝试从远到近的相对路径查找assets目录
            std::string test = exePath + "/" + rel + "/test.jpg"; // 拼接出测试文件路径
            FILE* f = fopen(test.c_str(), "rb");          // 尝试以二进制只读模式打开测试文件
            if (f) { fclose(f); return exePath + "/" + rel; } // 找到则关闭文件并返回完整assets路径
        }
    }
#endif
    return "assets";                                      // 所有尝试失败则回退到当前目录下的assets
}
```

#### 功能说明

`FindAssetDir` 是一个静态辅助函数，用于在运行时自动定位 `assets` 资源目录的绝对路径。它通过获取可执行文件所在位置，然后从近到远逐级向上搜索 `assets` 目录。

#### 实现原理

1. 在 Windows 平台上调用 `GetModuleFileNameA` 获取当前进程的可执行文件完整路径。
2. 通过 `find_last_of("\\/")` 找到最后一个路径分隔符，截取得到可执行文件所在的目录。
3. 使用一个有序的候选路径列表 `{"../../../assets", "../../assets", "../assets", "assets"}`，从最远的相对路径开始，逐个尝试打开 `assets/test.jpg` 文件来验证目录是否存在。
4. 第一个成功打开的路径即为正确的资源目录。

#### 为什么这样实现

- **可执行文件位置不固定**：项目可能从不同的构建目录（如 `build/Release/`、`build/Debug/`）运行，可执行文件与 `assets` 目录的相对距离不同。
- **探测式搜索**：与其硬编码一个固定路径，不如让程序自己"探测"资源目录位置，增强了部署灵活性。
- **跨层级搜索**：支持从可执行文件所在目录向上最多三级查找，覆盖了常见的 CMake 构建目录结构。
- **平台条件编译**：仅 Windows 平台需要此逻辑（Linux/macOS 通常使用固定安装路径或环境变量）。

---

### 1.2 主函数 main

```cpp
int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);                  // 禁用stdout缓冲，确保日志实时输出
    setvbuf(stderr, nullptr, _IONBF, 0);                  // 禁用stderr缓冲，确保错误信息实时输出

    // Check for auto-test mode via environment variable
    const char* autoTestEnv = getenv("AUTO_TEST");         // 读取AUTO_TEST环境变量
    bool autoTest = (autoTestEnv && strcmp(autoTestEnv, "1") == 0); // 当值为"1"时启用自动测试
    if (getenv("AUTO_TEST_DETAILS") && strcmp(getenv("AUTO_TEST_DETAILS"), "1") == 0) {
        autoTest = true;                                   // AUTO_TEST_DETAILS=1也启用自动测试
    }

    Application app;                                       // 创建Application主应用实例（栈分配）

    app.SetFrameCallback([&](float dt) {                  // 设置帧回调（lambda捕获app引用）
        (void)dt;                                         // dt参数未使用，避免编译器警告
        // Re-create scene if it was destroyed (e.g., after backend switch)
        if (app.GetCurrentScene() != nullptr) return;     // 如果场景已存在则直接返回（无需重建）

        IRenderBackend* backend = app.GetBackend();       // 获取当前渲染后端指针
        if (!backend) return;                              // 后端未就绪则返回

        std::string assetDir = FindAssetDir();             // 定位资源目录
        std::string jpgPath  = assetDir + "/test.jpg";    // 默认测试图片路径
        int iw = 0, ih = 0, comp = 0;                     // 图片宽度、高度、通道数变量
        stbi_set_flip_vertically_on_load(true);           // 翻转图片Y轴（OpenGL纹理坐标原点在左下角）
        stbi_uc* imgData = stbi_load(jpgPath.c_str(), &iw, &ih, &comp, 4); // 加载图片，强制4通道RGBA
        stbi_set_flip_vertically_on_load(false);          // 恢复默认不翻转设置
        if (!imgData) {                                    // 图片加载失败处理
            fprintf(stderr, "[main] Cannot load test image: %s\n", jpgPath.c_str());
            return;
        }
        printf("[main] Loaded image: %s (%d x %d)\n", jpgPath.c_str(), iw, ih);

        TextureHandle inputTex = backend->CreateTexture(iw, ih, TextureFormat::RGBA8, imgData);
        // 使用后端API创建GPU纹理，格式RGBA8
        stbi_image_free(imgData);                          // 释放CPU端图片内存

        const char* testImageList[] = {                   // 每个特效对应的测试图片文件名列表
            "00_grayscale_landscape.jpg",   // simple_test (Grayscale)
            "01_bloom_citynight.jpg",       // bloom
            "02_blur_brickwall.jpg",        // blur
            "03_sharpen_architecture.jpg",  // sharpen
            "04_edge_building.jpg",         // edge_detect
            "05_emboss_metal.jpg",          // emboss
            "06_pixelate_portrait.jpg",     // pixelate
            "07_vignette_flower.jpg",       // vignette
            "08_chromatic_leaves.jpg",      // chromatic
            "09_colorgrading_food.jpg",     // color_grade
            "10_noise_sky.jpg",             // noise
            "11_kaleidoscope_mandala.jpg",  // kaleidoscope
            "12_glitch_tech.jpg",           // glitch
            "13_toon_cartoon.jpg",          // toon
            "14_vhs_retro.jpg",             // vhs
            "15_crt_screen.jpg",            // crt
            "16_water_lake.jpg",            // water_ripple
            "17_lens_wideangle.jpg"         // lens_distort
        };
        const int NUM_EFFECTS = (int)(sizeof(testImageList) / sizeof(testImageList[0]));
        // 通过数组大小除以单个元素大小计算特效总数

        std::vector<TextureHandle> inputTexCache;          // 每个特效专用的输入纹理缓存
        inputTexCache.reserve(NUM_EFFECTS);                // 预分配空间避免多次重分配

        // Find test images directory (relative to executable)
        std::string testImageBaseDir;                      // 测试图片基础目录
#ifdef _WIN32
        char buf[MAX_PATH];                                // 路径缓冲区（复用局部变量）
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH); // 获取可执行文件路径
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);        // 转为string
            auto sl = exePath.find_last_of("\\/");        // 查找目录分隔符
            if (sl != std::string::npos) exePath = exePath.substr(0, sl); // 截取目录
            // Try to find assets/images relative to exe (new location)
            for (const char* rel : {"../../../assets/images", "../../assets/images",
                                    "../assets/images", "assets/images",
                                    "../../../screenshots/assets/images", "../../screenshots/assets/images",
                                    "../screenshots/assets/images", "screenshots/assets/images"}) {
                // 尝试多个候选路径（包括screenshots子目录下的备份路径）
                std::string test = exePath + "/" + rel + "/00_grayscale_landscape.jpg";
                FILE* f = fopen(test.c_str(), "rb");      // 尝试打开第一张测试图片
                if (f) { fclose(f); testImageBaseDir = exePath + "/" + rel + "/"; break; }
                // 找到则保存路径并跳出循环
            }
        }
#endif
        if (testImageBaseDir.empty()) {                    // 所有候选路径都失败
            testImageBaseDir = "assets/images/";           // 回退到默认相对路径
        }
        printf("[main] Test images directory: %s\n", testImageBaseDir.c_str());

        // Load and cache input textures for each effect
        for (int i = 0; i < NUM_EFFECTS; i++) {           // 遍历所有特效
            std::string testImgPath = testImageBaseDir + testImageList[i]; // 拼接完整路径
            int tiw = 0, tih = 0, tcomp = 0;              // 图片尺寸和通道变量
            stbi_set_flip_vertically_on_load(true);       // 翻转Y轴适配OpenGL
            stbi_uc* testImgData = stbi_load(testImgPath.c_str(), &tiw, &tih, &tcomp, 4);
            stbi_set_flip_vertically_on_load(false);       // 恢复默认
            TextureHandle effectInputTex = inputTex;        // 默认回退到通用测试图片纹理
            if (testImgData) {                             // 如果专属测试图片加载成功
                printf("[main] Loaded test image for effect %d: %s (%d x %d)\n",
                       i, testImgPath.c_str(), tiw, tih);
                effectInputTex = backend->CreateTexture(tiw, tih, TextureFormat::RGBA8, testImgData);
                // 创建该特效专用的GPU纹理
                stbi_image_free(testImgData);               // 释放CPU端内存
            } else {
                printf("[main] Warning: Cannot load test image %s, using default\n",
                       testImgPath.c_str());
            }
            inputTexCache.push_back(effectInputTex);       // 加入缓存列表
        }

        printf("[main] All %d input textures cached\n", (int)inputTexCache.size());

        auto coverFlow = std::make_unique<CoverFlowScene>(); // 创建封面流场景（unique_ptr管理）
        coverFlow->SetBackend(backend);                    // 注入渲染后端指针
        coverFlow->SetInputTexture(inputTex);              // 设置默认输入纹理
        coverFlow->SetInputTexCache(inputTexCache);        // 设置每个特效的缓存纹理
        coverFlow->SetTestImageBaseDir(testImageBaseDir);   // 设置测试图片目录
        coverFlow->SetApplication(&app);                   // 注入Application指针（用于拖放等）
        coverFlow->AddImageToPool(jpgPath);                 // 添加默认测试图片到图片池
        coverFlow->AddImageToPool(assetDir + "/portrait.jpg"); // 添加人像图片
        coverFlow->AddImageToPool(assetDir + "/nature.jpg");    // 添加自然风景图片
        coverFlow->AddImageToPool(assetDir + "/abstract.jpg");  // 添加抽象图片

        // Add test images to pool for Ctrl+Left/Right cycling
        for (int i = 0; i < NUM_EFFECTS; i++) {           // 将所有特效测试图片也加入图片池
            std::string imgPath = testImageBaseDir + testImageList[i];
            FILE* f = fopen(imgPath.c_str(), "rb");        // 检查文件是否存在
            if (f) {
                fclose(f);
                coverFlow->AddImageToPool(imgPath);         // 存在则加入池中
            }
        }

        // Add test videos to pool
        const char* testVideoList[] = {                    // 测试视频文件名列表
            "00_grayscale.mp4", "01_bloom.mp4", "02_blur.mp4", "03_sharpen.mp4",
            "04_edge.mp4", "05_emboss.mp4", "06_pixelate.mp4", "07_vignette.mp4",
            "08_chromatic.mp4", "09_colorgrade.mp4", "10_noise.mp4", "11_kaleidoscope.mp4",
            "12_glitch.mp4", "13_toon.mp4", "14_vhs.mp4", "15_crt.mp4",
            "16_water.mp4", "17_lens.mp4"
        };
        std::string testVideoBaseDir;                      // 测试视频基础目录
#ifdef _WIN32
        if (len > 0 && len < MAX_PATH) {                   // 复用之前获取的len值
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) exePath = exePath.substr(0, sl);
            for (const char* rel : {"../../../assets/videos", "../../assets/videos",
                                    "../assets/videos", "assets/videos",
                                    "../../../screenshots/assets/videos", "../../screenshots/assets/videos",
                                    "../screenshots/assets/videos", "screenshots/assets/videos"}) {
                // 与图片目录搜索逻辑相同，搜索视频目录
                std::string test = exePath + "/" + rel + "/00_grayscale.mp4";
                FILE* f = fopen(test.c_str(), "rb");
                if (f) { fclose(f); testVideoBaseDir = exePath + "/" + rel + "/"; break; }
            }
        }
#endif
        if (testVideoBaseDir.empty()) {
            testVideoBaseDir = "assets/videos/";           // 回退默认路径
        }
        printf("[main] Test videos directory: %s\n", testVideoBaseDir.c_str());

        for (int i = 0; i < NUM_EFFECTS; i++) {           // 遍历所有特效
            std::string vidPath = testVideoBaseDir + testVideoList[i]; // 拼接视频路径
            FILE* f = fopen(vidPath.c_str(), "rb");        // 检查视频文件是否存在
            if (f) {
                fclose(f);
                coverFlow->AddVideoToPool(vidPath);         // 存在则加入视频池
            }
        }

        if (autoTest) {                                   // 如果启用了自动测试模式
            coverFlow->EnableAutoTest(80);                 // 每张卡片停留80帧后自动切换
        }

        app.SetScene(std::move(coverFlow));                // 将场景转移给Application管理
        printf("[main] CoverFlowScene started (autoTest=%d)\n", autoTest);
    });

    return app.Run(argc, argv);                            // 启动应用主循环
}
```

#### 功能说明

`main` 函数是程序的入口点，完成以下核心任务：

1. **I/O 缓冲设置**：禁用 stdout/stderr 缓冲，确保日志实时输出到控制台。
2. **自动测试检测**：通过环境变量 `AUTO_TEST` 或 `AUTO_TEST_DETAILS` 判断是否进入自动测试模式。
3. **帧回调注册**：通过 lambda 表达式注册一个帧回调，在 Application 的主循环中首次调用时创建 CoverFlowScene。
4. **资源加载**：加载默认测试图片和每个特效的专属测试图片，创建 GPU 纹理并缓存。
5. **场景初始化**：创建 CoverFlowScene，注入后端、纹理、图片池、视频池等资源。
6. **启动主循环**：调用 `app.Run()` 进入应用主循环。

#### 实现原理

- **延迟场景创建**：场景创建逻辑放在 `SetFrameCallback` 中而非 `main` 函数直接调用，是因为 Application 需要先完成 GLFW 和后端初始化后才能创建 GPU 资源。帧回调在主循环的第一帧被调用，此时后端已就绪。
- **纹理缓存策略**：为每个特效预加载一张专属的测试图片纹理（如模糊效果用砖墙图片、色彩分级用食物图片），这样在封面流缩略图和详情页中可以展示每个特效的最佳效果。
- **探测式资源定位**：与 `FindAssetDir` 相同的策略，对图片和视频目录也采用多级候选路径搜索。

#### 为什么这样实现

- **`setvbuf` 禁用缓冲**：在 Windows 上，stdout 默认使用行缓冲或全缓冲，可能导致日志延迟显示。禁用缓冲后，`printf` 输出立即刷新到控制台，便于调试和自动化测试日志捕获。
- **帧回调模式**：这是一种"懒初始化"策略，将场景创建推迟到渲染后端完全就绪之后。如果直接在 `main` 中创建场景，后端尚未初始化，GPU 资源创建会失败。
- **环境变量控制自动测试**：通过环境变量而非命令行参数控制测试模式，方便 CI/CD 管线和脚本化测试，无需修改程序调用方式。

---

## 2. 应用核心 — Application

Application 类是整个程序的框架核心，负责 GLFW 窗口管理、渲染后端生命周期、场景系统管理和主循环驱动。

### 2.1 头文件 Application.h（类定义）

```cpp
#pragma once                                             // 防止头文件重复包含
#include "render/IRenderBackend.h"                       // 渲染后端抽象接口
#include <memory>                                        // 智能指针支持
#include <functional>                                    // std::function支持
#include <string>                                        // std::string支持

struct GLFWwindow;                                       // 前向声明GLFW窗口结构体

class Scene;                                             // 前向声明Scene基类

// Global screenshot request (for auto-test)
// 全局截图请求结构体（用于自动化测试截图）
struct ScreenshotRequest {
    static bool pending;                                // 是否有待处理的截图请求
    static char path[256];                               // 截图保存路径（静态缓冲区）
    static void Request(const char* p) { pending = true; snprintf(path, sizeof(path), "%s", p); }
    // 发起截图请求：设置pending标志并记录路径
    static bool Consume() { if (pending) { pending = false; return true; } return false; }
    // 消费截图请求：检查并清除pending标志，返回是否曾有请求
};

#include "ui/PerformancePanel.h"                         // 性能面板UI组件

class Application {
public:
    Application();                                       // 构造函数
    ~Application();                                      // 析构函数
    int Run(int argc, char* argv[]);                    // 应用启动入口
    void SwitchBackend(BackendType type);                // 切换渲染后端类型
    BackendType GetBackendType() const { return m_backendType; } // 获取当前后端类型
    IRenderBackend* GetBackend() const { return m_backend.get(); } // 获取后端接口指针
    GLFWwindow* GetWindow() const { return m_window; }  // 获取GLFW窗口句柄

    // Scene management（场景管理）
    void SetScene(std::unique_ptr<Scene> scene);        // 设置当前活动场景
    Scene* GetCurrentScene() const { return m_currentScene.get(); } // 获取当前场景指针

    // Frame callback (deprecated in favor of scene system;
    // kept for backward compatibility)
    // 帧回调（已弃用，保留向后兼容）
    using FrameCallback = std::function<void(float dt)>;
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

    // Drag-drop file loading（拖放文件加载）
    /// Returns the path of the most recently dropped file, or empty string.
    /// The caller should clear the pending flag after consuming.
    std::string ConsumeDroppedFile();                   // 消费最近拖放的文件路径

private:
    void InitBackend(BackendType type);                 // 初始化渲染后端
    void MainLoop();                                    // 主循环
    void Shutdown();                                   // 关闭清理

    GLFWwindow* m_window = nullptr;                     // 主GLFW窗口句柄
    GLFWwindow* m_retiredWindow = nullptr;              // 已退役的隐藏窗口（避免GLFW崩溃）
    std::unique_ptr<IRenderBackend> m_backend;         // 渲染后端（智能指针管理）
    BackendType m_backendType = BackendType::Vulkan;   // 当前后端类型（默认Vulkan）
    BackendType m_pendingBackend = BackendType::Vulkan; // 待切换的后端类型
    bool m_pendingBackendSwitch = false;               // 是否有待处理的后端切换
    FrameCallback m_frameCallback;                      // 帧回调函数
    std::unique_ptr<Scene> m_currentScene;              // 当前活动场景
    std::unique_ptr<Scene> m_pendingNextScene;          // 延迟切换的下一个场景
    bool m_running = false;                            // 主循环运行标志

    // Drag-drop state（拖放状态）
    std::string m_droppedFilePath;                      // 最近拖放的文件路径

    // UI panels（UI面板）
    PerformancePanel m_perfPanel;                      // 性能监控面板

    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height); // 窗口尺寸回调
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods); // 键盘回调
    static void DropCallback(GLFWwindow* w, int count, const char** paths); // 拖放回调
};
```

#### 功能说明

`Application.h` 定义了应用框架的核心类结构，包含：

- **ScreenshotRequest**：全局静态截图请求结构体，用于自动化测试中触发截图操作。
- **Application 类**：管理窗口生命周期、渲染后端（OpenGL/Vulkan）、场景切换、输入回调和性能面板。

#### 实现原理

- **前向声明**：`GLFWwindow` 和 `Scene` 使用前向声明而非 `#include`，减少头文件依赖，加快编译速度。
- **智能指针管理**：`m_backend`、`m_currentScene`、`m_pendingNextScene` 均使用 `std::unique_ptr`，确保资源独占所有权和自动释放。
- **延迟切换机制**：`m_pendingBackendSwitch` 和 `m_pendingNextScene` 实现了"延迟到下一帧"的切换策略，避免在 ImGui/Vulkan 渲染过程中销毁 GPU 资源导致崩溃。
- **退役窗口机制**：`m_retiredWindow` 保存从 Vulkan 切换到 OpenGL 时需要废弃的旧窗口，但不立即销毁（避免 NVIDIA 驱动在 Vulkan 占用表面后销毁窗口导致的崩溃）。

#### 为什么这样实现

- **`ScreenshotRequest` 使用静态成员**：截图请求可能从场景内部的任何地方发起（如自动测试逻辑），使用全局静态结构体避免了在场景和 Application 之间传递截图信号的复杂性。
- **`m_retiredWindow` 的存在**：这是 NVIDIA + Windows 平台上的一个已知问题——Vulkan 通过 `vkCreateWin32SurfaceKHR` 占用窗口表面后，如果销毁窗口再创建新窗口给 OpenGL 用，NVIDIA 驱动会崩溃。解决方案是隐藏旧窗口（不销毁），创建一个全新的窗口给 OpenGL 使用。
- **内联简单 getter**：`GetBackendType()`、`GetBackend()`、`GetWindow()` 等简单访问器直接在头文件中内联定义，避免函数调用开销。

---

### 2.2 构造与析构

```cpp
// 静态成员初始化（在Application.cpp中）
bool ScreenshotRequest::pending = false;                 // 截图请求标志初始化为false
char ScreenshotRequest::path[256] = {};                  // 截图路径缓冲区初始化为空

Application::Application() {}                            // 默认构造函数（空实现）

Application::~Application()
{
    Shutdown();                                          // 析构时执行资源清理
}
```

#### 功能说明

- 构造函数为空实现，所有实际初始化推迟到 `Run()` 中进行。
- 析构函数调用 `Shutdown()` 确保即使 `Run()` 因异常提前退出，资源也能被正确释放（RAII 模式）。

#### 实现原理

遵循 RAII（Resource Acquisition Is Initialization）原则：构造时不获取资源，析构时确保释放。实际资源获取在 `Run()` → `InitBackend()` 中完成。

#### 为什么这样实现

- **空构造函数**：避免在栈上创建 `Application` 对象时就触发 GLFW 初始化（GLFW 初始化可能失败，而构造函数不方便返回错误码）。
- **析构调用 Shutdown**：防御性编程，确保任何退出路径（正常退出、异常退出）都能正确清理资源。

---

### 2.3 Run — 应用启动入口

```cpp
int Application::Run(int argc, char* argv[])
{
    (void)argc;                                          // 未使用命令行参数，避免编译器警告
    (void)argv;

    if (!glfwInit())                                     // 初始化GLFW库
    {
        std::cerr << "[Application] FATAL: Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;                             // GLFW初始化失败则立即退出
    }

    InitBackend(m_backendType);                         // 使用默认后端类型（Vulkan）初始化

    if (!m_window || !m_backend)                         // 检查窗口和后端是否创建成功
    {
        std::cerr << "[Application] FATAL: Failed to initialize backend" << std::endl;
        glfwTerminate();                                 // 清理GLFW
        return EXIT_FAILURE;                             // 初始化失败则退出
    }

    m_running = true;                                    // 设置运行标志
    MainLoop();                                          // 进入主循环（阻塞直到退出）
    Shutdown();                                         // 主循环退出后执行清理
    glfwTerminate();                                     // 终止GLFW库

    return EXIT_SUCCESS;                                 // 正常退出
}
```

#### 功能说明

`Run` 是应用的顶层启动函数，按顺序执行：GLFW 初始化 → 后端初始化 → 主循环 → 清理 → 退出。

#### 实现原理

1. `glfwInit()` 初始化 GLFW 库，包括平台特定的窗口系统接口。
2. `InitBackend()` 创建窗口和渲染后端（OpenGL 或 Vulkan）。
3. `MainLoop()` 进入帧驱动的游戏循环，直到窗口关闭或 `m_running` 被设为 false。
4. `Shutdown()` 释放所有资源。
5. `glfwTerminate()` 清理 GLFW 全局状态。

#### 为什么这样实现

- **线性流程**：初始化 → 运行 → 清理的线性结构清晰直观，便于理解生命周期。
- **双重检查**：在 `InitBackend` 之后检查 `m_window` 和 `m_backend`，因为后端初始化可能因缺少驱动支持等原因失败。
- **`glfwTerminate` 调用两次**：`Shutdown()` 中调用一次（清理窗口等），`Run()` 末尾再调用一次（确保 GLFW 全局状态被清理）。`glfwTerminate` 是幂等的，多次调用安全。

---

### 2.4 InitBackend — 后端初始化与窗口管理

```cpp
void Application::InitBackend(BackendType type)
{
    // ---- Destroy old backend resources (but NOT the window) ----------------
    // 销毁旧后端资源（但不销毁窗口）
    if (m_currentScene) {                               // 如果当前有活动场景
        m_currentScene->OnExit();                       // 通知场景退出
        m_currentScene.reset();                         // 释放场景对象
    }

    if (m_backend) {                                    // 如果已有后端实例
        m_backend->ImGuiShutdown();                    // 关闭ImGui的GPU资源
        m_backend->Shutdown();                          // 关闭后端
        m_backend.reset();                               // 释放后端对象
    }

    // Don't destroy ImGui context — the backend's ImGuiShutdown already freed
    // GPU resources. Just reset the font atlas so the next backend can rebuild.
    // 不销毁ImGui上下文——后端的ImGuiShutdown已释放GPU资源。
    // 只需重置字体图集，让下一个后端重建。
    if (ImGui::GetCurrentContext() != nullptr) {       // 如果ImGui上下文存在
        ImGui::GetIO().Fonts->Clear();                  // 清空字体图集
    }

    m_pendingNextScene.reset();                         // 清除待切换的场景
    BackendType oldType = m_backendType;                // 记录旧后端类型
    m_backendType = type;                                // 更新为新后端类型

    // ---- Create/reuse window -----------------------------------------------
    // 创建或复用窗口
    // Always use GLFW_OPENGL_API — Vulkan creates its surface via native Win32
    // API (vkCreateWin32SurfaceKHR), which works on any HWND regardless of
    // GLFW client API.
    // 始终使用GLFW_OPENGL_API——Vulkan通过原生Win32 API创建表面，
    // 这在任何HWND上都有效，与GLFW客户端API无关。
    //
    // When switching FROM Vulkan TO OpenGL, the GL context has been corrupted
    // by Vulkan's ownership of the window surface on NVIDIA+Windows.
    // Retire the old window (hide it, never destroy it) and create a fresh one.
    // 当从Vulkan切换到OpenGL时，GL上下文已被Vulkan对窗口表面的占用所破坏。
    // 退役旧窗口（隐藏但不销毁），创建一个全新的窗口。
    bool needNewWindow = (type == BackendType::OpenGL && oldType == BackendType::Vulkan);
    // 判断是否需要新窗口：仅当从Vulkan切换到OpenGL时
    if (needNewWindow && m_window) {
        glfwHideWindow(m_window);                        // 隐藏旧窗口
        m_retiredWindow = m_window;                      // 保存为退役窗口
        m_window = nullptr;                              // 清空主窗口指针
    }

    if (!m_window) {                                     // 如果没有可用窗口
        glfwDefaultWindowHints();                        // 恢复默认窗口提示
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // 设置客户端API为OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);   // OpenGL主版本号4
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);   // OpenGL次版本号6
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // 前向兼容

        constexpr int W = 1280, H = 720;                 // 默认窗口尺寸1280x720
        m_window = glfwCreateWindow(W, H, LanguageManager::Instance().WindowTitle(), nullptr, nullptr);
        // 创建窗口，标题从语言管理器获取（支持多语言）
        if (!m_window) {                                 // 窗口创建失败
            std::cerr << "[Application] ERROR: Failed to create window\n";
            return;
        }

        glfwSetWindowUserPointer(m_window, this);       // 将Application指针绑定到窗口用户数据
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback); // 注册窗口尺寸回调
        glfwSetKeyCallback(m_window, KeyCallback);       // 注册键盘回调
        glfwSetDropCallback(m_window, DropCallback);     // 注册拖放回调
    }

    // ---- Make GL context current (both backends share the window) ---------
    // 使GL上下文成为当前上下文（两个后端共享窗口）
    glfwMakeContextCurrent(m_window);                    // 绑定GL上下文到当前线程
    if (type == BackendType::OpenGL) {                   // 如果是OpenGL后端
        glfwSwapInterval(1);                             // 开启垂直同步（每帧交换一次）
    }

    // ---- Create backend --------------------------------------------------
    // 创建渲染后端实例
    switch (type) {
    case BackendType::OpenGL:
        m_backend = std::make_unique<OpenGLBackend>();   // 创建OpenGL后端
        break;
    case BackendType::Vulkan:
        m_backend = std::make_unique<VulkanBackend>();  // 创建Vulkan后端
        break;
    }

    if (!m_backend) {                                    // 后端创建失败（编译时未启用）
        std::cerr << "[Application] ERROR: Backend not available\n";
        return;
    }

    if (!m_backend->Init(m_window)) {                   // 初始化后端（创建设备、管线等）
        std::cerr << "[Application] ERROR: Backend initialization failed\n";
        m_backend.reset();                               // 初始化失败则释放后端
        return;
    }

    m_backend->ImGuiInit(m_window);                     // 初始化ImGui的GPU后端

    std::string title = std::string(LanguageManager::Instance().WindowTitle()) + " [" + m_backend->GetName() + "]";
    // 拼接窗口标题：应用名 + [后端名称]
    glfwSetWindowTitle(m_window, title.c_str());        // 更新窗口标题

    std::cout << "[Application] Backend initialized: " << m_backend->GetName() << std::endl;
}
```

#### 功能说明

`InitBackend` 负责完整的后端初始化流程，包括：清理旧后端资源、处理窗口创建/复用、创建渲染后端实例、初始化 ImGui。

#### 实现原理

1. **旧资源清理**：先通知当前场景退出，再关闭 ImGui GPU 资源，最后关闭后端。顺序很重要——场景可能使用后端资源，必须先退出场景。
2. **ImGui 字体图集重置**：`ImGuiShutdown` 释放了 GPU 端的字体纹理，但 ImGui 上下文本身保留。`Clear()` 清空 CPU 端字体数据，让新后端在 `ImGuiInit` 时重建。
3. **窗口复用策略**：
   - 首次创建：直接创建新窗口。
   - OpenGL → Vulkan：复用同一窗口（Vulkan 通过 Win32 API 创建表面，不依赖 GL 上下文）。
   - Vulkan → OpenGL：必须创建新窗口（NVIDIA 驱动问题）。
4. **GLFW_OPENGL_API 始终设置**：即使使用 Vulkan 后端也设置 OpenGL API，因为 Vulkan 通过 `vkCreateWin32SurfaceKHR` 直接从 HWND 创建表面，不需要 GLFW 的 Vulkan API 模式。

#### 为什么这样实现

- **退役窗口而非销毁**：在 NVIDIA + Windows 上，Vulkan 通过 `vkCreateWin32SurfaceKHR` 绑定了窗口表面。如果销毁该窗口，NVIDIA 驱动内部状态可能损坏，导致后续操作崩溃。隐藏窗口但不销毁是最安全的做法。
- **垂直同步仅对 OpenGL 开启**：Vulkan 有自己的帧同步机制（`vkQueuePresentKHR` + swapchain），不需要 GLFW 的 `glfwSwapInterval`。
- **`glfwSetWindowUserPointer`**：GLFW 的回调函数是 C 风格的静态函数，没有 `this` 指针。通过 `glfwSetWindowUserPointer` 将 `Application*` 存储在窗口中，回调中通过 `glfwGetWindowUserPointer` 取回。

---

### 2.5 MainLoop — 主循环帧处理

```cpp
void Application::MainLoop()
{
    auto lastTime = std::chrono::high_resolution_clock::now(); // 记录上一帧时间点

    while (m_running && m_window && !glfwWindowShouldClose(m_window))
    // 循环条件：运行中 且 窗口存在 且 窗口未请求关闭
    {
        try {
        glfwPollEvents();                                // 处理所有待处理的事件（输入、窗口等）

        // Process pending backend switch (deferred to avoid corruption during ImGui rendering)
        // 处理待执行的后端切换（延迟到帧开始，避免ImGui/Vulkan状态损坏）
        if (m_pendingBackendSwitch) {
            m_pendingBackendSwitch = false;               // 清除待切换标志
            printf("[Application] Processing deferred backend switch...\n"); fflush(stdout);
            InitBackend(m_pendingBackend);               // 执行后端切换
        }

        // Compute delta time（计算帧间隔时间）
        auto now      = std::chrono::high_resolution_clock::now(); // 当前时间点
        float dt      = std::chrono::duration<float>(now - lastTime).count(); // 帧间隔（秒）
        lastTime      = now;                              // 更新上一帧时间

        if (!m_backend)                                   // 如果后端不可用
        {
            continue;                                     // 跳过本帧
        }

        m_backend->BeginFrame();                         // 开始帧：Vulkan等待围栏，OpenGL清除缓冲

        // --- Deferred scene transition ---
        // 延迟场景切换
        // Must happen AFTER BeginFrame's vkWaitForFences so GPU has finished
        // with old scene's resources before they're destroyed.
        // 必须在BeginFrame的vkWaitForFences之后执行，确保GPU已完成旧场景资源的使用
        if (m_pendingNextScene) {                        // 如果有待切换的场景
            printf("[Application] Processing deferred scene transition\n");
            if (m_currentScene) {                        // 如果有当前场景
                m_currentScene->OnExit();                 // 通知旧场景退出
                m_currentScene.reset();                   // 释放旧场景
            }
            m_currentScene = std::move(m_pendingNextScene); // 接管新场景
            if (m_currentScene) {
                m_currentScene->OnEnter();                // 通知新场景进入
                printf("[Application] New scene entered OK\n");
            }
        }

        m_backend->ImGuiNewFrame();                       // 开始ImGui新帧

        // --- Scene-driven update ---
        // 场景驱动的更新
        if (m_currentScene)                               // 如果有活动场景
        {
            m_currentScene->OnUpdate(dt);                 // 更新场景逻辑（输入处理、动画等）
            m_currentScene->OnRender(m_backend.get());    // 执行场景渲染

            m_currentScene->OnImGui();                    // 绘制场景UI

            // Check for scene transition - DEFER to next frame's BeginFrame
            // 检查场景是否请求切换——延迟到下一帧的BeginFrame处理
            if (m_currentScene->WantsExit())               // 场景请求退出
            {
                auto nextScene = m_currentScene->GetNextScene(); // 获取下一个场景
                if (nextScene)                            // 如果有下一个场景
                {
                    printf("[Application] Deferring scene transition to next frame\n");
                    m_pendingNextScene = std::move(nextScene); // 保存为待切换场景
                }
                else
                {
                    // No next scene — exit application
                    // 没有下一个场景——退出应用
                    printf("[Application] Scene requested exit with no replacement; shutting down\n");
                    m_running = false;                    // 设置退出标志
                }
            }
        }
        else if (m_frameCallback)                        // 如果没有场景但有帧回调（向后兼容）
        {
            m_frameCallback(dt);                          // 执行帧回调
        }

        // Always render performance panel (visible even without a scene)
        // 始终渲染性能面板（即使没有场景也可见）
        m_perfPanel.Render(this, m_backend.get());

        m_backend->ImGuiRender();                        // 渲染ImGui绘制数据

        // Check for screenshot request (from auto-test)
        // 检查截图请求（来自自动测试）
        if (ScreenshotRequest::Consume()) {              // 如果有待处理的截图请求
            printf("[Application] Processing screenshot request: %s\n", ScreenshotRequest::path);
            if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) { // 仅支持OpenGL后端截图
                gl->SaveScreenshot(ScreenshotRequest::path); // 保存截图
                printf("[Application] Screenshot saved: %s\n", ScreenshotRequest::path);
            } else {
                printf("[Application] Failed to get OpenGL backend for screenshot\n");
            }
        }

        // Auto-screenshot for UI demo
        // UI演示自动截图
        {
            static int screenshotFrame = -1;             // 静态变量记录截图帧计数
            if (screenshotFrame == -1 && getenv("AUTO_TEST_UI")) { // 检测AUTO_TEST_UI环境变量
                screenshotFrame = 0;                     // 初始化帧计数器
            }
            if (screenshotFrame >= 0 && screenshotFrame < 5) { // 在前5帧内
                screenshotFrame++;                        // 递增帧计数
                if (screenshotFrame == 3) {              // 在第3帧时截图
                    if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                        gl->SaveScreenshot("e:/AI/graph/hight-post-proc/ui_screenshot.ppm");
                        printf("[Application] UI screenshot saved at frame %d\n", screenshotFrame);
                    }
                }
            }
        }

        m_backend->EndFrame();                            // 结束帧：交换缓冲区/提交命令缓冲
        } catch (const std::exception& e) {               // 捕获标准异常
            fprintf(stderr, "[Application] EXCEPTION in main loop: %s\n", e.what());
            m_running = false;                            // 异常时退出循环
        } catch (...) {                                  // 捕获所有其他异常
            fprintf(stderr, "[Application] UNKNOWN EXCEPTION in main loop\n");
            m_running = false;                            // 异常时退出循环
        }
    }

    printf("[Application] Main loop exited: m_running=%d, m_window=%p, shouldClose=%d\n",
           (int)m_running, (void*)m_window,
           m_window ? glfwWindowShouldClose(m_window) : -1); // 打印退出状态

    m_running = false;                                    // 确保运行标志为false
}
```

#### 功能说明

`MainLoop` 是帧驱动的游戏循环，每帧执行：事件处理 → 后端切换 → 帧开始 → 场景切换 → 场景更新/渲染/UI → 性能面板 → 截图 → 帧结束。

#### 实现原理

1. **高精度计时**：使用 `std::chrono::high_resolution_clock` 计算帧间隔 `dt`，传递给场景用于动画和物理更新。
2. **延迟切换策略**：
   - 后端切换（`m_pendingBackendSwitch`）在帧循环开始时处理，此时 ImGui/Vulkan 状态稳定。
   - 场景切换（`m_pendingNextScene`）在 `BeginFrame` 之后处理，确保 Vulkan 的 `vkWaitForFences` 已完成，GPU 不再使用旧场景的资源。
3. **异常安全**：整个帧处理包裹在 `try-catch` 中，任何异常都会被捕获并安全退出循环，避免程序崩溃。
4. **双重渲染路径**：优先使用场景系统（`m_currentScene`），如果没有场景则回退到帧回调（`m_frameCallback`），保持向后兼容。

#### 为什么这样实现

- **延迟场景切换**：这是 Vulkan 编程的关键模式。Vulkan 是异步 API，GPU 可能在 CPU 已提交渲染命令后仍在执行。如果在 GPU 执行过程中销毁场景的 GPU 资源（如纹理、缓冲区），会导致 GPU 挂起或驱动崩溃。`BeginFrame` 中的 `vkWaitForFences` 确保 GPU 已完成上一帧的所有工作，此时销毁旧场景资源是安全的。
- **异常捕获**：渲染循环中可能发生各种运行时错误（着色器编译失败、纹理创建失败等），捕获异常可以优雅地退出而非崩溃。
- **`fflush(stdout)`**：在关键日志后立即刷新输出缓冲区，确保即使程序崩溃，日志也已写入。

---

### 2.6 Shutdown — 资源清理

```cpp
void Application::Shutdown()
{
    m_running = false;                                    // 停止主循环

    // Exit and destroy current scene（退出并销毁当前场景）
    if (m_currentScene)
    {
        m_currentScene->OnExit();                          // 通知场景退出
        m_currentScene.reset();                            // 释放场景
    }

    if (m_backend)                                        // 如果后端存在
    {
        m_backend->ImGuiShutdown();                       // 关闭ImGui GPU资源
        m_backend->Shutdown();                            // 关闭后端
        m_backend.reset();                                 // 释放后端
    }

    if (m_window)                                         // 如果主窗口存在
    {
        glfwDestroyWindow(m_window);                      // 销毁主窗口
        m_window = nullptr;
    }
    if (m_retiredWindow)                                  // 如果有退役窗口
    {
        glfwDestroyWindow(m_retiredWindow);               // 销毁退役窗口
        m_retiredWindow = nullptr;
    }
    glfwTerminate();                                      // 终止GLFW库
}
```

#### 功能说明

`Shutdown` 按正确顺序释放所有资源：场景 → 后端 → 窗口 → GLFW。

#### 实现原理

资源释放顺序与初始化顺序相反（LIFO——后进先出）：
1. 先停止运行标志（防止新的帧处理）
2. 退出并释放场景（场景使用后端资源）
3. 关闭后端（后端使用窗口）
4. 销毁窗口（窗口由 GLFW 管理）
5. 终止 GLFW

#### 为什么这样实现

- **反向释放顺序**：资源之间存在依赖关系，必须按依赖的反向顺序释放。场景依赖后端，后端依赖窗口，窗口依赖 GLFW。
- **`m_retiredWindow` 在最后销毁**：退役窗口在程序退出时才销毁，避免了 NVIDIA 驱动在运行期间销毁 Vulkan 表面绑定窗口的潜在问题。
- **幂等性**：`Shutdown` 可以被安全地多次调用（构造函数 → `Run` → 析构函数都可能触发），每个资源释放前都有空指针检查。

---

### 2.7 场景管理 SetScene / SwitchBackend

```cpp
void Application::SetScene(std::unique_ptr<Scene> scene)
{
    if (m_currentScene)                                   // 如果已有当前场景
    {
        printf("[Application] SetScene: exiting old scene\n");
        fflush(stdout);
        m_currentScene->OnExit();                          // 通知旧场景退出
        m_currentScene.reset();                            // 释放旧场景
    }
    m_currentScene = std::move(scene);                     // 接管新场景
    if (m_currentScene)                                    // 如果新场景有效
    {
        printf("[Application] SetScene: entering new scene\n");
        fflush(stdout);
        m_currentScene->OnEnter();                         // 通知新场景进入
        printf("[Application] SetScene: new scene entered OK\n");
        fflush(stdout);
    }
}

void Application::SwitchBackend(BackendType type)
{
    if (type == m_backendType && m_backend)               // 如果请求的后端与当前相同
    {
        return; // Already on the requested backend        // 无需切换
    }
    // Defer the actual switch to the start of the next frame
    // to avoid ImGui/Vulkan state corruption during rendering
    // 将实际切换延迟到下一帧开始，避免ImGui/Vulkan状态损坏
    m_pendingBackend = type;                              // 记录目标后端类型
    m_pendingBackendSwitch = true;                         // 设置待切换标志
    printf("[Application] Backend switch to %s scheduled for next frame\n",
           type == BackendType::OpenGL ? "OpenGL" : "Vulkan");
}
```

#### 功能说明

- **SetScene**：立即切换当前场景，先退出旧场景再进入新场景。
- **SwitchBackend**：延迟切换渲染后端，设置标志后由 `MainLoop` 在下一帧开始时执行实际切换。

#### 实现原理

- `SetScene` 是同步操作：直接退出旧场景、进入新场景。适用于外部代码（如 `main` 函数的帧回调）主动设置场景。
- `SwitchBackend` 是异步操作：仅设置标志，实际切换在 `MainLoop` 的安全时机执行。适用于键盘快捷键（Ctrl+1/Ctrl+2）触发的后端切换。

#### 为什么这样实现

- **SetScene 同步执行**：在帧回调中调用 `SetScene` 时，当前不在 ImGui/Vulkan 渲染过程中，可以安全地同步切换。
- **SwitchBackend 延迟执行**：后端切换通常由键盘回调触发，此时可能正在 ImGui 渲染过程中。立即切换会破坏 ImGui 的内部状态（如命令缓冲区、纹理绑定等），导致崩溃。

---

### 2.8 回调函数 KeyCallback / DropCallback / FramebufferSizeCallback

```cpp
void Application::FramebufferSizeCallback(GLFWwindow* w, int width, int height)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w)); // 从窗口用户数据取回Application指针
    if (app && app->m_backend)                            // 如果Application和后端都有效
    {
        app->m_backend->Resize(width, height);             // 通知后端调整渲染区域
    }
}

void Application::KeyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS)                             // 只处理按键按下事件
    {
        return;
    }

    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w)); // 取回Application指针
    if (!app)
    {
        return;
    }

    // ESC is now handled by the scene system (CoverFlowScene exits app,
    // EffectDetailScene returns to CoverFlow). The KeyCallback ignores ESC.
    // ESC现在由场景系统处理（CoverFlowScene退出应用，EffectDetailScene返回封面流）
    // Global override: Ctrl+Q always exits
    // 全局覆盖：Ctrl+Q始终退出应用
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(w, GLFW_TRUE);           // 请求关闭窗口
        return;
    }

    // Ctrl+1 — switch to OpenGL
    // Ctrl+1 — 切换到OpenGL后端
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_1)
    {
        app->SwitchBackend(BackendType::OpenGL);           // 延迟切换到OpenGL
        return;
    }

    // Ctrl+2 — switch to Vulkan
    // Ctrl+2 — 切换到Vulkan后端
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_2)
    {
        app->SwitchBackend(BackendType::Vulkan);          // 延迟切换到Vulkan
        return;
    }
}

void Application::DropCallback(GLFWwindow* w, int count, const char** paths)
{
    if (count < 1) return;                                // 没有拖放文件则返回
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w)); // 取回Application指针
    if (!app) return;

    // Accept only the first dropped file
    // 只接受第一个拖放的文件
    app->m_droppedFilePath = paths[0];                     // 保存文件路径
    printf("[Application] File dropped: %s\n", paths[0]);
}

std::string Application::ConsumeDroppedFile()
{
    std::string path;                                      // 创建空字符串
    std::swap(path, m_droppedFilePath);                    // 交换：取出路径，清空成员变量
    return path;                                           // 返回拖放文件路径（可能为空）
}
```

#### 功能说明

三个 GLFW 静态回调函数：

- **FramebufferSizeCallback**：窗口尺寸变化时通知后端调整渲染区域（如重建交换链、更新视口）。
- **KeyCallback**：处理全局键盘快捷键（Ctrl+Q 退出、Ctrl+1/2 切换后端）。
- **DropCallback**：处理文件拖放事件，保存第一个拖放文件的路径。
- **ConsumeDroppedFile**：消费（取出并清空）最近拖放的文件路径。

#### 实现原理

- **`glfwGetWindowUserPointer`**：GLFW 的 C 风格回调没有用户数据参数。通过在窗口创建时调用 `glfwSetWindowUserPointer(m_window, this)` 将 `Application*` 存储在窗口中，回调中通过此函数取回。
- **`std::swap` 消费模式**：`ConsumeDroppedFile` 使用 `std::swap` 原子性地取出路径并清空成员变量，避免竞态条件。

#### 为什么这样实现

- **ESC 交给场景处理**：不同场景对 ESC 的响应不同——封面流场景中 ESC 退出应用，详情场景中 ESC 返回封面流。全局回调不处理 ESC，让场景自行决定行为。
- **Ctrl+Q 全局退出**：无论在哪个场景，Ctrl+Q 都能退出应用，作为"紧急出口"。
- **只接受第一个拖放文件**：用户可能一次拖放多个文件，但程序只处理第一个，简化逻辑。

---

## 3. 封面流场景 — CoverFlowScene

CoverFlowScene 是应用的主场景，实现了一个类似 Apple Cover Flow 的 3D 卡片浏览界面，展示所有可用的后处理特效。每张卡片实时渲染对应特效的缩略图预览。

### 3.1 头文件 CoverFlowScene.h

```cpp
#pragma once                                             // 防止头文件重复包含

#include "app/Scene.h"                                    // Scene基类
#include "app/CoverFlowState.h"                           // 封面流状态结构体（用于场景间状态传递）
#include "shader/EffectMetadata.h"                       // 特效元数据（EffectCard等）
#include "render/IRenderBackend.h"                       // 渲染后端接口

#include <vector>                                        // 动态数组
#include <memory>                                        // 智能指针
#include <string>                                        // 字符串
#include <chrono>                                        // 高精度时钟

class Application;                                        // 前向声明
class ScreenCapture;                                     // 前向声明：屏幕捕获
class VideoPlayer;                                       // 前向声明：视频播放器

/// Per-card thumbnail real-time render state
/// 每张卡片的缩略图实时渲染状态
struct CardThumbnailState {
    ShaderHandle fragShader;                             // 该卡片特效的片段着色器句柄
    TextureHandle thumbTex;                              // 256x144缩略图FBO纹理
};

class CoverFlowScene : public Scene {                     // 继承自Scene基类
public:
    CoverFlowScene();                                     // 构造函数
    ~CoverFlowScene() override;                          // 析构函数（覆盖）

    void OnEnter() override;                             // 场景进入时调用
    void OnExit() override;                              // 场景退出时调用
    void OnUpdate(float dt) override;                   // 每帧更新
    void OnRender(IRenderBackend* backend) override;     // 每帧渲染
    void OnImGui() override;                             // 每帧ImGui绘制
    bool WantsExit() const override { return m_wantsExit; } // 是否请求退出

    std::unique_ptr<Scene> GetNextScene() override;      // 获取下一个场景
    void SetInputTexture(TextureHandle tex) { m_inputTex = tex; } // 设置输入纹理
    void SetInputTexCache(const std::vector<TextureHandle>& cache) { m_inputTexCache = cache; }
    // 设置每个特效的缓存输入纹理
    void SetBackend(IRenderBackend* backend) { m_backend = backend; } // 设置渲染后端
    void SetApplication(Application* app) { m_app = app; } // 设置Application指针

    /// Set pre-rendered thumbnail ImTextureIDs (one per card, same order as RegisterCards).
    /// 设置预渲染的缩略图ImTextureID（每张卡片一个，顺序与RegisterCards一致）
    void SetThumbnails(const std::vector<void*>& imTexIds) { m_thumbIds = imTexIds; }

    /// Restore selected card index and scroll offset (used when returning from detail scene).
    /// 恢复选中的卡片索引和滚动偏移（从详情场景返回时使用）
    void SetSelectedIndex(int index) { m_selectedIndex = index; m_targetOffset = 0.0f; m_scrollOffset = 0.0f; }

    /// Set test image base directory (for thumbnail input textures)
    /// 设置测试图片基础目录（用于缩略图输入纹理）
    void SetTestImageBaseDir(const std::string& dir) { m_testImageBaseDir = dir; }

    /// Transfer video player back from detail scene.
    /// 从详情场景传回视频播放器
    void SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime);

    /// Enable auto-test mode: cycle through all cards, holdFrames per card.
    /// 启用自动测试模式：循环所有卡片，每张停留holdFrames帧
    void EnableAutoTest(int holdFrames);

    /// Resume auto-test after returning from detail scene.
    /// 从详情场景返回后恢复自动测试
    void ResumeAutoTest(int holdFrames, int lastOpenedCard);

    /// Reload input texture from a new file path (drag-drop support).
    /// 从新文件路径重新加载输入纹理（拖放支持）
    void ReloadInputTexture(const std::string& filePath);

    /// Add an image path to the built-in image pool (for Ctrl+Left/Right cycling).
    /// 添加图片路径到内置图片池（用于Ctrl+左/右切换）
    void AddImageToPool(const std::string& path);

    /// Add a video path to the built-in video pool.
    /// 添加视频路径到内置视频池
    void AddVideoToPool(const std::string& path);

    /// Get current state for saving (used by EffectDetailScene to restore).
    /// 获取当前状态用于保存（EffectDetailScene用于恢复）
    CoverFlowState GetState() const;

private:
    std::vector<EffectCard> m_cards;                      // 所有特效卡片列表
    int m_selectedIndex   = 0;                            // 当前选中的卡片索引
    float m_scrollOffset  = 0.0f;                        // 当前滚动偏移（动画中间值）
    float m_targetOffset  = 0.0f;                         // 目标滚动偏移（动画目标值）
    TextureHandle m_inputTex = {0};                       // 当前输入纹理
    std::vector<TextureHandle> m_inputTexCache;           // 每个特效的缓存输入纹理
    IRenderBackend* m_backend = nullptr;                  // 渲染后端指针
    Application* m_app = nullptr;                         // Application指针
    bool m_wantsExit = false;                             // 是否请求退出场景
    std::unique_ptr<Scene> m_nextScene;                   // 下一个场景（用于跳转）

    // Auto-test mode（自动测试模式）
    bool m_autoTest = false;                              // 是否启用自动测试
    int  m_autoTestHoldFrames = 0;                        // 每张卡片停留帧数
    int  m_autoTestFrameCounter = 0;                       // 当前卡片已停留帧计数
    int  m_autoTestCardIndex = 0;                          // 当前测试的卡片索引
    int  m_autoTestLastOpenedCard = -1;                   // 上次打开的卡片索引（用于返回后继续）

    // Thumbnails（缩略图）
    std::vector<void*> m_thumbIds;                        // 每张卡片的ImTextureID

    // ---- Dynamic thumbnail rendering ----
    // ---- 动态缩略图渲染 ----
    ShaderHandle m_sharedVertShader = INVALID_SHADER;     // 共享的顶点着色器
    std::vector<CardThumbnailState> m_thumbnailStates;    // 每张卡片的缩略图渲染状态
    int m_thumbWidth  = 256;                               // 缩略图宽度
    int m_thumbHeight = 144;                               // 缩略图高度
    bool m_thumbInitialized = false;                      // 缩略图是否已初始化
    std::string m_testImageBaseDir;                       // 测试图片基础目录
    float m_thumbElapsedTime = 0.0f;                      // 缩略图累计时间（用于动画着色器）
    uint32_t m_thumbFrameCount = 0;                       // 缩略图帧计数

    void InitializeThumbnails();                          // 初始化缩略图渲染资源
    void RenderVisibleThumbnails();                        // 渲染可见卡片的缩略图

    // Mouse drag state（鼠标拖拽状态）
    bool  m_dragging    = false;                          // 是否正在拖拽
    float m_dragStartX  = 0.0f;                           // 拖拽起始X坐标
    float m_dragBaseOff = 0.0f;                           // 拖拽起始时的滚动偏移

    // FPS counter（FPS计数器）
    std::chrono::high_resolution_clock::time_point m_fpsLastTime; // 上次FPS更新时间
    int    m_fpsFrameCount = 0;                            // FPS帧计数
    float  m_fpsDisplay    = 0.0f;                         // 当前显示的FPS值

    // Screen capture（屏幕捕获）
    std::unique_ptr<ScreenCapture> m_screenCapture;       // 屏幕捕获实例
    TextureHandle m_captureTex = {0};                     // 捕获纹理
    bool m_captureActive  = false;                         // 是否正在捕获
    bool m_captureReady   = false;                         // 捕获是否就绪
    int  m_captureWidth   = 0;                             // 捕获宽度
    int  m_captureHeight  = 0;                             // 捕获高度

    // Image cycling（图片切换）
    std::vector<std::string> m_imagePool;                 // 内置图片路径池
    int m_currentImageIndex = 0;                           // 当前显示的图片索引

    // Video cycling（视频切换）
    std::vector<std::string> m_videoPool;                 // 内置视频路径池
    int m_currentVideoIndex = 0;                           // 当前视频索引

    // Video player（视频播放器）
    std::unique_ptr<VideoPlayer> m_videoPlayer;           // 视频播放器实例
    TextureHandle m_videoTex = {0};                       // 视频纹理
    bool m_videoActive = false;                            // 视频是否正在播放
    double m_videoLastFrameTime = 0.0;                     // 上一帧视频时间

    void RegisterCards();                                  // 注册所有特效卡片
    void SelectCard(int index);                            // 选中指定卡片
    void OpenSelectedEffect();                             // 打开选中特效的详情页
    void UpdateFPSCounter();                               // 更新FPS计数器
    void ToggleScreenCapture();                            // 切换屏幕捕获
    void CycleImage(int direction);                        // 切换图片（-1=上一张，+1=下一张）
    void LoadImageFromFile(const std::string& path);       // 从文件加载图片
    void OpenVideoFile(const std::string& path);           // 打开视频文件
    void StopVideo();                                      // 停止视频播放
};
```

#### 功能说明

`CoverFlowScene.h` 定义了封面流场景的完整类结构，是整个应用中最复杂的场景。它管理 18 张特效卡片、实时缩略图渲染、图片/视频输入切换、屏幕捕获、自动测试等功能。

#### 实现原理

- **继承 Scene 基类**：通过覆盖 `OnEnter/OnExit/OnUpdate/OnRender/OnImGui/WantsExit/GetNextScene` 虚函数，融入 Application 的场景管理系统。
- **CardThumbnailState**：每张卡片维护独立的片段着色器和 FBO 纹理，用于实时渲染该特效的缩略图预览。
- **多层输入源**：支持静态图片（图片池）、视频播放器、屏幕捕获三种输入源，通过 `m_inputTex` 统一传递给渲染管线。

#### 为什么这样实现

- **共享顶点着色器**：所有缩略图使用相同的全屏四边形顶点着色器（`m_sharedVertShader`），只有片段着色器不同。这减少了着色器切换开销。
- **`m_thumbElapsedTime` 和 `m_thumbFrameCount`**：某些特效着色器需要时间和帧数作为 uniform（如噪声动画、VHS 闪烁），这些全局计时器确保所有缩略图使用相同的时间基准。

---

### 3.2 卡片注册 RegisterCards（CARD宏）

```cpp
#define CARD(id, name, cat, desc, frag) \
    add(id, name, cat, desc, frag)                       // 宏展开为add函数调用

void CoverFlowScene::RegisterCards()
{
    std::string shaderDir = ShaderLoader::FindShaderDir(); // 查找着色器目录
    // OpenGL uses VAO vertex input, Vulkan uses VertexIndex-generated triangle
    // OpenGL使用VAO顶点输入，Vulkan使用VertexIndex生成的三角形
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv"; // 默认OpenGL顶点着色器
    if (m_backend && m_backend->GetType() == BackendType::Vulkan) { // 如果是Vulkan后端
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv"; // 使用Vulkan专用顶点着色器
    }

    m_cards.clear();                                      // 清空卡片列表
    m_cards.reserve(18);                                  // 预分配18张卡片的空间

    auto add = [&](const char* id, const char* name, const char* category,
                   const char* desc, const char* fragRelPath) { // lambda：添加一张卡片
        EffectCard c;                                     // 创建卡片结构体
        c.id = id; c.name = name; c.category = category; c.description = desc;
        c.vertSpirvPath = vertPath;                      // 设置顶点着色器SPIR-V路径
        c.fragSpirvPath = shaderDir + "/" + fragRelPath; // 设置片段着色器SPIR-V路径
        c.passes = 1;                                     // 默认单通道渲染
        m_cards.push_back(std::move(c));                  // 加入卡片列表
    };

    CARD("simple_test",  "Grayscale Test",    "Simple",
         "Basic grayscale shader, validates render pipeline",
         "effects/simple_test/simple_test.frag.spv");
    // 灰度测试：基础灰度着色器，验证渲染管线

    CARD("bloom",        "Bloom",             "Lighting",
         "Extract bright areas with blur overlay for dreamy glow",
         "effects/bloom/bloom.frag.spv");
    // 泛光：提取高亮区域并叠加模糊，产生梦幻光晕效果

    CARD("blur",         "Gaussian Blur",     "Filter",
         "Multi-pass separable Gaussian blur, large radius soft focus",
         "effects/blur/blur.frag.spv");
    // 高斯模糊：多通道可分离高斯模糊，大半径柔焦

    CARD("sharpen",      "Sharpen",           "Filter",
         "Unsharp mask image sharpening, enhances edge detail",
         "effects/sharpen/sharpen.frag.spv");
    // 锐化：USM锐化，增强边缘细节

    CARD("edge_detect",  "Edge Detection",    "Filter",
         "Sobel edge detection with optional normal visualization",
         "effects/edge_detect/edge_detect.frag.spv");
    // 边缘检测：Sobel算子边缘检测，可选法线可视化

    CARD("emboss",       "Emboss",            "Stylize",
         "Emboss filter for relief texture effect",
         "effects/emboss/emboss.frag.spv");
    // 浮雕：浮雕滤镜，产生浮雕纹理效果

    CARD("pixelate",     "Pixelate",          "Stylize",
         "Adjustable mosaic pixelation block size",
         "effects/pixelate/pixelate.frag.spv");
    // 像素化：可调马赛克像素块大小

    CARD("vignette",     "Vignette",          "Color",
         "Darken edges to focus on center subject",
         "effects/vignette/vignette.frag.spv");
    // 暗角：边缘变暗以聚焦中心主体

    CARD("chromatic",    "Chromatic Aberration", "Distort",
         "RGB channel offset simulating chromatic distortion",
         "effects/chromatic/chromatic.frag.spv");
    // 色差：RGB通道偏移模拟色散畸变

    CARD("color_grade",  "Color Grading",     "Color",
         "LUT-based cinematic color grading",
         "effects/color_grade/color_grade.frag.spv");
    // 色彩分级：基于LUT的电影级色彩调整

    CARD("noise",        "Noise Generator",   "Procedural",
         "Perlin noise with adjustable frequency and amplitude",
         "effects/noise/noise.frag.spv");
    // 噪声生成器：可调频率和振幅的Perlin噪声

    CARD("kaleidoscope", "Kaleidoscope",      "Distort",
         "Radial symmetry with adjustable sectors and rotation",
         "effects/kaleidoscope/kaleidoscope.frag.spv");
    // 万花筒：可调扇区数和旋转的径向对称

    CARD("glitch",       "Glitch Art",        "Stylize",
         "Digital glitch with random block shift and color tearing",
         "effects/glitch/glitch.frag.spv");
    // 故障艺术：随机块位移和色彩撕裂的数字故障效果

    CARD("toon",         "Toon Shading",      "Stylize",
         "Cartoon-style color quantization, cel shading",
         "effects/toon/toon.frag.spv");
    // 卡通着色：卡通风格的颜色量化，赛璐着色

    CARD("vhs",          "VHS Retro",         "Retro",
         "VHS tape scanlines, noise and color drift",
         "effects/vhs/vhs.frag.spv");
    // VHS复古：录像带扫描线、噪声和色彩漂移

    CARD("crt",          "CRT Monitor",       "Retro",
         "CRT scanlines + phosphor RGB pattern + screen curvature",
         "effects/crt/crt.frag.spv");
    // CRT显示器：扫描线+磷光RGB图案+屏幕弯曲

    CARD("water_ripple", "Water Ripple",      "Distort",
         "Normal-map based water ripple displacement",
         "effects/water_ripple/water_ripple.frag.spv");
    // 水波纹：基于法线贴图的水波纹位移

    CARD("lens_distort", "Lens Distortion",   "Distort",
         "Barrel/pincushion lens distortion correction and simulation",
         "effects/lens_distort/lens_distort.frag.spv");
    // 镜头畸变：桶形/枕形镜头畸变校正和模拟

    printf("[CoverFlowScene] Registered %zu effect cards\n", m_cards.size());
}

#undef CARD                                              // 取消CARD宏定义
```

#### 功能说明

`RegisterCards` 注册了 18 张特效卡片，每张卡片包含特效 ID、名称、分类、描述和片段着色器路径。使用 `CARD` 宏简化卡片注册语法。

#### 实现原理

1. **CARD 宏**：将 5 个参数的 `add` 调用封装为更简洁的 `CARD(id, name, cat, desc, frag)` 语法，提高可读性。
2. **Lambda `add`**：捕获 `shaderDir` 和 `vertPath` 的闭包，负责创建 `EffectCard` 结构体并加入列表。
3. **着色器路径适配**：根据当前后端类型选择不同的顶点着色器（OpenGL 使用 VAO 输入，Vulkan 使用 `gl_VertexIndex` 生成三角形）。
4. **18 个特效分类**：Simple、Lighting、Filter、Stylize、Color、Distort、Procedural、Retro 八大类。

#### 为什么这样实现

- **宏 + Lambda 组合**：宏提供了声明式的卡片注册语法，Lambda 提供了灵活的闭包捕获。这种组合既保持了代码简洁性，又避免了全局变量或静态注册表的复杂性。
- **`reserve(18)`**：预分配空间避免 `push_back` 时的多次内存重分配。
- **`#undef CARD`**：宏定义在使用后立即取消，防止污染其他翻译单元。

---

### 3.3 场景生命周期 OnEnter / OnExit

```cpp
void CoverFlowScene::OnEnter()
{
    m_fpsLastTime    = std::chrono::high_resolution_clock::now(); // 初始化FPS计时起点
    m_fpsFrameCount  = 0;                                // 重置FPS帧计数
    m_fpsDisplay     = 0.0f;                             // 重置FPS显示值

    printf("[CoverFlowScene] Entered with %zu cards, selected=%d\n",
           m_cards.size(), m_selectedIndex);

    // Initialize thumbnails for all backends (OpenGL and Vulkan)
    // 为所有后端（OpenGL和Vulkan）初始化缩略图
    InitializeThumbnails();                              // 初始化缩略图渲染资源
}

void CoverFlowScene::OnExit()
{
    printf("[CoverFlowScene] Exiting\n");                // 仅打印退出日志
}
```

#### 功能说明

- **OnEnter**：场景进入时重置 FPS 计数器并初始化缩略图渲染系统。
- **OnExit**：场景退出时仅打印日志（资源释放由析构函数处理）。

#### 实现原理

- `OnEnter` 在场景被 `SetScene` 设置或从详情场景返回时调用。
- `OnExit` 在场景被替换或应用关闭时调用。

#### 为什么这样实现

- **OnEnter 初始化缩略图**：缩略图初始化需要后端已就绪（创建着色器、纹理等 GPU 资源），放在 `OnEnter` 而非构造函数中，确保后端已注入。
- **OnExit 为空**：GPU 资源的释放由析构函数统一处理，避免在 `OnExit` 中释放资源后又被 `OnEnter` 重新创建的冗余操作。

---

### 3.4 帧更新 OnUpdate

```cpp
void CoverFlowScene::OnUpdate(float dt)
{
    // Auto-test: if we just returned from a detail scene and need to test next card
    // 自动测试：如果刚从详情场景返回，需要测试下一张卡片
    if (m_autoTest && m_autoTestLastOpenedCard >= 0) {
        m_autoTestCardIndex = m_autoTestLastOpenedCard + 1; // 移动到下一张卡片
        m_autoTestLastOpenedCard = -1;                   // 清除"返回"标志
        if (m_autoTestCardIndex >= (int)m_cards.size()) {
            // All cards tested!
            // 所有卡片测试完毕！
            printf("\n[AutoTest] ALL %zu CARDS TESTED SUCCESSFULLY!\n", m_cards.size());
            m_autoTest = false;                           // 关闭自动测试
        } else {
            printf("[AutoTest] Card %d/%zu passed, now testing card %d...\n",
                   m_autoTestCardIndex, m_cards.size(), m_autoTestCardIndex);
            SelectCard(m_autoTestCardIndex);              // 选中下一张卡片
            OpenSelectedEffect();                          // 进入详情页
            m_autoTestFrameCounter = m_autoTestHoldFrames; // 重置停留帧计数
        }
    }

    // Smooth scroll animation（平滑滚动动画）
    float diff = m_targetOffset - m_scrollOffset;        // 计算目标与当前的差值
    m_scrollOffset += diff * std::fmin(1.0f, dt * 8.0f); // 指数衰减插值，dt*8控制速度

    ImGuiIO& io = ImGui::GetIO();                        // 获取ImGui IO结构体引用

    // ---- FPS counter ----
    // ---- FPS计数器 ----
    UpdateFPSCounter();                                   // 更新FPS显示值

    // ---- Drag-drop: check for dropped files ----
    // ---- 拖放：检查是否有拖放的文件 ----
    if (m_app) {                                         // 如果Application指针有效
        std::string dropped = m_app->ConsumeDroppedFile(); // 消费拖放文件路径
        if (!dropped.empty()) {                           // 如果有拖放文件
            // Check if it's a video file
            // 检查是否为视频文件
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');            // 查找文件扩展名分隔符
            if (dot != std::string::npos) {
                ext = ext.substr(dot);                    // 提取扩展名
                // Convert to lowercase
                // 转换为小写
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    OpenVideoFile(dropped);                // 视频文件：打开视频播放器
                } else {
                    StopVideo();                           // 非视频文件：停止当前视频
                    ReloadInputTexture(dropped);          // 加载为输入纹理
                }
            }
        }
    }

    // ---- Video player: update frames ----
    // ---- 视频播放器：更新帧 ----
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();                    // 获取当前时间
        // ffmpeg pipe outputs at fixed 30fps (see StartFFmpegProcess: -r 30)
        // ffmpeg管道以固定30fps输出（见StartFFmpegProcess: -r 30）
        double frameInterval = 1.0 / 30.0;               // 帧间隔约33.3ms
        if (now - m_videoLastFrameTime >= frameInterval) { // 到达下一帧时间
            if (m_videoPlayer->ReadFrame()) {            // 读取一帧视频
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());          // 更新视频纹理的GPU数据
                m_inputTex = m_videoTex;                   // 将视频纹理设为当前输入
                m_videoLastFrameTime = now;               // 更新帧时间
            } else {
                // Video ended — loop
                // 视频结束——循环
                printf("[CoverFlowScene] Video ended, looping\n");
                StopVideo();                               // 停止视频（简化处理）
            }
        }
    }

    // ---- Screen capture: update input texture if active ----
    // ---- 屏幕捕获：如果激活则更新输入纹理 ----
    if (m_captureActive && m_screenCapture && m_screenCapture->IsReady()) {
        bool newFrame = m_screenCapture->CaptureFrame();  // 捕获一帧屏幕
        if (newFrame && m_backend) {                      // 如果有新帧
            m_backend->UpdateTexture(m_captureTex, 0, 0,
                m_captureWidth, m_captureHeight,
                m_screenCapture->GetPixels());            // 更新捕获纹理的GPU数据
        }
        // Use capture texture as input (even if no new frame, texture has previous capture)
        // 使用捕获纹理作为输入（即使没有新帧，纹理也包含上一次捕获的内容）
    }

    // ---- Auto-test timer ----
    // ---- 自动测试计时器 ----
    if (m_autoTest && !m_wantsExit) {                     // 如果自动测试激活且未请求退出
        m_autoTestFrameCounter--;                          // 递减帧计数
        if (m_autoTestFrameCounter <= 0) {                // 停留时间到
            // Time's up: open current card's detail scene
            // 时间到：打开当前卡片的详情场景
            printf("[AutoTest] Opening card %d: %s\n", m_autoTestCardIndex,
                   m_cards[m_autoTestCardIndex].name.c_str());
            m_autoTestLastOpenedCard = m_autoTestCardIndex; // 记录已打开的卡片
            OpenSelectedEffect();                          // 进入详情页
        }
    }

    // ---- Keyboard navigation ----
    // ---- 键盘导航 ----
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft))
        SelectCard(m_selectedIndex - 1);                   // 左箭头/手柄左：选中上一张
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight))
        SelectCard(m_selectedIndex + 1);                  // 右箭头/手柄右：选中下一张
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        OpenSelectedEffect();                              // 回车：打开选中特效
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {           // ESC：退出场景
        printf("[CoverFlowScene] ESC detected, exiting\n");
        m_wantsExit = true;
    }

    // ---- F1-F8: Quick effect selection ----
    // ---- F1-F8：快速选择特效 ----
    for (int k = 0; k < 8; k++) {                         // 遍历F1-F8
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + k))) { // 检测功能键
            int targetIdx = k; // F1=0, F2=1, ... F8=7
            if (targetIdx < (int)m_cards.size()) {        // 确保索引有效
                SelectCard(targetIdx);                     // 选中对应卡片
            }
        }
    }

    // ---- Ctrl+S: Toggle screen capture ----
    // ---- Ctrl+S：切换屏幕捕获 ----
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        ToggleScreenCapture();                             // 切换屏幕捕获状态
    }

    // ---- Ctrl+Left/Right: Cycle built-in images ----
    // ---- Ctrl+左/右：切换内置图片 ----
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        CycleImage(-1);                                   // 上一张图片
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        CycleImage(+1);                                   // 下一张图片
    }

    // ---- Mouse wheel ----
    // ---- 鼠标滚轮 ----
    float wheel = io.MouseWheel;                           // 获取滚轮偏移量
    if (wheel != 0.0f)
        SelectCard(m_selectedIndex - static_cast<int>(wheel)); // 滚轮上=上一张，下=下一张

    // ---- Mouse drag to scroll cards ----
    // ---- 鼠标拖拽滚动卡片 ----
    const float cardW   = 280.0f;                          // 卡片宽度
    const float spacing = 80.0f;                           // 卡片间距
    const float cardUnit = cardW + spacing;                // 相邻卡片中心距离

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) { // 如果正在拖拽左键
        float dx = io.MouseDelta.x;                       // 鼠标X方向移动量
        if (!m_dragging) {                                 // 如果刚开始拖拽
            m_dragging = true;
            m_dragStartX = io.MousePos.x;                  // 记录起始X坐标
            m_dragBaseOff = m_scrollOffset;               // 记录起始滚动偏移
        }
        // Convert pixel drag to card index offset
        // 将像素拖拽转换为卡片索引偏移
        float offset = m_dragBaseOff - dx / cardUnit;     // 累计偏移（以卡片为单位）
        // Clamp（限制范围）
        int idx = m_selectedIndex + static_cast<int>(std::round(offset)); // 四舍五入到最近卡片
        if (idx < 0) idx = 0;                              // 不小于0
        if (idx >= (int)m_cards.size()) idx = (int)m_cards.size() - 1; // 不超过最大值
        if (idx != m_selectedIndex) {                     // 如果选中了不同的卡片
            SelectCard(idx);                                // 切换选中卡片
            m_scrollOffset = offset - static_cast<float>(idx - m_selectedIndex); // 更新滚动偏移
        }
    } else {
        m_dragging = false;                                // 未拖拽时重置拖拽状态
    }
}
```

#### 功能说明

`OnUpdate` 是每帧调用的核心更新函数，处理：自动测试逻辑、平滑滚动动画、FPS 计算、拖放文件处理、视频播放更新、屏幕捕获更新、键盘/鼠标/手柄输入处理。

#### 实现原理

1. **自动测试状态机**：通过 `m_autoTestLastOpenedCard` 检测是否刚从详情场景返回，如果是则推进到下一张卡片。
2. **平滑滚动**：`m_scrollOffset` 通过指数衰减插值（`diff * dt * 8`）向 `m_targetOffset` 靠近，产生自然的减速效果。
3. **视频帧同步**：以 30fps 的固定间隔从 FFmpeg 管道读取帧，更新 GPU 纹理。
4. **输入优先级**：ESC 最先处理（退出场景），然后是功能键、快捷键、滚轮、拖拽。

#### 为什么这样实现

- **`std::fmin(1.0f, dt * 8.0f)`**：限制插值系数不超过 1.0，防止在低帧率（大 dt）时出现动画跳变。
- **拖拽使用 `io.MouseDelta`**：使用帧间鼠标增量而非绝对位置，确保拖拽响应与帧率无关。
- **视频帧间隔固定 30fps**：与 FFmpeg 管道的输出帧率匹配，避免不必要的帧读取或跳帧。

---

### 3.5 渲染 OnRender + RenderVisibleThumbnails

```cpp
void CoverFlowScene::OnRender(IRenderBackend* /*backend*/)
{
    // Update thumbnail time/frame counters
    // 更新缩略图时间和帧计数器
    static auto lastTime = std::chrono::high_resolution_clock::now(); // 静态变量记录上次时间
    auto now = std::chrono::high_resolution_clock::now(); // 当前时间
    float dt = std::chrono::duration<float>(now - lastTime).count(); // 帧间隔
    lastTime = now;                                      // 更新上次时间
    m_thumbElapsedTime += dt;                            // 累计缩略图时间
    m_thumbFrameCount++;                                  // 递增缩略图帧计数

    // Render visible card thumbnails every frame
    // 每帧渲染可见卡片的缩略图
    RenderVisibleThumbnails();                            // 渲染当前可见范围内的缩略图
}
```

```cpp
void CoverFlowScene::RenderVisibleThumbnails()
{
    if (!m_thumbInitialized || !m_backend) return;        // 未初始化或后端无效则返回

    const int vr = 3;                                     // 可见范围：当前卡片左右各3张

    for (int i = m_selectedIndex - vr; i <= m_selectedIndex + vr; i++) { // 遍历可见范围
        if (i < 0 || i >= (int)m_thumbnailStates.size()) continue; // 越界跳过
        const auto& state = m_thumbnailStates[i];         // 获取该卡片的缩略图状态
        if (state.fragShader.id == INVALID_SHADER.id) continue; // 着色器无效跳过
        if (state.thumbTex.id == INVALID_TEXTURE.id) continue;    // 纹理无效跳过

        TextureHandle input = m_inputTex;                  // 默认使用当前输入纹理
        if (i < (int)m_inputTexCache.size() && m_inputTexCache[i].id != INVALID_TEXTURE.id) {
            input = m_inputTexCache[i];                   // 如果有缓存纹理则使用缓存
        }

        m_backend->BeginRenderToTexture(state.thumbTex);  // 开始渲染到纹理（FBO）
        ShaderParams params;                               // 着色器参数
        params.inputTextures.push_back(input);            // 设置输入纹理
        params.uniformFloats  = {4.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}; // 默认uniform值
        params.viewportWidth  = m_thumbWidth;               // 视口宽度256
        params.viewportHeight = m_thumbHeight;             // 视口高度144
        params.time           = m_thumbElapsedTime;        // 累计时间（动画着色器用）
        params.frameCount     = m_thumbFrameCount;        // 帧计数（动画着色器用）
        m_backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
        // 使用共享顶点着色器和该卡片专属片段着色器绘制全屏四边形
        m_backend->EndRenderToTexture();                   // 结束渲染到纹理

        m_thumbIds[i] = m_backend->GetImTextureID(state.thumbTex);
        // 获取纹理的ImGui显示ID，用于后续ImGui::Image绘制
    }
}
```

#### 功能说明

- **OnRender**：更新缩略图的全局计时器并调用 `RenderVisibleThumbnails`。
- **RenderVisibleThumbnails**：为当前可见范围内的每张卡片（选中卡片左右各 3 张）实时渲染特效缩略图到独立的 FBO 纹理。

#### 实现原理

1. **FBO 离屏渲染**：每张卡片有独立的 256x144 FBO 纹理，通过 `BeginRenderToTexture/EndRenderToTexture` 将特效渲染到该纹理。
2. **可见范围优化**：只渲染选中卡片左右各 3 张（共 7 张），而非全部 18 张，减少 GPU 负载。
3. **ImGui 纹理 ID**：通过 `GetImTextureID` 将后端纹理句柄转换为 ImGui 可用的 `ImTextureID`，存储在 `m_thumbIds` 中供 `OnImGui` 使用。

#### 为什么这样实现

- **实时缩略图**：每帧重新渲染缩略图而非使用预渲染的静态图片，使动画类特效（如噪声、VHS、水波纹）在封面流中也能展示动态效果。
- **256x144 分辨率**：16:9 宽高比，与 1280x720 的窗口比例一致。低分辨率减少 GPU 负载，在封面流中显示已足够清晰。
- **`m_thumbElapsedTime` 使用静态局部变量**：`OnRender` 中的 `lastTime` 是静态变量，确保跨帧持续计时，不受场景重建影响。

---

### 3.6 UI 绘制 OnImGui

```cpp
void CoverFlowScene::OnImGui()
{
    ImGuiIO& io = ImGui::GetIO();                        // 获取ImGui IO
    float w = io.DisplaySize.x;                          // 窗口宽度
    float h = io.DisplaySize.y;                          // 窗口高度
    if (w > 0 && h > 0) {                                // 窗口尺寸有效
        ImGui::SetNextWindowPos(ImVec2(0, 0));           // 设置窗口位置为左上角
        ImGui::SetNextWindowSize(ImVec2(w, h));           // 设置窗口大小为全屏
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));
    // 设置窗口背景色为深灰蓝色

    ImGui::Begin("##CoverFlow", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);
    // 创建全屏无边框窗口，名称"##CoverFlow"（##前缀表示不在UI中显示）

    ImDrawList* dl = ImGui::GetWindowDrawList();         // 获取窗口绘制列表（自定义绘制）

    // ---- Title ----
    // ---- 标题 ----
    {
        auto& LM = LanguageManager::Instance();           // 获取语言管理器单例
        const char* title = LM.Title();                   // 获取本地化标题
        char subtitle[256];                               // 副标题缓冲区
        const char* srcLabel = m_captureActive ? LM.ScreenCaptureLabel()
                              : (!m_imagePool.empty() ? m_imagePool[m_currentImageIndex].c_str() : LM.StaticImageLabel());
        // 根据当前输入源选择副标题标签：屏幕捕获/图片池/静态图片
        // Extract just the filename
        // 提取纯文件名（去掉路径）
        const char* justFile = strrchr(srcLabel, '/');    // 查找最后一个'/'
        if (justFile) justFile++; else { justFile = strrchr(srcLabel, '\\'); if (justFile) justFile++; else justFile = srcLabel; }
        // 兼容Unix和Windows路径分隔符
        snprintf(subtitle, sizeof(subtitle), LM.InputInfoFormat(),
                 justFile, m_fpsDisplay);                 // 格式化副标题：文件名 + FPS
        ImVec2 ts = ImGui::CalcTextSize(title);           // 计算标题文本尺寸
        ImVec2 ss = ImGui::CalcTextSize(subtitle);        // 计算副标题文本尺寸
        dl->AddText(ImGui::GetFont(), 28.0f,              // 绘制标题：28号字体，居中
            ImVec2(w / 2.0f - ts.x / 2.0f, 30.0f),
            IM_COL32(220, 220, 255, 255), title);         // 浅蓝白色
        dl->AddText(ImGui::GetFont(), 16.0f,              // 绘制副标题：16号字体，居中
            ImVec2(w / 2.0f - ss.x / 2.0f, 65.0f),
            IM_COL32(140, 140, 160, 220), subtitle);       // 灰蓝色
    }

    // ---- Language toggle button (top-right corner) ----
    // ---- 语言切换按钮（右上角）----
    {
        auto& LM = LanguageManager::Instance();
        ImGui::SetCursorScreenPos(ImVec2(w - 120.0f, 20.0f)); // 定位到右上角
        if (ImGui::Button(LM.LanguageButton(), ImVec2(100, 30))) { // 绘制按钮
            LM.ToggleLanguage();                          // 切换语言（中文/英文）
        }
    }

    const float cardW   = 280.0f;                          // 卡片宽度
    const float cardH   = 380.0f;                          // 卡片高度
    const float spacing = 80.0f;                           // 卡片间距
    const float centerX = w / 2.0f;                        // 屏幕中心X
    const float centerY = h / 2.0f + 20.0f;              // 屏幕中心Y（略偏下）

    // ---- Page dots ----
    // ---- 页面指示点 ----
    {
        const float dotR = 4.0f, dotSp = 14.0f;          // 点半径4px，间距14px
        const int maxD = 18;                               // 最多显示18个点
        int ds = std::max(0, m_selectedIndex - 8);        // 起始索引（当前选中前8个）
        int de = std::min((int)m_cards.size(), ds + maxD); // 结束索引
        float dx = centerX - (de - ds - 1) * dotSp / 2.0f; // 起始X位置（居中）
        float dy = centerY - cardH / 2.0f - 25.0f;       // Y位置（卡片上方）
        for (int i = ds; i < de; i++) {                    // 遍历所有点
            dl->AddCircleFilled(ImVec2(dx, dy), dotR,    // 绘制圆点
                (i == m_selectedIndex) ? IM_COL32(180,180,255,255) : IM_COL32(80,80,100,150));
                // 选中的点为亮蓝白色，未选中为暗灰色
            dx += dotSp;                                    // 移动到下一个点的位置
        }
    }

    // ---- Cards ----
    // ---- 卡片绘制 ----
    int vr = 3; // visible range on each side（每侧可见范围3张）
    for (int i = (int)m_cards.size() - 1; i >= 0; i--) {  // 从后往前遍历（确保选中的卡片在最前面）
        // Only render cards in visible range
        // 只渲染可见范围内的卡片
        if (i < m_selectedIndex - vr || i > m_selectedIndex + vr) continue;

        float off  = (float)(i - m_selectedIndex) + m_scrollOffset - m_targetOffset;
        // 计算卡片偏移量（考虑滚动动画）
        float x    = centerX + off * (cardW + spacing);    // 卡片中心X坐标
        float scl  = 1.0f / (1.0f + std::fabs(off) * 0.3f); // 缩放因子：越远越小
        float alpha = 1.0f - std::fabs(off) * 0.35f;      // 透明度：越远越透明
        if (alpha < 0.0f) alpha = 0.0f;                   // 不小于0

        float w2 = cardW * scl, h2 = cardH * scl;        // 缩放后的卡片尺寸
        float x0 = x - w2/2, y0 = centerY - h2/2;        // 卡片左上角
        float x1 = x + w2/2, y1 = centerY + h2/2;        // 卡片右下角

        int ai = (int)(alpha * 255);                        // 透明度转换为0-255整数

        // ---- Thumbnail image ----
        // ---- 缩略图图片 ----
        bool hasThumb = i < (int)m_thumbIds.size() && m_thumbIds[i] != nullptr;
        // 检查是否有缩略图
        ImU32 bg, bd;                                       // 背景色和边框色
        if (i == m_selectedIndex) {                         // 选中的卡片
            bg = IM_COL32(50,55,80,ai); bd = IM_COL32(160,160,240,ai);
            // 背景深蓝灰色，边框亮蓝紫色
        } else {                                            // 未选中的卡片
            bg = IM_COL32(35,38,50,ai); bd = IM_COL32(80,80,110,(int)(alpha*200));
            // 背景更暗，边框更暗
        }

        float r = 10.0f * scl;                             // 圆角半径（随缩放变化）

        if (hasThumb && alpha > 0.1f) {                     // 有缩略图且足够可见
            // Draw thumbnail image first (tint with white, alpha controls visibility)
            // 先绘制缩略图（白色着色，alpha控制可见度）
            dl->AddImageRounded(m_thumbIds[i],
                ImVec2(x0, y0), ImVec2(x1, y1),
                ImVec2(0, 1), ImVec2(1, 0),                // UV翻转（OpenGL纹理坐标）
                IM_COL32(255, 255, 255, ai), r);           // 白色着色+透明度+圆角
        } else {
            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bg, r);
            // 无缩略图则绘制纯色背景
        }
        dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), bd, r, 0, 2.0f);
        // 绘制边框（2px宽）

        if (alpha > 0.25f) {                               // 透明度足够时绘制文字信息
            const auto& card = m_cards[i];
            float cx = x;                                   // 卡片中心X

            // ---- Card click area (InvisibleButton) ----
            // ---- 卡片点击区域（不可见按钮）----
            ImGui::SetCursorScreenPos(ImVec2(x0, y0));    // 设置光标位置
            ImGui::InvisibleButton(("##card_" + std::to_string(i)).c_str(), ImVec2(w2, h2));
            // 创建不可见按钮（##前缀避免显示在UI中）
            if (ImGui::IsItemClicked()) {                  // 如果按钮被点击
                // Click any card to open its detail page directly
                // 点击任意卡片直接打开其详情页
                SelectCard(i);                              // 选中该卡片
                OpenSelectedEffect();                        // 打开详情页
            }

            // ---- Dark overlay at bottom of card for text readability ----
            // ---- 卡片底部暗色遮罩（提高文字可读性）----
            float overlayH = 140.0f * scl;                 // 遮罩高度
            dl->AddRectFilledMultiColor(
                ImVec2(x0, y1 - overlayH), ImVec2(x1, y1),
                IM_COL32(0,0,0,0),                         // 左上透明
                IM_COL32(0,0,0,0),                         // 右上透明
                IM_COL32(20,22,30,(int)(alpha*220)),       // 左下半透明深色
                IM_COL32(20,22,30,(int)(alpha*220)));       // 右下半透明深色

            // ---- Category tag ----
            // ---- 分类标签 ----
            float cy = y1 - overlayH + 15.0f * scl;        // 标签Y位置
            {
                const char* catStr = LanguageManager::Instance().TranslateCategory(card.category);
                // 获取本地化分类名
                ImVec2 cs = ImGui::CalcTextSize(catStr);   // 计算文本尺寸
                float tw = cs.x + 12.0f, th = cs.y + 6.0f; // 标签背景尺寸（加内边距）
                dl->AddRectFilled(ImVec2(x0 + 10.0f*scl, cy - 2.0f),
                    ImVec2(x0 + 10.0f*scl + tw, cy + th + 2.0f),
                    IM_COL32(60,65,100,(int)(alpha*220)), 4.0f); // 蓝灰色圆角背景
                dl->AddText(ImGui::GetFont(), 11.0f * scl,
                    ImVec2(x0 + 16.0f*scl, cy),
                    IM_COL32(160,170,220,(int)(alpha*255)), catStr); // 浅蓝文字
            }

            // ---- Name ----
            // ---- 特效名称 ----
            cy = y1 - overlayH + 38.0f * scl;              // 名称Y位置
            {
                const char* nameStr = LanguageManager::Instance().CardName(card.id);
                // 获取本地化特效名
                dl->AddText(ImGui::GetFont(), 18.0f * scl,
                    ImVec2(x0 + 14.0f*scl, cy),
                    IM_COL32(255, 255, 255, (int)(alpha*255)), nameStr); // 白色大字
            }

            // ---- Index ----
            // ---- 索引编号 ----
            if (i == m_selectedIndex) {                     // 仅选中卡片显示索引
                char buf[32];
                snprintf(buf, sizeof(buf), "%d / %zu", i+1, m_cards.size());
                ImVec2 isz = ImGui::CalcTextSize(buf);
                dl->AddText(ImGui::GetFont(), 12.0f,
                    ImVec2(cx - isz.x/2, y1 - 16.0f), IM_COL32(140,140,180,200), buf);
                // 居中显示在卡片底部
            }
        }
    }

    // ---- Nav hint ----
    // ---- 导航提示 ----
    {
        const char* hint = LanguageManager::Instance().BottomHelp(); // 获取本地化帮助文本
        ImVec2 hs = ImGui::CalcTextSize(hint);
        dl->AddText(ImGui::GetFont(), 14.0f,
            ImVec2(centerX - hs.x/2, centerY + cardH/2 + 50.0f),
            IM_COL32(150,150,170,160), hint);              // 灰色居中文字
    }

    // ---- Category legend ----
    // ---- 分类图例 ----
    {
        auto& LM = LanguageManager::Instance();
        char leg[256];
        snprintf(leg, sizeof(leg), "%s | %s | %s | %s | %s | %s | %s | %s",
            LM.CategorySimple(), LM.CategoryLighting(), LM.CategoryFilter(), LM.CategoryStylize(),
            LM.CategoryColor(), LM.CategoryDistort(), LM.CategoryProcedural(), LM.CategoryRetro());
        // 拼接所有分类名称
        ImVec2 ls = ImGui::CalcTextSize(leg);
        dl->AddText(ImGui::GetFont(), 12.0f,
            ImVec2(centerX - ls/2, h - 35.0f), IM_COL32(100,100,120,130), leg);
        // 屏幕底部居中显示
    }

    ImGui::End();                                          // 结束ImGui窗口
    ImGui::PopStyleColor();                               // 恢复样式颜色

    // ---- Card screenshot mode ----
    // ---- 卡片截图模式（自动化测试用）----
    if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_DETAILS")) {
        g_cardScreenshotFrame++;                           // 递增帧计数
        // Take screenshot after a few frames of showing the card
        // 在显示卡片几帧后截图
        if (g_cardScreenshotNeedsShot && g_cardScreenshotFrame >= 5) { // 第5帧时截图
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/card_%02d.ppm", g_cardScreenshotIndex - 1);
            ScreenshotRequest::Request(path);             // 请求截图
            g_cardScreenshotNeedsShot = false;
        }
        // Move to next card（移动到下一张卡片）
        if (g_cardScreenshotFrame >= 30) {                // 每张卡片停留30帧
            g_cardScreenshotFrame = 0;
            if (g_cardScreenshotIndex < (int)m_cards.size()) {
                SelectCard(g_cardScreenshotIndex);          // 选中下一张
                printf("[CardScreenshot] Selected card %d: %s\n", g_cardScreenshotIndex, m_cards[g_cardScreenshotIndex].name.c_str());
                g_cardScreenshotIndex++;
                g_cardScreenshotNeedsShot = true;          // 标记需要截图
            } else {
                printf("[CardScreenshot] All %zu cards captured\n", m_cards.size());
                m_wantsExit = true;                         // 所有卡片截图完成，退出
            }
        }
    }

    // ---- Detail page screenshot mode ----
    // ---- 详情页截图模式（自动化测试用）----
    if (getenv("AUTO_TEST_DETAILS") && !getenv("AUTO_TEST_UI")) {
        static int detailCardIndex = 0;                     // 静态变量：当前详情页卡片索引
        static int frameWait = 0;                           // 静态变量：等待帧计数
        frameWait++;
        if (frameWait >= 30) {                             // 每30帧切换一张
            frameWait = 0;
            if (detailCardIndex < (int)m_cards.size()) {
                SelectCard(detailCardIndex);                // 选中卡片
                printf("[DetailScreenshot] Opening card %d: %s\n", detailCardIndex, m_cards[detailCardIndex].name.c_str());
                detailCardIndex++;
                OpenSelectedEffect();                      // 打开详情页（截图在详情场景中完成）
            } else {
                printf("[DetailScreenshot] All %zu detail pages captured\n", m_cards.size());
                m_wantsExit = true;                         // 所有详情页截图完成，退出
            }
        }
    }
}
```

#### 功能说明

`OnImGui` 使用 ImGui 的自定义绘制列表（`ImDrawList`）绘制整个封面流界面，包括：标题、语言切换按钮、页面指示点、特效卡片（含缩略图、分类标签、名称、索引）、导航提示、分类图例，以及自动化测试的截图模式。

#### 实现原理

1. **全屏窗口**：创建一个覆盖整个屏幕的无边框 ImGui 窗口作为绘制画布。
2. **卡片 3D 效果**：通过缩放因子 `1.0 / (1.0 + |off| * 0.3)` 和透明度衰减 `1.0 - |off| * 0.35` 模拟透视效果——选中卡片最大最亮，两侧卡片逐渐缩小变暗。
3. **从后往前绘制**：`for (int i = m_cards.size() - 1; i >= 0; i--)` 确保选中的卡片（索引较小）绘制在最后（最前面），产生正确的遮挡关系。
4. **`AddImageRounded`**：带圆角的纹理绘制，UV 坐标翻转 `(0,1)→(1,0)` 适配 OpenGL 的纹理坐标原点。
5. **`InvisibleButton`**：使用不可见按钮实现卡片点击检测，比手动计算鼠标碰撞更简洁可靠。

#### 为什么这样实现

- **自定义绘制而非 ImGui 控件**：卡片需要精确的透视缩放、透明度渐变和圆角纹理，ImGui 内置控件无法满足这些需求。使用 `ImDrawList` 的底层绘制 API 可以完全控制每个像素。
- **`AddRectFilledMultiColor` 渐变遮罩**：底部文字区域使用四角不同颜色的矩形，从顶部全透明过渡到底部半透明，产生自然的渐变效果，提高文字可读性。
- **静态变量用于自动测试**：`g_cardScreenshotIndex`、`g_cardScreenshotFrame` 等使用文件级静态变量而非成员变量，因为它们是纯测试用途，不应影响类的接口设计。

---

### 3.7 场景跳转 OpenSelectedEffect / GetNextScene

```cpp
void CoverFlowScene::SelectCard(int index) {
    if (index < 0) index = (int)m_cards.size() - 1;       // 超出左边界则循环到末尾
    if (index >= (int)m_cards.size()) index = 0;           // 超出右边界则循环到开头
    if (index != m_selectedIndex) {                        // 如果选中了不同的卡片
        m_selectedIndex = index;                            // 更新选中索引
        m_targetOffset = 0.0f;                             // 重置目标偏移（触发滚动动画）
        printf("[CoverFlowScene] Selected card %d: %s\n", index, m_cards[index].name.c_str());
    }
}
```

```cpp
void CoverFlowScene::OpenSelectedEffect()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_cards.size()) return; // 索引越界检查
    if (!m_backend) {                                      // 后端未就绪
        fprintf(stderr, "[CoverFlowScene] Cannot open effect: no backend set\n");
        return;
    }

    const auto& card = m_cards[m_selectedIndex];           // 获取选中的卡片
    EffectCard resolvedCard = card;                         // 复制卡片（可能被JSON覆盖）

    // Try to load effect.json for richer metadata (with try-catch safety)
    // 尝试加载effect.json获取更丰富的元数据（带异常安全保护）
    try {
        std::string shaderDir = ShaderLoader::FindShaderDir();
        std::string jsonPath = shaderDir + "/effects/" + card.id + "/effect.json";
        EffectCard fj = LoadEffectFromJson(jsonPath);      // 从JSON加载元数据
        if (!fj.name.empty()) {                            // JSON加载成功
            fj.vertSpirvPath = card.vertSpirvPath;         // 保留原始着色器路径
            fj.fragSpirvPath = card.fragSpirvPath;
            fj.id = card.id;
            resolvedCard = std::move(fj);                   // 使用JSON中的丰富元数据
        }
    } catch (...) {
        fprintf(stderr, "[CoverFlowScene] effect.json parse error for %s, using defaults\n", card.id.c_str());
    }

    // Pick the per-effect cached input texture if available
    // 如果有每个特效的缓存输入纹理则使用
    TextureHandle detailInputTex = m_inputTex;             // 默认回退到当前输入纹理
    if (m_selectedIndex < (int)m_inputTexCache.size() &&
        m_inputTexCache[m_selectedIndex].id != INVALID_TEXTURE.id) {
        detailInputTex = m_inputTexCache[m_selectedIndex];  // 使用该特效的专属纹理
        printf("[CoverFlowScene] Using cached input texture for card %d\n", m_selectedIndex);
    }

    auto ds = std::make_unique<EffectDetailScene>(resolvedCard, detailInputTex);
    // 创建特效详情场景
    ds->SetBackend(m_backend);                              // 注入后端
    ds->SetApplication(m_app);                              // 注入Application
    ds->SetCoverFlowState(GetState());                      // 保存当前状态（用于返回时恢复）

    // Transfer video player to detail scene (for dynamic playback in compare view)
    // 将视频播放器转移给详情场景（用于对比视图中的动态播放）
    if (m_videoActive && m_videoPlayer) {
        ds->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
        m_videoActive = false;                              // 标记视频不再由本场景管理
        m_videoTex = {0};
        printf("[CoverFlowScene] Transferred video player to detail scene\n");
    }

    printf("[CoverFlowScene] Opening effect: %s (%s)\n",
           resolvedCard.name.c_str(), resolvedCard.id.c_str());

    m_nextScene = std::move(ds);                            // 保存为下一个场景
    m_wantsExit = true;                                     // 请求退出当前场景
}
```

```cpp
std::unique_ptr<Scene> CoverFlowScene::GetNextScene()
{
    if (m_nextScene) {                                      // 如果有下一个场景
        printf("[CoverFlowScene] Transitioning to next scene\n");
        return std::move(m_nextScene);                     // 转移所有权给调用者
    }
    return nullptr;                                          // 没有下一个场景
}
```

#### 功能说明

- **SelectCard**：选中指定索引的卡片，支持循环索引（超出边界自动环绕）。触发滚动动画。
- **OpenSelectedEffect**：打开选中特效的详情页，创建 `EffectDetailScene` 并传递所有必要状态。
- **GetNextScene**：返回下一个场景（由 Application 在场景切换时调用）。

#### 实现原理

1. **SelectCard 循环索引**：`index < 0` 时跳到最后一张，`index >= size` 时跳到第一张，实现无限循环浏览。
2. **OpenSelectedEffect 状态传递**：
   - 从 `effect.json` 加载更丰富的元数据（如 uniform 滑块定义、多通道配置等），失败时回退到 `RegisterCards` 中的默认数据。
   - 使用 `GetState()` 保存当前封面流状态（选中索引、图片池、缩略图等），详情场景返回时可以完整恢复。
   - 视频播放器通过 `std::move` 转移所有权给详情场景，避免两个场景同时操作同一视频资源。
3. **延迟切换**：`m_wantsExit = true` + `m_nextScene` 的组合让 Application 在下一帧安全地执行场景切换。

#### 为什么这样实现

- **JSON 元数据覆盖**：`effect.json` 可以定义比代码中更丰富的元数据（如 uniform 范围、描述文本、多通道配置等），但 JSON 文件是可选的——如果不存在或解析失败，使用代码中的默认值，确保程序健壮性。
- **`std::move` 转移视频播放器**：视频播放器管理 FFmpeg 子进程和管道，不能被两个场景同时使用。`std::move` 确保所有权唯一转移，避免资源冲突。
- **`m_wantsExit` 而非直接切换**：场景不能直接替换自己（它正在被 Application 调用），必须通过 `WantsExit + GetNextScene` 协议让 Application 在安全时机执行切换。

---

### 3.8 状态保存与恢复 GetState

```cpp
CoverFlowState CoverFlowScene::GetState() const
{
    CoverFlowState s;                                       // 创建状态结构体
    s.thumbIds         = m_thumbIds;                       // 保存缩略图ImTextureID列表
    s.inputTexCache    = m_inputTexCache;                  // 保存每个特效的缓存纹理
    s.imagePool        = m_imagePool;                      // 保存图片池路径
    s.videoPool        = m_videoPool;                      // 保存视频池路径
    s.currentImageIndex = m_currentImageIndex;            // 保存当前图片索引
    s.selectedIndex    = m_selectedIndex;                 // 保存选中卡片索引
    s.app              = m_app;                            // 保存Application指针
    s.inputTex         = m_inputTex;                       // 保存当前输入纹理
    s.backend          = m_backend;                       // 保存后端指针
    s.captureActive    = m_captureActive;                  // 保存屏幕捕获状态
    s.autoTest         = m_autoTest;                       // 保存自动测试状态
    s.autoTestHoldFrames = m_autoTestHoldFrames;           // 保存自动测试停留帧数
    s.autoTestCardIndex  = m_autoTestCardIndex;             // 保存自动测试当前卡片索引
    s.testImageBaseDir = m_testImageBaseDir;               // 保存测试图片目录
    return s;                                              // 返回完整状态
}
```

#### 功能说明

`GetState` 将封面流场景的所有关键状态打包为 `CoverFlowState` 结构体，供 `EffectDetailScene` 在返回时恢复封面流的完整状态。

#### 实现原理

通过值拷贝将所有需要持久化的成员变量收集到一个结构体中。`EffectDetailScene` 在构造时保存此状态，返回封面流时通过 `SetSelectedIndex`、`SetThumbnails` 等方法恢复。

#### 为什么这样实现

- **快照模式**：在进入详情场景时拍摄"快照"，返回时从快照恢复。这比逐一传递每个状态变量更简洁，也更容易扩展——添加新状态只需修改 `CoverFlowState` 结构体。
- **不保存 transient 状态**：滚动偏移、拖拽状态、FPS 计数器等瞬态状态不需要保存，因为返回封面流时会重新初始化。

---

### 3.9 辅助功能 LoadImageFromFile / CycleImage / ToggleScreenCapture

```cpp
void CoverFlowScene::CycleImage(int direction)
{
    if (m_imagePool.empty()) return;                        // 图片池为空则返回
    if (m_captureActive) return;                           // 正在屏幕捕获时不切换

    m_currentImageIndex += direction;                      // 按方向移动索引
    if (m_currentImageIndex < 0)
        m_currentImageIndex = (int)m_imagePool.size() - 1; // 循环到末尾
    if (m_currentImageIndex >= (int)m_imagePool.size())
        m_currentImageIndex = 0;                           // 循环到开头

    LoadImageFromFile(m_imagePool[m_currentImageIndex]);  // 加载新图片
}
```

```cpp
void CoverFlowScene::LoadImageFromFile(const std::string& path)
{
    if (!m_backend) return;                                // 后端未就绪

    int iw, ih, comp;                                      // 图片尺寸和通道
    stbi_set_flip_vertically_on_load(true);                // 翻转Y轴适配OpenGL
    stbi_uc* data = stbi_load(path.c_str(), &iw, &ih, &comp, 4); // 加载为RGBA
    stbi_set_flip_vertically_on_load(false);               // 恢复默认
    if (!data) {                                           // 加载失败
        fprintf(stderr, "[CoverFlowScene] Cannot load: %s\n", path.c_str());
        return;
    }

    printf("[CoverFlowScene] Loading image: %s (%d x %d)\n", path.c_str(), iw, ih);

    // Destroy old input texture if valid
    // 如果旧纹理有效则销毁
    if (m_inputTex.id != INVALID_TEXTURE.id) {
        m_backend->DestroyTexture(m_inputTex);              // 释放旧GPU纹理
        m_inputTex = INVALID_TEXTURE;                      // 标记为无效
    }

    TextureHandle newTex = m_backend->CreateTexture(iw, ih, TextureFormat::RGBA8, data);
    // 创建新GPU纹理
    stbi_image_free(data);                                  // 释放CPU端图片数据

    if (newTex.id == INVALID_TEXTURE.id) return;           // 创建失败

    m_inputTex = newTex;                                   // 更新当前输入纹理
}
```

```cpp
void CoverFlowScene::ToggleScreenCapture()
{
    if (m_captureActive) {                                  // 当前正在捕获→停止
        // Disable screen capture
        m_captureActive = false;
        if (m_screenCapture) {
            m_screenCapture->Shutdown();                   // 关闭屏幕捕获
            m_screenCapture.reset();                        // 释放捕获实例
        }
        if (m_captureTex.id != INVALID_TEXTURE.id && m_backend) {
            m_backend->DestroyTexture(m_captureTex);        // 释放捕获纹理
            m_captureTex = INVALID_TEXTURE;
        }
        m_captureReady = false;
        printf("[CoverFlowScene] Screen capture stopped\n");
    } else {                                                // 当前未捕获→启动
        // Enable screen capture
        if (!m_backend) return;                             // 后端未就绪

        m_screenCapture = std::make_unique<ScreenCapture>(); // 创建屏幕捕获实例
        if (!m_screenCapture->Init()) {                     // 初始化（启动捕获进程）
            fprintf(stderr, "[CoverFlowScene] Screen capture init failed\n");
            m_screenCapture.reset();
            return;
        }

        // Capture first frame to get dimensions
        // 捕获第一帧以获取尺寸
        if (!m_screenCapture->CaptureFrame()) {
            fprintf(stderr, "[CoverFlowScene] Screen capture first frame failed\n");
            m_screenCapture->Shutdown();
            m_screenCapture.reset();
            return;
        }

        m_captureWidth  = m_screenCapture->GetWidth();     // 获取捕获宽度
        m_captureHeight = m_screenCapture->GetHeight();    // 获取捕获高度

        // Create OpenGL texture for screen capture
        // 为屏幕捕获创建GPU纹理
        m_captureTex = m_backend->CreateTexture(m_captureWidth, m_captureHeight,
                                                 TextureFormat::RGBA8,
                                                 m_screenCapture->GetPixels());
        if (m_captureTex.id == INVALID_TEXTURE.id) {       // 纹理创建失败
            fprintf(stderr, "[CoverFlowScene] Failed to create capture texture\n");
            m_screenCapture->Shutdown();
            m_screenCapture.reset();
            return;
        }

        // Switch input texture to capture
        // 将输入纹理切换为捕获纹理
        m_inputTex = m_captureTex;
        m_captureActive = true;
        m_captureReady  = true;
        printf("[CoverFlowScene] Screen capture started (%d x %d)\n",
               m_captureWidth, m_captureHeight);
    }
}
```

#### 功能说明

- **CycleImage**：在图片池中按方向循环切换图片（-1 上一张，+1 下一张），支持循环索引。
- **LoadImageFromFile**：从磁盘加载图片文件，销毁旧纹理并创建新 GPU 纹理。
- **ToggleScreenCapture**：切换屏幕捕获状态——开启时捕获桌面屏幕作为输入源，关闭时恢复静态图片输入。

#### 实现原理

1. **CycleImage**：修改 `m_currentImageIndex` 并调用 `LoadImageFromFile` 加载对应图片。索引超出范围时自动循环。
2. **LoadImageFromFile**：使用 `stb_image` 加载图片到 CPU 内存，翻转 Y 轴后创建 GPU 纹理。先销毁旧纹理再创建新纹理，避免 GPU 内存泄漏。
3. **ToggleScreenCapture**：开启时创建 `ScreenCapture` 实例，捕获第一帧获取尺寸，创建 GPU 纹理；关闭时按相反顺序释放资源。

#### 为什么这样实现

- **CycleImage 检查 `m_captureActive`**：屏幕捕获时不允许切换图片，因为输入源已被屏幕捕获接管。避免用户混淆输入源。
- **LoadImageFromFile 先销毁后创建**：确保同一时间只有一个输入纹理占用 GPU 内存。如果先创建再销毁，在 GPU 内存紧张时可能导致创建失败。
- **ToggleScreenCapture 捕获第一帧获取尺寸**：屏幕捕获的分辨率取决于桌面设置，无法提前知道。必须先捕获一帧才能获取正确的宽高来创建纹理。
- **`stbi_set_flip_vertically_on_load(true/false)`**：OpenGL 的纹理坐标原点在左下角（Y 向上），而图片文件的原点在左上角（Y 向下）。翻转确保图片在纹理中方向正确。使用后立即恢复默认值，避免影响其他图片加载。

---

> **文档结束** — 以上涵盖了 Shader Showcase 项目第 1-3 章的完整源码解析。


# Shader Showcase 项目代码详解 — 第 4~5 章

---

## 4. 特效详情页 — EffectDetailScene

`EffectDetailScene` 是用户从封面流（CoverFlow）中选择某张特效卡片后进入的全屏特效预览场景。它负责加载着色器、管理 uniform 参数、渲染全屏后处理特效，并提供 Before/After 对比模式和调试面板。

### 4.1 头文件 EffectDetailScene.h

```cpp
#pragma once

#include "app/Scene.h"              // Scene 基类，定义场景生命周期接口
#include "app/CoverFlowState.h"     // CoverFlowState 结构体，用于保存/恢复封面流状态
#include "shader/EffectMetadata.h"   // EffectCard / EffectMetadata 定义（特效卡片数据）
#include "render/IRenderBackend.h"  // IRenderBackend 渲染后端抽象接口
#include "ui/DebugPanel.h"          // DebugPanel ImGui 调试面板组件

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class Application;   // 前向声明：应用程序主类
class ScreenCapture;  // 前向声明：屏幕截图功能
class VideoPlayer;   // 前向声明：视频播放器（FFmpeg 管道）

class EffectDetailScene : public Scene {
public:
    // 构造函数：接收特效卡片数据和输入纹理句柄
    EffectDetailScene(const EffectCard& card, TextureHandle inputTex);
    // 析构函数：释放 GL 着色器和纹理资源
    ~EffectDetailScene();

    // 设置渲染后端指针（由 Application 在场景切换时注入）
    void SetBackend(IRenderBackend* backend) { m_backend = backend; }
    // 设置应用程序指针（用于访问拖放文件队列等全局功能）
    void SetApplication(Application* app) { m_app = app; }

    /// Save CoverFlowScene state so we can restore it on return.
    /// 保存 CoverFlowScene 的状态，以便返回时完整恢复
    void SetCoverFlowState(const CoverFlowState& state) {
        m_savedState = state;
        // Auto-test: if coming from auto-test mode, set up auto-return timer
        // 自动测试模式：如果从自动测试进入，设置自动返回计时器
        if (state.autoTest) {
            m_autoTestHoldFrames = state.autoTestHoldFrames;
        }
    }

    /// Transfer video player ownership from CoverFlowScene (for dynamic video playback).
    /// 从 CoverFlowScene 转移视频播放器所有权（用于动态视频播放）
    void SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime);

    // ---- Scene 生命周期接口 ----
    void OnEnter() override;                              // 场景进入：加载着色器、初始化参数
    void OnExit() override;                               // 场景退出：释放资源
    void OnUpdate(float dt) override;                     // 每帧更新：计时、输入处理、视频帧更新
    void OnRender(IRenderBackend* backend) override;      // 渲染：全屏特效绘制
    void OnImGui() override;                               // ImGui UI 绘制
    bool WantsExit() const override { return m_wantsExit; }  // 是否请求退出
    std::unique_ptr<Scene> GetNextScene() override;       // 获取下一个场景（返回 CoverFlow）

private:
    EffectCard      m_card;           // 当前特效卡片数据（名称、着色器路径、参数定义等）
    TextureHandle   m_inputTex;       // 输入纹理（原始图像或视频帧）
    IRenderBackend* m_backend = nullptr;  // 渲染后端接口指针
    Application*    m_app     = nullptr;  // 应用程序指针

    ShaderHandle m_vertShader = INVALID_SHADER;  // 顶点着色器句柄
    ShaderHandle m_fragShader = INVALID_SHADER;  // 片段着色器句柄

    float    m_time       = 0.0f;     // 累计时间（秒），传入着色器 uTime
    uint32_t m_frameCount = 0;        // 帧计数器，传入着色器 uFrameCount
    int      m_viewportWidth  = 0;    // 当前视口宽度
    int      m_viewportHeight = 0;    // 当前视口高度

    bool      m_showDebug = true;      // 是否显示调试面板
    DebugPanel m_debugPanel;           // ImGui 调试面板实例

    std::vector<float>   m_uniformFloats;   // float 类型 uniform 参数值数组
    std::vector<int32_t> m_uniformInts;     // int 类型 uniform 参数值数组
    size_t               m_expectedFloatCount = 0;  // 着色器实际期望的 float 参数数量

    bool m_wantsExit         = false;  // 是否请求退出当前场景
    bool m_returnToCoverFlow = false;  // 是否返回到封面流

    // Saved CoverFlow state for restoration
    // 保存的 CoverFlow 状态，用于返回时恢复
    CoverFlowState m_savedState;

    // Auto-test
    int m_autoTestHoldFrames = 0; // >0 means auto-exit after this many frames
    // 自动测试：>0 表示在此帧数后自动退出

    // Compare mode (slider before/after)
    // 对比模式：左右分割显示原图和特效处理后的图像
    bool m_compareMode = true;        // before/after comparison (enabled by default, toggle with C key)
    float m_compareSplitPos = 0.5f;   // split position (0=left all original, 1=right all effect)
    bool m_compareDragging = false;  // is user dragging the split handle?
    TextureHandle m_effectTex = {0};  // FBO texture for effect output
    bool m_effectTexCreated = false;

    // Screenshot counter (per-scene instance)
    int m_screenshotCaptured = 0;     // 截图计数器（每个场景实例独立）

    // Video playback
    std::unique_ptr<VideoPlayer> m_videoPlayer;  // 视频播放器（unique_ptr 独占所有权）
    TextureHandle m_videoTex = {0};               // 视频帧纹理
    bool m_videoActive = false;                    // 视频是否正在播放
    double m_videoLastFrameTime = 0.0;             // 上一帧视频更新时间戳

    void LoadImageFromFile(const std::string& path);   // 从文件加载图片（拖放支持）
    void LoadVideoFromFile(const std::string& path);   // 从文件加载视频（拖放支持）
    void StopVideo();                                   // 停止视频播放
    void EnsureEffectTexture();                         // 确保对比模式用的 FBO 纹理已创建

    // Compare view rendering
    void RenderCompareView(IRenderBackend* backend);       // 渲染对比视图
    void RenderFullscreenEffect(IRenderBackend* backend);  // 渲染全屏特效
};
```

#### 功能说明

`EffectDetailScene.h` 定义了特效详情页场景的完整数据结构和接口。该场景是整个应用的核心交互场景之一，用户在此查看单个后处理特效的全屏效果。

#### 实现原理

该类继承自 `Scene` 基类，遵循场景生命周期模式（`OnEnter` -> `OnUpdate`/`OnRender`/`OnImGui` 循环 -> `GetNextScene`）。通过组合模式持有渲染后端指针、调试面板、视频播放器等组件，实现关注点分离。

#### 为什么这样实现

- **状态保存/恢复机制**：`CoverFlowState` 结构体使场景切换时能完整恢复封面流的滚动位置、选中索引、图片池等状态，避免用户迷失位置。
- **对比模式独立纹理**：`m_effectTex` 作为 FBO 纹理，将特效渲染结果离屏保存，再由 ImGui 进行 Before/After 合成显示，解耦了渲染与 UI 展示。
- **视频播放器所有权转移**：使用 `unique_ptr` 管理视频播放器，在场景间转移时语义清晰，避免双重释放或悬挂指针。

---

### 4.2 构造与析构

#### 构造函数

```cpp
EffectDetailScene::EffectDetailScene(const EffectCard& card, TextureHandle inputTex)
    : m_card(card)        // 初始化成员：保存特效卡片数据（着色器路径、参数定义等）
    , m_inputTex(inputTex) // 初始化成员：保存输入纹理句柄（封面流传入的原始图像纹理）
{
    // 构造函数体为空，所有初始化通过成员初始化列表完成
    // 其他成员使用类内默认值初始化（如 m_backend = nullptr, m_time = 0.0f 等）
}
```

#### 功能说明

构造函数接收从 `CoverFlowScene` 传递过来的特效卡片数据和输入纹理，仅做最小化初始化。重量级资源（着色器编译、纹理创建）推迟到 `OnEnter` 中执行。

#### 实现原理

采用"延迟初始化"（Lazy Initialization）设计模式。构造时只保存传入参数，不执行任何可能失败或耗时的操作。这确保了场景对象可以快速创建，而实际的 GL 资源加载在场景激活时才进行。

#### 为什么这样实现

- **快速创建**：场景切换时需要先创建新场景再销毁旧场景，构造函数保持轻量可避免卡顿。
- **后端可用性**：构造时 `m_backend` 尚未设置，无法执行 GL 操作。`OnEnter` 时后端已就绪，是安全的初始化时机。

#### 析构函数

```cpp
EffectDetailScene::~EffectDetailScene()
{
    // Release GL resources to prevent accumulation across scene switches
    // 释放 GL 资源，防止场景切换时资源累积泄漏
    if (m_backend) {  // 确保后端指针有效
        if (m_vertShader.id != INVALID_SHADER.id) {  // 顶点着色器有效
            m_backend->DestroyShader(m_vertShader);  // 通过后端接口销毁顶点着色器
            m_vertShader = INVALID_SHADER;            // 重置句柄为无效值
        }
        if (m_fragShader.id != INVALID_SHADER.id) {  // 片段着色器有效
            m_backend->DestroyShader(m_fragShader);    // 通过后端接口销毁片段着色器
            m_fragShader = INVALID_SHADER;            // 重置句柄为无效值
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {  // 对比模式 FBO 纹理有效
            m_backend->DestroyTexture(m_effectTex);    // 销毁对比模式纹理
            m_effectTex = {0};                          // 重置句柄
            m_effectTexCreated = false;                 // 标记为未创建
        }
    }
    StopVideo();  // 停止视频播放（关闭 FFmpeg 管道，释放播放器）
}
```

#### 功能说明

析构函数作为资源释放的"安全网"，确保即使 `OnExit` 未被调用（如异常退出），GL 资源也不会泄漏。它销毁着色器、对比模式纹理，并停止视频播放。

#### 实现原理

采用 RAII（Resource Acquisition Is Initialization）模式的变体。虽然资源在 `OnEnter` 中获取而非构造函数，但析构函数仍负责释放，形成"获取-释放"配对。`OnExit` 也会执行相同的释放操作，形成双重保险。

#### 为什么这样实现

- **防御性编程**：场景生命周期中可能出现异常路径（如 Application 提前销毁），析构函数确保资源不泄漏。
- **幂等释放**：通过将句柄重置为 `INVALID_SHADER` / `{0}`，多次调用释放函数不会产生问题（`OnExit` + 析构函数双重调用安全）。

---

### 4.3 SetVideoPlayer — 视频播放器所有权转移

```cpp
void EffectDetailScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);   // 转移视频播放器的独占所有权（原指针置空）
    m_videoTex = videoTex;                // 保存视频帧纹理句柄
    m_videoActive = active;               // 标记视频是否正在播放
    m_videoLastFrameTime = lastFrameTime; // 保存上一帧时间戳（用于帧率控制）
    if (active) {
        m_inputTex = m_videoTex;  // Use video texture as input for effect rendering
        // 如果视频正在播放，将视频纹理设为特效的输入纹理
        // 这样特效着色器将处理视频帧而非静态图片
    }
}
```

#### 功能说明

从 `CoverFlowScene` 接收视频播放器的所有权。当用户在封面流中正在播放视频并点击进入详情页时，视频播放不应中断，而是无缝转移到详情页继续播放。

#### 实现原理

使用 C++11 的 `std::unique_ptr` 的移动语义实现所有权转移。`std::move` 将播放器从 CoverFlowScene 转移到 EffectDetailScene，原指针自动置空，确保同一时间只有一个所有者。

#### 为什么这样实现

- **无缝视频体验**：用户在封面流看到视频预览后进入详情页，视频应继续播放而非重新开始。
- **独占所有权**：`unique_ptr` 语义明确，编译器强制执行所有权唯一性，避免误用。
- **状态连续性**：`lastFrameTime` 参数确保帧率控制计时器在场景切换后保持连续，不会出现跳帧。

---

### 4.4 OnEnter — 着色器加载与参数初始化

```cpp
void EffectDetailScene::OnEnter()
{
    if (!m_backend) {  // 防御性检查：后端指针必须有效
        fprintf(stderr, "[EffectDetailScene] OnEnter called without backend!\n");
        return;  // 无后端则无法执行任何 GL 操作，直接返回
    }

    // Initialize debug panel with effect parameters (works on all backends)
    // 用特效参数初始化调试面板（适用于所有后端类型）
    m_debugPanel.SetParams(m_card.params);

    // Initialize uniform values from card params defaults
    // 从卡片参数的默认值初始化 uniform 变量
    m_uniformFloats.clear();   // 清空 float uniform 数组
    m_uniformInts.clear();    // 清空 int uniform 数组
    for (const auto& p : m_card.params) {  // 遍历所有参数定义
        switch (p.type) {  // 根据参数类型分别处理
        case ParamType::Float:  // 单个 float 参数
            m_uniformFloats.push_back(p.defaultVal[0]);  // 取默认值第1个分量
            break;
        case ParamType::Int:     // 整数参数
        case ParamType::Bool:    // 布尔参数（以 int 存储）
            m_uniformInts.push_back(static_cast<int32_t>(p.defaultVal[0]));  // 转为 int32
            break;
        case ParamType::Float2:  // 二维向量参数
            m_uniformFloats.push_back(p.defaultVal[0]);  // x 分量
            m_uniformFloats.push_back(p.defaultVal[1]);  // y 分量
            break;
        case ParamType::Float3:  // 三维向量参数
        case ParamType::Color:   // 颜色参数（RGB 三分量）
            m_uniformFloats.push_back(p.defaultVal[0]);  // r/x 分量
            m_uniformFloats.push_back(p.defaultVal[1]);  // g/y 分量
            m_uniformFloats.push_back(p.defaultVal[2]);  // b/z 分量
            break;
        case ParamType::Float4:  // 四维向量参数
            m_uniformFloats.push_back(p.defaultVal[0]);  // x 分量
            m_uniformFloats.push_back(p.defaultVal[1]);  // y 分量
            m_uniformFloats.push_back(p.defaultVal[2]);  // z 分量
            m_uniformFloats.push_back(p.defaultVal[3]);  // w 分量
            break;
        }
    }
    m_expectedFloatCount = m_uniformFloats.size();  // 记录期望的 float 数量，用于后续校验

    // Load SPIR-V shaders (works on all backends)
    // 加载 SPIR-V 着色器（适用于所有后端）
    // OpenGL uses VAO vertex input, Vulkan uses VertexIndex-generated triangle
    // OpenGL 使用 VAO 顶点输入，Vulkan 使用 VertexIndex 生成的三角形
    std::string vertPath = m_card.vertSpirvPath;  // 获取顶点着色器 SPIR-V 路径
    if (m_backend->GetType() == BackendType::Vulkan) {  // 如果后端是 Vulkan
        // Replace fullscreen.vert.spv with fullscreen_vk.vert.spv for Vulkan
        // 将 fullscreen.vert.spv 替换为 fullscreen_vk.vert.spv（Vulkan 专用版本）
        size_t pos = vertPath.find("fullscreen.vert.spv");  // 查找 OpenGL 版本文件名
        if (pos != std::string::npos) {  // 找到了
            vertPath.replace(pos, 19, "fullscreen_vk.vert.spv");  // 替换为 Vulkan 版本
        }
    }
    auto vertSpirv = ShaderLoader::LoadSPIRV(vertPath);  // 加载顶点着色器 SPIR-V 二进制

    auto fragSpirv = ShaderLoader::LoadSPIRV(m_card.fragSpirvPath);  // 加载片段着色器 SPIR-V 二进制

    if (vertSpirv.empty() || fragSpirv.empty()) {  // 检查着色器是否加载成功
        fprintf(stderr, "[EffectDetailScene] Failed to load SPIR-V shaders\n");
        return;  // 加载失败则退出，场景仍可运行但无特效渲染
    }

    // Always use SPIR-V shaders (consistent with CoverFlowScene thumbnail path).
    // 始终使用 SPIR-V 着色器（与 CoverFlowScene 缩略图路径一致）
    // GLSL+UBO has unreliable reflection on NVIDIA when vertex/fragment shader
    // types are mixed (SPIR-V vs GLSL). Pure SPIR-V works correctly in all cases.
    // 在 NVIDIA 上混合使用 SPIR-V 和 GLSL 时 UBO 反射不可靠，纯 SPIR-V 在所有情况下正常工作
    m_vertShader = m_backend->CreateVertexShader(vertSpirv.data(), vertSpirv.size());  // 创建顶点着色器
    m_fragShader = m_backend->CreateFragmentShader(fragSpirv.data(), fragSpirv.size());  // 创建片段着色器
    printf("[EffectDetailScene] Using SPIR-V shaders\n");  // 日志输出

    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) {
        // 检查着色器创建是否成功
        fprintf(stderr, "[EffectDetailScene] Failed to create shaders\n");
        return;  // 创建失败则退出
    }
}
```

#### 功能说明

`OnEnter` 是场景的初始化入口，负责三大任务：(1) 从 `EffectCard` 的参数定义初始化调试面板和 uniform 默认值；(2) 根据后端类型选择正确的顶点着色器变体；(3) 加载 SPIR-V 着色器二进制并通过后端接口编译。

#### 实现原理

参数初始化采用"展平数组"策略：将不同类型的参数（Float、Float2、Float3、Color、Float4）全部展平为一个连续的 `float` 数组，对应着色器 UBO 中的 `uParamFloat0~5` 字段。这种设计简化了数据传递，避免了复杂的类型映射。

着色器加载使用 SPIR-V 中间格式而非 GLSL 源码。SPIR-V 是 Khronos 标准的着色器中间语言，OpenGL 4.6+ 和 Vulkan 均原生支持，实现了"一次编译、多后端运行"。

#### 为什么这样实现

- **统一 SPIR-V**：注释中明确说明了原因——NVIDIA 驱动在混合 SPIR-V 顶点着色器和 GLSL 片段着色器时，UBO 反射（`glGetUniformBlockIndex`）可能返回错误结果。纯 SPIR-V 路径在所有硬件上行为一致。
- **展平 uniform 数组**：UBO 的 std140 布局要求严格的内存对齐，展平数组直接按偏移量写入，避免了结构体对齐的复杂性。
- **Vulkan 顶点着色器变体**：Vulkan 不支持 `gl_VertexID` 直接生成全屏三角形（需要 `VertexIndex` 内建变量），因此需要不同的顶点着色器。

---

### 4.5 OnExit — 场景退出与资源释放

```cpp
void EffectDetailScene::OnExit()
{
    // Stop video playback
    // 停止视频播放
    StopVideo();

    // Release GL resources (destructor also does this as safety net)
    // 释放 GL 资源（析构函数也会执行此操作作为安全网）
    if (m_backend) {  // 确保后端有效
        if (m_vertShader.id != INVALID_SHADER.id) {  // 顶点着色器有效
            m_backend->DestroyShader(m_vertShader);  // 销毁顶点着色器
            m_vertShader = INVALID_SHADER;            // 重置句柄
        }
        if (m_fragShader.id != INVALID_SHADER.id) {  // 片段着色器有效
            m_backend->DestroyShader(m_fragShader);    // 销毁片段着色器
            m_fragShader = INVALID_SHADER;              // 重置句柄
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {  // FBO 纹理有效
            m_backend->DestroyTexture(m_effectTex);    // 销毁 FBO 纹理
            m_effectTex = {0};                          // 重置句柄
            m_effectTexCreated = false;                 // 标记为未创建
        }
    }

    printf("[EffectDetailScene] Exited: %s\n", m_card.name.c_str());  // 日志输出特效名称
}
```

#### 功能说明

在场景退出时（切换到其他场景之前）释放所有 GL 资源并停止视频播放。这是正常的资源释放路径，与析构函数形成双重保险。

#### 实现原理

资源释放顺序为：视频播放器 -> 着色器 -> 纹理。先停止视频（可能正在使用纹理），再释放着色器和纹理。每个资源释放后立即重置句柄，确保幂等性。

#### 为什么这样实现

- **显式释放优先**：`OnExit` 是正常的资源释放点，析构函数是异常路径的安全网。正常情况下 `OnExit` 先执行，资源在此释放；如果 `OnExit` 未被调用（异常），析构函数兜底。
- **日志追踪**：输出退出的特效名称，便于调试场景切换问题。

---

### 4.6 OnUpdate — 帧更新与输入处理

```cpp
void EffectDetailScene::OnUpdate(float dt)
{
    m_time += dt;         // 累加时间增量，用于着色器动画（uTime）
    m_frameCount++;       // 递增帧计数，用于着色器动画（uFrameCount）

    // Auto-test: auto-return after holdFrames
    // 自动测试：在指定帧数后自动返回
    if (m_autoTestHoldFrames > 0) {  // 如果设置了自动测试帧数
        m_autoTestHoldFrames--;       // 递减计数器
        if (m_autoTestHoldFrames <= 0) {  // 计数器归零
            m_wantsExit = true;              // 标记请求退出
            m_returnToCoverFlow = true;      // 标记返回封面流
            printf("[EffectDetailScene] Auto-test timer expired, returning to CoverFlow\n");
        }
    }

    // ESC returns to CoverFlow
    // ESC 键返回封面流
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {  // 检测 ESC 键按下
        m_wantsExit         = true;  // 标记请求退出
        m_returnToCoverFlow = true;  // 标记返回封面流
        printf("[EffectDetailScene] ESC pressed, returning to CoverFlow\n");
    }

    // ---- Drag-drop: check for dropped files ----
    // ---- 拖放：检查是否有文件被拖入 ----
    if (m_app) {  // 应用指针有效
        std::string dropped = m_app->ConsumeDroppedFile();  // 取出拖放文件路径（消费队列）
        if (!dropped.empty()) {  // 有文件被拖入
            // Check if it's a video file
            // 检查是否为视频文件
            std::string ext = dropped;  // 复制路径字符串
            auto dot = ext.find_last_of('.');  // 查找最后一个点（扩展名分隔符）
            if (dot != std::string::npos) {  // 找到了扩展名
                ext = ext.substr(dot);  // 提取扩展名（含点号）
                // Convert to lowercase
                // 转为小写（手动实现，避免依赖 locale）
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    // 支持的视频格式列表
                    LoadVideoFromFile(dropped);  // 加载视频文件
                } else {
                    StopVideo();  // 如果当前在播放视频，先停止
                    LoadImageFromFile(dropped);  // 加载图片文件
                }
            }
        }
    }

    // ---- Video player: update frames ----
    // ---- 视频播放器：更新帧 ----
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        // 条件：视频激活 && 播放器有效 && 已打开 && 后端有效
        double now = ImGui::GetTime();  // 获取当前时间（秒）
        // ffmpeg pipe outputs at fixed 30fps (see StartFFmpegProcess: -r 30)
        // FFmpeg 管道以固定 30fps 输出（见 StartFFmpegProcess: -r 30）
        double frameInterval = 1.0 / 30.0;  // 帧间隔 = 1/30 秒
        if (now - m_videoLastFrameTime >= frameInterval) {  // 到达下一帧时间
            if (m_videoPlayer->ReadFrame()) {  // 从 FFmpeg 管道读取一帧
                m_backend->UpdateTexture(m_videoTex, 0, 0,  // 更新视频纹理
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());  // 像素数据指针
                m_inputTex = m_videoTex;  // 将视频纹理设为特效输入
                m_videoLastFrameTime = now;  // 更新帧时间戳
            } else {
                // Video ended — loop
                // 视频结束（FFmpeg 管道关闭）
                printf("[EffectDetailScene] Video ended, stopping playback\n");
                StopVideo();  // 停止播放（当前实现为停止而非循环）
            }
        }
    }
}
```

#### 功能说明

`OnUpdate` 每帧调用一次，负责：(1) 累加时间和帧计数供着色器使用；(2) 处理 ESC 键和自动测试退出逻辑；(3) 处理文件拖放（图片或视频）；(4) 更新视频播放器帧。

#### 实现原理

时间管理使用简单的累加器模式。`m_time` 和 `m_frameCount` 直接传入着色器 UBO，供特效实现时间驱动动画（如波纹、闪烁等）。

文件拖放通过 `Application::ConsumeDroppedFile()` 消费式队列实现。GLFW 的拖放回调将文件路径推入队列，`OnUpdate` 每帧检查并消费一个文件，避免在回调中执行耗时的 GL 操作。

视频帧更新采用固定帧率（30fps）节流。`ImGui::GetTime()` 提供高精度时间戳，与 `m_videoLastFrameTime` 比较决定是否读取新帧。

#### 为什么这样实现

- **消费式拖放队列**：GLFW 的文件拖放回调在事件线程触发，直接在其中执行 GL 操作不安全。队列模式将文件路径传递到主线程的安全时机处理。
- **固定帧率节流**：FFmpeg 管道按 30fps 输出，读取过快会阻塞，读取过慢会丢帧。精确匹配输出帧率可最大化流畅度。
- **手动小写转换**：避免依赖 `std::tolower` 的 locale 行为差异，手动 ASCII 范围检查更可靠。

---

### 4.7 OnRender — 全屏特效渲染与对比模式

```cpp
void EffectDetailScene::OnRender(IRenderBackend* backend)
{
    if (!backend) return;  // 后端无效则跳过
    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) return;
    // 着色器未加载成功则跳过（场景仍可显示 UI）

    // Sync uniform values from debug panel
    // 从调试面板同步 uniform 值（用户可能通过滑块修改了参数）
    m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);

    // Restore correct UBO float count (DebugPanel may shrink the array)
    // 恢复正确的 UBO float 数量（DebugPanel 可能缩小了数组）
    if (m_uniformFloats.size() != m_expectedFloatCount) {
        m_uniformFloats.resize(m_expectedFloatCount, 0.0f);  // 用 0 填充到期望大小
    }

    // Get framebuffer size
    // 获取帧缓冲区尺寸
    int width = 0, height = 0;
    backend->GetFramebufferSize(width, height);  // 查询当前帧缓冲区大小
    if (width <= 0 || height <= 0) return;  // 尺寸无效则跳过
    m_viewportWidth = width;   // 缓存视口宽度
    m_viewportHeight = height; // 缓存视口高度

    if (m_compareMode) {  // 对比模式开启
        RenderCompareView(backend);  // 渲染到 FBO 纹理，由 ImGui 合成显示
    } else {
        RenderFullscreenEffect(backend);  // 直接全屏渲染特效
    }
}
```

#### 功能说明

`OnRender` 是每帧渲染的入口，负责同步调试面板参数、获取视口尺寸，然后根据是否开启对比模式选择渲染路径。

#### 实现原理

渲染分为两条路径：
1. **对比模式**：先将特效渲染到离屏 FBO 纹理（`m_effectTex`），然后由 `OnImGui` 中的 ImGui 绘图指令将原图和特效图合成为 Before/After 分割视图。
2. **全屏模式**：直接将特效渲染到屏幕帧缓冲区。

参数同步在渲染前执行，确保用户在调试面板中的修改立即反映到渲染结果。

#### 为什么这样实现

- **参数校验与修复**：`m_expectedFloatCount` 机制防止 DebugPanel 内部操作（如删除参数）导致 float 数组长度不匹配 UBO 布局，避免越界写入。
- **对比模式离屏渲染**：将特效结果渲染到独立纹理，再由 ImGui 合成，可以利用 ImGui 的裁剪和混合功能实现平滑的分割线效果，无需编写额外的合成着色器。

---

### 4.8 RenderFullscreenEffect — 全屏特效渲染

```cpp
void EffectDetailScene::RenderFullscreenEffect(IRenderBackend* backend)
{
    ShaderParams params;  // 创建着色器参数结构体
    params.inputTextures.push_back(m_inputTex);  // 添加输入纹理（原图或视频帧）
    params.uniformFloats = m_uniformFloats;        // 设置 float uniform 参数
    params.uniformInts = m_uniformInts;            // 设置 int uniform 参数
    params.time = m_time;                           // 设置时间
    params.frameCount = m_frameCount;               // 设置帧计数
    params.viewportWidth = m_viewportWidth;         // 设置视口宽度
    params.viewportHeight = m_viewportHeight;       // 设置视口高度

    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
    // 调用后端接口绘制全屏四边形，执行后处理特效
}
```

#### 功能说明

将所有渲染所需参数打包为 `ShaderParams` 结构体，委托给渲染后端的 `DrawFullscreenQuad` 方法执行实际的全屏后处理渲染。

#### 实现原理

`ShaderParams` 是一个纯数据结构，包含纹理、uniform、时间、帧计数和视口尺寸。后端接收此结构体后，会绑定着色器程序、设置 uniform/UBO、绑定纹理，然后绘制一个覆盖整个屏幕的四边形（两个三角形），片段着色器对每个像素执行后处理计算。

#### 为什么这样实现

- **参数打包模式**：将多个零散参数打包为结构体，接口清晰且易于扩展。新增参数只需修改结构体定义，不影响函数签名。
- **后端抽象**：`EffectDetailScene` 不直接调用任何 GL API，通过 `IRenderBackend` 接口委托渲染，使场景代码与具体图形 API 解耦。

---

### 4.9 RenderCompareView — 对比视图渲染

```cpp
void EffectDetailScene::RenderCompareView(IRenderBackend* backend)
{
    // 1. Ensure effect texture FBO exists and matches size
    // 步骤1：确保特效纹理 FBO 存在且尺寸匹配
    EnsureEffectTexture();  // 懒创建 FBO 纹理（首次调用时创建）

    if (m_effectTex.id == INVALID_TEXTURE.id) return;  // FBO 纹理无效则跳过

    // 2. Render effect to FBO texture
    // 步骤2：将特效渲染到 FBO 纹理（离屏渲染）
    backend->BeginRenderToTexture(m_effectTex);  // 绑定 FBO 为渲染目标
    RenderFullscreenEffect(backend);               // 执行全屏特效渲染（输出到 FBO）
    backend->EndRenderToTexture();                 // 解绑 FBO，恢复默认帧缓冲区

    // 3. ImGui will handle the compare view display in OnImGui
    // 步骤3：ImGui 将在 OnImGui 中处理对比视图的显示
    // （原图和特效图通过 ImGui DrawList 合成为 Before/After 分割视图）
}
```

#### 功能说明

对比视图渲染分三步：(1) 确保 FBO 纹理已创建；(2) 将特效渲染到离屏 FBO；(3) 实际的视觉合成由 `OnImGui` 中的 ImGui 绘图指令完成。

#### 实现原理

离屏渲染（Off-screen Rendering）通过 FBO（Frame Buffer Object）实现。`BeginRenderToTexture` 将渲染目标从屏幕切换到 FBO 附着的纹理，`RenderFullscreenEffect` 在此纹理上执行特效计算，`EndRenderToTexture` 恢复屏幕渲染。之后 ImGui 的 `AddImage` 指令将两张纹理（原图 + 特效结果）绘制到屏幕上，通过 UV 坐标裁剪实现分割效果。

#### 为什么这样实现

- **渲染与展示分离**：特效计算在 GL 着色器中完成（GPU 加速），视觉合成在 ImGui 中完成（CPU 灵活控制），各取所长。
- **懒创建 FBO**：`EnsureEffectTexture` 仅在首次需要时创建纹理，避免在非对比模式下浪费 GPU 内存。
- **无需合成着色器**：利用 ImGui 的 `AddImage` 天然支持 UV 裁剪，无需编写额外的 Before/After 合成着色器，简化了代码。

---

### 4.10 EnsureEffectTexture — 确保 FBO 纹理已创建

```cpp
void EffectDetailScene::EnsureEffectTexture()
{
    if (m_effectTexCreated || !m_backend) return;  // 已创建或后端无效则跳过
    int w = 0, h = 0;
    m_backend->GetFramebufferSize(w, h);  // 获取当前帧缓冲区尺寸
    if (w <= 0 || h <= 0) return;         // 尺寸无效则跳过（窗口可能最小化）
    m_effectTex = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    // 创建与帧缓冲区同尺寸的 RGBA8 纹理（初始数据为 nullptr，即未初始化）
    m_effectTexCreated = true;  // 标记为已创建
}
```

#### 功能说明

懒创建对比模式所需的 FBO 纹理。纹理尺寸与当前帧缓冲区一致，格式为 RGBA8（每个通道 8 位，满足大多数后处理需求）。

#### 实现原理

懒初始化（Lazy Initialization）模式。仅在首次进入对比模式时创建纹理，避免在不需要时浪费 GPU 内存。传入 `nullptr` 作为初始数据，因为纹理会在每帧被特效渲染结果覆盖。

#### 为什么这样实现

- **按需分配**：用户可能从不使用对比模式，懒创建避免不必要的 GPU 内存占用。
- **尺寸匹配**：FBO 纹理尺寸与屏幕一致，确保特效渲染的分辨率正确，避免缩放伪影。

---

### 4.11 OnImGui — UI 面板绘制

```cpp
void EffectDetailScene::OnImGui()
{
    // TAB to toggle debug panel visibility
    // TAB 键切换调试面板显示/隐藏
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_showDebug = !m_showDebug;  // 切换布尔值
    }
    // C to toggle compare mode
    // C 键切换对比模式
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_compareMode = !m_compareMode;  // 切换布尔值
    }

    // Get framebuffer size for positioning
    // 获取帧缓冲区尺寸，用于窗口定位
    int width = 0, height = 0;
    if (m_backend) {
        m_backend->GetFramebufferSize(width, height);
    }

    // --- Compare mode: slider before/after overlay view ---
    // --- 对比模式：滑块式 Before/After 叠加视图 ---
    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id && m_backend) {
        // 条件：对比模式开启 && 特效纹理有效 && 后端有效
        void* origImTex = m_backend->GetImTextureID(m_inputTex);   // 获取原图的 ImGui 纹理 ID
        void* effectImTex = m_backend->GetImTextureID(m_effectTex); // 获取特效图的 ImGui 纹理 ID

        if (origImTex && effectImTex) {  // 两个纹理 ID 都有效
            // 创建全屏无边框窗口作为对比视图的画布
            ImGui::SetNextWindowPos(ImVec2(0, 0));  // 窗口位置：屏幕左上角
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));  // 窗口大小：全屏
            ImGui::Begin("##CompareView", nullptr,  // "##" 前缀表示隐藏标题栏
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
                // 窗口标志：无标题栏、不可调整大小、不可移动、无滚动条、
                // 无背景（透明）、点击时不置顶

            // Calculate display region
            // 计算显示区域（排除窗口边框/标题栏后的内容区域）
            ImVec2 winPos   = ImGui::GetWindowPos();              // 窗口位置
            ImVec2 regionMin = ImGui::GetWindowContentRegionMin(); // 内容区域最小坐标
            ImVec2 regionMax = ImGui::GetWindowContentRegionMax(); // 内容区域最大坐标
            ImVec2 displayMin = ImVec2(winPos.x + regionMin.x, winPos.y + regionMin.y); // 显示区域左上角（屏幕坐标）
            ImVec2 displayMax = ImVec2(winPos.x + regionMax.x, winPos.y + regionMax.y); // 显示区域右下角（屏幕坐标）
            ImVec2 displaySize(displayMax.x - displayMin.x, displayMax.y - displayMin.y); // 显示区域尺寸

            ImDrawList* dl = ImGui::GetWindowDrawList();  // 获取窗口绘图列表（用于自定义绘制）

            // Split position in screen coordinates
            // 分割线在屏幕坐标中的 X 位置
            float splitX = displayMin.x + displaySize.x * m_compareSplitPos;

            // Draw original image as the base layer (full display area)
            // 绘制原图作为底层（覆盖整个显示区域）
            // GL textures have bottom-left origin, ImGui expects top-left -> flip Y
            // GL 纹理原点在左下角，ImGui 期望左上角 -> 翻转 Y 轴 UV
            dl->AddImage(origImTex, displayMin, displayMax, ImVec2(0, 1), ImVec2(1, 0));
            // UV (0,1) 到 (1,0)：翻转 Y，使图像正确显示

            // Draw effect image on the right side of the split (clipped)
            // 在分割线右侧绘制特效图（通过 UV 裁剪实现）
            // UV mapping: the left edge of the effect image maps to splitX
            // UV 映射：特效图像的左边缘对应分割线位置
            float uvMinX = m_compareSplitPos;  // UV 的 X 最小值 = 分割位置比例
            dl->AddImage(effectImTex,
                ImVec2(splitX, displayMin.y), displayMax,  // 屏幕坐标：从分割线到右下角
                ImVec2(uvMinX, 1), ImVec2(1, 0));  // UV 坐标：从分割比例到右下角，Y 翻转

            // Draw split line (blue, 3px)
            // 绘制分割线（蓝色，3 像素宽）
            dl->AddLine(ImVec2(splitX, displayMin.y), ImVec2(splitX, displayMax.y),
                        IM_COL32(68, 175, 255, 255), 3.0f);  // RGBA(68,175,255) 蓝色

            // Draw drag handle (circle at center of display height)
            // 绘制拖动手柄（显示区域高度中心的圆形）
            float handleY = (displayMin.y + displayMax.y) * 0.5f;  // 手柄 Y 坐标 = 垂直中心
            dl->AddCircleFilled(ImVec2(splitX, handleY), 14.0f, IM_COL32(68, 175, 255, 255));
            // 外圈：蓝色，半径 14px
            dl->AddCircleFilled(ImVec2(splitX, handleY), 12.0f, IM_COL32(255, 255, 255, 255));
            // 内圈：白色，半径 12px

            // Draw handle icon (left/right arrows)
            // 绘制手柄图标（左右箭头）
            dl->AddTriangleFilled(
                ImVec2(splitX - 6, handleY - 3), ImVec2(splitX - 2, handleY), ImVec2(splitX - 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));  // 左箭头（三角形）
            dl->AddTriangleFilled(
                ImVec2(splitX + 6, handleY - 3), ImVec2(splitX + 2, handleY), ImVec2(splitX + 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));  // 右箭头（三角形）

            // Labels: "Before" on left, "After" on right
            // 标签：左侧 "Before"，右侧 "After"
            dl->AddText(ImVec2(displayMin.x + 10, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "Before");  // 半透明白色文字
            dl->AddText(ImVec2(displayMax.x - 60, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "After");   // 半透明白色文字

            // Handle mouse interaction for dragging the split
            // 处理鼠标拖动分割线的交互
            ImVec2 mousePos = ImGui::GetIO().MousePos;  // 获取鼠标位置
            bool mouseInWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
            // 检查鼠标是否在窗口内（包括子窗口）

            // Check if mouse is near the split line
            // 检查鼠标是否靠近分割线
            bool nearSplit = mouseInWindow &&
                std::abs(mousePos.x - splitX) < 20.0f &&  // 距离分割线 < 20px
                mousePos.y >= displayMin.y && mousePos.y <= displayMax.y;  // 在垂直范围内

            // Set cursor to resize east-west when near split
            // 靠近分割线时设置鼠标光标为东西调整大小样式
            if (nearSplit || m_compareDragging) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);  // 光标
            }

            // Handle drag start (on mouse down)
            // 处理拖动开始（鼠标按下时）
            if (nearSplit && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_compareDragging = true;  // 开始拖动
            }

            // Handle drag update
            // 处理拖动更新（鼠标移动时）
            if (m_compareDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                float newSplit = (mousePos.x - displayMin.x) / displaySize.x;  // 计算新的分割比例
                m_compareSplitPos = std::clamp(newSplit, 0.1f, 0.9f);  // 限制在 10%~90% 范围内
            }

            // Handle drag end
            // 处理拖动结束（鼠标释放时）
            if (m_compareDragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_compareDragging = false;  // 结束拖动
            }

            ImGui::End();  // 结束对比视图窗口
        }
    }

    // --- InfoBar at bottom of screen ---
    // --- 屏幕底部的信息栏 ---
    {
        const float barHeight = 60.0f;  // 信息栏高度 60px
        ImGui::SetNextWindowPos(ImVec2(0, (float)height - barHeight));  // 位置：屏幕底部
        ImGui::SetNextWindowSize(ImVec2((float)width, barHeight));       // 大小：全宽 x 60px
        ImGui::Begin("##InfoBar", nullptr,  // 隐藏标题栏
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);  // 不保存设置（避免影响布局）
        ImGui::TextColored(ImVec4(1, 1, 1, 0.9f), "%s", LanguageManager::Instance().CardName(m_card.id));
        // 显示特效名称（白色，90% 不透明度），通过 LanguageManager 支持多语言
        ImGui::SameLine();  // 在同一行继续
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 0.7f), "%s", LanguageManager::Instance().EscReturn());
        // 显示 "按 ESC 返回" 提示（灰色，70% 不透明度）
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", LanguageManager::Instance().CardDesc(m_card.id));
        // 显示特效描述（浅灰色，80% 不透明度）
        ImGui::End();
    }

    // --- Debug Panel ---
    // --- 调试面板 ---
    if (m_showDebug) {  // 调试面板可见
        ImGui::Begin(LanguageManager::Instance().EffectParams(), &m_showDebug);
        // 窗口标题从 LanguageManager 获取，&m_showDebug 允许通过关闭按钮隐藏
        m_debugPanel.Render(&m_showDebug);  // 渲染调试面板内容（滑块、颜色选择器等）
        ImGui::End();
        // Update uniform values after UI interaction
        // UI 交互后更新 uniform 值
        m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
        if (m_uniformFloats.size() != m_expectedFloatCount) {
            m_uniformFloats.resize(m_expectedFloatCount, 0.0f);  // 校验并修复数组长度
        }
    }

    // --- Asset Library Panel ---
    // --- 资产库面板 ---
    if (m_showDebug && !m_savedState.imagePool.empty()) {
        // 条件：调试面板可见 && 图片池非空
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
        // 首次使用时的默认大小 250x300
        if (ImGui::Begin(LanguageManager::Instance().AssetLibrary(), &m_showDebug)) {
            // Images section
            // 图片部分
            if (ImGui::CollapsingHeader(LanguageManager::Instance().Images())) {
                // 可折叠标题："图片"
                for (size_t i = 0; i < m_savedState.imagePool.size(); i++) {
                    const std::string& path = m_savedState.imagePool[i];  // 获取图片路径
                    // Extract filename for display
                    // 提取文件名用于显示
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");  // 查找最后一个路径分隔符
                    if (pos != std::string::npos) fname = path.substr(pos + 1);  // 截取文件名
                    if (ImGui::SmallButton(fname.c_str())) {  // 小按钮显示文件名
                        LoadImageFromFile(path);  // 点击后加载该图片
                    }
                }
            }
            // Videos section
            // 视频部分
            if (!m_savedState.videoPool.empty() && ImGui::CollapsingHeader(LanguageManager::Instance().Videos())) {
                // 条件：视频池非空 && 可折叠标题展开
                for (size_t i = 0; i < m_savedState.videoPool.size(); i++) {
                    const std::string& path = m_savedState.videoPool[i];
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadVideoFromFile(path);  // 点击后加载该视频
                    }
                }
            }
        }
        ImGui::End();
    }

    // ---- Detail page screenshot mode ----
    // ---- 详情页截图模式（自动测试用） ----
    if (getenv("AUTO_TEST_DETAILS")) {  // 检测环境变量，启用自动截图
        static int detailScreenshotIndex = 0;  // 截图序号（static 跨实例保持）
        static int frameCounter = 0;            // 帧计数器
        static bool needsScreenshot = true;    // 是否需要截图（首帧为 true）

        frameCounter++;

        // Take screenshot after UI renders
        // 在 UI 渲染后截图（等待 5 帧让 UI 稳定）
        if (needsScreenshot && frameCounter >= 5) {
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/detail_%02d.ppm", detailScreenshotIndex);
            // 生成截图文件路径
            ScreenshotRequest::Request(path);  // 请求截图（在帧结束时执行）
            printf("[DetailScreenshot] Requested screenshot: %s\n", path);
            needsScreenshot = false;  // 已请求，标记为不需要
        }

        // Exit after showing for a while
        // 显示一段时间后退出
        if (frameCounter >= 30) {  // 30 帧后退出（约 0.5 秒 @ 60fps）
            frameCounter = 0;       // 重置帧计数器
            detailScreenshotIndex++;  // 递增截图序号
            if (detailScreenshotIndex < 18) {  // 还有更多特效要截图
                needsScreenshot = true;  // 下一张需要截图
            }
            // Return to coverflow
            // 返回封面流
            m_returnToCoverFlow = true;
            m_wantsExit = true;
        }
    }
}
```

#### 功能说明

`OnImGui` 是 UI 渲染的核心，包含五大功能区域：(1) 对比模式的 Before/After 分割视图（含拖动交互）；(2) 底部信息栏（特效名称和操作提示）；(3) 调试参数面板（滑块/颜色选择器）；(4) 资产库面板（快速切换输入图片/视频）；(5) 自动测试截图模式。

#### 实现原理

**对比视图**使用 ImGui 的 `ImDrawList` 自定义绘制。通过两次 `AddImage` 调用叠加原图和特效图，利用 UV 坐标裁剪实现分割效果。原图绘制为全屏底层，特效图只绘制分割线右侧部分（UV 的 `uMinX = m_compareSplitPos`）。拖动交互通过三态状态机实现：`nearSplit && MouseClicked` -> `Dragging && MouseDown` -> `Dragging && MouseReleased`。

**信息栏**使用固定位置的 ImGui 窗口，通过 `LanguageManager` 实现多语言支持。

**调试面板**委托给 `DebugPanel::Render`，参数修改后立即通过 `SetUniformValues` 同步到 `m_uniformFloats`/`m_uniformInts`。

#### 为什么这样实现

- **UV 裁剪 vs 着色器裁剪**：使用 ImGui 的 `AddImage` UV 裁剪而非编写专门的合成着色器，代码更简洁，且天然支持 ImGui 的窗口管理和输入处理。
- **Y 轴翻转**：OpenGL 纹理坐标原点在左下角（Y=0 在底部），而 ImGui 屏幕坐标原点在左上角（Y=0 在顶部）。`AddImage` 的 UV 参数 `(0,1)` 到 `(1,0)` 实现了 Y 轴翻转，使图像正确显示。
- **拖动范围限制**：`std::clamp(newSplit, 0.1f, 0.9f)` 防止分割线被拖到极端位置导致一侧完全不可见。
- **static 变量**：自动测试的截图序号和帧计数器使用 `static`，跨场景实例保持连续性，确保每个特效按序截图。

---

### 4.12 GetNextScene — 返回封面流与状态恢复

```cpp
std::unique_ptr<Scene> EffectDetailScene::GetNextScene()
{
    if (m_returnToCoverFlow) {  // 如果需要返回封面流
        printf("[EffectDetailScene] Restoring CoverFlowScene with full state\n");

        auto coverFlow = std::make_unique<CoverFlowScene>();  // 创建新的封面流场景
        coverFlow->SetInputTexture(m_savedState.inputTex);  // 恢复输入纹理
        coverFlow->SetInputTexCache(m_savedState.inputTexCache);  // 恢复纹理缓存
        coverFlow->SetBackend(m_savedState.backend);  // 恢复后端指针
        coverFlow->SetApplication(m_savedState.app);  // 恢复应用指针
        // Thumbnails are now initialized internally by CoverFlowScene::OnEnter
        // 缩略图现在由 CoverFlowScene::OnEnter 内部初始化
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);  // 恢复测试图片目录

        // Restore selected card index so user returns to the same card
        // 恢复选中的卡片索引，使用户返回到同一张卡片
        coverFlow->SetSelectedIndex(m_savedState.selectedIndex);

        // Restore image pool
        // 恢复图片池（用户通过拖放添加的图片）
        for (const auto& img : m_savedState.imagePool) {
            coverFlow->AddImageToPool(img);  // 逐个添加图片路径到池中
        }

        // Restore auto-test state
        // 恢复自动测试状态
        if (m_savedState.autoTest) {  // 如果之前在自动测试模式
            coverFlow->ResumeAutoTest(m_savedState.autoTestHoldFrames, m_savedState.autoTestCardIndex);
            // 恢复自动测试的剩余帧数和当前卡片索引
        }

        // Transfer video player back to CoverFlowScene
        // 将视频播放器转移回 CoverFlowScene
        if (m_videoActive && m_videoPlayer) {  // 如果视频正在播放
            coverFlow->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
            // 移动转移视频播放器所有权回封面流
            m_videoActive = false;  // 标记为不再活跃
            m_videoTex = {0};        // 重置纹理句柄
            printf("[EffectDetailScene] Transferred video player back to CoverFlow\n");
        }

        printf("[EffectDetailScene] CoverFlowScene restored (thumbs=%zu, pool=%zu)\n",
               m_savedState.thumbIds.size(), m_savedState.imagePool.size());
        // 日志输出恢复的状态信息
        return coverFlow;  // 返回新创建的封面流场景
    }
    return nullptr;  // 不需要切换场景则返回空
}
```

#### 功能说明

在用户按 ESC 或自动测试计时器到期时，创建新的 `CoverFlowScene` 并恢复之前保存的完整状态，包括选中索引、图片池、视频播放器、自动测试进度等。

#### 实现原理

采用"销毁-重建"模式而非"暂停-恢复"模式。每次返回封面流都创建全新的 `CoverFlowScene` 对象，然后通过一系列 `Set*` 方法恢复状态。`CoverFlowState` 结构体作为状态快照，在进入详情页时保存，在返回时用于恢复。

视频播放器通过 `std::move` 在两个场景间转移所有权，确保同一时间只有一个场景持有播放器。

#### 为什么这样实现

- **销毁-重建 vs 暂停-恢复**：销毁-重建模式更简单，避免了复杂的状态管理。`CoverFlowScene` 的 `OnEnter` 会重新初始化缩略图等资源，确保状态一致性。
- **状态快照**：`CoverFlowState` 结构体是轻量的值类型（包含句柄、索引、路径等），复制成本低。
- **视频播放器连续性**：通过所有权转移，视频在场景切换时无缝继续播放，用户不会感知到中断。

---

### 4.13 LoadImageFromFile — 从文件加载图片

```cpp
void EffectDetailScene::LoadImageFromFile(const std::string& path)
{
    if (!m_backend) return;  // 后端无效则跳过

    int iw, ih, comp;  // 图片宽度、高度、通道数（stb_image 输出参数）
    stbi_set_flip_vertically_on_load(true);  // 设置 stb_image 加载时垂直翻转
    // OpenGL 纹理坐标原点在左下角，图片文件通常原点在左上角，需要翻转
    stbi_uc* data = stbi_load(path.c_str(), &iw, &ih, &comp, 4);
    // 加载图片，强制 4 通道（RGBA），comp 返回原始通道数
    stbi_set_flip_vertically_on_load(false);  // 恢复默认设置（不翻转）

    if (!data) {  // 加载失败
        fprintf(stderr, "[EffectDetailScene] Cannot load: %s\n", path.c_str());
        return;
    }

    printf("[EffectDetailScene] Loading image: %s (%d x %d)\n", path.c_str(), iw, ih);

    TextureHandle newTex = m_backend->CreateTexture(iw, ih, TextureFormat::RGBA8, data);
    // 创建 GL 纹理，传入图片像素数据
    stbi_image_free(data);  // 释放 stb_image 分配的像素数据

    if (newTex.id != INVALID_TEXTURE.id) {  // 纹理创建成功
        // Destroy old input texture to prevent leak
        // 销毁旧的输入纹理以防止内存泄漏
        if (m_inputTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_inputTex);
        }
        m_inputTex = newTex;  // 更新输入纹理为新加载的图片
        // Also update saved state so CoverFlow gets the new texture
        // 同时更新保存的状态，使封面流也能获得新纹理
        m_savedState.inputTex = newTex;
    }
}
```

#### 功能说明

使用 stb_image 库加载图片文件，创建 GL 纹理并替换当前输入纹理。支持拖放和资产库面板触发的图片加载。

#### 实现原理

stb_image 是一个单头文件图片加载库，支持 JPG、PNG、BMP 等常见格式。`stbi_load` 的第 5 个参数 `4` 表示强制输出 RGBA 格式，与 `TextureFormat::RGBA8` 匹配。`stbi_set_flip_vertically_on_load(true)` 在加载时翻转 Y 轴，使图片数据与 OpenGL 纹理坐标方向一致。

#### 为什么这样实现

- **stb_image 选择**：轻量级、零依赖、支持格式广泛，适合工具类应用。
- **旧纹理销毁**：每次加载新图片都销毁旧纹理，防止 GPU 内存泄漏。这是必要的，因为用户可能反复拖放不同图片。
- **状态同步**：更新 `m_savedState.inputTex` 确保返回封面流时使用最新的输入纹理。

---

### 4.14 LoadVideoFromFile / StopVideo — 视频加载与停止

```cpp
void EffectDetailScene::LoadVideoFromFile(const std::string& path)
{
    if (!m_backend) return;  // 后端无效则跳过

    // Stop any existing video
    // 停止当前正在播放的视频（如果有）
    StopVideo();

    m_videoPlayer = std::make_unique<VideoPlayer>();  // 创建新的视频播放器
    if (!m_videoPlayer->Open(path)) {  // 打开视频文件（启动 FFmpeg 管道）
        fprintf(stderr, "[EffectDetailScene] Cannot open video: %s\n", path.c_str());
        m_videoPlayer.reset();  // 打开失败，释放播放器
        return;
    }

    // Create texture for video frames
    // 为视频帧创建纹理
    m_videoTex = m_backend->CreateTexture(
        m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),  // 视频尺寸
        TextureFormat::RGBA8, m_videoPlayer->GetPixels());  // RGBA8 格式，初始帧数据

    if (m_videoTex.id == INVALID_TEXTURE.id) {  // 纹理创建失败
        fprintf(stderr, "[EffectDetailScene] Cannot create video texture\n");
        m_videoPlayer->Close();  // 关闭视频
        m_videoPlayer.reset();   // 释放播放器
        return;
    }

    m_inputTex = m_videoTex;  // 将视频纹理设为特效输入
    m_videoActive = true;      // 标记视频为活跃状态
    m_videoLastFrameTime = ImGui::GetTime();  // 初始化帧时间戳
    printf("[EffectDetailScene] Playing video: %s\n", path.c_str());
}

void EffectDetailScene::StopVideo()
{
    if (m_videoPlayer) {  // 播放器存在
        m_videoPlayer->Close();  // 关闭 FFmpeg 管道
        m_videoPlayer.reset();   // 释放播放器对象（unique_ptr）
    }
    m_videoActive = false;  // 标记视频为非活跃
    // Note: we don't destroy m_videoTex here because it might still be referenced
    // A proper implementation would use reference counting.
    // 注意：此处不销毁 m_videoTex，因为它可能仍被引用
    // 更完善的实现应使用引用计数
}
```

#### 功能说明

`LoadVideoFromFile` 打开视频文件并创建对应的 GL 纹理。视频通过 FFmpeg 管道解码，以 30fps 输出帧到纹理，特效着色器实时处理每一帧。`StopVideo` 停止视频播放并释放播放器资源。

#### 实现原理

`VideoPlayer::Open` 启动一个 FFmpeg 子进程，通过管道接收解码后的 RGB 帧数据。第一帧在 `Open` 时就已可用，用于初始化纹理。后续帧在 `OnUpdate` 中按 30fps 节流读取。

#### 为什么这样实现

- **先停后建**：加载新视频前先调用 `StopVideo()`，确保旧的 FFmpeg 管道被正确关闭，避免资源泄漏。
- **首帧初始化**：使用第一帧数据初始化纹理，避免显示空白帧。
- **纹理生命周期解耦**：`StopVideo` 不销毁视频纹理，因为它可能仍作为 `m_inputTex` 被特效着色器引用。纹理在场景退出时统一销毁，不会泄漏。

---

## 5. OpenGL 渲染后端

`OpenGLBackend` 是 `IRenderBackend` 接口的 OpenGL 4.6 实现，负责着色器编译（SPIR-V）、纹理管理、全屏四边形渲染、FBO 离屏渲染、3D 卡片渲染和 ImGui 集成。

### 5.1 IRenderBackend 接口定义

```cpp
#pragma once

#include "BackendType.h"  // BackendType 枚举（OpenGL / Vulkan）

#include <cstdint>
#include <vector>
#include <string>

// Forward declaration
// 前向声明：GLFW 窗口句柄
struct GLFWwindow;

// Pipeline description for creating pipelines
// 管线描述结构体，用于创建渲染管线
struct PipelineDesc {
    ShaderHandle vertShader;  // 顶点着色器句柄
    ShaderHandle fragShader;  // 片段着色器句柄
    int width;                 // 渲染宽度
    int height;                // 渲染高度
    bool blendEnable = true;   // 是否启用混合（默认启用）
};

// ---------------------------------------------------------------------------
// ShaderParams — data passed per fullscreen-quad draw
// ---------------------------------------------------------------------------
// ShaderParams — 每次全屏四边形绘制时传递的数据
struct ShaderParams {
    std::vector<TextureHandle> inputTextures;  // 输入纹理数组（最多 8 张）
    std::vector<float>         uniformFloats;   // float uniform 参数数组
    std::vector<int32_t>       uniformInts;     // int uniform 参数数组
    int   viewportWidth  = 1280;  // 视口宽度（默认 1280）
    int   viewportHeight = 720;   // 视口高度（默认 720）
    float time           = 0.0f;  // 时间（秒）
    uint32_t frameCount  = 0;     // 帧计数
};

// ---------------------------------------------------------------------------
// IRenderBackend — pure virtual render backend interface
// ---------------------------------------------------------------------------
// IRenderBackend — 纯虚渲染后端接口
class IRenderBackend {
public:
    // ---- nested CardDrawInfo -----------------------------------------------
    // 嵌套结构体：3D 卡片绘制信息
    struct CardDrawInfo {
        TextureHandle texture;  // 卡片纹理
        float posX, posY, posZ;      // 位置 (X, Y, Z)
        float scaleX, scaleY;        // 缩放 (X, Y)
        float rotationY;             // Y 轴旋转角度（弧度）
        float opacity;               // 不透明度 (0.0 ~ 1.0)
    };

    virtual ~IRenderBackend() = default;  // 虚析构函数（确保正确释放派生类）

    // ---- Lifecycle ---------------------------------------------------------
    virtual bool Init(GLFWwindow* window) = 0;
    virtual void Shutdown()               = 0;
    virtual void BeginFrame()             = 0;
    virtual void EndFrame()               = 0;
    virtual void WaitIdle()               = 0;

    // ---- Viewport ----------------------------------------------------------
    virtual void Resize(int width, int height)              = 0;
    virtual void GetFramebufferSize(int& width, int& height) = 0;

    // ---- Shaders -----------------------------------------------------------
    virtual ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size)   = 0;
    virtual ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) = 0;
    virtual ShaderHandle CreateVertexShaderFromGLSL(const std::string& source)    = 0;
    virtual ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source)  = 0;
    virtual void         DestroyShader(ShaderHandle handle)                        = 0;

    // ---- Textures ----------------------------------------------------------
    virtual TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) = 0;
    virtual void          UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) = 0;
    virtual void          DestroyTexture(TextureHandle handle)                                        = 0;
    virtual void*         GetImTextureID(TextureHandle handle)                                        = 0;

    // ---- Pipelines ---------------------------------------------------------
    virtual PipelineHandle CreatePipeline(const PipelineDesc& desc) = 0;
    virtual void           DestroyPipeline(PipelineHandle handle)  = 0;
    virtual void           BindPipeline(PipelineHandle handle)     = 0;

    // ---- Fullscreen quad ---------------------------------------------------
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) = 0;

    // ---- Cards (3D card rendering) -----------------------------------------
    virtual void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) = 0;

    // ---- Blit --------------------------------------------------------------
    virtual void BlitToScreen(TextureHandle src) = 0;

    // ---- Render targets ----------------------------------------------------
    virtual void BeginRenderToTexture(TextureHandle target) = 0;
    virtual void EndRenderToTexture()                       = 0;

    // ---- Utility -----------------------------------------------------------
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void Clear(float r, float g, float b, float a)        = 0;

    // ---- ImGui -------------------------------------------------------------
    virtual void ImGuiInit(GLFWwindow* window) = 0;
    virtual void ImGuiNewFrame()               = 0;
    virtual void ImGuiRender()                 = 0;
    virtual void ImGuiShutdown()               = 0;

    // ---- Query -------------------------------------------------------------
    virtual BackendType GetType()                     const = 0;
    virtual const char* GetName()                     const = 0;
    virtual int         GetMaxTextureSize()           const = 0;
};
```

#### 功能说明

`IRenderBackend` 是渲染后端的抽象接口，定义了所有后端（OpenGL、Vulkan）必须实现的操作。涵盖生命周期管理、着色器/纹理/管线资源管理、全屏渲染、3D 卡片渲染、FBO 离屏渲染和 ImGui 集成。

#### 实现原理

采用纯虚接口（Pure Virtual Interface）模式，所有方法都是纯虚函数（`= 0`）。通过指针或引用调用，运行时多态决定实际执行的是 OpenGL 还是 Vulkan 实现。

`ShaderParams` 和 `PipelineDesc` 是值类型结构体，作为接口的数据传输对象（DTO），封装了渲染所需的全部参数。

#### 为什么这样实现

- **后端可替换性**：接口抽象使 OpenGL 和 Vulkan 后端可互换，上层代码（场景、应用）无需修改。
- **最小接口原则**：接口只包含上层需要的操作，不暴露 GL/Vulkan 特有细节（如 VAO、FBO、CommandBuffer）。
- **ShaderParams 打包**：将多个零散参数打包为结构体，接口更清晰，新增参数不影响函数签名。

---

### 5.2 OpenGLBackend 头文件

```cpp
#pragma once

#include "IRenderBackend.h"  // 渲染后端抽象接口

#include <GLFW/glfw3.h>  // GLFW 窗口管理库

#include <string>
#include <unordered_map>  // 哈希表（用于着色器/纹理/程序缓存）
#include <vector>

class OpenGLBackend : public IRenderBackend {
public:
    OpenGLBackend()  = default;   // 默认构造函数
    ~OpenGLBackend() override = default;  // 默认析构函数（实际清理在 Shutdown 中）

    // ---- Lifecycle / Viewport / Shaders / Textures / Pipelines ----
    // （省略重复的接口声明，与 IRenderBackend 完全对应）

    /// Save current framebuffer to PPM file for debugging.
    void SaveScreenshot(const char* path) const;

    // ---- Query -------------------------------------------------------------
    BackendType GetType()           const override { return BackendType::OpenGL; }
    const char* GetName()           const override { return "OpenGL 4.6 (SPIR-V)"; }
    int         GetMaxTextureSize() const override;

private:
    // ---- Internal helpers --------------------------------------------------
    GLuint GetGLShader(ShaderHandle handle) const;      // 从句柄获取 GL 着色器对象 ID
    GLuint GetGLTexture(TextureHandle handle) const;     // 从句柄获取 GL 纹理对象 ID
    GLuint GetGLFramebuffer(TextureHandle textureHandle) const;  // 获取或创建 FBO

    /// Get or create a linked GL program from vs+fs handles. Cached.
    GLuint GetOrCreateProgram(GLuint vsKey, GLuint fsKey, GLuint vsGL, GLuint fsGL);

    static GLint  GLInternalFormat(TextureFormat fmt);  // 转换纹理内部格式
    static GLenum GLFormat(TextureFormat fmt);          // 转换纹理数据格式
    static GLenum GLType(TextureFormat fmt);            // 转换纹理数据类型

    void SetupQuadVAO();     // 设置全屏四边形 VAO/VBO/EBO
    void BindDefaultState(); // 设置默认 GL 状态

    // ---- Members -----------------------------------------------------------
    GLFWwindow* m_window     = nullptr;  // GLFW 窗口指针
    int         m_width      = 0;       // 帧缓冲区宽度
    int         m_height     = 0;       // 帧缓冲区高度

    // Shader pool: handle ID -> GL shader object
    // 着色器池：句柄 ID -> GL 着色器对象
    std::unordered_map<uint32_t, GLuint> m_shaders;
    uint32_t m_nextShaderId = 1;  // 下一个着色器句柄 ID（自增）

    // Program cache (key = vs_id << 32 | fs_id)
    // 程序缓存：复合键 = (顶点着色器ID << 32) | 片段着色器ID
    std::unordered_map<uint64_t, GLuint> m_programCache;

    // Texture pool
    std::unordered_map<uint32_t, GLuint> m_textures;
    std::unordered_map<uint32_t, TextureFormat> m_textureFormats;
    std::unordered_map<uint32_t, int> m_texWidths;   // 纹理宽度记录
    std::unordered_map<uint32_t, int> m_texHeights;  // 纹理高度记录
    uint32_t m_nextTextureId = 1;

    // Framebuffer pool (keyed by texture id)
    std::unordered_map<uint32_t, GLuint> m_framebuffers;

    // Temp UBO for per-draw uniform data
    GLuint m_tempUBO = 0;

    // Fullscreen quad
    GLuint m_quadVAO = 0;  // 顶点数组对象
    GLuint m_quadVBO = 0;  // 顶点缓冲对象

    // Default framebuffer
    GLuint m_defaultFBO = 0;   // GLFW 创建的默认 FBO
    GLuint m_currentFBO  = 0;  // 当前绑定的 FBO
};
```

#### 功能说明

`OpenGLBackend` 头文件声明了 OpenGL 4.6 渲染后端的完整实现。它管理三类 GL 资源池（着色器、纹理、FBO），维护程序缓存以避免重复链接，并持有全屏四边形的几何数据。

#### 实现原理

资源管理采用"句柄池"模式。外部代码通过不透明的 `ShaderHandle`/`TextureHandle` 操作资源，内部通过 `unordered_map` 将句柄 ID 映射到 GL 对象 ID。句柄 ID 从 1 开始自增，0 保留为无效值。

程序缓存使用 64 位复合键 `(vs_id << 32) | fs_id`，确保同一对着色器只链接一次。

#### 为什么这样实现

- **句柄抽象**：外部代码不直接操作 GL 对象 ID，通过句柄间接访问。这使得后端可以自由管理 GL 对象的生命周期。
- **程序缓存**：着色器链接是昂贵操作（尤其 SPIR-V 特化）。缓存避免重复链接，显著提升性能。
- **unordered_map**：O(1) 平均查找复杂度，适合频繁的句柄->GL ID 查找。

---

### 5.3 Init — GL 上下文初始化

```cpp
// ============================================================================
// Uniform buffer layout matching SPIR-V shaders (std140)
// ============================================================================
// 与 SPIR-V 着色器匹配的 Uniform 缓冲区布局（std140 标准布局）
#pragma pack(push, 1)  // 设置 1 字节对齐（紧密打包）
struct UniformData {
    float uParamFloat0;       // offset 0  — 第 1 个 float 参数
    float uParamFloat1;       // offset 4  — 第 2 个 float 参数
    float uParamFloat2;       // offset 8  — 第 3 个 float 参数
    float uParamFloat3;       // offset 12 — 第 4 个 float 参数
    float uParamFloat4;       // offset 16 — 第 5 个 float 参数
    float uParamFloat5;       // offset 20 — 第 6 个 float 参数
    float uResolution[2];     // offset 24 (vec2, 8-byte aligned) — 分辨率 (宽, 高)
    float uTime;              // offset 32 — 时间（秒）
    float uFrameCount;        // offset 36 — 帧计数
    // Padding to 48 bytes (std140 rounds up to vec4 = 16-byte boundary)
    // 填充到 48 字节（std140 规则向上取整到 vec4 = 16 字节边界）
    float padding[3];         // offset 40, 3 floats = 12 bytes
}; // total: 48 bytes  // 总计 48 字节
#pragma pack(pop)  // 恢复默认对齐

// ============================================================================
// Fullscreen quad vertex data
// ============================================================================
// 全屏四边形顶点数据
static const float kQuadVertices[] = {
    // Position (2D)    // UV coords
    -1.0f, -1.0f,       0.0f, 0.0f,  // 左下角
     1.0f, -1.0f,       1.0f, 0.0f,  // 右下角
     1.0f,  1.0f,       1.0f, 1.0f,  // 右上角
    -1.0f,  1.0f,       0.0f, 1.0f   // 左上角
};

static const unsigned int kQuadIndices[] = {
    0, 1, 2,  // 第一个三角形
    0, 2, 3   // 第二个三角形
};

static constexpr int kFullscreenQuadVertexCount = 6;  // 2 个三角形 x 3 顶点

bool OpenGLBackend::Init(GLFWwindow* window) {
    m_window = window;  // 保存窗口指针
    if (!m_window) {  // 验证窗口有效
        fprintf(stderr, "[OpenGL] Invalid window handle\n");
        return false;
    }

    // Make context current
    glfwMakeContextCurrent(m_window);  // 激活 GL 上下文

    // Load OpenGL function pointers via glad
    if (!LoadGL46Functions(m_window)) {  // 加载 GL 4.6 函数指针
        fprintf(stderr, "[OpenGL] Failed to initialize GLAD\n");
        return false;
    }

    // Get initial framebuffer size
    glfwGetFramebufferSize(m_window, &m_width, &m_height);  // 获取帧缓冲区尺寸

    // Get the default FBO (the one GLFW created for us)
    GLint defaultFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFBO);  // 查询当前 FBO
    m_defaultFBO = static_cast<GLuint>(defaultFBO);  // 保存默认 FBO
    m_currentFBO = m_defaultFBO;

    // Create temp UBO for per-draw uniform data (fallback path)
    glGenBuffers(1, &m_tempUBO);  // 生成 UBO
    glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), nullptr, GL_DYNAMIC_DRAW);
    // 分配 48 字节 UBO，动态绘制模式
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Setup fullscreen quad VAO/VBO
    SetupQuadVAO();  // 设置全屏四边形几何

    // Set default OpenGL state
    BindDefaultState();  // 设置默认 GL 状态

    printf("[OpenGL] Initialized - GL Version: %s\n", (const char*)glGetString(GL_VERSION));
    return true;
}
```

#### 功能说明

`Init` 是 OpenGL 后端的初始化入口，负责：(1) 激活 GL 上下文；(2) 通过 GLAD 加载 OpenGL 4.6 函数指针；(3) 获取帧缓冲区尺寸和默认 FBO；(4) 创建临时 UBO；(5) 设置全屏四边形几何；(6) 设置默认 GL 状态。

#### 实现原理

GLAD 在运行时解析 GL 驱动导出的函数地址。`LoadGL46Functions` 加载所有 OpenGL 4.6 核心函数，使 `glCreateShader`、`glShaderBinary` 等 SPIR-V 相关函数可用。

默认 FBO 通过 `glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING)` 查询，而非假设为 0。这在某些平台上可能不是 0。

`UniformData` 结构体使用 `#pragma pack(push, 1)` 紧密打包，确保与 SPIR-V 着色器中声明的 std140 UBO 布局完全匹配。

#### 为什么这样实现

- **GLAD 而非 GLEW**：GLAD 支持 OpenGL 4.6 和 SPIR-V 扩展，生成更轻量的加载代码。
- **动态 UBO**：`GL_DYNAMIC_DRAW` 提示驱动此缓冲区每帧更新，驱动可能将其放置在 CPU 可快速写入的内存区域。
- **紧密打包**：`#pragma pack(push, 1)` 确保 C++ 结构体的内存布局与 GLSL std140 规则完全一致，避免对齐不匹配导致的 uniform 值错误。

---

### 5.4 CreateTexture — 纹理创建

```cpp
TextureHandle OpenGLBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    GLuint texture;  // GL 纹理对象 ID
    glGenTextures(1, &texture);  // 生成一个纹理对象
    if (texture == 0) {
        fprintf(stderr, "[OpenGL] Failed to create texture\n");
        return INVALID_TEXTURE;
    }

    glBindTexture(GL_TEXTURE_2D, texture);  // 绑定纹理

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // 水平边缘钳位
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // 垂直边缘钳位
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);    // 缩小：线性
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    // 放大：线性

    // Allocate texture storage
    GLint internalFormat = GLInternalFormat(format);  // 转换内部格式
    GLenum glFormat = GLFormat(format);                // 转换数据格式
    GLenum glType = GLType(format);                    // 转换数据类型

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, glType, data);
    // 分配存储并上传初始数据（data 可为 nullptr）

    glBindTexture(GL_TEXTURE_2D, 0);  // 解绑纹理

    uint32_t id = m_nextTextureId++;  // 分配新句柄 ID
    m_textures[id] = texture;         // 存入纹理池
    m_textureFormats[id] = format;     // 记录格式
    m_texWidths[id] = width;          // 记录宽度
    m_texHeights[id] = height;        // 记录高度

    return TextureHandle{id};  // 返回句柄
}
```

#### 功能说明

创建一个 2D 纹理对象，可选上传初始像素数据。设置边缘钳位和线性过滤参数，并将纹理注册到内部纹理池。

#### 实现原理

使用 `glTexImage2D` 一次性分配存储和上传数据。边缘钳位（`GL_CLAMP_TO_EDGE`）避免后处理特效在边缘出现接缝。线性过滤确保缩放时图像平滑。

#### 为什么这样实现

- **边缘钳位**：后处理特效采样边缘像素时，钳位比重复（REPEAT）更合适，避免不自然的重复图案。
- **线性过滤**：对于缩略图等缩放场景，线性过滤视觉效果更好。
- **格式记录**：`m_textureFormats`/`m_texWidths`/`m_texHeights` 在 `UpdateTexture` 和 FBO 创建时需要查询。

---

### 5.5 CreateVertexShader / CreateFragmentShader — SPIR-V 着色器加载

```cpp
ShaderHandle OpenGLBackend::CreateVertexShader(const uint32_t* spirv, size_t size) {
    if (spirv == nullptr || size == 0) {  // 验证输入
        fprintf(stderr, "[OpenGL] Empty SPIR-V data for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);  // 创建顶点着色器对象
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    // Load SPIR-V binary
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv, static_cast<GLsizei>(size * sizeof(uint32_t)));
    // 加载 SPIR-V 二进制数据到着色器对象

    // Specialize with default entry point "main"
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);
    // 特化入口点 "main"，无特化常量

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader SPIR-V specialization failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;  // 分配新句柄 ID
    m_shaders[id] = shader;           // 存入着色器池
    return ShaderHandle{id};
}

// CreateFragmentShader 实现完全相同，仅 GL_VERTEX_SHADER 替换为 GL_FRAGMENT_SHADER
// 此处省略重复代码，参见 5.5 节源文件
```

#### 功能说明

从 SPIR-V 二进制数据创建 OpenGL 着色器。使用 `glShaderBinary` 加载 SPIR-V 二进制，然后通过 `glSpecializeShader` 特化入口点。片段着色器实现完全相同。

#### 实现原理

OpenGL 4.6 支持 `GL_ARB_gl_spirv` 扩展（核心特性），允许直接加载 SPIR-V 二进制。流程为：`glCreateShader` -> `glShaderBinary`（上传 SPIR-V） -> `glSpecializeShader`（特化入口点） -> 检查状态。

#### 为什么这样实现

- **SPIR-V vs GLSL**：SPIR-V 是预编译的中间格式，跳过运行时 GLSL 编译，加载更快且无驱动编译器差异。
- **glSpecializeShader**：SPIR-V 支持特化常量（Specialization Constants），API 要求必须调用此函数。此处不使用特化常量（数量为 0）。
- **错误日志**：特化失败时输出详细日志，便于调试。

---

### 5.6 CreatePipeline — 管线创建（懒创建 GL Program）

```cpp
PipelineHandle OpenGLBackend::CreatePipeline(const PipelineDesc& desc) {
    // OpenGL doesn't have pipeline objects like Vulkan
    // OpenGL 没有管线对象，只返回编码了着色器 ID 的句柄
    uint32_t id = (desc.vertShader.id << 16) | desc.fragShader.id;
    return PipelineHandle{id};
}

void OpenGLBackend::DestroyPipeline(PipelineHandle handle) {
    // Nothing to destroy in OpenGL
    (void)handle;
}

void OpenGLBackend::BindPipeline(PipelineHandle handle) {
    uint32_t vsId = handle.id >> 16;        // 高 16 位 = 顶点着色器 ID
    uint32_t fsId = handle.id & 0xFFFF;     // 低 16 位 = 片段着色器 ID

    GLuint vs = GetGLShader({vsId});
    GLuint fs = GetGLShader({fsId});

    if (vs != 0 && fs != 0) {
        GLuint program = GetOrCreateProgram(vsId, fsId, vs, fs);
        if (program != 0) {
            glUseProgram(program);
        }
    }
}
```

#### 功能说明

OpenGL 后端的管线管理是接口兼容性实现。管线句柄仅编码着色器 ID，实际 GL 程序在首次使用时懒创建。

#### 实现原理

`CreatePipeline` 将两个着色器 ID 编码为 32 位值（各 16 位）。`BindPipeline` 解码后调用 `GetOrCreateProgram`。`DestroyPipeline` 为空操作。

#### 为什么这样实现

- **接口兼容**：`IRenderBackend` 定义管线接口以适配 Vulkan PSO。OpenGL 提供空壳，上层代码无需区分后端。
- **懒创建**：GL 程序创建推迟到实际绘制时，避免创建未使用的程序。

---

### 5.7 DrawFullscreenQuad — 核心渲染流程

```cpp
void OpenGLBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    GLuint vs = GetGLShader(vert);  // 获取 GL 顶点着色器对象
    GLuint fs = GetGLShader(frag);  // 获取 GL 片段着色器对象

    if (vs == 0 || fs == 0) {
        fprintf(stderr, "[OpenGL] Invalid shader handles in DrawFullscreenQuad\n");
        return;
    }

    // Use auto-increment IDs for cache key, GL shader objects for glAttachShader
    GLuint program = GetOrCreateProgram(vert.id, frag.id, vs, fs);
    if (program == 0) {
        fprintf(stderr, "[OpenGL] Failed to create shader program\n");
        return;
    }

    glUseProgram(program);  // 使用着色器程序

    // Reset ALL GL state that ImGui may have changed
    // 重置 ImGui 可能修改的所有 GL 状态
    glDisable(GL_DEPTH_TEST);    // 禁用深度测试
    glDisable(GL_SCISSOR_TEST);  // 禁用裁剪测试
    glDisable(GL_STENCIL_TEST); // 禁用模板测试
    glDisable(GL_CULL_FACE);     // 禁用面剔除
    glEnable(GL_BLEND);          // 启用混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Alpha 混合
    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);  // 绑定当前 FBO
    glViewport(0, 0, m_width, m_height);  // 全屏视口

    // Bind input textures (up to 8)
    for (size_t i = 0; i < params.inputTextures.size() && i < 8; ++i) {
        GLuint tex = GetGLTexture(params.inputTextures[i]);
        if (tex != 0) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
            glBindTexture(GL_TEXTURE_2D, tex);

            // Try sampler uniform: uTexture0, uTexture1, ...
            char uniformName[32];
            snprintf(uniformName, sizeof(uniformName), "uTexture%d", static_cast<int>(i));
            GLint loc = glGetUniformLocation(program, uniformName);
            if (loc >= 0) glUniform1i(loc, static_cast<GLint>(i));

            // Also try "uInputTex" for single-texture shaders
            if (i == 0) {
                loc = glGetUniformLocation(program, "uInputTex");
                if (loc >= 0) glUniform1i(loc, 0);
            }
        }
    }

    // Fill uniform data structure
    UniformData data = {};
    if (params.uniformFloats.size() > 0) data.uParamFloat0 = params.uniformFloats[0];
    if (params.uniformFloats.size() > 1) data.uParamFloat1 = params.uniformFloats[1];
    if (params.uniformFloats.size() > 2) data.uParamFloat2 = params.uniformFloats[2];
    if (params.uniformFloats.size() > 3) data.uParamFloat3 = params.uniformFloats[3];
    if (params.uniformFloats.size() > 4) data.uParamFloat4 = params.uniformFloats[4];
    if (params.uniformFloats.size() > 5) data.uParamFloat5 = params.uniformFloats[5];
    data.uResolution[0] = static_cast<float>(params.viewportWidth);
    data.uResolution[1] = static_cast<float>(params.viewportHeight);
    data.uTime = params.time;
    data.uFrameCount = static_cast<float>(params.frameCount);

    // ---- Check if program has UBO ----
    GLint nb=0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);

    if (nb > 0) {
        // ---- UBO path (SPIR-V or NVIDIA-baked) ----
        GLuint blockIndex = glGetUniformBlockIndex(program, "Params");
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, blockIndex, 1);
        }

        const size_t UBO_SIZE = 48;
        uint8_t ubo[UBO_SIZE] = {};
        for (size_t i = 0; i < params.uniformFloats.size() && i < 6; ++i) {
            float v = params.uniformFloats[i];
            memcpy(ubo + i * 4, &v, sizeof(float));
        }
        { float r[2]={(float)params.viewportWidth,(float)params.viewportHeight}; memcpy(ubo+24,r,8); }
        { memcpy(ubo+32,&params.time,4); float fc=(float)params.frameCount; memcpy(ubo+36,&fc,4); }
        glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
        glBufferData(GL_UNIFORM_BUFFER, UBO_SIZE, ubo, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_tempUBO);
    } else {
        // ---- Individual uniform path (GLSL) ----
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0); // unbind any UBO
        GLint loc;
        auto getLoc = [&](const char* name) -> GLint {
            GLint l = glGetUniformLocation(program, name);
            if (l >= 0) return l;
            char buf[64];
            snprintf(buf, sizeof(buf), "Params.%s", name);
            return glGetUniformLocation(program, buf);
        };
        // 设置每个 uniform（省略重复的 if/loc/getLoc 模式，完整代码见源文件）
        // ... uParamFloat0~5, uResolution, uTime, uFrameCount
    }

    // Draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, kFullscreenQuadVertexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // Cleanup
    for (size_t i = 0; i < 8; ++i) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

#### 功能说明

`DrawFullscreenQuad` 是整个渲染管线的核心函数，负责：(1) 获取或创建着色器程序；(2) 重置 GL 状态；(3) 绑定输入纹理；(4) 通过 UBO 或单独 uniform 传递参数；(5) 绘制全屏四边形；(6) 清理状态。

#### 实现原理

**双路径 uniform 传递**：查询程序是否有活跃 UBO 块。有则使用 UBO 路径（高效，一次 `glBufferData` 传递所有参数）；无则使用单独 `glUniform*` 调用。

**UBO 数据布局**：48 字节通过 `memcpy` 精确控制偏移，与 `UniformData` 结构体和 SPIR-V 着色器的 std140 布局完全匹配。

**状态重置**：绘制前显式重置所有可能被 ImGui 修改的 GL 状态。

#### 为什么这样实现

- **双路径兼容**：SPIR-V 着色器使用 UBO，GLSL 着色器使用单独 uniform。双路径确保两种着色器都能正确接收参数。
- **状态重置必要性**：ImGui 的渲染会修改 GL 状态（启用裁剪、修改混合函数等）。不清除则后处理渲染可能受影响。
- **Params. 前缀查找**：GLSL 着色器中 uniform 可能定义在命名块内，完整名称为 `Params.uParamFloat0`。lambda 先尝试裸名称再尝试带前缀名称，兼容两种情况。

---

### 5.8 BeginRenderToTexture / EndRenderToTexture — FBO 渲染

```cpp
void OpenGLBackend::BeginRenderToTexture(TextureHandle target) {
    GLuint fbo = GetGLFramebuffer(target);  // 获取或创建 FBO
    if (fbo == 0) {
        fprintf(stderr, "[OpenGL] Failed to get/create framebuffer for texture\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);  // 绑定 FBO 为渲染目标
    m_currentFBO = fbo;  // 更新当前 FBO 记录

    // Get texture dimensions for viewport
    auto wIt = m_texWidths.find(target.id);
    auto hIt = m_texHeights.find(target.id);
    if (wIt != m_texWidths.end() && hIt != m_texHeights.end()) {
        glViewport(0, 0, wIt->second, hIt->second);  // 视口匹配纹理尺寸
    }
}

void OpenGLBackend::EndRenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);  // 恢复默认 FBO
    m_currentFBO = m_defaultFBO;
    glViewport(0, 0, m_width, m_height);  // 恢复全屏视口
}
```

#### 功能说明

`BeginRenderToTexture` 将渲染目标切换到指定纹理的 FBO，`EndRenderToTexture` 恢复默认帧缓冲区。实现了离屏渲染的 begin/end 模式。

#### 实现原理

`GetGLFramebuffer` 是懒创建函数：如果纹理对应的 FBO 尚不存在，自动创建并附着纹理为颜色附件。FBO 以纹理 ID 为键缓存在 `m_framebuffers` 中。

#### 为什么这样实现

- **Begin/End 配对**：语义清晰，调用者只需在 begin/end 之间执行渲染命令。
- **懒创建 FBO**：仅首次需要时创建，减少不必要的 GL 对象。
- **视口自动匹配**：渲染到纹理时自动设置视口，避免分辨率不匹配。

---

### 5.9 DrawCards — 3D 卡片渲染

```cpp
void OpenGLBackend::DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) {
    if (cards.empty() || !viewMat || !projMat) return;

    // Build a simple card shader if needed (hardcoded GLSL)
    static GLuint cardProgram = 0;  // 静态变量：只创建一次
    if (cardProgram == 0) {
        const char* cardVS = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec2 aTexCoord;
            uniform mat4 uModel, uView, uProj;
            out vec2 vTexCoord;
            void main() {
                gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
                vTexCoord = aTexCoord;
            }
        )";
        const char* cardFS = R"(
            #version 460 core
            in vec2 vTexCoord;
            uniform sampler2D uCardTexture;
            uniform float uOpacity;
            out vec4 FragColor;
            void main() {
                vec4 texColor = texture(uCardTexture, vTexCoord);
                FragColor = vec4(texColor.rgb, texColor.a * uOpacity);
            }
        )";
        // 编译、链接 cardProgram（完整代码见源文件，此处省略重复的编译错误检查）
        // ...
    }

    glUseProgram(cardProgram);

    // Set view and projection matrices
    GLint viewLoc = glGetUniformLocation(cardProgram, "uView");
    GLint projLoc = glGetUniformLocation(cardProgram, "uProj");
    if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);
    if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMat);

    // Card quad vertices (position + UV)
    static const float cardVerts[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
    };
    static const unsigned int cardIdx[] = { 0, 1, 2, 0, 2, 3 };

    // Create temporary VAO
    GLuint cardVAO, cardVBO, cardEBO;
    glGenVertexArrays(1, &cardVAO);
    glGenBuffers(1, &cardVBO);
    glGenBuffers(1, &cardEBO);

    glBindVertexArray(cardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cardVerts), cardVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cardEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cardIdx), cardIdx, GL_STATIC_DRAW);

    // Position (location 0): vec3
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // UV (location 1): vec2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    GLint texLoc = glGetUniformLocation(cardProgram, "uCardTexture");
    if (texLoc >= 0) glUniform1i(texLoc, 0);

    GLint modelLoc = glGetUniformLocation(cardProgram, "uModel");
    GLint opacityLoc = glGetUniformLocation(cardProgram, "uOpacity");

    for (const auto& card : cards) {
        GLuint tex = GetGLTexture(card.texture);
        if (tex == 0) continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // Build model matrix: translate * rotateY * scale
        float model[16] = {};
        model[0] = model[5] = model[10] = model[15] = 1.0f;  // identity
        model[0] *= card.scaleX;  // scale X
        model[5] *= card.scaleY;  // scale Y

        // Rotation around Y
        float cosR = std::cos(card.rotationY);
        float sinR = std::sin(card.rotationY);
        float rotMat[16] = {};
        rotMat[0] = cosR; rotMat[2] = sinR; rotMat[5] = 1.0f;
        rotMat[8] = -sinR; rotMat[10] = cosR; rotMat[15] = 1.0f;

        // model = rotMat * model
        float result[16] = {};
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                for (int k = 0; k < 4; ++k)
                    result[r*4+c] += rotMat[r*4+k] * model[k*4+c];

        // Translation
        result[12] += card.posX;
        result[13] += card.posY;
        result[14] += card.posZ;

        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, result);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, card.opacity);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    // Cleanup temporary geometry
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &cardVAO);
    glDeleteBuffers(1, &cardVBO);
    glDeleteBuffers(1, &cardEBO);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

#### 功能说明

`DrawCards` 渲染封面流中的 3D 卡片。每张卡片是带纹理的四边形，支持位置、缩放、Y 轴旋转和不透明度变换。使用标准 MVP 矩阵管线。

#### 实现原理

**着色器**：使用硬编码 GLSL 460（非 SPIR-V），因为卡片着色器是后端内部实现细节。`static GLuint cardProgram` 确保只编译链接一次。

**模型矩阵**：手动构建 4x4 列主序矩阵，变换顺序为 Scale -> RotateY -> Translate。矩阵乘法使用三重循环手动实现。

#### 为什么这样实现

- **硬编码 GLSL**：卡片着色器是 OpenGL 后端内部实现，不需要跨后端共享。使用 GLSL 简化代码。
- **手动矩阵运算**：避免引入 GLM 等数学库依赖，保持项目轻量。
- **临时 VAO**：卡片数量有限，每次创建/删除的开销可接受。

---

### 5.10 ImGui 集成

```cpp
void OpenGLBackend::ImGuiInit(GLFWwindow* window) {
    IMGUI_CHECKVERSION();  // 检查版本兼容性
    ImGui::CreateContext();  // 创建上下文

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 键盘导航
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // 窗口停靠

    // Load fonts: default Latin + Chinese merge
    const char* chineseFontPath = nullptr;
    // Check for Microsoft YaHei
    if (GetFileAttributesA("C:\\Windows\\Fonts\\msyh.ttc") != INVALID_FILE_ATTRIBUTES) {
        chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }

    if (chineseFontPath) {
        io.Fonts->AddFontDefault();  // 先加载默认拉丁字体
        ImFontConfig cfg;
        cfg.MergeMode = true;  // 合并模式
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());  // 合并中文字形
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);  // GLFW 后端
    ImGui_ImplOpenGL3_Init("#version 460 core");  // OpenGL3 后端
}

void OpenGLBackend::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OpenGLBackend::ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLBackend::ImGuiShutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
```

#### 功能说明

ImGui 集成的四个函数：初始化上下文/字体/后端、每帧开始、渲染绘制数据、关闭清理。

#### 实现原理

ImGui 使用双后端架构：`imgui_impl_glfw` 处理平台操作（窗口、输入），`imgui_impl_opengl3` 处理 GL 渲染。

字体加载采用"合并模式"：先加载默认拉丁字体，再以合并模式加载中文字体。合并模式下 ImGui 为每个字符查找第一个包含该字形的字体。

#### 为什么这样实现

- **微软雅黑优先**：Windows 上 CJK 覆盖最完整的系统字体。
- **合并模式**：保留默认字体的拉丁字符质量，同时添加中文支持。
- **`GetFileAttributesA` 检测**：比 `std::filesystem::exists` 更轻量，所有 Windows 版本可用。

---

### 5.11 Shutdown — 资源清理

```cpp
void OpenGLBackend::Shutdown() {
    // Cleanup shaders
    for (auto& [id, shader] : m_shaders) {
        if (shader != 0) glDeleteShader(shader);
    }
    m_shaders.clear();

    // Cleanup programs
    for (auto& [key, program] : m_programCache) {
        if (program != 0) glDeleteProgram(program);
    }
    m_programCache.clear();

    // Cleanup framebuffers
    for (auto& [id, fbo] : m_framebuffers) {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
    }
    m_framebuffers.clear();

    // Cleanup textures
    for (auto& [id, tex] : m_textures) {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    m_textures.clear();
    m_textureFormats.clear();
    m_texWidths.clear();
    m_texHeights.clear();

    // Cleanup quad
    if (m_quadVAO != 0) { glDeleteVertexArrays(1, &m_quadVAO); m_quadVAO = 0; }
    if (m_quadVBO != 0) { glDeleteBuffers(1, &m_quadVBO); m_quadVBO = 0; }

    // Cleanup UBO
    if (m_tempUBO != 0) { glDeleteBuffers(1, &m_tempUBO); m_tempUBO = 0; }

    // Unbind FBO and flush — prevents conflicts when switching to Vulkan
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glFinish();  // 等待所有 GL 命令完成

    printf("[OpenGL] Shutdown\n");
}
```

#### 功能说明

释放 OpenGL 后端持有的所有 GL 资源：着色器、程序、FBO、纹理、VAO/VBO、UBO。最后恢复默认 FBO 并刷新命令队列。

#### 实现原理

资源清理顺序为：着色器 -> 程序 -> FBO -> 纹理 -> 几何 -> UBO。先清理依赖其他资源的对象。

`glFinish()` 确保所有 GL 命令在返回前完成，在切换到 Vulkan 后端时尤为重要。

#### 为什么这样实现

- **全面清理**：遍历所有资源池逐一删除，确保无泄漏。
- **glFinish 重要性**：GL 命令异步执行，`glFinish` 强制同步。不调用可能导致切换到 Vulkan 时竞争条件。
- **恢复默认 FBO**：确保后端关闭后 GL 状态干净。

---

### 5.12 辅助函数

#### GetOrCreateProgram — 程序缓存与懒创建

```cpp
GLuint OpenGLBackend::GetOrCreateProgram(GLuint vsKey, GLuint fsKey, GLuint vsGL, GLuint fsGL) {
    uint64_t key = (static_cast<uint64_t>(vsKey) << 32) | fsKey;
    // 64 位缓存键

    auto it = m_programCache.find(key);
    if (it != m_programCache.end()) return it->second;  // 缓存命中

    GLuint program = glCreateProgram();
    glAttachShader(program, vsGL);
    glAttachShader(program, fsGL);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Shader program linking failed: %s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDetachShader(program, vsGL);  // 链接后分离
    glDetachShader(program, fsGL);

    // Explicitly bind "Params" UBO to binding point 1
    GLint nb = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);
    if (nb > 0) {
        GLuint blockIndex = glGetUniformBlockIndex(program, "Params");
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, blockIndex, 1);
        }
    }

    m_programCache[key] = program;
    return program;
}
```

#### 功能说明

核心的程序缓存函数。以顶点+片段着色器 ID 为键，查找或创建链接后的 GL 程序。

#### 实现原理

64 位复合键确保唯一性。链接成功后立即分离着色器（GL 最佳实践）。显式绑定 UBO 到绑定点 1 解决跨驱动兼容性问题。

#### 为什么这样实现

- **性能关键**：封面流渲染 18 张缩略图时，缓存使链接只执行一次。
- **显式绑定点**：某些驱动不遵守 `layout(binding=1)`，必须显式设置。

#### SetupQuadVAO — 全屏四边形几何设置

```cpp
void OpenGLBackend::SetupQuadVAO() {
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);

    // Position (location 0): 2 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // UV (location 1): 2 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // EBO for indexed drawing
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices), kQuadIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    m_quadVBO = vbo;
}
```

#### 功能说明

创建全屏四边形的 VAO、VBO 和 EBO。顶点数据使用交错布局（位置 + UV 交替），通过索引绘制减少顶点数（4 个顶点而非 6 个）。

#### 实现原理

VAO 封装了顶点属性配置，绑定 VAO 后只需调用 `glDrawElements` 即可绘制。EBO（Element Buffer Object）通过索引复用顶点，将 4 个顶点的四边形用 2 个三角形（6 个索引）绘制。

#### 为什么这样实现

- **交错布局**：位置和 UV 在同一缓冲区中交替排列，提高缓存命中率。
- **索引绘制**：4 个顶点 + 6 个索引，比 6 个独立顶点更节省内存和带宽。
- **EBO 归属 VAO**：EBO 在 VAO 绑定时创建，由 VAO 管理生命周期。VBO 需要单独保存句柄以便清理。

#### BindDefaultState — 默认 GL 状态

```cpp
void OpenGLBackend::BindDefaultState() {
    glEnable(GL_DEPTH_TEST);    // 启用深度测试
    glDepthFunc(GL_LESS);       // 深度函数：小于时通过
    glEnable(GL_BLEND);         // 启用混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Alpha 混合
    glDisable(GL_CULL_FACE);    // 禁用面剔除
    glViewport(0, 0, m_width, m_height);  // 全屏视口
}
```

#### 功能说明

设置 OpenGL 的默认渲染状态，在 `Init` 中调用一次，作为后续渲染操作的基准状态。

#### 为什么这样实现

- **深度测试启用**：3D 卡片渲染需要深度测试，默认启用避免遗漏。
- **Alpha 混合**：封面流卡片有透明度变化，混合是必需的。
- **禁用面剔除**：全屏四边形和卡片可能从背面观察（旋转时），禁用剔除确保始终可见。

---

> **文档说明**：本文档覆盖了 Shader Showcase 项目的第 4 章（EffectDetailScene 特效详情页）和第 5 章（OpenGL 渲染后端），对每个函数提供了完整源码、逐行中文注释、功能说明、实现原理和设计理由分析。

# Shader Showcase 项目代码详解（第三部分）

> 第6-8章：Vulkan 渲染后端、着色器加载与效果元数据、输入源与UI组件

---

## 6. Vulkan 渲染后端

本章深入分析 VulkanBackend 的完整实现，涵盖从 Vulkan 实例创建到全屏四边形渲染的整个生命周期。VulkanBackend 是整个项目的核心渲染引擎，实现了 `IRenderBackend` 接口，提供 SPIR-V 着色器支持、纹理管理和全屏后处理管线。

### 6.1 VulkanBackend 头文件

头文件定义了三个核心资源包装结构体和 VulkanBackend 类本身。

```cpp
#pragma once
#include "render/IRenderBackend.h"    // 引入渲染后端抽象接口
#include <vulkan/vulkan.h>            // Vulkan 核心头文件
#include <vector>                     // 动态数组容器
#include <unordered_map>              // 哈希映射容器
#include <memory>                     // 智能指针

struct GLFWwindow;                   // 前向声明 GLFW 窗口类型

// ============================================================================
// Vulkan Resource Wrappers — Vulkan 资源包装结构体
// ============================================================================

// 着色器模块包装：持有 VkShaderModule 和着色器阶段标识
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;           // Vulkan 着色器模块句柄
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT; // 着色器阶段（顶点/片段）
};

// 纹理包装：持有完整的 Vulkan 纹理资源链（Image + View + Sampler + 可选FBO）
struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;           // Vulkan 图像对象
    VkDeviceMemory memory = VK_NULL_HANDLE;   // 图像绑定的设备内存
    VkImageView view = VK_NULL_HANDLE;         // 图像视图（描述纹理的访问方式）
    VkSampler sampler = VK_NULL_HANDLE;       // 采样器（描述纹理的过滤和寻址模式）
    int width = 0, height = 0;               // 纹理宽高
    VkFormat format = VK_FORMAT_UNDEFINED;    // 像素格式
    bool isFBO = false;                       // 是否为帧缓冲对象（渲染目标）
    VkFramebuffer framebuffer = VK_NULL_HANDLE;   // 帧缓冲（仅渲染目标使用）
    VkRenderPass renderPass = VK_NULL_HANDLE;     // 渲染通道（仅渲染目标使用）
    // ImGui 描述符集缓存（用于 GetImTextureID，避免重复创建）
    VkDescriptorSet imguiDescriptorSet = VK_NULL_HANDLE;
};

// 管线包装：持有完整的 Vulkan 图形管线及其描述符资源
struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;             // 图形管线对象
    VkPipelineLayout layout = VK_NULL_HANDLE;        // 管线布局（描述符集布局绑定）
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE; // 描述符集布局
    VkDescriptorPool descPool = VK_NULL_HANDLE;      // 描述符池
    VkDescriptorSet descSet = VK_NULL_HANDLE;         // 预分配的描述符集（绑定纹理+UBO）
    ShaderHandle vertShader;                           // 顶点着色器句柄
    ShaderHandle fragShader;                           // 片段着色器句柄
    // UBO 缓冲区：用于着色器 Params 块（binding=1），每帧复用
    VkBuffer uboBuffer = VK_NULL_HANDLE;               // UBO 缓冲区对象
    VkDeviceMemory uboMemory = VK_NULL_HANDLE;        // UBO 缓冲区内存
};

// ============================================================================
// Vulkan Backend — Vulkan 渲染后端主类
// ============================================================================
class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();                    // 构造函数
    ~VulkanBackend() override;          // 析构函数（自动调用 Shutdown）

    // Lifecycle — 生命周期管理
    bool Init(GLFWwindow* window) override;   // 初始化整个 Vulkan 后端
    void Shutdown() override;                 // 关闭并释放所有资源
    void BeginFrame() override;               // 开始新帧（获取交换链图像、开始命令录制）
    void EndFrame() override;                 // 结束帧（提交命令、呈现）
    void WaitIdle() override;                 // 等待 GPU 空闲

    // Viewport — 视口管理
    void Resize(int width, int height) override;           // 标记窗口大小变更
    void GetFramebufferSize(int& width, int& height) override; // 获取当前帧缓冲尺寸

    // Shaders (SPIR-V) — 着色器管理
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // Textures — 纹理管理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;

    // Pipelines — 管线管理
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void DestroyPipeline(PipelineHandle handle) override;
    void BindPipeline(PipelineHandle handle) override;

    // Fullscreen draw — 全屏绘制
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<IRenderBackend::CardDrawInfo>& cards, const float* viewMatrix, const float* projMatrix) override;
    void BlitToScreen(TextureHandle src) override;
    void BeginRenderToTexture(TextureHandle target) override;  // 开始渲染到纹理（FBO）
    void EndRenderToTexture() override;                        // 结束渲染到纹理

    // Utility — 工具函数
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;

    // ImGui — ImGui 集成
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // Query — 查询接口
    BackendType GetType() const override { return BackendType::Vulkan; }
    const char* GetName() const override { return "Vulkan 1.2 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    // Window reference — 窗口引用
    GLFWwindow* m_window = nullptr;     // GLFW 窗口指针
    int m_width = 1280;                 // 当前宽度
    int m_height = 720;                 // 当前高度
    bool m_initialized = false;          // 是否已初始化
    bool m_framebufferResized = false;  // 帧缓冲是否需要重建

    // Core Vulkan Objects — Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsFamily = 0;
    uint32_t m_presentFamily = 0;

    // Surface and Swapchain — 表面和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D m_swapchainExtent = {};
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    // Command Buffers — 命令缓冲
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    // Synchronization Objects — 同步对象
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    // Frame State — 帧状态
    uint32_t m_currentImageIndex = 0;
    bool m_isRecording = false;

    // Resource Management — 资源管理
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;
    std::unordered_map<uint32_t, VulkanShader> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;

    // Current State — 当前状态
    PipelineHandle m_currentPipeline = {0};
    VkFramebuffer m_currentFramebuffer = VK_NULL_HANDLE;
    VkRenderPass m_currentRenderPass = VK_NULL_HANDLE;
    bool m_isRenderToTexture = false;

    // ImGui Vulkan Resources — ImGui Vulkan 资源
    VkDescriptorPool m_imguiDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_imguiDescSetLayout = VK_NULL_HANDLE;
    bool m_imguiInitialized = false;
    bool m_imguiRenderPending = false;

    // 延迟销毁队列（帧提交后销毁的资源）
    struct DeferredDestroy {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
    };
    std::vector<DeferredDestroy> m_deferredDestroys;

    // 管线缓存：复用相同着色器+渲染通道组合的管线
    std::unordered_map<uint64_t, PipelineHandle> m_pipelineCache;

    // Queue family indices — 队列族索引结构体
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        bool isComplete() const { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX; }
    };
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    // Swapchain support details — 交换链支持详情结构体
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    // 初始化辅助函数
    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    // 交换链管理
    void CleanupSwapchain();
    void RecreateSwapchain();

    // 工具函数
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkSampler CreateSampler();

    // 管线辅助函数
    VkDescriptorSetLayout CreateDescriptorSetLayout();
    VkDescriptorPool CreateDescriptorPool(uint32_t maxSets);
    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
    VkRenderPass CreateRenderPassForFormat(VkFormat format);

    // 辅助查询
    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
```

#### 功能说明

VulkanBackend 头文件定义了三个资源包装结构体（`VulkanShader`、`VulkanTexture`、`VulkanPipeline`）和主类 `VulkanBackend`。主类继承自 `IRenderBackend` 接口，提供了完整的 Vulkan 渲染后端实现。头文件中还包含了队列族索引结构体 `QueueFamilyIndices` 和交换链支持详情结构体 `SwapChainSupportDetails`，用于设备选择和交换链创建过程中的信息查询。

#### 实现原理

头文件采用"资源包装 + 哈希映射"的设计模式管理 Vulkan 资源。每个资源类型（着色器、纹理、管线）都有一个对应的包装结构体，通过 `unordered_map<uint32_t, Wrapper>` 存储，以自增 ID 作为键。这种设计将 Vulkan 的底层句柄与业务逻辑的句柄系统解耦，上层代码通过 `ShaderHandle`、`TextureHandle`、`PipelineHandle` 等轻量结构体操作资源，无需直接接触 Vulkan API。

#### 为什么这样实现

1. **资源包装结构体**：将 Vulkan 资源的多个关联句柄（如纹理的 Image + View + Sampler + Memory）封装在一起，确保资源创建和销毁时不会遗漏任何一个句柄。
2. **哈希映射存储**：提供 O(1) 的资源查找性能，同时支持通过 ID 进行跨模块的资源引用。
3. **延迟销毁队列**：Vulkan 命令是异步执行的，GPU 可能仍在使用已提交命令中引用的资源。延迟销毁确保在 GPU 完成使用后才释放资源。
4. **管线缓存**：避免为相同的着色器组合重复创建管线，减少 GPU 管线创建开销。

---

### 6.2 Init — Instance/Surface/Device/Swapchain 创建

`Init` 函数是 VulkanBackend 的入口点，按顺序调用所有初始化步骤。

```cpp
bool VulkanBackend::Init(GLFWwindow* window) {
    m_window = window;                              // 保存 GLFW 窗口指针
    glfwGetFramebufferSize(window, &m_width, &m_height); // 获取初始帧缓冲尺寸

    try {
        CreateInstance();        // 步骤1：创建 Vulkan 实例
        CreateSurface();         // 步骤2：创建窗口表面
        PickPhysicalDevice();    // 步骤3：选择物理设备（GPU）
        CreateLogicalDevice();  // 步骤4：创建逻辑设备与队列
        CreateSwapchain();       // 步骤5：创建交换链
        CreateRenderPass();      // 步骤6：创建渲染通道
        CreateFramebuffers();    // 步骤7：创建帧缓冲
        CreateCommandPool();     // 步骤8：创建命令池
        CreateCommandBuffers();  // 步骤9：分配命令缓冲
        CreateSyncObjects();     // 步骤10：创建同步对象（信号量+栅栏）
    } catch (const std::exception& e) {
        fprintf(stderr, "[Vulkan] Init failed: %s\n", e.what()); // 捕获初始化异常
        return false;            // 初始化失败返回 false
    }

    m_initialized = true;       // 标记初始化完成
    printf("[Vulkan] Initialized successfully (%dx%d)\n", m_width, m_height);
    return true;                 // 初始化成功返回 true
}
```

#### 功能说明

`Init` 函数是 Vulkan 后端的初始化入口，严格按照 Vulkan 规范要求的顺序执行十个初始化步骤。每个步骤都有独立的异常处理，任何步骤失败都会被 catch 捕获并输出错误信息。

#### 实现原理

Vulkan 的初始化有严格的依赖顺序：Instance 必须先于 Surface，Surface 必须先于 PhysicalDevice 选择（因为需要 Surface 来查询队列族支持），PhysicalDevice 必须先于 LogicalDevice，以此类推。使用 try-catch 包裹整个初始化序列，确保任何步骤失败时不会导致后续步骤在无效状态下执行。

#### 为什么这样实现

1. **顺序初始化**：Vulkan 对象之间存在严格的创建依赖关系，必须按序执行。
2. **异常捕获**：Vulkan 的很多创建函数通过返回值报告错误，代码中使用 `VK_CHECK` 宏在检测到错误时打印日志但不抛出异常。然而，关键步骤（如设备选择）在失败时抛出 `std::runtime_error`，确保初始化流程能被中断。
3. **单一入口点**：将所有初始化逻辑集中在一个函数中，调用者只需调用 `Init` 即可完成全部设置，降低了使用复杂度。

---

### 6.3 CreateInstance — Vulkan实例创建

```cpp
void VulkanBackend::CreateInstance() {
    // 检查验证层是否可用（仅在调试模式下启用）
    if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport()) {
        fprintf(stderr, "[Vulkan] Validation layers requested but not available\n");
        throw std::runtime_error("Validation layers requested but not available");
    }

    // 设置应用程序信息
    VkApplicationInfo appInfo{};                              // 零初始化应用信息结构体
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;        // 结构体类型标识
    appInfo.pApplicationName = "Shader Showcase";            // 应用名称
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);     // 应用版本 1.0.0
    appInfo.pEngineName = "Shader Showcase Engine";            // 引擎名称
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);        // 引擎版本 1.0.0
    appInfo.apiVersion = VK_API_VERSION_1_2;                   // 请求 Vulkan 1.2 API

    // 从 GLFW 获取所需的实例扩展列表（如 VK_KHR_surface、VK_KHR_win32_surface）
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // 如果启用验证层，添加调试工具扩展
    if (ENABLE_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // 调试回调扩展
    }

    // 配置实例创建信息
    VkInstanceCreateInfo createInfo{};                          // 零初始化创建信息
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;  // 结构体类型
    createInfo.pApplicationInfo = &appInfo;                    // 指向应用信息
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // 扩展数量
    createInfo.ppEnabledExtensionNames = extensions.data();     // 扩展名称数组

    // 根据调试模式启用或禁用验证层
    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // 创建 Vulkan 实例
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    printf("[Vulkan] Instance created\n");
}
```

#### 功能说明

创建 Vulkan 实例，这是使用 Vulkan API 的第一步。实例是 Vulkan 应用程序与 Vulkan 库之间的连接对象，所有后续的 Vulkan 对象都依赖于实例。

#### 实现原理

通过 `VkApplicationInfo` 告知驱动程序应用程序的基本信息（名称、版本、请求的 API 版本）。通过 `glfwGetRequiredInstanceExtensions` 获取 GLFW 窗口系统所需的扩展（如 Windows 平台的 `VK_KHR_win32_surface`），确保 Vulkan 能与窗口系统集成。在调试模式下额外启用 `VK_EXT_debug_utils` 扩展和 `VK_LAYER_KHRONOS_validation` 验证层，用于运行时错误检测。

#### 为什么这样实现

1. **VK_CHECK 宏**：封装了 Vulkan 结果检查，在出错时打印文件名和行号，便于调试。
2. **GLFW 集成**：使用 `glfwGetRequiredInstanceExtensions` 而非硬编码扩展列表，确保跨平台兼容性。
3. **条件验证层**：仅在 `_DEBUG` 模式下启用验证层，避免发布版本的性能开销。
4. **API 版本 1.2**：请求 Vulkan 1.2 以获得更现代的特性支持。

---

### 6.4 CreateSurface — Win32原生Surface

```cpp
void VulkanBackend::CreateSurface() {
    // 使用 Win32 原生 Surface 创建，而非 glfwCreateWindowSurface。
    // 原因：GLFW_OPENGL_API 窗口在部分 NVIDIA 驱动上会被
    // glfwCreateWindowSurface 拒绝，但 vkCreateWin32SurfaceKHR 可用于任何 HWND。
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};          // 零初始化
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);    // 当前进程模块句柄
    surfaceCreateInfo.hwnd = glfwGetWin32Window(m_window);      // 从 GLFW 窗口获取 HWND

    // 创建 Win32 Surface
    VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
    printf("[Vulkan] Surface created (Win32 native)\n");
}
```

#### 功能说明

创建 Vulkan 窗口表面（Surface），它是 Vulkan 渲染输出与操作系统窗口之间的桥梁。Surface 定义了渲染结果呈现的目标区域。

#### 实现原理

直接使用 Win32 API 的 `GetModuleHandle` 获取进程实例句柄，通过 GLFW 的 `glfwGetWin32Window` 从 GLFW 窗口获取原生 HWND，然后调用 `vkCreateWin32SurfaceKHR` 创建 Surface。这绕过了 GLFW 的 `glfwCreateWindowSurface`，因为后者在 OpenGL API 模式创建的窗口上可能失败。

#### 为什么这样实现

1. **绕过 GLFW 限制**：本项目同时支持 OpenGL 和 Vulkan 后端，窗口可能以 `GLFW_OPENGL_API` 模式创建。部分 NVIDIA 驱动会拒绝在此类窗口上调用 `glfwCreateWindowSurface`，但直接使用 Win32 HWND 创建 Surface 则没有此限制。
2. **跨后端兼容**：允许在同一个 GLFW 窗口上切换 OpenGL 和 Vulkan 渲染后端。
3. **Win32 原生调用**：通过 `#define GLFW_EXPOSE_NATIVE_WIN32` 和 `#include <GLFW/glfw3native.h>` 启用 GLFW 的原生窗口访问功能。

---

### 6.5 PickPhysicalDevice — GPU选择评分

```cpp
void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan-capable GPU found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    // 评分选择最佳 GPU
    int bestScore = -1;
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

    for (const auto& device : devices) {
        if (!IsDeviceSuitable(device)) continue;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;  // 独立显卡 +1000 分
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 100;   // 集成显卡 +100 分
        }

        // 根据显存大小加分（每 GB 加 1 分）
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);
        VkDeviceSize totalMemory = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalMemory += memProps.memoryHeaps[i].size;
            }
        }
        score += static_cast<int>(totalMemory / (1024 * 1024 * 1024));

        if (score > bestScore) {
            bestScore = score;
            bestDevice = device;
        }
    }

    if (bestDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("No suitable GPU found");
    }

    m_physicalDevice = bestDevice;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, props);
    printf("[Vulkan] Physical device: %s\n", props.deviceName);
}
```

#### 功能说明

枚举系统中所有支持 Vulkan 的物理设备（GPU），通过评分机制选择最佳的 GPU。评分标准包括设备类型（独立显卡 > 集成显卡）和显存大小。

#### 实现原理

首先枚举所有物理设备，然后对每个设备调用 `IsDeviceSuitable` 检查是否满足基本要求（支持图形队列、支持交换链扩展、交换链格式和呈现模式非空）。通过后的设备进入评分环节：独立显卡得 1000 分，集成显卡得 100 分，显存每 GB 加 1 分。最终选择得分最高的设备。

#### 为什么这样实现

1. **评分机制**：比简单的"第一个可用设备"更智能，优先选择性能更好的独立显卡。
2. **显存考量**：对于后处理应用，显存大小直接影响可处理的纹理分辨率和数量。
3. **IsDeviceSuitable 预检**：确保选中的设备具备所有必需的功能（队列族支持、交换链扩展、格式/呈现模式可用性），避免后续初始化失败。

---

### 6.6 CreateLogicalDevice — 逻辑设备与队列

```cpp
void VulkanBackend::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    m_graphicsFamily = indices.graphicsFamily;
    m_presentFamily = indices.presentFamily;

    // 创建队列创建信息列表（去重：图形和呈现可能是同一队列族）
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{}; // 不请求额外特性

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

    vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

    printf("[Vulkan] Logical device created (graphics queue: %u, present queue: %u)\n",
           indices.graphicsFamily, indices.presentFamily);
}
```

#### 功能说明

创建逻辑设备（与物理设备通信的软件接口）并获取图形队列和呈现队列的句柄。

#### 实现原理

首先通过 `FindQueueFamilies` 查找支持图形操作和表面呈现的队列族。使用 `std::set` 对队列族索引去重（因为图形和呈现可能在同一队列族），然后为每个唯一队列族创建队列请求。请求 `VK_KHR_swapchain` 设备扩展以支持交换链操作。

#### 为什么这样实现

1. **队列族去重**：使用 `std::set` 避免为同一队列族创建重复的队列请求，这是 Vulkan 规范推荐的实践。
2. **队列优先级 1.0**：由于只有一个队列，设为最高优先级确保命令及时执行。
3. **不请求额外特性**：后处理应用不需要特殊硬件特性，保持设备创建简单。

---

### 6.7 CreateSwapchain — 交换链管理

```cpp
void VulkanBackend::CreateSwapchain() {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));
    m_swapchainImages.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data()));

    m_swapchainFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    m_width = static_cast<int>(extent.width);
    m_height = static_cast<int>(extent.height);

    printf("[Vulkan] Swapchain created (%dx%d, %zu images)\n", extent.width, extent.height, m_swapchainImages.size());
}
```

#### 功能说明

创建 Vulkan 交换链，它是渲染结果的缓冲队列。交换链管理着一组图像，应用程序渲染到其中一个图像，然后将其呈现到屏幕。

#### 实现原理

首先查询物理设备的交换链支持详情，然后选择最佳配置：优先选择 `VK_FORMAT_B8G8R8A8_UNORM` + `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` 格式，优先选择 `VK_PRESENT_MODE_FIFO_KHR`（VSync）呈现模式。图像数量设为最小值 + 1 以实现双缓冲效果。

#### 为什么这样实现

1. **FIFO 呈现模式**：选择 VSync 模式避免画面撕裂，对于后处理预览应用更稳定。
2. **并发/独占模式自动选择**：根据队列族是否相同自动选择共享模式，确保在任何 GPU 上都能正确工作。
3. **minImageCount + 1**：比最小值多一个图像实现双缓冲，减少等待时间。

---

### 6.8 CreateRenderPass — 渲染通道

```cpp
void VulkanBackend::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;    // 渲染通道开始时清除
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // 渲染通道结束时存储
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 最终布局为呈现源

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
    printf("[Vulkan] Render pass created\n");
}
```

#### 功能说明

创建主渲染通道，定义了渲染操作如何使用交换链图像。渲染通道描述了附件的格式和使用方式，以及渲染子通道之间的依赖关系。

#### 实现原理

渲染通道包含一个颜色附件（交换链图像），配置为在开始时清除（`LOAD_OP_CLEAR`），结束时存储（`STORE_OP_STORE`）。初始布局为 `UNDEFINED`，最终布局为 `PRESENT_SRC_KHR`（准备好呈现）。子通道依赖确保在颜色附件写入之前，外部管线阶段已经完成。

#### 为什么这样实现

1. **LOAD_OP_CLEAR**：每帧开始时清除为固定颜色（深蓝色），避免上一帧残留。
2. **finalLayout = PRESENT_SRC_KHR**：告诉 Vulkan 渲染通道结束后图像将用于呈现，驱动可自动执行必要的布局转换。
3. **子通道依赖**：`VK_SUBPASS_EXTERNAL` 到子通道 0 的依赖确保了正确的执行顺序。

---

### 6.9 CreateTexture — Staging Buffer纹理上传

```cpp
TextureHandle VulkanBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    size_t pixelSize = GetTextureFormatSize(format);
    VkDeviceSize imageSize = width * height * pixelSize;

    auto texture = std::make_unique<VulkanTexture>();
    texture->width = width;
    texture->height = height;
    texture->format = vkFormat;
    texture->isFBO = (data == nullptr);  // 无初始数据 → 渲染目标

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture->isFBO) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    // 创建 Vulkan 图像（GPU 端，使用设备本地内存）
    CreateImage(
        static_cast<uint32_t>(width), static_cast<uint32_t>(height),
        vkFormat, VK_IMAGE_TILING_OPTIMAL, usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture->image, texture->memory
    );

    texture->view = CreateImageView(texture->image, vkFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    texture->sampler = CreateSampler();

    if (data != nullptr) {
        // 创建 Staging Buffer（CPU 可见内存，用于数据传输）
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory
        );

        // 将数据复制到 Staging Buffer
        void* mappedData;
        vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingBufferMemory);

        // 执行布局转换和缓冲复制
        TransitionImageLayout(texture->image, vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, texture->image,
            static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        TransitionImageLayout(texture->image, vkFormat,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // 清理 Staging Buffer
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    } else {
        TransitionImageLayout(texture->image, vkFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    uint32_t id = m_nextTextureId++;
    m_textures[id] = std::move(texture);

    printf("[Vulkan] Texture created (id=%u, %dx%d, format=%d)\n", id, width, height, static_cast<int>(format));
    return {id};
}
```

#### 功能说明

创建 Vulkan 纹理，支持两种模式：带初始数据的普通纹理（通过 Staging Buffer 上传）和无初始数据的渲染目标纹理（FBO）。

#### 实现原理

纹理创建遵循 Vulkan 的标准 Staging Buffer 模式：在 GPU 端创建使用设备本地内存的图像，创建 CPU 可见的 Staging Buffer，将像素数据复制到其中，通过命令缓冲执行 `CopyBufferToImage` 将数据从 Staging Buffer 复制到 GPU 图像，最后执行图像布局转换。

#### 为什么这样实现

1. **Staging Buffer 模式**：GPU 的设备本地内存通常不可被 CPU 直接访问，必须通过中间缓冲区传输数据。这是 Vulkan 中上传纹理数据的标准做法。
2. **HOST_COHERENT**：使用主机一致性内存属性，避免手动调用 `vkFlushMappedMemoryRanges`，简化代码。
3. **isFBO 判断**：通过 `data == nullptr` 判断是否为渲染目标，自动添加 `COLOR_ATTACHMENT_BIT` 标志。

---

### 6.10 CreatePipeline — 完整Vulkan管线创建

```cpp
PipelineHandle VulkanBackend::CreatePipeline(const PipelineDesc& desc) {
    auto vertIt = m_shaders.find(desc.vertShader.id);
    auto fragIt = m_shaders.find(desc.fragShader.id);
    if (vertIt == m_shaders.end() || fragIt == m_shaders.end()) {
        fprintf(stderr, "[Vulkan] Invalid shader handles for pipeline creation\n");
        return {0};
    }

    // 计算缓存键
    VkRenderPass renderPass = m_currentRenderPass != VK_NULL_HANDLE ? m_currentRenderPass : m_renderPass;
    uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
    cacheKey ^= (uint64_t)renderPass;

    auto cacheIt = m_pipelineCache.find(cacheKey);
    if (cacheIt != m_pipelineCache.end()) {
        return cacheIt->second; // 缓存命中
    }

    auto pipeline = std::make_unique<VulkanPipeline>();
    pipeline->vertShader = desc.vertShader;
    pipeline->fragShader = desc.fragShader;

    pipeline->descSetLayout = CreateDescriptorSetLayout();
    pipeline->layout = CreatePipelineLayout(pipeline->descSetLayout);
    pipeline->descPool = CreateDescriptorPool(1000);

    // 着色器阶段
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertIt->second.module;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragIt->second.module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // 顶点输入：无顶点数据（全屏三角形通过着色器生成）
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // 输入装配：三角形列表
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 视口和裁剪
    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = static_cast<float>(desc.width);
    viewport.height = static_cast<float>(desc.height);
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(desc.width), static_cast<uint32_t>(desc.height)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // 光栅化
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // 多重采样
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 颜色混合
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (desc.blendEnable) {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    } else {
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 动态状态
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // 创建图形管线
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipeline->layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline));

    uint32_t id = m_nextPipelineId++;
    m_pipelines[id] = std::move(pipeline);
    m_pipelineCache[cacheKey] = PipelineHandle{id};

    // 创建管线专属 UBO 和描述符集
    auto& pp = m_pipelines[id];
    const size_t UBO_SIZE = 48;
    CreateBuffer(UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 pp->uboBuffer, pp->uboMemory);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pp->descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &pp->descSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &pp->descSet));

    printf("[Vulkan] Pipeline created (id=%u)\n", id);
    return {id};
}
```

#### 功能说明

创建完整的 Vulkan 图形管线，包括管线布局、描述符集、UBO 缓冲区和所有固定管线状态。管线创建后会被缓存，相同着色器组合不会重复创建。

#### 实现原理

管线创建过程包含以下关键步骤：缓存检查、描述符集布局创建（binding=0 组合图像采样器 + binding=1 UBO）、固定管线状态配置（无顶点输入、三角形列表、填充模式、不剔除、动态视口）、图形管线创建、UBO 缓冲区和描述符集分配。

#### 为什么这样实现

1. **管线缓存**：Vulkan 管线创建是重量级操作（涉及 GPU 端的 PSO 编译），缓存可显著减少创建开销。
2. **无顶点输入**：后处理着色器不需要顶点数据，全屏三角形通过 `vkCmdDraw(cmd, 3, 1, 0, 0)` 直接绘制。
3. **每管线 UBO**：每个管线拥有独立的 48 字节 UBO，避免不同管线之间的 UBO 竞争。
4. **动态视口**：允许在不重建管线的情况下调整视口大小。

---

### 6.11 DrawFullscreenQuad — 无顶点缓冲全屏三角形

```cpp
void VulkanBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    if (!m_isRecording) return;

    PipelineDesc desc;
    desc.vertShader = vert;
    desc.fragShader = frag;
    desc.width = params.viewportWidth;
    desc.height = params.viewportHeight;
    desc.blendEnable = false;

    PipelineHandle pipeHandle = CreatePipeline(desc);
    if (pipeHandle.id == 0) return;

    BindPipeline(pipeHandle);

    // 设置动态视口
    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = static_cast<float>(params.viewportWidth);
    viewport.height = static_cast<float>(params.viewportHeight);
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(params.viewportWidth), static_cast<uint32_t>(params.viewportHeight)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    // 绑定描述符集并更新 UBO
    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end() && pipeIt->second->descSet != VK_NULL_HANDLE) {
        const size_t UBO_SIZE = 48;

        // 更新 UBO 数据
        {
            uint8_t uboData[UBO_SIZE] = {};
            for (size_t i = 0; i < std::min(params.uniformFloats.size(), size_t(6)); i++) {
                float v = params.uniformFloats[i];
                memcpy(uboData + i * 4, &v, sizeof(float));
            }
            { float r[2] = { static_cast<float>(params.viewportWidth), static_cast<float>(params.viewportHeight) }; memcpy(uboData + 24, r, 8); }
            { memcpy(uboData + 32, &params.time, 4); float fc = static_cast<float>(params.frameCount); memcpy(uboData + 36, &fc, 4); }

            void* mapped = nullptr;
            vkMapMemory(m_device, pipeIt->second->uboMemory, 0, UBO_SIZE, 0, &mapped);
            memcpy(mapped, uboData, UBO_SIZE);
            vkUnmapMemory(m_device, pipeIt->second->uboMemory);
        }

        // 更新描述符集
        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
        std::vector<VkWriteDescriptorSet> writes;

        if (!params.inputTextures.empty()) {
            auto texIt = m_textures.find(params.inputTextures[0].id);
            if (texIt != m_textures.end()) {
                imageInfo.sampler = texIt->second->sampler;
                imageInfo.imageView = texIt->second->view;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet texWrite{};
                texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                texWrite.dstSet = pipeIt->second->descSet;
                texWrite.dstBinding = 0;
                texWrite.dstArrayElement = 0;
                texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                texWrite.descriptorCount = 1;
                texWrite.pImageInfo = &imageInfo;
                writes.push_back(texWrite);
            }
        }

        bufferInfo.buffer = pipeIt->second->uboBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = UBO_SIZE;

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = pipeIt->second->descSet;
        uboWrite.dstBinding = 1;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        writes.push_back(uboWrite);

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeIt->second->layout, 0, 1, &pipeIt->second->descSet, 0, nullptr);
    }

    // 绘制全屏三角形（3 个顶点，无顶点缓冲）
    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    m_currentPipeline = {0};
}
```

#### 功能说明

绘制一个全屏三角形，这是后处理着色器的核心绘制函数。它创建（或从缓存获取）管线，更新 UBO 参数，绑定纹理，然后通过 `vkCmdDraw(cmd, 3, 1, 0, 0)` 绘制 3 个顶点组成的全屏三角形。

#### 实现原理

全屏三角形是一种经典的优化技术：不使用顶点缓冲区和 VAO，而是在顶点着色器中通过 `gl_VertexIndex` 生成覆盖整个屏幕的三角形。UBO 数据布局严格匹配着色器中的 `std140` 布局：偏移 0-23 为 6 个 float 参数，偏移 24-31 为 vec2 分辨率，偏移 32-35 为 float 时间，偏移 36-39 为 float 帧计数。

#### 为什么这样实现

1. **无顶点缓冲**：省去了 VAO/VBO 的创建和管理开销，对于后处理场景非常高效。
2. **每帧更新描述符集**：通过 `vkUpdateDescriptorSets` 每帧更新纹理绑定和 UBO 数据。
3. **管线缓存**：`CreatePipeline` 内部检查缓存，相同着色器组合只创建一次管线。
4. **不销毁管线**：Vulkan 命令异步执行，在 `vkCmdDraw` 后立即销毁管线会导致 GPU 访问无效资源。

---

### 6.12 BeginFrame / EndFrame — 帧同步与命令提交

#### BeginFrame

```cpp
void VulkanBackend::BeginFrame() {
    if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) {
        RecreateSwapchain();
        if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0)
            return;
    }

    // 等待上一帧完成（栅栏同步）
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);

    // 清理上一帧的延迟销毁资源
    for (auto& dd : m_deferredDestroys) {
        if (dd.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, dd.pipeline, nullptr);
        if (dd.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, dd.layout, nullptr);
        if (dd.descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, dd.descSetLayout, nullptr);
        if (dd.descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, dd.descPool, nullptr);
        if (dd.uboBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, dd.uboBuffer, nullptr);
        if (dd.uboMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, dd.uboMemory, nullptr);
    }
    m_deferredDestroys.clear();

    // 获取下一个可用交换链图像
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE,
                                            &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "[Vulkan] Failed to acquire swapchain image: %d\n", result);
        return;
    }

    // 重置并开始命令缓冲录制
    vkResetCommandBuffer(m_commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
    m_isRecording = true;

    // 开始渲染通道
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchainExtent;

    VkClearValue clearColor = {{{0.12f, 0.16f, 0.24f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_currentRenderPass = m_renderPass;
    m_currentFramebuffer = m_swapchainFramebuffers[m_currentImageIndex];
}
```

#### EndFrame

```cpp
void VulkanBackend::EndFrame() {
    if (!m_isRecording) return;

    // 渲染 ImGui（在结束渲染通道之前）
    if (m_imguiRenderPending) {
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->CmdListsCount > 0) {
            ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffer);
        }
        m_imguiRenderPending = false;
    }

    vkCmdEndRenderPass(m_commandBuffer);
    m_currentRenderPass = VK_NULL_HANDLE;
    m_currentFramebuffer = VK_NULL_HANDLE;

    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
    m_isRecording = false;

    // 提交命令缓冲
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence));

    // 呈现
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_currentImageIndex;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to present swapchain image: %d\n", result);
    }
}
```

#### 功能说明

`BeginFrame` 在每帧开始时执行帧同步、获取交换链图像、开始命令缓冲录制和渲染通道。`EndFrame` 在每帧结束时渲染 ImGui、结束渲染通道、提交命令缓冲并呈现结果。

#### 实现原理

帧同步采用"栅栏 + 信号量"双重机制：栅栏（Fence）用于 CPU 端同步，确保上一帧的命令已执行完毕；信号量（Semaphore）用于 GPU 端同步，确保交换链图像获取完成后才开始渲染，渲染完成后才开始呈现。

#### 为什么这样实现

1. **单帧缓冲**：只使用一个命令缓冲和一个栅栏，简化多缓冲的复杂性。
2. **延迟销毁**：在 `BeginFrame` 中清理上一帧标记为延迟销毁的资源，此时 GPU 已完成使用。
3. **自动重建交换链**：在 `OUT_OF_DATE` 或 `SUBOPTIMAL` 状态时自动重建交换链。
4. **ImGui 延迟渲染**：`ImGuiRender()` 只设置标志，实际渲染在 `EndFrame` 中执行。

---

### 6.13 ImGui 集成

```cpp
void VulkanBackend::ImGuiInit(GLFWwindow* window) {
    if (m_imguiInitialized) return;

    IMGUI_CHECKVERSION();
    if (ImGui::GetCurrentContext() == nullptr) {
        ImGui::CreateContext();
    }

    ImGui_ImplGlfw_InitForVulkan(window, true);

    // 加载字体：默认拉丁字体 + 中文字体合并
    ImGuiIO& io = ImGui::GetIO();
    const char* chineseFontPath = nullptr;

    // 检查微软雅黑
    {
        std::ifstream testFile("C:\\Windows\\Fonts\\msyh.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }
    // 回退：宋体
    if (!chineseFontPath) {
        std::ifstream testFile("C:\\Windows\\Fonts\\simsun.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\simsun.ttc";
    }

    if (chineseFontPath) {
        io.Fonts->AddFontDefault();
        ImFontConfig cfg;
        cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());
    }

    // 创建 ImGui 专用描述符池
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_imguiDescriptorPool));

    // 初始化 ImGui Vulkan 后端
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.QueueFamily = m_graphicsFamily;
    initInfo.Queue = m_graphicsQueue;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_imguiDescriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(m_swapchainImages.size());
    initInfo.PipelineInfoMain.RenderPass = m_renderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
    m_imguiInitialized = true;
}

void VulkanBackend::ImGuiNewFrame() {
    if (!m_imguiInitialized) return;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VulkanBackend::ImGuiRender() {
    if (!m_imguiInitialized) return;
    ImGui::Render();
    m_imguiRenderPending = true;
}

void VulkanBackend::ImGuiShutdown() {
    if (!m_imguiInitialized) return;
    WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    if (m_imguiDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
        m_imguiDescriptorPool = VK_NULL_HANDLE;
    }
    if (m_imguiDescSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device, m_imguiDescSetLayout, nullptr);
        m_imguiDescSetLayout = VK_NULL_HANDLE;
    }
    m_imguiInitialized = false;
}
```

#### 功能说明

ImGui Vulkan 集成包含四个函数：`ImGuiInit` 初始化 ImGui 的 Vulkan 后端（包括字体加载、描述符池创建），`ImGuiNewFrame` 开始新帧，`ImGuiRender` 生成绘制数据并标记待渲染，`ImGuiShutdown` 清理所有 ImGui 资源。

#### 实现原理

ImGui Vulkan 集成的关键设计：字体合并（先加载默认字体再以合并模式加载中文字体）、大型描述符池（覆盖所有 Vulkan 描述符类型）、延迟渲染（`ImGuiRender()` 只设置标志，实际 Vulkan 命令录制在 `EndFrame` 中执行）。

#### 为什么这样实现

1. **字体合并模式**：使用 `MergeMode = true` 而非替换默认字体，保持 ImGui 内置图标的正确渲染。
2. **大描述符池**：ImGui 的 Vulkan 后端需要多种描述符类型，预分配大型池避免运行时分配失败。
3. **延迟渲染标志**：将 ImGui 渲染分为"数据生成"和"命令录制"两步，确保 Vulkan 命令在活跃的渲染通道内录制。

---

### 6.14 Shutdown — 资源清理

```cpp
void VulkanBackend::Shutdown() {
    if (!m_initialized) return;

    WaitIdle(); // 等待 GPU 完成所有工作

    printf("[Vulkan] Shutting down...\n");

    // 清理所有管线
    for (auto& [id, pipeline] : m_pipelines) {
        if (pipeline->pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, pipeline->pipeline, nullptr);
        if (pipeline->layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, pipeline->layout, nullptr);
        if (pipeline->descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, pipeline->descSetLayout, nullptr);
        if (pipeline->descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, pipeline->descPool, nullptr);
    }
    m_pipelines.clear();

    // 清理所有纹理
    for (auto& [id, texture] : m_textures) {
        if (texture->sampler != VK_NULL_HANDLE) vkDestroySampler(m_device, texture->sampler, nullptr);
        if (texture->view != VK_NULL_HANDLE) vkDestroyImageView(m_device, texture->view, nullptr);
        if (texture->image != VK_NULL_HANDLE) vkDestroyImage(m_device, texture->image, nullptr);
        if (texture->memory != VK_NULL_HANDLE) vkFreeMemory(m_device, texture->memory, nullptr);
        if (texture->framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(m_device, texture->framebuffer, nullptr);
        if (texture->renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(m_device, texture->renderPass, nullptr);
    }
    m_textures.clear();

    // 清理所有着色器
    for (auto& [id, shader] : m_shaders) {
        if (shader.module != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, shader.module, nullptr);
    }
    m_shaders.clear();

    // 销毁同步对象
    if (m_inFlightFence != VK_NULL_HANDLE) { vkDestroyFence(m_device, m_inFlightFence, nullptr); m_inFlightFence = VK_NULL_HANDLE; }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) { vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr); m_renderFinishedSemaphore = VK_NULL_HANDLE; }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) { vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr); m_imageAvailableSemaphore = VK_NULL_HANDLE; }

    // 销毁命令池
    if (m_commandPool != VK_NULL_HANDLE) { vkDestroyCommandPool(m_device, m_commandPool, nullptr); m_commandPool = VK_NULL_HANDLE; }

    // 清理交换链
    CleanupSwapchain();

    // 销毁渲染通道
    if (m_renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(m_device, m_renderPass, nullptr); m_renderPass = VK_NULL_HANDLE; }

    // 销毁逻辑设备
    if (m_device != VK_NULL_HANDLE) { vkDestroyDevice(m_device, nullptr); m_device = VK_NULL_HANDLE; }

    // 销毁表面
    if (m_surface != VK_NULL_HANDLE) { vkDestroySurfaceKHR(m_instance, m_surface, nullptr); m_surface = VK_NULL_HANDLE; }

    // 销毁实例
    if (m_instance != VK_NULL_HANDLE) { vkDestroyInstance(m_instance, nullptr); m_instance = VK_NULL_HANDLE; }

    m_initialized = false;
    printf("[Vulkan] Shutdown complete\n");
}
```

#### 功能说明

按正确顺序销毁所有 Vulkan 资源。销毁顺序与创建顺序相反，确保没有资源在被销毁后仍被其他资源引用。

#### 实现原理

资源销毁遵循 Vulkan 规范要求的严格顺序：WaitIdle -> 管线/纹理/着色器 -> 同步对象 -> 命令池 -> 交换链 -> 渲染通道 -> 逻辑设备 -> 表面 -> 实例。

#### 为什么这样实现

1. **逆序销毁**：Vulkan 对象之间存在依赖关系，必须按创建的逆序销毁。
2. **WaitIdle 在最前**：确保 GPU 不再使用任何资源后再开始销毁。
3. **NULL 赋值**：每个句柄销毁后设为 `VK_NULL_HANDLE`，防止重复销毁。

---

## 7. 着色器加载与效果元数据

本章分析着色器加载系统（SPIR-V 二进制加载和目录定位）和效果元数据系统（手写 JSON 解析器）。这两个模块共同构成了着色器效果的发现、加载和参数化机制。

### 7.1 ShaderLoader — SPIR-V加载与目录定位

#### ShaderLoader.h

```cpp
#pragma once
#include <vector>
#include <cstdint>
#include <string>

class ShaderLoader {
public:
    // 从文件加载 SPIR-V 二进制数据，返回 uint32_t 数组
    static std::vector<uint32_t> LoadSPIRV(const std::string& filepath);
    // 查找着色器目录（相对于可执行文件的多级搜索）
    static std::string FindShaderDir();
};
```

#### ShaderLoader.cpp

```cpp
#include "shader/ShaderLoader.h"
#include <fstream>
#include <string>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

std::vector<uint32_t> ShaderLoader::LoadSPIRV(const std::string& filepath) {
    std::vector<uint32_t> result;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        fprintf(stderr, "[ShaderLoader] Cannot open SPIR-V file: %s\n", filepath.c_str());
        return result;
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        fprintf(stderr, "[ShaderLoader] Empty SPIR-V file: %s\n", filepath.c_str());
        return result;
    }

    if (size % sizeof(uint32_t) != 0) {
        fprintf(stderr, "[ShaderLoader] SPIR-V file size not multiple of 4: %s\n", filepath.c_str());
        return result;
    }

    result.resize(static_cast<size_t>(size) / sizeof(uint32_t));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(result.data()), size);
    file.close();

    printf("[ShaderLoader] Loaded SPIR-V: %s (%zu bytes, %zu words)\n",
           filepath.c_str(), static_cast<size_t>(size), result.size());
    return result;
}

std::string ShaderLoader::FindShaderDir() {
#ifdef _WIN32
    char exePathBuf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        std::string exeDir(exePathBuf, len);
        size_t slash = exeDir.find_last_of("\\/");
        if (slash != std::string::npos)
            exeDir = exeDir.substr(0, slash);

        // 尝试：exe_dir/../../shaders
        std::string candidate = exeDir + "/../../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        // 尝试：exe_dir/../shaders
        candidate = exeDir + "/../shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        // 尝试：exe_dir/shaders
        candidate = exeDir + "/shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;
    }
#else
    char exePathBuf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePathBuf, sizeof(exePathBuf) - 1);
    if (len > 0) {
        exePathBuf[len] = '\0';
        std::string exeDir(exePathBuf);
        size_t slash = exeDir.find_last_of('/');
        if (slash != std::string::npos)
            exeDir = exeDir.substr(0, slash);

        std::string candidate = exeDir + "/../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        candidate = exeDir + "/shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;
    }
#endif

    fprintf(stderr, "[ShaderLoader] Could not find shader directory\n");
    return "shaders";
}
```

#### 功能说明

`ShaderLoader` 提供两个静态方法：`LoadSPIRV` 从磁盘加载 SPIR-V 二进制着色器文件，`FindShaderDir` 自动定位着色器目录。

#### 实现原理

`LoadSPIRV` 以二进制模式打开文件，通过 `tellg()` 获取文件大小，验证大小为 4 的倍数（SPIR-V 规范要求），然后一次性读取全部内容到 `uint32_t` 数组中。`FindShaderDir` 通过获取可执行文件路径，向上搜索多个候选目录，用 `fullscreen.vert.spv` 的存在性作为探测依据。

#### 为什么这样实现

1. **静态方法设计**：`ShaderLoader` 不需要实例状态，所有方法都是静态的，使用简单。
2. **多级目录搜索**：适应不同的构建目录布局（Visual Studio 的 `build/bin/Release`、CMake 的 `build/bin` 等）。
3. **文件探测**：通过尝试打开已知文件来验证目录正确性，比检查目录是否存在更可靠。
4. **SPIR-V 大小验证**：确保文件是有效的 SPIR-V 二进制，避免将损坏的文件传递给 Vulkan 驱动。

---

### 7.2 EffectMetadata — 手写JSON解析器

#### EffectMetadata.h

```cpp
#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class ParamType { Float, Int, Bool, Float2, Float3, Float4, Color };

struct ShaderParam {
    std::string name;          // 参数名（对应着色器 uniform 变量名）
    std::string label;         // UI 显示标签（中文）
    ParamType type = ParamType::Float;
    float minVal = 0.0f, maxVal = 1.0f;
    float defaultVal[4] = {0,0,0,0};
    std::string uiType;        // slider, drag, combo, color, checkbox
    std::vector<std::string> comboOptions;
};

struct EffectCard {
    std::string id;
    std::string name;          // 效果名称（中文）
    std::string category;     // 分类
    std::string description;
    std::string thumbnailPath;
    std::string vertSpirvPath;
    std::string fragSpirvPath;
    int passes = 1;
    std::vector<ShaderParam> params;
};

struct UniformBinding {
    int location = -1;
    ParamType type = ParamType::Float;
    float currentValue[4] = {0,0,0,0};
};

EffectCard LoadEffectFromJson(const std::string& filepath);
```

#### EffectMetadata.cpp

```cpp
#include "shader/EffectMetadata.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>

namespace {

std::string Trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) ++start;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) --end;
    return s.substr(start, end - start);
}

std::string ReadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) { fprintf(stderr, "[EffectMetadata] Cannot open: %s\n", path.c_str()); return ""; }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string ExtractString(const std::string& json, size_t& pos) {
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t start = ++pos;
    while (pos < json.size()) {
        if (json[pos] == '\\') { pos += 2; continue; }
        if (json[pos] == '"') break;
        ++pos;
    }
    std::string result = json.substr(start, pos - start);
    ++pos;
    return result;
}

double ExtractNumber(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    size_t start = pos;
    if (json[pos] == '-') ++pos;
    while (pos < json.size() && (std::isdigit(static_cast<unsigned char>(json[pos])) || json[pos] == '.')) ++pos;
    return std::stod(json.substr(start, pos - start));
}

bool ExtractBool(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    if (json.compare(pos, 4, "true") == 0) { pos += 4; return true; }
    if (json.compare(pos, 5, "false") == 0) { pos += 5; return false; }
    return false;
}

std::vector<std::string> ExtractStringArray(const std::string& json, size_t& pos) {
    std::vector<std::string> result;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return result;
    ++pos;
    while (pos < json.size()) {
        if (json[pos] == ']') { ++pos; break; }
        if (json[pos] == ',' || std::isspace(static_cast<unsigned char>(json[pos]))) { ++pos; continue; }
        if (json[pos] == '"') { result.push_back(ExtractString(json, pos)); continue; }
        ++pos;
    }
    return result;
}

std::string GetStringValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return "";
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return "";
    ++p;
    while (p < scope.size() && std::isspace(static_cast<unsigned char>(scope[p]))) ++p;
    if (p >= scope.size() || scope[p] != '"') return "";
    --p;
    return ExtractString(scope, p);
}

double GetNumberValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return 0.0;
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return 0.0;
    ++p;
    return ExtractNumber(scope, p);
}

bool GetBoolValue(const std::string& scope, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t p = scope.find(pattern);
    if (p == std::string::npos) return false;
    p = scope.find(':', p + pattern.size());
    if (p == std::string::npos) return false;
    ++p;
    return ExtractBool(scope, p);
}

ParamType ParseParamType(const std::string& typeStr) {
    if (typeStr == "Float")  return ParamType::Float;
    if (typeStr == "Int")    return ParamType::Int;
    if (typeStr == "Bool")   return ParamType::Bool;
    if (typeStr == "Float2") return ParamType::Float2;
    if (typeStr == "Float3") return ParamType::Float3;
    if (typeStr == "Float4") return ParamType::Float4;
    if (typeStr == "Color")  return ParamType::Color;
    return ParamType::Float;
}

} // anonymous namespace

EffectCard LoadEffectFromJson(const std::string& filepath) {
    EffectCard card;

    FILE* test = fopen(filepath.c_str(), "rb");
    if (!test) { fprintf(stderr, "[EffectMetadata] File not found: %s\n", filepath.c_str()); return card; }
    fseek(test, 0, SEEK_END);
    long sz = ftell(test);
    fclose(test);
    if (sz <= 0 || sz > 102400) { fprintf(stderr, "[EffectMetadata] Invalid file size %ld for: %s\n", sz, filepath.c_str()); return card; }

    std::string json = ReadFile(filepath);
    if (json.empty()) { fprintf(stderr, "[EffectMetadata] Empty or missing file: %s\n", filepath.c_str()); return card; }

    // 移除 C 风格注释
    {
        std::string cleaned;
        cleaned.reserve(json.size());
        for (size_t i = 0; i < json.size(); ++i) {
            if (json[i] == '/' && i+1 < json.size()) {
                if (json[i+1] == '/') {
                    i += 2;
                    while (i < json.size() && json[i] != '\n') ++i;
                    if (i < json.size()) cleaned += '\n';
                    continue;
                }
                if (json[i+1] == '*') {
                    i += 2;
                    while (i+1 < json.size() && !(json[i] == '*' && json[i+1] == '/')) ++i;
                    i += 1;
                    continue;
                }
            }
            cleaned += json[i];
        }
        json = std::move(cleaned);
    }

    card.name        = GetStringValue(json, "name");
    card.category    = GetStringValue(json, "category");
    card.description = GetStringValue(json, "description");
    card.passes      = static_cast<int>(GetNumberValue(json, "passes"));
    if (card.passes < 1) card.passes = 1;

    // 解析 params 数组
    size_t paramsStart = json.find("\"params\"");
    if (paramsStart != std::string::npos) {
        paramsStart = json.find('[', paramsStart);
        if (paramsStart != std::string::npos) {
            int bracketDepth = 0;
            size_t paramsEnd = paramsStart;
            for (size_t i = paramsStart; i < json.size(); ++i) {
                if (json[i] == '[') ++bracketDepth;
                else if (json[i] == ']') { --bracketDepth; if (bracketDepth == 0) { paramsEnd = i; break; } }
            }

            size_t pos = paramsStart + 1;
            while (pos < paramsEnd) {
                pos = json.find('{', pos);
                if (pos == std::string::npos || pos >= paramsEnd) break;

                int depth = 0;
                size_t objEnd = pos;
                for (size_t i = pos; i < paramsEnd; ++i) {
                    if (json[i] == '{') ++depth;
                    else if (json[i] == '}') { --depth; if (depth == 0) { objEnd = i; break; } }
                }

                std::string paramScope = json.substr(pos, objEnd - pos + 1);

                ShaderParam param;
                param.name    = GetStringValue(paramScope, "name");
                param.label   = GetStringValue(paramScope, "label");
                param.type    = ParseParamType(GetStringValue(paramScope, "type"));
                param.minVal  = static_cast<float>(GetNumberValue(paramScope, "min"));
                param.maxVal  = static_cast<float>(GetNumberValue(paramScope, "max"));

                std::string defStr = GetStringValue(paramScope, "default");
                if (!defStr.empty()) {
                    try { param.defaultVal[0] = static_cast<float>(std::stod(defStr)); }
                    catch (...) { defStr.clear(); }
                }
                if (defStr.empty()) {
                    double defNum = GetNumberValue(paramScope, "default");
                    param.defaultVal[0] = static_cast<float>(defNum);
                }

                param.uiType = GetStringValue(paramScope, "ui_type");

                size_t comboPos = paramScope.find("\"combo_options\"");
                if (comboPos != std::string::npos) {
                    param.comboOptions = ExtractStringArray(paramScope, comboPos);
                }

                card.params.push_back(std::move(param));
                pos = objEnd + 1;
            }
        }
    }

    return card;
}
```

#### 功能说明

`EffectMetadata` 模块提供了一个零依赖的 JSON 解析器，专门用于解析 `effect.json` 文件。它从 JSON 中提取效果名称、分类、描述、参数列表等信息，填充到 `EffectCard` 结构体中。

#### 实现原理

解析器采用"字符串搜索 + 作用域提取"的方式工作：先移除注释，然后通过 `GetStringValue`、`GetNumberValue` 在整个 JSON 文本中搜索键值对。对于 `params` 数组，先定位 `"params"` 键，然后找到匹配的 `[...]`，在数组内逐个提取 `{...}` 参数对象。

#### 为什么这样实现

1. **零外部依赖**：不使用 nlohmann/json、rapidjson 等第三方库，减少项目依赖和编译时间。
2. **专用解析器**：只处理 effect.json 的特定结构，不需要通用 JSON 解析能力，代码更简洁。
3. **文件大小限制**：100KB 上限防止意外读取大文件。
4. **容错设计**：所有字段都有默认值，缺失字段不会导致崩溃。

---

### 7.3 Shader代码结构（fullscreen.vert + bloom.frag示例）

#### fullscreen.vert -- 全屏顶点着色器

```glsl
#version 460

layout(location=0) in vec2 aPos;      // 顶点位置（从 VAO 读取）
layout(location=0) out vec2 vUV;      // 输出 UV 坐标到片段着色器

void main() {
    vUV = (aPos + 1.0) * 0.5;        // 将 [-1,1] 映射到 [0,1] 作为纹理坐标
    gl_Position = vec4(aPos, 0, 1);   // 直接使用顶点位置作为裁剪空间坐标
}
```

#### bloom.frag -- 泛光片段着色器

```glsl
#version 460
layout(location=0) in vec2 vUV;       // 从顶点着色器接收的 UV 坐标
layout(location=0) out vec4 outColor;  // 输出颜色

layout(binding=0) uniform sampler2D uInputTex; // 输入纹理（binding=0）

layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 泛光强度
    float uParamFloat1;  // 阈值
    float uParamFloat2;  // 模糊大小
    float uParamFloat3;  // 预留参数
    float uParamFloat4;  // 预留参数
    float uParamFloat5;  // 预留参数
    vec2 uResolution;    // 分辨率
    float uTime;          // 时间
    float uFrameCount;    // 帧计数
};

void main() {
    vec3 color = texture(uInputTex, vUV).rgb; // 采样输入纹理获取原始颜色

    float BloomIntensity = uParamFloat0;
    float Threshold = uParamFloat1;
    float BlurSize = max(uParamFloat2, 1.0);

    if (BloomIntensity <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    vec2 texelSize = 1.0 / uResolution;

    int range = int(ceil(BlurSize));
    float sigma = BlurSize;
    float sigma2 = 2.0 * sigma * sigma;

    vec3 bloom = vec3(0.0);
    float weightSum = 0.0;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);

            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurSize;
            vec3 sampleColor = texture(uInputTex, vUV + offset).rgb;

            float sampleLuma = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));
            if (sampleLuma > Threshold) {
                float brightness = (sampleLuma - Threshold) / (1.0 - Threshold + 0.001);
                bloom += sampleColor * brightness * w;
            }
            weightSum += w;
        }
    }
    bloom /= max(weightSum, 0.001);

    outColor = vec4(color + bloom * BloomIntensity, 1.0);
}
```

#### 功能说明

`fullscreen.vert` 是全屏顶点着色器，接收 VAO 中的顶点位置，将其转换为纹理坐标并直接输出为裁剪空间坐标。`bloom.frag` 是泛光后处理着色器，实现了带亮度阈值的高斯模糊泛光效果。

#### 实现原理

泛光着色器的核心算法：使用 ITU-R BT.709 标准计算像素亮度，只有亮度超过阈值的像素才参与泛光计算；在以当前像素为中心的矩形区域内采样，每个采样点的权重由高斯函数决定；最终将归一化的泛光颜色乘以强度后加到原始颜色上。

#### 为什么这样实现

1. **单通道实现**：将亮度提取和高斯模糊合并为单通道，简化了多通道泛光的复杂性，适合实时预览。
2. **BlurSize 参数化**：同时影响采样半径和采样偏移量，产生从轻微发光到强烈光晕的连续可调效果。
3. **早期退出**：当 `BloomIntensity <= 0` 时直接返回原始颜色，避免不必要的采样循环。
4. **std140 布局**：UBO 使用 `std140` 布局规则，确保 CPU 和 GPU 的内存布局一致。

---

### 7.4 effect.json参数化机制

```json
{
  "name": "泛光",
  "category": "Lighting",
  "description": "Single-pass bloom effect with brightness threshold extraction, 13-tap Gaussian blur approximation, and additive blending",
  "passes": 1,
  "params": [
    {
      "name": "uParamFloat0",
      "label": "泛光强度",
      "type": "Float",
      "min": 0.0,
      "max": 3.0,
      "default": 0.8,
      "ui_type": "slider"
    },
    {
      "name": "uParamFloat1",
      "label": "阈值",
      "type": "Float",
      "min": 0.0,
      "max": 1.0,
      "default": 0.7,
      "ui_type": "slider"
    },
    {
      "name": "uParamFloat2",
      "label": "模糊大小",
      "type": "Float",
      "min": 1.0,
      "max": 10.0,
      "default": 4.0,
      "ui_type": "slider"
    }
  ]
}
```

#### 功能说明

`effect.json` 是效果元数据文件，描述了一个着色器效果的名称、分类、参数等信息。UI 层读取此文件后自动生成参数控制面板。

#### 实现原理

JSON 文件中的每个参数对象包含：`name`（对应着色器 UBO 中的 uniform 变量名）、`label`（UI 显示的中文名称）、`type`（参数类型）、`min`/`max`（值范围）、`default`（默认值）、`ui_type`（UI 控件类型）。

#### 为什么这样实现

1. **声明式配置**：将效果参数从代码中分离到 JSON 文件，添加新效果无需修改 C++ 代码。
2. **中文标签**：`label` 字段使用中文，直接用于 UI 显示，无需额外的本地化系统。
3. **名称映射**：`name` 字段直接对应 UBO 中的变量名，建立了 JSON 参数与着色器 uniform 之间的映射关系。

---

### 7.5 UBO固定布局设计原理

着色器中的 UBO（Uniform Buffer Object）采用 `std140` 布局规则，固定布局如下：

```
偏移量    字段              类型        大小
0-3      uParamFloat0     float       4 字节
4-7      uParamFloat1     float       4 字节
8-11     uParamFloat2     float       4 字节
12-15    uParamFloat3     float       4 字节
16-19    uParamFloat4     float       4 字节
20-23    uParamFloat5     float       4 字节
24-31    uResolution      vec2        8 字节
32-35    uTime            float       4 字节
36-39    uFrameCount      float       4 字节
总计: 40 字节（实际分配 48 字节，包含 std140 对齐填充）
```

#### 功能说明

UBO 是 CPU 和 GPU 之间传递着色器参数的桥梁。固定布局确保 CPU 端写入的数据能被 GPU 端正确读取。

#### 实现原理

`std140` 是 GLSL/Vulkan 标准的 UBO 布局规则：`float` 占 4 字节对齐到 4 字节边界，`vec2` 占 8 字节对齐到 8 字节边界。在本项目中，6 个 `float` 连续排列（偏移 0-23），`vec2` 从偏移 24 开始（24 是 8 的倍数），`float` 从偏移 32 开始。UBO 总大小为 48 字节。

#### 为什么这样实现

1. **std140 标准布局**：跨平台、跨编译器保证一致的内存布局。
2. **固定参数槽**：6 个 `float` 参数槽为所有效果提供统一的参数接口。
3. **48 字节对齐**：UBO 大小为 48 字节，满足 Vulkan 对 UBO 大小的对齐要求。
4. **CPU 端精确映射**：`DrawFullscreenQuad` 中通过 `memcpy(uboData + i * 4, &v, sizeof(float))` 精确写入每个参数。

---

## 8. 输入源与UI组件

本章分析三个辅助模块：视频播放器（FFmpeg 子进程管道解码）、屏幕捕获（DXGI 桌面复制）和性能面板（FPS 显示与后端切换）。

### 8.1 VideoPlayer -- FFmpeg子进程管道解码

#### VideoPlayer.h

```cpp
#pragma once

// VideoPlayer -- 使用 ffmpeg 子进程的轻量级视频帧读取器。
// 打开视频文件，通过管道从 ffmpeg 读取解码后的 RGBA 帧。
// 无需链接 FFmpeg 库——只需 ffmpeg.exe 在 PATH 中。

#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#define VIDEO_PIPE_TYPE HANDLE
#else
#define VIDEO_PIPE_TYPE int
#endif

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool Open(const std::string& filePath);  // 打开视频文件
    bool ReadFrame();                         // 读取下一帧
    void Seek(double seconds);                // 跳转（当前为空操作）
    void Close();                             // 关闭视频并释放资源

    bool IsOpen() const { return m_open; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    double GetDuration() const { return m_duration; }
    double GetCurrentTime() const { return m_currentTime; }
    const uint8_t* GetPixels() const { return m_pixels.data(); }
    double GetFPS() const { return m_fps; }

private:
    bool StartFFmpegProcess(const std::string& filePath);
    void StopProcess();

    VIDEO_PIPE_TYPE m_pipeRead = nullptr;
#ifdef _WIN32
    HANDLE m_processHandle = nullptr;
#endif

    int    m_width = 0;
    int    m_height = 0;
    double m_duration = 0.0;
    double m_fps = 30.0;
    double m_currentTime = 0.0;
    bool   m_open = false;

    std::vector<uint8_t> m_pixels;
    std::vector<uint8_t> m_readBuf;

    static constexpr size_t READ_BUF_SIZE = 4 * 1024 * 1024; // 4MB 读取缓冲区
};
```

#### VideoPlayer.cpp（核心函数）

```cpp
#include "input/VideoPlayer.h"

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#include <cstring>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

VideoPlayer::VideoPlayer() {
    m_readBuf.resize(READ_BUF_SIZE); // 预分配 4MB 读取缓冲区
}

VideoPlayer::~VideoPlayer() {
    Close();
}

void VideoPlayer::Close() {
    StopProcess();
    m_open = false;
    m_pixels.clear();
    m_width = 0;
    m_height = 0;
}

bool VideoPlayer::Open(const std::string& filePath) {
    Close();

    if (!StartFFmpegProcess(filePath)) {
        fprintf(stderr, "[VideoPlayer] Failed to start ffmpeg process\n");
        return false;
    }

    if (!ReadFrame()) {
        fprintf(stderr, "[VideoPlayer] Failed to read first frame\n");
        Close();
        return false;
    }

    m_open = true;
    printf("[VideoPlayer] Opened: %s (%d x %d, %.1f fps, %.1f sec)\n",
           filePath.c_str(), m_width, m_height, m_fps, m_duration);
    return true;
}

bool VideoPlayer::ReadFrame() {
    if (!m_pipeRead) return false;

    size_t needed = (size_t)m_width * m_height * 4; // RGBA 每像素 4 字节
    if (needed == 0) return false;

    m_pixels.resize(needed);
    size_t totalRead = 0;

    while (totalRead < needed) {
        size_t toRead = needed - totalRead;
        if (toRead > m_readBuf.size()) toRead = m_readBuf.size();

#ifdef _WIN32
        DWORD bytesRead = 0;
        if (!ReadFile(m_pipeRead, m_readBuf.data(), (DWORD)toRead, &bytesRead, nullptr)) {
            return false; // 管道关闭或错误
        }
        if (bytesRead == 0) return false;
#else
        ssize_t bytesRead = read((int)m_pipeRead, m_readBuf.data(), toRead);
        if (bytesRead <= 0) return false;
#endif

        memcpy(m_pixels.data() + totalRead, m_readBuf.data(), bytesRead);
        totalRead += bytesRead;
    }

    m_currentTime += 1.0 / m_fps;
    return true;
}

void VideoPlayer::Seek(double seconds) {
    (void)seconds; // 管道读取器不支持跳转
}

#ifdef _WIN32

bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
    // 第一步：使用 ffprobe 探测视频信息
    char probeCmd[1024];
    snprintf(probeCmd, sizeof(probeCmd),
        "ffprobe -v error -select_streams v:0 -show_entries "
        "stream=width,height,r_frame_rate,duration -of csv=p=0 \"%s\"",
        filePath.c_str());

    FILE* probePipe = _popen(probeCmd, "r");
    if (probePipe) {
        char line[512];
        if (fgets(line, sizeof(line), probePipe)) {
            int w = 0, h = 0;
            float fpsNum = 0, fpsDen = 1;
            double dur = 0;
            if (sscanf(line, "%d,%d,%f/%f,%lf", &w, &h, &fpsNum, &fpsDen, &dur) >= 2) {
                m_width = w;
                m_height = h;
                if (fpsDen > 0) m_fps = fpsNum / fpsDen;
                m_duration = dur;
            }
        }
        _pclose(probePipe);
    }

    if (m_width == 0 || m_height == 0) {
        m_width = 1920; m_height = 1080; m_fps = 30.0; m_duration = 10.0;
    }

    m_pixels.resize((size_t)m_width * m_height * 4);

    // 创建管道
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        fprintf(stderr, "[VideoPlayer] CreatePipe failed\n");
        return false;
    }

    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    // 构建 ffmpeg 命令
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "ffmpeg -i \"%s\" -loglevel error -f rawvideo -pix_fmt rgba "
        "-s %dx%d -r 30 pipe:1",
        filePath.c_str(), m_width, m_height);

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

    if (!CreateProcessA(nullptr, cmd, nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        fprintf(stderr, "[VideoPlayer] CreateProcess failed for ffmpeg\n");
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }

    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    m_pipeRead = hRead;
    m_processHandle = pi.hProcess;
    return true;
}

void VideoPlayer::StopProcess() {
    if (m_pipeRead) {
        CloseHandle((HANDLE)m_pipeRead);
        m_pipeRead = nullptr;
    }
    if (m_processHandle) {
        TerminateProcess(m_processHandle, 0);
        WaitForSingleObject(m_processHandle, 1000);
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
}

#endif
```

#### 功能说明

`VideoPlayer` 通过启动 FFmpeg 子进程并将输出重定向到管道，实现视频帧的逐帧读取。FFmpeg 将视频解码为 RGBA 原始像素数据，通过管道传输给主进程。

#### 实现原理

工作流程分为两步：探测阶段使用 `ffprobe` 获取视频的宽度、高度、帧率和时长；解码阶段启动 `ffmpeg` 子进程，命令行参数指定输出为 `rawvideo` 格式、`rgba` 像素格式，输出到 `pipe:1`（标准输出）。主进程通过管道逐帧读取 RGBA 数据。

#### 为什么这样实现

1. **零库依赖**：不需要链接 FFmpeg 的 libavcodec/libavformat 库，只需系统上安装 `ffmpeg.exe`。
2. **管道通信**：FFmpeg 的 `pipe:1` 输出模式天然适合流式帧读取。
3. **4MB 缓冲区**：较大的读取缓冲区减少系统调用次数，提高读取效率。
4. **CREATE_NO_WINDOW**：Windows 上创建无窗口的子进程，避免弹出控制台窗口。

---

### 8.2 ScreenCapture -- DXGI桌面捕获

#### ScreenCapture.h

```cpp
#pragma once

// DXGI Desktop Duplication 屏幕捕获
// 捕获主显示器并提供 RGBA8 像素数据

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

#include <vector>
#include <cstdint>
#include <string>

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    bool Init();        // 初始化 DXGI Desktop Duplication
    void Shutdown();     // 关闭并释放所有资源
    bool CaptureFrame(); // 捕获一帧桌面图像

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const uint8_t* GetPixels() const { return m_pixels.data(); }
    bool IsReady() const { return m_ready; }

private:
#ifdef _WIN32
    ID3D11Device*           m_d3dDevice        = nullptr;
    ID3D11DeviceContext*    m_d3dContext        = nullptr;
    IDXGIOutputDuplication* m_deskDupl          = nullptr;
    ID3D11Texture2D*        m_stagingTex        = nullptr;
#endif

    int m_width  = 0;
    int m_height = 0;
    bool m_ready = false;

    std::vector<uint8_t> m_pixels;
};
```

#### ScreenCapture.cpp（核心函数）

```cpp
#include "input/ScreenCapture.h"

#ifdef _WIN32
#include <cstdio>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static IDXGIOutput* FindPrimaryOutput(IDXGIFactory1* factory, IDXGIAdapter1*& outAdapter) {
    outAdapter = nullptr;
    for (UINT i = 0; ; i++) {
        IDXGIAdapter1* adapter = nullptr;
        if (factory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND)
            break;

        for (UINT j = 0; ; j++) {
            IDXGIOutput* output = nullptr;
            if (adapter->EnumOutputs(j, &output) == DXGI_ERROR_NOT_FOUND)
                break;

            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);

            if (desc.AttachedToDesktop) {
                outAdapter = adapter;
                printf("[ScreenCapture] Found output: %ls (%d x %d)\n",
                       desc.DeviceName,
                       desc.DesktopCoordinates.right - desc.DesktopCoordinates.left,
                       desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
                return output;
            }
            output->Release();
        }
        adapter->Release();
    }
    return nullptr;
}

ScreenCapture::ScreenCapture() = default;

ScreenCapture::~ScreenCapture() {
    Shutdown();
}

bool ScreenCapture::Init() {
    if (m_ready) return true;

    // 创建 D3D11 设备
    D3D_FEATURE_LEVEL featLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &m_d3dDevice, &featLevel, &m_d3dContext);

    if (FAILED(hr)) {
        fprintf(stderr, "[ScreenCapture] D3D11CreateDevice failed: 0x%08X\n", (unsigned)hr);
        return false;
    }

    // 获取 DXGI Factory
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) { Shutdown(); return false; }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) { Shutdown(); return false; }

    IDXGIFactory1* factory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&factory);
    dxgiAdapter->Release();
    if (FAILED(hr)) { Shutdown(); return false; }

    // 查找主输出
    IDXGIAdapter1* foundAdapter = nullptr;
    IDXGIOutput* output = FindPrimaryOutput(factory, foundAdapter);
    factory->Release();

    if (!output) {
        fprintf(stderr, "[ScreenCapture] No desktop output found\n");
        if (foundAdapter) foundAdapter->Release();
        Shutdown();
        return false;
    }

    // 获取 IDXGIOutput1
    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr)) { Shutdown(); return false; }

    // 创建桌面复制接口
    hr = output1->DuplicateOutput(m_d3dDevice, &m_deskDupl);
    output1->Release();
    if (FAILED(hr)) {
        fprintf(stderr, "[ScreenCapture] DuplicateOutput failed: 0x%08X\n", (unsigned)hr);
        Shutdown();
        return false;
    }

    DXGI_OUTDUPL_DESC duplDesc;
    m_deskDupl->GetDesc(&duplDesc);
    m_width  = duplDesc.ModeDesc.Width;
    m_height = duplDesc.ModeDesc.Height;

    m_pixels.resize(m_width * m_height * 4);
    m_ready = true;
    return true;
}

void ScreenCapture::Shutdown() {
    if (m_stagingTex) { m_stagingTex->Release(); m_stagingTex = nullptr; }
    if (m_deskDupl)   { m_deskDupl->Release();   m_deskDupl   = nullptr; }
    if (m_d3dContext) { m_d3dContext->Release(); m_d3dContext = nullptr; }
    if (m_d3dDevice)  { m_d3dDevice->Release();  m_d3dDevice  = nullptr; }
    m_ready = false;
}

bool ScreenCapture::CaptureFrame() {
    if (!m_ready) return false;

    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_deskDupl->AcquireNextFrame(16, &frameInfo, &desktopResource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false; // 无新帧
    }

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            fprintf(stderr, "[ScreenCapture] Access lost, re-initializing...\n");
            Shutdown();
            return Init(); // 尝试重新初始化
        }
        fprintf(stderr, "[ScreenCapture] AcquireNextFrame failed: 0x%08X\n", (unsigned)hr);
        return false;
    }

    // 获取桌面纹理
    ID3D11Texture2D* desktopTex = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTex);
    desktopResource->Release();
    if (FAILED(hr)) {
        m_deskDupl->ReleaseFrame();
        return false;
    }

    // 首次捕获时创建暂存纹理
    if (!m_stagingTex) {
        D3D11_TEXTURE2D_DESC desc = {};
        desktopTex->GetDesc(&desc);
        desc.Usage          = D3D11_USAGE_STAGING;
        desc.BindFlags      = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags      = 0;
        hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, &m_stagingTex);
        if (FAILED(hr)) {
            desktopTex->Release();
            m_deskDupl->ReleaseFrame();
            return false;
        }
    }

    // 复制桌面纹理到暂存纹理
    m_d3dContext->CopyResource(m_stagingTex, desktopTex);
    desktopTex->Release();

    // 映射暂存纹理并读取像素
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_d3dContext->Map(m_stagingTex, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        m_deskDupl->ReleaseFrame();
        return false;
    }

    // BGRA -> RGBA 转换 + 行距处理
    const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
    uint8_t* dst = m_pixels.data();
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            dst[(y * m_width + x) * 4 + 0] = src[y * mapped.RowPitch + x * 4 + 2]; // R <- B
            dst[(y * m_width + x) * 4 + 1] = src[y * mapped.RowPitch + x * 4 + 1]; // G <- G
            dst[(y * m_width + x) * 4 + 2] = src[y * mapped.RowPitch + x * 4 + 0]; // B <- R
            dst[(y * m_width + x) * 4 + 3] = src[y * mapped.RowPitch + x * 4 + 3]; // A <- A
        }
    }

    m_d3dContext->Unmap(m_stagingTex, 0);
    m_deskDupl->ReleaseFrame();

    return true;
}

#else
ScreenCapture::ScreenCapture() = default;
ScreenCapture::~ScreenCapture() = default;
bool ScreenCapture::Init() { return false; }
void ScreenCapture::Shutdown() {}
bool ScreenCapture::CaptureFrame() { return false; }
#endif
```

#### 功能说明

`ScreenCapture` 使用 Windows DXGI Desktop Duplication API 捕获主显示器内容，将其转换为 RGBA8 像素数据供后处理使用。

#### 实现原理

初始化过程：创建 D3D11 设备 -> 获取 DXGI Factory -> 枚举适配器和输出 -> 找到连接桌面的主输出 -> 调用 `DuplicateOutput` 创建桌面复制接口。

捕获过程：调用 `AcquireNextFrame` 获取桌面纹理 -> 复制到 CPU 可读的 Staging 纹理 -> 映射纹理内存 -> 逐像素 BGRA 到 RGBA 转换 -> 释放帧。

#### 为什么这样实现

1. **DXGI Desktop Duplication**：这是 Windows 上最高效的桌面捕获 API，直接获取 GPU 端的桌面纹理，无需 GDI/BitBlt 的 CPU 拷贝。
2. **Staging 纹理**：GPU 纹理不能直接被 CPU 读取，必须先复制到 Staging 纹理（`D3D11_USAGE_STAGING` + `CPU_ACCESS_READ`）。
3. **BGRA 到 RGBA 转换**：Windows 桌面使用 BGRA 像素格式，而后处理管线使用 RGBA，需要在捕获时转换。
4. **行距处理**：使用 `mapped.RowPitch` 而非 `width * 4`，因为 GPU 纹理的行距可能包含填充字节。
5. **自动恢复**：当 `DXGI_ERROR_ACCESS_LOST` 发生时（如模式切换、UAC 弹窗），自动重新初始化捕获。

---

### 8.3 PerformancePanel -- FPS性能面板

#### PerformancePanel.h

```cpp
#pragma once
#include "render/BackendType.h"
#include <imgui.h>

class Application;
class IRenderBackend;

class PerformancePanel {
public:
    void Render(Application* app, IRenderBackend* backend);

private:
    float m_lastFps = 0.0f;       // 上次计算的 FPS 值
    int m_fpsFrameCount = 0;       // FPS 计数器
    float m_fpsElapsed = 0.0f;     // FPS 计时器

    void UpdateFPS();
    void RenderBackendButtons(Application* app);
    void RenderFPSDisplay(Application* app);
};
```

#### PerformancePanel.cpp

```cpp
#include "PerformancePanel.h"
#include "app/Application.h"
#include "render/IRenderBackend.h"
#include <imgui.h>

void PerformancePanel::Render(Application* app, IRenderBackend* backend) {
    UpdateFPS(); // 更新 FPS 计算

    // 计算面板位置（右上角）
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float panelWidth = 100.0f;
    float panelHeight = 70.0f;
    ImVec2 pos(windowSize.x - panelWidth - 8.0f, 8.0f);

    ImGui::SetNextWindowPos(pos); // 固定位置
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight)); // 固定大小

    // 窗口标志：无标题栏、不可调整大小、不可移动、无滚动条
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar;

    // 设置半透明黑色背景和圆角
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

    if (ImGui::Begin("##PerformancePanel", nullptr, flags)) {
        RenderBackendButtons(app); // 渲染后端切换按钮
        RenderFPSDisplay(app);      // 渲染 FPS 显示
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void PerformancePanel::UpdateFPS() {
    float dt = ImGui::GetIO().DeltaTime; // 获取帧间隔时间
    m_fpsElapsed += dt;                  // 累加时间
    m_fpsFrameCount++;                   // 累加帧数

    if (m_fpsElapsed >= 1.0f) {         // 每秒更新一次 FPS
        m_lastFps = m_fpsFrameCount / m_fpsElapsed; // 计算平均 FPS
        m_fpsFrameCount = 0;            // 重置计数器
        m_fpsElapsed = 0.0f;            // 重置计时器
    }
}

void PerformancePanel::RenderBackendButtons(Application* app) {
    BackendType current = app->GetBackendType();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

    // OpenGL 按钮
    bool isOpenGL = (current == BackendType::OpenGL);
    if (isOpenGL) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f)); // 激活状态：亮灰
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f)); // 非激活：暗灰半透明
    }
    if (ImGui::Button("GL", ImVec2(32, 20))) {
        if (!isOpenGL) app->SwitchBackend(BackendType::OpenGL); // 点击切换到 OpenGL
    }
    ImGui::PopStyleColor();

    ImGui::SameLine(); // 同一行显示

    // Vulkan 按钮
    bool isVulkan = (current == BackendType::Vulkan);
#ifdef USE_VULKAN_BACKEND
    if (isVulkan) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    }
    if (ImGui::Button("VK", ImVec2(32, 20))) {
        if (!isVulkan) app->SwitchBackend(BackendType::Vulkan); // 点击切换到 Vulkan
    }
    ImGui::PopStyleColor();
#else
    // Vulkan 未编译时显示灰色禁用按钮
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
    ImGui::Button("VK", ImVec2(32, 20));
    ImGui::PopStyleColor(2);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Vulkan not available"); // 悬停提示
    }
#endif

    ImGui::PopStyleVar();
}

void PerformancePanel::RenderFPSDisplay(Application* app) {
    // FPS 数值（绿色显示）
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.33f, 1.0f, 0.33f, 1.0f));
    ImGui::Text("%.1f FPS", m_lastFps);
    ImGui::PopStyleColor();

    // 后端名称（灰色显示）
    const char* backendLabel = (app->GetBackendType() == BackendType::Vulkan) ? "Vulkan 1.2" : "OpenGL 4.6";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 0.53f, 0.53f, 1.0f));
    ImGui::TextUnformatted(backendLabel);
    ImGui::PopStyleColor();
}
```

#### 功能说明

`PerformancePanel` 是一个轻量级的 ImGui 面板组件，固定在窗口右上角，显示当前 FPS 和后端名称，并提供 OpenGL/Vulkan 后端切换按钮。

#### 实现原理

面板使用 ImGui 的固定位置和固定大小窗口（`NoTitleBar | NoResize | NoMove | NoScrollbar`），背景设为半透明黑色（alpha=0.75）并带圆角。FPS 通过累加帧数和帧间隔时间，每秒计算一次平均值。后端切换按钮使用条件编译（`#ifdef USE_VULKAN_BACKEND`），未编译 Vulkan 时显示灰色禁用按钮。

#### 为什么这样实现

1. **固定位置窗口**：使用 `SetNextWindowPos` 和 `SetNextWindowSize` 确保面板始终在右上角，不受 ImGui 布局系统影响。
2. **每秒更新 FPS**：避免 FPS 数值频繁跳动，提供稳定的显示。
3. **条件编译**：通过 `USE_VULKAN_BACKEND` 宏控制 Vulkan 按钮的显示状态，在未编译 Vulkan 支持时优雅降级。
4. **视觉反馈**：激活的后端按钮使用较亮的颜色（0.27），非激活的按钮使用较暗的半透明颜色（0.2, alpha=0.6），提供清晰的视觉区分。
