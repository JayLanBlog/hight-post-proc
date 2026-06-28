# ShaderShowcase 项目知识

C++17 桌面应用，基于 GLFW + ImGui，支持 OpenGL 4.6 / Vulkan 双后端，
用于展示 18 种实时 Shader 后处理效果。

## 项目根目录
`e:\AI\graph\hight-post-proc\shader-showcase`

## 编译
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 源码结构 (src/)
- `app/` — 应用层：Application, CoverFlowScene, EffectDetailScene, SceneGalleryScene, PageManager, LanguageManager
- `render/` — IRenderBackend 抽象 + OpenGLBackend + VulkanBackend + FullscreenQuad
- `shader/` — EffectMetadata (effect.json 解析), ShaderLoader (SPIR-V)
- `ui/` — DebugPanel (ImGui 参数面板), PerformancePanel
- `input/` — ScreenCapture (DXGI), VideoPlayer (FFmpeg)

## Shader 效果 (shaders/effects/)
18 个效果，各含 `.frag` (GLSL) + `.frag.spv` (SPIR-V) + `effect.json` (元数据)

## 详细文档
参阅 `references/` 目录下的架构、效果清册、渲染管线和场景系统文档。
