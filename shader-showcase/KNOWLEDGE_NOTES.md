# ShaderShowcase 项目技术笔记

> 本文件记录项目特有的 L3/L4 层经验。项目尚未初始化 `.trae/skills/` 知识系统，
> 建议后续运行 project-knowledge-generator 初始化。

---

<!-- layer:L3 -->
## Vulkan Alpha 混合: 四点协调修改

### 场景
在 AUS3DScene 中添加透明立方体效果时，alpha 输出正确但画面仍不透明。

### 根因
Vulkan 的 alpha 混合需要在**四个层面**同时正确配置，缺一不可：

| 层面 | 文件 | 修改前 | 修改后 |
|------|------|--------|--------|
| 1. 参数传递 | `IRenderBackend.h` ShaderParams | 无 blendEnable 字段 | `bool blendEnable = false;` |
| 2. 管线创建 | `VulkanBackend.cpp` DrawFullscreenQuad | `desc.blendEnable = false;` (硬编码) | `desc.blendEnable = params.blendEnable;` |
| 3. 缓存键 | `VulkanBackend.cpp` CreatePipeline | 缓存键不含 blendEnable | `cacheKey ^= (desc.blendEnable ? (1ULL<<59) : 0);` |
| 4. 场景触发 | `AUS3DScene.cpp` OnRender | 无条件不混合 | `if (fx.name.find("透明")!=npos) p.blendEnable=true;` |

### 关键教训
- **只改 1-2 点不够**：blendEnable 从硬编码 false 改为读取 params 后，如果缓存键没加位，CreatePipeline 会返回旧的 blendEnable=false 管线
- **缓存键是隐性依赖**：PipelineDesc.blendEnable 默认值是 `true`，但 DrawFullscreenQuad 之前硬编码为 `false`，两者不一致时缓存行为不可预测
- **std::string vs C字符串**：`strstr(fx.name.c_str(), "透明")` 或 `fx.name.find("透明") != npos`，不能直接对 std::string 用 strstr

### 验证方法
```python
# 自动化像素验证：对比混合开/关的中心像素差异
# Blend ON  center=(42,45,58)  → diff_from_bg=84
# Blend OFF center=(13,23,71)  → diff_from_bg=221 (更亮，不透明)
# 差异 > 20 说明混合确实生效
```

---

<!-- layer:L3 -->
## SPIR-V 着色器懒加载: 避免 CMake 自动编译与预加载的缓存竞争

### 场景
修改 `.frag` 后重新编译运行，但画面仍是旧着色器效果。

### 根因
CMake 的 `CompileAUS3DShaders` target (CMakeLists.txt:214) 在每次 build 时自动重编译所有 `aus3d/*.frag`。
而 `AUS3DScene::LoadShaders()` 在构造时预加载所有 SPV 到内存。

```
时间线:
  t0: 手动 glslangValidator 编译新 .frag → 新 .spv
  t1: cmake --build → CompileAUS3DShaders 覆盖 .spv (可能用旧的缓存?)
  t2: 应用启动 → LoadShaders() 读取 .spv → 可能读到 t0 或 t1 的版本
  t3: 修改 .frag 后再 build → SPV 更新，但 LoadShaders 已在 t2 加载了旧版本
```

### 解决方案
改为**懒加载**：构造时只加载 vertex shader，fragment shader 在首次 OnRender 时按需加载。

```cpp
// AUS3DScene.cpp OnEnter()
void AUS3DScene::OnEnter() {
    LoadShaders();  // 只加载 vertex shader
    for(auto&fx:m_effects) { fx.fragShader = {0}; } // 强制懒加载
}

// AUS3DScene.cpp OnRender() — 首次渲染时加载
if(!fx.fragShader.id) {
    auto fd = ReadSPIRV(fx.fragShaderPath.c_str());
    if(!fd.empty()) fx.fragShader = be->CreateFragmentShader(fd.data(), fd.size());
    if(!fx.fragShader.id) return; // 每帧重试
}
```

