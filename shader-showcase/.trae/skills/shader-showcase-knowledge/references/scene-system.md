# 场景系统

## Scene 基类 (src/app/Scene.h)

```cpp
class Scene {
public:
    virtual ~Scene() = default;
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender(IRenderBackend* backend) = 0;
    virtual void OnImGui() = 0;
    virtual bool WantsExit() const = 0;
    virtual bool WantsReturn() const = 0;
    virtual std::unique_ptr<Scene> GetNextScene() = 0;
};
```

## SceneRegistry — 场景注册表

```cpp
// 所有场景在此注册，按类别展示
SceneRegistry::Instance().Register("SceneGallery",   "场景陈列",  "入口");
SceneRegistry::Instance().Register("CoverFlowScene", "后处理效果", "主要");
SceneRegistry::Instance().Register("EffectDetailScene", "详情页", "主要");
```

## PageManager — 页面路由

```
调用链: Application → PageManager::GetNextScene(prevSceneName)
  ├─ CoverFlowScene::WantsReturn() → GetNextScene() → PageManager → SceneGalleryScene
  ├─ CoverFlowScene::GetNextScene() → EffectDetailScene (点击卡片)
  └─ EffectDetailScene::WantsReturn() → GetNextScene() → CoverFlowScene (恢复状态)
```

## 场景间状态传递 (CoverFlowState)

```cpp
struct CoverFlowState {
    std::vector<std::string> imagePool;
    std::vector<std::string> videoPool;
    int selectedIndex;
    Application* app;
    TextureHandle inputTex;
    IRenderBackend* backend;
    bool captureActive, autoTest;
    std::string testImageBaseDir;
};
```

## CoverFlowScene 布局

```
 ┌─────────────────────────────────────────────┐
 │[处理][模糊][像素化][边缘][故障][色彩][暗角][图像]│ ← 分类标签，左上角贴顶
 │              ┌──────┬──────┐                  │
 │              │ 动态  │ ■静态 │                  │ ← 分段控件，居中
 │              └──────┴──────┘                  │
 │                   全屏 Shader 渲染              │
 │                  效果名称 (h*0.60)              │
 │                  描述文字 (h*0.66)              │
 │                  ○ ○ ○ ○ ○ (h*0.80)            │
 │                   3 / 6 (h*0.84)               │
 │              ← → 切换  Enter 调参 (h*0.93)     │
 └─────────────────────────────────────────────┘
```

## EffectDetailScene 布局

```
 ┌─────────────────────────────────────────────┐
 │  ┌──────────┐                                │
 │  │ 参数面板  │  ← 半透明深色背景，左上角       │
 │  │  强度 ●──│  Tab 切换显示/隐藏              │
 │  └──────────┘                                │
 │            全屏 Shader 渲染                   │
 │                                              │
 │              效果名称 (h*0.84)                │
 │              描述文字 (h*0.89)                │
 │         ESC 返回 (h*0.94)                    │
 │        Tab 参数 ｜ C 对比 (h*0.96)           │
 └─────────────────────────────────────────────┘
```
