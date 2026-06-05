# Shader Showcase 源码详解

## 目录

- [1. 程序入口 — main.cpp](#1-程序入口--maincpp)
  - [1.1 FindAssetDir — 资源目录定位](#11-findassetdir--资源目录定位)
  - [1.2 main — 主函数](#12-main--主函数)
- [2. 应用核心 — Application](#2-应用核心--application)
  - [2.1 Application.h — 类定义](#21-applicationh--类定义)
  - [2.2 构造与析构](#22-构造与析构)
  - [2.3 Run — 应用启动](#23-run--应用启动)
  - [2.4 InitBackend — 后端初始化](#24-initbackend--后端初始化)
  - [2.5 MainLoop — 主循环](#25-mainloop--主循环)
  - [2.6 Shutdown — 资源清理](#26-shutdown--资源清理)
  - [2.7 SetScene / SwitchBackend](#27-setscene--switchbackend)
  - [2.8 回调函数](#28-回调函数)
- [3. CoverFlowScene — 封面流场景](#3-coverflowscene--封面流场景)
  - [3.1 类定义](#31-类定义)
  - [3.2 RegisterCards — 卡片注册](#32-registercards--卡片注册)
  - [3.3 OnEnter / OnExit](#33-onenter--onexit)
  - [3.4 OnUpdate](#34-onupdate)
  - [3.5 OnRender + RenderVisibleThumbnails](#35-onrender--rendervisiblethumbnails)
  - [3.6 OnImGui](#36-onimgui)
  - [3.7 OpenSelectedEffect / GetNextScene](#37-openselectedeffect--getnextscene)
  - [3.8 GetState](#38-getstate)
  - [3.9 辅助功能](#39-辅助功能)

---

## 1. 程序入口 — main.cpp

`main.cpp` 是整个 Shader Showcase 程序的入口文件。它负责初始化运行环境、定位资源目录、创建应用实例、加载纹理资源并启动主循环。文件包含两个核心部分：`FindAssetDir` 辅助函数和 `main` 主函数。

### 1.1 FindAssetDir — 资源目录定位

`FindAssetDir` 是一个静态辅助函数，用于在运行时定位 `assets` 资源目录的绝对路径。由于程序可能从不同的工作目录启动（例如 IDE 调试、命令行运行、打包发布等），资源目录相对于可执行文件的位置可能不同，因此需要通过试探性查找来确定正确的路径。

整个函数可以分为以下逻辑段：

#### 第一段：获取可执行文件路径

**分析**：在 Windows 平台下，使用 `GetModuleFileNameA` API 获取当前可执行文件的完整路径。该函数将路径写入 `buf` 缓冲区，并返回路径长度。通过检查返回值确保路径有效且未溢出缓冲区。

```cpp
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
```

#### 第二段：提取目录并逐级搜索

**分析**：从完整路径中找到最后一个路径分隔符（`\` 或 `/`），截取得到可执行文件所在的目录。然后按照从远到近的顺序，依次在 `../../../assets`、`../../assets`、`../assets`、`assets` 四个候选相对路径下查找 `test.jpg` 文件。找到第一个存在的路径即返回。这种多级搜索策略覆盖了从项目根目录、build 目录到可执行文件同级目录等多种部署场景。

```cpp
        std::string exePath(buf, (size_t)len);
        auto sl = exePath.find_last_of("\\/");
        if (sl != std::string::npos) exePath = exePath.substr(0, sl);
        for (const char* rel : {"../../../assets", "../../assets", "../assets", "assets"}) {
            std::string test = exePath + "/" + rel + "/test.jpg";
            FILE* f = fopen(test.c_str(), "rb");
            if (f) { fclose(f); return exePath + "/" + rel; }
        }
    }
#endif
```

#### 第三段：默认回退

**分析**：如果所有搜索路径均未找到资源目录（例如非 Windows 平台），则返回简单的相对路径 `"assets"` 作为默认值。这意味着程序期望在当前工作目录下直接存在 `assets` 文件夹。

```cpp
    return "assets";
```

#### 完整源码

```cpp
// 静态辅助函数：在运行时定位 assets 资源目录的绝对路径
static std::string FindAssetDir() {
#ifdef _WIN32
    char buf[MAX_PATH];                                    // 路径缓冲区，MAX_PATH 通常为 260
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH); // 获取可执行文件完整路径
    if (len > 0 && len < MAX_PATH) {                       // 确保路径有效且未溢出
        std::string exePath(buf, (size_t)len);             // 将路径转为 std::string
        auto sl = exePath.find_last_of("\\/");              // 查找最后一个路径分隔符
        if (sl != std::string::npos) exePath = exePath.substr(0, sl); // 截取目录部分
        // 按从远到近的顺序搜索候选路径
        for (const char* rel : {"../../../assets", "../../assets", "../assets", "assets"}) {
            std::string test = exePath + "/" + rel + "/test.jpg"; // 拼接测试文件路径
            FILE* f = fopen(test.c_str(), "rb");            // 尝试以二进制模式打开测试文件
            if (f) { fclose(f); return exePath + "/" + rel; } // 找到则关闭文件并返回路径
        }
    }
#endif
    return "assets";                                        // 默认回退到当前目录下的 assets
}
```

### 1.2 main — 主函数

`main` 是程序的入口点，负责完成从环境初始化到应用启动的全部准备工作。整个函数按照调用链可以分为以下逻辑段：

#### 第一段：关闭 I/O 缓冲

**分析**：调用 `setvbuf` 将 `stdout` 和 `stderr` 的缓冲模式设为无缓冲（`_IONBF`）。这意味着所有通过 `printf`/`fprintf` 输出的内容会立即写入控制台，不会被缓冲延迟。这在调试和自动测试场景中尤为重要，可以确保日志实时可见，避免程序崩溃时丢失缓冲区中的日志。

```cpp
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
```

#### 第二段：自动测试环境变量检测

**分析**：通过读取环境变量 `AUTO_TEST` 和 `AUTO_TEST_DETAILS` 来判断是否启用自动测试模式。`AUTO_TEST=1` 表示启用基本的自动测试（循环浏览所有卡片），`AUTO_TEST_DETAILS=1` 表示启用详情页截图模式。两个变量任一为 `"1"` 即开启自动测试。自动测试模式用于 CI/CD 流水线中验证所有着色器效果是否正常渲染。

```cpp
    const char* autoTestEnv = getenv("AUTO_TEST");
    bool autoTest = (autoTestEnv && strcmp(autoTestEnv, "1") == 0);
    if (getenv("AUTO_TEST_DETAILS") && strcmp(getenv("AUTO_TEST_DETAILS"), "1") == 0) {
        autoTest = true;
    }
```

#### 第三段：创建 Application 并设置帧回调

**分析**：创建 `Application` 实例，然后通过 `SetFrameCallback` 注册一个 lambda 回调函数。该回调在主循环中每帧被调用，用于在场景被销毁后（例如后端切换后）重新创建 `CoverFlowScene`。回调首先检查当前场景是否已存在，若已存在则直接返回，避免重复创建。

```cpp
    Application app;

    app.SetFrameCallback([&](float dt) {
        (void)dt;
        if (app.GetCurrentScene() != nullptr) return;

        IRenderBackend* backend = app.GetBackend();
        if (!backend) return;
```

#### 第四段：加载默认测试图片

**分析**：在回调内部，首先调用 `FindAssetDir()` 获取资源目录，然后加载 `test.jpg` 作为默认输入纹理。使用 `stbi_load` 加载图片数据，注意在加载前设置 `stbi_set_flip_vertically_on_load(true)` 以适配 OpenGL 的左下角原点坐标系。加载完成后通过后端接口创建 GPU 纹理，并释放 CPU 端的图片数据。

```cpp
        std::string assetDir = FindAssetDir();
        std::string jpgPath  = assetDir + "/test.jpg";
        int iw = 0, ih = 0, comp = 0;
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* imgData = stbi_load(jpgPath.c_str(), &iw, &ih, &comp, 4);
        stbi_set_flip_vertically_on_load(false);
        if (!imgData) {
            fprintf(stderr, "[main] Cannot load test image: %s\n", jpgPath.c_str());
            return;
        }
        printf("[main] Loaded image: %s (%d x %d)\n", jpgPath.c_str(), iw, ih);

        TextureHandle inputTex = backend->CreateTexture(iw, ih, TextureFormat::RGBA8, imgData);
        stbi_image_free(imgData);
```

#### 第五段：定义测试图片列表并查找图片目录

**分析**：定义了 18 个着色器效果对应的测试图片文件名数组，每个效果都有专门的测试图片以获得最佳展示效果。随后通过类似 `FindAssetDir` 的多级搜索策略查找 `assets/images` 目录。

```cpp
        const char* testImageList[] = {
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

        std::string testImageBaseDir;
#ifdef _WIN32
        // ... 多级搜索逻辑
#endif
        if (testImageBaseDir.empty()) {
            testImageBaseDir = "assets/images/";
        }
```

#### 第六段：加载并缓存每个效果的输入纹理

**分析**：遍历所有 18 个效果，为每个效果加载对应的测试图片并创建 GPU 纹理，存入 `inputTexCache` 向量中。如果某个效果的测试图片加载失败，则回退使用默认的 `inputTex`。这种每效果独立缓存的设计使得每个卡片在封面流中可以展示与其效果最匹配的输入图片。

```cpp
        std::vector<TextureHandle> inputTexCache;
        inputTexCache.reserve(NUM_EFFECTS);

        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string testImgPath = testImageBaseDir + testImageList[i];
            // ... 加载图片并创建纹理
            TextureHandle effectInputTex = inputTex; // fallback
            if (testImgData) {
                effectInputTex = backend->CreateTexture(tiw, tih, TextureFormat::RGBA8, testImgData);
                stbi_image_free(testImgData);
            }
            inputTexCache.push_back(effectInputTex);
        }
```

#### 第七段：创建 CoverFlowScene 并配置

**分析**：创建 `CoverFlowScene` 实例，并设置其后端、输入纹理、纹理缓存、测试图片目录、应用指针等属性。然后向场景的图片池中添加 4 张内置图片以及所有 18 张测试图片，用于 Ctrl+Left/Right 切换浏览。

```cpp
        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetBackend(backend);
        coverFlow->SetInputTexture(inputTex);
        coverFlow->SetInputTexCache(inputTexCache);
        coverFlow->SetTestImageBaseDir(testImageBaseDir);
        coverFlow->SetApplication(&app);
        coverFlow->AddImageToPool(jpgPath);
        coverFlow->AddImageToPool(assetDir + "/portrait.jpg");
        coverFlow->AddImageToPool(assetDir + "/nature.jpg");
        coverFlow->AddImageToPool(assetDir + "/abstract.jpg");

        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string imgPath = testImageBaseDir + testImageList[i];
            FILE* f = fopen(imgPath.c_str(), "rb");
            if (f) { fclose(f); coverFlow->AddImageToPool(imgPath); }
        }
```

#### 第八段：添加测试视频并启用自动测试

**分析**：类似图片的处理方式，查找 `assets/videos` 目录并将 18 个测试视频文件添加到视频池中。如果启用了自动测试模式，调用 `EnableAutoTest(80)` 设置每张卡片停留 80 帧。最后将创建好的场景设置到 Application 中，并调用 `app.Run()` 启动主循环。

```cpp
        const char* testVideoList[] = { /* 18个视频文件名 */ };
        // ... 查找视频目录并添加到视频池

        if (autoTest) {
            coverFlow->EnableAutoTest(80);
        }

        app.SetScene(std::move(coverFlow));
        printf("[main] CoverFlowScene started (autoTest=%d)\n", autoTest);
    });

    return app.Run(argc, argv);
```

#### 完整源码

```cpp
// 自动测试模式：循环浏览所有18张卡片
// 设置 AUTO_TEST=1 启用，或移除以使用正常模式

#include "app/Application.h"
#include "app/CoverFlowScene.h"
#include "render/IRenderBackend.h"
#include "render/OpenGLBackend.h"
#include "stb_image.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "stb_image.h"

// 静态辅助函数：定位 assets 资源目录（见 1.1 节详解）
static std::string FindAssetDir() {
#ifdef _WIN32
    char buf[MAX_PATH];                                    // Windows 路径缓冲区
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH); // 获取可执行文件路径
    if (len > 0 && len < MAX_PATH) {
        std::string exePath(buf, (size_t)len);             // 转为字符串
        auto sl = exePath.find_last_of("\\/");              // 查找最后路径分隔符
        if (sl != std::string::npos) exePath = exePath.substr(0, sl); // 截取目录
        for (const char* rel : {"../../../assets", "../../assets", "../assets", "assets"}) {
            std::string test = exePath + "/" + rel + "/test.jpg"; // 拼接测试路径
            FILE* f = fopen(test.c_str(), "rb");            // 尝试打开
            if (f) { fclose(f); return exePath + "/" + rel; } // 找到即返回
        }
    }
#endif
    return "assets";                                        // 默认回退
}

// 程序主入口
int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);                   // 关闭 stdout 缓冲，确保日志实时输出
    setvbuf(stderr, nullptr, _IONBF, 0);                   // 关闭 stderr 缓冲

    // 检查自动测试环境变量
    const char* autoTestEnv = getenv("AUTO_TEST");           // 读取 AUTO_TEST 变量
    bool autoTest = (autoTestEnv && strcmp(autoTestEnv, "1") == 0); // 判断是否启用
    if (getenv("AUTO_TEST_DETAILS") && strcmp(getenv("AUTO_TEST_DETAILS"), "1") == 0) {
        autoTest = true;                                    // 详情页测试模式也启用
    }

    Application app;                                         // 创建应用实例

    // 设置帧回调：在场景被销毁后重建 CoverFlowScene
    app.SetFrameCallback([&](float dt) {
        (void)dt;                                           // 未使用 dt 参数
        if (app.GetCurrentScene() != nullptr) return;       // 场景已存在则返回

        IRenderBackend* backend = app.GetBackend();         // 获取渲染后端
        if (!backend) return;                               // 无后端则返回

        std::string assetDir = FindAssetDir();              // 定位资源目录
        std::string jpgPath  = assetDir + "/test.jpg";      // 默认测试图片路径
        int iw = 0, ih = 0, comp = 0;                      // 图片尺寸和通道数
        stbi_set_flip_vertically_on_load(true);             // 翻转图片以适配 OpenGL 坐标系
        stbi_uc* imgData = stbi_load(jpgPath.c_str(), &iw, &ih, &comp, 4); // 加载为 RGBA
        stbi_set_flip_vertically_on_load(false);            // 恢复默认设置
        if (!imgData) {
            fprintf(stderr, "[main] Cannot load test image: %s\n", jpgPath.c_str());
            return;
        }
        printf("[main] Loaded image: %s (%d x %d)\n", jpgPath.c_str(), iw, ih);

        TextureHandle inputTex = backend->CreateTexture(iw, ih, TextureFormat::RGBA8, imgData); // 创建 GPU 纹理
        stbi_image_free(imgData);                            // 释放 CPU 端图片数据

        // 18 个着色器效果对应的测试图片文件名
        const char* testImageList[] = {
            "00_grayscale_landscape.jpg",   // 简单测试（灰度）
            "01_bloom_citynight.jpg",       // 泛光
            "02_blur_brickwall.jpg",        // 高斯模糊
            "03_sharpen_architecture.jpg",  // 锐化
            "04_edge_building.jpg",         // 边缘检测
            "05_emboss_metal.jpg",          // 浮雕
            "06_pixelate_portrait.jpg",     // 像素化
            "07_vignette_flower.jpg",       // 暗角
            "08_chromatic_leaves.jpg",      // 色差
            "09_colorgrading_food.jpg",     // 色彩分级
            "10_noise_sky.jpg",             // 噪声生成
            "11_kaleidoscope_mandala.jpg",  // 万花筒
            "12_glitch_tech.jpg",           // 故障艺术
            "13_toon_cartoon.jpg",          // 卡通着色
            "14_vhs_retro.jpg",             // VHS 复古
            "15_crt_screen.jpg",            // CRT 显示器
            "16_water_lake.jpg",            // 水波纹
            "17_lens_wideangle.jpg"         // 镜头畸变
        };
        const int NUM_EFFECTS = (int)(sizeof(testImageList) / sizeof(testImageList[0])); // 效果总数

        std::vector<TextureHandle> inputTexCache;            // 每个效果的缓存输入纹理
        inputTexCache.reserve(NUM_EFFECTS);                  // 预分配空间

        // 查找测试图片目录（相对于可执行文件）
        std::string testImageBaseDir;
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) exePath = exePath.substr(0, sl);
            for (const char* rel : {"../../../assets/images", "../../assets/images",
                                    "../assets/images", "assets/images",
                                    "../../../screenshots/assets/images", "../../screenshots/assets/images",
                                    "../screenshots/assets/images", "screenshots/assets/images"}) {
                std::string test = exePath + "/" + rel + "/00_grayscale_landscape.jpg";
                FILE* f = fopen(test.c_str(), "rb");
                if (f) { fclose(f); testImageBaseDir = exePath + "/" + rel + "/"; break; }
            }
        }
#endif
        if (testImageBaseDir.empty()) {
            testImageBaseDir = "assets/images/";             // 默认回退路径
        }
        printf("[main] Test images directory: %s\n", testImageBaseDir.c_str());

        // 为每个效果加载并缓存输入纹理
        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string testImgPath = testImageBaseDir + testImageList[i];
            int tiw = 0, tih = 0, tcomp = 0;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* testImgData = stbi_load(testImgPath.c_str(), &tiw, &tih, &tcomp, 4);
            stbi_set_flip_vertically_on_load(false);

            TextureHandle effectInputTex = inputTex;          // 默认回退纹理
            if (testImgData) {
                printf("[main] Loaded test image for effect %d: %s (%d x %d)\n", i, testImgPath.c_str(), tiw, tih);
                effectInputTex = backend->CreateTexture(tiw, tih, TextureFormat::RGBA8, testImgData);
                stbi_image_free(testImgData);
            } else {
                printf("[main] Warning: Cannot load test image %s, using default\n", testImgPath.c_str());
            }
            inputTexCache.push_back(effectInputTex);
        }

        printf("[main] All %d input textures cached\n", (int)inputTexCache.size());

        // 创建封面流场景
        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetBackend(backend);                     // 设置渲染后端
        coverFlow->SetInputTexture(inputTex);                // 设置默认输入纹理
        coverFlow->SetInputTexCache(inputTexCache);          // 设置每效果纹理缓存
        coverFlow->SetTestImageBaseDir(testImageBaseDir);    // 设置测试图片目录
        coverFlow->SetApplication(&app);                    // 设置应用指针
        coverFlow->AddImageToPool(jpgPath);                  // 添加默认图片到池
        coverFlow->AddImageToPool(assetDir + "/portrait.jpg"); // 添加人像图片
        coverFlow->AddImageToPool(assetDir + "/nature.jpg");   // 添加自然风景图片
        coverFlow->AddImageToPool(assetDir + "/abstract.jpg");  // 添加抽象图片

        // 将测试图片添加到图片池（用于 Ctrl+Left/Right 循环切换）
        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string imgPath = testImageBaseDir + testImageList[i];
            FILE* f = fopen(imgPath.c_str(), "rb");
            if (f) {
                fclose(f);
                coverFlow->AddImageToPool(imgPath);
            }
        }

        // 添加测试视频到视频池
        const char* testVideoList[] = {
            "00_grayscale.mp4", "01_bloom.mp4", "02_blur.mp4", "03_sharpen.mp4",
            "04_edge.mp4", "05_emboss.mp4", "06_pixelate.mp4", "07_vignette.mp4",
            "08_chromatic.mp4", "09_colorgrade.mp4", "10_noise.mp4", "11_kaleidoscope.mp4",
            "12_glitch.mp4", "13_toon.mp4", "14_vhs.mp4", "15_crt.mp4",
            "16_water.mp4", "17_lens.mp4"
        };
        std::string testVideoBaseDir;
#ifdef _WIN32
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) exePath = exePath.substr(0, sl);
            for (const char* rel : {"../../../assets/videos", "../../assets/videos",
                                    "../assets/videos", "assets/videos",
                                    "../../../screenshots/assets/videos", "../../screenshots/assets/videos",
                                    "../screenshots/assets/videos", "screenshots/assets/videos"}) {
                std::string test = exePath + "/" + rel + "/00_grayscale.mp4";
                FILE* f = fopen(test.c_str(), "rb");
                if (f) { fclose(f); testVideoBaseDir = exePath + "/" + rel + "/"; break; }
            }
        }
#endif
        if (testVideoBaseDir.empty()) {
            testVideoBaseDir = "assets/videos/";
        }
        printf("[main] Test videos directory: %s\n", testVideoBaseDir.c_str());

        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string vidPath = testVideoBaseDir + testVideoList[i];
            FILE* f = fopen(vidPath.c_str(), "rb");
            if (f) {
                fclose(f);
                coverFlow->AddVideoToPool(vidPath);
            }
        }

        if (autoTest) {
            coverFlow->EnableAutoTest(80);                   // 启用自动测试，每卡停留80帧
        }

        app.SetScene(std::move(coverFlow));                  // 设置为当前场景
        printf("[main] CoverFlowScene started (autoTest=%d)\n", autoTest);
    });

    return app.Run(argc, argv);                               // 启动应用主循环
}
```

---

## 2. 应用核心 — Application

`Application` 类是整个 Shader Showcase 的核心管理器，负责窗口创建、渲染后端管理、场景切换、主循环驱动以及输入事件处理。它采用场景（Scene）系统架构，通过 `OnEnter`/`OnExit`/`OnUpdate`/`OnRender`/`OnImGui` 生命周期接口管理不同场景的切换。

### 2.1 Application.h — 类定义

`Application.h` 定义了 `Application` 类的完整接口和成员变量，同时也定义了用于自动测试截图的全局 `ScreenshotRequest` 结构体。

整个头文件可以分为以下逻辑段：

#### 第一段：ScreenshotRequest 全局截图请求结构体

**分析**：`ScreenshotRequest` 是一个轻量级的全局单例结构体，用于在自动测试流程中请求截图。它使用静态成员变量实现全局状态共享，`pending` 标志表示是否有待处理的截图请求，`path` 存储截图保存路径。`Request` 方法设置请求，`Consume` 方法以原子方式读取并清除请求标志，确保每个请求只被处理一次。

```cpp
struct ScreenshotRequest {
    static bool pending;
    static char path[256];
    static void Request(const char* p) { pending = true; snprintf(path, sizeof(path), "%s", p); }
    static bool Consume() { if (pending) { pending = false; return true; } return false; }
};
```

#### 第二段：公共接口

**分析**：`Application` 的公共接口分为几个功能组：
- **生命周期**：`Run` 是应用的主入口，`SwitchBackend` 用于运行时切换渲染后端
- **访问器**：`GetBackendType`、`GetBackend`、`GetWindow` 提供对内部状态的只读访问
- **场景管理**：`SetScene` 设置当前场景，`GetCurrentScene` 获取当前场景指针
- **帧回调**：`SetFrameCallback` 注册遗留的帧回调（为向后兼容保留）
- **拖放支持**：`ConsumeDroppedFile` 获取最近拖放到窗口的文件路径

```cpp
class Application {
public:
    Application();
    ~Application();
    int Run(int argc, char* argv[]);
    void SwitchBackend(BackendType type);
    BackendType GetBackendType() const { return m_backendType; }
    IRenderBackend* GetBackend() const { return m_backend.get(); }
    GLFWwindow* GetWindow() const { return m_window; }

    void SetScene(std::unique_ptr<Scene> scene);
    Scene* GetCurrentScene() const { return m_currentScene.get(); }

    using FrameCallback = std::function<void(float dt)>;
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

    std::string ConsumeDroppedFile();
```

#### 第三段：私有成员与回调声明

**分析**：私有成员包括：
- **窗口管理**：`m_window`（当前窗口）和 `m_retiredWindow`（退役窗口，用于避免 NVIDIA+Windows 上 Vulkan 到 OpenGL 切换时的 GL 上下文损坏问题）
- **后端管理**：`m_backend`（渲染后端智能指针）、`m_backendType`（当前后端类型）、`m_pendingBackend`/`m_pendingBackendSwitch`（延迟切换的状态）
- **场景系统**：`m_currentScene`（当前场景）和 `m_pendingNextScene`（延迟切换的下一场景）
- **UI 面板**：`m_perfPanel`（性能监控面板）
- **拖放状态**：`m_droppedFilePath`

三个静态回调函数通过 GLFW 的用户指针机制将窗口事件转发到 Application 实例。

```cpp
private:
    void InitBackend(BackendType type);
    void MainLoop();
    void Shutdown();

    GLFWwindow* m_window = nullptr;
    GLFWwindow* m_retiredWindow = nullptr;
    std::unique_ptr<IRenderBackend> m_backend;
    BackendType m_backendType = BackendType::Vulkan;
    BackendType m_pendingBackend = BackendType::Vulkan;
    bool m_pendingBackendSwitch = false;
    FrameCallback m_frameCallback;
    std::unique_ptr<Scene> m_currentScene;
    std::unique_ptr<Scene> m_pendingNextScene;
    bool m_running = false;
    std::string m_droppedFilePath;
    PerformancePanel m_perfPanel;

    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height);
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void DropCallback(GLFWwindow* w, int count, const char** paths);
};
```

#### 完整源码

```cpp
#pragma once                                                 // 防止头文件重复包含
#include "render/IRenderBackend.h"                          // 渲染后端抽象接口
#include <memory>                                            // 智能指针
#include <functional>                                        // std::function
#include <string>

struct GLFWwindow;                                           // GLFW 窗口句柄前向声明

class Scene;                                                 // 场景基类前向声明

// 全局截图请求结构体（用于自动测试）
struct ScreenshotRequest {
    static bool pending;                                     // 是否有待处理的截图请求
    static char path[256];                                   // 截图保存路径
    static void Request(const char* p) { pending = true; snprintf(path, sizeof(path), "%s", p); } // 发起截图请求
    static bool Consume() { if (pending) { pending = false; return true; } return false; } // 消费并清除请求
};

#include "ui/PerformancePanel.h"                             // 性能监控面板

class Application {
public:
    Application();                                          // 构造函数
    ~Application();                                          // 析构函数
    int Run(int argc, char* argv[]);                         // 应用主入口
    void SwitchBackend(BackendType type);                    // 切换渲染后端（OpenGL/Vulkan）
    BackendType GetBackendType() const { return m_backendType; } // 获取当前后端类型
    IRenderBackend* GetBackend() const { return m_backend.get(); } // 获取渲染后端指针
    GLFWwindow* GetWindow() const { return m_window; }       // 获取 GLFW 窗口句柄

    // 场景管理
    void SetScene(std::unique_ptr<Scene> scene);            // 设置当前场景
    Scene* GetCurrentScene() const { return m_currentScene.get(); } // 获取当前场景

    // 帧回调（遗留接口，为向后兼容保留）
    using FrameCallback = std::function<void(float dt)>;     // 帧回调类型定义
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); } // 设置帧回调

    // 拖放文件加载
    std::string ConsumeDroppedFile();                        // 获取并清除拖放文件路径

private:
    void InitBackend(BackendType type);                      // 初始化渲染后端
    void MainLoop();                                         // 主循环
    void Shutdown();                                          // 关闭清理

    GLFWwindow* m_window = nullptr;                           // 当前 GLFW 窗口
    GLFWwindow* m_retiredWindow = nullptr;                   // 退役窗口（避免 GLFW 崩溃）
    std::unique_ptr<IRenderBackend> m_backend;               // 渲染后端智能指针
    BackendType m_backendType = BackendType::Vulkan;        // 当前后端类型（默认 Vulkan）
    BackendType m_pendingBackend = BackendType::Vulkan;      // 待切换的后端类型
    bool m_pendingBackendSwitch = false;                     // 是否有待处理的后端切换
    FrameCallback m_frameCallback;                           // 帧回调函数
    std::unique_ptr<Scene> m_currentScene;                   // 当前活动场景
    std::unique_ptr<Scene> m_pendingNextScene;              // 延迟切换的下一场景
    bool m_running = false;                                  // 主循环运行标志

    std::string m_droppedFilePath;                           // 拖放文件路径

    PerformancePanel m_perfPanel;                            // 性能监控面板

    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height); // 帧缓冲大小回调
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods); // 键盘回调
    static void DropCallback(GLFWwindow* w, int count, const char** paths); // 拖放回调
};
```

### 2.2 构造与析构

构造函数和析构函数的实现非常简洁。构造函数为空（所有成员使用类内默认初始化），析构函数调用 `Shutdown()` 确保资源正确释放。

#### 第一段：构造函数

**分析**：构造函数体为空。所有成员变量已在头文件中通过类内初始化器赋予了默认值（如 `m_window = nullptr`、`m_backendType = BackendType::Vulkan` 等），因此无需在构造函数中额外初始化。这种设计遵循了"在声明处初始化"的现代 C++ 最佳实践。

```cpp
Application::Application() {}
```

#### 第二段：析构函数

**分析**：析构函数调用 `Shutdown()` 方法。`Shutdown` 负责安全地销毁场景、后端和窗口等所有资源。将清理逻辑集中在 `Shutdown` 中而非分散在析构函数里，是因为 `Shutdown` 也可以在 `Run` 的正常退出流程中被显式调用，实现资源清理逻辑的复用。

```cpp
Application::~Application()
{
    Shutdown();
}
```

#### 完整源码

```cpp
// ============================================================================
// 构造函数 / 析构函数
// ============================================================================

Application::Application() {}                               // 空构造，成员已在头文件中默认初始化

Application::~Application()
{
    Shutdown();                                              // 析构时确保资源全部释放
}
```

### 2.3 Run — 应用启动

`Run` 是 Application 的主入口函数，负责初始化 GLFW、创建渲染后端、启动主循环并在退出时清理资源。整个执行流程为：`glfwInit` -> `InitBackend` -> `MainLoop` -> `Shutdown` -> `glfwTerminate`。

#### 第一段：初始化 GLFW

**分析**：调用 `glfwInit()` 初始化 GLFW 库。如果初始化失败，输出错误信息并返回 `EXIT_FAILURE`。GLFW 是跨平台的窗口和输入管理库，必须在创建窗口之前初始化。

```cpp
    if (!glfwInit())
    {
        std::cerr << "[Application] FATAL: Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }
```

#### 第二段：初始化渲染后端

**分析**：调用 `InitBackend` 使用默认的后端类型（`m_backendType`，初始为 Vulkan）初始化渲染后端。`InitBackend` 内部会创建窗口、初始化 OpenGL/Vulkan 后端并设置 ImGui。如果后端或窗口创建失败，终止 GLFW 并返回错误码。

```cpp
    InitBackend(m_backendType);

    if (!m_window || !m_backend)
    {
        std::cerr << "[Application] FATAL: Failed to initialize backend" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
```

#### 第三段：启动主循环并清理

**分析**：设置 `m_running = true` 后进入 `MainLoop`。主循环退出后依次调用 `Shutdown`（释放所有资源）和 `glfwTerminate`（终止 GLFW 库），最后返回成功退出码。

```cpp
    m_running = true;
    MainLoop();
    Shutdown();
    glfwTerminate();

    return EXIT_SUCCESS;
```

#### 完整源码

```cpp
// ============================================================================
// Run — 应用入口
// ============================================================================

int Application::Run(int argc, char* argv[])
{
    (void)argc;                                             // 未使用命令行参数
    (void)argv;

    if (!glfwInit())                                         // 初始化 GLFW 库
    {
        std::cerr << "[Application] FATAL: Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;                                 // 初始化失败则退出
    }

    InitBackend(m_backendType);                             // 初始化渲染后端（默认 Vulkan）

    if (!m_window || !m_backend)                            // 检查窗口和后端是否创建成功
    {
        std::cerr << "[Application] FATAL: Failed to initialize backend" << std::endl;
        glfwTerminate();                                    // 终止 GLFW
        return EXIT_FAILURE;
    }

    m_running = true;                                       // 设置运行标志
    MainLoop();                                              // 进入主循环
    Shutdown();                                             // 清理所有资源
    glfwTerminate();                                        // 终止 GLFW 库

    return EXIT_SUCCESS;                                     // 正常退出
}
```

### 2.4 InitBackend — 后端初始化

`InitBackend` 是 Application 中最复杂的函数之一，负责销毁旧后端资源、创建或复用窗口、初始化新的渲染后端并设置 ImGui。它需要处理 Vulkan 到 OpenGL 切换时的窗口退役问题。

#### 第一段：销毁旧后端资源

**分析**：首先退出并销毁当前场景，然后依次调用后端的 `ImGuiShutdown` 和 `Shutdown` 释放 GPU 资源，最后重置后端智能指针。如果 ImGui 上下文仍然存在，清除字体图集以便新后端可以重建。同时重置延迟场景切换的待处理状态。

```cpp
    if (m_currentScene) {
        m_currentScene->OnExit();
        m_currentScene.reset();
    }

    if (m_backend) {
        m_backend->ImGuiShutdown();
        m_backend->Shutdown();
        m_backend.reset();
    }

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::GetIO().Fonts->Clear();
    }

    m_pendingNextScene.reset();
    BackendType oldType = m_backendType;
    m_backendType = type;
```

#### 第二段：窗口管理（处理 Vulkan 到 OpenGL 切换）

**分析**：这是 InitBackend 中最关键的部分。由于 Vulkan 和 OpenGL 共享同一个 GLFW 窗口，在 NVIDIA + Windows 平台上，从 Vulkan 切换到 OpenGL 时，GL 上下文可能已被 Vulkan 损坏。解决方案是"退役"旧窗口（隐藏但不销毁，因为销毁会导致 GLFW 崩溃），然后创建一个全新的窗口。判断条件为 `type == OpenGL && oldType == Vulkan`。

```cpp
    bool needNewWindow = (type == BackendType::OpenGL && oldType == BackendType::Vulkan);
    if (needNewWindow && m_window) {
        glfwHideWindow(m_window);
        m_retiredWindow = m_window;
        m_window = nullptr;
    }
```

#### 第三段：创建新窗口（如需要）

**分析**：如果当前没有窗口（首次初始化或退役了旧窗口），则创建新窗口。始终使用 `GLFW_OPENGL_API` 作为客户端 API 提示，因为 Vulkan 通过 Win32 原生 API 创建表面，不依赖 GLFW 的客户端 API 设置。窗口大小固定为 1280x720。创建后注册用户指针和三个回调函数。

```cpp
    if (!m_window) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

        constexpr int W = 1280, H = 720;
        m_window = glfwCreateWindow(W, H, LanguageManager::Instance().WindowTitle(), nullptr, nullptr);
        if (!m_window) {
            std::cerr << "[Application] ERROR: Failed to create window\n";
            return;
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
        glfwSetKeyCallback(m_window, KeyCallback);
        glfwSetDropCallback(m_window, DropCallback);
    }
```

#### 第四段：创建并初始化后端

**分析**：将 GL 上下文设为当前（两个后端共享窗口），如果是 OpenGL 后端则启用垂直同步。然后根据后端类型创建对应的 `OpenGLBackend` 或 `VulkanBackend` 实例，调用 `Init` 进行初始化，再调用 `ImGuiInit` 初始化 ImGui 集成。最后更新窗口标题以显示当前后端名称。

```cpp
    glfwMakeContextCurrent(m_window);
    if (type == BackendType::OpenGL) {
        glfwSwapInterval(1);
    }

    switch (type) {
    case BackendType::OpenGL:
        m_backend = std::make_unique<OpenGLBackend>();
        break;
    case BackendType::Vulkan:
        m_backend = std::make_unique<VulkanBackend>();
        break;
    }

    if (!m_backend->Init(m_window)) {
        m_backend.reset();
        return;
    }

    m_backend->ImGuiInit(m_window);

    std::string title = std::string(LanguageManager::Instance().WindowTitle()) + " [" + m_backend->GetName() + "]";
    glfwSetWindowTitle(m_window, title.c_str());
```

#### 完整源码

```cpp
// ============================================================================
// InitBackend — 销毁旧后端，创建新后端
// ============================================================================

void Application::InitBackend(BackendType type)
{
    // ---- 销毁旧后端资源（但不销毁窗口）----
    if (m_currentScene) {
        m_currentScene->OnExit();                            // 退出当前场景
        m_currentScene.reset();                              // 销毁场景对象
    }

    if (m_backend) {
        m_backend->ImGuiShutdown();                          // 释放 ImGui GPU 资源
        m_backend->Shutdown();                               // 释放后端资源
        m_backend.reset();                                   // 重置智能指针
    }

    // 不销毁 ImGui 上下文，只清除字体图集以便新后端重建
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::GetIO().Fonts->Clear();
    }

    m_pendingNextScene.reset();                              // 清除待处理的场景切换
    BackendType oldType = m_backendType;                     // 记录旧后端类型
    m_backendType = type;                                    // 更新为新的后端类型

    // ---- 创建或复用窗口 ----
    // 始终使用 GLFW_OPENGL_API — Vulkan 通过 Win32 原生 API 创建表面
    // 从 Vulkan 切换到 OpenGL 时，GL 上下文可能已被损坏，需要创建新窗口
    bool needNewWindow = (type == BackendType::OpenGL && oldType == BackendType::Vulkan);
    if (needNewWindow && m_window) {
        glfwHideWindow(m_window);                             // 隐藏旧窗口
        m_retiredWindow = m_window;                           // 保存为退役窗口（不销毁）
        m_window = nullptr;                                   // 清空当前窗口指针
    }

    if (!m_window) {                                         // 如果没有窗口则创建新窗口
        glfwDefaultWindowHints();                            // 恢复默认窗口提示
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);    // 使用 OpenGL API
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);       // OpenGL 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // 前向兼容

        constexpr int W = 1280, H = 720;                    // 窗口尺寸
        m_window = glfwCreateWindow(W, H, LanguageManager::Instance().WindowTitle(), nullptr, nullptr);
        if (!m_window) {
            std::cerr << "[Application] ERROR: Failed to create window\n";
            return;
        }

        glfwSetWindowUserPointer(m_window, this);           // 设置用户指针（回调中使用）
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback); // 注册帧缓冲大小回调
        glfwSetKeyCallback(m_window, KeyCallback);           // 注册键盘回调
        glfwSetDropCallback(m_window, DropCallback);         // 注册拖放回调
    }

    // ---- 将 GL 上下文设为当前（两个后端共享窗口）----
    glfwMakeContextCurrent(m_window);
    if (type == BackendType::OpenGL) {
        glfwSwapInterval(1);                                 // OpenGL 启用垂直同步
    }

    // ---- 创建后端实例 ----
    switch (type) {
    case BackendType::OpenGL:
        m_backend = std::make_unique<OpenGLBackend>();       // 创建 OpenGL 后端
        break;
    case BackendType::Vulkan:
        m_backend = std::make_unique<VulkanBackend>();       // 创建 Vulkan 后端
        break;
    }

    if (!m_backend) {
        std::cerr << "[Application] ERROR: Backend not available\n";
        return;
    }

    if (!m_backend->Init(m_window)) {                        // 初始化后端
        std::cerr << "[Application] ERROR: Backend initialization failed\n";
        m_backend.reset();
        return;
    }

    m_backend->ImGuiInit(m_window);                          // 初始化 ImGui 集成

    // 更新窗口标题，附加后端名称
    std::string title = std::string(LanguageManager::Instance().WindowTitle()) + " [" + m_backend->GetName() + "]";
    glfwSetWindowTitle(m_window, title.c_str());

    std::cout << "[Application] Backend initialized: " << m_backend->GetName() << std::endl;
}
```

### 2.5 MainLoop — 主循环

`MainLoop` 是帧驱动的游戏循环，每帧依次处理事件、计算时间增量、驱动场景更新和渲染、处理场景切换、渲染性能面板和 ImGui，最后处理截图请求。

#### 第一段：循环条件与事件轮询

**分析**：主循环在 `m_running` 为 true、窗口存在且未请求关闭时持续运行。每帧首先调用 `glfwPollEvents()` 处理所有待处理的窗口事件。整个循环体包裹在 try-catch 中，确保异常不会导致程序无提示崩溃。

```cpp
    while (m_running && m_window && !glfwWindowShouldClose(m_window))
    {
        try {
            glfwPollEvents();
```

#### 第二段：延迟后端切换处理

**分析**：检查 `m_pendingBackendSwitch` 标志，如果存在待处理的后端切换请求，在帧开始时执行切换。将切换延迟到帧开始而非在回调中立即执行，是为了避免在 ImGui/Vulkan 渲染过程中切换后端导致的状态损坏。

```cpp
        if (m_pendingBackendSwitch) {
            m_pendingBackendSwitch = false;
            printf("[Application] Processing deferred backend switch...\n"); fflush(stdout);
            InitBackend(m_pendingBackend);
        }
```

#### 第三段：计算帧时间增量

**分析**：使用 `std::chrono::high_resolution_clock` 获取高精度时间戳，计算当前帧与上一帧之间的时间差 `dt`（单位为秒）。这个 `dt` 值将传递给场景的 `OnUpdate` 方法，用于驱动基于时间的动画和物理模拟。

```cpp
        auto now      = std::chrono::high_resolution_clock::now();
        float dt      = std::chrono::duration<float>(now - lastTime).count();
        lastTime      = now;
```

#### 第四段：帧开始与延迟场景切换

**分析**：调用 `m_backend->BeginFrame()` 开始新的渲染帧（对于 Vulkan 后端，这会等待 GPU 完成上一帧的工作）。然后检查是否有延迟的场景切换请求。场景切换必须在 `BeginFrame` 之后执行，因为 `vkWaitForFences` 确保 GPU 已完成对旧场景资源的访问，此时销毁旧场景的 GPU 资源才是安全的。

```cpp
        m_backend->BeginFrame();

        if (m_pendingNextScene) {
            if (m_currentScene) {
                m_currentScene->OnExit();
                m_currentScene.reset();
            }
            m_currentScene = std::move(m_pendingNextScene);
            if (m_currentScene) {
                m_currentScene->OnEnter();
            }
        }
```

#### 第五段：场景驱动更新与渲染

**分析**：如果存在当前场景，依次调用 `OnUpdate(dt)`（逻辑更新）、`OnRender(m_backend.get())`（渲染）和 `OnImGui()`（ImGui UI 绘制）。然后检查场景是否请求退出（`WantsExit`），如果是则获取下一场景并延迟到下一帧处理，或者如果没有下一场景则关闭应用。

```cpp
        m_backend->ImGuiNewFrame();

        if (m_currentScene)
        {
            m_currentScene->OnUpdate(dt);
            m_currentScene->OnRender(m_backend.get());
            m_currentScene->OnImGui();

            if (m_currentScene->WantsExit())
            {
                auto nextScene = m_currentScene->GetNextScene();
                if (nextScene)
                {
                    m_pendingNextScene = std::move(nextScene);
                }
                else
                {
                    m_running = false;
                }
            }
        }
        else if (m_frameCallback)
        {
            m_frameCallback(dt);
        }
```

#### 第六段：性能面板、ImGui 渲染与截图

**分析**：始终渲染性能面板（即使没有场景也可见），然后调用 `ImGuiRender` 提交 ImGui 绘制数据。之后检查全局截图请求，如果存在则通过 OpenGL 后端保存截图。还有一个自动 UI 截图功能，在 `AUTO_TEST_UI` 环境变量设置时，在第 3 帧自动保存 UI 截图。

```cpp
        m_perfPanel.Render(this, m_backend.get());

        m_backend->ImGuiRender();

        if (ScreenshotRequest::Consume()) {
            if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                gl->SaveScreenshot(ScreenshotRequest::path);
            }
        }

        {
            static int screenshotFrame = -1;
            if (screenshotFrame == -1 && getenv("AUTO_TEST_UI")) {
                screenshotFrame = 0;
            }
            if (screenshotFrame >= 0 && screenshotFrame < 5) {
                screenshotFrame++;
                if (screenshotFrame == 3) {
                    if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                        gl->SaveScreenshot("e:/AI/graph/hight-post-proc/ui_screenshot.ppm");
                    }
                }
            }
        }

        m_backend->EndFrame();
```

#### 第七段：异常处理与循环退出

**分析**：catch 块捕获主循环中的所有异常，输出错误信息并设置 `m_running = false` 终止循环。循环退出后输出状态日志并确保 `m_running` 被设为 false。

```cpp
        } catch (const std::exception& e) {
            fprintf(stderr, "[Application] EXCEPTION in main loop: %s\n", e.what());
            m_running = false;
        } catch (...) {
            fprintf(stderr, "[Application] UNKNOWN EXCEPTION in main loop\n");
            m_running = false;
        }
    }

    printf("[Application] Main loop exited: m_running=%d, m_window=%p, shouldClose=%d\n",
           (int)m_running, (void*)m_window,
           m_window ? glfwWindowShouldClose(m_window) : -1);

    m_running = false;
