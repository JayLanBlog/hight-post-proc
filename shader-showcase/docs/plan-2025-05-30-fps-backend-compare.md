# FPS性能显示 + 后端切换 + 原图对比 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现右上角性能面板（后端切换 + FPS显示）和详情页滑动对比视图

**架构:** 新增 `PerformancePanel` 类全局渲染性能信息，修改 `EffectDetailScene` 实现滑动对比渲染逻辑

**技术栈:** C++17, OpenGL 4.6, ImGui, GLFW

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `src/ui/PerformancePanel.h` | 性能面板类声明 |
| `src/ui/PerformancePanel.cpp` | 性能面板渲染实现 |
| `src/app/Application.cpp:280-290` | 在OnImGui中调用PerformancePanel渲染 |
| `src/app/EffectDetailScene.h:70-75` | 添加滑动对比相关成员变量 |
| `src/app/EffectDetailScene.cpp:300-500` | 修改渲染流程实现滑动对比 |
| `CMakeLists.txt:140-145` | 添加PerformancePanel到源文件列表 |

---

## Task 1: 创建 PerformancePanel 类

**Files:**
- Create: `src/ui/PerformancePanel.h`
- Create: `src/ui/PerformancePanel.cpp`

### Step 1.1: 编写头文件

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
    float m_lastFps = 0.0f;
    int m_fpsFrameCount = 0;
    float m_fpsElapsed = 0.0f;
    
    void UpdateFPS();
    void RenderBackendButtons(Application* app);
    void RenderFPSDisplay();
};
```

### Step 1.2: 编写实现文件

```cpp
#include "PerformancePanel.h"
#include "app/Application.h"
#include "render/IRenderBackend.h"
#include <imgui.h>

void PerformancePanel::Render(Application* app, IRenderBackend* backend) {
    UpdateFPS();
    
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float panelWidth = 100.0f;
    float panelHeight = 70.0f;
    ImVec2 pos(windowSize.x - panelWidth - 8.0f, 8.0f);
    
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar;
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    
    if (ImGui::Begin("##PerformancePanel", nullptr, flags)) {
        RenderBackendButtons(app);
        RenderFPSDisplay();
    }
    ImGui::End();
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void PerformancePanel::UpdateFPS() {
    float dt = ImGui::GetIO().DeltaTime;
    m_fpsElapsed += dt;
    m_fpsFrameCount++;
    
    if (m_fpsElapsed >= 1.0f) {
        m_lastFps = m_fpsFrameCount / m_fpsElapsed;
        m_fpsFrameCount = 0;
        m_fpsElapsed = 0.0f;
    }
}

void PerformancePanel::RenderBackendButtons(Application* app) {
    BackendType current = app->GetBackendType();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    
    // OpenGL button
    bool isOpenGL = (current == BackendType::OpenGL);
    if (isOpenGL) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    }
    if (ImGui::Button("GL", ImVec2(32, 20))) {
        if (!isOpenGL) app->SwitchBackend(BackendType::OpenGL);
    }
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    
    // Vulkan button
    bool isVulkan = (current == BackendType::Vulkan);
#ifdef USE_VULKAN_BACKEND
    if (isVulkan) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    }
    if (ImGui::Button("VK", ImVec2(32, 20))) {
        if (!isVulkan) app->SwitchBackend(BackendType::Vulkan);
    }
    ImGui::PopStyleColor();
#else
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
    ImGui::Button("VK", ImVec2(32, 20));
    ImGui::PopStyleColor(2);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Vulkan not available");
    }
#endif
    
    ImGui::PopStyleVar();
}

void PerformancePanel::RenderFPSDisplay() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.33f, 1.0f, 0.33f, 1.0f));
    ImGui::Text("%.1f FPS", m_lastFps);
    ImGui::PopStyleColor();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 0.53f, 0.53f, 1.0f));
    ImGui::TextUnformatted("OpenGL 4.6");
    ImGui::PopStyleColor();
}
```

### Step 1.3: 添加到 CMakeLists.txt

在 `src/ui/DebugPanel.cpp` 行附近添加：

```cmake
    src/ui/PerformancePanel.cpp
