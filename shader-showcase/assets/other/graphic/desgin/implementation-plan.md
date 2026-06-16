# 图形学设计文档优化 — 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 按照 `optimization-spec.md` 优化方案，重写全部 5 份设计文档，实现与 Games101/LearnOpenGL 的内化融合，篇幅缩减 30-40%。

**Architecture:** 5 份文档独立改写，Part 1 是最重的重构（5 Stage → 4 模块三层结构），Part 2-4 是格式收紧+表替文，总索引是轻型格式调整。改写顺序：Part 1 → Part 2 → Part 3 → Part 4 → 总索引（按依赖和工作量排序）。

**Tech Stack:** Markdown 文档编辑，无代码依赖。

---

## 文件结构

```
e:\AI\bockwork\graphic\desgin\
├── graphics-programming-handbook.md  ← 修改：压缩为路线选择表 + 三色标签 + 去重引用
├── part1-foundation.md              ← 重写：5 Stage → 4 模块三层结构，最大变更
├── part2-advanced.md                ← 修改：统一四段式格式 + 砍推导 + 去重
├── part3-commercial.md              ← 修改：引擎表替文 + 策略矩阵表 + 砍推测
├── part4-frontier.md                ← 修改：加"本体系判断"结尾 + 砍NeRF时间线 + 聚焦趋势
└── optimization-spec.md             ← 只读参考，不改动
```

---

### Task 1: 重写 part1-foundation.md（最大重构）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part1-foundation.md`
- 参考：`e:\AI\bockwork\graphic\desgin\optimization-spec.md`

**新结构对照 spec 一、Part 1 重构部分。**

**注意：** 不要删除旧文件→写新文件。先用 Read 读入当前 part1 内容，提取其中不应丢弃的代码示例（MVP 变换 Python 代码、ShaderToy shader 代码、后处理管线代码等），然后在新文件中组织。

- [ ] **Step 1: 提取旧 Part 1 中需保留的代码片段和关键表格**

  需要提取和保留的内容：
  - MVP 变换 NumPy 手写代码（Stage 1）
  - 最小 OpenGL 彩色三角形代码（Stage 2）
  - Shader 编译链接 C++ 流程（Stage 2）
  - Phong/Blinn-Phong GLSL 代码（Stage 3）
  - Shadow Map + PCF 核心代码（Stage 3）
  - 后处理管线 GLSL（反相/灰度/边缘检测/模糊）（Stage 4）
  - 延迟渲染 GBuffer 设计 + 100 点光源对比（Stage 4）
  - ShaderToy SDF 图元 + CSG 代码（Stage 5）
  - Ray Marching 完整场景 GLSL（Stage 5）
  - 噪声函数代码（Stage 5）
  - 结业检查清单（Stage 5 末尾）

  读取命令：分多次 Read 读取 `e:\AI\bockwork\graphic\desgin\part1-foundation.md`，标记以上片段的行号范围。