```

#### 完整源码

```cpp
// ============================================================================
// MainLoop — 帧驱动的主循环
// ============================================================================

void Application::MainLoop()
{
    auto lastTime = std::chrono::high_resolution_clock::now(); // 记录初始时间戳

    while (m_running && m_window && !glfwWindowShouldClose(m_window)) // 主循环条件
    {
        try {
            glfwPollEvents();                                // 轮询并处理所有窗口事件

            // 处理延迟的后端切换（避免在 ImGui 渲染过程中切换导致状态损坏）
            if (m_pendingBackendSwitch) {
                m_pendingBackendSwitch = false;             // 清除标志
                printf("[Application] Processing deferred backend switch...\n"); fflush(stdout);
                InitBackend(m_pendingBackend);               // 执行后端切换
            }

            // 计算帧时间增量
            auto now      = std::chrono::high_resolution_clock::now();
            float dt      = std::chrono::duration<float>(now - lastTime).count(); // 秒为单位
            lastTime      = now;                              // 更新上一帧时间

            if (!m_backend)
            {
                continue;                                    // 无后端则跳过本帧
            }

            m_backend->BeginFrame();                          // 开始新渲染帧（Vulkan 等待 GPU）

            // --- 延迟场景切换 ---
            // 必须在 BeginFrame 之后执行，确保 GPU 已完成对旧场景资源的访问
            if (m_pendingNextScene) {
                printf("[Application] Processing deferred scene transition\n");
                if (m_currentScene) {
                    m_currentScene->OnExit();                 // 退出旧场景
                    m_currentScene.reset();                   // 销毁旧场景
                }
                m_currentScene = std::move(m_pendingNextScene); // 接管新场景
                if (m_currentScene) {
                    m_currentScene->OnEnter();               // 进入新场景
                    printf("[Application] New scene entered OK\n");
                }
            }

            m_backend->ImGuiNewFrame();                       // 开始 ImGui 新帧

            // --- 场景驱动的更新 ---
            if (m_currentScene)
            {
                m_currentScene->OnUpdate(dt);                 // 场景逻辑更新
                m_currentScene->OnRender(m_backend.get());    // 场景渲染
                m_currentScene->OnImGui();                    // 场景 ImGui UI

                // 检查场景是否请求退出
                if (m_currentScene->WantsExit())
                {
                    auto nextScene = m_currentScene->GetNextScene(); // 获取下一场景
                    if (nextScene)
                    {
                        printf("[Application] Deferring scene transition to next frame\n");
                        m_pendingNextScene = std::move(nextScene); // 延迟到下一帧
                    }
                    else
                    {
                        // 无下一场景 — 退出应用
                        printf("[Application] Scene requested exit with no replacement; shutting down\n");
                        m_running = false;
                    }
                }
            }
            else if (m_frameCallback)
            {
                m_frameCallback(dt);                         // 遗留帧回调后备方案
            }

            // 始终渲染性能面板（即使没有场景也可见）
            m_perfPanel.Render(this, m_backend.get());

            m_backend->ImGuiRender();                        // 提交 ImGui 绘制数据

            // 检查截图请求（来自自动测试）
            if (ScreenshotRequest::Consume()) {
                printf("[Application] Processing screenshot request: %s\n", ScreenshotRequest::path);
                if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                    gl->SaveScreenshot(ScreenshotRequest::path); // 保存截图
                    printf("[Application] Screenshot saved: %s\n", ScreenshotRequest::path);
                } else {
                    printf("[Application] Failed to get OpenGL backend for screenshot\n");
                }
            }

            // 自动 UI 截图（用于文档展示）
            {
                static int screenshotFrame = -1;
                if (screenshotFrame == -1 && getenv("AUTO_TEST_UI")) {
                    screenshotFrame = 0;                    // 检测到环境变量则开始计数
                }
                if (screenshotFrame >= 0 && screenshotFrame < 5) {
                    screenshotFrame++;
                    if (screenshotFrame == 3) {              // 在第 3 帧截图
                        if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                            gl->SaveScreenshot("e:/AI/graph/hight-post-proc/ui_screenshot.ppm");
                            printf("[Application] UI screenshot saved at frame %d\n", screenshotFrame);
                        }
                    }
                }
            }

            m_backend->EndFrame();                           // 结束渲染帧（交换缓冲区/提交命令）
        } catch (const std::exception& e) {
            fprintf(stderr, "[Application] EXCEPTION in main loop: %s\n", e.what());
            m_running = false;                               // 异常时退出循环
        } catch (...) {
            fprintf(stderr, "[Application] UNKNOWN EXCEPTION in main loop\n");
            m_running = false;
        }
    }

    printf("[Application] Main loop exited: m_running=%d, m_window=%p, shouldClose=%d\n",
           (int)m_running, (void*)m_window,
           m_window ? glfwWindowShouldClose(m_window) : -1); // 输出退出状态

    m_running = false;                                       // 确保运行标志已清除
}
```

### 2.6 Shutdown — 资源清理

`Shutdown` 负责按正确顺序释放所有资源：先退出并销毁场景，再关闭后端，最后销毁窗口。注意退役窗口也会被销毁。

#### 第一段：停止运行并销毁场景

**分析**：首先将 `m_running` 设为 false 以通知主循环退出。然后检查并退出当前场景（调用 `OnExit` 让场景执行清理逻辑），最后重置场景智能指针。

```cpp
    m_running = false;

    if (m_currentScene)
    {
        m_currentScene->OnExit();
        m_currentScene.reset();
    }
```

#### 第二段：关闭后端与销毁窗口

**分析**：依次调用后端的 `ImGuiShutdown` 和 `Shutdown` 释放 GPU 资源，然后重置后端智能指针。接着销毁当前窗口和退役窗口（如果存在），最后调用 `glfwTerminate` 终止 GLFW 库。

```cpp
    if (m_backend)
    {
        m_backend->ImGuiShutdown();
        m_backend->Shutdown();
        m_backend.reset();
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_retiredWindow)
    {
        glfwDestroyWindow(m_retiredWindow);
        m_retiredWindow = nullptr;
    }
    glfwTerminate();
```

#### 完整源码

```cpp
// ============================================================================
// Shutdown — 释放所有资源
// ============================================================================

void Application::Shutdown()
{
    m_running = false;                                       // 停止主循环

    // 退出并销毁当前场景
    if (m_currentScene)
    {
        m_currentScene->OnExit();                            // 让场景执行清理
        m_currentScene.reset();                              // 销毁场景对象
    }

    // 关闭并销毁渲染后端
    if (m_backend)
    {
        m_backend->ImGuiShutdown();                          // 释放 ImGui GPU 资源
        m_backend->Shutdown();                               // 释放后端资源
        m_backend.reset();                                   // 销毁后端对象
    }

    // 销毁当前窗口
    if (m_window)
    {
        glfwDestroyWindow(m_window);                         // 销毁 GLFW 窗口
        m_window = nullptr;
    }
    // 销毁退役窗口（Vulkan 到 OpenGL 切换时保留的旧窗口）
    if (m_retiredWindow)
    {
        glfwDestroyWindow(m_retiredWindow);                   // 销毁退役窗口
        m_retiredWindow = nullptr;
    }
    glfwTerminate();                                         // 终止 GLFW 库
}
```

### 2.7 SetScene / SwitchBackend

这两个函数分别负责场景切换和渲染后端切换。

#### SetScene — 场景切换

**分析**：`SetScene` 实现即时的场景切换。首先检查是否存在旧场景，如果有则调用 `OnExit` 退出并销毁。然后将新场景移入 `m_currentScene`，如果新场景有效则调用 `OnEnter` 进入。每次操作都输出日志并刷新 stdout，便于调试场景切换问题。

```cpp
void Application::SetScene(std::unique_ptr<Scene> scene)
{
    if (m_currentScene)
    {
        printf("[Application] SetScene: exiting old scene\n");
        fflush(stdout);
        m_currentScene->OnExit();
        m_currentScene.reset();
    }
    m_currentScene = std::move(scene);
    if (m_currentScene)
    {
        printf("[Application] SetScene: entering new scene\n");
        fflush(stdout);
        m_currentScene->OnEnter();
        printf("[Application] SetScene: new scene entered OK\n");
        fflush(stdout);
    }
}
```

#### SwitchBackend — 延迟后端切换

**分析**：`SwitchBackend` 不立即执行后端切换，而是将切换请求延迟到下一帧开始时处理。这是因为在 ImGui 渲染过程中切换后端会导致 Vulkan/ImGui 状态损坏。函数首先检查是否已经在目标后端上，如果是则直接返回。否则设置 `m_pendingBackend` 和 `m_pendingBackendSwitch` 标志，由 `MainLoop` 在下一帧处理。

```cpp
void Application::SwitchBackend(BackendType type)
{
    if (type == m_backendType && m_backend)
    {
        return;
    }
    m_pendingBackend = type;
    m_pendingBackendSwitch = true;
    printf("[Application] Backend switch to %s scheduled for next frame\n",
           type == BackendType::OpenGL ? "OpenGL" : "Vulkan");
}
```

#### 完整源码

```cpp
// ============================================================================
// SetScene — 设置当前场景
// ============================================================================

void Application::SetScene(std::unique_ptr<Scene> scene)
{
    if (m_currentScene)                                     // 如果存在旧场景
    {
        printf("[Application] SetScene: exiting old scene\n");
        fflush(stdout);                                      // 确保日志立即输出
        m_currentScene->OnExit();                            // 退出旧场景
        m_currentScene.reset();                              // 销毁旧场景
    }
    m_currentScene = std::move(scene);                       // 接管新场景
    if (m_currentScene)                                       // 如果新场景有效
    {
        printf("[Application] SetScene: entering new scene\n");
        fflush(stdout);
        m_currentScene->OnEnter();                           // 进入新场景
        printf("[Application] SetScene: new scene entered OK\n");
        fflush(stdout);
    }
}

// ============================================================================
// SwitchBackend — 切换到不同的渲染后端
// ============================================================================

void Application::SwitchBackend(BackendType type)
{
    if (type == m_backendType && m_backend)                   // 如果已在目标后端上
    {
        return;                                              // 无需切换
    }
    // 延迟到下一帧开始时执行，避免 ImGui/Vulkan 状态损坏
    m_pendingBackend = type;                                // 记录目标后端类型
    m_pendingBackendSwitch = true;                           // 设置延迟切换标志
    printf("[Application] Backend switch to %s scheduled for next frame\n",
           type == BackendType::OpenGL ? "OpenGL" : "Vulkan");
}
```

### 2.8 回调函数

Application 定义了三个 GLFW 静态回调函数，通过 `glfwSetWindowUserPointer` 机制将事件转发到 Application 实例。

#### FramebufferSizeCallback — 帧缓冲大小回调

**分析**：当窗口大小改变时，GLFW 调用此回调。函数通过 `glfwGetWindowUserPointer` 获取 Application 实例指针，然后调用后端的 `Resize` 方法通知渲染后端更新视口和帧缓冲大小。

```cpp
void Application::FramebufferSizeCallback(GLFWwindow* w, int width, int height)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (app && app->m_backend)
    {
        app->m_backend->Resize(width, height);
    }
}
```

#### KeyCallback — 键盘回调

**分析**：只处理按键按下事件（`GLFW_PRESS`）。全局快捷键包括：
- `Ctrl+Q`：退出应用（设置窗口关闭标志）
- `Ctrl+1`：切换到 OpenGL 后端
- `Ctrl+2`：切换到 Vulkan 后端
- ESC 键不再由全局回调处理，而是交给场景系统

```cpp
void Application::KeyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(w, GLFW_TRUE);
        return;
    }

    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_1) {
        app->SwitchBackend(BackendType::OpenGL);
        return;
    }

    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_2) {
        app->SwitchBackend(BackendType::Vulkan);
        return;
    }
}
```

#### DropCallback 与 ConsumeDroppedFile

**分析**：`DropCallback` 在用户将文件拖放到窗口时被调用，只接受第一个文件路径。`ConsumeDroppedFile` 使用 `std::swap` 原子地获取并清除拖放文件路径，确保每个拖放文件只被处理一次。

```cpp
void Application::DropCallback(GLFWwindow* w, int count, const char** paths)
{
    if (count < 1) return;
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    app->m_droppedFilePath = paths[0];
    printf("[Application] File dropped: %s\n", paths[0]);
}

std::string Application::ConsumeDroppedFile()
{
    std::string path;
    std::swap(path, m_droppedFilePath);
    return path;
}
```

#### 完整源码

```cpp
// ============================================================================
// 静态回调函数
// ============================================================================

// 帧缓冲大小变化回调
void Application::FramebufferSizeCallback(GLFWwindow* w, int width, int height)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w)); // 获取 Application 实例
    if (app && app->m_backend)
    {
        app->m_backend->Resize(width, height);               // 通知后端调整大小
    }
}

// 键盘按键回调
void Application::KeyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS)                                // 只处理按下事件
    {
        return;
    }

    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w)); // 获取 Application 实例
    if (!app)
    {
        return;
    }

    // ESC 由场景系统处理（CoverFlowScene 退出，EffectDetailScene 返回）
    // 全局覆盖：Ctrl+Q 始终退出
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(w, GLFW_TRUE);              // 设置窗口关闭标志
        return;
    }

    // Ctrl+1 — 切换到 OpenGL 后端
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_1)
    {
        app->SwitchBackend(BackendType::OpenGL);
        return;
    }

    // Ctrl+2 — 切换到 Vulkan 后端
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_2)
    {
        app->SwitchBackend(BackendType::Vulkan);
        return;
    }
}

// 文件拖放回调
void Application::DropCallback(GLFWwindow* w, int count, const char** paths)
{
    if (count < 1) return;                                    // 无文件则返回
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    app->m_droppedFilePath = paths[0];                       // 只接受第一个拖放文件
    printf("[Application] File dropped: %s\n", paths[0]);
}

// 消费拖放文件路径（获取并清除）
std::string Application::ConsumeDroppedFile()
{
    std::string path;
    std::swap(path, m_droppedFilePath);                      // 原子交换，确保只处理一次
    return path;
}
```

---

## 3. CoverFlowScene — 封面流场景

`CoverFlowScene` 是 Shader Showcase 的主界面场景，以类似 Apple Cover Flow 的 3D 卡片流形式展示所有 18 个着色器效果。用户可以通过键盘、鼠标滚轮、拖拽等方式浏览卡片，点击卡片进入效果详情页。该场景还支持实时缩略图渲染、屏幕捕获、视频播放、图片切换和自动测试等功能。

### 3.1 类定义

`CoverFlowScene` 继承自 `Scene` 基类，实现了完整的场景生命周期接口。头文件中还定义了 `CardThumbnailState` 结构体，用于管理每个卡片的实时缩略图渲染状态。

#### 第一段：CardThumbnailState 结构体

**分析**：该结构体为每个效果卡片维护独立的缩略图渲染状态。`fragShader` 是该效果对应的片段着色器句柄，`thumbTex` 是 256x144 大小的缩略图 FBO 纹理。每张卡片都有独立的着色器和纹理，使得封面流中每张卡片都能实时展示对应效果的处理结果。

```cpp
struct CardThumbnailState {
    ShaderHandle fragShader;   // 每张卡片的效果着色器
    TextureHandle thumbTex;    // 256x144 缩略图 FBO 纹理
};
```

#### 第二段：公共接口

**分析**：公共接口包括：
- **生命周期**：`OnEnter`/`OnExit`/`OnUpdate`/`OnRender`/`OnImGui`/`WantsExit`/`GetNextScene`（Scene 基类虚函数）
- **配置方法**：`SetInputTexture`、`SetInputTexCache`、`SetBackend`、`SetApplication`、`SetThumbnails`、`SetSelectedIndex`、`SetTestImageBaseDir` 等
- **视频管理**：`SetVideoPlayer` 用于从详情页接收视频播放器
- **自动测试**：`EnableAutoTest` 和 `ResumeAutoTest` 控制自动测试流程
- **输入管理**：`ReloadInputTexture`（拖放重载）、`AddImageToPool`、`AddVideoToPool`
- **状态保存**：`GetState` 返回当前状态用于详情页返回时恢复

#### 第三段：私有成员变量

**分析**：私有成员按功能分组：
- **卡片与选择**：`m_cards`（效果卡片列表）、`m_selectedIndex`（当前选中索引）、`m_scrollOffset`/`m_targetOffset`（滚动偏移）
- **纹理与后端**：`m_inputTex`（输入纹理）、`m_inputTexCache`（每效果纹理缓存）、`m_backend`（渲染后端指针）
- **自动测试**：`m_autoTest`、`m_autoTestHoldFrames`、`m_autoTestFrameCounter`、`m_autoTestCardIndex` 等
- **缩略图渲染**：`m_sharedVertShader`（共享顶点着色器）、`m_thumbnailStates`（每卡缩略图状态）、`m_thumbInitialized` 等
- **鼠标拖拽**：`m_dragging`、`m_dragStartX`、`m_dragBaseOff`
- **FPS 计数**：`m_fpsLastTime`、`m_fpsFrameCount`、`m_fpsDisplay`
- **屏幕捕获**：`m_screenCapture`、`m_captureTex`、`m_captureActive` 等
- **图片/视频池**：`m_imagePool`/`m_videoPool` 及对应索引
- **视频播放器**：`m_videoPlayer`、`m_videoTex`、`m_videoActive`

#### 完整源码

```cpp
#pragma once                                                 // 防止头文件重复包含

#include "app/Scene.h"                                       // 场景基类
#include "app/CoverFlowState.h"                              // 封面流状态结构体
#include "shader/EffectMetadata.h"                            // 效果元数据
#include "render/IRenderBackend.h"                           // 渲染后端接口

#include <vector>
#include <memory>
#include <string>
#include <chrono>

class Application;                                           // 前向声明
class ScreenCapture;                                         // 前向声明
class VideoPlayer;                                           // 前向声明

/// 每张卡片的缩略图实时渲染状态
struct CardThumbnailState {
    ShaderHandle fragShader;                                  // 效果片段着色器句柄
    TextureHandle thumbTex;                                   // 256x144 缩略图 FBO 纹理
};

class CoverFlowScene : public Scene {
public:
    CoverFlowScene();                                         // 构造函数（调用 RegisterCards）
    ~CoverFlowScene() override;                               // 析构函数（释放着色器和纹理）

    void OnEnter() override;                                  // 场景进入
    void OnExit() override;                                   // 场景退出
    void OnUpdate(float dt) override;                        // 每帧更新
    void OnRender(IRenderBackend* backend) override;         // 渲染
    void OnImGui() override;                                  // ImGui UI 绘制
    bool WantsExit() const override { return m_wantsExit; } // 是否请求退出

    std::unique_ptr<Scene> GetNextScene() override;          // 获取下一场景
    void SetInputTexture(TextureHandle tex) { m_inputTex = tex; } // 设置输入纹理
    void SetInputTexCache(const std::vector<TextureHandle>& cache) { m_inputTexCache = cache; }
    void SetBackend(IRenderBackend* backend) { m_backend = backend; } // 设置渲染后端
    void SetApplication(Application* app) { m_app = app; }    // 设置应用指针

    /// 设置预渲染缩略图 ImGui 纹理 ID
    void SetThumbnails(const std::vector<void*>& imTexIds) { m_thumbIds = imTexIds; }

    /// 恢复选中卡片索引和滚动偏移（从详情页返回时使用）
    void SetSelectedIndex(int index) { m_selectedIndex = index; m_targetOffset = 0.0f; m_scrollOffset = 0.0f; }

    /// 设置测试图片目录
    void SetTestImageBaseDir(const std::string& dir) { m_testImageBaseDir = dir; }

    /// 从详情页接收视频播放器
    void SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime);

    /// 启用自动测试模式
    void EnableAutoTest(int holdFrames);

    /// 从详情页返回后恢复自动测试
    void ResumeAutoTest(int holdFrames, int lastOpenedCard);

    /// 重新加载输入纹理（拖放支持）
    void ReloadInputTexture(const std::string& filePath);

    /// 添加图片到内置图片池
    void AddImageToPool(const std::string& path);

    /// 添加视频到内置视频池
    void AddVideoToPool(const std::string& path);

    /// 获取当前状态（用于详情页返回时恢复）
    CoverFlowState GetState() const;

