# 效果一致性审计与修复 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 ShaderShowcase 的 37 个实际效果与 Awesome-Unity-Shader 参考源码逐项对比，建立差异表，然后逐个修复，确保算法一致。

**Architecture:** 三阶段流程：Phase 1 读取所有参考 shader + 当前 frag，建立差异表 `docs/effect-diff-table.md`；Phase 2 按差异等级从高到低逐个修复，每个效果修复前需用户确认、修复后编译+截图验证；Phase 3 全量回归测试。

**Tech Stack:** GLSL 460, Python 3, CMake, PowerShell, PPM 截图分析

---

## 效果→参考映射表

| 索引 | 名称 | 当前Frag | 参考Shader |
|------|------|---------|-----------|
| 08 | TEST单色 | v02_solid.frag | Vol02/1.基础单色Shader.shader |
| 09 | 凹凸边缘光 | v01_rim_bump.frag | Vol01/0.TheFirstShader.shader |
| 10 | 基础单色 | v02_solid.frag | Vol02/1.基础单色Shader.shader（同08） |
| 11 | 漫反射纹理 | v02_diffuse_tex.frag | Vol02/6.光照材质完备正式版Shader.shader |
| 12 | 卡通渐变 | v07_toon.frag | Vol07/4.自定义卡通渐变光照.shader |
| 13 | 半兰伯特 | v07_halflambert.frag | Vol07/3.自制半Lambert光照.shader |
| 14 | 镜面高光 | v07_specular.frag | Vol07/1.自定义高光效果.shader |
| 15 | 边缘发光 | v14_rim.frag | Vol14/BasicRimShader.shader |
| 16 | 车漆MatCap | v16_carpaint.frag | Vol16/CarPaintShader.shader |
| 17 | 简单基础色 | v12_simple.frag | Vol12/1.SimpleShader.shader |
| 18 | 变色偏移 | v12_color_change.frag | Vol12/2.ColorChange.shader |
| 19 | 标准漫反射 | v12_diffuse.frag | Vol12/6.Diffuse(Lambert) Shader.shader |
| 20 | 棋盘纹理 | v12_diffuse_tex.frag | Vol12/7.Diffuse(Lambert) Shader with Texture.shader |
| 21 | RGB立方体 | v12_rgb_cube.frag | Vol12/3.RGB Cube.shader |
| 22 | 透明立方体 | v13_alpha_cube.frag | Vol13/1.SimpleAlphaShader.shader |
| 23 | 双面立方体 | v13_twoside_cube.frag | Vol13/3.TwoSideColorChangeAlpha.shader |
| 24 | 透明球体 | v13_simple_alpha.frag | Vol13/1.SimpleAlphaShader.shader |
| 25 | 可调色透明 | v13_color_alpha.frag | Vol13/2.ColorChangeAlpha.shader |
| 26 | 玻璃球体 | v04_glass.frag | Vol04/3.用剔除实现玻璃效果.shader |
| 27 | Alpha裁剪 | v04_alpha_test.frag | Vol04/4.基本Alpha测试.shader |
| 28 | 纹理混合 | v03_alpha_blend.frag | Vol03/1.Alpha纹理混合.shader |
| 29 | 自发光球 | v03_emissive.frag | Vol03/2.Alpha通道与自发光混合.shader |
| 30 | 纹理全组合 | v03_full_combo.frag | Vol03/5.顶点光照+自发光+纹理混合.shader |
| 31 | 乘法混合 | v05_blend_multiply.frag | Vol05/2.基本blend使用.shader |
| 32 | 玻璃v2 | v05_glass_v2.frag | Vol05/5.玻璃效果v2版.shader |
| 33 | 玻璃v3 | v05_glass_v3.frag | Vol05/6.玻璃效果v3版.shader |
| 34 | 径向模糊 | v08_post_radial.frag | Vol08/MotionBlurEffects.shader |
| 35 | 水幕特效 | v09_pass0_drop.frag | Vol09/ScreenWaterDropEffect.shader |
| 36 | 油画特效 | v10_post_oil.frag | Vol10/ScreenOilPaintEffect.shader |
| 37 | 像素化 | v11_post_pixelate.frag | Vol11/PixelEffect.shader |
| 38 | 高斯模糊 | v15_pass0/1/2.frag | Vol15/RapidBlurEffect.shader |
| 39 | 背面渲染 | v04_cull_front.frag | Vol04/1.用剔除操作渲染对象背面.shader |
| 40 | 顶点透明 | v04_vertex_alpha.frag | Vol04/5.顶点光照+可调透明度.shader |
| 41 | 植被效果 | v04_vegetation.frag | Vol04/6.简单的植被Shader.shader |
| 42 | 可编程管线 | v05_programmable.frag | Vol05/三、可编程Shader示例.shader |
| 43 | 细节纹理 | v06_detail_tex.frag | Vol06/8.细节纹理.shader |
| 44 | 凹凸全组合 | v06_full_combo.frag | Vol06/9.凹凸纹理+颜色可调+边缘光照+细节纹理.shader |