- [ ] **Step 2: 写出新 Part 1 骨架（4 模块三层结构）**

  写入 `e:\AI\bockwork\graphic\desgin\part1-foundation.md`：

  ```markdown
  # Part 1: 基础入门 — 理论·实践·Shader思维

  > 本部分内化整合 [Games101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)（理论）和
  > [LearnOpenGL](https://learnopengl-cn.github.io/)（实践）的核心知识，并以本体系独有的
  > **Shader视点**组织三层表达：理论给结论不展开推导，实践给最小可运行代码，
  > Shader视点从着色器内部理解图形。

  ## 模块1: 空间变换 — 从坐标到屏幕

  ### ① 理论核心

  - 齐次坐标：引入第4维的唯二理由——用矩阵统一表达平移 + 透视除法
  - MVP 三步结论：
    - **Model**: 物体本地坐标 → 世界坐标 (GLM: `glm::translate`/`glm::rotate`/`glm::scale`)
    - **View**: 世界坐标 → 相机坐标 (GLM: `glm::lookAt`)
    - **Projection**: 相机坐标 → NDC [-1,1]³ (GLM: `glm::perspective`/`glm::ortho`)
  - 左手系 vs 右手系：OpenGL 右手系，DirectX 左手系，区别在于 Z 轴正方向
  - 四元数：解决欧拉角万向节锁的最小方案，`glm::quat` 直接封装，不展开推导
  - 📎 [Games101 Lec 2-4] / [LearnOpenGL 入门-变换-坐标系-摄像机]

  ### ② 实践核心

  <插入 MVP 变换 NumPy 代码和一帧 GLM+VAO/VBO 旋转立方体代码>
  <给关键 API 调用链，不写完整文件>

  ### ③ Shader视点

  fragCoord → 世界坐标逆变换：在 ShaderToy 中，已知屏幕像素(fragCoord)和 MVP 矩阵，
  反算该像素对应的世界坐标位置，是 Ray Marching 和屏幕空间效果的基础。

  ---

  ## 模块2: 光照着色 — 为什么物体有颜色

  ### ① 理论核心

  - Blinn-Phong 三成分公式：
    ```
    L = ambient + diffuse + specular
    ambient = kₐ · Iₐ
    diffuse = kₐ · I · max(0, n·l)
    specular = kₛ · I · max(0, n·h)^α   ← Blinn-Phong 用 half-vector，比 Phong 快
    ```
  - Gamma 校正：显示器非线性，线性空间计算后做 `pow(1/2.2)` 输出
  - 光源类型：方向光(方向+w=0)、点光源(位置+w=1)、聚光灯(位置+角度)
  - 📎 [Games101 Lec 7-8] / [LearnOpenGL 光照-阴影系列]

  ### ② 实践核心

  <插入 Blinn-Phong GLSL 代码 + Shadow Map 两步法 + PCF 核心代码>
  步骤：① 光源视角渲染深度图 ② 正常渲染时比较当前片元深度与深度图 ③ PCF: 采样周围4个纹素取平均

  ### ③ Shader视点

  在 ShaderToy 中手写 Blinn-Phong：没有 VAO/VBO/管线，一切计算发生在片元着色器。
  光照的本质是"已知片元的世界坐标+法线+材质参数 → 算颜色"。

  ---

  ## 模块3: 渲染架构 — 组织画面而非画单个物体

  ### ① 理论核心

  - FBO = 离屏画布：把渲染结果写到纹理而非屏幕
  - 延迟渲染 vs 前向渲染：
    - 前向：每个物体 × 每个光源 = O(N²)，光源多时崩溃
    - 延迟：先填 G-Buffer（位置/法线/颜色/材质）一次 O(N)，再光照一次 O(M)
  - G-Buffer 典型 128bpp 布局：RGB16F(RGBA) + RGB16F(Normal) + RGB16F(Albedo) + R16F(Metal+Rough+AO)
  - 📎 [Games101 Lec 8] / [LearnOpenGL 高级OpenGL-帧缓冲-延迟着色]

  ### ② 实践核心

  <插入后处理管线 GLSL（反相/灰度/边缘检测/模糊）+ FBO 创建/绑定代码>
  <插入延迟渲染 G-Buffer 写入 + 光照累积 Pass 代码>

  ### ③ Shader视点

  全屏三角形 = 滤镜：后处理的本质是"把上一帧的渲染结果当纹理，用全屏着色器逐像素处理"。
  边缘检测不过是采样周围像素做卷积。

  ---

  ## 模块4: Shader思维 ★核心差异化★

  > 以下内容 Games101 和 LearnOpenGL 均未覆盖，是本体系的独特价值。

  ### ① SDF — 有向距离场

  - 核心思想：每个点存储到最近表面的距离（正=外，负=内），等值面=0
  - 基本图元：球体 `length(p)-r`、立方体 `max(abs(p)-b,0.0)`、平面 `p.y`
  - CSG 组合：并`min(d1,d2)`、交`max(d1,d2)`、差`max(d1,-d2)`、平滑混合`smin(d1,d2,k)`
  - <插入 SDF 图元 + CSG GLSL 代码>

  ### ② Ray Marching — 光线步进

  - 从屏幕像素发射射线，沿射线方向步进，每步用 SDF 得到距场景最小距离
  - 当距离 < 阈值 → 命中；步数超限 → 未命中
  - 法线重建：`normalize(vec3(SDF(p+ε) - SDF(p-ε), ...))`
  - 软阴影：Ray Marching 自然获得，步进时记录最近遮挡距离
  - <插入完整 Ray Marching GLSL 场景（金属球+玻璃球+软阴影+地面反射）>

  ### ③ 噪声与程序化

  - 一句话区分：Value(块状) → Perlin(平滑梯度) → Simplex(高维高效) → Voronoi(细胞距离)
  - fBm = 多层噪声叠加，每层频率×2、振幅÷2，制造自然纹理
  - <插入噪声函数 GLSL 代码>

  ### ④ ShaderToy 完整案例

  <插入完整 Ray Marching 场景代码>

  ---

  ## 结业检查清单

  - [ ] 能手写 MVP 矩阵变换（代码）
  - [ ] 能创建 VAO/VBO 绘制彩色三角形
  - [ ] 能手写 Blinn-Phong Shader
  - [ ] 能实现 Shadow Map + PCF
  - [ ] 能搭建后处理管线（至少 3 种效果）
  - [ ] 能解释延迟渲染 vs 前向渲染的架构差异
  - [ ] 能手写 SDF 球体+CSG 组合
  - [ ] 能手写 Ray Marching 基础场景
  - [ ] 能在 ShaderToy 中独立完成 SDF+RayMarching 作品
  ```