private:
    std::vector<EffectCard> m_cards;                         // 效果卡片列表
    int m_selectedIndex   = 0;                               // 当前选中索引
    float m_scrollOffset  = 0.0f;                            // 滚动偏移（动画中间值）
    float m_targetOffset  = 0.0f;                            // 目标偏移（动画目标值）
    TextureHandle m_inputTex = {0};                           // 当前输入纹理
    std::vector<TextureHandle> m_inputTexCache;              // 每效果纹理缓存
    IRenderBackend* m_backend = nullptr;                     // 渲染后端指针
    Application* m_app = nullptr;                             // 应用指针
    bool m_wantsExit = false;                                // 是否请求退出
    std::unique_ptr<Scene> m_nextScene;                     // 下一场景（详情页）

    // 自动测试模式
    bool m_autoTest = false;
    int  m_autoTestHoldFrames = 0;
    int  m_autoTestFrameCounter = 0;
    int  m_autoTestCardIndex = 0;
    int  m_autoTestLastOpenedCard = -1;

    // 缩略图 ImGui 纹理 ID
    std::vector<void*> m_thumbIds;

    // 动态缩略图渲染
    ShaderHandle m_sharedVertShader = INVALID_SHADER;        // 共享顶点着色器
    std::vector<CardThumbnailState> m_thumbnailStates;       // 每卡缩略图状态
    int m_thumbWidth  = 256;                                 // 缩略图宽度
    int m_thumbHeight = 144;                                 // 缩略图高度
    bool m_thumbInitialized = false;                          // 是否已初始化
    std::string m_testImageBaseDir;                          // 测试图片目录
    float m_thumbElapsedTime = 0.0f;                          // 缩略图累计时间
    uint32_t m_thumbFrameCount = 0;                          // 缩略图帧计数

    // 鼠标拖拽
    bool  m_dragging    = false;
    float m_dragStartX  = 0.0f;
    float m_dragBaseOff = 0.0f;

    // FPS 计数器
    std::chrono::high_resolution_clock::time_point m_fpsLastTime;
    int    m_fpsFrameCount = 0;
    float  m_fpsDisplay    = 0.0f;

    // 屏幕捕获
    std::unique_ptr<ScreenCapture> m_screenCapture;
    TextureHandle m_captureTex = {0};
    bool m_captureActive  = false;
    bool m_captureReady   = false;
    int  m_captureWidth   = 0;
    int  m_captureHeight  = 0;

    // 图片循环
    std::vector<std::string> m_imagePool;                    // 图片路径列表
    int m_currentImageIndex = 0;                             // 当前图片索引

    // 视频循环
    std::vector<std::string> m_videoPool;                    // 视频路径列表
    int m_currentVideoIndex = 0;                             // 当前视频索引

    // 视频播放器
    std::unique_ptr<VideoPlayer> m_videoPlayer;
    TextureHandle m_videoTex = {0};
    bool m_videoActive = false;
    double m_videoLastFrameTime = 0.0;

    void RegisterCards();                                     // 注册效果卡片
    void SelectCard(int index);                                // 选择卡片
    void OpenSelectedEffect();                                // 打开效果详情
    void UpdateFPSCounter();                                  // 更新 FPS
    void ToggleScreenCapture();                               // 切换屏幕捕获
    void CycleImage(int direction);                           // 切换图片
    void LoadImageFromFile(const std::string& path);         // 加载图片
    void OpenVideoFile(const std::string& path);              // 打开视频
    void StopVideo();                                         // 停止视频
    void InitializeThumbnails();                             // 初始化缩略图
    void RenderVisibleThumbnails();                           // 渲染可见缩略图
};
```

### 3.2 RegisterCards — 卡片注册

`RegisterCards` 在构造函数中被调用，负责注册所有 18 个着色器效果卡片。每个卡片包含 ID、名称、分类、描述以及着色器 SPIR-V 文件路径。

#### 第一段：确定着色器目录和顶点着色器路径

**分析**：通过 `ShaderLoader::FindShaderDir()` 定位着色器目录。OpenGL 后端使用 VAO 顶点输入的顶点着色器（`fullscreen.vert.spv`），Vulkan 后端使用 `VertexIndex` 生成的三角形的顶点着色器（`fullscreen_vk.vert.spv`）。

```cpp
    std::string shaderDir = ShaderLoader::FindShaderDir();
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv";
    if (m_backend && m_backend->GetType() == BackendType::Vulkan) {
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv";
    }
```

#### 第二段：定义 lambda 和注册卡片

**分析**：使用 `CARD` 宏简化卡片注册语法。宏展开后调用内部的 `add` lambda，该 lambda 创建 `EffectCard` 结构体并填充所有字段，然后加入 `m_cards` 向量。18 个效果涵盖简单测试、光照、滤镜、风格化、色彩、扭曲、程序化生成和复古等 8 个分类。

```cpp
    m_cards.clear();
    m_cards.reserve(18);

    auto add = [&](const char* id, const char* name, const char* category,
                   const char* desc, const char* fragRelPath) {
        EffectCard c;
        c.id = id; c.name = name; c.category = category; c.description = desc;
        c.vertSpirvPath = vertPath;
        c.fragSpirvPath = shaderDir + "/" + fragRelPath;
        c.passes = 1;
        m_cards.push_back(std::move(c));
    };

    CARD("simple_test",  "Grayscale Test",    "Simple",
         "Basic grayscale shader, validates render pipeline",
         "effects/simple_test/simple_test.frag.spv");
    // ... 其余 17 个卡片
```

#### 完整源码

```cpp
#define CARD(id, name, cat, desc, frag) \
    add(id, name, cat, desc, frag)                            // 简化卡片注册的宏

void CoverFlowScene::RegisterCards()
{
    std::string shaderDir = ShaderLoader::FindShaderDir();   // 定位着色器目录
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv";
    if (m_backend && m_backend->GetType() == BackendType::Vulkan) {
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv"; // Vulkan 专用顶点着色器
    }

    m_cards.clear();                                         // 清空卡片列表
    m_cards.reserve(18);                                     // 预分配 18 个卡片空间

    auto add = [&](const char* id, const char* name, const char* category,
                   const char* desc, const char* fragRelPath) {
        EffectCard c;
        c.id = id; c.name = name; c.category = category; c.description = desc;
        c.vertSpirvPath = vertPath;
        c.fragSpirvPath = shaderDir + "/" + fragRelPath;
        c.passes = 1;
        m_cards.push_back(std::move(c));
    };

    CARD("simple_test",  "Grayscale Test",    "Simple",
         "Basic grayscale shader, validates render pipeline",
         "effects/simple_test/simple_test.frag.spv");

    CARD("bloom",        "Bloom",             "Lighting",
         "Extract bright areas with blur overlay for dreamy glow",
         "effects/bloom/bloom.frag.spv");

    CARD("blur",         "Gaussian Blur",     "Filter",
         "Multi-pass separable Gaussian blur, large radius soft focus",
         "effects/blur/blur.frag.spv");

    CARD("sharpen",      "Sharpen",           "Filter",
         "Unsharp mask image sharpening, enhances edge detail",
         "effects/sharpen/sharpen.frag.spv");

    CARD("edge_detect",  "Edge Detection",    "Filter",
         "Sobel edge detection with optional normal visualization",
         "effects/edge_detect/edge_detect.frag.spv");

    CARD("emboss",       "Emboss",            "Stylize",
         "Emboss filter for relief texture effect",
         "effects/emboss/emboss.frag.spv");

    CARD("pixelate",     "Pixelate",          "Stylize",
         "Adjustable mosaic pixelation block size",
         "effects/pixelate/pixelate.frag.spv");

    CARD("vignette",     "Vignette",          "Color",
         "Darken edges to focus on center subject",
         "effects/vignette/vignette.frag.spv");

    CARD("chromatic",    "Chromatic Aberration", "Distort",
         "RGB channel offset simulating chromatic distortion",
         "effects/chromatic/chromatic.frag.spv");

    CARD("color_grade",  "Color Grading",     "Color",
         "LUT-based cinematic color grading",
         "effects/color_grade/color_grade.frag.spv");

    CARD("noise",        "Noise Generator",   "Procedural",
         "Perlin noise with adjustable frequency and amplitude",
         "effects/noise/noise.frag.spv");

    CARD("kaleidoscope", "Kaleidoscope",      "Distort",
         "Radial symmetry with adjustable sectors and rotation",
         "effects/kaleidoscope/kaleidoscope.frag.spv");

    CARD("glitch",       "Glitch Art",        "Stylize",
         "Digital glitch with random block shift and color tearing",
         "effects/glitch/glitch.frag.spv");

    CARD("toon",         "Toon Shading",      "Stylize",
         "Cartoon-style color quantization, cel shading",
         "effects/toon/toon.frag.spv");

    CARD("vhs",          "VHS Retro",         "Retro",
         "VHS tape scanlines, noise and color drift",
         "effects/vhs/vhs.frag.spv");

    CARD("crt",          "CRT Monitor",       "Retro",
         "CRT scanlines + phosphor RGB pattern + screen curvature",
         "effects/crt/crt.frag.spv");

    CARD("water_ripple", "Water Ripple",      "Distort",
         "Normal-map based water ripple displacement",
         "effects/water_ripple/water_ripple.frag.spv");

    CARD("lens_distort", "Lens Distortion",   "Distort",
         "Barrel/pincushion lens distortion correction and simulation",
         "effects/lens_distort/lens_distort.frag.spv");

    printf("[CoverFlowScene] Registered %zu effect cards\n", m_cards.size());
}

#undef CARD
```

### 3.3 OnEnter / OnExit

#### OnEnter — 场景进入

**分析**：`OnEnter` 在场景被设置为当前场景时调用。它初始化 FPS 计数器的时间戳和计数器，然后调用 `InitializeThumbnails()` 初始化所有卡片的缩略图渲染资源。

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

#### OnExit — 场景退出

**分析**：`OnExit` 在场景被替换或应用关闭时调用。当前实现仅输出日志，没有额外的清理逻辑。资源的释放由析构函数处理。

```cpp
void CoverFlowScene::OnExit()
{
    printf("[CoverFlowScene] Exiting\n");
}
```

#### 完整源码

```cpp
void CoverFlowScene::OnEnter()
{
    m_fpsLastTime    = std::chrono::high_resolution_clock::now(); // 初始化 FPS 计时起点
    m_fpsFrameCount  = 0;                                   // 重置帧计数
    m_fpsDisplay     = 0.0f;                                 // 重置 FPS 显示值

    printf("[CoverFlowScene] Entered with %zu cards, selected=%d\n",
           m_cards.size(), m_selectedIndex);

    InitializeThumbnails();                                  // 初始化所有卡片的缩略图
}

void CoverFlowScene::OnExit()
{
    printf("[CoverFlowScene] Exiting\n");                    // 输出退出日志
}
```

### 3.4 OnUpdate

`OnUpdate` 是每帧调用的核心更新函数，负责处理自动测试逻辑、滚动动画、FPS 计数、拖放文件、视频播放、屏幕捕获、键盘/鼠标输入等所有交互逻辑。

#### 第一段：自动测试恢复逻辑

**分析**：当从详情页返回且自动测试模式激活时，`m_autoTestLastOpenedCard` 被设为非负值。此时将测试索引推进到下一张卡片，如果所有卡片已测试完毕则关闭自动测试，否则选择下一张卡片并打开其详情页。

```cpp
    if (m_autoTest && m_autoTestLastOpenedCard >= 0) {
        m_autoTestCardIndex = m_autoTestLastOpenedCard + 1;
        m_autoTestLastOpenedCard = -1;
        if (m_autoTestCardIndex >= (int)m_cards.size()) {
            printf("\n[AutoTest] ALL %zu CARDS TESTED SUCCESSFULLY!\n", m_cards.size());
            m_autoTest = false;
        } else {
            SelectCard(m_autoTestCardIndex);
            OpenSelectedEffect();
            m_autoTestFrameCounter = m_autoTestHoldFrames;
        }
    }
```

#### 第二段：平滑滚动动画

**分析**：使用线性插值实现平滑滚动效果。`m_scrollOffset` 以 `dt * 8.0` 的速率向 `m_targetOffset` 靠近，`std::fmin(1.0f, dt * 8.0f)` 确保每帧最多移动差值的 100%，避免过冲。

```cpp
    float diff = m_targetOffset - m_scrollOffset;
    m_scrollOffset += diff * std::fmin(1.0f, dt * 8.0f);
```

#### 第三段：拖放文件处理

**分析**：通过 `m_app->ConsumeDroppedFile()` 获取拖放的文件路径。根据文件扩展名判断是视频文件还是图片文件，分别调用对应处理函数。

```cpp
    if (m_app) {
        std::string dropped = m_app->ConsumeDroppedFile();
        if (!dropped.empty()) {
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');
            if (dot != std::string::npos) {
                ext = ext.substr(dot);
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    OpenVideoFile(dropped);
                } else {
                    StopVideo();
                    ReloadInputTexture(dropped);
                }
            }
        }
    }
```

#### 第四段：视频播放器更新

**分析**：如果视频播放器处于活跃状态，按照 30fps 的固定帧率检查是否需要读取下一帧。如果 ffmpeg 管道输出了新帧，则更新视频纹理并设为当前输入纹理。

```cpp
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();
        double frameInterval = 1.0 / 30.0;
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else {
                StopVideo();
            }
        }
    }
```

#### 第五段：屏幕捕获更新

**分析**：如果屏幕捕获处于活跃状态且捕获器已就绪，尝试捕获新帧并更新 GPU 纹理。

```cpp
    if (m_captureActive && m_screenCapture && m_screenCapture->IsReady()) {
        bool newFrame = m_screenCapture->CaptureFrame();
        if (newFrame && m_backend) {
            m_backend->UpdateTexture(m_captureTex, 0, 0,
                m_captureWidth, m_captureHeight,
                m_screenCapture->GetPixels());
        }
    }
```

#### 第六段：键盘导航与快捷键

**分析**：处理多种键盘输入：左右箭头选择相邻卡片、Enter 打开详情页、ESC 退出、F1-F8 快速选择前 8 个效果、Ctrl+S 切换屏幕捕获、Ctrl+Left/Right 切换内置图片。

```cpp
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft))
        SelectCard(m_selectedIndex - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight))
        SelectCard(m_selectedIndex + 1);
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        OpenSelectedEffect();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_wantsExit = true;
    }

    for (int k = 0; k < 8; k++) {
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + k))) {
            int targetIdx = k;
            if (targetIdx < (int)m_cards.size()) SelectCard(targetIdx);
        }
    }

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) ToggleScreenCapture();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) CycleImage(-1);
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) CycleImage(+1);
```

#### 第七段：鼠标滚轮与拖拽

**分析**：鼠标滚轮向上滚动选择前一张卡片，向下滚动选择后一张。鼠标左键拖拽时，将像素位移转换为卡片索引偏移，实现流畅的拖拽浏览。

```cpp
    float wheel = io.MouseWheel;
    if (wheel != 0.0f)
        SelectCard(m_selectedIndex - static_cast<int>(wheel));

    const float cardW = 280.0f;
    const float spacing = 80.0f;
    const float cardUnit = cardW + spacing;

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        float dx = io.MouseDelta.x;
        if (!m_dragging) {
            m_dragging = true;
            m_dragStartX = io.MousePos.x;
            m_dragBaseOff = m_scrollOffset;
        }
        float offset = m_dragBaseOff - dx / cardUnit;
        int idx = m_selectedIndex + static_cast<int>(std::round(offset));
        if (idx < 0) idx = 0;
        if (idx >= (int)m_cards.size()) idx = (int)m_cards.size() - 1;
        if (idx != m_selectedIndex) {
            SelectCard(idx);
            m_scrollOffset = offset - static_cast<float>(idx - m_selectedIndex);
        }
    } else {
        m_dragging = false;
    }
```

#### 完整源码

```cpp
void CoverFlowScene::OnUpdate(float dt)
{
    // 自动测试恢复：从详情页返回后推进到下一张卡片
    if (m_autoTest && m_autoTestLastOpenedCard >= 0) {
        m_autoTestCardIndex = m_autoTestLastOpenedCard + 1;
        m_autoTestLastOpenedCard = -1;
        if (m_autoTestCardIndex >= (int)m_cards.size()) {
            printf("\n[AutoTest] ALL %zu CARDS TESTED SUCCESSFULLY!\n", m_cards.size());
            m_autoTest = false;
        } else {
            SelectCard(m_autoTestCardIndex);
            OpenSelectedEffect();
            m_autoTestFrameCounter = m_autoTestHoldFrames;
        }
    }

    // 平滑滚动动画（线性插值）
    float diff = m_targetOffset - m_scrollOffset;
    m_scrollOffset += diff * std::fmin(1.0f, dt * 8.0f);

    ImGuiIO& io = ImGui::GetIO();

    UpdateFPSCounter();                                       // 更新 FPS 计数器

    // 拖放文件处理
    if (m_app) {
        std::string dropped = m_app->ConsumeDroppedFile();
        if (!dropped.empty()) {
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');
            if (dot != std::string::npos) {
                ext = ext.substr(dot);
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    OpenVideoFile(dropped);
                } else {
                    StopVideo();
                    ReloadInputTexture(dropped);
                }
            }
        }
    }

    // 视频播放器更新
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();
        double frameInterval = 1.0 / 30.0;
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else {
                StopVideo();
            }
        }
    }

    // 屏幕捕获更新
    if (m_captureActive && m_screenCapture && m_screenCapture->IsReady()) {
        bool newFrame = m_screenCapture->CaptureFrame();
        if (newFrame && m_backend) {
            m_backend->UpdateTexture(m_captureTex, 0, 0,
                m_captureWidth, m_captureHeight,
                m_screenCapture->GetPixels());
        }
    }

    // 自动测试计时器
    if (m_autoTest && !m_wantsExit) {
        m_autoTestFrameCounter--;
        if (m_autoTestFrameCounter <= 0) {
            m_autoTestLastOpenedCard = m_autoTestCardIndex;
            OpenSelectedEffect();
        }
    }

    // 键盘导航
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft))
        SelectCard(m_selectedIndex - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight))
        SelectCard(m_selectedIndex + 1);
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        OpenSelectedEffect();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        printf("[CoverFlowScene] ESC detected, exiting\n");
        m_wantsExit = true;
    }

    // F1-F8 快速选择
    for (int k = 0; k < 8; k++) {
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + k))) {
            int targetIdx = k;
            if (targetIdx < (int)m_cards.size()) SelectCard(targetIdx);
        }
    }

    // Ctrl+S：切换屏幕捕获
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) ToggleScreenCapture();

    // Ctrl+Left/Right：切换内置图片
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) CycleImage(-1);
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) CycleImage(+1);

    // 鼠标滚轮
    float wheel = io.MouseWheel;
    if (wheel != 0.0f)
        SelectCard(m_selectedIndex - static_cast<int>(wheel));

    // 鼠标拖拽浏览卡片
    const float cardW   = 280.0f;
    const float spacing = 80.0f;
    const float cardUnit = cardW + spacing;

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        float dx = io.MouseDelta.x;
        if (!m_dragging) {
            m_dragging = true;
            m_dragStartX = io.MousePos.x;
            m_dragBaseOff = m_scrollOffset;
        }
        float offset = m_dragBaseOff - dx / cardUnit;
        int idx = m_selectedIndex + static_cast<int>(std::round(offset));
        if (idx < 0) idx = 0;
        if (idx >= (int)m_cards.size()) idx = (int)m_cards.size() - 1;
        if (idx != m_selectedIndex) {
            SelectCard(idx);
            m_scrollOffset = offset - static_cast<float>(idx - m_selectedIndex);
        }
    } else {
        m_dragging = false;
    }
}
```

### 3.5 OnRender + RenderVisibleThumbnails

#### OnRender — 渲染入口

**分析**：`OnRender` 每帧被调用，主要职责是更新缩略图的时间/帧计数器，然后调用 `RenderVisibleThumbnails()` 渲染当前可见范围内的卡片缩略图。缩略图渲染在 GPU 上通过 FBO 完成，每张可见卡片的效果着色器以低分辨率（256x144）实时处理输入纹理。

```cpp
void CoverFlowScene::OnRender(IRenderBackend* /*backend*/)
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    m_thumbElapsedTime += dt;
    m_thumbFrameCount++;

    RenderVisibleThumbnails();
}
```

#### RenderVisibleThumbnails — 渲染可见缩略图

**分析**：只渲染选中卡片前后各 3 张（共 7 张）的缩略图，避免不必要的 GPU 开销。对于每张可见卡片，优先使用其专属的缓存输入纹理，如果没有则使用默认输入纹理。渲染流程为：开始渲染到纹理 -> 设置着色器参数 -> 绘制全屏四边形 -> 结束渲染到纹理 -> 获取 ImGui 纹理 ID。

```cpp
void CoverFlowScene::RenderVisibleThumbnails()
{
    if (!m_thumbInitialized || !m_backend) return;

    const int vr = 3;

    for (int i = m_selectedIndex - vr; i <= m_selectedIndex + vr; i++) {
        if (i < 0 || i >= (int)m_thumbnailStates.size()) continue;
        const auto& state = m_thumbnailStates[i];
        if (state.fragShader.id == INVALID_SHADER.id) continue;
        if (state.thumbTex.id == INVALID_TEXTURE.id) continue;

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

        m_thumbIds[i] = m_backend->GetImTextureID(state.thumbTex);
    }
}
```

#### 完整源码

```cpp
void CoverFlowScene::OnRender(IRenderBackend* /*backend*/)
{
    // 更新缩略图时间/帧计数器
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    m_thumbElapsedTime += dt;                                // 累计时间（用于动画效果）
    m_thumbFrameCount++;                                     // 递增帧计数

    RenderVisibleThumbnails();                                // 渲染可见卡片的缩略图
}

void CoverFlowScene::RenderVisibleThumbnails()
{
    if (!m_thumbInitialized || !m_backend) return;

    const int vr = 3;                                         // 可见范围：选中卡片前后各3张

    for (int i = m_selectedIndex - vr; i <= m_selectedIndex + vr; i++) {
        if (i < 0 || i >= (int)m_thumbnailStates.size()) continue;
        const auto& state = m_thumbnailStates[i];
        if (state.fragShader.id == INVALID_SHADER.id) continue;
        if (state.thumbTex.id == INVALID_TEXTURE.id) continue;

        // 选择输入纹理：优先使用每效果缓存纹理
        TextureHandle input = m_inputTex;
        if (i < (int)m_inputTexCache.size() && m_inputTexCache[i].id != INVALID_TEXTURE.id) {
            input = m_inputTexCache[i];
        }

        m_backend->BeginRenderToTexture(state.thumbTex);       // 开始渲染到 FBO 纹理
        ShaderParams params;
        params.inputTextures.push_back(input);
        params.uniformFloats  = {4.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        params.viewportWidth  = m_thumbWidth;                  // 256
        params.viewportHeight = m_thumbHeight;                // 144
        params.time           = m_thumbElapsedTime;
        params.frameCount     = m_thumbFrameCount;
        m_backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
        m_backend->EndRenderToTexture();

        m_thumbIds[i] = m_backend->GetImTextureID(state.thumbTex); // 获取 ImGui 纹理 ID
    }
}
```

### 3.6 OnImGui

`OnImGui` 是 CoverFlowScene 中最大的函数，负责绘制整个封面流 UI 界面。由于篇幅较长，此处展示其核心逻辑段的分段分析和完整源码。

#### 第一段：窗口设置与标题绘制

**分析**：创建一个全屏无边框的 ImGui 窗口作为绘制画布。窗口背景色设为深灰色 `(0.08, 0.09, 0.12)`。在窗口顶部居中绘制标题和副标题（显示当前输入源和 FPS）。

#### 第二段：语言切换按钮与分页指示点

**分析**：右上角放置语言切换按钮。卡片区域上方绘制一排分页指示点，选中的卡片对应高亮圆点。

#### 第三段：卡片绘制（核心渲染逻辑）

**分析**：从最远处的卡片开始向前绘制（确保近处卡片覆盖远处卡片）。只绘制可见范围内的卡片（选中索引前后各 3 张）。每张卡片根据偏移量计算位置、缩放和透明度。每张卡片包含：缩略图、圆角矩形边框、底部半透明遮罩、分类标签、效果名称和索引编号。选中卡片还有可点击的 InvisibleButton。

#### 第四段：导航提示、分类图例与自动测试截图模式

**分析**：底部绘制导航提示和分类图例。自动测试截图模式在 `AUTO_TEST_CARDS` 环境变量设置时自动切换卡片并截图，在 `AUTO_TEST_DETAILS` 设置时自动打开详情页。

#### 完整源码

```cpp
void CoverFlowScene::OnImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x;
    float h = io.DisplaySize.y;
    if (w > 0 && h > 0) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(w, h));
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));

    ImGui::Begin("##CoverFlow", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // ---- 标题 ----
    {
        auto& LM = LanguageManager::Instance();
        const char* title = LM.Title();
        char subtitle[256];
        const char* srcLabel = m_captureActive ? LM.ScreenCaptureLabel()
                              : (!m_imagePool.empty() ? m_imagePool[m_currentImageIndex].c_str() : LM.StaticImageLabel());
        const char* justFile = strrchr(srcLabel, '/');
        if (justFile) justFile++; else { justFile = strrchr(srcLabel, '\\'); if (justFile) justFile++; else justFile = srcLabel; }
        snprintf(subtitle, sizeof(subtitle), LM.InputInfoFormat(), justFile, m_fpsDisplay);
        ImVec2 ts = ImGui::CalcTextSize(title);
        ImVec2 ss = ImGui::CalcTextSize(subtitle);
        dl->AddText(ImGui::GetFont(), 28.0f,
            ImVec2(w / 2.0f - ts.x / 2.0f, 30.0f),
            IM_COL32(220, 220, 255, 255), title);
        dl->AddText(ImGui::GetFont(), 16.0f,
            ImVec2(w / 2.0f - ss.x / 2.0f, 65.0f),
            IM_COL32(140, 140, 160, 220), subtitle);
    }

    // ---- 语言切换按钮（右上角）----
    {
        auto& LM = LanguageManager::Instance();
        ImGui::SetCursorScreenPos(ImVec2(w - 120.0f, 20.0f));
        if (ImGui::Button(LM.LanguageButton(), ImVec2(100, 30))) {
            LM.ToggleLanguage();
        }
    }

    const float cardW   = 280.0f;
    const float cardH   = 380.0f;
    const float spacing = 80.0f;
    const float centerX = w / 2.0f;
    const float centerY = h / 2.0f + 20.0f;

    // ---- 分页指示点 ----
    {
        const float dotR = 4.0f, dotSp = 14.0f;
        const int maxD = 18;
        int ds = std::max(0, m_selectedIndex - 8);
        int de = std::min((int)m_cards.size(), ds + maxD);
        float dx = centerX - (de - ds - 1) * dotSp / 2.0f;
        float dy = centerY - cardH / 2.0f - 25.0f;
        for (int i = ds; i < de; i++) {
            dl->AddCircleFilled(ImVec2(dx, dy), dotR,
                (i == m_selectedIndex) ? IM_COL32(180,180,255,255) : IM_COL32(80,80,100,150));
            dx += dotSp;
        }
    }

    // ---- 卡片绘制 ----
    int vr = 3;
    for (int i = (int)m_cards.size() - 1; i >= 0; i--) {
        if (i < m_selectedIndex - vr || i > m_selectedIndex + vr) continue;

        float off  = (float)(i - m_selectedIndex) + m_scrollOffset - m_targetOffset;
        float x    = centerX + off * (cardW + spacing);
        float scl  = 1.0f / (1.0f + std::fabs(off) * 0.3f);
        float alpha = 1.0f - std::fabs(off) * 0.35f;
        if (alpha < 0.0f) alpha = 0.0f;

        float w2 = cardW * scl, h2 = cardH * scl;
        float x0 = x - w2/2, y0 = centerY - h2/2;
        float x1 = x + w2/2, y1 = centerY + h2/2;

        int ai = (int)(alpha * 255);

        bool hasThumb = i < (int)m_thumbIds.size() && m_thumbIds[i] != nullptr;
        ImU32 bg, bd;
        if (i == m_selectedIndex) {
            bg = IM_COL32(50,55,80,ai); bd = IM_COL32(160,160,240,ai);
        } else {
            bg = IM_COL32(35,38,50,ai); bd = IM_COL32(80,80,110,(int)(alpha*200));
        }

        float r = 10.0f * scl;

        if (hasThumb && alpha > 0.1f) {
            dl->AddImageRounded(m_thumbIds[i],
                ImVec2(x0, y0), ImVec2(x1, y1),
                ImVec2(0, 1), ImVec2(1, 0),
                IM_COL32(255, 255, 255, ai), r);
        } else {
            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bg, r);
        }
        dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), bd, r, 0, 2.0f);

        if (alpha > 0.25f) {
            const auto& card = m_cards[i];
            float cx = x;

            // 卡片点击区域
            ImGui::SetCursorScreenPos(ImVec2(x0, y0));
            ImGui::InvisibleButton(("##card_" + std::to_string(i)).c_str(), ImVec2(w2, h2));
            if (ImGui::IsItemClicked()) {
                SelectCard(i);
                OpenSelectedEffect();
            }

            // 底部半透明遮罩
            float overlayH = 140.0f * scl;
            dl->AddRectFilledMultiColor(
                ImVec2(x0, y1 - overlayH), ImVec2(x1, y1),
                IM_COL32(0,0,0,0), IM_COL32(0,0,0,0),
                IM_COL32(20,22,30,(int)(alpha*220)), IM_COL32(20,22,30,(int)(alpha*220)));

            // 分类标签
            float cy = y1 - overlayH + 15.0f * scl;
            {
                const char* catStr = LanguageManager::Instance().TranslateCategory(card.category);
                ImVec2 cs = ImGui::CalcTextSize(catStr);
                float tw = cs.x + 12.0f, th = cs.y + 6.0f;
                dl->AddRectFilled(ImVec2(x0 + 10.0f*scl, cy - 2.0f),
                    ImVec2(x0 + 10.0f*scl + tw, cy + th + 2.0f),
                    IM_COL32(60,65,100,(int)(alpha*220)), 4.0f);
                dl->AddText(ImGui::GetFont(), 11.0f * scl,
                    ImVec2(x0 + 16.0f*scl, cy),
                    IM_COL32(160,170,220,(int)(alpha*255)), catStr);
            }

            // 效果名称
            cy = y1 - overlayH + 38.0f * scl;
            {
                const char* nameStr = LanguageManager::Instance().CardName(card.id);
                dl->AddText(ImGui::GetFont(), 18.0f * scl,
                    ImVec2(x0 + 14.0f*scl, cy),
                    IM_COL32(255, 255, 255, (int)(alpha*255)), nameStr);
            }

            // 索引编号（仅选中卡片）
            if (i == m_selectedIndex) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%d / %zu", i+1, m_cards.size());
                ImVec2 isz = ImGui::CalcTextSize(buf);
                dl->AddText(ImGui::GetFont(), 12.0f,
                    ImVec2(cx - isz.x/2, y1 - 16.0f), IM_COL32(140,140,180,200), buf);
            }
        }
    }

    // ---- 导航提示 ----
    {
        const char* hint = LanguageManager::Instance().BottomHelp();
        ImVec2 hs = ImGui::CalcTextSize(hint);
        dl->AddText(ImGui::GetFont(), 14.0f,
            ImVec2(centerX - hs.x/2, centerY + cardH/2 + 50.0f),
            IM_COL32(150,150,170,160), hint);
    }

    // ---- 分类图例 ----
    {
        auto& LM = LanguageManager::Instance();
        char leg[256];
        snprintf(leg, sizeof(leg), "%s | %s | %s | %s | %s | %s | %s | %s",
            LM.CategorySimple(), LM.CategoryLighting(), LM.CategoryFilter(), LM.CategoryStylize(),
            LM.CategoryColor(), LM.CategoryDistort(), LM.CategoryProcedural(), LM.CategoryRetro());
        ImVec2 ls = ImGui::CalcTextSize(leg);
        dl->AddText(ImGui::GetFont(), 12.0f,
            ImVec2(centerX - ls.x/2, h - 35.0f), IM_COL32(100,100,120,130), leg);
    }

    ImGui::End();
    ImGui::PopStyleColor();

    // ---- 卡片截图模式（AUTO_TEST_CARDS）----
    if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_DETAILS")) {
        g_cardScreenshotFrame++;
        if (g_cardScreenshotNeedsShot && g_cardScreenshotFrame >= 5) {
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/card_%02d.ppm", g_cardScreenshotIndex - 1);
            ScreenshotRequest::Request(path);
            g_cardScreenshotNeedsShot = false;
        }
        if (g_cardScreenshotFrame >= 30) {
            g_cardScreenshotFrame = 0;
            if (g_cardScreenshotIndex < (int)m_cards.size()) {
                SelectCard(g_cardScreenshotIndex);
                printf("[CardScreenshot] Selected card %d: %s\n", g_cardScreenshotIndex, m_cards[g_cardScreenshotIndex].name.c_str());
                g_cardScreenshotIndex++;
                g_cardScreenshotNeedsShot = true;
            } else {
                printf("[CardScreenshot] All %zu cards captured\n", m_cards.size());
                m_wantsExit = true;
            }
        }
    }

    // ---- 详情页截图模式（AUTO_TEST_DETAILS）----
    if (getenv("AUTO_TEST_DETAILS") && !getenv("AUTO_TEST_UI")) {
        static int detailCardIndex = 0;
        static int frameWait = 0;

        frameWait++;
        if (frameWait >= 30) {
            frameWait = 0;
            if (detailCardIndex < (int)m_cards.size()) {
                SelectCard(detailCardIndex);
                printf("[DetailScreenshot] Opening card %d: %s\n", detailCardIndex, m_cards[detailCardIndex].name.c_str());
                detailCardIndex++;
                OpenSelectedEffect();
            } else {
                printf("[DetailScreenshot] All %zu detail pages captured\n", m_cards.size());
                m_wantsExit = true;
            }
        }
    }
}
```

### 3.7 OpenSelectedEffect / GetNextScene

#### OpenSelectedEffect — 打开选中效果的详情页

**分析**：创建 `EffectDetailScene` 实例，传入选中卡片的数据和对应的输入纹理。首先尝试从 `effect.json` 加载更丰富的元数据（如 UI 控件定义），如果加载失败则使用默认卡片数据。优先使用该效果的缓存输入纹理。还会将视频播放器（如果活跃）转移给详情页。

```cpp
void CoverFlowScene::OpenSelectedEffect()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_cards.size()) return;
    if (!m_backend) return;

    const auto& card = m_cards[m_selectedIndex];
    EffectCard resolvedCard = card;

    try {
        std::string shaderDir = ShaderLoader::FindShaderDir();
        std::string jsonPath = shaderDir + "/effects/" + card.id + "/effect.json";
        EffectCard fj = LoadEffectFromJson(jsonPath);
        if (!fj.name.empty()) {
            fj.vertSpirvPath = card.vertSpirvPath;
            fj.fragSpirvPath = card.fragSpirvPath;
            fj.id = card.id;
            resolvedCard = std::move(fj);
        }
    } catch (...) {
        fprintf(stderr, "[CoverFlowScene] effect.json parse error for %s, using defaults\n", card.id.c_str());
    }

    TextureHandle detailInputTex = m_inputTex;
    if (m_selectedIndex < (int)m_inputTexCache.size() &&
        m_inputTexCache[m_selectedIndex].id != INVALID_TEXTURE.id) {
        detailInputTex = m_inputTexCache[m_selectedIndex];
    }

    auto ds = std::make_unique<EffectDetailScene>(resolvedCard, detailInputTex);
    ds->SetBackend(m_backend);
    ds->SetApplication(m_app);
    ds->SetCoverFlowState(GetState());

    if (m_videoActive && m_videoPlayer) {
        ds->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
        m_videoActive = false;
        m_videoTex = {0};
    }

    m_nextScene = std::move(ds);
    m_wantsExit = true;
}
```

#### GetNextScene — 获取下一场景

**分析**：返回之前通过 `OpenSelectedEffect` 设置的 `m_nextScene`，使用 `std::move` 转移所有权。

```cpp
std::unique_ptr<Scene> CoverFlowScene::GetNextScene()
{
    if (m_nextScene) {
        printf("[CoverFlowScene] Transitioning to next scene\n");
        return std::move(m_nextScene);
    }
    return nullptr;
}
```

#### 完整源码

```cpp
void CoverFlowScene::OpenSelectedEffect()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_cards.size()) return;
    if (!m_backend) {
        fprintf(stderr, "[CoverFlowScene] Cannot open effect: no backend set\n");
        return;
    }

    const auto& card = m_cards[m_selectedIndex];
    EffectCard resolvedCard = card;

    // 尝试加载 effect.json 获取更丰富的元数据
    try {
        std::string shaderDir = ShaderLoader::FindShaderDir();
        std::string jsonPath = shaderDir + "/effects/" + card.id + "/effect.json";
        EffectCard fj = LoadEffectFromJson(jsonPath);
        if (!fj.name.empty()) {
            fj.vertSpirvPath = card.vertSpirvPath;
            fj.fragSpirvPath = card.fragSpirvPath;
            fj.id = card.id;
            resolvedCard = std::move(fj);
        }
    } catch (...) {
        fprintf(stderr, "[CoverFlowScene] effect.json parse error for %s, using defaults\n", card.id.c_str());
    }

    // 选择输入纹理：优先使用该效果的缓存纹理
    TextureHandle detailInputTex = m_inputTex;
    if (m_selectedIndex < (int)m_inputTexCache.size() &&
        m_inputTexCache[m_selectedIndex].id != INVALID_TEXTURE.id) {
        detailInputTex = m_inputTexCache[m_selectedIndex];
        printf("[CoverFlowScene] Using cached input texture for card %d\n", m_selectedIndex);
    }

    // 创建效果详情页场景
    auto ds = std::make_unique<EffectDetailScene>(resolvedCard, detailInputTex);
    ds->SetBackend(m_backend);
    ds->SetApplication(m_app);
    ds->SetCoverFlowState(GetState());

    // 转移视频播放器到详情页
    if (m_videoActive && m_videoPlayer) {
        ds->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
        m_videoActive = false;
        m_videoTex = {0};
        printf("[CoverFlowScene] Transferred video player to detail scene\n");
    }

    printf("[CoverFlowScene] Opening effect: %s (%s)\n",
           resolvedCard.name.c_str(), resolvedCard.id.c_str());

    m_nextScene = std::move(ds);
    m_wantsExit = true;
}

