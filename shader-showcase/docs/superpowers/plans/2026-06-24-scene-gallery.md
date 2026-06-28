# Scene Gallery 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 构建 Steam 库风格的场景大厅，替换 PageManager 作为应用唯一入口

**Architecture:** SceneRegistry 单例注册场景元数据 + 工厂 → SceneGalleryScene 左侧分类栏 + 右侧卡片列表 → 点击进入对应场景，ESC 返回大厅

**Tech Stack:** C++17, ImGui (ImDrawList 自绘), GLFW

---

### Task 1: Scene 基类新增 WantsReturn()

**Files:**
- Modify: `src/app/Scene.h:12-16`

- [ ] **Step 1: 新增虚方法**

```cpp
// Scene.h - 在 WantsExit() 后面新增:
virtual bool WantsReturn() const { return false; }
```

- [ ] **Step 2: 编译验证**

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' 'e:\AI\graph\hight-post-proc\shader-showcase\build\ShaderShowcase.vcxproj' /p:Configuration=Release /v:minimal 2>&1 | Select-Object -Last 3
```

---

### Task 2: SceneRegistry 场景注册表单例

**Files:**
- Create: `src/app/SceneRegistry.h`
- Create: `src/app/SceneRegistry.cpp`

- [ ] **Step 1: SceneRegistry.h**

```cpp
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

class Scene;

struct SceneEntry {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string thumbPath;          // 预览图（暂用 assets/images/ 中的图片）
    std::function<std::unique_ptr<Scene>()> factory;
    bool available = true;
};

class SceneRegistry {
public:
    static SceneRegistry& Instance();

    void Register(SceneEntry entry);
    const std::vector<SceneEntry>& All() const { return m_entries; }
    std::vector<std::string> Categories() const;
    std::vector<SceneEntry> ByCategory(const std::string& cat) const;

private:
    SceneRegistry() = default;
    std::vector<SceneEntry> m_entries;
};
```

- [ ] **Step 2: SceneRegistry.cpp**

```cpp
#include "app/SceneRegistry.h"
#include <algorithm>
#include <set>

SceneRegistry& SceneRegistry::Instance() {
    static SceneRegistry inst;
    return inst;
}

void SceneRegistry::Register(SceneEntry entry) {
    m_entries.push_back(std::move(entry));
}

std::vector<std::string> SceneRegistry::Categories() const {
    std::set<std::string> cats;
    for (auto& e : m_entries)
        cats.insert(e.category);
    return std::vector<std::string>(cats.begin(), cats.end());
}

std::vector<SceneEntry> SceneRegistry::ByCategory(const std::string& cat) const {
    if (cat == "全部") return m_entries;
    std::vector<SceneEntry> result;
    for (auto& e : m_entries)
        if (e.category == cat) result.push_back(e);
    return result;
}
```

- [ ] **Step 3: 编译验证**

---

### Task 3: SceneGalleryScene.h 大厅声明

**Files:**
- Create: `src/app/SceneGalleryScene.h`

- [ ] **Step 1: 写入头文件**

```cpp
#pragma once

#include "app/Scene.h"
#include <string>
#include <vector>

class IRenderBackend;
class Application;

class SceneGalleryScene : public Scene {
public:
    void SetApplication(Application* app) { m_app = app; }

    void OnEnter() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    std::unique_ptr<Scene> GetNextScene() override;

private:
    void DrawSidebar(float w, float h);
    void DrawCardList(float w, float h);

    std::string m_activeCategory = "全部";
    std::string m_pendingEnter;         // 待进入场景 id
    int         m_hoverCard  = -1;
    Application* m_app        = nullptr;
    float m_sidebarW = 220.0f;
};
```

- [ ] **Step 2: 编译验证**

---

### Task 4: SceneGalleryScene.cpp 大厅实现

**Files:**
- Create: `src/app/SceneGalleryScene.cpp`

- [ ] **Step 1: 写入实现文件**

```cpp
#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>

void SceneGalleryScene::OnEnter() {
    printf("[SceneGallery] Entered, %zu scenes registered\n",
           SceneRegistry::Instance().All().size());
}