- [ ] **Step 3: 填充全部代码片段**

  将 Step 1 中提取的代码片段插入到 Step 2 骨架的对应位置。代码保持原样不修改，但只保留核心片段（非完整文件），加注释说明省略部分。

- [ ] **Step 4: 验证 Part 1 结构**

  检查清单：
  - 4 个模块是否都有 `① 理论` `② 实践` `③ Shader视点` 三层
  - 每个模块末尾是否有 `📎 [课程对照]`
  - 模块4 是否标注了"核心差异化"
  - 结业检查清单是否保留（可缩减为 9 条核心项）
  - 总篇幅是否比原 Part 1 缩减了约 50%

  读取新文件，逐模块确认。

- [ ] **Step 5: 提交 Part 1**

  ```bash
  # 无 git 环境则跳过 commit，改为人工确认文件内容
  Read e:\AI\bockwork\graphic\desgin\part1-foundation.md 确认完整
  ```

---

### Task 2: 收紧 part2-advanced.md（格式统一 + 砍推导）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part2-advanced.md`
- 参考：`e:\AI\bockwork\graphic\desgin\optimization-spec.md` 二、Part 2 部分

- [ ] **Step 1: 处理 Cook-Torrance 推导（Case 1）**

  当前：三个因子（D/F/G）各自有完整推导段落。
  改为：每个因子只给结论公式 + GLSL 代码，末尾加一句 `> 推导参考 [PBR Book 8.4]`。
  
  伪代码变换：
  ```markdown
  ### Cook-Torrance BRDF

  **结论公式：**
  ```
  f(l,v) = D(h)·F(v,h)·G(l,v,h) / (4·(n·l)·(n·v))
  ```

  - **D (GGX)**: `D = α² / (π·(n·h)²·(α²-1) + 1)²`  — α = roughness²，长尾特性模拟真实表面微结构
  - **F (Schlick)**: `F = F₀ + (1-F₀)·(1-h·v)⁵`  — F₀=0.04 for dielectrics
  - **G (Smith GGX)**: `G₁(v) = (n·v) / ((n·v)·(1-k)+k)`，`G = G₁(l)·G₁(v)`，`k = (roughness+1)²/8`
  - 推导参考 [PBR Book §8.4] 和 [LearnOpenGL PBR]
  ```

- [ ] **Step 2: 处理 IBL 三件套（Case 1）**

  改为组装顺序描述 + 代码链接，不展开每件计算原理：
  ```markdown
  ### IBL 组装顺序

  1. Diffuse Irradiance Map: 对环境贴图做卷积 → 漫反射项查表
  2. Pre-filtered Environment Map: 多级 mip 对应不同粗糙度 → mip level = roughness·maxMip
  3. BRDF LUT: 2D 查找表 (n·v, roughness) → (F₀_scale, F₀_bias)

  IBL = diffuse_sample * albedo/π * (1-F) + prefilter_sample * (F * F₀_scale + F₀_bias)
  ```

- [ ] **Step 3: 统一 Case 3 屏幕空间四效果的节拍**

  每个效果格式化为：`**原理一句** → **核心伪代码** → **效果关键词**`，删除叙述性展开。例如：
  ```markdown
  ### SSAO
  - 原理：屏幕空间像素周围半球采样，统计被遮挡的采样点比例 → 暗化该像素
  - 核心：深度缓冲重建世界坐标 → 随机方向半球采样 → 采样点在表面后方则计入遮挡
  - 降噪：4×4 双边模糊（保留深度/法线边缘）
  ```

- [ ] **Step 4: 处理 GPU Driven Case 总起句**

  在 Case 4 开头添加：
  ```markdown
  > GPU Driven 本质：把 CPU 端的判断逻辑（视锥体剔除、遮挡查询、DrawCall 生成）
  > 整体搬迁到 GPU Compute Shader，消除 CPU-GPU 往返瓶颈。
  > 以下四种技术是同一逻辑的不同应用场景。
  ```

