# Shader 效果展示项目 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 构建一个基于 GLFW + OpenGL 4.6 / Vulkan 1.2 双后端的 Shader 效果展示应用，CoverFlow 风格总览页 + 全屏详情页 + ImGui 调参面板，SPIR-V 统一 Shader 双端复用。

**Architecture:** 分层架构 —— GLFW 窗口层 → 应用场景层（CoverFlowScene / EffectDetailScene）→ 渲染抽象层（IRenderBackend → OpenGLBackend / VulkanBackend）→ SPIR-V Shader 层 → 资源层 → ImGui UI 层。场景通过 Application 类管理状态切换，渲染后端通过工厂模式创建。

**Tech Stack:** GLFW 3.3+, OpenGL 4.6 (ARB_gl_spirv), Vulkan 1.2, Dear ImGui (docking), glslangValidator, stb_image, CMake 3.20+, MSVC 2022, C++17

---

## 文件结构规划

```
shader-showcase/                          # 项目根目录
├── CMakeLists.txt                         # 顶层构建（总控）
├── cmake/
│   └── FindGLFW.cmake                     # GLFW 查找模块
├── shaders/
│   ├── CMakeLists.txt                     # Shader 编译规则
│   ├── common/
│   │   └── fullscreen.vert               # 共用全屏顶点 Shader
│   └── effects/
│       ├── simple_test/
│       │   ├── simple_test.frag           # 第一个测试效果
│       │   └── effect.json               # 参数元数据
│       └── ...                            # 后续 17 个效果
├── src/
│   ├── main.cpp                           # 入口
│   ├── app/
│   │   ├── Application.h                  # 主应用（窗口、后端、场景管理）
│   │   ├── Application.cpp
│   │   ├── Scene.h                        # 场景抽象基类
│   │   ├── CoverFlowScene.h               # CoverFlow 场景
│   │   ├── CoverFlowScene.cpp
│   │   ├── EffectDetailScene.h            # 详情场景
│   │   └── EffectDetailScene.cpp
│   ├── render/
│   │   ├── BackendType.h                  # 后端枚举 + 句柄类型
│   │   ├── IRenderBackend.h               # 渲染后端纯虚接口
│   │   ├── OpenGLBackend.h                # OpenGL 后端
│   │   ├── OpenGLBackend.cpp
│   │   ├── VulkanBackend.h                # Vulkan 后端
│   │   ├── VulkanBackend.cpp
│   │   ├── FullscreenQuad.h               # 全屏四边形（兼容双后端）
│   │   └── FullscreenQuad.cpp
│   ├── shader/
│   │   ├── EffectMetadata.h               # 效果元数据结构 + JSON 解析
│   │   ├── EffectMetadata.cpp
│   │   ├── ShaderLoader.h                 # SPIR-V 二进制加载
│   │   └── ShaderLoader.cpp
│   ├── input/
│   │   ├── InputSource.h                  # 输入源抽象
│   │   ├── BuiltinInput.h                 # 内置资源
│   │   ├── BuiltinInput.cpp
│   │   ├── FileInput.h                    # 用户文件
│   │   └── FileInput.cpp
│   └── ui/
│       ├── DebugPanel.h                   # ImGui 调参面板
│       └── DebugPanel.cpp
├── assets/
│   └── textures/
│       └── test.png                       # 默认测试图
└── external/                              # 第三方库（vendored）
    ├── imgui/                             # Dear ImGui docking 分支
    ├── GLFW/                              # 已安装系统级可省略
    └── stb/
        └── stb_image.h
```

---

## 里程碑一：框架搭建

### Task 1: 项目骨架与 CMake 构建系统

**Files:**
- Create: `shader-showcase/CMakeLists.txt`
- Create: `shader-showcase/cmake/FindGLFW.cmake`
- Create: `shader-showcase/src/main.cpp`

- [ ] **Step 1: 创建顶层 CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(ShaderShowcase VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_OPENGL "Build with OpenGL backend" ON)
option(USE_VULKAN "Build with Vulkan backend" ON)

# GLFW
find_package(glfw3 REQUIRED)

# OpenGL
if(USE_OPENGL)
    find_package(OpenGL REQUIRED)
    add_definitions(-DUSE_OPENGL_BACKEND)
endif()

# Vulkan
if(USE_VULKAN)
    find_package(Vulkan REQUIRED)
    add_definitions(-DUSE_VULKAN_BACKEND)
endif()

# Vendor: stb_image
set(STB_DIR ${CMAKE_SOURCE_DIR}/external/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})

# Sources
file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(ShaderShowcase ${SRC_FILES})
target_include_directories(ShaderShowcase PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ShaderShowcase PRIVATE glfw stb)

if(USE_OPENGL)
    target_link_libraries(ShaderShowcase PRIVATE OpenGL::GL)
endif()

if(USE_VULKAN)
    target_link_libraries(ShaderShowcase PRIVATE Vulkan::Vulkan)
endif()

# ImGui
add_subdirectory(external/imgui)
target_link_libraries(ShaderShowcase PRIVATE imgui)

# Shaders (subdirectory for SPIR-V compilation)
add_subdirectory(shaders)
```

- [ ] **Step 2: 创建 FindGLFW.cmake**

```cmake
# cmake/FindGLFW.cmake
find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
find_library(GLFW_LIBRARY glfw3 glfw3dll)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glfw3 DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)

if(glfw3_FOUND AND NOT TARGET glfw)
    add_library(glfw UNKNOWN IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}")
endif()
```

- [ ] **Step 3: 创建最小 main.cpp，验证 GLFW 窗口能打开**

```cpp
// src/main.cpp
#include <GLFW/glfw3.h>
#include <cstdio>

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 稍后由后端设置

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Shader Showcase", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

- [ ] **Step 4: 创建 external/stb/stb_image.h，下载 stb_image 单头文件**