void SceneGalleryScene::OnUpdate(float /*dt*/) {}
void SceneGalleryScene::OnRender(IRenderBackend* /*backend*/) {}

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) return;

    // 全屏底色
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.10f, 1.0f));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    // ---- 标题 ----
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(ImGui::GetFont(), h * 0.045f,
                ImVec2(w * 0.04f, h * 0.04f),
                IM_COL32(200, 210, 255, 255), "场景选择");

    // ---- 左侧栏 ----
    DrawSidebar(w, h);

    // ---- 右侧卡片列表 ----
    DrawCardList(w, h);

    ImGui::End();
    ImGui::PopStyleColor();
}

void SceneGalleryScene::DrawSidebar(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float sx = 0;
    float sy = h * 0.11f;
    float sw = m_sidebarW;
    float sh = h - sy;

    // 背景
    dl->AddRectFilled(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh),
                      IM_COL32(16, 18, 26, 220));

    auto cats = SceneRegistry::Instance().Categories();

    // "全部" 始终在第一位
    std::vector<std::string> items;
    items.push_back("全部");
    for (auto& c : cats) items.push_back(c);

    float itemH = h * 0.045f;
    float y = sy + 12.0f;

    for (size_t i = 0; i < items.size(); i++) {
        bool active = (items[i] == m_activeCategory);
        float ix = sx + 8.0f;
        float iw = sw - 16.0f;

        // 高亮背景
        if (active) {
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + iw, y + itemH),
                              IM_COL32(55, 60, 90, 200), 4.0f);
            // 左边强调线
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + 3.0f, y + itemH),
                              IM_COL32(140, 150, 220, 255));
        } else if (m_hoverCard == -(int)(i + 1)) {
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + iw, y + itemH),
                              IM_COL32(26, 28, 40, 160), 4.0f);
        }

        // 计数
        char label[128];
        const auto& list = (items[i] == "全部")
            ? SceneRegistry::Instance().All()
            : SceneRegistry::Instance().ByCategory(items[i]);
        snprintf(label, sizeof(label), "%s (%zu)", items[i].c_str(), list.size());

        ImU32 tc = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 170, 200, 220);
        dl->AddText(ImGui::GetFont(), h * 0.019f,
                    ImVec2(ix + 14.0f, y + (itemH - h * 0.019f) * 0.5f),
                    tc, label);

        // 点击
        ImGui::SetCursorScreenPos(ImVec2(ix, y));
        ImGui::InvisibleButton(("##cat_" + std::to_string(i)).c_str(), ImVec2(iw, itemH));
        if (ImGui::IsItemHovered()) m_hoverCard = -(int)(i + 1);
        if (ImGui::IsItemClicked()) m_activeCategory = items[i];

        y += itemH + 4.0f;
    }
}