std::unique_ptr<Scene> CoverFlowScene::GetNextScene()
{
    if (m_nextScene) {
        printf("[CoverFlowScene] Transitioning to next scene\n");
        return std::move(m_nextScene);
    }
    return nullptr;
}
```

### 3.8 GetState

**分析**：`GetState` 将 CoverFlowScene 的所有重要状态打包到 `CoverFlowState` 结构体中，以便在进入详情页后能够完整恢复。保存的状态包括：缩略图 ID 列表、纹理缓存、图片/视频池、选中索引、应用指针、输入纹理、后端指针、屏幕捕获状态、自动测试状态和测试图片目录。

#### 完整源码

```cpp
CoverFlowState CoverFlowScene::GetState() const
{
    CoverFlowState s;
    s.thumbIds         = m_thumbIds;                         // 缩略图 ImGui 纹理 ID
    s.inputTexCache    = m_inputTexCache;                    // 每效果纹理缓存
    s.imagePool        = m_imagePool;                        // 图片路径池
    s.videoPool        = m_videoPool;                        // 视频路径池
    s.currentImageIndex = m_currentImageIndex;               // 当前图片索引
    s.selectedIndex    = m_selectedIndex;                     // 当前选中卡片索引
    s.app              = m_app;                              // 应用指针
    s.inputTex         = m_inputTex;                         // 当前输入纹理
    s.backend          = m_backend;                           // 渲染后端指针
    s.captureActive    = m_captureActive;                     // 屏幕捕获状态
    s.autoTest         = m_autoTest;                         // 自动测试状态
    s.autoTestHoldFrames = m_autoTestHoldFrames;              // 自动测试每卡停留帧数
    s.autoTestCardIndex  = m_autoTestCardIndex;               // 自动测试当前卡片索引
    s.testImageBaseDir = m_testImageBaseDir;                 // 测试图片目录
    return s;
}
```

### 3.9 辅助功能

本节涵盖 CoverFlowScene 中的辅助函数，包括构造/析构函数、SelectCard、UpdateFPSCounter、ToggleScreenCapture、CycleImage、LoadImageFromFile、AddImageToPool、AddVideoToPool、ReloadInputTexture、InitializeThumbnails、SetVideoPlayer、OpenVideoFile、StopVideo、EnableAutoTest、ResumeAutoTest。

#### 构造与析构函数

**分析**：构造函数调用 `RegisterCards()` 注册所有效果卡片。析构函数遍历所有缩略图状态，销毁每个卡片的片段着色器和 FBO 纹理，以及共享的顶点着色器。

```cpp
CoverFlowScene::CoverFlowScene() { RegisterCards(); }
CoverFlowScene::~CoverFlowScene()
{
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

#### SelectCard — 选择卡片

**分析**：将索引限制在合法范围内（支持循环），如果索引发生变化则更新选中索引并重置滚动偏移。

```cpp
void CoverFlowScene::SelectCard(int index) {
    if (index < 0) index = (int)m_cards.size() - 1;
    if (index >= (int)m_cards.size()) index = 0;
    if (index != m_selectedIndex) {
        m_selectedIndex = index;
        m_targetOffset = 0.0f;
        printf("[CoverFlowScene] Selected card %d: %s\n", index, m_cards[index].name.c_str());
    }
}
```

#### UpdateFPSCounter — 更新 FPS 计数器

**分析**：每帧递增帧计数器，每秒计算一次平均 FPS 并更新显示值。

```cpp
void CoverFlowScene::UpdateFPSCounter()
{
    auto now = std::chrono::high_resolution_clock::now();
    m_fpsFrameCount++;
    float elapsed = std::chrono::duration<float>(now - m_fpsLastTime).count();
    if (elapsed >= 1.0f) {
        m_fpsDisplay = m_fpsFrameCount / elapsed;
        m_fpsFrameCount = 0;
        m_fpsLastTime = now;
    }
}
```

#### ToggleScreenCapture — 切换屏幕捕获

**分析**：如果当前未启用屏幕捕获，则创建 `ScreenCapture` 实例，捕获第一帧获取尺寸，创建 GPU 纹理，并将输入纹理切换为捕获纹理。如果当前已启用，则停止捕获并释放资源。

```cpp
void CoverFlowScene::ToggleScreenCapture()
{
    if (m_captureActive) {
        m_captureActive = false;
        if (m_screenCapture) { m_screenCapture->Shutdown(); m_screenCapture.reset(); }
        if (m_captureTex.id != INVALID_TEXTURE.id && m_backend) {
            m_backend->DestroyTexture(m_captureTex);
            m_captureTex = INVALID_TEXTURE;
        }
        m_captureReady = false;
    } else {
        if (!m_backend) return;
        m_screenCapture = std::make_unique<ScreenCapture>();
        if (!m_screenCapture->Init()) { m_screenCapture.reset(); return; }
        if (!m_screenCapture->CaptureFrame()) { m_screenCapture->Shutdown(); m_screenCapture.reset(); return; }
        m_captureWidth  = m_screenCapture->GetWidth();
        m_captureHeight = m_screenCapture->GetHeight();
        m_captureTex = m_backend->CreateTexture(m_captureWidth, m_captureHeight,
                                                 TextureFormat::RGBA8, m_screenCapture->GetPixels());
        if (m_captureTex.id == INVALID_TEXTURE.id) { m_screenCapture->Shutdown(); m_screenCapture.reset(); return; }
        m_inputTex = m_captureTex;
        m_captureActive = true;
        m_captureReady  = true;
    }
}
```

#### CycleImage / LoadImageFromFile — 图片切换与加载

**分析**：`CycleImage` 在图片池中循环切换（支持前后方向），`LoadImageFromFile` 使用 stb_image 加载图片并创建 GPU 纹理替换当前输入纹理。

```cpp
void CoverFlowScene::CycleImage(int direction)
{
    if (m_imagePool.empty()) return;
    if (m_captureActive) return;
    m_currentImageIndex += direction;
    if (m_currentImageIndex < 0) m_currentImageIndex = (int)m_imagePool.size() - 1;
    if (m_currentImageIndex >= (int)m_imagePool.size()) m_currentImageIndex = 0;
    LoadImageFromFile(m_imagePool[m_currentImageIndex]);
}

void CoverFlowScene::LoadImageFromFile(const std::string& path)
{
    if (!m_backend) return;
    int iw, ih, comp;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data = stbi_load(path.c_str(), &iw, &ih, &comp, 4);
    stbi_set_flip_vertically_on_load(false);
    if (!data) { fprintf(stderr, "[CoverFlowScene] Cannot load: %s\n", path.c_str()); return; }
    if (m_inputTex.id != INVALID_TEXTURE.id) {
        m_backend->DestroyTexture(m_inputTex);
        m_inputTex = INVALID_TEXTURE;
    }
    TextureHandle newTex = m_backend->CreateTexture(iw, ih, TextureFormat::RGBA8, data);
    stbi_image_free(data);
    if (newTex.id == INVALID_TEXTURE.id) return;
    m_inputTex = newTex;
}
```

#### AddImageToPool / AddVideoToPool / ReloadInputTexture

**分析**：`AddImageToPool` 和 `AddVideoToPool` 将路径添加到对应的池中（去重）。`ReloadInputTexture` 加载新图片并更新当前图片索引。

```cpp
void CoverFlowScene::AddImageToPool(const std::string& path)
{
    for (const auto& p : m_imagePool) { if (p == path) return; }
    m_imagePool.push_back(path);
}

void CoverFlowScene::AddVideoToPool(const std::string& path)
{
    for (const auto& p : m_videoPool) { if (p == path) return; }
    m_videoPool.push_back(path);
}

void CoverFlowScene::ReloadInputTexture(const std::string& filePath)
{
    LoadImageFromFile(filePath);
    AddImageToPool(filePath);
    for (int i = 0; i < (int)m_imagePool.size(); i++) {
        if (m_imagePool[i] == filePath) { m_currentImageIndex = i; break; }
    }
}
```

#### InitializeThumbnails — 初始化缩略图渲染

**分析**：加载共享顶点着色器，为每个效果加载片段着色器并创建 256x144 的 FBO 纹理。初始化后设置 `m_thumbInitialized = true` 防止重复初始化。

```cpp
void CoverFlowScene::InitializeThumbnails()
{
    if (m_thumbInitialized || !m_backend) return;
    if (m_cards.empty()) return;

    std::string shaderDir = ShaderLoader::FindShaderDir();
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv";
    if (m_backend->GetType() == BackendType::Vulkan) {
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv";
    }
    auto vertSpv = ShaderLoader::LoadSPIRV(vertPath);
    if (vertSpv.empty()) return;
    m_sharedVertShader = m_backend->CreateVertexShader(vertSpv.data(), vertSpv.size());
    if (m_sharedVertShader.id == INVALID_SHADER.id) return;

    m_thumbIds.resize(m_cards.size(), nullptr);

    for (int i = 0; i < (int)m_cards.size(); i++) {
        CardThumbnailState state;
        state.fragShader = INVALID_SHADER;
        state.thumbTex = INVALID_TEXTURE;

        auto fragSpv = ShaderLoader::LoadSPIRV(m_cards[i].fragSpirvPath);
        if (!fragSpv.empty()) {
            state.fragShader = m_backend->CreateFragmentShader(fragSpv.data(), fragSpv.size());
        }

        state.thumbTex = m_backend->CreateTexture(m_thumbWidth, m_thumbHeight, TextureFormat::RGBA8, nullptr);
        m_thumbnailStates.push_back(state);
    }

    m_thumbInitialized = true;
}
```

#### SetVideoPlayer / OpenVideoFile / StopVideo

**分析**：`SetVideoPlayer` 从详情页接收视频播放器所有权。`OpenVideoFile` 创建新的视频播放器并创建 GPU 纹理。`StopVideo` 关闭视频播放器但不销毁纹理（可能仍被详情页引用）。

```cpp
void CoverFlowScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);
    m_videoTex = videoTex;
    m_videoActive = active;
    m_videoLastFrameTime = lastFrameTime;
    if (active) m_inputTex = m_videoTex;
}

void CoverFlowScene::OpenVideoFile(const std::string& path)
{
    if (!m_backend) return;
    StopVideo();
    m_videoPlayer = std::make_unique<VideoPlayer>();
    if (!m_videoPlayer->Open(path)) { m_videoPlayer.reset(); return; }
    m_videoTex = m_backend->CreateTexture(
        m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
        TextureFormat::RGBA8, m_videoPlayer->GetPixels());
    if (m_videoTex.id == INVALID_TEXTURE.id) { m_videoPlayer->Close(); m_videoPlayer.reset(); return; }
    m_inputTex = m_videoTex;
    m_videoActive = true;
    m_videoLastFrameTime = ImGui::GetTime();
}

void CoverFlowScene::StopVideo()
{
    if (m_videoPlayer) { m_videoPlayer->Close(); m_videoPlayer.reset(); }
    m_videoActive = false;
}
```

#### EnableAutoTest / ResumeAutoTest

**分析**：`EnableAutoTest` 启用自动测试，设置每卡停留帧数，并立即打开第一张卡片的详情页。`ResumeAutoTest` 在从详情页返回后恢复自动测试，设置 `m_autoTestLastOpenedCard` 以触发 OnUpdate 中的"下一张卡片"逻辑。

```cpp
void CoverFlowScene::EnableAutoTest(int holdFrames) {
    m_autoTest = true;
    m_autoTestHoldFrames = holdFrames;
    m_autoTestFrameCounter = holdFrames;
    m_autoTestCardIndex = 0;
    m_autoTestLastOpenedCard = -1;
    if (!getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_CARDS")) {
        SelectCard(0);
        OpenSelectedEffect();
    }
}

void CoverFlowScene::ResumeAutoTest(int holdFrames, int lastOpenedCard) {
    m_autoTest = true;
    m_autoTestHoldFrames = holdFrames;
    m_autoTestFrameCounter = 0;
    m_autoTestCardIndex = lastOpenedCard;
    m_autoTestLastOpenedCard = lastOpenedCard;
}
```


# Shader Showcase 技术文档（第二部分）

## 4. EffectDetailScene — 特效详情页

`EffectDetailScene` 是用户从封面流（CoverFlow）点击某张特效卡片后进入的详情页面。它的核心职责是：加载该特效对应的 SPIR-V 着色器，将用户输入的图像/视频作为纹理传入着色器，实时渲染全屏后处理效果，并提供 Before/After 对比视图、参数调试面板和素材拖放加载等功能。

---

### 4.1 类定义

#### 头文件概览

**分析**：`EffectDetailScene` 继承自 `Scene` 基类，通过虚函数接口实现场景生命周期管理。头文件声明了该场景所需的全部成员变量和私有方法。成员变量可分为以下几类：

1. **场景核心数据**：`m_card`（特效卡片元数据）、`m_inputTex`（输入纹理句柄）、`m_backend`（渲染后端指针）
2. **着色器资源**：`m_vertShader`、`m_fragShader`（SPIR-V 着色器句柄）
3. **运行时状态**：`m_time`（累计时间）、`m_frameCount`（帧计数）、`m_viewportWidth/Height`（视口尺寸）
4. **调试面板**：`m_debugPanel`（DebugPanel 实例）、`m_showDebug`（面板可见性开关）
5. **Uniform 数据**：`m_uniformFloats`、`m_uniformInts`（从 effect.json 参数默认值初始化）
6. **场景切换**：`m_wantsExit`、`m_returnToCoverFlow`（退出标志与返回目标）
7. **CoverFlow 状态保存**：`m_savedState`（用于返回时完整恢复 CoverFlowScene）
8. **自动测试**：`m_autoTestHoldFrames`（自动退出帧计数器）
9. **对比视图**：`m_compareMode`、`m_compareSplitPos`、`m_compareDragging`、`m_effectTex`（Before/After 分屏对比相关）
10. **视频播放**：`m_videoPlayer`、`m_videoTex`、`m_videoActive`（动态视频输入源）

```cpp
#pragma once

#include "app/Scene.h"
#include "app/CoverFlowState.h"
#include "shader/EffectMetadata.h"
#include "render/IRenderBackend.h"
#include "ui/DebugPanel.h"

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class Application;
class ScreenCapture;
class VideoPlayer;

class EffectDetailScene : public Scene {
public:
    EffectDetailScene(const EffectCard& card, TextureHandle inputTex);
    ~EffectDetailScene();

    void SetBackend(IRenderBackend* backend) { m_backend = backend; }
    void SetApplication(Application* app) { m_app = app; }

    /// 保存 CoverFlowScene 状态以便返回时恢复
    void SetCoverFlowState(const CoverFlowState& state) {
        m_savedState = state;
        // 自动测试模式：设置自动返回计时器
        if (state.autoTest) {
            m_autoTestHoldFrames = state.autoTestHoldFrames;
        }
    }

    /// 从 CoverFlowScene 转移视频播放器所有权（用于动态视频播放）
    void SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime);

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsExit() const override { return m_wantsExit; }
    std::unique_ptr<Scene> GetNextScene() override;

private:
    EffectCard      m_card;          // 特效卡片元数据
    TextureHandle   m_inputTex;      // 输入纹理句柄
    IRenderBackend* m_backend = nullptr;
    Application*    m_app     = nullptr;

    ShaderHandle m_vertShader = INVALID_SHADER;  // 顶点着色器句柄
    ShaderHandle m_fragShader = INVALID_SHADER;  // 片段着色器句柄

    float    m_time       = 0.0f;    // 累计时间（秒）
    uint32_t m_frameCount = 0;       // 帧计数
    int      m_viewportWidth  = 0;   // 视口宽度
    int      m_viewportHeight = 0;   // 视口高度

    bool      m_showDebug = true;     // 调试面板可见性
    DebugPanel m_debugPanel;          // 参数调试面板

    std::vector<float>   m_uniformFloats;   // float 类型 uniform 值
    std::vector<int32_t> m_uniformInts;     // int 类型 uniform 值
    size_t               m_expectedFloatCount = 0;  // effect.json 中定义的 float 参数总数

    bool m_wantsExit         = false;  // 是否请求退出场景
    bool m_returnToCoverFlow = false;  // 是否返回 CoverFlow

    // 保存的 CoverFlow 状态（用于恢复）
    CoverFlowState m_savedState;

    // 自动测试
    int m_autoTestHoldFrames = 0;  // >0 表示在此帧数后自动退出

    // 对比模式（滑块 Before/After）
    bool m_compareMode = true;         // 默认启用对比模式，按 C 键切换
    float m_compareSplitPos = 0.5f;    // 分割位置（0=左侧全原图，1=右侧全效果图）
    bool m_compareDragging = false;    // 用户是否正在拖动分割手柄
    TextureHandle m_effectTex = {0};   // 效果输出的 FBO 纹理
    bool m_effectTexCreated = false;  // 效果纹理是否已创建

    // 截图计数器（每个场景实例独立）
    int m_screenshotCaptured = 0;

    // 视频播放
    std::unique_ptr<VideoPlayer> m_videoPlayer;  // 视频播放器
    TextureHandle m_videoTex = {0};               // 视频帧纹理
    bool m_videoActive = false;                    // 视频是否正在播放
    double m_videoLastFrameTime = 0.0;            // 上一帧视频时间戳

    void LoadImageFromFile(const std::string& path);  // 从文件加载图片
    void LoadVideoFromFile(const std::string& path);  // 从文件加载视频
    void StopVideo();                                 // 停止视频播放
    void EnsureEffectTexture();                       // 确保效果纹理已创建

    // 对比视图渲染
    void RenderCompareView(IRenderBackend* backend);
    void RenderFullscreenEffect(IRenderBackend* backend);
};
```

---

### 4.2 构造与析构

#### 构造函数

**分析**：构造函数非常简洁，仅通过初始化列表保存传入的特效卡片元数据和输入纹理句柄。此时不进行任何 GL 资源分配，所有重量级初始化推迟到 `OnEnter()` 中进行。这种"延迟初始化"模式确保了构造函数永远不会失败。

```cpp
EffectDetailScene::EffectDetailScene(const EffectCard& card, TextureHandle inputTex)
    : m_card(card)
    , m_inputTex(inputTex)
{
}
```

#### 析构函数

**分析**：析构函数承担资源清理的"安全网"角色。它检查 `m_backend` 指针是否有效，然后逐一释放顶点着色器、片段着色器和效果纹理（FBO 纹理）。注意 `OnExit()` 也会执行相同的清理逻辑——这是双重保险模式，确保无论场景如何退出（正常返回、异常中断），GL 资源都不会泄漏。最后调用 `StopVideo()` 停止视频播放并释放播放器。

```cpp
EffectDetailScene::~EffectDetailScene()
{
    // 释放 GL 资源，防止场景切换时资源累积
    if (m_backend) {
        if (m_vertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_vertShader);  // 销毁顶点着色器
            m_vertShader = INVALID_SHADER;
        }
        if (m_fragShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_fragShader);  // 销毁片段着色器
            m_fragShader = INVALID_SHADER;
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_effectTex);  // 销毁效果纹理
            m_effectTex = {0};
            m_effectTexCreated = false;
        }
    }
    StopVideo();  // 停止视频播放并释放播放器
}
```

#### SetVideoPlayer

**分析**：此方法用于从 CoverFlowScene 接管视频播放器的所有权。使用 `std::move` 转移 `unique_ptr`，避免拷贝。如果视频正在播放（`active == true`），则将视频纹理设为当前输入纹理，使后续着色器渲染使用视频帧作为输入源。

```cpp
void EffectDetailScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);     // 转移视频播放器所有权
    m_videoTex = videoTex;                 // 保存视频纹理句柄
    m_videoActive = active;                // 记录播放状态
    m_videoLastFrameTime = lastFrameTime;  // 保存上一帧时间戳
    if (active) {
        m_inputTex = m_videoTex;  // 将视频纹理设为着色器输入
    }
}
```

#### 完整源码

```cpp
EffectDetailScene::EffectDetailScene(const EffectCard& card, TextureHandle inputTex)
    : m_card(card)
    , m_inputTex(inputTex)
{
}

EffectDetailScene::~EffectDetailScene()
{
    // 释放 GL 资源以防止场景切换时资源累积
    if (m_backend) {
        if (m_vertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_vertShader);
            m_vertShader = INVALID_SHADER;
        }
        if (m_fragShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_fragShader);
            m_fragShader = INVALID_SHADER;
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_effectTex);
            m_effectTex = {0};
            m_effectTexCreated = false;
        }
    }
    StopVideo();
}

void EffectDetailScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);
    m_videoTex = videoTex;
    m_videoActive = active;
    m_videoLastFrameTime = lastFrameTime;
    if (active) {
        m_inputTex = m_videoTex;  // 使用视频纹理作为效果渲染的输入
    }
}
```

---

### 4.3 OnEnter

#### 第一段：后端检查与调试面板初始化

**分析**：`OnEnter()` 在场景被激活时调用。首先检查 `m_backend` 是否有效——如果没有渲染后端，整个场景无法工作，直接返回。然后初始化调试面板（`DebugPanel`），将 effect.json 中定义的参数传入面板，面板会为每个参数生成对应的 ImGui 控件。

```cpp
void EffectDetailScene::OnEnter()
{
    if (!m_backend) {
        fprintf(stderr, "[EffectDetailScene] OnEnter called without backend!\n");
        return;
    }

    // 用特效参数初始化调试面板（所有后端通用）
    m_debugPanel.SetParams(m_card.params);
```

#### 第二段：从参数默认值初始化 Uniform 数组

**分析**：遍历 `m_card.params`（来自 effect.json），根据每个参数的类型（Float、Int、Bool、Float2、Float3、Color、Float4）将默认值压入 `m_uniformFloats` 或 `m_uniformInts` 向量。这个数组将在每帧渲染时传递给着色器。`m_expectedFloatCount` 记录了预期的 float 数量，用于后续 DebugPanel 可能修改数组长度时的修正。

```cpp
    // 从卡片参数默认值初始化 uniform 值
    m_uniformFloats.clear();
    m_uniformInts.clear();
    for (const auto& p : m_card.params) {
        switch (p.type) {
        case ParamType::Float:
            m_uniformFloats.push_back(p.defaultVal[0]);
            break;
        case ParamType::Int:
        case ParamType::Bool:
            m_uniformInts.push_back(static_cast<int32_t>(p.defaultVal[0]));
            break;
        case ParamType::Float2:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            break;
        case ParamType::Float3:
        case ParamType::Color:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            break;
        case ParamType::Float4:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            m_uniformFloats.push_back(p.defaultVal[3]);
            break;
        }
    }
    m_expectedFloatCount = m_uniformFloats.size();
```

#### 第三段：加载 SPIR-V 着色器

**分析**：从 effect.json 中记录的路径加载 SPIR-V 二进制着色器。对于 Vulkan 后端，需要将 `fullscreen.vert.spv` 替换为 `fullscreen_vk.vert.spv`（Vulkan 使用 `gl_VertexIndex` 生成三角形，而 OpenGL 使用 VAO 顶点输入）。使用 `ShaderLoader::LoadSPIRV()` 读取文件，然后通过后端接口创建着色器。注释中解释了为什么统一使用 SPIR-V 而非 GLSL：NVIDIA 驱动在混合使用 SPIR-V 和 GLSL 时 UBO 反射不可靠。

```cpp
    // 加载 SPIR-V 着色器（所有后端通用）
    // OpenGL 使用 VAO 顶点输入，Vulkan 使用 VertexIndex 生成的三角形
    std::string vertPath = m_card.vertSpirvPath;
    if (m_backend->GetType() == BackendType::Vulkan) {
        // 为 Vulkan 替换 fullscreen.vert.spv 为 fullscreen_vk.vert.spv
        size_t pos = vertPath.find("fullscreen.vert.spv");
        if (pos != std::string::npos) {
            vertPath.replace(pos, 19, "fullscreen_vk.vert.spv");
        }
    }
    auto vertSpirv = ShaderLoader::LoadSPIRV(vertPath);

    auto fragSpirv = ShaderLoader::LoadSPIRV(m_card.fragSpirvPath);

    if (vertSpirv.empty() || fragSpirv.empty()) {
        fprintf(stderr, "[EffectDetailScene] Failed to load SPIR-V shaders\n");
        return;
    }

    // 始终使用 SPIR-V 着色器（与 CoverFlowScene 缩略图路径一致）
    // GLSL+UBO 在 NVIDIA 上混合顶点/片段着色器类型时反射不可靠
    // 纯 SPIR-V 在所有情况下都能正常工作
    m_vertShader = m_backend->CreateVertexShader(vertSpirv.data(), vertSpirv.size());
    m_fragShader = m_backend->CreateFragmentShader(fragSpirv.data(), fragSpirv.size());
    printf("[EffectDetailScene] Using SPIR-V shaders\n");

    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[EffectDetailScene] Failed to create shaders\n");
        return;
    }
}
```

#### 完整源码

```cpp
void EffectDetailScene::OnEnter()
{
    if (!m_backend) {
        fprintf(stderr, "[EffectDetailScene] OnEnter called without backend!\n");
        return;
    }

    // 用特效参数初始化调试面板（所有后端通用）
    m_debugPanel.SetParams(m_card.params);
    
    // 从卡片参数默认值初始化 uniform 值
    m_uniformFloats.clear();
    m_uniformInts.clear();
    for (const auto& p : m_card.params) {
        switch (p.type) {
        case ParamType::Float:
            m_uniformFloats.push_back(p.defaultVal[0]);
            break;
        case ParamType::Int:
        case ParamType::Bool:
            m_uniformInts.push_back(static_cast<int32_t>(p.defaultVal[0]));
            break;
        case ParamType::Float2:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            break;
        case ParamType::Float3:
        case ParamType::Color:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            break;
        case ParamType::Float4:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            m_uniformFloats.push_back(p.defaultVal[3]);
            break;
        }
    }
    m_expectedFloatCount = m_uniformFloats.size();

    // 加载 SPIR-V 着色器（所有后端通用）
    // OpenGL 使用 VAO 顶点输入，Vulkan 使用 VertexIndex 生成的三角形
    std::string vertPath = m_card.vertSpirvPath;
    if (m_backend->GetType() == BackendType::Vulkan) {
        // 为 Vulkan 替换 fullscreen.vert.spv 为 fullscreen_vk.vert.spv
        size_t pos = vertPath.find("fullscreen.vert.spv");
        if (pos != std::string::npos) {
            vertPath.replace(pos, 19, "fullscreen_vk.vert.spv");
        }
    }
    auto vertSpirv = ShaderLoader::LoadSPIRV(vertPath);

    auto fragSpirv = ShaderLoader::LoadSPIRV(m_card.fragSpirvPath);

    if (vertSpirv.empty() || fragSpirv.empty()) {
        fprintf(stderr, "[EffectDetailScene] Failed to load SPIR-V shaders\n");
        return;
    }

    // 始终使用 SPIR-V 着色器
    m_vertShader = m_backend->CreateVertexShader(vertSpirv.data(), vertSpirv.size());
    m_fragShader = m_backend->CreateFragmentShader(fragSpirv.data(), fragSpirv.size());
    printf("[EffectDetailScene] Using SPIR-V shaders\n");

    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[EffectDetailScene] Failed to create shaders\n");
        return;
    }
}
```

---

### 4.4 OnUpdate

#### 第一段：时间累计与自动测试退出

**分析**：每帧累加时间和帧计数。自动测试模式下，`m_autoTestHoldFrames` 递减，到达 0 时自动设置退出标志并返回 CoverFlow。这是 CI/CD 自动化测试的关键机制——无需用户交互即可遍历所有特效。

```cpp
void EffectDetailScene::OnUpdate(float dt)
{
    m_time += dt;        // 累加时间
    m_frameCount++;      // 递增帧计数

    // 自动测试：在 holdFrames 帧后自动返回
    if (m_autoTestHoldFrames > 0) {
        m_autoTestHoldFrames--;
        if (m_autoTestHoldFrames <= 0) {
            m_wantsExit = true;
            m_returnToCoverFlow = true;
            printf("[EffectDetailScene] Auto-test timer expired, returning to CoverFlow\n");
        }
    }
```

#### 第二段：ESC 键返回

**分析**：监听 ImGui 的键盘输入事件，按下 ESC 键时设置退出标志。使用 ImGui 而非 GLFW 回调的好处是输入处理与 UI 系统统一，避免事件冲突。

```cpp
    // ESC 返回 CoverFlow
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_wantsExit         = true;
        m_returnToCoverFlow = true;
        printf("[EffectDetailScene] ESC pressed, returning to CoverFlow\n");
    }
```

#### 第三段：拖放文件处理

**分析**：通过 `Application::ConsumeDroppedFile()` 获取用户拖入窗口的文件路径。根据文件扩展名判断是视频（.mp4/.mkv/.avi/.mov/.webm）还是图片，分别调用 `LoadVideoFromFile()` 或 `LoadImageFromFile()` 加载。扩展名转换使用手动 ASCII 偏移而非 `std::tolower`，避免 locale 依赖。

```cpp
    // ---- 拖放：检查是否有拖入的文件 ----
    if (m_app) {
        std::string dropped = m_app->ConsumeDroppedFile();
        if (!dropped.empty()) {
            // 检查是否为视频文件
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');
            if (dot != std::string::npos) {
                ext = ext.substr(dot);
                // 转换为小写
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    LoadVideoFromFile(dropped);
                } else {
                    StopVideo();
                    LoadImageFromFile(dropped);
                }
            }
        }
    }
```

#### 第四段：视频帧更新

**分析**：如果视频正在播放，以 30fps 的固定间隔（ffmpeg 管道输出帧率）从 `VideoPlayer` 读取新帧。读取成功后通过 `UpdateTexture()` 将像素数据上传到 GPU 纹理，并将视频纹理设为当前输入纹理。视频结束时调用 `StopVideo()` 停止播放。

```cpp
    // ---- 视频播放器：更新帧 ----
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();
        // ffmpeg 管道以固定 30fps 输出（见 StartFFmpegProcess: -r 30）
        double frameInterval = 1.0 / 30.0;
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;  // 将视频帧设为着色器输入
                m_videoLastFrameTime = now;
            } else {
                // 视频结束 — 停止循环
                printf("[EffectDetailScene] Video ended, stopping playback\n");
                StopVideo();
            }
        }
    }
}
```

#### 完整源码

```cpp
void EffectDetailScene::OnUpdate(float dt)
{
    m_time += dt;
    m_frameCount++;

    // 自动测试：在 holdFrames 帧后自动返回
    if (m_autoTestHoldFrames > 0) {
        m_autoTestHoldFrames--;
        if (m_autoTestHoldFrames <= 0) {
            m_wantsExit = true;
            m_returnToCoverFlow = true;
            printf("[EffectDetailScene] Auto-test timer expired, returning to CoverFlow\n");
        }
    }

    // ESC 返回 CoverFlow
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_wantsExit         = true;
        m_returnToCoverFlow = true;
        printf("[EffectDetailScene] ESC pressed, returning to CoverFlow\n");
    }

    // ---- 拖放：检查是否有拖入的文件 ----
    if (m_app) {
        std::string dropped = m_app->ConsumeDroppedFile();
        if (!dropped.empty()) {
            // 检查是否为视频文件
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');
            if (dot != std::string::npos) {
                ext = ext.substr(dot);
                // 转换为小写
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    LoadVideoFromFile(dropped);
                } else {
                    StopVideo();
                    LoadImageFromFile(dropped);
                }
            }
        }
    }

    // ---- 视频播放器：更新帧 ----
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();
        // ffmpeg 管道以固定 30fps 输出
        double frameInterval = 1.0 / 30.0;
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else {
                printf("[EffectDetailScene] Video ended, stopping playback\n");
                StopVideo();
            }
        }
    }
}
```

---

### 4.5 OnRender

#### 第一段：前置检查与 Uniform 同步

**分析**：`OnRender()` 是每帧渲染的入口。首先检查后端和着色器句柄是否有效。然后通过 `m_debugPanel.SetUniformValues()` 将 ImGui 面板中用户修改的参数值同步回 `m_uniformFloats` 和 `m_uniformInts`。由于 DebugPanel 可能缩减数组长度（例如某些参数被隐藏），需要用 `resize()` 恢复到预期长度，用 0.0f 填充缺失值。

```cpp
void EffectDetailScene::OnRender(IRenderBackend* backend)
{
    if (!backend) return;
    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) return;

    // 从调试面板同步 uniform 值
    m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
    
    // 恢复正确的 UBO float 数量（DebugPanel 可能缩减数组）
    if (m_uniformFloats.size() != m_expectedFloatCount) {
        m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
    }
```

#### 第二段：获取视口尺寸与分发渲染

**分析**：从后端获取当前帧缓冲区尺寸，保存到成员变量供 ImGui 绘制使用。然后根据 `m_compareMode` 标志选择渲染路径：对比模式下先渲染效果到 FBO 纹理（`RenderCompareView`），再由 ImGui 绘制分屏；非对比模式下直接全屏渲染效果（`RenderFullscreenEffect`）。

```cpp
    // 获取帧缓冲区尺寸
    int width = 0, height = 0;
    backend->GetFramebufferSize(width, height);
    if (width <= 0 || height <= 0) return;
    m_viewportWidth = width;
    m_viewportHeight = height;

    if (m_compareMode) {
        RenderCompareView(backend);      // 对比模式：渲染到 FBO + ImGui 分屏
    } else {
        RenderFullscreenEffect(backend); // 全屏模式：直接渲染效果
    }
}
```

#### 完整源码

```cpp
void EffectDetailScene::OnRender(IRenderBackend* backend)
{
    if (!backend) return;
    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) return;

    // 从调试面板同步 uniform 值
    m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
    
    // 恢复正确的 UBO float 数量（DebugPanel 可能缩减数组）
    if (m_uniformFloats.size() != m_expectedFloatCount) {
        m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
    }

    // 获取帧缓冲区尺寸
    int width = 0, height = 0;
    backend->GetFramebufferSize(width, height);
    if (width <= 0 || height <= 0) return;
    m_viewportWidth = width;
    m_viewportHeight = height;

    if (m_compareMode) {
        RenderCompareView(backend);
    } else {
        RenderFullscreenEffect(backend);
    }
}
```

---

### 4.6 RenderFullscreenEffect

**分析**：这是最直接的渲染路径——构建 `ShaderParams` 结构体，填入输入纹理、uniform 值、时间、帧计数和视口尺寸，然后调用后端的 `DrawFullscreenQuad()` 执行一次全屏四边形绘制。着色器会对整个屏幕的每个像素执行片段着色器，实现后处理效果。

```cpp
void EffectDetailScene::RenderFullscreenEffect(IRenderBackend* backend)
{
    ShaderParams params;
    params.inputTextures.push_back(m_inputTex);     // 输入纹理（图片或视频帧）
    params.uniformFloats = m_uniformFloats;           // float 参数数组
    params.uniformInts = m_uniformInts;               // int 参数数组
    params.time = m_time;                             // 累计时间
    params.frameCount = m_frameCount;                  // 帧计数
    params.viewportWidth = m_viewportWidth;            // 视口宽度
    params.viewportHeight = m_viewportHeight;          // 视口高度

    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
}
```

---

### 4.7 RenderCompareView

#### 第一段：确保效果纹理存在

**分析**：对比视图需要先将效果渲染到一个离屏 FBO 纹理中，然后在 ImGui 中将原图和效果图分别绘制在分割线的左右两侧。`EnsureEffectTexture()` 使用懒初始化模式——仅在第一次需要时创建与屏幕尺寸匹配的 RGBA8 纹理。

```cpp
void EffectDetailScene::RenderCompareView(IRenderBackend* backend)
{
    // 1. 确保效果纹理 FBO 存在且尺寸匹配
    EnsureEffectTexture();

    if (m_effectTex.id == INVALID_TEXTURE.id) return;
```

#### 第二段：渲染到 FBO 纹理

**分析**：调用 `BeginRenderToTexture()` 将后续渲染输出重定向到 `m_effectTex` 关联的 FBO。然后调用 `RenderFullscreenEffect()` 执行效果渲染（此时渲染结果写入 FBO 而非屏幕）。最后调用 `EndRenderToTexture()` 恢复默认帧缓冲区。实际的分屏显示在 `OnImGui()` 中通过 ImGui DrawList 完成。

```cpp
    // 2. 将效果渲染到 FBO 纹理
    backend->BeginRenderToTexture(m_effectTex);
    RenderFullscreenEffect(backend);
    backend->EndRenderToTexture();

    // 3. ImGui 将在 OnImGui 中处理对比视图的显示
}
```

#### EnsureEffectTexture 辅助方法

**分析**：懒初始化效果纹理。如果已创建则直接返回，否则获取当前帧缓冲区尺寸，创建 RGBA8 格式纹理（初始数据为 nullptr，即未初始化）。此纹理在后端内部会自动关联一个 FBO。

```cpp
void EffectDetailScene::EnsureEffectTexture()
{
    if (m_effectTexCreated || !m_backend) return;
    int w = 0, h = 0;
    m_backend->GetFramebufferSize(w, h);
    if (w <= 0 || h <= 0) return;
    m_effectTex = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    m_effectTexCreated = true;
}
```

#### 完整源码

```cpp
void EffectDetailScene::EnsureEffectTexture()
{
    if (m_effectTexCreated || !m_backend) return;
    int w = 0, h = 0;
    m_backend->GetFramebufferSize(w, h);
    if (w <= 0 || h <= 0) return;
    m_effectTex = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    m_effectTexCreated = true;
}

void EffectDetailScene::RenderCompareView(IRenderBackend* backend)
{
    // 1. 确保效果纹理 FBO 存在且尺寸匹配
    EnsureEffectTexture();

    if (m_effectTex.id == INVALID_TEXTURE.id) return;

    // 2. 将效果渲染到 FBO 纹理
    backend->BeginRenderToTexture(m_effectTex);
    RenderFullscreenEffect(backend);
    backend->EndRenderToTexture();

    // 3. ImGui 将在 OnImGui 中处理对比视图的显示
}
```

---

### 4.8 OnImGui

#### 第一段：快捷键处理

**分析**：TAB 键切换调试面板的显示/隐藏，C 键切换对比模式的开/关。这些快捷键在 `OnImGui()` 而非 `OnUpdate()` 中处理，因为 ImGui 的键盘状态在 `NewFrame()` 之后才可用。

```cpp
void EffectDetailScene::OnImGui()
{
    // TAB 切换调试面板可见性
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_showDebug = !m_showDebug;
    }
    // C 切换对比模式
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_compareMode = !m_compareMode;
    }
```

#### 第二段：对比视图 — 原图与效果图绘制

**分析**：创建一个全屏无边框的 ImGui 窗口作为对比视图的画布。使用 `ImDrawList` 的 `AddImage()` 绘制两张纹理：原图铺满整个显示区域作为底层，效果图只绘制在分割线右侧（通过 UV 映射裁剪）。注意 UV 的 Y 轴翻转（`ImVec2(0,1)` 到 `ImVec2(1,0)`），因为 OpenGL 纹理坐标原点在左下角，而 ImGui 在左上角。

```cpp
    // 获取帧缓冲区尺寸用于定位
    int width = 0, height = 0;
    if (m_backend) {
        m_backend->GetFramebufferSize(width, height);
    }

    // --- 对比模式：滑块 Before/After 叠加视图 ---
    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id && m_backend) {
        void* origImTex = m_backend->GetImTextureID(m_inputTex);
        void* effectImTex = m_backend->GetImTextureID(m_effectTex);

        if (origImTex && effectImTex) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
            ImGui::Begin("##CompareView", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            // 计算显示区域
            ImVec2 winPos   = ImGui::GetWindowPos();
            ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
            ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
            ImVec2 displayMin = ImVec2(winPos.x + regionMin.x, winPos.y + regionMin.y);
            ImVec2 displayMax = ImVec2(winPos.x + regionMax.x, winPos.y + regionMax.y);
            ImVec2 displaySize(displayMax.x - displayMin.x, displayMax.y - displayMin.y);

            ImDrawList* dl = ImGui::GetWindowDrawList();

            // 分割位置在屏幕坐标中的 X 值
            float splitX = displayMin.x + displaySize.x * m_compareSplitPos;

            // 绘制原图作为底层（整个显示区域）
            // GL 纹理原点在左下角，ImGui 期望左上角 → 翻转 Y
            dl->AddImage(origImTex, displayMin, displayMax, ImVec2(0, 1), ImVec2(1, 0));

            // 在分割线右侧绘制效果图（裁剪）
            // UV 映射：效果图左边缘映射到 splitX
            float uvMinX = m_compareSplitPos;
            dl->AddImage(effectImTex,
                ImVec2(splitX, displayMin.y), displayMax,
                ImVec2(uvMinX, 1), ImVec2(1, 0));
```

#### 第三段：分割线与拖拽手柄绘制

**分析**：在分割位置绘制一条 3 像素宽的蓝色竖线，中间放置一个圆形拖拽手柄（蓝色外圈 + 白色内圈），手柄内有左右箭头图标提示用户可以拖动。同时在左上角标注 "Before"、右上角标注 "After"。

```cpp
            // 绘制分割线（蓝色，3px）
            dl->AddLine(ImVec2(splitX, displayMin.y), ImVec2(splitX, displayMax.y),
                        IM_COL32(68, 175, 255, 255), 3.0f);

            // 绘制拖拽手柄（在显示区域高度的中间位置画圆）
            float handleY = (displayMin.y + displayMax.y) * 0.5f;
            dl->AddCircleFilled(ImVec2(splitX, handleY), 14.0f, IM_COL32(68, 175, 255, 255));
            dl->AddCircleFilled(ImVec2(splitX, handleY), 12.0f, IM_COL32(255, 255, 255, 255));

            // 绘制手柄图标（左右箭头）
            dl->AddTriangleFilled(
                ImVec2(splitX - 6, handleY - 3), ImVec2(splitX - 2, handleY), ImVec2(splitX - 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));
            dl->AddTriangleFilled(
                ImVec2(splitX + 6, handleY - 3), ImVec2(splitX + 2, handleY), ImVec2(splitX + 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));

            // 标签：左侧 "Before"，右侧 "After"
            dl->AddText(ImVec2(displayMin.x + 10, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "Before");
            dl->AddText(ImVec2(displayMax.x - 60, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "After");
```

#### 第四段：鼠标拖拽交互

**分析**：实现分割线的鼠标拖拽交互。首先检测鼠标是否靠近分割线（20 像素范围内），如果是则将光标设为东西向调整大小样式。鼠标按下时开始拖拽，拖拽过程中实时更新分割位置（限制在 0.1~0.9 范围内），鼠标释放时结束拖拽。

```cpp
            // 处理分割线拖拽的鼠标交互
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            bool mouseInWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

            // 检测鼠标是否靠近分割线
            bool nearSplit = mouseInWindow &&
                std::abs(mousePos.x - splitX) < 20.0f &&
                mousePos.y >= displayMin.y && mousePos.y <= displayMax.y;

            // 靠近分割线时设置光标为东西向调整大小
            if (nearSplit || m_compareDragging) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            // 处理拖拽开始（鼠标按下）
            if (nearSplit && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_compareDragging = true;
            }

            // 处理拖拽更新
            if (m_compareDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                float newSplit = (mousePos.x - displayMin.x) / displaySize.x;
                m_compareSplitPos = std::clamp(newSplit, 0.1f, 0.9f);
            }

            // 处理拖拽结束
            if (m_compareDragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_compareDragging = false;
            }

            ImGui::End();
        }
    }
```

#### 第五段：底部信息栏

**分析**：在屏幕底部创建一个 60 像素高的信息栏，显示当前特效名称（白色）、ESC 返回提示（灰色）和特效描述（浅灰色）。使用 `LanguageManager` 实现多语言支持。

```cpp
    // --- 底部信息栏 ---
    {
        const float barHeight = 60.0f;
        ImGui::SetNextWindowPos(ImVec2(0, (float)height - barHeight));
        ImGui::SetNextWindowSize(ImVec2((float)width, barHeight));
        ImGui::Begin("##InfoBar", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.9f), "%s", LanguageManager::Instance().CardName(m_card.id));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 0.7f), "%s", LanguageManager::Instance().EscReturn());
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", LanguageManager::Instance().CardDesc(m_card.id));
        ImGui::End();
    }
```

#### 第六段：调试面板与素材库

**分析**：调试面板显示所有着色器参数的滑动条/输入框，用户可实时调整参数值。面板渲染后立即同步 uniform 值，确保下一帧渲染使用最新参数。素材库面板显示 CoverFlow 中保存的图片和视频池，用户可点击切换输入源。

```cpp
    // --- 调试面板 ---
    if (m_showDebug) {
        ImGui::Begin(LanguageManager::Instance().EffectParams(), &m_showDebug);
        m_debugPanel.Render(&m_showDebug);
        ImGui::End();
        // UI 交互后更新 uniform 值
        m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
        if (m_uniformFloats.size() != m_expectedFloatCount) {
            m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
        }
    }

    // --- 素材库面板 ---
    if (m_showDebug && !m_savedState.imagePool.empty()) {
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(LanguageManager::Instance().AssetLibrary(), &m_showDebug)) {
            // 图片区域
            if (ImGui::CollapsingHeader(LanguageManager::Instance().Images())) {
                for (size_t i = 0; i < m_savedState.imagePool.size(); i++) {
                    const std::string& path = m_savedState.imagePool[i];
                    // 提取文件名用于显示
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadImageFromFile(path);
                    }
                }
            }
            // 视频区域
            if (!m_savedState.videoPool.empty() && ImGui::CollapsingHeader(LanguageManager::Instance().Videos())) {
                for (size_t i = 0; i < m_savedState.videoPool.size(); i++) {
                    const std::string& path = m_savedState.videoPool[i];
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadVideoFromFile(path);
                    }
                }
            }
        }
        ImGui::End();
    }
```

#### 完整源码

```cpp
void EffectDetailScene::OnImGui()
{
    // TAB 切换调试面板可见性
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_showDebug = !m_showDebug;
    }
    // C 切换对比模式
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_compareMode = !m_compareMode;
    }

    // 获取帧缓冲区尺寸用于定位
    int width = 0, height = 0;
    if (m_backend) {
        m_backend->GetFramebufferSize(width, height);
    }

    // --- 对比模式：滑块 Before/After 叠加视图 ---
    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id && m_backend) {
        void* origImTex = m_backend->GetImTextureID(m_inputTex);
        void* effectImTex = m_backend->GetImTextureID(m_effectTex);

        if (origImTex && effectImTex) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
            ImGui::Begin("##CompareView", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            // 计算显示区域
            ImVec2 winPos   = ImGui::GetWindowPos();
            ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
            ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
            ImVec2 displayMin = ImVec2(winPos.x + regionMin.x, winPos.y + regionMin.y);
            ImVec2 displayMax = ImVec2(winPos.x + regionMax.x, winPos.y + regionMax.y);
            ImVec2 displaySize(displayMax.x - displayMin.x, displayMax.y - displayMin.y);

            ImDrawList* dl = ImGui::GetWindowDrawList();

            // 分割位置在屏幕坐标中的 X 值
            float splitX = displayMin.x + displaySize.x * m_compareSplitPos;

            // 绘制原图作为底层（整个显示区域）
            // GL 纹理原点在左下角，ImGui 期望左上角 → 翻转 Y
            dl->AddImage(origImTex, displayMin, displayMax, ImVec2(0, 1), ImVec2(1, 0));

            // 在分割线右侧绘制效果图（裁剪）
            float uvMinX = m_compareSplitPos;
            dl->AddImage(effectImTex,
                ImVec2(splitX, displayMin.y), displayMax,
                ImVec2(uvMinX, 1), ImVec2(1, 0));

            // 绘制分割线（蓝色，3px）
            dl->AddLine(ImVec2(splitX, displayMin.y), ImVec2(splitX, displayMax.y),
                        IM_COL32(68, 175, 255, 255), 3.0f);

            // 绘制拖拽手柄（圆形）
            float handleY = (displayMin.y + displayMax.y) * 0.5f;
            dl->AddCircleFilled(ImVec2(splitX, handleY), 14.0f, IM_COL32(68, 175, 255, 255));
            dl->AddCircleFilled(ImVec2(splitX, handleY), 12.0f, IM_COL32(255, 255, 255, 255));

            // 绘制手柄图标（左右箭头）
            dl->AddTriangleFilled(
                ImVec2(splitX - 6, handleY - 3), ImVec2(splitX - 2, handleY), ImVec2(splitX - 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));
            dl->AddTriangleFilled(
                ImVec2(splitX + 6, handleY - 3), ImVec2(splitX + 2, handleY), ImVec2(splitX + 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));

            // 标签：左侧 "Before"，右侧 "After"
            dl->AddText(ImVec2(displayMin.x + 10, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "Before");
            dl->AddText(ImVec2(displayMax.x - 60, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "After");

            // 处理分割线拖拽的鼠标交互
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            bool mouseInWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

            // 检测鼠标是否靠近分割线
            bool nearSplit = mouseInWindow &&
                std::abs(mousePos.x - splitX) < 20.0f &&
                mousePos.y >= displayMin.y && mousePos.y <= displayMax.y;

            // 靠近分割线时设置光标为东西向调整大小
            if (nearSplit || m_compareDragging) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            // 处理拖拽开始（鼠标按下）
            if (nearSplit && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_compareDragging = true;
            }

            // 处理拖拽更新
            if (m_compareDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                float newSplit = (mousePos.x - displayMin.x) / displaySize.x;
                m_compareSplitPos = std::clamp(newSplit, 0.1f, 0.9f);
            }

            // 处理拖拽结束
            if (m_compareDragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_compareDragging = false;
            }

            ImGui::End();
        }
    }

    // --- 底部信息栏 ---
    {
        const float barHeight = 60.0f;
        ImGui::SetNextWindowPos(ImVec2(0, (float)height - barHeight));
        ImGui::SetNextWindowSize(ImVec2((float)width, barHeight));
        ImGui::Begin("##InfoBar", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.9f), "%s", LanguageManager::Instance().CardName(m_card.id));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 0.7f), "%s", LanguageManager::Instance().EscReturn());
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", LanguageManager::Instance().CardDesc(m_card.id));
        ImGui::End();
    }

    // --- 调试面板 ---
    if (m_showDebug) {
        ImGui::Begin(LanguageManager::Instance().EffectParams(), &m_showDebug);
        m_debugPanel.Render(&m_showDebug);
        ImGui::End();
        // UI 交互后更新 uniform 值
        m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
        if (m_uniformFloats.size() != m_expectedFloatCount) {
            m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
        }
    }

    // --- 素材库面板 ---
    if (m_showDebug && !m_savedState.imagePool.empty()) {
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(LanguageManager::Instance().AssetLibrary(), &m_showDebug)) {
            // 图片区域
            if (ImGui::CollapsingHeader(LanguageManager::Instance().Images())) {
                for (size_t i = 0; i < m_savedState.imagePool.size(); i++) {
                    const std::string& path = m_savedState.imagePool[i];
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadImageFromFile(path);
                    }
                }
            }
            // 视频区域
            if (!m_savedState.videoPool.empty() && ImGui::CollapsingHeader(LanguageManager::Instance().Videos())) {
                for (size_t i = 0; i < m_savedState.videoPool.size(); i++) {
                    const std::string& path = m_savedState.videoPool[i];
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadVideoFromFile(path);
                    }
                }
            }
        }
        ImGui::End();
    }
}
```

---

### 4.9 GetNextScene

#### 第一段：重建 CoverFlowScene

**分析**：当 `m_returnToCoverFlow` 为 true 时，创建新的 `CoverFlowScene` 并完整恢复之前保存的状态：输入纹理、纹理缓存、后端指针、应用指针、测试图片目录、选中卡片索引、图片池和自动测试状态。这种"状态快照-恢复"模式确保用户返回 CoverFlow 时看到的状态与离开时完全一致。

```cpp
std::unique_ptr<Scene> EffectDetailScene::GetNextScene()
{
    if (m_returnToCoverFlow) {
        printf("[EffectDetailScene] Restoring CoverFlowScene with full state\n");
        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetInputTexture(m_savedState.inputTex);       // 恢复输入纹理
        coverFlow->SetInputTexCache(m_savedState.inputTexCache);  // 恢复纹理缓存
        coverFlow->SetBackend(m_savedState.backend);              // 恢复后端
        coverFlow->SetApplication(m_savedState.app);              // 恢复应用指针
        // 缩略图由 CoverFlowScene::OnEnter 内部初始化
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);

        // 恢复选中的卡片索引，使用户返回到同一张卡片
        coverFlow->SetSelectedIndex(m_savedState.selectedIndex);

        // 恢复图片池
        for (const auto& img : m_savedState.imagePool) {
            coverFlow->AddImageToPool(img);
        }

        // 恢复自动测试状态
        if (m_savedState.autoTest) {
            coverFlow->ResumeAutoTest(m_savedState.autoTestHoldFrames, m_savedState.autoTestCardIndex);
        }
```

#### 第二段：视频播放器回传

**分析**：如果详情页正在播放视频，将视频播放器所有权转移回 CoverFlowScene。使用 `std::move` 确保唯一所有权语义，避免两个场景同时持有播放器。转移后将本地的 `m_videoActive` 设为 false，防止析构时重复释放。

```cpp
        // 将视频播放器转移回 CoverFlowScene
        if (m_videoActive && m_videoPlayer) {
            coverFlow->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
            m_videoActive = false;
            m_videoTex = {0};
            printf("[EffectDetailScene] Transferred video player back to CoverFlow\n");
        }

        printf("[EffectDetailScene] CoverFlowScene restored (thumbs=%zu, pool=%zu)\n",
               m_savedState.thumbIds.size(), m_savedState.imagePool.size());
        return coverFlow;
    }
    return nullptr;
}
```

#### 完整源码

```cpp
std::unique_ptr<Scene> EffectDetailScene::GetNextScene()
{
    if (m_returnToCoverFlow) {
        printf("[EffectDetailScene] Restoring CoverFlowScene with full state\n");
        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetInputTexture(m_savedState.inputTex);
        coverFlow->SetInputTexCache(m_savedState.inputTexCache);
        coverFlow->SetBackend(m_savedState.backend);
        coverFlow->SetApplication(m_savedState.app);
        // 缩略图由 CoverFlowScene::OnEnter 内部初始化
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);

        // 恢复选中的卡片索引
        coverFlow->SetSelectedIndex(m_savedState.selectedIndex);

        // 恢复图片池
        for (const auto& img : m_savedState.imagePool) {
            coverFlow->AddImageToPool(img);
        }

        // 恢复自动测试状态
        if (m_savedState.autoTest) {
            coverFlow->ResumeAutoTest(m_savedState.autoTestHoldFrames, m_savedState.autoTestCardIndex);
        }

        // 将视频播放器转移回 CoverFlowScene
        if (m_videoActive && m_videoPlayer) {
            coverFlow->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
            m_videoActive = false;
            m_videoTex = {0};
            printf("[EffectDetailScene] Transferred video player back to CoverFlow\n");
        }

        printf("[EffectDetailScene] CoverFlowScene restored (thumbs=%zu, pool=%zu)\n",
               m_savedState.thumbIds.size(), m_savedState.imagePool.size());
        return coverFlow;
    }
    return nullptr;
}
```

---

## 5. OpenGL 渲染后端

`OpenGLBackend` 是 `IRenderBackend` 接口的 OpenGL 4.6 实现，负责所有 GPU 资源管理、着色器编译、全屏后处理渲染、3D 卡片渲染和 ImGui 集成。它使用 SPIR-V 作为着色器中间语言，通过 `glShaderBinary` + `glSpecializeShader` 加载，实现了与 Vulkan 后端的着色器共享。

---

### 5.1 IRenderBackend 接口

#### 接口概览

**分析**：`IRenderBackend` 是纯虚接口（抽象基类），定义了渲染后端必须实现的全部功能。设计上遵循"接口隔离"原则，将功能分为几个逻辑组：

1. **生命周期**：`Init`/`Shutdown`/`BeginFrame`/`EndFrame`/`WaitIdle`
2. **视口管理**：`Resize`/`GetFramebufferSize`
3. **着色器管理**：支持 SPIR-V（`CreateVertexShader`/`CreateFragmentShader`）和 GLSL（`CreateVertexShaderFromGLSL`/`CreateFragmentShaderFromGLSL`）两种路径
4. **纹理管理**：`CreateTexture`/`UpdateTexture`/`DestroyTexture`/`GetImTextureID`
5. **管线对象**：`CreatePipeline`/`DestroyPipeline`/`BindPipeline`（OpenGL 中为兼容性桩实现）
6. **全屏渲染**：`DrawFullscreenQuad`（核心后处理绘制入口）
7. **3D 卡片**：`DrawCards`（封面流 3D 卡片渲染）
8. **帧缓冲操作**：`BlitToScreen`/`BeginRenderToTexture`/`EndRenderToTexture`
9. **ImGui 集成**：`ImGuiInit`/`ImGuiNewFrame`/`ImGuiRender`/`ImGuiShutdown`
10. **查询**：`GetType`/`GetName`/`GetMaxTextureSize`

`PipelineDesc` 结构体描述管线创建参数（着色器句柄 + 尺寸 + 混合开关）。`ShaderParams` 结构体封装每次全屏绘制所需的全部数据：输入纹理数组、uniform float/int 数组、视口尺寸、时间和帧计数。`CardDrawInfo` 结构体描述单张 3D 卡片的变换参数（位置、缩放、旋转、不透明度）。

```cpp
#pragma once

#include "BackendType.h"

#include <cstdint>
#include <vector>
#include <string>

// 前向声明
struct GLFWwindow;

// 管线描述（用于创建管线）
struct PipelineDesc {
    ShaderHandle vertShader;   // 顶点着色器句柄
    ShaderHandle fragShader;   // 片段着色器句柄
    int width;                 // 宽度
    int height;                // 高度
    bool blendEnable = true;   // 是否启用混合
};

// ---------------------------------------------------------------------------
// ShaderParams — 每次全屏四边形绘制时传入的数据
// ---------------------------------------------------------------------------
struct ShaderParams {
    std::vector<TextureHandle> inputTextures;  // 输入纹理数组（最多 8 张）
    std::vector<float>         uniformFloats;  // float 类型 uniform 值
    std::vector<int32_t>       uniformInts;    // int 类型 uniform 值
    int   viewportWidth  = 1280;               // 视口宽度
    int   viewportHeight = 720;                // 视口高度
    float time           = 0.0f;               // 累计时间
    uint32_t frameCount  = 0;                   // 帧计数
};

// ---------------------------------------------------------------------------
// IRenderBackend — 纯虚渲染后端接口
// ---------------------------------------------------------------------------
class IRenderBackend {
public:
    // ---- 嵌套结构体：卡片绘制信息 ----
    struct CardDrawInfo {
        TextureHandle texture;     // 卡片纹理
        float posX, posY, posZ;    // 位置
        float scaleX, scaleY;     // 缩放
        float rotationY;          // Y 轴旋转角度
        float opacity;            // 不透明度
    };

    virtual ~IRenderBackend() = default;

    // ---- 生命周期 ----
    virtual bool Init(GLFWwindow* window) = 0;    // 初始化后端
    virtual void Shutdown()               = 0;    // 关闭后端
    virtual void BeginFrame()             = 0;    // 帧开始
    virtual void EndFrame()               = 0;    // 帧结束（交换缓冲区）
    virtual void WaitIdle()               = 0;    // 等待 GPU 空闲

    // ---- 视口 ----
    virtual void Resize(int width, int height)              = 0;
    virtual void GetFramebufferSize(int& width, int& height) = 0;

    // ---- 着色器 ----
    virtual ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size)   = 0;
    virtual ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) = 0;
    virtual ShaderHandle CreateVertexShaderFromGLSL(const std::string& source)    = 0;
    virtual ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source)  = 0;
    virtual void         DestroyShader(ShaderHandle handle)                        = 0;

    // ---- 纹理 ----
    virtual TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) = 0;
    virtual void          UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) = 0;
    virtual void          DestroyTexture(TextureHandle handle)                                        = 0;
    virtual void*         GetImTextureID(TextureHandle handle)                                        = 0;

    // ---- 管线 ----
    virtual PipelineHandle CreatePipeline(const PipelineDesc& desc) = 0;
    virtual void           DestroyPipeline(PipelineHandle handle)  = 0;
    virtual void           BindPipeline(PipelineHandle handle)     = 0;

    // ---- 全屏四边形 ----
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) = 0;

    // ---- 3D 卡片渲染 ----
    virtual void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) = 0;

    // ---- Blit ----
    virtual void BlitToScreen(TextureHandle src) = 0;

    // ---- 渲染目标 ----
    virtual void BeginRenderToTexture(TextureHandle target) = 0;
    virtual void EndRenderToTexture()                       = 0;

    // ---- 工具 ----
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void Clear(float r, float g, float b, float a)        = 0;

    // ---- ImGui ----
    virtual void ImGuiInit(GLFWwindow* window) = 0;
    virtual void ImGuiNewFrame()               = 0;
    virtual void ImGuiRender()                 = 0;
    virtual void ImGuiShutdown()               = 0;

    // ---- 查询 ----
    virtual BackendType GetType()                     const = 0;
    virtual const char* GetName()                     const = 0;
    virtual int         GetMaxTextureSize()           const = 0;
};
```

---

### 5.2 OpenGLBackend 头文件

#### 类声明概览

**分析**：`OpenGLBackend` 继承 `IRenderBackend`，声明了所有接口方法的 override 实现。私有成员可分为：

1. **GL 资源池**：`m_shaders`（着色器 ID → GL 着色器对象映射）、`m_programCache`（vs+fs 组合 → GL 程序缓存）、`m_textures`（纹理 ID → GL 纹理对象映射）、`m_framebuffers`（纹理 ID → GL FBO 映射）
2. **自增 ID 计数器**：`m_nextShaderId`、`m_nextTextureId`，用于生成不重复的句柄 ID
3. **纹理元数据**：`m_textureFormats`、`m_texWidths`、`m_texHeights`，记录每个纹理的格式和尺寸
4. **全屏四边形几何**：`m_quadVAO`、`m_quadVBO`（顶点数组对象和缓冲区）
5. **临时 UBO**：`m_tempUBO`（每帧绘制的 uniform 缓冲区对象）
6. **帧缓冲区状态**：`m_defaultFBO`（GLFW 创建的默认 FBO）、`m_currentFBO`（当前绑定的 FBO）

私有辅助方法包括句柄转换（`GetGLShader`/`GetGLTexture`/`GetGLFramebuffer`）、程序缓存查找（`GetOrCreateProgram`）、格式转换（`GLInternalFormat`/`GLFormat`/`GLType`）和几何体初始化（`SetupQuadVAO`/`BindDefaultState`）。

```cpp
#pragma once

#include "IRenderBackend.h"

#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>

class OpenGLBackend : public IRenderBackend {
public:
    OpenGLBackend()  = default;
    ~OpenGLBackend() override = default;

    // ---- 生命周期 ----
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // ---- 视口 ----
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) override;

    // ---- 着色器 ----
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    /// 从 GLSL 源码创建着色器（SPIR-V UBO 查询失败时的回退路径）
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& glslSource) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& glslSource) override;
    void         DestroyShader(ShaderHandle handle) override;

    // ---- 纹理 ----
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void          UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void          DestroyTexture(TextureHandle handle) override;
    void*         GetImTextureID(TextureHandle handle) override;

    // ---- 管线 ----
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void           DestroyPipeline(PipelineHandle handle) override;
    void           BindPipeline(PipelineHandle handle) override;

    /// 保存当前帧缓冲区到 PPM 文件用于调试
    void SaveScreenshot(const char* path) const;

    // ---- 全屏四边形 ----
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;

    // ---- 3D 卡片渲染 ----
    void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) override;

    // ---- Blit ----
    void BlitToScreen(TextureHandle src) override;

    // ---- 渲染目标 ----
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // ---- 工具 ----
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;

    // ---- ImGui ----
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // ---- 查询 ----
    BackendType GetType()           const override { return BackendType::OpenGL; }
    const char* GetName()           const override { return "OpenGL 4.6 (SPIR-V)"; }
    int         GetMaxTextureSize() const override;

private:
    // ---- 内部辅助方法 ----
    GLuint GetGLShader(ShaderHandle handle) const;       // 句柄 → GL 着色器对象
    GLuint GetGLTexture(TextureHandle handle) const;     // 句柄 → GL 纹理对象
    GLuint GetGLFramebuffer(TextureHandle textureHandle) const;  // 句柄 → GL FBO

    /// 获取或创建链接的 GL 程序（vs+fs 组合）。带缓存。
    GLuint GetOrCreateProgram(GLuint vsKey, GLuint fsKey, GLuint vsGL, GLuint fsGL);

    static GLint  GLInternalFormat(TextureFormat fmt);  // 格式 → GL 内部格式
    static GLenum GLFormat(TextureFormat fmt);          // 格式 → GL 数据格式
    static GLenum GLType(TextureFormat fmt);             // 格式 → GL 数据类型

    void SetupQuadVAO();       // 初始化全屏四边形 VAO
    void BindDefaultState();   // 设置默认 OpenGL 状态

    // ---- 成员变量 ----
    GLFWwindow* m_window     = nullptr;   // GLFW 窗口句柄
    int         m_width      = 0;         // 帧缓冲区宽度
    int         m_height     = 0;         // 帧缓冲区高度

    // 着色器池
    std::unordered_map<uint32_t, GLuint> m_shaders;     // ID → GL 着色器对象
    uint32_t m_nextShaderId = 1;                         // 下一个着色器 ID

    // 程序缓存（key = vs_id << 32 | fs_id）
    std::unordered_map<uint64_t, GLuint> m_programCache;  // 组合键 → GL 程序

    // 纹理池
    std::unordered_map<uint32_t, GLuint> m_textures;       // ID → GL 纹理对象
    std::unordered_map<uint32_t, TextureFormat> m_textureFormats;  // ID → 格式
    std::unordered_map<uint32_t, int> m_texWidths;         // ID → 宽度
    std::unordered_map<uint32_t, int> m_texHeights;        // ID → 高度
    uint32_t m_nextTextureId = 1;                           // 下一个纹理 ID

    // 帧缓冲区池（以纹理 ID 为键）
    std::unordered_map<uint32_t, GLuint> m_framebuffers;  // 纹理 ID → GL FBO

    // 临时 UBO（用于每帧绘制的 uniform 数据）
    GLuint m_tempUBO = 0;

    // 全屏四边形
    GLuint m_quadVAO = 0;  // 顶点数组对象
    GLuint m_quadVBO = 0;  // 顶点缓冲区对象

    // 默认帧缓冲区
    GLuint m_defaultFBO = 0;  // GLFW 创建的默认 FBO
    GLuint m_currentFBO  = 0; // 当前绑定的 FBO
};
```

---

### 5.3 Init

#### 第一段：上下文初始化与 GL 函数加载

**分析**：`Init()` 是后端初始化的入口。首先保存 GLFW 窗口句柄并设为当前 OpenGL 上下文。然后通过 `LoadGL46Functions()`（glad 生成的加载器）加载所有 OpenGL 4.6 函数指针。如果加载失败，整个后端无法工作。

```cpp
bool OpenGLBackend::Init(GLFWwindow* window) {
    m_window = window;
    if (!m_window) {
        fprintf(stderr, "[OpenGL] Invalid window handle\n");
        return false;
    }

    // 设为当前 OpenGL 上下文
    glfwMakeContextCurrent(m_window);

    // 通过 glad 加载 OpenGL 函数指针
    if (!LoadGL46Functions(m_window)) {
        fprintf(stderr, "[OpenGL] Failed to initialize GLAD\n");
        return false;
    }
```

#### 第二段：帧缓冲区与 UBO 初始化

**分析**：获取 GLFW 窗口的初始帧缓冲区尺寸。通过 `glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING)` 读取当前绑定的 FBO ID 作为默认 FBO（GLFW 可能创建非零的默认 FBO，例如在 macOS 上）。创建临时 UBO（Uniform Buffer Object）用于每帧绘制时传递 uniform 数据，大小为 `UniformData` 结构体（48 字节），使用 `GL_DYNAMIC_DRAW` 提示频繁更新。

```cpp
    // 获取初始帧缓冲区尺寸
    glfwGetFramebufferSize(m_window, &m_width, &m_height);

    // 获取 GLFW 创建的默认 FBO
    GLint defaultFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFBO);
    m_defaultFBO = static_cast<GLuint>(defaultFBO);
    m_currentFBO = m_defaultFBO;

    // 创建临时 UBO 用于每帧绘制的 uniform 数据（回退路径）
    glGenBuffers(1, &m_tempUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
```

#### 第三段：几何体初始化与默认状态

**分析**：调用 `SetupQuadVAO()` 创建全屏四边形的顶点数组对象（包含位置和 UV 属性的交错顶点数据）。调用 `BindDefaultState()` 设置 OpenGL 的默认渲染状态（深度测试、混合、面剔除等）。最后打印 GL 版本信息确认初始化成功。

```cpp
    // 设置全屏四边形 VAO/VBO
    SetupQuadVAO();

    // 设置默认 OpenGL 状态
    BindDefaultState();

    printf("[OpenGL] Initialized - GL Version: %s\n", (const char*)glGetString(GL_VERSION));
    return true;
}
```

#### 完整源码

```cpp
bool OpenGLBackend::Init(GLFWwindow* window) {
    m_window = window;
    if (!m_window) {
        fprintf(stderr, "[OpenGL] Invalid window handle\n");
        return false;
    }

    // 设为当前 OpenGL 上下文
    glfwMakeContextCurrent(m_window);

    // 通过 glad 加载 OpenGL 4.6 函数指针
    if (!LoadGL46Functions(m_window)) {
        fprintf(stderr, "[OpenGL] Failed to initialize GLAD\n");
        return false;
    }

    // 获取初始帧缓冲区尺寸
    glfwGetFramebufferSize(m_window, &m_width, &m_height);

    // 获取 GLFW 创建的默认 FBO
    GLint defaultFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFBO);
    m_defaultFBO = static_cast<GLuint>(defaultFBO);
    m_currentFBO = m_defaultFBO;

    // 创建临时 UBO 用于每帧绘制的 uniform 数据
    glGenBuffers(1, &m_tempUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_tempUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // 设置全屏四边形 VAO/VBO
    SetupQuadVAO();

    // 设置默认 OpenGL 状态
    BindDefaultState();

    printf("[OpenGL] Initialized - GL Version: %s\n", (const char*)glGetString(GL_VERSION));
    return true;
}
```

---

### 5.4 CreateTexture

#### 第一段：纹理对象创建与参数设置

**分析**：通过 `glGenTextures` 生成 GL 纹理对象，绑定到 `GL_TEXTURE_2D` 目标。设置纹理环绕模式为 `GL_CLAMP_TO_EDGE`（防止边缘采样产生接缝），过滤模式为 `GL_LINEAR`（双线性过滤，保证缩放质量）。

```cpp
TextureHandle OpenGLBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    GLuint texture;
    glGenTextures(1, &texture);
    if (texture == 0) {
        fprintf(stderr, "[OpenGL] Failed to create texture\n");
        return INVALID_TEXTURE;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

#### 第二段：纹理存储分配与元数据记录

**分析**：通过三个静态辅助方法（`GLInternalFormat`/`GLFormat`/`GLType`）将抽象的 `TextureFormat` 枚举转换为 OpenGL 具体的格式常量。调用 `glTexImage2D` 一次性分配存储并上传初始数据（`data` 可为 nullptr，表示仅分配不初始化）。最后生成唯一 ID，将 GL 纹理对象和元数据存入对应的池中。

```cpp
    // 分配纹理存储
    GLint internalFormat = GLInternalFormat(format);
    GLenum glFormat = GLFormat(format);
    GLenum glType = GLType(format);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, glType, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t id = m_nextTextureId++;
    m_textures[id] = texture;          // 保存 GL 纹理对象
    m_textureFormats[id] = format;     // 保存格式
    m_texWidths[id] = width;           // 保存宽度
    m_texHeights[id] = height;         // 保存高度

    return TextureHandle{id};
}
```

#### 完整源码

```cpp
TextureHandle OpenGLBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    GLuint texture;
    glGenTextures(1, &texture);
    if (texture == 0) {
        fprintf(stderr, "[OpenGL] Failed to create texture\n");
        return INVALID_TEXTURE;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 分配纹理存储
    GLint internalFormat = GLInternalFormat(format);
    GLenum glFormat = GLFormat(format);
    GLenum glType = GLType(format);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, glType, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t id = m_nextTextureId++;
    m_textures[id] = texture;
    m_textureFormats[id] = format;
    m_texWidths[id] = width;
    m_texHeights[id] = height;

    return TextureHandle{id};
}
```

---

### 5.5 着色器加载

#### 第一段：SPIR-V 着色器创建（以顶点着色器为例）

**分析**：OpenGL 4.6 支持 `GL_ARB_gl_spirv` 扩展（核心特性），允许直接加载 SPIR-V 二进制。流程为：创建空着色器对象 → `glShaderBinary` 加载 SPIR-V 二进制 → `glSpecializeShader` 指定入口点 "main" → 检查编译状态。这种路径避免了 GLSL 编译器的开销，且与 Vulkan 后端共享同一份着色器二进制。片段着色器的创建流程完全相同，仅 `GL_VERTEX_SHADER` 替换为 `GL_FRAGMENT_SHADER`。

```cpp
ShaderHandle OpenGLBackend::CreateVertexShader(const uint32_t* spirv, size_t size) {
    if (spirv == nullptr || size == 0) {
        fprintf(stderr, "[OpenGL] Empty SPIR-V data for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    // 加载 SPIR-V 二进制
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv, static_cast<GLsizei>(size * sizeof(uint32_t)));

    // 以默认入口点 "main" 进行特化
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    // 检查编译状态
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader SPIR-V specialization failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}
```

#### 第二段：GLSL 着色器创建（回退路径）

**分析**：当 SPIR-V 路径不可用时（例如某些旧驱动），提供 GLSL 源码编译的回退路径。使用传统的 `glShaderSource` + `glCompileShader` 流程。此路径在当前项目中主要作为调试手段，生产环境统一使用 SPIR-V。

```cpp
ShaderHandle OpenGLBackend::CreateVertexShaderFromGLSL(const std::string& glslSource) {
    if (glslSource.empty()) {
        fprintf(stderr, "[OpenGL] Empty GLSL source for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    const char* source = glslSource.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}
```

#### 第三段：DestroyShader — 级联清理

**分析**：销毁着色器时不仅删除 GL 着色器对象，还必须清理程序缓存中所有引用该着色器的程序。遍历 `m_programCache`，解析组合键（高 32 位为 vs ID，低 32 位为 fs ID），找到匹配的程序后删除并从缓存中移除。这确保了着色器销毁后不会留下悬空引用。

```cpp
void OpenGLBackend::DestroyShader(ShaderHandle handle) {
    if (handle.id == 0) return;

    auto it = m_shaders.find(handle.id);
    if (it != m_shaders.end()) {
        // 移除所有使用此着色器的缓存程序
        std::vector<uint64_t> keysToRemove;
        for (auto& [key, program] : m_programCache) {
            uint32_t vsId = static_cast<uint32_t>(key >> 32);
            uint32_t fsId = static_cast<uint32_t>(key & 0xFFFFFFFF);
            if (vsId == handle.id || fsId == handle.id) {
                glDeleteProgram(program);
                keysToRemove.push_back(key);
            }
        }
        for (auto key : keysToRemove) {
            m_programCache.erase(key);
        }

        glDeleteShader(it->second);  // 删除 GL 着色器对象
        m_shaders.erase(it);        // 从池中移除
    }
}
```

#### 完整源码（顶点着色器 SPIR-V 创建 + 销毁）

```cpp
ShaderHandle OpenGLBackend::CreateVertexShader(const uint32_t* spirv, size_t size) {
    if (spirv == nullptr || size == 0) {
        fprintf(stderr, "[OpenGL] Empty SPIR-V data for vertex shader\n");
        return INVALID_SHADER;
    }

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    if (shader == 0) {
        fprintf(stderr, "[OpenGL] Failed to create vertex shader object\n");
        return INVALID_SHADER;
    }

    // 加载 SPIR-V 二进制
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv, static_cast<GLsizei>(size * sizeof(uint32_t)));

    // 以默认入口点 "main" 进行特化
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    // 检查编译状态
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "[OpenGL] Vertex shader SPIR-V specialization failed: %s\n", infoLog);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = m_nextShaderId++;
    m_shaders[id] = shader;
    return ShaderHandle{id};
}

void OpenGLBackend::DestroyShader(ShaderHandle handle) {
    if (handle.id == 0) return;

    auto it = m_shaders.find(handle.id);
    if (it != m_shaders.end()) {
        // 移除所有使用此着色器的缓存程序
        std::vector<uint64_t> keysToRemove;
        for (auto& [key, program] : m_programCache) {
            uint32_t vsId = static_cast<uint32_t>(key >> 32);
            uint32_t fsId = static_cast<uint32_t>(key & 0xFFFFFFFF);
            if (vsId == handle.id || fsId == handle.id) {
                glDeleteProgram(program);
                keysToRemove.push_back(key);
            }
        }
        for (auto key : keysToRemove) {
            m_programCache.erase(key);
        }

        glDeleteShader(it->second);
        m_shaders.erase(it);
    }
}
```

---

### 5.6 CreatePipeline

**分析**：OpenGL 没有像 Vulkan 那样的管线状态对象（Pipeline State Object）。`CreatePipeline` 仅将顶点和片段着色器 ID 打包为一个 64 位句柄返回（高 16 位为 vs ID，低 16 位为 fs ID）。实际的 GL 程序创建在 `DrawFullscreenQuad` 中延迟进行。`DestroyPipeline` 和 `BindPipeline` 同样是兼容性桩实现——程序的生命周期由着色器销毁和缓存管理。

```cpp
PipelineHandle OpenGLBackend::CreatePipeline(const PipelineDesc& desc) {
    // OpenGL 没有管线对象，仅将着色器 ID 打包为句柄
    // 实际程序在 DrawFullscreenQuad 中延迟创建
    uint32_t id = (desc.vertShader.id << 16) | desc.fragShader.id;
    return PipelineHandle{id};
}

void OpenGLBackend::DestroyPipeline(PipelineHandle handle) {
    // OpenGL 中无需销毁——程序由缓存管理，在着色器销毁时清理
    (void)handle;
}

void OpenGLBackend::BindPipeline(PipelineHandle handle) {
    // 从管线句柄提取着色器 ID
    uint32_t vsId = handle.id >> 16;
    uint32_t fsId = handle.id & 0xFFFF;

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

---

### 5.7 DrawFullscreenQuad — 核心渲染流程

#### 第一段：获取或创建 GL Program

**分析**：OpenGL 没有管线对象概念，每次绘制需要绑定一个链接好的着色器程序。`GetOrCreateProgram` 使用 `m_programCache` 缓存已链接的程序，键为 `(vs_id << 32) | fs_id`。首次遇到某对 vs+fs 组合时，创建程序、链接、绑定 UBO 块，然后缓存。后续调用直接命中缓存，避免重复链接开销。

```cpp
void OpenGLBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    GLuint vs = GetGLShader(vert);
    GLuint fs = GetGLShader(frag);

    if (vs == 0 || fs == 0) {
        fprintf(stderr, "[OpenGL] Invalid shader handles in DrawFullscreenQuad vs=%u fs=%u\n", vs, fs);
        return;
    }

    // 使用自增 ID 作为程序缓存键（匹配 DestroyShader 的清理逻辑），
    // 使用 GL 着色器对象 ID 进行 glAttachShader
    GLuint program = GetOrCreateProgram(vert.id, frag.id, vs, fs);
    if (program == 0) {
        fprintf(stderr, "[OpenGL] Failed to create shader program\n");
        return;
    }

    glUseProgram(program);
```

#### 第二段：重置 GL 状态

**分析**：在绘制前必须重置所有可能被 ImGui 修改的 GL 状态。ImGui 的渲染会修改深度测试、裁剪测试、模板测试、面剔除等状态。如果不清除，后处理着色器可能在不正确的状态下执行，导致渲染异常。这是 OpenGL 立即模式 API 的典型痛点——全局状态机需要手动管理。

```cpp
    // 重置所有 ImGui 可能修改的 GL 状态
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
    glViewport(0, 0, m_width, m_height);
```

#### 第三段：绑定输入纹理

**分析**：遍历 `params.inputTextures`（最多 8 张纹理），将每张纹理绑定到对应的纹理单元（`GL_TEXTURE0` 到 `GL_TEXTURE7`）。对于每个纹理单元，尝试设置采样器 uniform：先尝试 `uTexture0`~`uTexture7` 的命名约定，对于第一张纹理还额外尝试 `uInputTex`（兼容单纹理着色器）。

```cpp
    // 绑定输入纹理（最多 8 张）
    for (size_t i = 0; i < params.inputTextures.size() && i < 8; ++i) {
        GLuint tex = GetGLTexture(params.inputTextures[i]);
        if (tex != 0) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
            glBindTexture(GL_TEXTURE_2D, tex);

            // 尝试设置采样器 uniform
            char uniformName[32];
            snprintf(uniformName, sizeof(uniformName), "uTexture%d", static_cast<int>(i));
            GLint loc = glGetUniformLocation(program, uniformName);
            if (loc >= 0) {
                glUniform1i(loc, static_cast<GLint>(i));
            }

            // 对单纹理着色器也尝试 "uInputTex"
            if (i == 0) {
                loc = glGetUniformLocation(program, "uInputTex");
                if (loc >= 0) {
                    glUniform1i(loc, 0);
                }
            }
        }
    }
```

#### 第四段：填充 Uniform 数据

**分析**：将 `ShaderParams` 中的 uniform 值填入 `UniformData` 结构体（48 字节，std140 布局）。最多支持 6 个 float 参数（`uParamFloat0`~`uParamFloat5`），加上分辨率（vec2）、时间（float）和帧计数（float）。填充完成后检查程序是否有 UBO 块。

```cpp
    // 填充 uniform 数据结构
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
```

#### 第五段：UBO 路径 vs 独立 Uniform 路径

**分析**：这是整个渲染函数最关键的分叉点。通过 `glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb)` 检测程序是否有 UBO 块。

- **UBO 路径**（SPIR-V 着色器）：将 uniform 数据打包为 48 字节的原始缓冲区，通过 `glBindBufferBase` 绑定到 binding point 1。SPIR-V 着色器通过 `layout(binding=1) uniform Params { ... }` 声明 UBO 块。
- **独立 Uniform 路径**（GLSL 着色器）：使用 `glGetUniformLocation` 逐个设置 uniform。先尝试裸名称（如 `uParamFloat0`），再尝试带 `Params.` 前缀的名称（兼容命名块 uniform）。

```cpp
    // ---- 检查程序是否有 UBO ----
    GLint nb=0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);

    if (nb > 0) {
        // ---- UBO 路径（SPIR-V 或 NVIDIA 预编译） ----
        // 确保 "Params" uniform 块绑定到 binding point 1
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
        // ---- 独立 uniform 路径（GLSL） ----
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0); // 解绑上一帧的 UBO
        GLint loc;

        // 辅助 lambda：先尝试裸名称，再尝试 Params. 前缀
        auto getLoc = [&](const char* name) -> GLint {
            GLint l = glGetUniformLocation(program, name);
            if (l >= 0) return l;
            char buf[64];
            snprintf(buf, sizeof(buf), "Params.%s", name);
            return glGetUniformLocation(program, buf);
        };

        if (params.uniformFloats.size() > 0) {
            loc = getLoc("uParamFloat0");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[0]);
        }
        // ... (uParamFloat1~5 类似)
        loc = getLoc("uResolution");
        if (loc >= 0) glUniform2f(loc, (float)params.viewportWidth, (float)params.viewportHeight);
        loc = getLoc("uTime");
        if (loc >= 0) glUniform1f(loc, params.time);
        loc = getLoc("uFrameCount");
        if (loc >= 0) glUniform1ui(loc, params.frameCount);
    }
```

#### 第六段：绘制与清理

**分析**：绑定全屏四边形 VAO，使用 `glDrawElements` 绘制两个三角形（6 个索引）。绘制完成后解绑所有纹理单元、重置活动纹理单元为 `GL_TEXTURE0`、解绑程序。这种"绘制后清理"模式确保不会影响后续的 ImGui 渲染。

```cpp
    // 绘制全屏四边形
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, kFullscreenQuadVertexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // 清理：解绑所有纹理单元并重置状态
    for (size_t i = 0; i < 8; ++i) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

#### 完整源码

```cpp
void OpenGLBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    GLuint vs = GetGLShader(vert);
    GLuint fs = GetGLShader(frag);

    if (vs == 0 || fs == 0) {
        fprintf(stderr, "[OpenGL] Invalid shader handles in DrawFullscreenQuad vs=%u fs=%u\n", vs, fs);
        return;
    }

    // 使用自增 ID 作为程序缓存键，GL 着色器对象 ID 用于 glAttachShader
    GLuint program = GetOrCreateProgram(vert.id, frag.id, vs, fs);
    if (program == 0) {
        fprintf(stderr, "[OpenGL] Failed to create shader program\n");
        return;
    }

    glUseProgram(program);

    // 重置所有 ImGui 可能修改的 GL 状态
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
    glViewport(0, 0, m_width, m_height);

    // 绑定输入纹理（最多 8 张）
    for (size_t i = 0; i < params.inputTextures.size() && i < 8; ++i) {
        GLuint tex = GetGLTexture(params.inputTextures[i]);
        if (tex != 0) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
            glBindTexture(GL_TEXTURE_2D, tex);

            // 尝试设置采样器 uniform
            char uniformName[32];
            snprintf(uniformName, sizeof(uniformName), "uTexture%d", static_cast<int>(i));
            GLint loc = glGetUniformLocation(program, uniformName);
            if (loc >= 0) {
                glUniform1i(loc, static_cast<GLint>(i));
            }

            // 对单纹理着色器也尝试 "uInputTex"
            if (i == 0) {
                loc = glGetUniformLocation(program, "uInputTex");
                if (loc >= 0) {
                    glUniform1i(loc, 0);
                }
            }
        }
    }

    // 填充 uniform 数据结构
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

    // ---- 检查程序是否有 UBO ----
    GLint nb=0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &nb);

    if (nb > 0) {
        // ---- UBO 路径（SPIR-V 或 NVIDIA 预编译） ----
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
        // ---- 独立 uniform 路径（GLSL） ----
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0); // 解绑上一帧的 UBO
        GLint loc;

        // 辅助 lambda：先尝试裸名称，再尝试 Params. 前缀
        auto getLoc = [&](const char* name) -> GLint {
            GLint l = glGetUniformLocation(program, name);
            if (l >= 0) return l;
            char buf[64];
            snprintf(buf, sizeof(buf), "Params.%s", name);
            return glGetUniformLocation(program, buf);
        };

        if (params.uniformFloats.size() > 0) {
            loc = getLoc("uParamFloat0");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[0]);
        }
        if (params.uniformFloats.size() > 1) {
            loc = getLoc("uParamFloat1");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[1]);
        }
        if (params.uniformFloats.size() > 2) {
            loc = getLoc("uParamFloat2");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[2]);
        }
        if (params.uniformFloats.size() > 3) {
            loc = getLoc("uParamFloat3");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[3]);
        }
        if (params.uniformFloats.size() > 4) {
            loc = getLoc("uParamFloat4");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[4]);
        }
        if (params.uniformFloats.size() > 5) {
            loc = getLoc("uParamFloat5");
            if (loc >= 0) glUniform1f(loc, params.uniformFloats[5]);
        }
        loc = getLoc("uResolution");
        if (loc >= 0) glUniform2f(loc, (float)params.viewportWidth, (float)params.viewportHeight);
        loc = getLoc("uTime");
        if (loc >= 0) glUniform1f(loc, params.time);
        loc = getLoc("uFrameCount");
        if (loc >= 0) glUniform1ui(loc, params.frameCount);
    }

    // 绘制全屏四边形
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, kFullscreenQuadVertexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // 清理：解绑所有纹理单元并重置状态
    for (size_t i = 0; i < 8; ++i) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