```

### Step 1.4: 提交

```bash
git add src/ui/PerformancePanel.h src/ui/PerformancePanel.cpp CMakeLists.txt
git commit -m "feat: add PerformancePanel class for backend switch and FPS display"
```

---

## Task 2: 在 Application 中集成 PerformancePanel

**Files:**
- Modify: `src/app/Application.h:45-50` - 添加成员变量
- Modify: `src/app/Application.cpp:280-290` - 在 OnImGui 中渲染

### Step 2.1: 修改 Application.h

在 `private:` 区域添加：

```cpp
#include "ui/PerformancePanel.h"
// ... 其他成员 ...
PerformancePanel m_perfPanel;
```

### Step 2.2: 修改 Application.cpp 的 OnImGui

找到 `void Application::OnImGui()` 方法，在开头添加：

```cpp
void Application::OnImGui() {
    // 渲染性能面板（全局可见）
    m_perfPanel.Render(this, m_backend.get());
    
    // ... 原有代码 ...
}
```

### Step 2.3: 添加必要的 include

在 Application.cpp 顶部添加：

```cpp
#include "ui/PerformancePanel.h"
```

### Step 2.4: 提交

```bash
git add src/app/Application.h src/app/Application.cpp
git commit -m "feat: integrate PerformancePanel into Application"
```

---

## Task 3: 实现详情页滑动对比视图

**Files:**
- Modify: `src/app/EffectDetailScene.h:70-75` - 添加成员变量
- Modify: `src/app/EffectDetailScene.cpp:300-500` - 修改渲染流程

### Step 3.1: 修改 EffectDetailScene.h

在 `private:` 区域添加/修改：

```cpp
// 滑动对比
float m_compareSplit = 0.5f;  // 分割线位置 (0.0 ~ 1.0)
bool m_compareDragging = false;
bool m_compareMode = true;    // 默认开启对比模式

// 渲染对比视图
void RenderCompareView(IRenderBackend* backend);
void RenderFullscreenEffect(IRenderBackend* backend);
```

### Step 3.2: 修改 EffectDetailScene.cpp 的 OnRender

替换原有 `OnRender` 方法：

```cpp
void EffectDetailScene::OnRender(IRenderBackend* backend) {
    // 同步参数
    m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
    
    if (m_compareMode) {
        RenderCompareView(backend);
    } else {
        RenderFullscreenEffect(backend);
    }
}
```

### Step 3.3: 添加新的渲染方法

在 `EffectDetailScene.cpp` 中添加：

```cpp
void EffectDetailScene::RenderFullscreenEffect(IRenderBackend* backend) {
    ShaderParams params;
    params.inputTextures.push_back(m_inputTex);
    params.uniformFloats = m_uniformFloats;
    params.uniformInts = m_uniformInts;
    params.time = m_time;
    params.frameCount = m_frameCount;
    params.viewportWidth = m_viewportWidth;
    params.viewportHeight = m_viewportHeight;
    
    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
}

void EffectDetailScene::RenderCompareView(IRenderBackend* backend) {
    // 1. 先渲染效果图到 FBO
    if (m_effectTex.id == 0) {
        TextureDesc desc;
        desc.width = m_viewportWidth;
        desc.height = m_viewportHeight;
        desc.format = TextureFormat::RGBA8;
        m_effectTex = backend->CreateTexture(desc);
    }
    
    backend->BeginRenderToTexture(m_effectTex);
    RenderFullscreenEffect(backend);
    backend->EndRenderToTexture();
    
    // 2. ImGui 将处理对比视图的显示（在 OnImGui 中）
}
```

### Step 3.4: 修改 OnImGui 实现滑动对比

在 `OnImGui` 方法中，找到对比视图渲染部分，替换为：

```cpp
// 计算显示区域
ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
ImVec2 contentPos = ImGui::GetWindowPos();
float viewX = contentPos.x + contentMin.x;
float viewY = contentPos.y + contentMin.y;
float viewW = contentMax.x - contentMin.x;
float viewH = contentMax.y - contentMin.y;

// 减去底部信息栏高度
viewH -= 60.0f;

// 获取纹理 ID
void* inputTexId = nullptr;
void* effectTexId = nullptr;
if (auto* gl = dynamic_cast<OpenGLBackend*>(backend)) {
    inputTexId = gl->GetImTextureID(m_inputTex);
    effectTexId = gl->GetImTextureID(m_effectTex);
}

