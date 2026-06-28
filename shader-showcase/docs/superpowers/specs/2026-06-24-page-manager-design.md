# PageManager — 页面管理器设计文档

> 日期: 2026-06-24

## 目标

为 Shader Showcase 添加一个页面管理器，支持多页切换管理。每页是一个独立 Scene，可展示不同场景的效果案例。CoverFlowScene（18 后处理特效）作为第一页，后续可添加粒子、光线追踪等页面。

## 架构

```
Application
 └─ PageManager (Scene 子类)          ← "根Scene"，Application 只持有这一个
      ├─ 底部导航栏 UI                 ← OnImGui 中绘制
      └─ m_pages[]                      ← 多个子 Scene
           ├─ CoverFlowScene (page 0)   ← 18 特效
           ├─ (page N)                  ← 未来扩展
           └─ ...
```

关键决策：
- **PageManager 是 Scene**：复用现有 Application 的场景切换机制
- **子页面也是 Scene**：接口不变，`OnEnter/OnUpdate/OnRender/OnImGui` 完全复用
- **底部导航栏**：紧凑、不遮挡内容，适合页面数 < 10 的场景

## 类设计

### PageManager
```cpp
class PageManager : public Scene {
public:
    struct PageInfo {
        std::string name;               // 底部按钮显示名
        std::unique_ptr<Scene> scene;   // 页面实例
    };

    void AddPage(const std::string& name, std::unique_ptr<Scene> page);
    void SwitchTo(int index);
    int  CurrentIndex() const;

    // Scene interface
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    std::unique_ptr<Scene> GetNextScene() override;

private:
    void DrawNavBar();                     // 底部导航栏

    std::vector<PageInfo> m_pages;
    int m_currentIndex = 0;
    IRenderBackend* m_backend = nullptr;
    float m_navHeight = 48.0f;             // 导航栏高度（比例化）
};
```

### 生命周期

```
SwitchTo(newIdx):
  1. 旧页 m_pages[old].scene->OnExit()
  2. m_currentIndex = newIdx
  3. 新页 m_pages[new].scene->OnEnter()
```

### 渲染流程

```
PageManager::OnUpdate(dt)  → m_pages[m_currentIndex].scene->OnUpdate(dt)
PageManager::OnRender(be)  → m_pages[m_currentIndex].scene->OnRender(be)
PageManager::OnImGui()     → 子页面 OnImGui() + 底部导航栏
```

子页面的 ImGui 内容区自动避开导航栏区域（通过调整 Begin 窗口高度）。

### GetNextScene 处理

子页面可能通过 `GetNextScene()` 请求切换到其他 Scene（如 CoverFlow → EffectDetail）。
PageManager 拦截此调用：如果返回非空，PageManager 将子场景替换注入到当前页。

## main.cpp 改动

```cpp
// 旧：直接创建 CoverFlowScene
auto coverFlow = std::make_unique<CoverFlowScene>();
app.SetScene(std::move(coverFlow));

// 新：通过 PageManager 创建
auto pm = std::make_unique<PageManager>();
pm->AddPage("后处理", std::make_unique<CoverFlowScene>());
// pm->AddPage("粒子",   std::make_unique<ParticleScene>());   // 未来
app.SetScene(std::move(pm));
```

## 导航栏 UI

- 位置：窗口底部
- 高度：`h * 0.065`（比例化）
- 按钮：ImGui Button 或自绘圆角矩形 + 文字
- 激活页高亮色，非激活页暗色
- 右侧 "+" 按钮位置预留（未来添加页面功能）

## 改动文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/app/PageManager.h` | 新增 | PageManager 声明 |
| `src/app/PageManager.cpp` | 新增 | PageManager 实现 |
| `src/main.cpp` | 修改 | PageManager 替代直接 CoverFlowScene |
| `CMakeLists.txt` | 修改 | 添加新文件 |

## 不受影响的部分

- Scene 基类
- Application
- CoverFlowScene / EffectDetailScene
- IRenderBackend / OpenGLBackend / VulkanBackend