---

### 5.8 FBO 渲染

#### BeginRenderToTexture

**分析**：将渲染输出重定向到指定纹理关联的 FBO。`GetGLFramebuffer` 使用懒初始化——如果该纹理还没有关联的 FBO，则创建一个新的 FBO 并将纹理附加为颜色附件 0。绑定 FBO 后，将视口设置为纹理的实际尺寸（而非窗口尺寸），确保渲染分辨率匹配。

```cpp
void OpenGLBackend::BeginRenderToTexture(TextureHandle target) {
    GLuint fbo = GetGLFramebuffer(target);
    if (fbo == 0) {
        fprintf(stderr, "[OpenGL] Failed to get/create framebuffer for texture\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    m_currentFBO = fbo;

    // 获取纹理尺寸用于设置视口
    auto wIt = m_texWidths.find(target.id);
    auto hIt = m_texHeights.find(target.id);
    if (wIt != m_texWidths.end() && hIt != m_texHeights.end()) {
        glViewport(0, 0, wIt->second, hIt->second);
    }
}
```

#### EndRenderToTexture

**分析**：恢复默认帧缓冲区绑定，将 `m_currentFBO` 设回默认值，视口恢复为窗口尺寸。此后所有渲染输出将直接显示到屏幕。

```cpp
void OpenGLBackend::EndRenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    m_currentFBO = m_defaultFBO;
    glViewport(0, 0, m_width, m_height);
}
```

