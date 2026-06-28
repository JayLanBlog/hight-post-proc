# ShaderShowcase 开发经验总结

> 基于本次会话中遇到的所有技术问题和解决方案

---

## 1. Shader 跨后端兼容性

### 问题：push_constant 在 OpenGL/NVIDIA 上不生效

**症状**：17 个 XPL Glitch shader 全部渲染不出效果，画面看起来像什么都没发生。

**根因**：XPL shader 使用了 Vulkan 风格的 `layout(push_constant) uniform PC { ... }`。NVIDIA GL 驱动将 push_constant 转成 UBO，但块名是 `pc`（小写），而引擎的 `OpenGLBackend::DrawFullscreenQuad()` 只向名为 `Params`（大写 P）的 UBO（binding=1）写入数据。数据从未到达 shader，所有参数为 0/垃圾值。

**教训**：
- 跨后端（OpenGL + Vulkan）的 shader 必须使用 **双方都支持的 uniform 传递方式**
- OpenGL 路径强制要求 `layout(std140, binding=1) uniform Params { ... }`
- 不要假设 SPIR-V 编译器会自动桥接 push_constant 和 UBO

---

## 2. std140 UBO 内存布局

### 问题：参数数量不足 6 个时数据偏移错位

**症状**：把 push_constant 改为 Params UBO 后仍然无效。

**根因**：引擎固定向 UBO 写入 6 个 float（byte 0~23）+ `uResolution`（byte 24~31）+ `uTime`（byte 32~35）+ `uFrameCount`（byte 36~39）。但 std140 布局中，如果只声明 1 个 float 参数，`uResolution`（vec2）会紧接其后从 byte 4 开始，造成所有后续数据全部偏移。

**修复**：不足 6 个参数时，用 `_pad0.._padN` 补齐到恰好 6 个 float：
```glsl
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // byte 0
    float _pad0;          // byte 4
    float _pad1;          // byte 8
    float _pad2;          // byte 12
    float _pad3;          // byte 16
    float _pad4;          // byte 20
    vec2  uResolution;    // byte 24  ← 必须对齐这里
    float uTime;          // byte 32
    float uFrameCount;    // byte 36
};
```

**教训**：
- std140 规则严格：`float`=4B，`vec2`=8B 且 8 字节对齐
- 写入端（C++ UBO buffer）和读取端（GLSL layout）的 memcpy 偏移必须逐字节匹配
- 两端不一致时不会报错，只会默默读到错位的数据

---

## 3. 显示名称的数据源

### 问题：改了 effect.json 和 CARD 宏，界面仍然显示英文

**症状**：轮播页卡片名始终是 "Glitch Screen Jump" 等英文，不是中文。

**根因**：`CoverFlowScene` 和 `EffectDetailScene` 的显示文本来自 `LanguageManager::CardName()` / `CardDesc()` 硬编码查找表，而不是 CARD 宏或 effect.json。该表有 3 个字段：`{id, en, zh}`。

**修复链路**：
1. `effect.json` — 元数据存储（仅供 DetailScene DebugPanel 读取 params）
2. `CARD("id", "name", ...)` — 仅构建 EffectCard 结构体（内部索引用）
3. `LanguageManager::CardName(id)` — **真正的显示源**，必须同步更新中文条目

**教训**：修改显示文本前，先 grep `CardName` / `CardDesc` 找到真正渲染文字的地方。

---

## 4. 自动化测试的环境隔离

### 问题：AUTO_TEST_CARDS 启动后进入错误的场景

**症状**：设置 `AUTO_TEST_CARDS=1` 后程序进入 `SceneGalleryScene`（首页），而不是 `CoverFlowScene`（轮播页）。

**根因**：`main.cpp` 默认通过 SceneRegistry + SceneGalleryScene 路由，即使用户想直接进 CoverFlowScene 截图。

**修复**：
```cpp
if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI")) {
    auto cs = std::move(*cfShared);
    cs->SetApplication(&app);
    app.SetScene(std::move(cs));  // 跳过 gallery，直接进轮播页
}
```

还需要绕过 Dynamic/Static 隔离（XPL 效果只在 Dynamic 池）：
```cpp
if (getenv("AUTO_TEST_CARDS")) {
    m_filteredIndices.clear();
    for (int i = 0; i < (int)m_cards.size(); i++)
        m_filteredIndices.push_back(i);
}
```

