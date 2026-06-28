# Scene Gallery — 场景大厅设计文档

> 日期: 2026-06-24

## 目标

构建一个"场景大厅"作为应用程序唯一入口页面。所有演示场景（后处理、粒子等）以 Steam 库风格展示：左侧分类栏 + 右侧卡片列表。点击卡片进入对应场景，ESC 返回大厅。

当前 CoverFlowScene（18 后处理特效）是第一个注册的场景，后续可无限扩展。

## 架构

```
Application
 └─ SceneGalleryScene (场景大厅 — 唯一根 Scene)
      │
      ├─ 左侧分类栏 (220px 固定宽)
      │    ├─ "全部" (默认选中)
      │    ├─ "后处理"    ← CoverFlowScene
      │    ├─ "粒子系统"   ← 未来
      │    └─ ...
      │
      └─ 右侧卡片列表
           └─ 当前分类下每个场景一行大卡片
                ├─ [缩略图]  场景名称
                ├─ 描述文字
                └─ [进入] 按钮
```

**关键决策：**

- **SceneGalleryScene 是唯一根 Scene** — Application 只持有它
- **子场景独立运行** — 进入后接管全屏，Application::SetScene() 直接切换
- **返回机制** — Scene 基类新增 `WantsReturn()`，Application 主循环检测后切回大厅
- **注册表模式** — `SceneRegistry` 单例管理所有场景元数据 + 工厂函数

## 类设计

### SceneRegistry

```cpp
struct SceneEntry {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string thumbPath;
    std::function<std::unique_ptr<Scene>()> factory;
    bool available = true;
};

class SceneRegistry {
public:
    static SceneRegistry& Instance();
    void Register(SceneEntry entry);
    const std::vector<SceneEntry>& All() const;
    std::vector<std::string> Categories() const;
    std::vector<SceneEntry> ByCategory(const std::string& cat) const;
private:
    std::vector<SceneEntry> m_entries;
};
```

### SceneGalleryScene

```cpp
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
    void DrawCard(float x, float y, float w, const SceneEntry& entry, int index);

    std::string m_activeCategory = "全部";
    std::string m_pendingEnter;       // 待进入场景 id，下一帧切换
    int         m_hoverCard = -1;
    Application* m_app = nullptr;
    float m_sidebarW = 220.0f;
};
```

### Scene 基类改动

```cpp
class Scene {
    // ... 保持不变 ...
    virtual bool WantsReturn() const { return false; }  // 新增：ESC 返回大厅
};
```

### 生命周期

```
main(): 注册所有场景到 SceneRegistry → 创建 SceneGalleryScene → app.SetScene(gallery)

运行时:
  SceneGalleryScene (大厅)
    │ 用户点击 [进入]
    ├→ Application::SetScene(CoverFlowScene::factory())
    │     │
    │     ├ 用户按 ESC
    │     │  → CoverFlowScene::WantsReturn() = true
    │     │  → Application 检测 → 回到 SceneGalleryScene
    │     │
    │     └ (未来: 左上角"返回"按钮同理)
    │
    └ 始终可回到大厅
```

### Application 改动

主循环中检测 `WantsReturn()`：

```cpp
// Application::MainLoop 内，每帧检查
if (m_currentScene && m_currentScene->WantsReturn()) {
    auto gallery = std::make_unique<SceneGalleryScene>();
    gallery->SetApplication(this);
    m_pendingNextScene = std::move(gallery);
}
```

## 注册示例 (main.cpp)

```cpp
SceneRegistry::Instance().Register({
    "post-processing",
    "后处理特效",
    "后处理",
    "18 种 GPU 实时后处理效果：模糊、辉光、故障、CRT…",
    "assets/thumb_post.jpg",
    []() { return std::make_unique<CoverFlowScene>(); },
    true
});

// 未来:
SceneRegistry::Instance().Register({
    "particle-system",
    "GPU 粒子系统",
    "粒子系统",
    "百万级实时粒子模拟",
    "assets/thumb_particles.jpg",
    []() { return std::make_unique<ParticleScene>(); },
    false  // 敬请期待
});
```

## 左侧栏 UI

- 宽度：220px 固定
- 分类列表纵向排列
- 选中项高亮背景 + 左边框强调色
- 每个分类旁显示计数 "(N)"

## 右侧卡片 UI

- 每行高度 ~100px
- 左侧缩略图预览区 (~170×95)
- 右侧：标题 (18px) + 描述 (13px) + [进入] 按钮
- 鼠标悬停：背景微亮
- 不可用场景：[进入] 替换为灰显"敬请期待"

## 改动文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/app/SceneRegistry.h` | 新增 | 场景注册表单例 |
| `src/app/SceneRegistry.cpp` | 新增 | 注册表实现 |
| `src/app/SceneGalleryScene.h` | 新增 | 大厅场景声明 |
| `src/app/SceneGalleryScene.cpp` | 新增 | 大厅场景实现 |
| `src/app/Scene.h` | 修改 | 新增 `WantsReturn()` |
| `src/app/Application.cpp` | 修改 | 主循环检测 WantsReturn |
| `src/app/CoverFlowScene.cpp` | 修改 | ESC 设 WantsReturn=true |
| `src/main.cpp` | 修改 | 注册场景 + 使用 SceneGalleryScene |
| `CMakeLists.txt` | 修改 | 添加新文件 |

## 与现有 PageManager 的关系

之前实现的 `PageManager` 被**替代** — `SceneGalleryScene` 概念更贴合"场景大厅"。
`main.cpp` 中移除 `PageManager` 引用，改为使用 `SceneGalleryScene`。

`PageManager.h/cpp` 保留在仓库中，后续可能用于别的场景内部子页管理。

## 不受影响的部分

- Scene 基类（仅新增一个虚方法）
- IRenderBackend / OpenGLBackend / VulkanBackend
- CoverFlowScene 内部功能
- EffectDetailScene