---

### Task 1: Phase 1 — 建立差异表

**Files:**
- Create: `docs/effect-diff-table.md`
- Read: 所有参考 `.shader` 文件（`e:\AI\test\Awesome-Unity-Shader\Volume *\*.shader`）
- Read: 所有当前 `.frag` 文件（`shaders/aus3d/*.frag`，仅上表列出的）

**说明：** 此任务为大型探索任务，使用 Explore subagent 读取所有参考 shader 和当前 frag，逐项对比光照模型、颜色公式、纹理混合方式，记录差异到 `docs/effect-diff-table.md`。

- [ ] **Step 1: 读取所有参考 shader 源码**

  使用 Explore subagent 批量读取所有 37 个参考 `.shader` 文件，提取每个文件的关键算法特征：
  - 光照模型类型（Lambert / Half-Lambert / Blinn-Phong / 顶点光照 / 无光照 / 固定管线）
  - 颜色计算公式（diffuse / ambient / specular / emission 的组合方式）
  - 纹理混合方式（Combine texture * primary DOUBLE / Blend / Alpha / Multiply / 无纹理）
  - 特殊效果（Rim / Fresnel / AlphaTest / Cubemap / MatCap / 后处理）

  命令示例：
  ```powershell
  # 单个参考shader的读取示例
  # 对每个索引对应的参考shader，读取完整内容
  ```

- [ ] **Step 2: 读取所有当前 .frag 源码**

  读取所有 37 个当前 `.frag` 文件，提取相同维度的算法特征。

- [ ] **Step 3: 逐项对比并生成差异表**

  对每个效果，对比参考 vs 当前：
  - 光照模型是否一致
  - 颜色公式是否一致
  - 纹理混合方式是否一致
  - 特殊效果是否一致
  - 默认参数值是否一致

  写入 `docs/effect-diff-table.md`，表头如下：

  ```markdown
  | 索引 | 名称 | 参考Shader | 当前Frag | 参考光照 | 当前光照 | 参考公式 | 当前公式 | 参考纹理 | 当前纹理 | 预期特征 | 差异等级 | 修复优先级 |
  |------|------|-----------|---------|---------|---------|---------|---------|---------|---------|---------|---------|-----------|
  ```

  差异等级：高/中/低/无
  修复优先级：1-5（1最高，高差异→1，中差异→2-3，低差异→4-5）

- [ ] **Step 4: 提交差异表**

  ```bash
  git add docs/effect-diff-table.md
  git commit -m "docs: add effect consistency diff table (Phase 1)"
  ```

---

### Task 2: Phase 2 — 逐个修复（模板任务）

**说明：** 此任务为模板，实际修复按差异表顺序逐个执行。每修复一个效果，重复以下步骤。

**前置条件：** 差异表已建立，用户已确认要修复的效果列表。

- [ ] **Step 1: 向用户展示修复计划**

  展示以下信息：
  - 效果名称和索引
  - 参考 shader vs 当前 frag 的具体差异点
  - 计划修改方案（具体到哪几行、改什么）
  - 预期修复后的画面特征变化

  等待用户确认后才能继续。

- [ ] **Step 2: 修改 .frag 文件**

  使用 SearchReplace 修改 `shaders/aus3d/<name>.frag`。
  修改原则：
  - 最小化修改，只改与参考不一致的部分
  - 保持现有代码结构（ray tracing、UBO布局、相机设置等不变）
  - 只修改光照计算、颜色公式、纹理混合部分

- [ ] **Step 3: 编译**

  ```powershell
  taskkill /f /im ShaderShowcase.exe 2>$null
  cd 'e:\AI\graph\hight-post-proc\shader-showcase\build'
  cmake --build . --config Release 2>&1 | Select-Object -Last 10
  ```
  预期：编译成功，无错误。

- [ ] **Step 4: 同步 SPV**

  ```powershell
  Copy-Item 'e:\AI\graph\hight-post-proc\shader-showcase\build\shaders\aus3d\<name>.frag.spv' 'e:\AI\graph\hight-post-proc\shader-showcase\shaders\aus3d\<name>.frag.spv' -Force
  ```