- [ ] **Step 5: 验证 Part 2**

  检查：每个 Case 是否仍有过度展开的推导段落，篇幅是否缩减约 30%。

---

### Task 3: 重写 part3-commercial.md（表替文 + 压缩）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part3-commercial.md`
- 参考：`e:\AI\bockwork\graphic\desgin\optimization-spec.md` 二、Part 3 部分

- [ ] **Step 1: 将 11 引擎逐一介绍改为对照表**

  将第 1 章中全部引擎逐一介绍的段落替换为以下表格：

  ```markdown
  ## 第1章：商业引擎渲染架构对比

  | 引擎 | Nanite等效 | Lumen/GI | 着色管线 | TAA/TSR | RT支持 | 移动端 | 开源 |
  |---|---|---|---|---|---|---|---|
  | UE5 | Nanite虚拟几何 | Lumen+Lightmap | Deferred→Visibility Buffer | TSR | HWRT+Lumen | ❌ | ✓源码 |
  | Unity 6 | ❌ (GPU Resident Drawer) | APV/Probe-based | URP Forward+ / HDRP Deferred | TAA | HWRT混合 | ✓ | ❌ |
  | Decima | 大规模地形GPU Cull | Probe GI+天空模型 | Forward+ | TAA | ❌ | ❌ | ❌ |
  | RE Engine | ❌ (Interior网格) | Lightmap+Probe | Forward+ | TAA | RT反射/阴影 | ✓(Switch) | ❌ |
  | Snowdrop | ❌ (程序化生成) | 动态GI+Probe | Forward+ | 时间上采样 | RT阴影 | ❌ | ❌ |
  | id Tech | ❌ (MegaTexture) | Lightmap | Forward+ | TAA | RT | ❌ | ✓ |
  | Frostbite | ❌ | Enlighten GI | Forward+ | TAA | RT反射 | ❌ | ❌ |
  ```

  保留各引擎的"一句话定位"（如 UE5="通用AAA引擎标杆"），删除所有详细介绍段落。

- [ ] **Step 2: 保留 Nanite/Lumen 深度分析，砍推测**

  第 2 章保持结构，但：
  - 删除所有"可能""预计""推测"字样的句子
  - 代码伪代码保留，但代码块前后的解释文字各砍一半

- [ ] **Step 3: 移动端&Web 压缩为策略矩阵**

  第 4 章移动端 PBR 降级策略 → 改为：
  ```markdown
  ## 移动端 PBR 降级策略

  | 预算级别 | 分辨率 | 光源数 | PBR | 阴影 | 后处理 | 代表 |
  |---|---|---|---|---|---|---|
  | 高端(旗舰) | 1080p+ | <8 动态 | Full PBR+IBL | CSM 2级 | Bloom+AO | 原神 |
  | 中端 | 720p | <4 动态 | Cook-Torrance 主光源 | 静态烘焙 | ❌ | 王者 |
  | 低端 | ≤540p | 1 方向光 | Lambert+Blinn | ❌ | ❌ | 轻量游戏 |
  ```

- [ ] **Step 4: 验证 Part 3

  检查：是否仍有段落式引擎介绍、推测语句，篇幅是否缩减约 40%。

---

### Task 4: 收紧 part4-frontier.md（强化判断 + 砍时间线）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part4-frontier.md`
- 参考：`e:\AI\bockwork\graphic\desgin\optimization-spec.md` 二、Part 4 部分

- [ ] **Step 1: 每节末尾加"本体系判断"**

  以第 1 章 Neural Rendering 为例：
  ```markdown
  ### 🧭 本体系判断

  - **3DGS：值得深入**。已开始渗透游戏引擎（Unity/UE 社区方案）、VR/AR、视觉特效。
    理解显式点云+可微分光栅化是理解"神经+传统混合管线"的钥匙。
  - **NeRF 经典公式：了解即可**。已被 InstantNGP/3DGS 取代，直接学后者。
  - **Mip-NeRF/Zip-NeRF：暂时跳过**。前沿研究，工程中暂无可落地方案。
  ```

- [ ] **Step 2: 砍 NeRF 家族时间线**

  NeRF 家族时间线从 2020-2025 全列表砍为：
  ```markdown
  ### NeRF 发展里程碑

  2020 NeRF → 2022 Instant-NGP (千倍加速) → 2023 3DGS (实时渲染 + 显式表示)
  ```
  保留 3DGS 详解内容不动。