以及强切 OpenGL 后端（Vulkan 不支持 SaveScreenshot）：
```cpp
if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI")) {
    m_backendType = BackendType::OpenGL;
}
```

**教训**：
- 自动化模式需要三层绕过：场景路由 → 数据池隔离 → 渲染后端
- 用环境变量标记 + `getenv` 分支，不要改动正常模式逻辑

---

## 5. 截图验证的局限性

### 问题：像素分析无法区分 shader 是否生效

**症状**：用 Python 读取 PPM 截图分析像素方差，17 个效果全部显示完全相同的数值（variance=98, rgb_avg 一致）。

**根因**：截图保存的是包含 ImGui UI 叠加层的完整 framebuffer（分类标签栏、分段控件、文字、页码点、箭头），这些 UI 元素占据了大量像素且对所有效果都一样。shader 实际只渲染在 FBO 纹理上，然后作为 ImGui Image 叠加在 UI 后面。

**正确验证方式**：
- 肉眼观察运行中的应用
- 或修改截图逻辑只保存 FBO 纹理（不含 UI 叠加）

**教训**：自动化像素分析前，确认截图是否包含 UI chrome。shader 效果验证最终靠人眼。

---

## 6. 进程残留问题

### 问题：编译时 LNK1104 无法写入 exe

**症状**：`fatal error LNK1104: 无法打开文件 ShaderShowcase.exe`

**根因**：之前的多次自动化测试或手动启动未正确退出，遗留多个 ShaderShowcase 进程持有 exe 文件锁。

**修复**：每次编译前 `taskkill /F /IM ShaderShowcase.exe`

**教训**：
- GUI 应用的 build 脚本第一行必须是 kill 旧进程
- 自动化测试结束时确保 `Application::MainLoop` 正常退出并调用 `glfwTerminate`

---

## 7. 工作目录与资源路径

### 问题：SceneGalleryScene 的 Hero 和 Grid 卡片全是黑色

**症状**：`stbi_load("assets/images/00_grayscale_landscape.jpg")` 返回 NULL，因为启动目录不是项目根目录。

**根因**：从 VS 调试器、文件管理器直接双击、或 PowerShell 的任意位置 `Start-Process` 启动时，进程的工作目录各不同。`SceneRegistry` 中硬编码的相对路径 `"assets/images/xxx.jpg"` 依赖 CWD。

**修复**：在 `main()` 入口处，通过 exe 路径反向查找项目根目录并 `_chdir()`：
```cpp
// 从 exe 所在目录向上查找包含 assets/images/00_*.jpg 的目录
for (const char* up : {"../../..", "../..", "..", "."}) {
    if (fopen((exeDir + "/" + up + "/assets/images/00_grayscale_landscape.jpg").c_str(), "rb")) {
        _chdir(exeDir + "/" + up);
        break;
    }
}
```

**教训**：
- 桌面应用启动时主动 `chdir` 到已知根目录
- 不要在代码中依赖"用户从哪个目录启动"

---

## 8. 速查清单

| 问题 | 排查方向 | 关键词 |
|------|---------|--------|
| shader 无效 | UBO 布局/Padding | `push_constant` → `std140 Params` |
| 显示名称错误 | LanguageManager | `CardName` / `CardDesc` |
| 运行黑屏/无预览 | 工作目录 | `_chdir` / `FindAssetDir` |
| 编译 LNK1104 | 旧进程残留 | `taskkill` |
| 自动化入口错 | 场景路由 | `AUTO_TEST_CARDS` |
| 像素分析误判 | UI 叠加层 | framebuffer vs FBO |

---

## 9. 文件修改点总结

| 文件 | 改动 |
|------|------|
| `shaders/effects/xpl_glitch_*/*.frag` | push_constant → std140 Params + padding |
| `CoverFlowScene.cpp` | CARD 名中文化、reserve(35)、AUTO_TEST 绕过隔离 |
| `LanguageManager.cpp` | 新增 17 个 XPL 效果的 CardName + CardDesc 中文条目 |
| `SceneGalleryScene.cpp` | 3 列布局、aspect-ratio 缩略图、渐变色占位块 |
| `main.cpp` | `_chdir` 设置工作目录、OpenGL 强切、CoverFlowScene 直启 |
| `Application.cpp` | AUTO_TEST_CARDS 强切 OpenGL 后端 |