#### GetGLFramebuffer — FBO 懒创建

**分析**：这是 FBO 管理的核心方法。首先在缓存中查找，命中则直接返回。未命中时创建新 FBO，将纹理附加为 `GL_COLOR_ATTACHMENT0`，检查完整性状态。注意此方法声明为 `const`，但需要修改 `m_framebuffers` 缓存——通过 `const_cast` 绕过，这是 OpenGL 后端中常见的实用主义做法。

```cpp
GLuint OpenGLBackend::GetGLFramebuffer(TextureHandle textureHandle) const {
    if (textureHandle.id == 0) return 0;

    // 检查帧缓冲区是否已存在
    auto it = m_framebuffers.find(textureHandle.id);
    if (it != m_framebuffers.end()) {
        return it->second;
    }

    // 创建新的帧缓冲区
    GLuint tex = GetGLTexture(textureHandle);
    if (tex == 0) return 0;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    if (fbo == 0) return 0;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[OpenGL] Framebuffer incomplete: 0x%x\n", status);
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);

    // 存入缓存（const 方法需要修改缓存）
    const_cast<OpenGLBackend*>(this)->m_framebuffers[textureHandle.id] = fbo;

    return fbo;
}
```

#### 完整源码

```cpp
GLuint OpenGLBackend::GetGLFramebuffer(TextureHandle textureHandle) const {
    if (textureHandle.id == 0) return 0;

    // 检查帧缓冲区是否已存在
    auto it = m_framebuffers.find(textureHandle.id);
    if (it != m_framebuffers.end()) {
        return it->second;
    }

    // 创建新的帧缓冲区
    GLuint tex = GetGLTexture(textureHandle);
    if (tex == 0) return 0;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    if (fbo == 0) return 0;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[OpenGL] Framebuffer incomplete: 0x%x\n", status);
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);

    // 存入缓存（const 方法需要修改缓存）
    const_cast<OpenGLBackend*>(this)->m_framebuffers[textureHandle.id] = fbo;

    return fbo;
}

void OpenGLBackend::BeginRenderToTexture(TextureHandle target) {
    GLuint fbo = GetGLFramebuffer(target);
    if (fbo == 0) {
        fprintf(stderr, "[OpenGL] Failed to get/create framebuffer for texture\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    m_currentFBO = fbo;

    // 获取纹理尺寸用于设置视口
    auto wIt = m_texWidths.find(target.id);
    auto hIt = m_texHeights.find(target.id);
    if (wIt != m_texWidths.end() && hIt != m_texHeights.end()) {
        glViewport(0, 0, wIt->second, hIt->second);
    }
}

void OpenGLBackend::EndRenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    m_currentFBO = m_defaultFBO;
    glViewport(0, 0, m_width, m_height);
}
```

---

### 5.9 DrawCards

#### 第一段：卡片着色器编译（懒初始化）

**分析**：`DrawCards` 使用硬编码的 GLSL 着色器（非 SPIR-V），因为卡片渲染是后端内部功能，不涉及跨后端共享。使用 `static GLuint cardProgram` 确保着色器只编译一次。顶点着色器实现 MVP 变换（Model-View-Projection），片段着色器实现纹理采样和透明度混合。

```cpp
void OpenGLBackend::DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) {
    if (cards.empty() || !viewMat || !projMat) return;

    // 按需构建卡片着色器（硬编码 GLSL）
    static GLuint cardProgram = 0;
    if (cardProgram == 0) {
        const char* cardVS = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec2 aTexCoord;
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProj;
            out vec2 vTexCoord;
            out float vOpacity;
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

        // 编译、链接着色器...
        // (省略编译错误处理代码)
```

#### 第二段：卡片几何体与属性设置

**分析**：创建临时 VAO/VBO/EBO 用于卡片渲染。顶点数据为交错格式：每个顶点包含 3 个位置分量（x,y,z）和 2 个 UV 分量（u,v），步长为 5 个 float。使用索引绘制（两个三角形组成一个四边形）。位置属性绑定到 location 0，UV 属性绑定到 location 1。

```cpp
    glUseProgram(cardProgram);

    // 设置视图和投影矩阵
    GLint viewLoc = glGetUniformLocation(cardProgram, "uView");
    GLint projLoc = glGetUniformLocation(cardProgram, "uProj");
    if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);
    if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMat);

    // 卡片四边形顶点（位置 + UV）
    static const float cardVerts[] = {
        // pos x, y, z    uv u, v
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
    };
    static const unsigned int cardIdx[] = { 0, 1, 2, 0, 2, 3 };

    // 创建临时 VAO 用于卡片渲染
    GLuint cardVAO, cardVBO, cardEBO;
    glGenVertexArrays(1, &cardVAO);
    glGenBuffers(1, &cardVBO);
    glGenBuffers(1, &cardEBO);

    glBindVertexArray(cardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cardVerts), cardVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cardEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cardIdx), cardIdx, GL_STATIC_DRAW);

    // 位置属性（location 0）：vec3
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV 属性（location 1）：vec2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
```

#### 第三段：逐卡绘制与模型矩阵构建

**分析**：遍历每张卡片，构建模型矩阵（缩放 → Y 轴旋转 → 平移）。矩阵乘法手动实现（三重循环），先应用缩放到单位矩阵，再乘以旋转矩阵，最后加上平移分量。将纹理绑定到单元 0，设置模型矩阵和不透明度 uniform，绘制 6 个索引（两个三角形）。

```cpp
    for (const auto& card : cards) {
        GLuint tex = GetGLTexture(card.texture);
        if (tex == 0) continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // 构建模型矩阵：平移 * 绕Y旋转 * 缩放
        float model[16] = {};

        // 从单位矩阵开始
        model[0] = model[5] = model[10] = model[15] = 1.0f;

        // 应用缩放
        model[0]  *= card.scaleX;
        model[5]  *= card.scaleY;

        // 应用绕 Y 轴旋转
        float cosR = std::cos(card.rotationY);
        float sinR = std::sin(card.rotationY);
        float rotMat[16] = {};
        rotMat[0]  =  cosR;
        rotMat[2]  =  sinR;
        rotMat[5]  =  1.0f;
        rotMat[8]  = -sinR;
        rotMat[10] =  cosR;
        rotMat[15] =  1.0f;

        // 矩阵乘法：result = rotMat * model
        float result[16] = {};
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                for (int k = 0; k < 4; ++k) {
                    result[r * 4 + c] += rotMat[r * 4 + k] * model[k * 4 + c];
                }
            }
        }

        // 应用平移
        result[12] += card.posX;
        result[13] += card.posY;
        result[14] += card.posZ;

        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, result);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, card.opacity);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    // 清理临时卡片几何体
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &cardVAO);
    glDeleteBuffers(1, &cardVBO);
    glDeleteBuffers(1, &cardEBO);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

#### 完整源码

```cpp
void OpenGLBackend::DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMat, const float* projMat) {
    if (cards.empty() || !viewMat || !projMat) return;

    // 按需构建卡片着色器（硬编码 GLSL）
    static GLuint cardProgram = 0;
    if (cardProgram == 0) {
        const char* cardVS = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec2 aTexCoord;
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProj;
            out vec2 vTexCoord;
            out float vOpacity;
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

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &cardVS, nullptr);
        glCompileShader(vs);
        GLint success = 0;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(vs, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card vertex shader failed: %s\n", log);
            glDeleteShader(vs);
            return;
        }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &cardFS, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(fs, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card fragment shader failed: %s\n", log);
            glDeleteShader(vs);
            glDeleteShader(fs);
            return;
        }

        cardProgram = glCreateProgram();
        glAttachShader(cardProgram, vs);
        glAttachShader(cardProgram, fs);
        glLinkProgram(cardProgram);
        glGetProgramiv(cardProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(cardProgram, 512, nullptr, log);
            fprintf(stderr, "[OpenGL] Card program link failed: %s\n", log);
            glDeleteProgram(cardProgram);
            cardProgram = 0;
            glDeleteShader(vs);
            glDeleteShader(fs);
            return;
        }
        glDetachShader(cardProgram, vs);
        glDetachShader(cardProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    glUseProgram(cardProgram);

    // 设置视图和投影矩阵
    GLint viewLoc = glGetUniformLocation(cardProgram, "uView");
    GLint projLoc = glGetUniformLocation(cardProgram, "uProj");
    if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);
    if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMat);

    // 卡片四边形顶点（位置 + UV）
    static const float cardVerts[] = {
        // pos x, y, z    uv u, v
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
    };
    static const unsigned int cardIdx[] = { 0, 1, 2, 0, 2, 3 };

    // 创建临时 VAO 用于卡片渲染
    GLuint cardVAO, cardVBO, cardEBO;
    glGenVertexArrays(1, &cardVAO);
    glGenBuffers(1, &cardVBO);
    glGenBuffers(1, &cardEBO);

    glBindVertexArray(cardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cardVerts), cardVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cardEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cardIdx), cardIdx, GL_STATIC_DRAW);

    // 位置属性（location 0）：vec3
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV 属性（location 1）：vec2
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // 设置纹理采样器
    GLint texLoc = glGetUniformLocation(cardProgram, "uCardTexture");
    if (texLoc >= 0) glUniform1i(texLoc, 0);

    GLint modelLoc = glGetUniformLocation(cardProgram, "uModel");
    GLint opacityLoc = glGetUniformLocation(cardProgram, "uOpacity");

    for (const auto& card : cards) {
        GLuint tex = GetGLTexture(card.texture);
        if (tex == 0) continue;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // 构建模型矩阵：平移 * 绕Y旋转 * 缩放
        float model[16] = {};

        // 从单位矩阵开始
        model[0] = model[5] = model[10] = model[15] = 1.0f;

        // 应用缩放
        model[0]  *= card.scaleX;
        model[5]  *= card.scaleY;

        // 应用绕 Y 轴旋转
        float cosR = std::cos(card.rotationY);
        float sinR = std::sin(card.rotationY);
        float rotMat[16] = {};
        rotMat[0]  =  cosR;
        rotMat[2]  =  sinR;
        rotMat[5]  =  1.0f;
        rotMat[8]  = -sinR;
        rotMat[10] =  cosR;
        rotMat[15] =  1.0f;

        // 矩阵乘法：result = rotMat * model
        float result[16] = {};
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                for (int k = 0; k < 4; ++k) {
                    result[r * 4 + c] += rotMat[r * 4 + k] * model[k * 4 + c];
                }
            }
        }

        // 应用平移
        result[12] += card.posX;
        result[13] += card.posY;
        result[14] += card.posZ;

        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, result);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, card.opacity);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    // 清理临时卡片几何体
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &cardVAO);
    glDeleteBuffers(1, &cardVBO);
    glDeleteBuffers(1, &cardEBO);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}