- [ ] **Step 3: 2030 推测聚焦**

  当前"总结"部分删除泛化预测，聚焦 3 条：
  ```markdown
  ## 三条确定性趋势

  1. **物理→神经混合管线下移**：3DGS 类方法进入实时应用，取代部分传统 LOD/Imposter
  2. **GPU 调度权上移**：Work Graphs/Mesh Shader 让 GPU 自调度，减少 CPU 往返
  3. **AI 降噪 + 超分 = 新标准**：DLSS/FSR/XeSS 成为 AAA 游戏标配，神经降噪替代传统 TAA
  ```

- [ ] **Step 4: 验证 Part 4**

  检查：每节末尾是否有"本体系判断"，NeRF 时间线是否砍半，趋势是否聚焦。

---

### Task 5: 压缩 graphics-programming-handbook.md（总索引）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\graphics-programming-handbook.md`

- [ ] **Step 1: 四条路线改为选择表**

  替换为：
  ```markdown
  ## 学习路线选择

  | 你的目标 | 核心章节 | 辅助章节 | 建议跳过 |
  |---|---|---|---|
  | 游戏渲染 | Part 1 模块1-3 + Part 2 Case 1-4 + Part 3 第1-2章 | Part 4 第2-3章 | 影视离线渲染 |
  | 图形学研究 | Part 1 全部 + Part 2 Case 1,5 + Part 4 第1章 | Part 3 第3章 | 移动端渲染 |
  | 自研引擎 | Part 1 全部 + Part 2 全部 + Part 3 第1-2,4章 | Part 4 第2,4章 | — |
  | Web/移动 | Part 1 模块1-3 + Part 3 第4章 | Part 2 Case 1-3 | GPU Driven/Case 4 |
  ```

- [ ] **Step 2: 难度标签改三色**

  全文搜索 `🔵` `🟢` `🟡` `🟠` `🔴` 五个标签，统一改为：

  | 旧标签 | 新标签 |
  |---|---|
  | 🔵 入门 | 🟢 入门 |
  | 🟢 中级 | 🟡 进阶 |
  | 🟡 高级 | 🟡 进阶 |
  | 🟠 前沿 | 🔴 前沿 |
  | 🔴 前沿 | 🔴 前沿 |

  即合并为三色：🟢入门 🟡进阶 🔴前沿

- [ ] **Step 3: 去重参考资源**

  删除参考资源中与 Games101/LearnOpenGL 重复的入门级推荐（如"3D Math Primer"、"OpenGL Programming Guide"），改为在开头统一引用：
  ```markdown
  > 本体系预设读者已完成或同步学习 [Games101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)
  > （理论）和 [LearnOpenGL](https://learnopengl-cn.github.io/)（实践），不再重复推荐同类入门资源。
  ```

- [ ] **Step 4: 同步 Part 1 新结构到文件索引**

  将总索引中"Part 1 包含 5 个 Stage"的描述更新为：
  ```markdown
  - **[Part 1](part1-foundation.md)** — 基础入门：4 模块·三层结构
    - 模块1 空间变换 | 模块2 光照着色 | 模块3 渲染架构 | 模块4 Shader思维
  ```

- [ ] **Step 5: 验证总索引**

  读取文件，确认表替文、标签统一、引用去重完成。

---

### Task 6: 全体系一致性校验

- [ ] **Step 1: 交叉引用验证**

  检查所有 `.md` 文件中指向其他文件的链接是否有效：
  - 总索引 → Part 1-4 的链接
  - Part 1 → 总索引的链接
  - Part 2-4 内部的交叉引用

- [ ] **Step 2: 篇幅验证**

  统计各文件行数并与估算对比：
  - 需求：全体系缩减 30-40%
  - Part 1 预期缩减 50%
  - Part 2 预期缩减 30%
  - Part 3 预期缩减 40%
  - Part 4 预期缩减 15%
  - 总索引预期缩减 30%

  运行统计命令：
  ```powershell
  Get-ChildItem e:\AI\bockwork\graphic\desgin\*.md | Where-Object {$_.Name -ne 'optimization-spec.md' -and $_.Name -ne 'implementation-plan.md'} | ForEach-Object { $lines = (Get-Content $_.FullName | Measure-Object -Line).Lines; Write-Output "$($_.Name): $lines lines" }
  ```

- [ ] **Step 3: 核心规则最终检查**

  - 每个概念是否只展开核心点，无过度叙述？
  - Part 1 每个模块是否三层结构完整？
  - Part 1 每个模块末尾是否有课程对照引用？
  - Part 4 每节末尾是否有"本体系判断"？
  - 全体系是否不再有"可能""预计""推测"等模糊语句？