void SceneGalleryScene::DrawCardList(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float lx = m_sidebarW + 16.0f;
    float ly = h * 0.11f;
    float rw = w - lx - 16.0f;

    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    float cardH = h * 0.12f;
    float gap   = 10.0f;
    float y = ly;

    // 滚动区域
    float totalH = scenes.size() * (cardH + gap);
    float viewH  = h - ly;
    ImGui::SetCursorScreenPos(ImVec2(lx, ly));

    for (size_t i = 0; i < scenes.size(); i++) {
        const auto& e = scenes[i];
        float cy = y + i * (cardH + gap);
        if (cy + cardH < ly || cy > h) continue; // 超出视口跳过

        // 卡片背景
        bool hover = (m_hoverCard == (int)i);
        ImU32 bg = hover ? IM_COL32(35, 40, 60, 240) : IM_COL32(22, 25, 38, 200);
        dl->AddRectFilled(ImVec2(lx, cy), ImVec2(lx + rw, cy + cardH), bg, 6.0f);

        // 缩略图占位 (~170×95)
        float thumbW = cardH * 1.78f;
        float thumbH = cardH - 6.0f;
        dl->AddRectFilled(ImVec2(lx + 8.0f, cy + 3.0f),
                          ImVec2(lx + 8.0f + thumbW, cy + 3.0f + thumbH),
                          IM_COL32(35, 40, 55, 255), 4.0f);
        dl->AddText(ImGui::GetFont(), h * 0.02f,
                    ImVec2(lx + 8.0f + thumbW * 0.3f, cy + thumbH * 0.42f),
                    IM_COL32(120, 130, 160, 160), "Preview");

        // 标题
        float tx = lx + 8.0f + thumbW + 16.0f;
        dl->AddText(ImGui::GetFont(), h * 0.025f,
                    ImVec2(tx, cy + 10.0f),
                    IM_COL32(255, 255, 255, 255), e.name.c_str());

        // 描述
        dl->AddText(ImGui::GetFont(), h * 0.018f,
                    ImVec2(tx, cy + h * 0.05f),
                    IM_COL32(140, 150, 170, 200), e.description.c_str());

        // [进入] 按钮
        float btnW = h * 0.08f;
        float btnH = h * 0.035f;
        float btnX = lx + rw - btnW - 16.0f;
        float btnY = cy + (cardH - btnH) * 0.5f;

        if (e.available) {
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                              IM_COL32(72, 78, 120, 255), 4.0f);
            dl->AddText(ImGui::GetFont(), h * 0.018f,
                        ImVec2(btnX + btnW * 0.22f, btnY + btnH * 0.18f),
                        IM_COL32(255, 255, 255, 255), "进入");
        } else {
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                              IM_COL32(35, 38, 50, 200), 4.0f);
            dl->AddText(ImGui::GetFont(), h * 0.015f,
                        ImVec2(btnX + btnW * 0.12f, btnY + btnH * 0.2f),
                        IM_COL32(100, 100, 120, 180), "敬请期待");
        }

        // 点击
        ImGui::SetCursorScreenPos(ImVec2(lx, cy));
        ImGui::InvisibleButton(("##card_" + std::to_string(i)).c_str(),
                               ImVec2(rw, cardH));
        if (ImGui::IsItemHovered()) m_hoverCard = (int)i;
        if (ImGui::IsItemClicked() && e.available) {
            m_pendingEnter = e.id;
        }
    }
}

std::unique_ptr<Scene> SceneGalleryScene::GetNextScene() {
    if (m_pendingEnter.empty()) return nullptr;

    const auto& all = SceneRegistry::Instance().All();
    for (auto& e : all) {
        if (e.id == m_pendingEnter && e.factory) {
            printf("[SceneGallery] Entering scene: %s\n", e.name.c_str());
            return e.factory();
        }
    }
    return nullptr;
}
```

- [ ] **Step 2: 编译验证**

---

### Task 5: Application 主循环检测 WantsReturn

**Files:**
- Modify: `src/app/Application.cpp:265-293`

- [ ] **Step 1: 在 OnUpdate/OnRender 之前添加返回检测**

```cpp
// 在 Application::MainLoop 内，m_currentScene->OnUpdate(dt) 之前添加:
if (m_currentScene && m_currentScene->WantsReturn()) {
    printf("[Application] Scene requested return to gallery\n");
    m_currentScene->OnExit();
    m_currentScene.reset();
    auto gallery = std::make_unique<SceneGalleryScene>();
    gallery->SetApplication(this);
    m_currentScene = std::move(gallery);
    m_currentScene->OnEnter();
}
```

需要添加的 include:
```cpp
#include "app/SceneGalleryScene.h"  // 在 Application.cpp 顶部
```

- [ ] **Step 2: 编译验证**

---

### Task 6: CoverFlowScene ESC 改为 WantsReturn

**Files:**
- Modify: `src/app/CoverFlowScene.h:80` (新增成员)
- Modify: `src/app/CoverFlowScene.cpp:258-267` (修改 ESC 行为)

- [ ] **Step 1: CoverFlowScene.h 新增成员**

```cpp
// 在 m_wantsExit 旁边新增:
bool m_wantsReturn = false;  // ESC 返回大厅（而非退出程序）
```

- [ ] **Step 2: CoverFlowScene.cpp 新增 override**

```cpp
// 在 public 区域或析构函数后新增:
bool CoverFlowScene::WantsReturn() const { return m_wantsReturn; }
```

- [ ] **Step 3: 修改 ESC 处理逻辑**

CoverFlowScene 通过 `GetNextScene()` 切换到 EffectDetailScene（不是嵌套子场景），
因此 ESC 直接触发返回画廊即可。EffectDetailScene 内部自行处理 ESC 返回 CoverFlow。

定位到 `CoverFlowScene.cpp:264` 行附近的 ESC 检测：

```cpp
// 修改前:
if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    printf("[CoverFlowScene] ESC detected, exiting\n");
    m_wantsExit = true;
}