```bash
# 下载 stb_image.h
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" -OutFile "e:\AI\graph\hight-post-proc\shader-showcase\external\stb\stb_image.h"
```

- [ ] **Step 5: 克隆 ImGui docking 分支到 external/**

```bash
git clone -b docking https://github.com/ocornut/imgui.git e:\AI\graph\hight-post-proc\shader-showcase\external\imgui
```

- [ ] **Step 6: 配置并构建验证**

```bash
cmake -B build -S .
cmake --build build --config Release
```

Expected: 编译通过，运行后显示空白 1280x720 窗口。

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt cmake/FindGLFW.cmake src/main.cpp external/stb/ external/imgui/
git commit -m "feat: project skeleton with GLFW window"
```

---

### Task 2: 渲染后端抽象接口

**Files:**
- Create: `src/render/BackendType.h`
- Create: `src/render/IRenderBackend.h`

- [ ] **Step 1: 创建 BackendType.h — 后端枚举和通用句柄类型**

```cpp
// src/render/BackendType.h
#pragma once
#include <cstdint>

enum class BackendType { OpenGL, Vulkan };
enum class TextureFormat { RGBA8, RGBA32F, R8 };

struct ShaderHandle { uint32_t id = 0; };
struct TextureHandle { uint32_t id = 0; };
struct PipelineHandle { uint32_t id = 0; };

constexpr ShaderHandle INVALID_SHADER  = { 0xFFFFFFFF };
constexpr TextureHandle INVALID_TEXTURE = { 0xFFFFFFFF };
```

- [ ] **Step 2: 创建 IRenderBackend.h — 完整纯虚接口**

```cpp
// src/render/IRenderBackend.h
#pragma once
#include "BackendType.h"
#include <cstdint>
#include <vector>
#include <functional>

struct GLFWwindow;

// Shader uniform 参数（传递给 DrawFullscreenQuad）
struct ShaderParams {
    // 输入纹理列表
    std::vector<TextureHandle> inputTextures;
    // uniform 数据块（按 location 排列，类型在 EffectMetadata 中定义）
    std::vector<float> uniformFloats;
    std::vector<int32_t> uniformInts;
    // 窗口分辨率
    int viewportWidth = 1280;
    int viewportHeight = 720;
    // 帧时间（秒）
    float time = 0.0f;
    // 帧序号（用于 noise seed）
    uint32_t frameCount = 0;
};

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    // === 生命周期 ===
    virtual bool Init(GLFWwindow* window) = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void WaitIdle() = 0;

    // === 视口 ===
    virtual void Resize(int width, int height) = 0;
    virtual void GetFramebufferSize(int& width, int& height) const = 0;

    // === Shader ===
    // 从 SPIR-V 二进制创建顶点 Shader（返回 handle）
    virtual ShaderHandle CreateVertexShader(const std::vector<uint32_t>& spirv) = 0;
    // 从 SPIR-V 二进制创建片段 Shader（返回 handle）
    virtual ShaderHandle CreateFragmentShader(const std::vector<uint32_t>& spirv) = 0;
    // 销毁 Shader
    virtual void DestroyShader(ShaderHandle handle) = 0;

    // === 纹理 ===
    virtual TextureHandle CreateTexture(int width, int height, TextureFormat fmt,
                                         const void* data = nullptr) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;
    virtual void UpdateTexture(TextureHandle handle, int width, int height,
                                const void* data) = 0;

    // === 全屏渲染 ===
    // 绑定顶点+片段 Shader，设置输入纹理和 uniform，绘制一帧
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag,
                                     const ShaderParams& params) = 0;

    // === 自定义渲染（CoverFlow 用） ===
    // 绘制带纹理的 3D 变换面片列表，每个面片有自己的 model 矩阵和纹理
    struct CardDrawInfo {
        TextureHandle texture;
        float posX, posY, posZ;       // 3D 位置
        float scaleX, scaleY;         // 缩放
        float rotationY;              // 绕 Y 轴旋转（度）
        float opacity;                // 透明度
    };
    virtual void DrawCards(const std::vector<CardDrawInfo>& cards,
                            const float* viewMatrix, const float* projMatrix) = 0;

    // === 全屏 Blit ===
    // 将纹理直接 Blit 到整个屏幕（用于背景模糊等）
    virtual void BlitToScreen(TextureHandle texture) = 0;

    // === G-Buffer / 中间纹理 ===
    virtual void BeginRenderToTexture(TextureHandle target) = 0;
    virtual void EndRenderToTexture() = 0;

    // === ImGui ===
    virtual void ImGuiInit(GLFWwindow* window) = 0;
    virtual void ImGuiNewFrame() = 0;
    virtual void ImGuiRender() = 0;
    virtual void ImGuiShutdown() = 0;

    // === 查询 ===
    virtual BackendType GetType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetMaxTextureSize() const = 0;
};
```

- [ ] **Step 3: Commit**

```bash
git add src/render/BackendType.h src/render/IRenderBackend.h
git commit -m "feat: render backend abstraction interface"
```

---

### Task 3: OpenGL 后端实现

**Files:**
- Create: `src/render/OpenGLBackend.h`
- Create: `src/render/OpenGLBackend.cpp`
- Create: `src/render/FullscreenQuad.h`
- Create: `src/render/FullscreenQuad.cpp`

- [ ] **Step 1: 创建 FullscreenQuad — 全屏三角形（GPU 生成，无需 VBO）**

```cpp
// src/render/FullscreenQuad.h
#pragma once

// 生成全屏三角形的顶点数据（NDC 空间）
// 返回两个三角形组成的四边形 = 6 个顶点
// position.xy + texcoord.uv = 每个顶点 4 个 float
inline const float* GetFullscreenQuadVertices() {
    static const float vertices[] = {
        // position(xy)   texcoord(uv)
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,

        -1.0f,  1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
    };
    return vertices;
}
inline constexpr int kFullscreenQuadVertexCount = 6;
```

```cpp
// src/render/FullscreenQuad.cpp
#include "FullscreenQuad.h"
// 此文件当前仅包含头文件中的内联函数
// 后续可在此添加后端特定的 Quad 绘制辅助
```

- [ ] **Step 2: 创建 OpenGLBackend.h — 类声明**

```cpp
// src/render/OpenGLBackend.h
#pragma once
#include "IRenderBackend.h"
#include <unordered_map>
#include <vector>
#include <string>

struct GLFWwindow;

class OpenGLBackend : public IRenderBackend {
public:
    OpenGLBackend() = default;
    ~OpenGLBackend() override;

    // lifecycle
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // viewport
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) const override;

    // shader
    ShaderHandle CreateVertexShader(const std::vector<uint32_t>& spirv) override;
    ShaderHandle CreateFragmentShader(const std::vector<uint32_t>& spirv) override;
    void DestroyShader(ShaderHandle handle) override;

    // texture
    TextureHandle CreateTexture(int width, int height, TextureFormat fmt,
                                 const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void UpdateTexture(TextureHandle handle, int width, int height,
                        const void* data) override;

    // fullscreen pass
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag,
                             const ShaderParams& params) override;

    // card rendering
    void DrawCards(const std::vector<CardDrawInfo>& cards,
                    const float* viewMatrix, const float* projMatrix) override;

    // blit
    void BlitToScreen(TextureHandle texture) override;

    // render to texture
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // imgui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // query
    BackendType GetType() const override { return BackendType::OpenGL; }
    const char* GetName() const override { return "OpenGL 4.6 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 1280, m_height = 720;

    // resource pools
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    std::unordered_map<uint32_t, uint32_t> m_shaders;   // our id -> GL name
    std::unordered_map<uint32_t, uint32_t> m_textures;   // our id -> GL name
    std::unordered_map<uint32_t, uint32_t> m_framebuffers; // our id -> GL FBO

    // fullscreen quad
    uint32_t m_quadVAO = 0;
    uint32_t m_quadVBO = 0;

    // default FBO for backbuffer
    uint32_t m_defaultFBO = 0;
    uint32_t m_currentFBO = 0;

    // helpers
    uint32_t GetGLShader(ShaderHandle h) const;
    uint32_t GetGLTexture(TextureHandle h) const;
    uint32_t GetGLFramebuffer(TextureHandle h) const;
    uint32_t GLFormatFromTextureFormat(TextureFormat fmt) const;
    uint32_t GLInternalFormatFromTextureFormat(TextureFormat fmt) const;
    uint32_t GLTypeFromTextureFormat(TextureFormat fmt) const;
    uint32_t AllocateShaderId() { return m_nextShaderId++; }
    uint32_t AllocateTextureId() { return m_nextTextureId++; }
    void SetupQuadVAO();
    void BindDefaultState();
};
```

- [ ] **Step 3: 实现 OpenGLBackend.cpp — Init / Shutdown / BeginFrame / EndFrame**

```cpp
// src/render/OpenGLBackend.cpp
#include "OpenGLBackend.h"
#include "FullscreenQuad.h"
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <cstdio>
#include <cassert>

// === Lifecycle ===

bool OpenGLBackend::Init(GLFWwindow* window) {
    m_window = window;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    m_width = w;
    m_height = h;

    // Check SPIR-V support
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    bool hasSpirV = false;
    for (int i = 0; i < numExt; i++) {
        const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (strstr(ext, "GL_ARB_gl_spirv")) { hasSpirV = true; break; }
    }
    if (!hasSpirV) {
        fprintf(stderr, "[OpenGL] ERROR: GL_ARB_gl_spirv not supported!\n");
        return false;
    }

    printf("[OpenGL] Initialized: %s\n", glGetString(GL_VERSION));
    printf("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));

    SetupQuadVAO();
    BindDefaultState();

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&m_defaultFBO);
    return true;
}

void OpenGLBackend::Shutdown() {
    // Delete all resources
    for (auto& [id, glName] : m_shaders) glDeleteProgram(glName);
    for (auto& [id, glName] : m_textures) glDeleteTextures(1, &glName);
    for (auto& [id, glName] : m_framebuffers) glDeleteFramebuffers(1, &glName);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    m_shaders.clear();
    m_textures.clear();
    m_framebuffers.clear();
    printf("[OpenGL] Shutdown\n");
}

void OpenGLBackend::BeginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_currentFBO = m_defaultFBO;
}

void OpenGLBackend::EndFrame() {
    glfwSwapBuffers(m_window);
}

void OpenGLBackend::WaitIdle() { glFinish(); }

void OpenGLBackend::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void OpenGLBackend::GetFramebufferSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

int OpenGLBackend::GetMaxTextureSize() const {
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    return maxSize;
}
```

- [ ] **Step 4: 实现 Quad VAO 和默认状态**

```cpp
void OpenGLBackend::SetupQuadVAO() {
    const float* verts = GetFullscreenQuadVertices();

    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, kFullscreenQuadVertexCount * 4 * sizeof(float),
                 verts, GL_STATIC_DRAW);

    // position: location=0, vec2
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord: location=1, vec2
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void OpenGLBackend::BindDefaultState() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
```

- [ ] **Step 5: 实现 Shader 创建（SPIR-V → GL Program）**

```cpp
ShaderHandle OpenGLBackend::CreateVertexShader(const std::vector<uint32_t>& spirv) {
    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv.data(), spirv.size() * sizeof(uint32_t));
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    // Check compile status
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "[OpenGL] VS compile error: %s\n", log);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = AllocateShaderId();
    m_shaders[id] = shader;
    return { id };
}

ShaderHandle OpenGLBackend::CreateFragmentShader(const std::vector<uint32_t>& spirv) {
    // 片段 Shader 独立存储，在 DrawFullscreenQuad 时动态链接为 Program
    // （因为 VS 是共用的，每次组合创建临时 program）
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V,
                   spirv.data(), spirv.size() * sizeof(uint32_t));
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "[OpenGL] FS compile error: %s\n", log);
        glDeleteShader(shader);
        return INVALID_SHADER;
    }

    uint32_t id = AllocateShaderId();
    m_shaders[id] = shader; // 存储为独立 shader object
    return { id };
}

void OpenGLBackend::DestroyShader(ShaderHandle handle) {
    auto it = m_shaders.find(handle.id);
    if (it != m_shaders.end()) {
        glDeleteShader(it->second); // glDeleteShader 也用于 program 对象
        m_shaders.erase(it);
    }
}
```

- [ ] **Step 6: 实现纹理创建/更新/销毁**

```cpp
uint32_t OpenGLBackend::GLInternalFormatFromTextureFormat(TextureFormat fmt) const {
    switch (fmt) {
        case TextureFormat::RGBA8:   return GL_RGBA8;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::R8:      return GL_R8;
        default: return GL_RGBA8;
    }
}

uint32_t OpenGLBackend::GLFormatFromTextureFormat(TextureFormat fmt) const {
    switch (fmt) {
        case TextureFormat::RGBA8:   return GL_RGBA;
        case TextureFormat::RGBA32F: return GL_RGBA;
        case TextureFormat::R8:      return GL_RED;
        default: return GL_RGBA;
    }
}

uint32_t OpenGLBackend::GLTypeFromTextureFormat(TextureFormat fmt) const {
    switch (fmt) {
        case TextureFormat::RGBA8:   return GL_UNSIGNED_BYTE;
        case TextureFormat::RGBA32F: return GL_FLOAT;
        case TextureFormat::R8:      return GL_UNSIGNED_BYTE;
        default: return GL_UNSIGNED_BYTE;
    }
}

TextureHandle OpenGLBackend::CreateTexture(int width, int height, TextureFormat fmt,
                                            const void* data) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GLInternalFormatFromTextureFormat(fmt),
                 width, height, 0, GLFormatFromTextureFormat(fmt),
                 GLTypeFromTextureFormat(fmt), data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t id = AllocateTextureId();
    m_textures[id] = tex;
    return { id };
}

void OpenGLBackend::UpdateTexture(TextureHandle handle, int width, int height,
                                   const void* data) {
    auto it = m_textures.find(handle.id);
    if (it == m_textures.end()) return;
    glBindTexture(GL_TEXTURE_2D, it->second);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLBackend::DestroyTexture(TextureHandle handle) {
    auto it = m_textures.find(handle.id);
    if (it != m_textures.end()) {
        glDeleteTextures(1, &it->second);
        m_textures.erase(it);
    }
    // Also delete associated FBO if exists
    auto fbIt = m_framebuffers.find(handle.id);
    if (fbIt != m_framebuffers.end()) {
        glDeleteFramebuffers(1, &fbIt->second);
        m_framebuffers.erase(fbIt);
    }
}
```

- [ ] **Step 7: 实现 DrawFullscreenQuad — 核心全屏渲染函数**

```cpp
void OpenGLBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag,
                                        const ShaderParams& params) {
    // 动态创建 program（VS + FS）
    GLuint vs = GetGLShader(vert);
    GLuint fs = GetGLShader(frag);
    if (!vs || !fs) return;

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        fprintf(stderr, "[OpenGL] Link error: %s\n", log);
        glDeleteProgram(program);
        return;
    }

    glUseProgram(program);

    // 绑定输入纹理到 texture units 0,1,2...
    for (size_t i = 0; i < params.inputTextures.size() && i < 8; i++) {
        GLuint glTex = GetGLTexture(params.inputTextures[i]);
        glActiveTexture(GL_TEXTURE0 + (GLenum)i);
        glBindTexture(GL_TEXTURE_2D, glTex);
    }

    // 设置内置 uniform (location 通过 glGetUniformLocation 或约定 location)
    // 分辨率
    GLint loc = glGetUniformLocation(program, "uResolution");
    if (loc >= 0) glUniform2f(loc, (float)params.viewportWidth, (float)params.viewportHeight);

    // 时间
    loc = glGetUniformLocation(program, "uTime");
    if (loc >= 0) glUniform1f(loc, params.time);

    // 帧序号
    loc = glGetUniformLocation(program, "uFrameCount");
    if (loc >= 0) glUniform1ui(loc, params.frameCount);

    // 设置用户 uniform（使用 glUniform* 系列）
    for (size_t i = 0; i < params.uniformFloats.size(); i++) {
        loc = glGetUniformLocation(program, ("uParamFloat" + std::to_string(i)).c_str());
        if (loc >= 0) glUniform1f(loc, params.uniformFloats[i]);
    }
    for (size_t i = 0; i < params.uniformInts.size(); i++) {
        loc = glGetUniformLocation(program, ("uParamInt" + std::to_string(i)).c_str());
        if (loc >= 0) glUniform1i(loc, params.uniformInts[i]);
    }

    // 绘制全屏四边形
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, kFullscreenQuadVertexCount);
    glBindVertexArray(0);

    glUseProgram(0);
    glDeleteProgram(program);
}
```

- [ ] **Step 8: 实现 Render-to-Texture 和 Blit**

```cpp
void OpenGLBackend::BeginRenderToTexture(TextureHandle target) {
    auto texIt = m_textures.find(target.id);
    if (texIt == m_textures.end()) return;

    // 创建或复用 FBO
    uint32_t fbo;
    auto fbIt = m_framebuffers.find(target.id);
    if (fbIt == m_framebuffers.end()) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, texIt->second, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "[OpenGL] FBO incomplete: 0x%X\n", status);
            glDeleteFramebuffers(1, &fbo);
            return;
        }
        m_framebuffers[target.id] = fbo;
    } else {
        fbo = fbIt->second;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, m_width, m_height); // 假设 RT 与窗口同大小
    glClear(GL_COLOR_BUFFER_BIT);
    m_currentFBO = fbo;
}

void OpenGLBackend::EndRenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_width, m_height);
    m_currentFBO = m_defaultFBO;
}

void OpenGLBackend::BlitToScreen(TextureHandle texture) {
    // 简化：使用全屏 quad + passthrough shader
    // 实际这个函数在 CoverFlow 场景中用于渲染模糊背景
    auto texIt = m_textures.find(texture.id);
    if (texIt == m_textures.end()) return;
    // 后续在 CoverFlow Scene 中实现
}
```

- [ ] **Step 9: 实现 DrawCards stub 和 ImGui 集成 stub**

```cpp
void OpenGLBackend::DrawCards(const std::vector<CardDrawInfo>& cards,
                               const float* viewMatrix, const float* projMatrix) {
    // 远期 CoverFlow 任务中实现
    (void)cards; (void)viewMatrix; (void)projMatrix;
}

// ImGui OpenGL 集成（使用官方 imgui_impl_glfw + imgui_impl_opengl3）
void OpenGLBackend::ImGuiInit(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
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

- [ ] **Step 10: Commit**

```bash
git add src/render/OpenGLBackend.h src/render/OpenGLBackend.cpp src/render/FullscreenQuad.h src/render/FullscreenQuad.cpp
git commit -m "feat: OpenGL 4.6 backend with SPIR-V shader support and ImGui"
```

---

### Task 4: Vulkan 后端实现（骨架）

**Files:**
- Create: `src/render/VulkanBackend.h`
- Create: `src/render/VulkanBackend.cpp`

**注意：** Vulkan 后端是实现工作量最大的部分。在此任务中创建骨架（Init/Shutdown/BeginFrame/EndFrame 和 Swapchain），具体纹理/管线/Pipeline 在后续与第一个 Shader 效果联调时完成。

- [ ] **Step 1: 创建 VulkanBackend.h — 类声明**

```cpp
// src/render/VulkanBackend.h
#pragma once
#include "IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend() = default;
    ~VulkanBackend() override;

    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) const override;

    ShaderHandle CreateVertexShader(const std::vector<uint32_t>& spirv) override;
    ShaderHandle CreateFragmentShader(const std::vector<uint32_t>& spirv) override;
    void DestroyShader(ShaderHandle handle) override;

    TextureHandle CreateTexture(int width, int height, TextureFormat fmt,
                                 const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void UpdateTexture(TextureHandle handle, int width, int height,
                        const void* data) override;

    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag,
                             const ShaderParams& params) override;
    void DrawCards(const std::vector<CardDrawInfo>& cards,
                    const float* viewMatrix, const float* projMatrix) override;
    void BlitToScreen(TextureHandle texture) override;
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
    int m_width = 1280, m_height = 720;
    bool m_initialized = false;

    // Vulkan 核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsFamily = 0;
    uint32_t m_presentFamily = 0;

    // Swapchain
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    uint32_t m_currentImageIndex = 0;

    // Command
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    // Sync
    VkSemaphore m_imageAvailable = VK_NULL_HANDLE;
    VkSemaphore m_renderFinished = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    // Resource pools
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    std::unordered_map<uint32_t, VkShaderModule> m_shaderModules;
    std::unordered_map<uint32_t, VkImageView> m_textureViews;
    std::unordered_map<uint32_t, VkImage> m_textureImages;
    std::unordered_map<uint32_t, VkDeviceMemory> m_textureMemories;

    // Helpers
    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void CleanupSwapchain();
    void RecreateSwapchain();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer cmd);
    void TransitionImageLayout(VkImage image, VkFormat format,
                                VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t w, uint32_t h);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags props, VkBuffer& buffer,
                       VkDeviceMemory& memory);
};
```

- [ ] **Step 2: 实现 VulkanBackend.cpp 核心初始化函数**

```cpp
// src/render/VulkanBackend.cpp
#include "VulkanBackend.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstring>
#include <set>
#include <algorithm>

// ================ Lifecycle ================

bool VulkanBackend::Init(GLFWwindow* window) {
    m_window = window;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();

    m_initialized = true;
    printf("[Vulkan] Initialized\n");
    return true;
}

void VulkanBackend::Shutdown() {
    if (!m_initialized) return;
    WaitIdle();
    ImGuiShutdown();

    // Destroy sync objects
    if (m_imageAvailable) vkDestroySemaphore(m_device, m_imageAvailable, nullptr);
    if (m_renderFinished) vkDestroySemaphore(m_device, m_renderFinished, nullptr);
    if (m_inFlightFence) vkDestroyFence(m_device, m_inFlightFence, nullptr);

    // Destroy command pool
    if (m_commandPool) vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    // Destroy shader modules
    for (auto& [id, module] : m_shaderModules)
        vkDestroyShaderModule(m_device, module, nullptr);
    m_shaderModules.clear();

    // Destroy textures
    for (auto& [id, view] : m_textureViews)
        vkDestroyImageView(m_device, view, nullptr);
    for (auto& [id, image] : m_textureImages)
        vkDestroyImage(m_device, image, nullptr);
    for (auto& [id, mem] : m_textureMemories)
        vkFreeMemory(m_device, mem, nullptr);

    CleanupSwapchain();
    if (m_device) vkDestroyDevice(m_device, nullptr);
    if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_instance) vkDestroyInstance(m_instance, nullptr);
    printf("[Vulkan] Shutdown\n");
}

void VulkanBackend::BeginFrame() {
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);

    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                             m_imageAvailable, VK_NULL_HANDLE,
                                             &m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }

    vkResetCommandBuffer(m_commandBuffer, 0);
    RecordCommandBuffer(); // 空实现，先占位
}

void VulkanBackend::EndFrame() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = { m_imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    VkSemaphore signalSemaphores[] = { m_renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImageIndex;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        RecreateSwapchain();
}

void VulkanBackend::WaitIdle() {
    if (m_device) vkDeviceWaitIdle(m_device);
}

void VulkanBackend::Resize(int width, int height) {
    m_width = width;
    m_height = height;
}

void VulkanBackend::GetFramebufferSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

int VulkanBackend::GetMaxTextureSize() const {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    return (int)props.limits.maxImageDimension2D;
}
```

- [ ] **Step 3: 实现 Instance、Device、Swapchain 创建**

```cpp
void VulkanBackend::CreateInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shader Showcase";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    std::vector<const char*> extensions(glfwExts, glfwExts + glfwExtCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = &validationLayer;
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to create instance\n");
        exit(1);
    }
}

void VulkanBackend::CreateSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to create surface\n");
        exit(1);
    }
}

void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            m_physicalDevice = device;
            printf("[Vulkan] Physical device: %s\n", props.deviceName);
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) m_physicalDevice = devices[0];
}

void VulkanBackend::CreateLogicalDevice() {
    // Find queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount,
                                              queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport) m_presentFamily = i;
    }

    std::set<uint32_t> uniqueFamilies = { m_graphicsFamily, m_presentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo qInfo{};
        qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qInfo.queueFamilyIndex = family;
        qInfo.queueCount = 1;
        qInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(qInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    const char* deviceExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExts;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to create device\n");
        exit(1);
    }

    vkGetDeviceQueue(m_device, m_graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentFamily, 0, &m_presentQueue);
}

void VulkanBackend::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &caps);
    m_width = caps.currentExtent.width;
    m_height = caps.currentExtent.height;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = 2;
    createInfo.imageFormat = m_swapchainFormat;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = caps.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    uint32_t familyIndices[] = { m_graphicsFamily, m_presentFamily };
    if (m_graphicsFamily != m_presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to create swapchain\n");
        exit(1);
    }

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchainImageViews[i]);
    }
}

void VulkanBackend::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
}

void VulkanBackend::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);
}

void VulkanBackend::CreateSyncObjects() {
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device, &semInfo, nullptr, &m_imageAvailable);
    vkCreateSemaphore(m_device, &semInfo, nullptr, &m_renderFinished);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence);
}

void VulkanBackend::CleanupSwapchain() {
    for (auto& fb : m_swapchainFramebuffers)
        vkDestroyFramebuffer(m_device, fb, nullptr);
    for (auto& iv : m_swapchainImageViews)
        vkDestroyImageView(m_device, iv, nullptr);
    if (m_swapchain) vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchainFramebuffers.clear();
    m_swapchainImageViews.clear();
    m_swapchainImages.clear();
}

void VulkanBackend::RecreateSwapchain() {
    WaitIdle();
    CleanupSwapchain();
    CreateSwapchain();
}

// Stub implementations for remaining methods
ShaderHandle VulkanBackend::CreateVertexShader(const std::vector<uint32_t>& spirv) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkShaderModule module;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &module) != VK_SUCCESS)
        return INVALID_SHADER;

    uint32_t id = m_nextShaderId++;
    m_shaderModules[id] = module;
    return { id };
}

ShaderHandle VulkanBackend::CreateFragmentShader(const std::vector<uint32_t>& spirv) {
    return CreateVertexShader(spirv); // same SPIR-V load path
}

void VulkanBackend::DestroyShader(ShaderHandle handle) {
    auto it = m_shaderModules.find(handle.id);
    if (it != m_shaderModules.end()) {
        vkDestroyShaderModule(m_device, it->second, nullptr);
        m_shaderModules.erase(it);
    }
}

// Stubs for drawing (will be implemented in later tasks)
void VulkanBackend::DrawFullscreenQuad(ShaderHandle, ShaderHandle, const ShaderParams&) {}
void VulkanBackend::DrawCards(const std::vector<CardDrawInfo>&, const float*, const float*) {}
void VulkanBackend::BlitToScreen(TextureHandle) {}
void VulkanBackend::BeginRenderToTexture(TextureHandle) {}
void VulkanBackend::EndRenderToTexture() {}

// Texture stubs
TextureHandle VulkanBackend::CreateTexture(int, int, TextureFormat, const void*) {
    return INVALID_TEXTURE;
}
void VulkanBackend::DestroyTexture(TextureHandle) {}
void VulkanBackend::UpdateTexture(TextureHandle, int, int, const void*) {}

// ImGui stub
void VulkanBackend::ImGuiInit(GLFWwindow*) {}
void VulkanBackend::ImGuiNewFrame() {}
void VulkanBackend::ImGuiRender() {}
void VulkanBackend::ImGuiShutdown() {}
```

- [ ] **Step 4: Commit**

```bash
git add src/render/VulkanBackend.h src/render/VulkanBackend.cpp
git commit -m "feat: Vulkan 1.2 backend skeleton with swapchain and SPIR-V loading"
```

---

### Task 5: Application 主应用类与后端切换

**Files:**
- Create: `src/app/Application.h`
- Create: `src/app/Application.cpp`
- Modify: `src/main.cpp`

- [ ] **Step 1: 创建 Application.h**

```cpp
// src/app/Application.h
#pragma once
#include "render/IRenderBackend.h"
#include <memory>
#include <functional>

struct GLFWwindow;

class Application {
public:
    Application();
    ~Application();

    int Run(int argc, char* argv[]);

    // 运行时切换后端（需要重建窗口上下文）
    void SwitchBackend(BackendType type);

    IRenderBackend* GetBackend() const { return m_backend.get(); }
    GLFWwindow* GetWindow() const { return m_window; }

    // 帧回调：在此设置场景渲染逻辑
    using FrameCallback = std::function<void(float dt)>;
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

private:
    bool InitWindow();
    void InitBackend(BackendType type);
    void MainLoop();
    void Shutdown();

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<IRenderBackend> m_backend;
    BackendType m_backendType = BackendType::OpenGL;
    FrameCallback m_frameCallback;
    bool m_running = false;

    static void FramebufferSizeCallback(GLFWwindow* window, int w, int h);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
```

- [ ] **Step 2: 创建 Application.cpp**

```cpp
// src/app/Application.cpp
#include "Application.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <chrono>

#ifdef USE_OPENGL_BACKEND
#include "render/OpenGLBackend.h"
#endif
#ifdef USE_VULKAN_BACKEND
#include "render/VulkanBackend.h"
#endif

Application::Application() {}
Application::~Application() { Shutdown(); }

int Application::Run(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW\n");
        return -1;
    }

    InitBackend(m_backendType);
    MainLoop();
    Shutdown();

    glfwTerminate();
    return 0;
}

void Application::InitBackend(BackendType type) {
    // 先销毁旧后端
    if (m_backend) {
        m_backend->Shutdown();
        m_backend.reset();
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    m_window = glfwCreateWindow(1280, 720, "Shader Showcase",
                                 nullptr, nullptr);
    if (!m_window) {
        fprintf(stderr, "Failed to create window\n");
        exit(1);
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetKeyCallback(m_window, KeyCallback);

    // 创建后端
    switch (type) {
#ifdef USE_OPENGL_BACKEND
        case BackendType::OpenGL:
            m_backend = std::make_unique<OpenGLBackend>();
            break;
#endif
#ifdef USE_VULKAN_BACKEND
        case BackendType::Vulkan:
            m_backend = std::make_unique<VulkanBackend>();
            break;
#endif
        default:
            fprintf(stderr, "Unknown backend type\n");
            exit(1);
    }

    if (!m_backend->Init(m_window)) {
        fprintf(stderr, "Failed to init backend: %s\n", m_backend->GetName());
        exit(1);
    }

    m_backend->ImGuiInit(m_window);
    m_backendType = type;

    char title[256];
    snprintf(title, sizeof(title), "Shader Showcase - %s", m_backend->GetName());
    glfwSetWindowTitle(m_window, title);
}

void Application::SwitchBackend(BackendType type) {
    InitBackend(type);
}

void Application::MainLoop() {
    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && !glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        m_backend->BeginFrame();
        m_backend->ImGuiNewFrame();

        if (m_frameCallback) {
            m_frameCallback(dt);
        }

        m_backend->ImGuiRender();
        m_backend->EndFrame();
    }
}

void Application::Shutdown() {
    if (m_backend) {
        m_backend->ImGuiShutdown();
        m_backend->Shutdown();
        m_backend.reset();
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int w, int h) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && app->m_backend) {
        app->m_backend->Resize(w, h);
    }
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app || action != GLFW_PRESS) return;

    // 数字键切换后端
    if (mods & GLFW_MOD_CONTROL) {
        if (key == GLFW_KEY_1) app->SwitchBackend(BackendType::OpenGL);
        if (key == GLFW_KEY_2) app->SwitchBackend(BackendType::Vulkan);
    }
}
```

- [ ] **Step 3: 更新 main.cpp**

```cpp
// src/main.cpp
#include "app/Application.h"

int main(int argc, char* argv[]) {
    Application app;
    return app.Run(argc, argv);
}
```

- [ ] **Step 4: Commit**

```bash
git add src/app/Application.h src/app/Application.cpp src/main.cpp
git commit -m "feat: Application class with backend switching (Ctrl+1/2)"
```

---

## 里程碑二：Shader 管线

### Task 6: Shader 元数据与 SPIR-V 加载器

**Files:**
- Create: `src/shader/EffectMetadata.h`
- Create: `src/shader/EffectMetadata.cpp`
- Create: `src/shader/ShaderLoader.h`
- Create: `src/shader/ShaderLoader.cpp`

- [ ] **Step 1: 创建 EffectMetadata.h — 效果参数数据结构**

```cpp
- [ ] **Step 1: 创建 EffectMetadata.h/cpp + ShaderLoader.h/cpp（完整代码见设计规格书第9节）**

EffectMetadata 结构体定义：`EffectCard`（id/name/category/description/thumbnailPath/spirvPath）、`ShaderParam`（name/label/type/min/max/default/uiType/comboOptions）、`UniformBinding`。JSON 解析使用 nlohmann/json 或手写简易解析器。ShaderLoader 负责从 .spv 文件读取 `std::vector<uint32_t>`。

- [ ] **Step 2: Commit**

---

### Task 7: 第一个测试 Shader + 顶点 Shader + CMake SPIR-V 编译

**Files:**
- Create: `shaders/common/fullscreen.vert` — 全屏 pass-through 顶点 Shader（GLSL 460）
- Create: `shaders/effects/simple_test/simple_test.frag` — 灰度测试效果
- Create: `shaders/effects/simple_test/effect.json` — 元数据
- Create: `shaders/CMakeLists.txt` — glslangValidator 编译 SPIR-V

- [ ] **fullscreen.vert**:
```glsl
#version 460
layout(location=0) out vec2 vUV;
void main() {
    float x = float((gl_VertexID & 1) << 2) - 1.0;
    float y = float((gl_VertexID & 2) << 1) - 1.0;
    vUV = vec2((x+1.0)*0.5, (y+1.0)*0.5);
    gl_Position = vec4(x, y, 0, 1);
}
```

- [ ] **simple_test.frag**:
```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(location=10) uniform float uIntensity = 0.5;
layout(location=11) uniform vec2 uResolution;

void main() {
    vec3 col = texture(uInputTex, vUV).rgb;
    float gray = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(col, vec3(gray), uIntensity);
    outColor = vec4(col, 1.0);
}
```

- [ ] **effect.json**: `{"name":"灰度测试","category":"Simple","description":"基础灰度效果，验证 Shader 管线是否正常","params":[{"name":"uIntensity","type":"slider","min":0,"max":1,"default":0.5,"label":"灰度强度"}]}`

- [ ] **shaders/CMakeLists.txt**: 添加 `glslangValidator` 编译步骤，产出 .spv 文件到构建目录

- [ ] **Step 3: 验证** — 在 Application::Run 中加载测试图 + simple_test.spv + 调用 DrawFullscreenQuad，确认 OpenGL 端能看到灰度效果

- [ ] **Step 4: Commit**

---

### Task 8: Scene 基类 + EffectDetailScene（全屏效果 + 简介条）

**Files:**
- Create: `src/app/Scene.h` — 虚基类 `OnEnter/OnExit/OnUpdate/OnRender`
- Create: `src/app/EffectDetailScene.h/cpp`
- Modify: `src/main.cpp` — 启动后进入详情场景

- [ ] **Step 1: Scene.h**
```cpp
class Scene {
public:
    virtual ~Scene() = default;
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate(float dt, IRenderBackend* rb) = 0;
    virtual void OnRender(IRenderBackend* rb) = 0;
    virtual void OnImGui() {}
};
```

- [ ] **Step 2: EffectDetailScene** — 持有当前 EffectCard + ShaderHandle(vert)+ShaderHandle(frag)+InputTexture + 底部简介条（ImGui overlay）+ 按 Tab 弹出调参面板。OnRender 中调用 `rb->DrawFullscreenQuad(vert, frag, params)`. OnImGui 中渲染底部半透明简介条和 DebugPanel.

- [ ] **Step 3: main.cpp** 集成测试 — 启动后直接显示 simple_test 效果，验证：全屏灰度效果可见 + 底部半透明条显示标题和简介 + Tab 唤出 ImGui 调参面板

- [ ] **Step 4: Commit**

---

### Task 9: DebugPanel — ImGui 动态调参面板

**Files:**
- Create: `src/ui/DebugPanel.h/cpp`

- [ ] 根据 `EffectMetadata.params` 动态生成 ImGui 控件：SliderFloat、DragFloat、ColorEdit3、Combo、Checkbox。修改参数后立即更新 `ShaderParams.uniformFloats`，下一帧生效。将静态截图替换为实时 rendering。

- [ ] Commit

---

## 里程碑三：CoverFlow 总览页

### Task 10: CoverFlowScene — 卡片数据 + 横向滑动

**Files:**
- Create: `src/app/CoverFlowScene.h/cpp` — 注册全部 18 张 EffectCard，Track 当前选中索引 `m_selectedIndex`，鼠标滚轮/左右方向键翻页，smoothstep 动画插值目标位置

- [ ] 卡片位置计算：中心卡片在 screen (0.5, 0.5)，scale=1.0；两侧卡片按 `offset = (i - center) * cardSpacing` 偏移，scale = 1.0 / (1.0 + abs(offset) * 0.3)，opacity = 1.0 - abs(offset) * 0.4

- [ ] Commit

### Task 11: CoverFlow 3D 卡片渲染（OpenGL 端）

**Files:**
- Modify: `OpenGLBackend.cpp` — 实现 `DrawCards()`
- Create: `shaders/ui/coverflow_card.vert/frag` — 带 3D 变换的卡片 Shader

- [ ] 卡片 Shader 在每个顶点接收 model 矩阵（pos+scale+rotationY），应用 MVP 变换。片段 Shader 采样卡片纹理 + 应用透明度。背景使用当前选中的缩略图 + 高斯模糊（后续优化为实时小窗口渲染）

- [ ] Commit

### Task 12: CoverFlow → EffectDetail 场景切换

**Files:**
- Modify: `Application.h/cpp` — 添加 `SwitchScene(std::unique_ptr<Scene>)`
- 点击事件（Enter 键 / 鼠标点击中间卡片）→ `SwitchScene(new EffectDetailScene(card))`；ESC 返回 CoverFlow

- [ ] Commit

---

## 里程碑四：效果扩展

### Task 13-16: 移植剩余 17 个效果

每 4-5 个效果为一组任务，按类别顺序：Bloom(2) → 色彩(5) → CRT(2)+Tonemap(1) → 其余(CA+DOF+LensFlare+Film+Unsharp+RetroFog+Dither+Vignette). 每个效果：移植 .fx → GLSL 460 .frag → 编写 effect.json 元数据 → 编译 .spv → 注册到 CoverFlowScene 卡片列表 → 测试调参

---

## 里程碑五：输入源

### Task 17: 内置资源 + 用户文件加载

**Files:** Create: `src/input/BuiltinInput.h/cpp`, `src/input/FileInput.h/cpp`, `src/input/InputSource.h`

- [ ] 下载代表性测试图/视频到 `assets/`。stb_image 加载图片 → CreateTexture → 更新 InputTexture。视频解码使用轻量方案（可暂用图片序列模拟）。GLFW 拖放回调触发 FileInput。

### Task 18: 屏幕捕获

**Files:** Create: `src/input/ScreenCapture.h/cpp`

- [ ] Windows DXGI Desktop Duplication → 桌面纹理 → backend CreateTexture/UpdateTexture。跨 API 纹理共享（DXGI → GL 通过 WGL_NV_DX_interop，DXGI → VK 通过 external memory）。

---

## 里程碑六：打磨

### Task 19: 动画优化 + 双后端对比 + 性能

- [ ] CoverFlow 动画使用 ease-out cubic。添加 FPS 显示。完成后端纹理/Buffer/Vulkan Pipeline 中的 stub 实现。添加 VSync 开关和性能统计 overlay。

---

## 执行顺序总结

```
M1: T1(骨架) → T2(接口) → T3(OpenGL) → T4(Vulkan骨架) → T5(Application)
M2: T6(元数据) → T7(测试Shader) → T8(详情场景) → T9(DebugPanel)
M3: T10(数据+滑动) → T11(3D卡片渲染) → T12(场景切换)
M4: T13-T16(效果批量移植)
M5: T17(内置+文件) → T18(屏幕捕获)
M6: T19(打磨)
```

> **总任务数: 19 | 总文件数: ~35 新增文件**