```

---

### 5.10 ImGui 集成

#### 第一段：ImGuiInit — 上下文创建与字体加载

**分析**：初始化 ImGui 上下文，启用键盘导航和 Docking 功能。字体加载采用"默认拉丁字体 + 中文字体合并"策略：先加载 ImGui 默认字体（覆盖拉丁字符），再以 MergeMode 将微软雅黑（msyh.ttc）的中文字形合并到同一字体图集中。使用 `GetGlyphRangesChineseFull()` 获取完整的中文字符范围。这种方案避免了为中文单独创建字体图集，减少纹理切换。

```cpp
void OpenGLBackend::ImGuiInit(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // 启用键盘导航
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // 启用 Docking

    // 加载字体：默认拉丁字体 + 中文字体合并
    // 优先尝试 Windows 系统字体
    const char* chineseFontPath = nullptr;
    const char* latinFontPath = nullptr;

    // 检查微软雅黑（msyh.ttc）— Windows 上最佳 CJK 覆盖
    if (GetFileAttributesA("C:\\Windows\\Fonts\\msyh.ttc") != INVALID_FILE_ATTRIBUTES) {
        chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }

    if (chineseFontPath) {
        // 先加载默认 ImGui 字体（覆盖拉丁字形）
        io.Fonts->AddFontDefault();

        // 将中文字形合并到默认字体之上
        ImFontConfig cfg;
        cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());
    }
    // 如果没有找到中文字体，使用 ImGui 默认字体（ProggyClean）

    // 初始化 ImGui GLFW 后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // 初始化 ImGui OpenGL3 后端
    ImGui_ImplOpenGL3_Init("#version 460 core");
}
```

#### 第二段：ImGuiNewFrame / ImGuiRender / ImGuiShutdown

**分析**：`ImGuiNewFrame` 按照正确的顺序调用 OpenGL3 → GLFW → ImGui 的 NewFrame，确保输入事件和 GL 状态正确初始化。`ImGuiRender` 先调用 `ImGui::Render()` 生成绘制数据，再通过 `ImGui_ImplOpenGL3_RenderDrawData` 将绘制命令转化为 OpenGL 调用。`ImGuiShutdown` 按相反顺序销毁后端和上下文。

```cpp
void OpenGLBackend::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();  // OpenGL3 后端新帧
    ImGui_ImplGlfw_NewFrame();     // GLFW 后端新帧（处理输入事件）
    ImGui::NewFrame();            // ImGui 核心新帧
}

void OpenGLBackend::ImGuiRender() {
    ImGui::Render();                                       // 生成绘制数据
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // 转化为 GL 调用
}

void OpenGLBackend::ImGuiShutdown() {
    ImGui_ImplOpenGL3_Shutdown();  // 销毁 OpenGL3 后端
    ImGui_ImplGlfw_Shutdown();     // 销毁 GLFW 后端
    ImGui::DestroyContext();       // 销毁 ImGui 上下文
}
```

#### 完整源码

```cpp
void OpenGLBackend::ImGuiInit(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // 加载字体：默认拉丁字体 + 中文字体合并
    const char* chineseFontPath = nullptr;

    // 检查微软雅黑（msyh.ttc）
    if (GetFileAttributesA("C:\\Windows\\Fonts\\msyh.ttc") != INVALID_FILE_ATTRIBUTES) {
        chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }

    if (chineseFontPath) {
        // 先加载默认 ImGui 字体
        io.Fonts->AddFontDefault();

        // 将中文字形合并到默认字体之上
        ImFontConfig cfg;
        cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());
    }

    // 初始化 ImGui GLFW 后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // 初始化 ImGui OpenGL3 后端
    ImGui_ImplOpenGL3_Init("#version 460 core");
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

---

### 5.11 Shutdown

#### 第一段：着色器与程序清理

**分析**：`Shutdown()` 按照依赖关系的逆序清理所有 GL 资源。首先遍历 `m_shaders` 删除所有着色器对象，然后遍历 `m_programCache` 删除所有程序对象。注意：程序必须在着色器之前或同时删除，因为程序引用着色器。

```cpp
void OpenGLBackend::Shutdown() {
    // 清理着色器
    for (auto& [id, shader] : m_shaders) {
        if (shader != 0) {
            glDeleteShader(shader);
        }
    }
    m_shaders.clear();

    // 清理程序
    for (auto& [key, program] : m_programCache) {
        if (program != 0) {
            glDeleteProgram(program);
        }
    }
    m_programCache.clear();
```

#### 第二段：帧缓冲区、纹理与几何体清理

**分析**：清理所有帧缓冲区对象、纹理对象（同时清理关联的格式和尺寸元数据）、全屏四边形的 VAO/VBO 和临时 UBO。最后绑定默认 FBO 并调用 `glFinish()` 确保所有 GPU 命令完成——这防止了在同一 GLFW 窗口上切换到 Vulkan 后端时出现资源冲突。

```cpp
    // 清理帧缓冲区
    for (auto& [id, fbo] : m_framebuffers) {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }
    m_framebuffers.clear();

    // 清理纹理
    for (auto& [id, tex] : m_textures) {
        if (tex != 0) {
            glDeleteTextures(1, &tex);
        }
    }
    m_textures.clear();
    m_textureFormats.clear();
    m_texWidths.clear();
    m_texHeights.clear();

    // 清理全屏四边形
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }

    // 清理 UBO
    if (m_tempUBO != 0) {
        glDeleteBuffers(1, &m_tempUBO);
        m_tempUBO = 0;
    }

    // 解绑 FBO 并刷新 — 防止在同一 GLFW 窗口上切换到 Vulkan 时冲突
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glFinish();

    printf("[OpenGL] Shutdown\n");
}
```

#### 完整源码

```cpp
void OpenGLBackend::Shutdown() {
    // 清理着色器
    for (auto& [id, shader] : m_shaders) {
        if (shader != 0) {
            glDeleteShader(shader);
        }
    }
    m_shaders.clear();

    // 清理程序
    for (auto& [key, program] : m_programCache) {
        if (program != 0) {
            glDeleteProgram(program);
        }
    }
    m_programCache.clear();

    // 清理帧缓冲区
    for (auto& [id, fbo] : m_framebuffers) {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }
    m_framebuffers.clear();

    // 清理纹理
    for (auto& [id, tex] : m_textures) {
        if (tex != 0) {
            glDeleteTextures(1, &tex);
        }
    }
    m_textures.clear();
    m_textureFormats.clear();
    m_texWidths.clear();
    m_texHeights.clear();

    // 清理全屏四边形
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }

    // 清理 UBO
    if (m_tempUBO != 0) {
        glDeleteBuffers(1, &m_tempUBO);
        m_tempUBO = 0;
    }

    // 解绑 FBO 并刷新 — 防止在同一 GLFW 窗口上切换到 Vulkan 时冲突
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glFinish();

    printf("[OpenGL] Shutdown\n");
}
```


# Shader Showcase 技术文档（第三部分：第6-8章）

---

## 6. Vulkan 渲染后端

本章详细分析 `VulkanBackend` 类的完整实现。该类继承自 `IRenderBackend` 接口，封装了 Vulkan 1.2 图形 API 的全部初始化、资源管理和渲染流程。与 OpenGL 后端不同，Vulkan 要求开发者显式管理每一个底层对象——从实例创建到命令缓冲录制——因此代码量更大，但性能可控性也更强。

---

### 6.1 头文件

**文件**: `src/render/VulkanBackend.h`

头文件定义了三类核心数据结构：Vulkan 资源包装器、后端类本身，以及辅助查询结构体。

#### 第一段：Vulkan 资源包装结构体

**分析**：Vulkan 没有内置的资源管理机制，所有 GPU 对象都以不透明句柄形式存在。本项目定义了三个包装结构体来统一管理这些句柄及其关联元数据：

- `VulkanShader`：封装 `VkShaderModule` 和着色器阶段标志位（顶点/片元）。
- `VulkanTexture`：封装图像三件套（Image + Memory + ImageView）外加 Sampler，并额外携带 FBO 相关的 Framebuffer 和 RenderPass 句柄，以及 ImGui 描述符集缓存。
- `VulkanPipeline`：封装管线对象及其布局、描述集布局/池/集，以及管线自有的 UBO 缓冲区。

```cpp
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSet imguiDescriptorSet = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    ShaderHandle vertShader;
    ShaderHandle fragShader;
    VkBuffer uboBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uboMemory = VK_NULL_HANDLE;
};
```

#### 第二段：VulkanBackend 类声明——核心成员

**分析**：后端类的成员变量按功能域分组，包括：窗口引用与尺寸、Vulkan 核心对象（实例/物理设备/逻辑设备/队列）、Surface 与交换链、命令缓冲与同步对象、帧状态追踪、资源管理容器（使用 `unordered_map` + 自增 ID 的句柄系统）、当前渲染状态、ImGui 资源，以及延迟销毁队列和管线缓存。

```cpp
class VulkanBackend : public IRenderBackend {
public:
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) override;
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void DestroyPipeline(PipelineHandle handle) override;
    void BindPipeline(PipelineHandle handle) override;
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;
    BackendType GetType() const override { return BackendType::Vulkan; }
    const char* GetName() const override { return "Vulkan 1.2 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 1280;
    int m_height = 720;
    bool m_initialized = false;
    bool m_framebufferResized = false;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsFamily = 0;
    uint32_t m_presentFamily = 0;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D m_swapchainExtent = {};
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    uint32_t m_currentImageIndex = 0;
    bool m_isRecording = false;

    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;
    std::unordered_map<uint32_t, VulkanShader> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;

    PipelineHandle m_currentPipeline = {0};
    VkFramebuffer m_currentFramebuffer = VK_NULL_HANDLE;
    VkRenderPass m_currentRenderPass = VK_NULL_HANDLE;
    bool m_isRenderToTexture = false;

    VkDescriptorPool m_imguiDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_imguiDescSetLayout = VK_NULL_HANDLE;
    bool m_imguiInitialized = false;
    bool m_imguiRenderPending = false;

    struct DeferredDestroy {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
    };
    std::vector<DeferredDestroy> m_deferredDestroys;
    std::unordered_map<uint64_t, PipelineHandle> m_pipelineCache;
};
```

#### 完整头文件源码

```cpp
#pragma once
#include "render/IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

struct GLFWwindow;

// ============================================================================
// Vulkan Resource Wrappers
// ============================================================================
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;           // 着色器模块句柄
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT; // 着色器阶段标志
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;                    // 图像对象
    VkDeviceMemory memory = VK_NULL_HANDLE;             // 图像绑定的设备内存
    VkImageView view = VK_NULL_HANDLE;                 // 图像视图
    VkSampler sampler = VK_NULL_HANDLE;                 // 采样器
    int width = 0, height = 0;                          // 纹理尺寸
    VkFormat format = VK_FORMAT_UNDEFINED;               // 像素格式
    bool isFBO = false;                                  // 是否为帧缓冲目标
    VkFramebuffer framebuffer = VK_NULL_HANDLE;          // FBO 帧缓冲
    VkRenderPass renderPass = VK_NULL_HANDLE;            // FBO 渲染通道
    VkDescriptorSet imguiDescriptorSet = VK_NULL_HANDLE; // ImGui 纹理描述符集缓存
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;                // 图形管线
    VkPipelineLayout layout = VK_NULL_HANDLE;            // 管线布局
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE; // 描述符集布局
    VkDescriptorPool descPool = VK_NULL_HANDLE;          // 描述符池
    VkDescriptorSet descSet = VK_NULL_HANDLE;            // 预分配描述符集（绑定纹理+UBO）
    ShaderHandle vertShader;                             // 顶点着色器句柄
    ShaderHandle fragShader;                             // 片元着色器句柄
    VkBuffer uboBuffer = VK_NULL_HANDLE;                 // 管线自有 UBO 缓冲区
    VkDeviceMemory uboMemory = VK_NULL_HANDLE;           // UBO 绑定的设备内存
};

// ============================================================================
// Vulkan Backend
// ============================================================================
class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // 生命周期
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // 视口
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) override;

    // 着色器（SPIR-V）
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // 纹理
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;

    // 管线
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void DestroyPipeline(PipelineHandle handle) override;
    void BindPipeline(PipelineHandle handle) override;

    // 全屏绘制
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<IRenderBackend::CardDrawInfo>& cards,
                  const float* viewMatrix, const float* projMatrix) override;
    void BlitToScreen(TextureHandle src) override;
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // 工具函数
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // 查询
    BackendType GetType() const override { return BackendType::Vulkan; }
    const char* GetName() const override { return "Vulkan 1.2 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    // 窗口引用
    GLFWwindow* m_window = nullptr;                       // GLFW 窗口指针
    int m_width = 1280;                                    // 帧缓冲宽度
    int m_height = 720;                                   // 帧缓冲高度
    bool m_initialized = false;                            // 是否已初始化
    bool m_framebufferResized = false;                     // 帧缓冲是否需要重建

    // 核心 Vulkan 对象
    VkInstance m_instance = VK_NULL_HANDLE;               // Vulkan 实例
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;   // 物理设备（GPU）
    VkDevice m_device = VK_NULL_HANDLE;                    // 逻辑设备
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;              // 图形队列
    VkQueue m_presentQueue = VK_NULL_HANDLE;               // 呈现队列
    uint32_t m_graphicsFamily = 0;                          // 图形队列族索引
    uint32_t m_presentFamily = 0;                          // 呈现队列族索引

    // Surface 和交换链
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;              // 窗口 Surface
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;           // 交换链
    std::vector<VkImage> m_swapchainImages;                 // 交换链图像数组
    std::vector<VkImageView> m_swapchainImageViews;        // 交换链图像视图数组
    std::vector<VkFramebuffer> m_swapchainFramebuffers;   // 交换链帧缓冲数组
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM; // 交换链格式
    VkExtent2D m_swapchainExtent = {};                     // 交换链尺寸
    VkRenderPass m_renderPass = VK_NULL_HANDLE;            // 主渲染通道

    // 命令缓冲
    VkCommandPool m_commandPool = VK_NULL_HANDLE;           // 命令池
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;      // 主命令缓冲

    // 同步对象
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;  // 图像可用信号量
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;  // 渲染完成信号量
    VkFence m_inFlightFence = VK_NULL_HANDLE;                // 帧内飞行栅栏

    // 帧状态
    uint32_t m_currentImageIndex = 0;                       // 当前交换链图像索引
    bool m_isRecording = false;                              // 是否正在录制命令

    // 资源管理
    uint32_t m_nextShaderId = 1;                             // 下一个着色器 ID
    uint32_t m_nextTextureId = 1;                           // 下一个纹理 ID
    uint32_t m_nextPipelineId = 1;                           // 下一个管线 ID
    std::unordered_map<uint32_t, VulkanShader> m_shaders;    // 着色器存储
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures; // 纹理存储
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines; // 管线存储

    // 当前状态
    PipelineHandle m_currentPipeline = {0};                  // 当前绑定的管线
    VkFramebuffer m_currentFramebuffer = VK_NULL_HANDLE;     // 当前帧缓冲
    VkRenderPass m_currentRenderPass = VK_NULL_HANDLE;        // 当前渲染通道
    bool m_isRenderToTexture = false;                         // 是否在渲染到纹理

    // ImGui Vulkan 资源
    VkDescriptorPool m_imguiDescriptorPool = VK_NULL_HANDLE; // ImGui 描述符池
    VkDescriptorSetLayout m_imguiDescSetLayout = VK_NULL_HANDLE; // ImGui 描述符集布局
    bool m_imguiInitialized = false;                          // ImGui 是否已初始化
    bool m_imguiRenderPending = false;                         // ImGui 渲染是否待执行

    // 延迟销毁队列
    struct DeferredDestroy {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
    };
    std::vector<DeferredDestroy> m_deferredDestroys;

    // 管线缓存
    std::unordered_map<uint64_t, PipelineHandle> m_pipelineCache;

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

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        bool isComplete() const { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX; }
    };
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};
```

---

### 6.2 Init

**文件**: `src/render/VulkanBackend.cpp`

#### 第一段：初始化入口与异常处理

**分析**：`Init` 是 Vulkan 后端的入口函数，接收 GLFW 窗口指针后，按严格顺序依次调用 9 个初始化步骤。每个步骤都可能抛出 `std::runtime_error`，因此整个流程被包裹在 `try-catch` 中。

```cpp
bool VulkanBackend::Init(GLFWwindow* window) {
    m_window = window;
    glfwGetFramebufferSize(window, &m_width, &m_height);

    try {
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateRenderPass();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSyncObjects();
    } catch (const std::exception& e) {
        fprintf(stderr, "[Vulkan] Init failed: %s\n", e.what());
        return false;
    }

    m_initialized = true;
    printf("[Vulkan] Initialized successfully (%dx%d)\n", m_width, m_height);
    return true;
}
```

#### 完整源码

```cpp
bool VulkanBackend::Init(GLFWwindow* window) {
    m_window = window;                                       // 保存窗口指针
    glfwGetFramebufferSize(window, &m_width, &m_height);     // 获取帧缓冲尺寸

    try {
        CreateInstance();       // 创建 Vulkan 实例
        CreateSurface();        // 创建窗口 Surface
        PickPhysicalDevice();   // 选择物理设备（GPU）
        CreateLogicalDevice();  // 创建逻辑设备与队列
        CreateSwapchain();      // 创建交换链
        CreateRenderPass();     // 创建渲染通道
        CreateFramebuffers();   // 创建帧缓冲
        CreateCommandPool();    // 创建命令池
        CreateCommandBuffers(); // 分配命令缓冲
        CreateSyncObjects();    // 创建同步对象（信号量+栅栏）
    } catch (const std::exception& e) {
        fprintf(stderr, "[Vulkan] Init failed: %s\n", e.what());
        return false;
    }

    m_initialized = true;
    printf("[Vulkan] Initialized successfully (%dx%d)\n", m_width, m_height);
    return true;
}
```

---

### 6.3 CreateInstance

#### 第一段：验证层检查与应用信息

**分析**：创建 Vulkan 实例前，首先检查系统是否支持请求的验证层（仅 Debug 模式启用）。然后填充 `VkApplicationInfo`，指定应用名称、引擎名称和目标 API 版本（Vulkan 1.2）。GLFW 通过 `glfwGetRequiredInstanceExtensions` 返回创建 Surface 所需的扩展列表。

```cpp
void VulkanBackend::CreateInstance() {
    if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shader Showcase";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shader Showcase Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLE_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
```

#### 第二段：实例创建

**分析**：填充 `VkInstanceCreateInfo`，将应用信息、扩展列表和验证层列表传入，调用 `vkCreateInstance` 完成实例创建。

```cpp
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    printf("[Vulkan] Instance created\n");
}
```

#### 完整源码

```cpp
void VulkanBackend::CreateInstance() {
    if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport()) {
        fprintf(stderr, "[Vulkan] Validation layers requested but not available\n");
        throw std::runtime_error("Validation layers requested but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shader Showcase";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shader Showcase Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLE_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    printf("[Vulkan] Instance created\n");
}
```

---

### 6.4 CreateSurface

#### 第一段：Win32 原生 Surface 创建

**分析**：本项目使用 GLFW 创建窗口，但 GLFW 默认以 OpenGL API 创建窗口。在某些 NVIDIA 驱动上，`glfwCreateWindowSurface` 会拒绝 OpenGL 窗口。解决方案是绕过 GLFW，直接使用 Win32 API 的 `vkCreateWin32SurfaceKHR`。

```cpp
void VulkanBackend::CreateSurface() {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = glfwGetWin32Window(m_window);

    VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
    printf("[Vulkan] Surface created (Win32 native)\n");
}
```

#### 完整源码

```cpp
void VulkanBackend::CreateSurface() {
    // 使用 Win32 原生 Surface 创建，绕过 glfwCreateWindowSurface
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);    // 当前进程实例句柄
    surfaceCreateInfo.hwnd = glfwGetWin32Window(m_window);      // GLFW 窗口的 HWND

    VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
    printf("[Vulkan] Surface created (Win32 native)\n");
}
```

---

### 6.5 PickPhysicalDevice

#### 第一段：设备枚举与评分选择

**分析**：物理设备选择采用评分策略。独立显卡 +1000 分，集成显卡 +100 分，显存每 GB +1 分。

```cpp
void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
    if (deviceCount == 0) throw std::runtime_error("No Vulkan-capable GPU found");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    int bestScore = -1;
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

    for (const auto& device : devices) {
        if (!IsDeviceSuitable(device)) continue;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 100;

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);
        VkDeviceSize totalMemory = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                totalMemory += memProps.memoryHeaps[i].size;
        }
        score += static_cast<int>(totalMemory / (1024 * 1024 * 1024));

        if (score > bestScore) { bestScore = score; bestDevice = device; }
    }

    if (bestDevice == VK_NULL_HANDLE) throw std::runtime_error("No suitable GPU found");
    m_physicalDevice = bestDevice;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    printf("[Vulkan] Physical device: %s\n", props.deviceName);
}
```

#### 完整源码

```cpp
void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan-capable GPU found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    int bestScore = -1;
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

    for (const auto& device : devices) {
        if (!IsDeviceSuitable(device)) continue;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 100;
        }

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
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    printf("[Vulkan] Physical device: %s\n", props.deviceName);
}
```

---

### 6.6 CreateLogicalDevice

#### 第一段：队列族查找与设备创建

**分析**：逻辑设备创建首先查找图形队列族和呈现队列族的索引，使用 `std::set` 去重后为每个队列族创建队列创建信息，优先级设为 1.0f。

```cpp
void VulkanBackend::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    m_graphicsFamily = indices.graphicsFamily;
    m_presentFamily = indices.presentFamily;

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

    VkPhysicalDeviceFeatures deviceFeatures{};

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

#### 完整源码

```cpp
void VulkanBackend::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    m_graphicsFamily = indices.graphicsFamily;
    m_presentFamily = indices.presentFamily;

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

    VkPhysicalDeviceFeatures deviceFeatures{};

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

---

### 6.7 CreateSwapchain

#### 第一段：交换链参数选择

**分析**：交换链创建涉及三个关键选择：Surface 格式（优先 B8G8R8A8 + SRGB）、呈现模式（优先 FIFO 即 VSync）、以及尺寸（使用 Surface 能力报告的当前尺寸）。图像数量设为最小值 +1 以实现双缓冲。

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
```

#### 第二段：交换链创建与图像获取

**分析**：填充 `VkSwapchainCreateInfoKHR`，根据图形和呈现队列族是否相同选择共享模式或独占模式。创建交换链后获取图像句柄并保存格式和尺寸。

```cpp
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

#### 完整源码

```cpp
void VulkanBackend::CreateSwapchain() {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
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

    printf("[Vulkan] Swapchain created (%dx%d, %zu images)\n",
           extent.width, extent.height, m_swapchainImages.size());
}
```

---

### 6.8 CreateRenderPass

#### 第一段：渲染通道描述

**分析**：渲染通道定义了帧缓冲输出的格式和操作。使用单颜色附件，加载操作为 CLEAR，存储操作为 STORE。`initialLayout` 为 `UNDEFINED`，`finalLayout` 为 `PRESENT_SRC_KHR`。

```cpp
void VulkanBackend::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

#### 完整源码

```cpp
void VulkanBackend::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

---

### 6.9 CreateTexture

#### 第一段：纹理创建与格式转换

**分析**：`CreateTexture` 支持三种格式（RGBA8、RGBA32F、R8）。当 `data == nullptr` 时，纹理被标记为 FBO，自动添加 `VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`。

```cpp
TextureHandle VulkanBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    size_t pixelSize = GetTextureFormatSize(format);
    VkDeviceSize imageSize = width * height * pixelSize;

    auto texture = std::make_unique<VulkanTexture>();
    texture->width = width;
    texture->height = height;
    texture->format = vkFormat;
    texture->isFBO = (data == nullptr);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture->isFBO) usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    CreateImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                vkFormat, VK_IMAGE_TILING_OPTIMAL, usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                texture->image, texture->memory);

    texture->view = CreateImageView(texture->image, vkFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    texture->sampler = CreateSampler();
```

#### 第二段：数据上传与布局转换

**分析**：对于有初始数据的纹理，创建暂存缓冲区上传数据，执行两步布局转换：`UNDEFINED -> TRANSFER_DST -> SHADER_READ_ONLY`。对于 FBO 纹理，只需一步转换。

```cpp
    if (data != nullptr) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        void* mappedData;
        vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingBufferMemory);

        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, texture->image,
                          static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    } else {
        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    uint32_t id = m_nextTextureId++;
    m_textures[id] = std::move(texture);
    return {id};
}
```

#### 完整源码

```cpp
TextureHandle VulkanBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    size_t pixelSize = GetTextureFormatSize(format);
    VkDeviceSize imageSize = width * height * pixelSize;

    auto texture = std::make_unique<VulkanTexture>();
    texture->width = width;
    texture->height = height;
    texture->format = vkFormat;
    texture->isFBO = (data == nullptr);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture->isFBO) usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    CreateImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                vkFormat, VK_IMAGE_TILING_OPTIMAL, usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                texture->image, texture->memory);

    texture->view = CreateImageView(texture->image, vkFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    texture->sampler = CreateSampler();

    if (data != nullptr) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        void* mappedData;
        vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingBufferMemory);

        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, texture->image,
                          static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    } else {
        TransitionImageLayout(texture->image, vkFormat,
                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    uint32_t id = m_nextTextureId++;
    m_textures[id] = std::move(texture);
    return {id};
}
```

---

### 6.10 CreatePipeline

#### 第一段：管线缓存与着色器阶段

**分析**：`CreatePipeline` 首先通过缓存键检查是否已有相同管线，避免重复创建。然后创建描述符集布局、管线布局和描述符池。

```cpp
PipelineHandle VulkanBackend::CreatePipeline(const PipelineDesc& desc) {
    auto vertIt = m_shaders.find(desc.vertShader.id);
    auto fragIt = m_shaders.find(desc.fragShader.id);
    if (vertIt == m_shaders.end() || fragIt == m_shaders.end()) return {0};

    VkRenderPass renderPass = m_currentRenderPass != VK_NULL_HANDLE ? m_currentRenderPass : m_renderPass;
    uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
    cacheKey ^= (uint64_t)renderPass;

    auto cacheIt = m_pipelineCache.find(cacheKey);
    if (cacheIt != m_pipelineCache.end()) return cacheIt->second;

    auto pipeline = std::make_unique<VulkanPipeline>();
    pipeline->descSetLayout = CreateDescriptorSetLayout();
    pipeline->layout = CreatePipelineLayout(pipeline->descSetLayout);
    pipeline->descPool = CreateDescriptorPool(1000);
```

#### 第二段：管线状态配置

**分析**：全屏四边形管线不需要顶点数据，使用三角形列表拓扑。视口和裁剪设为动态状态。光栅化器禁用背面剔除。

```cpp
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
```

#### 第三段：管线创建与 UBO 分配

**分析**：创建管线后为每个管线分配 48 字节的 UBO 缓冲区和描述符集，这些资源与管线生命周期绑定，每帧复用。

```cpp
    VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline));

    uint32_t id = m_nextPipelineId++;
    m_pipelines[id] = std::move(pipeline);
    m_pipelineCache[cacheKey] = PipelineHandle{id};

    auto& pp = m_pipelines[id];
    const size_t UBO_SIZE = 48;
    CreateBuffer(UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 pp->uboBuffer, pp->uboMemory);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pp->descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &pp->descSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &pp->descSet));

    return {id};
}
```

#### 完整源码

```cpp
PipelineHandle VulkanBackend::CreatePipeline(const PipelineDesc& desc) {
    auto vertIt = m_shaders.find(desc.vertShader.id);
    auto fragIt = m_shaders.find(desc.fragShader.id);
    if (vertIt == m_shaders.end() || fragIt == m_shaders.end()) {
        fprintf(stderr, "[Vulkan] Invalid shader handles for pipeline creation\n");
        return {0};
    }

    VkRenderPass renderPass = m_currentRenderPass != VK_NULL_HANDLE ? m_currentRenderPass : m_renderPass;
    uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
    cacheKey ^= (uint64_t)renderPass;

    auto cacheIt = m_pipelineCache.find(cacheKey);
    if (cacheIt != m_pipelineCache.end()) return cacheIt->second;

    auto pipeline = std::make_unique<VulkanPipeline>();
    pipeline->vertShader = desc.vertShader;
    pipeline->fragShader = desc.fragShader;

    pipeline->descSetLayout = CreateDescriptorSetLayout();
    pipeline->layout = CreatePipelineLayout(pipeline->descSetLayout);
    pipeline->descPool = CreateDescriptorPool(1000);

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertIt->second.module;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragIt->second.module;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.width = static_cast<float>(desc.width);
    viewport.height = static_cast<float>(desc.height);
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = {static_cast<uint32_t>(desc.width), static_cast<uint32_t>(desc.height)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

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

    auto& pp = m_pipelines[id];
    const size_t UBO_SIZE = 48;
    CreateBuffer(UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 pp->uboBuffer, pp->uboMemory);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pp->descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &pp->descSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &pp->descSet));

    printf("[Vulkan] Pipeline created (id=%u)\n", id);
    return {id};
}
```

---

### 6.11 DrawFullscreenQuad

#### 第一段：管线创建与绑定

**分析**：`DrawFullscreenQuad` 是后处理渲染的核心函数。构建 `PipelineDesc` 后调用 `CreatePipeline`（内部有缓存机制），然后绑定管线并设置动态视口和裁剪矩形。

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

    VkViewport viewport{};
    viewport.width = static_cast<float>(params.viewportWidth);
    viewport.height = static_cast<float>(params.viewportHeight);
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = {static_cast<uint32_t>(params.viewportWidth), static_cast<uint32_t>(params.viewportHeight)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
```

#### 第二段：UBO 数据更新

**分析**：UBO 布局固定为 48 字节：6 个 float 参数（偏移 0-23）、vec2 分辨率（偏移 24-31）、float 时间（偏移 32-35）、float 帧计数（偏移 36-39）。使用 HOST_COHERENT 内存直接 memcpy 写入。

```cpp
    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end() && pipeIt->second->descSet != VK_NULL_HANDLE) {
        const size_t UBO_SIZE = 48;

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
```

#### 第三段：描述符集更新与绘制

**分析**：每帧更新描述符集，将输入纹理（binding=0）和 UBO（binding=1）绑定。最后通过 `vkCmdDraw(3, 1, 0, 0)` 绘制全屏三角形。

```cpp
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
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        writes.push_back(uboWrite);

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeIt->second->layout, 0, 1, &pipeIt->second->descSet, 0, nullptr);
    }

    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
    m_currentPipeline = {0};
}
```

#### 完整源码

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

    VkViewport viewport{};
    viewport.width = static_cast<float>(params.viewportWidth);
    viewport.height = static_cast<float>(params.viewportHeight);
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = {static_cast<uint32_t>(params.viewportWidth), static_cast<uint32_t>(params.viewportHeight)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end() && pipeIt->second->descSet != VK_NULL_HANDLE) {
        const size_t UBO_SIZE = 48;

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
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        writes.push_back(uboWrite);

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeIt->second->layout, 0, 1, &pipeIt->second->descSet, 0, nullptr);
    }

    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
    m_currentPipeline = {0};
}
```

