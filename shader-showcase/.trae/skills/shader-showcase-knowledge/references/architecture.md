# 架构概览

## 场景/页面路由

```
main.cpp → Application (主循环)
  ├─ GLFW 窗口创建 + ImGui 初始化
  ├─ 渲染后端选择 (OpenGL/Vulkan)
  └─ PageManager
       ├─ SceneGalleryScene  (首页：场景选择)
       ├─ CoverFlowScene     (沉浸式轮播)
       └─ EffectDetailScene  (详情页：参数调节)
```

## 渲染后端接口

```cpp
struct IRenderBackend {
    virtual BackendType GetType() = 0;
    virtual void GetFramebufferSize(int& w, int& h) = 0;

    // Shader
    virtual ShaderHandle CreateVertexShader(const void*, size_t) = 0;
    virtual ShaderHandle CreateFragmentShader(const void*, size_t) = 0;

    // Texture
    virtual TextureHandle CreateTexture(int w, int h, TextureFormat, const void*) = 0;
    virtual void DestroyTexture(TextureHandle) = 0;
    virtual void BeginRenderToTexture(TextureHandle) = 0;
    virtual void EndRenderToTexture() = 0;

    // Draw
    virtual void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams&) = 0;

    // ImGui integration
    virtual void* GetImTextureID(TextureHandle) = 0;
};
```

## CoverFlowScene 状态机

```
OnEnter() → 加载 Shader + 创建缩略图 FBO
OnRender() → 全屏 FBO 渲染当前效果
OnImGui() → 3 层 UI 叠加（背景/浮动/交互）
OnUpdate() → 键盘/鼠标导航
WantsReturn() → 返回 SceneGalleryScene
GetNextScene() → 进入 EffectDetailScene
```

## EffectDetailScene 状态机

```
OnEnter() → 加载单个效果 Shader
OnRender() → 直接全屏渲染 / 对比模式
OnImGui() → 浮动信息栏 + DebugPanel 窗口
WantsReturn() → 返回 CoverFlowScene
```