### 关键教训
- **预加载与自动编译冲突**：当构建系统会自动重编译资源时，预加载会在启动时"冻结"一个可能过时的版本
- **懒加载更安全**：每次启动应用时，懒加载确保读到的是磁盘上最新的 SPV
- **ReadSPIRV 双路径**：尝试 `shaders/` 和 `build/shaders/` 两个路径，适应不同工作目录

---

<!-- layer:L4 -->
## 效果索引偏移: BuildEffects 顺序与自动化测试

### 场景
在 BuildEffects() 中添加新效果后，原有效果的索引全部偏移，自动化测试脚本使用硬编码索引截图错误效果。

### 具体案例
```
原始: [11] = 车漆MatCap
添加"基础单色"和"边缘发光"后:
偏移: [13] = 车漆MatCap  (索引+2)
测试脚本仍用 card_aus3d_11.ppm → 截到了错误效果
```

### 解决方案
- 自动化测试脚本不依赖固定索引，每次运行前打印当前效果列表
- 或在 BuildEffects() 中为每个效果分配稳定 ID

### 关键教训
- 效果列表是动态的，索引不是稳定的标识符
- 自动化测试需要先验证"当前索引对应的名称"再截图

---

<!-- layer:L4 -->
## 立方体效果 FOV 与尺寸标定

### 场景
新增立方体效果时，立方体填满整个屏幕或太小看不见。

### 迭代过程
| 尝试 | FOV | 立方体半径 | 结果 |
|------|-----|-----------|------|
| 1 | 0.55 (同球体) | 1.0 | 填满屏幕 |
| 2 | 0.3 | 0.6 | 仍太大 |
| 3 | 0.15 | 0.4 | 太小，四周黑边 |
| 4 | fwd*1.6+uv*0.25 | 0.12 | 宽屏拉伸 |
| 5 | **0.55 (同球体)** | **0.4** | 正确 |

### 最终参数
```glsl
// FOV 与球体一致 (0.55)
vec3 rd = normalize(fwd + uv.x*rt*0.55 + uv.y*up*0.55);
// 立方体半径 0.4 (AABB: -0.4 到 0.4)
vec3 t1 = (vec3(-0.4) - ro) / rd;
vec3 t2 = (vec3(0.4) - ro) / rd;
```

### 关键教训
- 立方体和球体应使用相同的 FOV 参数，通过调整几何体半径来控制视觉大小
- 球体半径 1.0 + FOV 0.55 看起来合适；立方体半径 0.4 + FOV 0.55 视觉大小接近
- 立方体的"视觉大小"约为球体的 0.4 倍，因为 AABB 的对角线比球直径短

---

<!-- layer:L4 -->
## LNK1104 链接错误: EXE 被占用

### 场景
构建时报 `LNK1104: 无法打开文件 ShaderShowcase.exe`。

### 原因
上一次运行的 ShaderShowcase.exe 进程仍在运行，锁定了输出文件。

### 解决方案
```powershell
taskkill /F /IM ShaderShowcase.exe
# 然后重新构建
```

### 关键教训
- Windows 上 EXE 运行时文件被锁定，无法覆盖
- 构建前应确保上一次运行已退出
- 可在 CMake pre-build step 中加入 taskkill 自动化

---

<!-- layer:L4 -->
## 待解决问题: 透明立方体 (idx=15) 不可见

### 当前状态
自动化测试结果：
```
[14] RGB立方体    Ctr=(184,37,18)  diff=219  VISIBLE
[15] 透明立方体   Ctr=(42,45,58)   diff=0    HIDDEN  ← 问题
[16] 双面立方体   Ctr=(38,76,170)  diff=264  VISIBLE
```

### 分析
- diff=0 说明中心像素 = 背景色 (42,45,58 = 0.05*255 ≈ 13? 不匹配)
- 可能原因：着色器未加载（fragShader.id=0 → return → 背景色）
- 或 alpha=0.5 时混合后颜色恰好 = 背景色

### 下一步
1. 确保 build 成功（先 taskkill 再 build）
2. 在 v13_alpha_cube.frag 中提高颜色饱和度，避免混合后=BG
3. 或降低默认 alpha 值（0.5 → 0.3），增加可见度
4. 重新运行自动化验证