---

### 6.12 BeginFrame / EndFrame

#### 第一段：BeginFrame

**分析**：`BeginFrame` 执行帧开始的关键步骤：等待上一帧栅栏、重置栅栏、处理延迟销毁队列、获取交换链图像、开始命令缓冲录制、开始渲染通道。

```cpp
void VulkanBackend::BeginFrame() {
    if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) {
        RecreateSwapchain();
        if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) return;
    }

    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);

    for (auto& dd : m_deferredDestroys) {
        if (dd.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, dd.pipeline, nullptr);
        if (dd.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, dd.layout, nullptr);
        if (dd.descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, dd.descSetLayout, nullptr);
        if (dd.descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, dd.descPool, nullptr);
        if (dd.uboBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, dd.uboBuffer, nullptr);
        if (dd.uboMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, dd.uboMemory, nullptr);
    }
    m_deferredDestroys.clear();

    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { RecreateSwapchain(); return; }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return;

    vkResetCommandBuffer(m_commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
    m_isRecording = true;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    renderPassInfo.renderArea.extent = m_swapchainExtent;
    VkClearValue clearColor = {{{0.12f, 0.16f, 0.24f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_currentRenderPass = m_renderPass;
    m_currentFramebuffer = m_swapchainFramebuffers[m_currentImageIndex];
}
```

#### 第二段：EndFrame

**分析**：`EndFrame` 先渲染 ImGui 绘制数据，然后结束渲染通道和命令缓冲，提交到图形队列，最后通过呈现队列显示到屏幕。

```cpp
void VulkanBackend::EndFrame() {
    if (!m_isRecording) return;

    if (m_imguiRenderPending) {
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->CmdListsCount > 0)
            ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffer);
        m_imguiRenderPending = false;
    }

    vkCmdEndRenderPass(m_commandBuffer);
    m_currentRenderPass = VK_NULL_HANDLE;
    m_currentFramebuffer = VK_NULL_HANDLE;

    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
    m_isRecording = false;

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

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_currentImageIndex;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        RecreateSwapchain();
}
```

---

### 6.13 ImGui 集成

#### 第一段：ImGui Vulkan 初始化

**分析**：ImGui 的 Vulkan 后端初始化需要：创建 ImGui 上下文、加载中文字体（优先微软雅黑，回退宋体，使用 MergeMode 合并字形）、创建大型描述符池、创建 ImGui 专用描述符集布局、调用 `ImGui_ImplVulkan_Init`。

```cpp
void VulkanBackend::ImGuiInit(GLFWwindow* window) {
    if (m_imguiInitialized) return;

    IMGUI_CHECKVERSION();
    if (ImGui::GetCurrentContext() == nullptr) ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGuiIO& io = ImGui::GetIO();
    const char* chineseFontPath = nullptr;
    {
        std::ifstream testFile("C:\\Windows\\Fonts\\msyh.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }
    if (!chineseFontPath) {
        std::ifstream testFile("C:\\Windows\\Fonts\\simsun.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\simsun.ttc";
    }
    if (chineseFontPath) {
        io.Fonts->AddFontDefault();
        ImFontConfig cfg;
        cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(chineseFontPath, 16.0f, &cfg, io.Fonts->GetGlyphRangesChineseFull());
    }

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
    poolInfo.sType = VK_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_imguiDescriptorPool));

    VkDescriptorSetLayoutBinding imguiSamplerBinding{};
    imguiSamplerBinding.binding = 0;
    imguiSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imguiSamplerBinding.descriptorCount = 1;
    imguiSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo imguiLayoutInfo{};
    imguiLayoutInfo.sType = VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    imguiLayoutInfo.bindingCount = 1;
    imguiLayoutInfo.pBindings = &imguiSamplerBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &imguiLayoutInfo, nullptr, &m_imguiDescSetLayout));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.QueueFamily = m_graphicsFamily;
    initInfo.Queue = m_graphicsQueue;
    initInfo.DescriptorPool = m_imguiDescriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(m_swapchainImages.size());
    initInfo.PipelineInfoMain.RenderPass = m_renderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    ImGui_ImplVulkan_Init(&initInfo);
    m_imguiInitialized = true;
}
```

#### 第二段：ImGuiNewFrame / ImGuiRender / ImGuiShutdown

**分析**：`ImGuiRender` 设置 `m_imguiRenderPending` 标志，实际的 Vulkan 绘制延迟到 `EndFrame` 中执行。`ImGuiShutdown` 按相反顺序清理资源。

```cpp
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
    if (m_imguiDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
    if (m_imguiDescSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(m_device, m_imguiDescSetLayout, nullptr);
    m_imguiInitialized = false;
}
```

---

### 6.14 Shutdown

#### 第一段：资源逆序销毁

**分析**：`Shutdown` 按照与初始化相反的顺序销毁所有 Vulkan 资源。先调用 `WaitIdle` 确保 GPU 空闲，然后依次清理管线、纹理、着色器、同步对象、命令池、交换链、渲染通道、逻辑设备、Surface 和实例。

```cpp
void VulkanBackend::Shutdown() {
    if (!m_initialized) return;
    WaitIdle();

    for (auto& [id, pipeline] : m_pipelines) {
        if (pipeline->pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, pipeline->pipeline, nullptr);
        if (pipeline->layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, pipeline->layout, nullptr);
        if (pipeline->descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, pipeline->descSetLayout, nullptr);
        if (pipeline->descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, pipeline->descPool, nullptr);
    }
    m_pipelines.clear();

    for (auto& [id, texture] : m_textures) {
        if (texture->sampler != VK_NULL_HANDLE) vkDestroySampler(m_device, texture->sampler, nullptr);
        if (texture->view != VK_NULL_HANDLE) vkDestroyImageView(m_device, texture->view, nullptr);
        if (texture->image != VK_NULL_HANDLE) vkDestroyImage(m_device, texture->image, nullptr);
        if (texture->memory != VK_NULL_HANDLE) vkFreeMemory(m_device, texture->memory, nullptr);
        if (texture->framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(m_device, texture->framebuffer, nullptr);
        if (texture->renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(m_device, texture->renderPass, nullptr);
    }
    m_textures.clear();

    for (auto& [id, shader] : m_shaders) {
        if (shader.module != VK_NULL_HANDLE) vkDestroyShaderModule(m_device, shader.module, nullptr);
    }
    m_shaders.clear();

    if (m_inFlightFence != VK_NULL_HANDLE) { vkDestroyFence(m_device, m_inFlightFence, nullptr); m_inFlightFence = VK_NULL_HANDLE; }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) { vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr); m_renderFinishedSemaphore = VK_NULL_HANDLE; }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) { vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr); m_imageAvailableSemaphore = VK_NULL_HANDLE; }

    if (m_commandPool != VK_NULL_HANDLE) { vkDestroyCommandPool(m_device, m_commandPool, nullptr); m_commandPool = VK_NULL_HANDLE; }

    CleanupSwapchain();

    if (m_renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(m_device, m_renderPass, nullptr); m_renderPass = VK_NULL_HANDLE; }
    if (m_device != VK_NULL_HANDLE) { vkDestroyDevice(m_device, nullptr); m_device = VK_NULL_HANDLE; }
    if (m_surface != VK_NULL_HANDLE) { vkDestroySurfaceKHR(m_instance, m_surface, nullptr); m_surface = VK_NULL_HANDLE; }
    if (m_instance != VK_NULL_HANDLE) { vkDestroyInstance(m_instance, nullptr); m_instance = VK_NULL_HANDLE; }

    m_initialized = false;
}
```

---

## 7. 着色器加载与效果元数据

本章分析着色器加载系统和效果元数据解析模块。这两个模块共同构成了后处理效果的"声明式"配置系统。

---

### 7.1 ShaderLoader

**文件**: `src/shader/ShaderLoader.h`, `src/shader/ShaderLoader.cpp`

#### 第一段：SPIR-V 文件加载

**分析**：`LoadSPIRV` 以二进制模式打开 `.spv` 文件，通过 `tellg` 获取文件大小，验证大小是否为 4 的倍数（SPIR-V 由 32 位字组成），然后将整个文件读入 `vector<uint32_t>`。

```cpp
std::vector<uint32_t> ShaderLoader::LoadSPIRV(const std::string& filepath) {
    std::vector<uint32_t> result;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        fprintf(stderr, "[ShaderLoader] Cannot open SPIR-V file: %s\n", filepath.c_str());
        return result;
    }

    std::streamsize size = file.tellg();
    if (size <= 0 || size % sizeof(uint32_t) != 0) {
        fprintf(stderr, "[ShaderLoader] Invalid SPIR-V file: %s\n", filepath.c_str());
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
```

#### 第二段：着色器目录查找

**分析**：`FindShaderDir` 解决了可执行文件与着色器资源之间的路径映射问题。依次尝试三个候选路径：`../../shaders`、`../shaders`、`./shaders`。

```cpp
std::string ShaderLoader::FindShaderDir() {
#ifdef _WIN32
    char exePathBuf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        std::string exeDir(exePathBuf, len);
        size_t slash = exeDir.find_last_of("\\/");
        if (slash != std::string::npos) exeDir = exeDir.substr(0, slash);

        std::string candidate = exeDir + "/../../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        candidate = exeDir + "/../shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        candidate = exeDir + "/shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;
    }
#endif
    return "shaders";
}
```

#### 完整源码

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
        if (slash != std::string::npos) exeDir = exeDir.substr(0, slash);

        std::string candidate = exeDir + "/../../shaders";
        std::ifstream test(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

        candidate = exeDir + "/../shaders";
        test.open(candidate + "/common/fullscreen.vert.spv");
        if (test.good()) return candidate;

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
        if (slash != std::string::npos) exeDir = exeDir.substr(0, slash);

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

---

### 7.2 EffectMetadata

**文件**: `src/shader/EffectMetadata.h`, `src/shader/EffectMetadata.cpp`

#### 第一段：数据结构定义

**分析**：`ShaderParam` 描述单个着色器参数的元数据（名称、类型、范围、默认值、UI 控件类型）。`EffectCard` 描述一个完整的效果卡片。`UniformBinding` 描述运行时的 uniform 绑定状态。

```cpp
enum class ParamType { Float, Int, Bool, Float2, Float3, Float4, Color };

struct ShaderParam {
    std::string name;
    std::string label;
    ParamType type = ParamType::Float;
    float minVal = 0.0f, maxVal = 1.0f;
    float defaultVal[4] = {0,0,0,0};
    std::string uiType;
    std::vector<std::string> comboOptions;
};

struct EffectCard {
    std::string id;
    std::string name;
    std::string category;
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
```

#### 第二段：JSON 解析器（无外部依赖）

**分析**：`EffectMetadata.cpp` 实现了一个轻量级 JSON 解析器，不依赖任何第三方库。核心函数包括 `ExtractString`、`ExtractNumber`、`GetStringValue`、`GetNumberValue`。解析器通过字符串查找和位置追踪实现。

```cpp
namespace {

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
```

#### 第三段：LoadEffectFromJson 主函数

**分析**：`LoadEffectFromJson` 首先进行安全检查（文件存在性和大小限制 100KB），然后移除 C 风格注释，提取顶层字段，最后解析 `params` 数组中的每个参数对象。

```cpp
EffectCard LoadEffectFromJson(const std::string& filepath) {
    EffectCard card;

    FILE* test = fopen(filepath.c_str(), "rb");
    if (!test) { fprintf(stderr, "[EffectMetadata] File not found: %s\n", filepath.c_str()); return card; }
    fseek(test, 0, SEEK_END);
    long sz = ftell(test);
    fclose(test);
    if (sz <= 0 || sz > 102400) return card;

    std::string json = ReadFile(filepath);
    if (json.empty()) return card;

    // 移除 C 风格注释
    {
        std::string cleaned;
        cleaned.reserve(json.size());
        for (size_t i = 0; i < json.size(); ++i) {
            if (json[i] == '/' && i+1 < json.size()) {
                if (json[i+1] == '/') { i += 2; while (i < json.size() && json[i] != '\n') ++i; if (i < json.size()) cleaned += '\n'; continue; }
                if (json[i+1] == '*') { i += 2; while (i+1 < json.size() && !(json[i] == '*' && json[i+1] == '/')) ++i; i += 1; continue; }
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
                if (!defStr.empty()) { try { param.defaultVal[0] = static_cast<float>(std::stod(defStr)); } catch (...) { defStr.clear(); } }
                if (defStr.empty()) { param.defaultVal[0] = static_cast<float>(GetNumberValue(paramScope, "default")); }

                param.uiType = GetStringValue(paramScope, "ui_type");

                size_t comboPos = paramScope.find("\"combo_options\"");
                if (comboPos != std::string::npos) param.comboOptions = ExtractStringArray(paramScope, comboPos);

                card.params.push_back(std::move(param));
                pos = objEnd + 1;
            }
        }
    }

    return card;
}
```

#### 完整源码

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
    if (sz <= 0 || sz > 102400) { fprintf(stderr, "[EffectMetadata] Invalid file size %ld\n", sz); return card; }

    std::string json = ReadFile(filepath);
    if (json.empty()) return card;

    // 移除 C 风格注释
    {
        std::string cleaned;
        cleaned.reserve(json.size());
        for (size_t i = 0; i < json.size(); ++i) {
            if (json[i] == '/' && i+1 < json.size()) {
                if (json[i+1] == '/') { i += 2; while (i < json.size() && json[i] != '\n') ++i; if (i < json.size()) cleaned += '\n'; continue; }
                if (json[i+1] == '*') { i += 2; while (i+1 < json.size() && !(json[i] == '*' && json[i+1] == '/')) ++i; i += 1; continue; }
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
                if (!defStr.empty()) { try { param.defaultVal[0] = static_cast<float>(std::stod(defStr)); } catch (...) { defStr.clear(); } }
                if (defStr.empty()) { param.defaultVal[0] = static_cast<float>(GetNumberValue(paramScope, "default")); }

                param.uiType = GetStringValue(paramScope, "ui_type");

                size_t comboPos = paramScope.find("\"combo_options\"");
                if (comboPos != std::string::npos) param.comboOptions = ExtractStringArray(paramScope, comboPos);

                card.params.push_back(std::move(param));
                pos = objEnd + 1;
            }
        }
    }

    return card;
}
```

---

### 7.3 Shader 代码结构

**文件**: `shaders/common/fullscreen.vert`, `shaders/effects/bloom/bloom.frag`

#### 第一段：全屏顶点着色器

**分析**：顶点着色器接收 `aPos`（location=0）作为顶点位置输入，将 `[-1, 1]` 的 NDC 坐标映射到 `[0, 1]` 的 UV 坐标。

```glsl
#version 460

layout(location=0) in vec2 aPos;
layout(location=0) out vec2 vUV;

void main() {
    vUV = (aPos + 1.0) * 0.5;
    gl_Position = vec4(aPos, 0, 1);
}
```

#### 第二段：Bloom 片元着色器

**分析**：Bloom 效果的片元着色器实现了单通道泛光算法。核心流程：采样输入纹理获取原始颜色；根据 `BlurSize` 计算高斯模糊核范围；双重循环遍历核内采样点；对亮度超过 `Threshold` 的像素加权累加；将泛光结果乘以 `BloomIntensity` 后叠加到原始颜色。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;

layout(binding=0) uniform sampler2D uInputTex;

layout(std140, binding=1) uniform Params {
    float uParamFloat0;     // 泛光强度
    float uParamFloat1;     // 阈值
    float uParamFloat2;     // 模糊大小
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

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

---

### 7.4 effect.json 参数化

**文件**: `shaders/effects/bloom/effect.json`

#### 第一段：Bloom 效果配置

**分析**：`effect.json` 是效果参数化的核心配置文件。Bloom 效果定义了 3 个 Float 类型参数：泛光强度（0-3，默认 0.8）、阈值（0-1，默认 0.7）、模糊大小（1-10，默认 4.0）。这些参数通过 `EffectMetadata` 解析后，在 UI 中自动生成对应的滑块控件。

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

#### 完整源码

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

---

### 7.5 UBO 固定布局

**分析**：本项目使用固定的 UBO（Uniform Buffer Object）布局，所有后处理效果共享同一个 48 字节的结构。这个布局在着色器中通过 `layout(std140, binding=1) uniform Params` 声明，在 C++ 端通过 `ShaderParams` 结构体对应。`std140` 布局规则保证了跨平台的一致性。

UBO 内存布局（48 字节）：

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0 | 4 | `uParamFloat0` | 效果参数 0 |
| 4 | 4 | `uParamFloat1` | 效果参数 1 |
| 8 | 4 | `uParamFloat2` | 效果参数 2 |
| 12 | 4 | `uParamFloat3` | 效果参数 3 |
| 16 | 4 | `uParamFloat4` | 效果参数 4 |
| 20 | 4 | `uParamFloat5` | 效果参数 5 |
| 24 | 8 | `uResolution` | vec2 分辨率（std140 对齐到 8 字节边界） |
| 32 | 4 | `uTime` | 时间（秒） |
| 36 | 4 | `uFrameCount` | 帧计数 |
| 40-47 | 8 | （未使用） | 填充到 48 字节 |

关键设计决策：
- 使用 `std140` 布局而非 `std430`，确保在所有 GPU 上的一致性
- 6 个通用 float 参数足以覆盖大多数单通道效果
- vec2 分辨率从偏移 24 开始（std140 规则：vec2 对齐到 8 字节边界）
- 时间和帧计数为系统自动填充，不需要 effect.json 配置

---

## 8. 输入源与 UI 组件

本章分析两个输入源模块（视频播放器和屏幕捕获）和一个 UI 组件（性能面板）。

---

### 8.1 VideoPlayer

**文件**: `src/input/VideoPlayer.h`, `src/input/VideoPlayer.cpp`

#### 第一段：头文件与成员变量

**分析**：`VideoPlayer` 采用 ffmpeg 子进程方式解码视频，无需链接 FFmpeg 库。核心设计：通过匿名管道从 ffmpeg 读取原始 RGBA 像素数据。Windows 平台使用 `CreateProcess` + `CreatePipe`，Linux 平台使用 `popen`。4MB 的读取缓冲区平衡了内存使用和读取效率。

```cpp
class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();
    bool Open(const std::string& filePath);
    bool ReadFrame();
    void Seek(double seconds);
    void Close();
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
    HANDLE m_processHandle = nullptr;
    int m_width = 0, m_height = 0;
    double m_duration = 0.0;
    double m_fps = 30.0;
    double m_currentTime = 0.0;
    bool m_open = false;
    std::vector<uint8_t> m_pixels;
    std::vector<uint8_t> m_readBuf;
    static constexpr size_t READ_BUF_SIZE = 4 * 1024 * 1024;
};
```

#### 第二段：Open 函数

**分析**：`Open` 函数首先关闭已有资源，然后启动 ffmpeg 子进程，读取第一帧以获取视频尺寸信息。

```cpp
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
```

#### 第三段：ReadFrame 函数

**分析**：`ReadFrame` 从管道中读取一帧完整的 RGBA 数据。由于管道读取可能返回不完整的数据，使用循环确保读取到 `width * height * 4` 字节。

```cpp
bool VideoPlayer::ReadFrame() {
    if (!m_pipeRead) return false;

    size_t needed = (size_t)m_width * m_height * 4;
    if (needed == 0) return false;

    m_pixels.resize(needed);
    size_t totalRead = 0;

    while (totalRead < needed) {
        size_t toRead = needed - totalRead;
        if (toRead > m_readBuf.size()) toRead = m_readBuf.size();

#ifdef _WIN32
        DWORD bytesRead = 0;
        if (!ReadFile(m_pipeRead, m_readBuf.data(), (DWORD)toRead, &bytesRead, nullptr)) {
            return false;
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
```

#### 第四段：StartFFmpegProcess（Windows）

**分析**：Windows 实现分为两步：(1) 使用 `ffprobe` 探测视频的宽度、高度、帧率和时长；(2) 使用 `CreatePipe` + `CreateProcess` 启动 ffmpeg 进程，将输出重定向到管道。

```cpp
bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
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

    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return false;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

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
        CloseHandle(hRead); CloseHandle(hWrite);
        return false;
    }

    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    m_pipeRead = hRead;
    m_processHandle = pi.hProcess;
    return true;
}
```

#### 完整源码

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
    m_readBuf.resize(READ_BUF_SIZE);                            // 预分配 4MB 读取缓冲
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

    size_t needed = (size_t)m_width * m_height * 4;           // RGBA 每像素 4 字节
    if (needed == 0) return false;

    m_pixels.resize(needed);
    size_t totalRead = 0;

    while (totalRead < needed) {
        size_t toRead = needed - totalRead;
        if (toRead > m_readBuf.size()) toRead = m_readBuf.size();

#ifdef _WIN32
        DWORD bytesRead = 0;
        if (!ReadFile(m_pipeRead, m_readBuf.data(), (DWORD)toRead, &bytesRead, nullptr)) {
            return false;
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
    (void)seconds;                                             // 管道方式不支持 seek
}

#ifdef _WIN32

bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
    // 使用 ffprobe 探测视频信息
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
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return false;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    // 启动 ffmpeg 进程
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
        CloseHandle(hRead); CloseHandle(hWrite);
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

#else

bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "ffprobe -v error -select_streams v:0 -show_entries "
        "stream=width,height,r_frame_rate,duration -of csv=p=0 '%s'",
        filePath.c_str());

    FILE* probePipe = popen(cmd, "r");
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
        pclose(probePipe);
    }

    if (m_width == 0 || m_height == 0) {
        m_width = 1920; m_height = 1080; m_fps = 30.0;
    }

    snprintf(cmd, sizeof(cmd),
        "ffmpeg -i '%s' -loglevel error -f rawvideo -pix_fmt rgba pipe:1",
        filePath.c_str());

    FILE* pipe = popen(cmd, "r");
    if (!pipe) return false;

    m_pipeRead = (VIDEO_PIPE_TYPE)fileno(pipe);
    return true;
}

void VideoPlayer::StopProcess() {
    if (m_pipeRead) {
        close((int)m_pipeRead);
        m_pipeRead = nullptr;
    }
}

#endif
```

---

### 8.2 ScreenCapture

**文件**: `src/input/ScreenCapture.h`, `src/input/ScreenCapture.cpp`

#### 第一段：头文件与成员变量

**分析**：`ScreenCapture` 使用 DXGI Desktop Duplication API（Windows 8+）捕获主显示器内容。核心组件包括：D3D11 设备和上下文（用于 GPU 资源访问）、DXGI Output Duplication 接口（用于帧捕获）、Staging Texture（用于 CPU 读取）。捕获的帧数据以 BGRA 格式从 GPU 读出后转换为 RGBA。

```cpp
class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    bool Init();
    void Shutdown();
    bool CaptureFrame();
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const uint8_t* GetPixels() const { return m_pixels.data(); }
    bool IsReady() const { return m_ready; }

private:
    ID3D11Device*           m_d3dDevice        = nullptr;
    ID3D11DeviceContext*    m_d3dContext        = nullptr;
    IDXGIOutputDuplication* m_deskDupl          = nullptr;
    ID3D11Texture2D*        m_stagingTex        = nullptr;
    int m_width  = 0;
    int m_height = 0;
    bool m_ready = false;
    std::vector<uint8_t> m_pixels;
};
```

#### 第二段：Init 函数

**分析**：`Init` 函数首先创建 D3D11 设备，然后通过 DXGI 适配器枚举找到绑定到桌面的输出，最后调用 `DuplicateOutput` 创建桌面复制接口。获取输出尺寸后分配像素缓冲区。

```cpp
bool ScreenCapture::Init() {
    if (m_ready) return true;

    D3D_FEATURE_LEVEL featLevel;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                    nullptr, 0, D3D11_SDK_VERSION,
                                    &m_d3dDevice, &featLevel, &m_d3dContext);
    if (FAILED(hr)) return false;

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

    IDXGIAdapter1* foundAdapter = nullptr;
    IDXGIOutput* output = FindPrimaryOutput(factory, foundAdapter);
    factory->Release();

    if (!output) { if (foundAdapter) foundAdapter->Release(); Shutdown(); return false; }

    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr)) { Shutdown(); return false; }

    hr = output1->DuplicateOutput(m_d3dDevice, &m_deskDupl);
    output1->Release();
    if (FAILED(hr)) { Shutdown(); return false; }

    DXGI_OUTDUPL_DESC duplDesc;
    m_deskDupl->GetDesc(&duplDesc);
    m_width  = duplDesc.ModeDesc.Width;
    m_height = duplDesc.ModeDesc.Height;

    m_pixels.resize(m_width * m_height * 4);
    m_ready = true;
    return true;
}
```

#### 第三段：CaptureFrame 函数

**分析**：`CaptureFrame` 调用 `AcquireNextFrame` 获取桌面帧，超时设为 16ms（约 60fps）。获取成功后将桌面纹理复制到 Staging Texture，映射到 CPU 内存，执行 BGRA 到 RGBA 的像素格式转换。如果遇到 `DXGI_ERROR_ACCESS_LOST`，自动重新初始化。

```cpp
bool ScreenCapture::CaptureFrame() {
    if (!m_ready) return false;

    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_deskDupl->AcquireNextFrame(16, &frameInfo, &desktopResource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) return false;

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            Shutdown();
            return Init();
        }
        return false;
    }

    ID3D11Texture2D* desktopTex = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTex);
    desktopResource->Release();
    if (FAILED(hr)) { m_deskDupl->ReleaseFrame(); return false; }

    if (!m_stagingTex) {
        D3D11_TEXTURE2D_DESC desc = {};
        desktopTex->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;
        hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, &m_stagingTex);
        if (FAILED(hr)) { desktopTex->Release(); m_deskDupl->ReleaseFrame(); return false; }
    }

    m_d3dContext->CopyResource(m_stagingTex, desktopTex);
    desktopTex->Release();

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_d3dContext->Map(m_stagingTex, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) { m_deskDupl->ReleaseFrame(); return false; }

    // BGRA -> RGBA 转换
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

#endif // _WIN32
```

---

### 8.3 PerformancePanel

**文件**: `src/ui/PerformancePanel.h`, `src/ui/PerformancePanel.cpp`

#### 第一段：头文件与成员变量

**分析**：`PerformancePanel` 使用 ImGui 绘制实时性能监控面板。核心功能包括：帧时间图表（最近 120 帧的滚动窗口）、FPS 计数器、帧时间统计（平均/最小/最大/百分位）、GPU 信息显示。使用环形缓冲区存储历史帧时间数据，避免频繁的内存分配。

```cpp
class PerformancePanel {
public:
    void Init();
    void BeginFrame(float deltaTime);
    void Render();
    void Reset();

private:
    static constexpr int HISTORY_SIZE = 120;                    // 帧时间历史长度
    float m_frameTimeHistory[HISTORY_SIZE] = {};               // 帧时间环形缓冲
    int m_historyIndex = 0;                                     // 当前写入位置
    float m_avgFrameTime = 0.0f;                                // 平均帧时间
    float m_minFrameTime = 999.0f;                              // 最小帧时间
    float m_maxFrameTime = 0.0f;                                // 最大帧时间
    float m_fps = 0.0f;                                        // 当前 FPS
    float m_fpsAccum = 0.0f;                                   // FPS 累加器
    int m_fpsFrameCount = 0;                                   // FPS 帧计数
    float m_fpsUpdateTimer = 0.0f;                             // FPS 更新定时器
    bool m_showPanel = true;                                    // 是否显示面板
    char m_gpuName[256] = {};                                   // GPU 名称
};
```

#### 第二段：BeginFrame — 帧时间采集

**分析**：`BeginFrame` 在每帧开始时调用，记录当前帧时间到环形缓冲区，更新 FPS 统计（每 0.5 秒刷新一次），计算平均/最小/最大帧时间。

```cpp
void PerformancePanel::BeginFrame(float deltaTime) {
    // 记录帧时间到环形缓冲区
    m_frameTimeHistory[m_historyIndex] = deltaTime * 1000.0f;    // 转换为毫秒
    m_historyIndex = (m_historyIndex + 1) % HISTORY_SIZE;

    // FPS 统计
    m_fpsAccum += deltaTime;
    m_fpsFrameCount++;
    m_fpsUpdateTimer += deltaTime;

    if (m_fpsUpdateTimer >= 0.5f) {                              // 每 0.5 秒更新
        m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsAccum;
        m_fpsAccum = 0.0f;
        m_fpsFrameCount = 0;
        m_fpsUpdateTimer = 0.0f;

        // 计算统计值
        m_avgFrameTime = 0.0f;
        m_minFrameTime = 999.0f;
        m_maxFrameTime = 0.0f;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            float ft = m_frameTimeHistory[i];
            m_avgFrameTime += ft;
            if (ft > 0.0f && ft < m_minFrameTime) m_minFrameTime = ft;
            if (ft > m_maxFrameTime) m_maxFrameTime = ft;
        }
        m_avgFrameTime /= HISTORY_SIZE;
    }
}
```

#### 第三段：Render — ImGui 面板绘制

**分析**：`Render` 使用 ImGui 窗口 API 绘制性能面板。顶部显示 FPS 和帧时间统计，中部绘制帧时间折线图（使用 `ImGui::PlotLines`），底部显示 GPU 信息和后端名称。面板可通过右上角关闭按钮隐藏。

```cpp
void PerformancePanel::Render() {
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Performance", &m_showPanel)) {
        ImGui::End();
        return;
    }

    // FPS 和帧时间统计
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Frame: %.2f ms", m_avgFrameTime);
    ImGui::Text("Min: %.2f ms  Max: %.2f ms", m_minFrameTime, m_maxFrameTime);

    // 帧时间折线图
    float displayHistory[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (m_historyIndex + i) % HISTORY_SIZE;         // 从最旧到最新
        displayHistory[i] = m_frameTimeHistory[idx];
    }

    ImGui::PlotLines("##FrameTime", displayHistory, HISTORY_SIZE,
                     0.0f, nullptr, 0.0f, 50.0f, ImVec2(-1, 80));

    // 16.67ms 参考线（60fps）
    ImGui::Text("GPU: %s", m_gpuName);

    ImGui::End();
}
```

#### 完整源码

```cpp
#include "ui/PerformancePanel.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

void PerformancePanel::Init() {
    memset(m_frameTimeHistory, 0, sizeof(m_frameTimeHistory));
    memset(m_gpuName, 0, sizeof(m_gpuName));
    m_historyIndex = 0;
    m_avgFrameTime = 0.0f;
    m_minFrameTime = 999.0f;
    m_maxFrameTime = 0.0f;
    m_fps = 0.0f;
    m_showPanel = true;
}

void PerformancePanel::BeginFrame(float deltaTime) {
    // 记录帧时间到环形缓冲区
    m_frameTimeHistory[m_historyIndex] = deltaTime * 1000.0f;    // 转换为毫秒
    m_historyIndex = (m_historyIndex + 1) % HISTORY_SIZE;        // 环形递增

    // FPS 统计（每 0.5 秒更新）
    m_fpsAccum += deltaTime;
    m_fpsFrameCount++;
    m_fpsUpdateTimer += deltaTime;

    if (m_fpsUpdateTimer >= 0.5f) {
        m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsAccum;
        m_fpsAccum = 0.0f;
        m_fpsFrameCount = 0;
        m_fpsUpdateTimer = 0.0f;

        // 计算帧时间统计值
        m_avgFrameTime = 0.0f;
        m_minFrameTime = 999.0f;
        m_maxFrameTime = 0.0f;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            float ft = m_frameTimeHistory[i];
            m_avgFrameTime += ft;
            if (ft > 0.0f && ft < m_minFrameTime) m_minFrameTime = ft;
            if (ft > m_maxFrameTime) m_maxFrameTime = ft;
        }
        m_avgFrameTime /= HISTORY_SIZE;
    }
}

void PerformancePanel::Render() {
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Performance", &m_showPanel)) {
        ImGui::End();
        return;
    }

    // FPS 和帧时间统计
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Frame: %.2f ms", m_avgFrameTime);
    ImGui::Text("Min: %.2f ms  Max: %.2f ms", m_minFrameTime, m_maxFrameTime);

    // 帧时间折线图（从最旧到最新排列）
    float displayHistory[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (m_historyIndex + i) % HISTORY_SIZE;
        displayHistory[i] = m_frameTimeHistory[idx];
    }

    ImGui::PlotLines("##FrameTime", displayHistory, HISTORY_SIZE,
                     0.0f, nullptr, 0.0f, 50.0f, ImVec2(-1, 80));

    // GPU 信息
    if (m_gpuName[0] != '\0') {
        ImGui::Text("GPU: %s", m_gpuName);
    }

    ImGui::End();
}

void PerformancePanel::Reset() {
    memset(m_frameTimeHistory, 0, sizeof(m_frameTimeHistory));
    m_historyIndex = 0;
    m_avgFrameTime = 0.0f;
    m_minFrameTime = 999.0f;
    m_maxFrameTime = 0.0f;
    m_fps = 0.0f;
    m_fpsAccum = 0.0f;
    m_fpsFrameCount = 0;
    m_fpsUpdateTimer = 0.0f;
}
```

---

> 文档结束。本文档覆盖了 Shader Showcase 项目的第 6-8 章，包括 Vulkan 渲染后端的完整实现（初始化、交换链、渲染通道、纹理、管线、全屏绘制、帧同步、ImGui 集成、资源销毁）、着色器加载与效果元数据系统（SPIR-V 加载、JSON 解析、UBO 布局）、以及输入源与 UI 组件（视频播放器、屏幕捕获、性能面板）。