// 修改后:
if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    printf("[CoverFlowScene] ESC detected, returning to gallery\n");
    m_wantsReturn = true;
}
```

- [ ] **Step 4: 编译验证**

---

### Task 7: main.cpp 注册场景 + 使用 SceneGalleryScene

**Files:**
- Modify: `src/main.cpp:4-5, 209-212`

- [ ] **Step 1: 修改 includes**

```cpp
// 移除: #include "app/PageManager.h"
// 新增:
#include "app/SceneRegistry.h"
#include "app/SceneGalleryScene.h"
```

注意：删除 PageManager include，保留 CoverFlowScene include。

- [ ] **Step 2: 修改启动代码**

```cpp
// 在 app.SetScene 之前注册场景：
SceneRegistry::Instance().Register({
    "post-processing",
    "后处理特效",
    "后处理",
    "18 种 GPU 实时后处理效果：模糊、辉光、故障、CRT、卡通着色…",
    "assets/images/00_grayscale_landscape.jpg",
    [&coverFlow]() { return std::move(coverFlow); },
    true
});

auto gallery = std::make_unique<SceneGalleryScene>();
gallery->SetApplication(&app);
app.SetScene(std::move(gallery));
printf("[main] SceneGalleryScene started\n");
```

注意：`coverFlow` 是 `std::unique_ptr`，lambda 需要用 `[cf = std::move(coverFlow)]() mutable { return std::move(cf); }` 方式捕获。重构 main.cpp 的 backend init callback 结构：

```cpp
// 先创建 coverFlow（在 lambda 内初始化），再把所有权交给 registry
std::unique_ptr<CoverFlowScene> coverFlow;

app.SetAfterInit([&]() {
    coverFlow = std::make_unique<CoverFlowScene>();
    coverFlow->SetApplication(&app);
    coverFlow->SetTestImageBaseDir("assets/images");
    coverFlow->LoadImagePoolDirectory("assets/images");
    coverFlow->LoadVideoPoolDirectory("assets/videos");
    // ... 其他初始化 ...

    SceneRegistry::Instance().Register({
        "post-processing",
        "后处理特效",
        "后处理",
        "18 种 GPU 实时后处理效果演示",
        "assets/images/00_grayscale_landscape.jpg",
        [cf = std::move(coverFlow)]() mutable { return std::move(cf); },
        true
    });

    auto gallery = std::make_unique<SceneGalleryScene>();
    gallery->SetApplication(&app);
    app.SetScene(std::move(gallery));
});
```

- [ ] **Step 3: 编译验证**

---

### Task 8: CMakeLists.txt 添加新文件

**Files:**
- Modify: `CMakeLists.txt:158-159`

- [ ] **Step 1: 添加新源文件**

```cmake
# 在 src/app/Scene.h 后面插入:
    src/app/SceneRegistry.cpp
    src/app/SceneRegistry.h
    src/app/SceneGalleryScene.cpp
    src/app/SceneGalleryScene.h
```

- [ ] **Step 2: 编译验证**

---

### 编译 + 运行

```powershell
Stop-Process -Name ShaderShowcase -Force -ErrorAction SilentlyContinue
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' 'e:\AI\graph\hight-post-proc\shader-showcase\build\ShaderShowcase.vcxproj' /p:Configuration=Release /v:minimal 2>&1 | Select-Object -Last 3
Start-Process 'e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release\ShaderShowcase.exe'
```

**预期行为：**
1. 启动 → 场景大厅，左侧"全部(1)"/"后处理(1)"，右侧一张"后处理特效"卡片
2. 点击"进入" → CoverFlowScene 全屏
3. 按 ESC → 返回场景大厅