if (m_compareMode && inputTexId && effectTexId) {
    // 滑动对比模式
    float splitX = viewX + viewW * m_compareSplit;
    
    // 绘制原图（全区域，作为底层）
    ImGui::SetCursorPos(ImVec2(contentMin.x, contentMin.y));
    ImGui::Image(inputTexId, ImVec2(viewW, viewH));
    
    // 绘制效果图（仅右侧区域）
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 uv0(0.0f, 0.0f);
    ImVec2 uv1(1.0f, 1.0f);
    
    // 计算效果图裁剪区域
    float effectU0 = m_compareSplit;
    float effectU1 = 1.0f;
    
    drawList->AddImage(effectTexId,
        ImVec2(splitX, viewY),
        ImVec2(viewX + viewW, viewY + viewH),
        ImVec2(effectU0, 0.0f),
        ImVec2(effectU1, 1.0f),
        IM_COL32(255, 255, 255, 255));
    
    // 绘制分割线
    drawList->AddLine(
        ImVec2(splitX, viewY),
        ImVec2(splitX, viewY + viewH),
        IM_COL32(68, 175, 255, 255),  // #4af
        3.0f);
    
    // 绘制拖拽手柄
    float handleRadius = 14.0f;
    ImVec2 handleCenter(splitX, viewY + viewH * 0.5f);
    drawList->AddCircleFilled(handleCenter, handleRadius, IM_COL32(68, 175, 255, 255));
    drawList->AddText(handleCenter + ImVec2(-6, -7), IM_COL32(255, 255, 255, 255), "\u27f7"); // ⟺
    
    // 区域标签
    drawList->AddText(ImVec2(viewX + viewW * 0.25f - 20, viewY + viewH * 0.5f), 
        IM_COL32(255, 255, 255, 128), "Before");
    drawList->AddText(ImVec2(viewX + viewW * 0.75f - 20, viewY + viewH * 0.5f), 
        IM_COL32(255, 255, 255, 128), "After");
    
    // 处理拖拽交互
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool mouseInHandle = (std::abs(mousePos.x - splitX) < handleRadius + 5.0f) &&
                         (std::abs(mousePos.y - handleCenter.y) < handleRadius + 20.0f);
    
    if (mouseInHandle || m_compareDragging) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }
    
    if (ImGui::IsMouseClicked(0) && mouseInHandle) {
        m_compareDragging = true;
    }
    
    if (m_compareDragging) {
        if (ImGui::IsMouseDown(0)) {
            float newSplit = (mousePos.x - viewX) / viewW;
            m_compareSplit = std::max(0.1f, std::min(0.9f, newSplit));
        } else {
            m_compareDragging = false;
        }
    }
} else {
    // 全屏效果图模式
    ImGui::SetCursorPos(ImVec2(contentMin.x, contentMin.y));
    if (effectTexId) {
        ImGui::Image(effectTexId, ImVec2(viewW, viewH));
    }
}
```

### Step 3.5: 修改键盘处理

在 `OnUpdate` 或键盘处理中，确保 `C` 键切换对比模式：

```cpp
// 在键盘处理中
if (key == GLFW_KEY_C) {
    m_compareMode = !m_compareMode;
}
```

### Step 3.6: 提交

```bash
git add src/app/EffectDetailScene.h src/app/EffectDetailScene.cpp
git commit -m "feat: implement slide comparison view in EffectDetailScene"
```

---

## Task 4: 编译测试

### Step 4.1: 配置 CMake

```bash
cd e:\AI\graph\hight-post-proc\shader-showcase\build
cmake ..
```

### Step 4.2: 编译 Release

```bash
cmake --build . --config Release
```

### Step 4.3: 运行测试

```bash
.\bin\Release\ShaderShowcase.exe
```

验证：
- [ ] 右上角显示性能面板（GL/VK按钮 + FPS）
- [ ] 点击按钮可切换后端
- [ ] 详情页默认显示滑动对比
- [ ] 可拖拽分割线
- [ ] 按 C 键切换全屏/对比模式

### Step 4.4: 提交

```bash
git add docs/plan-2025-05-30-fps-backend-compare.md
git commit -m "docs: add implementation plan for FPS/backend/compare features"
```

---

## 自检清单

| 需求 | 实现位置 |
|------|----------|
| 右上角浮动面板 | `PerformancePanel::Render()` |
| 后端切换按钮 | `PerformancePanel::RenderBackendButtons()` |
| FPS 显示 | `PerformancePanel::RenderFPSDisplay()` |
| 滑动对比视图 | `EffectDetailScene::RenderCompareView()` |
| 可拖拽分割线 | `EffectDetailScene::OnImGui()` 拖拽处理 |
| C 键切换模式 | 键盘处理中切换 `m_compareMode` |