- [ ] **Step 5: 运行该效果截图验证**

  ```powershell
  $env:AUTO_TEST_AUS3D='1'; $env:AUS3D_START_INDEX='<N>'
  $proc = Start-Process -FilePath 'e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release\ShaderShowcase.exe' -WorkingDirectory 'e:\AI\graph\hight-post-proc\shader-showcase' -PassThru -NoNewWindow -RedirectStandardOutput 'temp_out.txt' -RedirectStandardError 'temp_err.txt'
  Start-Sleep -Seconds 10
  if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force }
  ```

  分析截图：
  ```powershell
  python -c "
  import struct
  p = 'e:/AI/graph/hight-post-proc/screenshots/card_aus3d_<N>.ppm'
  with open(p, 'rb') as f:
      h = f.readline(); dims = f.readline()
      while dims.startswith(b'#'): dims = f.readline()
      w, h = map(int, dims.split()); mv = int(f.readline())
      data = f.read()
  cx, cy = w//2, h//2
  idx = (cy*w+cx)*3
  r, g, b = data[idx], data[idx+1], data[idx+2]
  print(f'center=({r},{g},{b})')
  # Sample grid around center
  for dy in range(-150, 151, 75):
      for dx in range(-150, 151, 75):
          idx2 = ((cy+dy)*w+(cx+dx))*3
          print(f'  ({dx:+4d},{dy:+4d}): ({data[idx2]},{data[idx2+1]},{data[idx2+2]})')
  "
  ```

- [ ] **Step 6: 展示修复前后对比**

  向用户展示：
  - 修复前中心像素 vs 修复后中心像素
  - 修复前采样网格 vs 修复后采样网格
  - 确认是否符合预期特征

- [ ] **Step 7: 提交**

  ```bash
  git add shaders/aus3d/<name>.frag docs/effect-diff-table.md
  git commit -m "fix: correct <effect-name> to match reference shader"
  ```

---

### Task 3: Phase 3 — 全量回归测试

**Files:**
- 无修改，只运行现有测试

- [ ] **Step 1: 运行全量自动化截图测试**

  ```powershell
  taskkill /f /im ShaderShowcase.exe 2>$null
  $env:AUTO_TEST_AUS3D='1'; $env:AUS3D_START_INDEX='0'
  $proc = Start-Process -FilePath 'e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release\ShaderShowcase.exe' -WorkingDirectory 'e:\AI\graph\hight-post-proc\shader-showcase' -PassThru -NoNewWindow -RedirectStandardOutput 'aus3d_regression.txt' -RedirectStandardError 'aus3d_regression_err.txt'
  Start-Sleep -Seconds 60
  if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force }
  ```

- [ ] **Step 2: 验证所有 45 个效果截图**

  ```powershell
  python -c "
  import os
  d = 'e:/AI/graph/hight-post-proc/screenshots'
  missing = []
  for i in range(45):
      p = os.path.join(d, f'card_aus3d_{i:02d}.ppm')
      if not os.path.exists(p):
          missing.append(i)
  if missing:
      print(f'MISSING screenshots: {missing}')
  else:
      print('All 45 screenshots captured successfully')
  "
  ```
  预期：所有 45 个截图存在。

- [ ] **Step 3: 检查回归**

  对比修复前后所有效果的中心像素，确认：
  - 修复过的效果中心像素发生变化（符合预期）
  - 未修复的效果中心像素不变（无回归）

  ```powershell
  python -c "
  # 提取所有中心像素，与修复前的基线对比
  import struct, os
  d = 'e:/AI/graph/hight-post-proc/screenshots'
  for i in range(8, 45):  # skip diagnostic 0-7
      p = os.path.join(d, f'card_aus3d_{i:02d}.ppm')
      with open(p, 'rb') as f:
          h = f.readline(); dims = f.readline()
          while dims.startswith(b'#'): dims = f.readline()
          w, hh = map(int, dims.split()); mv = int(f.readline())
          data = f.read()
      cx, cy = w//2, hh//2
      idx = (cy*w+cx)*3
      r, g, b = data[idx], data[idx+1], data[idx+2]
      print(f'[{i:02d}] center=({r:3d},{g:3d},{b:3d})')
  "
  ```

- [ ] **Step 4: 提交最终结果**

  ```bash
  git add -A
  git commit -m "test: Phase 3 regression test passed - all 45 effects verified"
  ```

---

## 执行顺序

```
Task 1 (Phase 1) → 用户审阅差异表 → Task 2 (Phase 2, 逐个循环) → Task 3 (Phase 3)
```

执行 Phase 2 时，每一轮修复都是独立的 Task 2 实例，包含 Step 1-7 的完整流程。