# Part 1-4 场景层重写 — 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 按照 `optimization-spec-v2.md` 重写全部 5 份主文档，剥离所有代码块和公式，保留场景描述+效果图占位+课程引用，创建空 implementation/ 目录。

**Architecture:** 4 份主文档独立重写，各自遵循统一模板（场景与阶段→做了什么→效果展示→📎深入学习→💡实现原理）。总索引微调引用路径。全为 Markdown 文档编辑，无代码依赖。

**Tech Stack:** Markdown 文档编辑，无代码依赖。

---

## 文件结构

```
e:\AI\bockwork\graphic\desgin\
├── graphics-programming-handbook.md  ← 修改：微调引用路径 + 实现专栏索引
├── part1-foundation.md              ← 重写：5 Stage → 4 模块纯场景层
├── part2-advanced.md                ← 重写：5 Case 纯场景层
├── part3-commercial.md              ← 重写：4 章 纯场景层
├── part4-frontier.md                ← 重写：4 章 纯场景层（保留🧭判断）
└── implementation/                  ← 新建：空目录，后续填充
```

---

### Task 1: 创建 implementation 目录 + 4 个占位文件

**Files:**
- 创建：`e:\AI\bockwork\graphic\desgin\implementation\` (目录)
- 创建：`e:\AI\bockwork\graphic\desgin\implementation\README.md`

- [ ] **Step 1: 创建目录**

```powershell
New-Item -ItemType Directory -Path 'e:\AI\bockwork\graphic\desgin\implementation' -Force
```

- [ ] **Step 2: 写入 README.md**

写入 `e:\AI\bockwork\graphic\desgin\implementation\README.md`：

```markdown
# 实现专栏

本目录包含各 Part 的详细实现原理、代码片段和公式推导。

> ⚠️ 本目录内容为后续补充，当前为空占位。

## 文件索引

| 文件 | 对应主文档 | 状态 |
|---|---|---|
| part1-implementation.md | Part 1 基础入门 | 🔜 待补充 |
| part2-implementation.md | Part 2 高级渲染 | 🔜 待补充 |
| part3-implementation.md | Part 3 商业级别 | 🔜 待补充 |
| part4-implementation.md | Part 4 前沿发展 | 🔜 待补充 |
```

---

### Task 2: 重写 part1-foundation.md（4 模块·纯场景层）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part1-foundation.md`

**硬约束：** 不得出现任何 ` ``` ` 代码块，不得出现任何数学公式。

- [ ] **Step 1: 写入 Part 1 新内容**

写入 `e:\AI\bockwork\graphic\desgin\part1-foundation.md`：

```markdown
# Part 1: 基础入门 — 场景设计

> 本部分描绘图形渲染的核心场景：顶点如何到达屏幕、物体如何被照亮、画面如何被组织、
> 以及如何用纯着色器思维构建世界。实现细节见 [Part 1 实现专栏](implementation/part1-implementation.md)。

---

## 模块1: 空间变换 — 从坐标到屏幕 🟢

### 场景与阶段
> 顶点着色器阶段。物体从本地坐标系出发，经过 Model→View→Projection 三步变换，抵达屏幕二维平面。

### 做了什么
- **MVP 变换**：Model 将物体从本地空间搬到世界空间；View 将整个世界「装进」相机取景框；Projection 将 3D 场景压扁到 2D 平面，同时保留深度值用于后续遮挡判定
- **坐标系切换**：全过程涉及右手系（OpenGL，Z轴指向屏幕外）和左手系（DirectX，Z轴指向屏幕内）的转换——核心区别在于 Z 轴正方向朝向
- **旋转方案**：用四元数替代欧拉角做旋转，避免「万向节锁」。四元数 = 一次绕轴旋转，欧拉角 = 三次连续旋转（中间可能卡死）

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 旋转立方体 3D 线框投影到 2D | `[待替换]` | LearnOpenGL 入门-坐标系 最终截图 |
| MVP 变换流程示意 | `[待替换]` | Games101 Lec 4 课件第15页 |

### 📎 深入学习
- 理论：[Games101 Lec 2-4] — 向量与线性代数、变换、MVP
- 实践：[LearnOpenGL 入门-变换-坐标系-摄像机]

### 💡 实现原理
> 见 [Part 1 实现专栏](implementation/part1-implementation.md) — GLM 矩阵运算、VAO/VBO/EBO 配置、Shader 编译链接流程、fragCoord→世界坐标逆变换

---

## 模块2: 光照着色 — 物体为什么有颜色 🟢

### 场景与阶段
> 片元着色器阶段。已知表面位置和法线，需要计算该点最终呈现的颜色。

### 做了什么
- **Blinn-Phong 光照模型**：将光照分解为环境光（全局底色，防止暗面全黑）、漫反射（粗糙表面均匀散射，不依赖视角）和镜面反射（光滑表面高光，依赖视角）三层叠加。Blinn-Phong 用半角向量替代 Phong 的反射向量——计算更快且高光更自然
- **阴影映射（Shadow Map）**：先以光源视角渲染一张深度图（记录「从光源看最近物体的距离」），正常渲染时比较每个片元到光源的距离——比深度图远则判定被遮挡
- **软阴影（PCF）**：采样周围多个纹素取平均，让阴影边缘从硬锯齿过渡为柔和的半影，模拟真实世界的软阴影

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| Blinn-Phong 球体（环境+漫反射+镜面三层叠加） | `[待替换]` | LearnOpenGL 光照-Basic Lighting 最终截图 |
| Shadow Map 硬阴影 vs PCF 软阴影对比 | `[待替换]` | LearnOpenGL Advanced-Lighting Shadows 最终截图 |

### 📎 深入学习
- 理论：[Games101 Lec 7-8] — 着色模型、图形管线、纹理映射
- 实践：[LearnOpenGL 光照-阴影系列]

### 💡 实现原理
> 见 [Part 1 实现专栏](implementation/part1-implementation.md) — Blinn-Phong GLSL 代码、Shadow Map 两步法渲染流程、PCF 采样核实现

---

## 模块3: 渲染架构 — 组织画面而非画单个物体 🟢

### 场景与阶段
> 引擎渲染架构设计阶段。决定几何数据如何组织、光照何时计算、后处理何时叠加。

### 做了什么
- **离屏渲染（FBO）**：先把场景渲染到纹理而非屏幕，再将纹理作为素材做后处理——反相、灰度、边缘检测、模糊等全屏效果。本质是「把上一帧渲染结果当纹理，用全屏着色器逐像素处理」
- **延迟渲染（Deferred Rendering）**：先跑几何 Pass（填 G-Buffer，存位置/法线/颜色/材质），再跑光照 Pass（全屏累加所有光源）。复杂度从 O(N×M) 降至 O(N+M)，让百级动态光源成为现实。代价是透明物体需额外处理、不支持 MSAA
- **后处理管线**：边缘检测（Sobel 卷积采样周围像素）、模糊（高斯核加权平均）、Bloom（提取亮部→多级模糊→叠加回原图），全部通过一个全屏三角形 + 着色器完成

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 延迟渲染支持大量动态点光源的场景 | `[待替换]` | LearnOpenGL Deferred Shading 最终截图 |
| 后处理效果切换（原始/灰度/反相/边缘检测/模糊） | `[待替换]` | LearnOpenGL Framebuffers 后处理截图 |

### 📎 深入学习
- 理论：[Games101 Lec 8] — 着色频率、图形管线
- 实践：[LearnOpenGL 高级OpenGL-帧缓冲-延迟着色]

### 💡 实现原理
> 见 [Part 1 实现专栏](implementation/part1-implementation.md) — FBO 创建与绑定流程、G-Buffer 128bpp 典型布局、光照累积 Pass 着色器、后处理效果切换着色器

---

## 模块4: Shader思维 ★核心差异化★ 🟡

> Games101 和 LearnOpenGL 均未覆盖，是本体系独特价值。

### 场景与阶段
> 完全脱离传统管线框架——无三角形、无 VAO/VBO、无 Model 矩阵。场景定义、光线求解、着色全部在片元着色器中完成。

### 做了什么
- **SDF 有向距离场**：用纯数学函数定义几何体——球体 = 到球心的距离减半径、立方体 = 轴对称距离、环面 = 环形距离。通过 CSG 操作（布尔并/交/差/平滑混合）像搭积木一样组合出复杂形状
- **Ray Marching 光线步进**：屏幕上每个像素发射一条射线，沿射线方向一步步安全前进（每步距离 = 当前点到最近表面的距离，保证不穿透），碰到表面后重建法线、计算光照、输出颜色——整个过程没有三角形参与
- **噪声与程序化纹理**：Value/Perlin/Simplex/Voronoi 四类噪声函数 + fBm 分形布朗运动（多层噪声叠加，每层频率翻倍振幅减半），用纯数学算法生成山脉、云层、木质纹路、细胞结构等自然纹理

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| SDF 图元 + CSG 布尔运算渲染 | `[待替换]` | ShaderToy 搜索 "SDF primitives" |
| Ray Marching 完整场景（金属球+玻璃球+软阴影+反射） | `[待替换]` | ShaderToy 搜索 "ray marching spheres" |
| fBm 程序化纹理效果 | `[待替换]` | ShaderToy 搜索 "fbm" 或 The Book of Shaders Noise 章节 |

### 📎 深入学习
- 无直接课程对应。参考 [The Book of Shaders] / [ShaderToy] / [Inigo Quilez 博客]

### 💡 实现原理
> 见 [Part 1 实现专栏](implementation/part1-implementation.md) — SDF 基本图元 GLSL、CSG 组合操作、Ray Marching 主循环、法线重建与软阴影、噪声哈希函数、fBm 叠加逻辑、完整 Ray Marching 场景代码

---

## 结业检查清单

- [ ] 能解释 MVP 变换的三步流程及坐标空间变化
- [ ] 能区分 Blinn-Phong 三个成分的含义和视觉贡献
- [ ] 能描述 Shadow Map 的两步法原理及 PCF 软化的作用
- [ ] 能对比延迟渲染 vs 前向渲染的架构差异和适用场景
- [ ] 能描述后处理管线的设计思路（离屏渲染→全屏着色器→效果叠加）
- [ ] 能解释 SDF 是什么、CSG 操作有哪些
- [ ] 能描述 Ray Marching 算法的核心循环逻辑
- [ ] 能区分四类噪声函数的特点和用途
- [ ] 能在 ShaderToy 中独立完成 SDF + Ray Marching + 噪声的综合作品

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 1 实现专栏](implementation/part1-implementation.md)  
> 📖 继续阅读 [Part 2: 高级渲染](part2-advanced.md)
```

---

### Task 3: 重写 part2-advanced.md（5 Case·纯场景层）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part2-advanced.md`

**硬约束：** 不得出现任何 ` ``` ` 代码块，不得出现任何数学公式。

- [ ] **Step 1: 写入 Part 2 新内容**

写入 `e:\AI\bockwork\graphic\desgin\part2-advanced.md`：

```markdown
# Part 2: 高级渲染 — 场景设计

> 从 PBR 材质到体积云，Part 2 描绘了实时渲染进阶技术的「做什么」与「为什么」。  
> 实现细节见 [Part 2 实现专栏](implementation/part2-implementation.md)。

---

## Case 1: PBR 管线 — 让材质「真实」 🟡

### 场景与阶段
> 材质系统升级阶段。传统 Phong 模型无法区分金属/非金属、不保证能量守恒。

### 做了什么
- **微表面 BRDF（Cook-Torrance GGX）**：用三个物理因子——法线分布（D，描述微表面粗糙度）、菲涅尔（F，描述掠射角反射增强）、几何遮挡（G，描述微表面自遮挡）——统一描述金属和非金属材质的光照响应
- **IBL 三件套**：将环境光预先编码为三张查询纹理。漫反射辐照度图处理粗糙面的环境光、预滤波环境图处理不同粗糙度级别的镜面反射、BRDF 积分 LUT 做最终查表合成
- **HDR + 色调映射**：渲染在线性高动态范围中进行（光强无上限），输出到显示器前用 ACES 或 Reinhard 色调映射压缩到 [0,1]，再做 Gamma 校正

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| PBR 金属球系列（粗糙度 0.0→1.0 渐变） | `[待替换]` | LearnOpenGL PBR Lighting 页面截图 |
| 同场景 Phong vs PBR 对比 | `[待替换]` | LearnOpenGL PBR Theory 页面截图 |
| IBL 环境光照效果（金属球反射环境） | `[待替换]` | LearnOpenGL IBL Diffuse irradiance 页面截图 |

### 📎 深入学习
- 理论：[Games101 Lec 17] — 材质与外观
- 实践：[LearnOpenGL PBR 系列] — Theory / Lighting / IBL

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — Cook-Torrance BRDF GLSL、IBL 组装流程、ACES 色调映射、多材质 PBR 参数配置表

---

## Case 2: 延迟渲染 + 海量光源 🟡

### 场景与阶段
> 管线架构决策阶段。当场景中动态光源达到百级以上时，前向渲染 O(N×M) 的复杂度让帧率崩溃。

### 做了什么
- **G-Buffer 几何缓冲**：第一趟渲染不计算光照，只把每个像素的位置、法线、颜色、材质参数写入多张纹理（典型 128bpp 四通道布局），存下「这个像素是什么表面」
- **光照累积 Pass**：第二趟用一个全屏四边形，从 G-Buffer 读取数据，遍历所有影响当前像素的光源，累加光照。复杂度从 O(N×M) 降为 O(N+M)
- **Tiled Light Culling**：将屏幕分成 16×16 的 Tile，每个 Tile 只计算实际影响它的光源。在 Compute Shader 中并用原子操作确定每个 Tile 的深度范围，大幅减少无效光照计算

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 延迟渲染 100+ 动态点光源场景 | `[待替换]` | LearnOpenGL Deferred Shading 页面截图 |
| Tiled Culling 开启/关闭性能对比 | `[待替换]` | GPUOpen 或 SIGGRAPH 相关演讲截图 |

### 📎 深入学习
- 实践：[LearnOpenGL Deferred Shading]
- 延伸：[Clustered Forward Rendering (SIGGRAPH 2015)]

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — G-Buffer 布局与 MRT 写入、从深度重建世界坐标、Tiled Culling Compute Shader、透明物体 Forward Pass 混合策略、性能对比数据

---

## Case 3: 屏幕空间效果 — 后期「滤镜」 🟡

### 场景与阶段
> 后处理阶段。基于已渲染的颜色缓冲和深度缓冲，在 2D 图像域中近似 3D 效果。

### 做了什么
- **SSAO 环境光遮蔽**：在屏幕空间像素周围做半球采样，统计被周围几何遮挡的比例，暗化被遮挡区域。HBAO+ 改进了采样方向策略，双边模糊保留深度和法线边缘
- **SSR 屏幕空间反射**：在深度缓冲中做 Ray Marching，利用 Hi-Z 深度金字塔加速——大步跳跃跳过空白区域。屏幕边缘和视野外用 Cubemap 反射兜底
- **Bloom 泛光**：提取场景亮部（亮度超过阈值）→ 多级降采样高斯模糊 → 上采样叠加回原图，模拟强光在镜头中的光晕扩散
- **景深 DoF**：基于 CoC（弥散圆半径）做圆形散景模糊——离焦平面越远越模糊，前景散景不渗到背景

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| SSAO 关闭/开启对比 | `[待替换]` | LearnOpenGL SSAO 页面截图 |
| SSR 屏幕空间反射效果 | `[待替换]` | LearnOpenGL 或 GPUOpen SSR 示例截图 |
| Bloom 开启/关闭对比 | `[待替换]` | LearnOpenGL Bloom 页面截图 |
| 景深 DoF 散景效果 | `[待替换]` | UE5 文档 或 开源 DoF 示例截图 |

### 📎 深入学习
- 实践：[LearnOpenGL SSAO / Bloom]

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — SSAO 朴素实现与 HBAO+ 改进、Hi-Z SSR 步进循环、Bloom 多级降采样管线、DoF CoC 计算与散景核

---

## Case 4: GPU Driven Rendering 🟡

### 场景与阶段
> 渲染提交阶段。当场景物体数量达到 10 万级时，CPU 逐个判断可见性和提交 Draw Call 成为瓶颈。

### 做了什么
- **GPU 自主剔除**：把视锥体剔除、Hi-Z 遮挡查询、LOD 选择这些传统由 CPU 做的判断，整体搬迁到 GPU Compute Shader 中并行执行
- **Indirect Draw**：GPU 自己在 Buffer 中填写 Draw Command（画哪些物体、多少实例），然后一次性提交给 `MultiDrawIndirect`——CPU 只需要发一条命令
- **Mesh Shading 管线**：以 Meshlet（约 64 顶点、128 三角形）为最小处理单元，GPU 可以在 Amplification Shader 阶段动态决定生成多少 Mesh Shader 工作组，实现更细粒度的剔除
- **Bindless 纹理**：所有纹理注册到全局描述符表，Shader 通过索引直接访问任意纹理——不再受传统绑定槽数量限制（通常 16-32 个），配合 GPU Driven 实现材质多样性

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 10 万+ 树木实例森林场景（GPU Driven vs CPU Driven 对比） | `[待替换]` | GPUOpen / NVIDIA 演示截图 |
| Mesh Shader 管线流水示意图 | `[待替换]` | DirectX 12 Ultimate 文档 |

### 📎 深入学习
- 延伸：[UE5 Nanite (Part 3) / Mesh Shading (Part 4)]
- 实践：[DirectX 12 Mesh Shader 示例 / Vulkan Mesh Shader 扩展]

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — Indirect Draw Command 结构、GPU Frustum Culling Compute Shader、Hi-Z 深度金字塔构建与遮挡测试、Mesh Shader HLSL 代码、Bindless 纹理访问模式、森林场景性能对比数据

---

## Case 5: 体积渲染与粒子系统 🟡

### 场景与阶段
> 氛围渲染阶段。让「看不见」的空气可见——雾、云、光柱、粒子效果。

### 做了什么
- **体积光 God Rays**：屏幕空间 Ray Marching 沿视线方向步进，每步检查该点是否被光源照亮并累加散射光。Mie 散射相位函数决定前向散射为主的自然光行为
- **GPU 粒子系统**：百万级粒子全部在 GPU 上完成物理模拟（位置更新、生命周期管理）和渲染（Billboarding 展开为始终面向相机的四边形），CPU 只发一次 Dispatch
- **Froxel 体积雾**：将相机视锥体切分成 3D 单元格（Froxel = Frustum + Voxel），每个单元格独立计算散射和消光，叠加到最终画面
- **程序化体积云**：用多层 Worley 噪声叠加定义云密度函数，沿光线步进采样密度并计算 Beer-Lambert 消光，实现实时体积云渲染

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 体积光 God Rays 效果 | `[待替换]` | UE5 Lumen / God Rays 演示截图 |
| GPU 粒子特效场景 | `[待替换]` | UE5 Niagara 演示截图 |
| 实时体积云效果 | `[待替换]` | ShaderToy 搜索 "volumetric clouds" 或 UE5 Volumetric Clouds |

### 📎 深入学习
- 实践：[LearnOpenGL Guest Articles 体积渲染]
- 延伸：[Nubis (Guerrilla Games, Decima 引擎体积云方案)]

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — 体积光屏幕空间 Ray Marching、粒子物理 Compute Shader、Billboarding 顶点着色器、Froxel 体素化与散射计算、Worley 噪声密度函数、Beer-Lambert 光线步进、最终合成管线

---

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 2 实现专栏](implementation/part2-implementation.md)  
> 📖 继续阅读 [Part 3: 商业级别](part3-commercial.md)
```

---

### Task 4: 重写 part3-commercial.md（4 章·纯场景层）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part3-commercial.md`

**硬约束：** 不得出现任何 ` ``` ` 代码块（HLSL/C++/glsl/wgsl 全删），不得出现任何数学公式。保留所有表格。

- [ ] **Step 1: 写入 Part 3 新内容**

写入 `e:\AI\bockwork\graphic\desgin\part3-commercial.md`：

```markdown
# Part 3: 商业级别 — 场景设计

> 从学术到工程——7 大商业引擎的渲染架构决策、Nanite/Lumen 的 AAA 级实现方案、
> 影视离线渲染管线与移动端渲染策略。实现细节见 [Part 3 实现专栏](implementation/part3-implementation.md)。

---

## 1. 商业引擎渲染架构对比 🔴

### 场景与阶段
> 引擎选型或架构设计阶段。不同引擎为不同游戏类型做出了截然不同的渲染技术决策。

### 引擎一句话定位

- **UE5** (Epic Games) — 通用 AAA 引擎标杆，Nanite 虚拟几何 + Lumen 全动态 GI
- **Unity 6** (Unity Technologies) — 跨平台通用引擎，URP/HDRP 双管线全频谱覆盖
- **Decima** (Guerrilla Games) — 大规模自然场景开放世界渲染专家
- **RE Engine** (Capcom) — 写实人物与室内场景渲染专家
- **Snowdrop** (Massive/Ubisoft) — 动态环境交互与程序化生成引擎
- **id Tech** (id Software) — 极致帧率优先的竞技 FPS 引擎
- **Frostbite** (DICE/EA) — 大规模破坏与多人战场渲染引擎

### 商业引擎渲染架构速查

| 引擎 | Nanite等效 | GI方案 | 着色管线 | TAA方案 | RT支持 | 移动端 | 开源 |
|---|---|---|---|---|---|---|---|
| UE5 | Nanite虚拟几何 | Lumen+Lightmap | Visibility Buffer | TSR | HWRT+Lumen | ❌ | ✓源码 |
| Unity 6 | GPU Resident Drawer | APV/Probe | URP Forward+ / HDRP Deferred | TAA | HWRT混合 | ✓ | ❌ |
| Decima | GPU Cull地形 | Probe GI+天空 | Forward+ | TAA | ❌ | ❌ | ❌ |
| RE Engine | 精细网格 | Lightmap+Probe | Forward+ | TAA | RT反射/阴影 | ✓(Switch) | ❌ |
| Snowdrop | 程序化生成 | 动态GI+Probe | Forward+ | 时间上采样 | RT阴影 | ❌ | ❌ |
| id Tech | MegaTexture | Lightmap | Forward+ | TAA | RT | ❌ | ✓ |
| Frostbite | 传统管线 | Enlighten GI | Forward+ | TAA | RT反射 | ❌ | ❌ |

### 引擎对比总结 🔴

| 维度 | UE5 | Unity 6 | Decima | RE Engine | Snowdrop | id Tech | Frostbite |
|------|-----|---------|--------|-----------|----------|---------|-----------|
| 几何系统 | Nanite 虚拟几何 | 传统 LOD + GRD | Patch LOD | 静态 LOD + 摄影测量 | 程序化生成 | 连续 LOD | 层次化 LOD |
| 全局光照 | Lumen (全动态) | Light Probe + RT GI | 探针 + 体积 | 探针网格 + SSR | 动态GI+Probe | 静态 + 探针 | Enlighten / 自研 |
| 渲染管线 | Deferred (VB) + RDG | Forward+ / Deferred | Deferred (时间性) | Deferred (混合) | Forward+ | Forward+ | Deferred (分Layer) |
| 阴影方案 | Virtual Shadow Maps | CSM + RT | CSM + 烘焙 | CSM + 预计算 | RT阴影 | 静态阴影 | CSM + 破坏更新 |
| 目标帧率 | 30-60fps | 30-120fps+ | 30fps | 30-60fps | 30-60fps | 60-144fps | 30-60fps |
| 开放程度 | 源码可用 | 源码可用 | 完全封闭 | 完全封闭 | 完全封闭 | 源码可用(部分) | 完全封闭 |

---

## 2. AAA 级渲染方案拆解 🔴

### 2.1 Nanite — 虚拟化几何系统

#### 场景与阶段
> 几何管线重构。传统管线以三角形为最小处理单元，当几何密度极高（摄影测量/高精度扫描）时，海量亚像素三角形导致 GPU 利用率急剧下降。

#### 做了什么
- **Cluster 集群化**：将最小处理单元从单个三角形提升到 Cluster（约 128 个三角形），附带包围盒和朝向锥。所有后续剔除和 LOD 选择都在 Cluster 级别执行，而非三角形级别
- **两级剔除**：Instance 级（粗粒度，用包围盒+HZB 遮挡查询过滤整个物体），Cluster 级（细粒度，逐个 Cluster 做视锥剔除、背面剔除、HZB 遮挡剔除）
- **混合光栅化**：屏幕投影面积大 → 硬件光栅化（效率高）；投影面积小 → 软件光栅化（Compute Shader 实现，避免 Quad Overdraw）；极端小三角形 → 原子操作直接写入
- **Visibility Buffer**：几何阶段只写两样东西——深度和 MaterialID/TriangleID。材质属性在后续 Material Pass 中按需查询。相比传统 G-Buffer（每个像素 160+ 位），每像素仅 64-128 位，极大节省带宽
- **自动 LOD**：不是预先生成几级固定 LOD，而是从最精细几何通过误差引导的集群简化自动生成连续细节层次。GPU 沿 DAG 树向下遍历，根据屏幕空间误差逐节点决定细化或停止
- **流式加载**：Cluster 数据按页面组织（64KB），仅加载当前帧可见且所需 LOD 级别的页面。使用 LRU 策略淘汰不常用页面，预测性加载相机行进方向的内容

#### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| Nanite 高几何密度场景（百万级三角形） | `[待替换]` | UE5 官方 Nanite 演示截图（Valley of the Ancient） |
| Nanite Cluster 可视化（不同颜色显示不同 LOD 级别） | `[待替换]` | UE5 文档 Nanite Visualization 截图 |
| Nanite 开关对比（传统 LOD vs Nanite） | `[待替换]` | Epic GDC/SIGGRAPH Nanite 演讲截图 |

#### Nanite 局限性
- 不支持骨骼动画/蒙皮网格（Cluster 数据为预处理的静态几何）
- 不支持透明物体（Visibility Buffer 方案难以处理半透明排序）
- 薄几何（树叶、草）的 Cluster 分组和 LOD 简化容易产生视觉瑕疵
- 需要 Shader Model 6.6+ 和较新 GPU 架构
- 最适合：建筑可视化、遗迹重建、摄影测量扫描资产的高密度静态场景

---

### 2.2 Lumen — 全动态全局光照

#### 场景与阶段
> 全局光照方案决策阶段。传统方案要么静态烘焙（高品质但无法动态变化），要么屏幕空间（全动态但视野外无信息）。

#### 做了什么
- **Surface Cache 路径（漫反射主力）**：将场景体素化→提取表面卡片（Card）→在卡片间传播辐射度。Card 的间接光照跨帧异步更新，覆盖面广且开销稳定，适合大范围漫反射 GI
- **Screen Probe Gather 路径（高光/镜面主力）**：在屏幕空间自适应布设探针→球谐编码入射辐亮度→SDF/屏幕空间/硬件 RT 追踪→滤波积分为 SH 系数→逐像素插值着色。提供近处高频细节和镜面反射
- **双路径混合**：根据材质粗糙度自动混合。粗糙表面（漫反射为主）→ Surface Cache，光滑表面（需要镜面方向性）→ Screen Probe Gather

#### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| Lumen 室内全局光照效果（动态光源+间接光反弹） | `[待替换]` | UE5 官方 Lumen 演示截图 |
| Lumen Surface Cache 可视化 | `[待替换]` | UE5 文档 Lumen Debug View 截图 |
| Lumen 开关对比（Lumen vs 静态烘焙 Lightmap） | `[待替换]` | Epic SIGGRAPH 2022 Lumen 演讲截图 |

#### 与传统 Lightmap 对比

| 维度 | Lightmap 烘焙 | Lumen |
|------|-------------|-------|
| 制作流程 | 需要预烘焙，迭代慢 | 实时，所见即所得 |
| 光照动态性 | 完全静态 | 全动态 |
| 品质上限 | 极高（无限制计算预算） | 高（受实时预算约束） |
| 适用场景 | 最终品质打磨、静态光照 | 动态昼夜、可破坏环境、快速迭代 |

---

## 3. 影视级离线渲染 🔴

### 场景与阶段
> 影视/VFX 制作阶段。不受实时帧预算约束，追求物理精度与视觉品质极限。

### 做了什么
- **路径追踪渲染器对比**：RenderMan（Pixar，RIS 架构+多层材质）、Arnold（Autodesk，无偏差渲染+业界标准体积散射）、Cycles（Blender，开源方案+OptiX 硬件加速）
- **高级光传输**：BDPT（双向路径追踪，擅长焦散）、MLT（马尔可夫链蒙特卡洛，擅长极端困难光照）、VCM（统一 BDPT+光子映射）
- **降噪技术**：OptiX AI Denoiser（NVIDIA，CNN 降噪）、OpenImageDenoise（Intel，CPU AI 降噪）、时空组合降噪策略
- **材质标准统一**：MaterialX（跨工具材质交换）、OpenPBR（开放 PBR 标准）、Disney Principled BSDF（11 个直觉化参数统一材质描述）
- **色彩管理 ACES**：线性场景参考色彩空间→参考渲染变换→目标显示设备，保证跨工作室色彩一致性

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 离线路径追踪渲染成品帧（电影级画质） | `[待替换]` | RenderMan/Arnold 官方 Gallery |
| 降噪开关对比（噪声 vs 降噪后） | `[待替换]` | OpenImageDenoise 官方示例 |
| ACES 色彩管线流程图 | `[待替换]` | ACES Central 官方文档 |

### 📎 深入学习
- [Physically Based Rendering: From Theory to Implementation (PBR Book)](https://pbr-book.org/)
- [OpenPBR Specification](https://academysoftwarefoundation.github.io/OpenPBR/)
- [ACES Central](https://acescentral.com/)

---

## 4. 移动端与 Web 渲染 🔴

### 场景与阶段
> 移动端/Web 平台渲染适配阶段。在极其有限的功耗、带宽和 GPU 算力下，尽可能接近桌面端的视觉品质。

### 做了什么
- **TBDR 架构适配**：移动端 GPU（ARM Mali、Adreno、Apple GPU）采用 Tile-Based Deferred Rendering——先分块再着色，On-Chip Tile Memory 天然消除 Overdraw。避免频繁切换 Render Target（强制 Flush Tile Memory）、避免读取上一帧 Framebuffer 作为纹理（需要写回主存）
- **Vulkan Mobile 最佳实践**：管线预编译、Render Pass Subpass 表达数据依赖、LoadOp/StoreOp 精确控制、Push Descriptors 减少绑定开销
- **ASTC 纹理压缩**：自适应块大小（4×4 到 12×12），支持 LDR/HDR/3D/Cube Map。Base Color 推荐 6×6 块、Normal Map 推荐 5×5、Roughness/Metallic 可用 8×8
- **移动端 PBR 降级策略**：高端（1080p+ Full PBR+IBL，如原神）、中端（720p Cook-Torrance 单主光+静态烘焙，如王者荣耀）、低端（≤540p Lambert+Blinn）
- **WebGPU 迁移**：从 WebGL 2.0 全局状态机转向声明式 Pipeline，原生支持 Compute Shader（GPU 粒子、剔除、后处理加速）、多线程 Command Encoder、Render Bundle 预录制回放

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 原神移动端渲染效果 | `[待替换]` | 原神官方截图 |
| WebGPU 粒子系统演示 | `[待替换]` | WebGPU Samples 官方演示 |
| ASTC 不同块大小压缩质量对比 | `[待替换]` | ARM ASTC 官方文档 |

### 📎 深入学习
- [LearnOpenGL Guest Articles / WebGPU]
- [ARM Mali GPU 最佳实践]
- [Vulkan Mobile 最佳实践 (Samsung/Google)]

### 💡 实现原理
> 见 [Part 3 实现专栏](implementation/part3-implementation.md) — Nanite 软件光栅化 HLSL 伪代码、Lumen Radiance Propagation 追踪流程、WebGL PBR Shader 代码、WebGPU Compute 粒子系统、移动端带宽优化策略矩阵

---

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 3 实现专栏](implementation/part3-implementation.md)  
> 📖 继续阅读 [Part 4: 前沿发展](part4-frontier.md)
```

---

### Task 5: 重写 part4-frontier.md（4 章·纯场景层）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\part4-frontier.md`

**硬约束：** 不得出现任何 ` ``` ` 代码块（HLSL/GLSL 全删），不得出现任何数学公式。保留 🧭 本体系判断。

- [ ] **Step 1: 写入 Part 4 新内容**

写入 `e:\AI\bockwork\graphic\desgin\part4-frontier.md`：

```markdown
# Part 4: 前沿发展趋势 — 场景设计

> 从神经渲染到 Work Graphs，Part 4 追踪正在发生、尚未定论的变革。  
> 实现细节见 [Part 4 实现专栏](implementation/part4-implementation.md)。

---

## 1. Neural Rendering — 神经网络重塑渲染 💎

### 场景与阶段
> 渲染范式变革期。从「手工编写物理规则」到「数据驱动学习如何观察世界」。

### NeRF 发展里程碑

2020 NeRF → 2022 Instant-NGP (千倍加速、多分辨率 Hash Encoding) → 2023 3DGS (实时渲染 + 显式椭球表示)

---

### 1.1 NeRF 做了什么

- **隐式场景表示**：用一个小型 MLP 网络存储整个场景——输入空间坐标和视角方向，输出颜色和体密度。不需要显式的三角形网格
- **体渲染积分**：沿光线采样多个点，逐点送入 MLP 查询颜色和密度，然后沿光线累积得到像素颜色。整个过程可微分，支持端到端训练（只需要多视角照片）
- **Instant-NGP 千倍加速**：多分辨率 Hash Encoding 将空间划分为多级网格，每个顶点存储可学习的特征向量，替代大型 MLP 的逐层计算。查表+插值的速度远超深层网络

### 1.2 3DGS — 范式转换（核心锚定案例）💎

#### 场景与阶段
> 3DGS 用显式椭球体直接表示场景，通过可微分光栅化实现端到端优化——训练分钟级、渲染 >100FPS。

#### 做了什么
- **显式椭球表示**：场景 = 数百万个 3D 高斯椭球，每个椭球带有位置、协方差矩阵（形状/大小/朝向）、颜色（球谐函数编码视角依赖颜色）、不透明度
- **Splatting 渲染**：将 3D 高斯投影到 2D 屏幕（EWA Splatting），按深度排序后做 α 混合——相比 NeRF 的每像素 256 次 MLP 推理，3DGS 每像素只需一次椭球遍历
- **自适应密度控制**：训练过程中动态管理高斯数量——欠重建区域分裂/克隆高斯，低 α 或过大高斯删除，最终收敛到紧凑高质量表示
- **Tile-based Rasterizer**：GPU 友好的渲染器，将屏幕分块并行排序，充分利用 Shared Memory，实现 CUDA 级别的极致性能

#### 3DGS vs NeRF 根本差异

| 维度 | NeRF | 3DGS |
|------|------|------|
| 场景表示 | 隐式 (MLP 权重) | 显式 (百万高斯椭球) |
| 渲染方式 | 体积光线积分 | Splatting + α 混合 |
| 训练时间 | 小时~天 | 10-20 分钟 |
| 渲染 FPS | <1 | >100 |
| 可编辑性 | 困难 | 相对容易（直接操作高斯） |
| 游戏引擎集成 | 困难 | UE5/Unity 已有社区插件 |

#### 前沿扩展
- **4DGS**：时间维度加入，高斯参数随时间变化，实现动态场景重建
- **压缩**：向量量化+锚点表示，从几 GB 压缩到几十 MB，保持 90%+ 质量
- **Mesh 提取**：SuGaR/2DGS 将高斯平贴在表面上，Poisson 重建提取标准 Mesh——这是融入传统管线的关键桥梁

#### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| 3DGS 新视角渲染效果 | `[待替换]` | 3DGS 论文 (Kerbl et al., SIGGRAPH 2023) 关键帧 |
| NeRF vs 3DGS 渲染质量+速度对比 | `[待替换]` | 3DGS 论文对比表+截图 |
| 3DGS UE5 插件集成效果 | `[待替换]` | Luma AI / XVERSE 3DGS UE 插件演示 |

---

### 🧭 本体系判断

- **3DGS：值得深入**。已开始渗透游戏引擎、VR/AR、视觉特效。理解显式点云+可微分光栅化是理解「神经+传统混合管线」的钥匙
- **NeRF 经典公式：了解即可**。已被 InstantNGP/3DGS 取代，直接学后者
- **Mip-NeRF/Zip-NeRF：暂时跳过**。前沿研究，工程中暂无可落地方案

---

## 2. 实时路径追踪 — 电影级画质实时化 💎

### 场景与阶段
> 光照技术演进期。从光栅化 → 混合 RT → 全路径追踪的渐进路线。

### 做了什么
- **硬件 RT 核心演进**：NVIDIA RT Core（Turing→Ada→Blackwell）卸载光线-三角形求交和 BVH 遍历。SER（线程重排序避免 warp divergence）、OMM（预计算透明微图跳过无效 shade）、DMM（微网格位移）
- **降噪魔法**：SVGF（时域累积+空间 à-trous 滤波）、NRD（NVIDIA 实时降噪库：ReBLUR/ReLAX/SIGMA）、DLSS Ray Reconstruction（Transformer 神经网络从 1-2spp 重建干净画面）
- **商业落地里程碑**：Cyberpunk 2077 RT Overdrive（首个 AAA 全路径追踪）、Portal RTX（RTX Remix 翻新老游戏）、Alan Wake 2（混合 RT 渐进迁移）
- **实时 PT 的核心矛盾**：每像素 1-4 spp vs 离线 1000-10000 spp —— 差距由降噪+超分+帧生成填补

### RT 应用策略总结

| 策略 | 代表游戏 | 说明 |
|---|---|---|
| 仅 RT 阴影 | Shadow of the Tomb Raider | 最低成本 |
| RT 反射 + 阴影 | Spider-Man, Battlefield V | 最明显的视觉提升 |
| RTGI（全局光照） | Metro Exodus Enhanced | 全场景动态 GI |
| 混合 RT（多效果） | Alan Wake 2, Black Myth Wukong | 保留光栅化基础 |
| 全路径追踪 | Cyberpunk 2077 PT, Portal RTX | 完全物理正确 |

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| Cyberpunk 2077 路径追踪模式 画面对比 | `[待替换]` | NVIDIA RT Overdrive 官方截图 |
| DLSS Ray Reconstruction 降噪前后对比 | `[待替换]` | NVIDIA DLSS 3.5 官方演示截图 |

---

### 🧭 本体系判断

- **全路径追踪落地：了解里程碑**。Cyberpunk 2077 PT 证明了可行性，硬件要求决定普及还需 2-3 年
- **NRD / DLSS-RR：关注接口而非实现**。降噪器作为黑盒的接入方式比具体网络结构更重要

---

## 3. AI 辅助渲染与生成 💎

### 场景与阶段
> 渲染加速与内容生成阶段。AI 正在深度渗透渲染管线的每个环节。

### 做了什么
- **超分辨率与帧生成**：DLSS 4（Vision Transformer 架构→多帧生成 3-4×）、FSR 4（AMD 转向 ML 方法）、XeSS 2（Intel CNN+时序）。核心思路一致——历史帧+运动向量+神经网络 = 从低分辨率合成高清输出
- **神经纹理压缩（NTC）**：用小型 MLP 替代传统 BCn/ASTC 固定块编码——压缩率 15-20× 超越 BC7，同时质量更高。神经网络在 Shader 中推理（通过 Cooperative Vectors 硬件加速）
- **Photo→Material AI 管线**：输入单张照片 → 分割材质区域 → AI 估计 Albedo/Roughness/Normal/Metalness（去除光照影响）→ 输出完整 PBR 材质包。Adobe Substance Sampler 已是工业标准
- **Text-to-3D 生成**：DreamFusion→DreamGaussian→Meshy/Luma Genie 的演进路线，从小时级到分钟级，从研究到商业 API

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| DLSS 超分辨率 开关对比 | `[待替换]` | NVIDIA DLSS 官方演示截图 |
| 神经纹理压缩 NTC vs BC7 对比 | `[待替换]` | NVIDIA NTC 论文对比图 |

---

### 🧭 本体系判断

- **DLSS/FSR/XeSS：了解差异即可**。三种方案已趋同，选型取决于目标硬件（NVIDIA→DLSS, AMD→FSR, Intel→XeSS）
- **神经纹理压缩：关注但暂不投入**。压缩率惊艳，生态成熟需等待硬件原生支持
- **Text-to-3D：实验性关注**。输出质量距生产级别的几何/材质还有差距

---

## 4. 下一代渲染范式 💎

### 场景与阶段
> 管线架构重塑期。GPU 从「执行者」变为「自主决策者」。

### 做了什么
- **Mesh Shading**：以 Meshlet（64 顶点+84 三角形）替代三角形为最小处理单元。Amplification Shader 阶段 GPU 可动态决定工作量——视锥体剔除、LOD 选择全部在 GPU 上完成，零 CPU 参与。UE5 Nanite 的 Cluster 系统基于此思想
- **Work Graphs (DirectX 12)**：GPU 自调度计算图。传统模式 CPU 逐个 dispatch GPU 工作并等待结果→Work Graphs 让 GPU 自行管理依赖链：生成高度场→自动触发岩石生成→自动触发 Mesh 生成→自动触发渲染，零 CPU-GPU 往返
- **Neural Shader**：在渲染管线中嵌入小型神经网络。Cooperative Vectors 指令让 Shader 能高效执行矩阵-向量乘法（Tensor Core 加速），网络推理替代手工 BRDF 公式、纹理采样、阴影计算

### 2030 混合管线推测
> 传统 Mesh Shader（主几何体） + Neural Shader（程序化细节） + 3DGS（远景/背景） + Path Tracing（镜面反射/折射） + Neural Material（BRDF 评估） + DLSS RR（降噪+超分） + DLSS MFG（多帧生成）

### 效果展示

| 效果 | 占位 | 参考来源 |
|---|---|---|
| Mesh Shader 管线流水示意图 | `[待替换]` | DirectX 12 Ultimate 文档 |
| Work Graphs GPU 自调度示例 | `[待替换]` | Microsoft DirectX Work Graphs 演示 |

---

### 🧭 本体系判断

- **Mesh Shading：即将成为必须**。DX12 Ultimate 推广 + UE5 Nanite 背书，meshlet 思维是新一代几何管线基础
- **Work Graphs：关注微软动态**。GPU 自调度是范式级进步，AMD 兼容性和实际落地案例待观察
- **Neural Shader：远期储备**。Cooperative Vectors 尚未普及，但方向明确——神经网络将逐渐渗透到管线的每个阶段

---

## 三条确定性趋势

1. **物理→神经混合管线下移**：3DGS 类方法进入实时应用，取代部分传统 LOD/Imposter
2. **GPU 调度权上移**：Work Graphs / Mesh Shader 让 GPU 自调度，减少 CPU 往返
3. **AI 降噪 + 超分 = 新标准**：DLSS/FSR/XeSS 成为 AAA 游戏标配，神经降噪替代传统 TAA

---

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 4 实现专栏](implementation/part4-implementation.md)  
> 📖 返回 [Part 1](part1-foundation.md) | [Part 2](part2-advanced.md) | [Part 3](part3-commercial.md)
```

---

### Task 6: 微调 graphics-programming-handbook.md（总索引）

**Files:**
- 修改：`e:\AI\bockwork\graphic\desgin\graphics-programming-handbook.md`

- [ ] **Step 1: 更新文件索引 + 添加实现专栏入口**

写入 `e:\AI\bockwork\graphic\desgin\graphics-programming-handbook.md`（保留六维雷达图、五圈螺旋、学习路线和参考资源，只更新文件索引部分）：

```markdown
# 图形学编程全覆盖知识库

> **设计：** 六维能力雷达图 × 五圈案例螺旋  
> **定位：** 从零基础到前沿趋势的图形学全阶段知识体系  
> **参考蓝本：** [Graphics Programming – Where To Start?](https://stefanpijnacker.nl/article/graphics-programming-where-to-start/)
>  
> 本体系内化整合 [Games101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)（理论）和 [LearnOpenGL](https://learnopengl-cn.github.io/)（实践）的核心知识。**主文档聚焦「做什么」与「场景设计」，实现原理见实现专栏。**

---

## 能力雷达图（六维总览）

```
                     数学与理论基础
                           ▲
                          / \
                         / ① \
                        /     \
          性能优化     /       \    图形API
          与硬件     /  ⑥   ②  \   与管线
                   /           \
                  /    🌀核心    \
                 /     案例线     \
                /                 \
          引擎架构  \  ⑤   ④  /  着色器编程
          与工具链   \       /
                     \  ③ /
                      \ /
                  渲染技术与算法
```

| 编号 | 维度 | 涵盖内容 |
|:----:|------|----------|
| ① | 数学与理论基础 | 线性代数、向量/矩阵/变换、辐射度量学、几何光学 |
| ② | 图形API与管线 | OpenGL/Vulkan/DX12/Metal/WebGPU、固定→可编程管线 |
| ③ | 着色器编程 | GLSL/HLSL/WGSL、ShaderToy、Ray Marching/SDF |
| ④ | 渲染技术与算法 | 光栅化→光追→路径追踪、PBR/GI/后处理、粒子/体积 |
| ⑤ | 引擎架构与工具链 | UE5/Unity渲染架构、RenderGraph、资产管线 |
| ⑥ | 性能优化与硬件 | GPU微架构、CUDA/CS加速、RenderDoc/NSight |

---

## 五圈螺旋速览

| 圈层 | 名称 | 目录 | 锚点项目 |
|:----:|------|------|----------|
| 1 | 🟢 启蒙 | [Part 1](./part1-foundation.md) | 旋转的彩色立方体 |
| 2 | 🟡 筑基 | [Part 1](./part1-foundation.md) | PBR材质球预览器 |
| 3 | 🟡 精进 | [Part 2](./part2-advanced.md) | 迷你路径追踪渲染器 |
| 4 | 🔴 实战 | [Part 3](./part3-commercial.md) | UE5 Nanite/Lumen 拆解 |
| 5 | 🔴 前沿 | [Part 4](./part4-frontier.md) | NeRF→3DGS 对比实验 |

---

## 学习路线选择

| 你的目标 | 核心章节 | 辅助章节 | 建议跳过 |
|---|---|---|---|
| 游戏渲染 | Part 1 模块1-3 + Part 2 Case 1-4 + Part 3 第1-2章 | Part 4 第2-3章 | 影视离线渲染 |
| 图形学研究 | Part 1 全部 + Part 2 Case 1,5 + Part 4 第1章 | Part 3 第3章 | 移动端渲染 |
| 自研引擎 | Part 1 全部 + Part 2 全部 + Part 3 第1-2,4章 | Part 4 第2,4章 | — |
| Web/移动 | Part 1 模块1-3 + Part 3 第4章 | Part 2 Case 1-3 | GPU Driven/Case 4 |

---

## 难度标签约定

| 标签 | 含义 |
|:----:|------|
| 🟢 | 入门 — 零基础可读 |
| 🟡 | 进阶 — 需要基本图形学概念或完整管线知识 |
| 🔴 | 前沿 — 涉及最新论文和研究方向 |

---

## 参考资源索引

> 本体系预设读者已完成或同步学习 [Games101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)（理论）和 [LearnOpenGL](https://learnopengl-cn.github.io/)（实践），不再重复推荐同类入门资源。

### 数学基础
- 3Blue1Brown [「线性代数的本质」](https://www.youtube.com/playlist?list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab)
- [Foundations of Game Engine Development Vol.1](https://foundationsofgameenginedev.com/#fged1) — Eric Lengyel

### 图形API与渲染
- [Learning DirectX 12](https://www.3dgep.com/learning-directx-12-1/) — Jeremiah van Oosten
- [Real-Time Rendering](https://www.realtimerendering.com/) — Akenine-Möller et al.
- [GPU Gems](https://developer.nvidia.com/gpugems) — NVIDIA

### Shader编程
- [The Book of Shaders](https://thebookofshaders.com/) — Patricio Vivo & Jen Lowe
- [ShaderToy](https://www.shadertoy.com/) — Inigo Quilez
- [Iquilezles](https://iquilezles.org/) — Inigo Quilez 技术文章

### 进阶阅读
- [Physically Based Rendering: From Theory to Implementation](https://pbr-book.org/) — Pharr, Jakob, Humphreys
- [Ray Tracing in One Weekend](https://raytracing.github.io/) — Peter Shirley

---

## 文件索引

| 文件 | 内容 |
|------|------|
| `graphics-programming-handbook.md` | 📍 本文件 — 总索引 |
| `part1-foundation.md` | 🟢 Part 1：基础入门 — 4 模块·场景层<br>模块1 空间变换 \| 模块2 光照着色 \| 模块3 渲染架构 \| 模块4 Shader思维 |
| `part2-advanced.md` | 🟡 Part 2：高级渲染 — 5 Case·场景层<br>PBR / 延迟渲染 / 屏幕空间 / GPU Driven / 体积渲染 |
| `part3-commercial.md` | 🔴 Part 3：商业级别 — 场景层<br>引擎架构 / AAA方案 / 离线渲染 / 移动端Web |
| `part4-frontier.md` | 💎 Part 4：前沿发展 — 场景层<br>Neural / 路径追踪 / AI辅助 / 下一代范式 |

### 实现专栏 🔧

> 详细实现原理、代码片段和公式推导见以下专栏（后续补充）：

| 文件 | 对应主文档 | 状态 |
|---|---|---|
| [implementation/part1-implementation.md](implementation/part1-implementation.md) | Part 1 | 🔜 待补充 |
| [implementation/part2-implementation.md](implementation/part2-implementation.md) | Part 2 | 🔜 待补充 |
| [implementation/part3-implementation.md](implementation/part3-implementation.md) | Part 3 | 🔜 待补充 |
| [implementation/part4-implementation.md](implementation/part4-implementation.md) | Part 4 | 🔜 待补充 |
```

---

### Task 7: 全体系一致性校验

- [ ] **Step 1: 规则检查**

Grep 所有 5 个主文档，确认：
- 全文搜索 ` ``` ` — 预期全部命中 0 次（无代码块）
- Part 1 每个模块有「场景与阶段」「做了什么」「📎 深入学习」「💡 实现原理」
- Part 4 有 4 处 🧭 本体系判断
- Part 3 保留引擎对比表（`| 引擎 |`）

```powershell
Select-String -Path 'e:\AI\bockwork\graphic\desgin\part1-foundation.md','e:\AI\bockwork\graphic\desgin\part2-advanced.md','e:\AI\bockwork\graphic\desgin\part3-commercial.md','e:\AI\bockwork\graphic\desgin\part4-frontier.md' -Pattern '```' | Measure-Object | Select-Object -ExpandProperty Count
```

预期输出：0

- [ ] **Step 2: 篇幅统计**

```powershell
Get-ChildItem e:\AI\bockwork\graphic\desgin\graphics-programming-handbook.md,e:\AI\bockwork\graphic\desgin\part1-foundation.md,e:\AI\bockwork\graphic\desgin\part2-advanced.md,e:\AI\bockwork\graphic\desgin\part3-commercial.md,e:\AI\bockwork\graphic\desgin\part4-frontier.md | ForEach-Object { $lines = (Get-Content $_.FullName | Measure-Object -Line).Lines; Write-Output "$($_.Name): $lines lines" }
```

预期：全体系 ~1570 行

- [ ] **Step 3: Implementation 目录检查**

```powershell
Test-Path 'e:\AI\bockwork\graphic\desgin\implementation'
Test-Path 'e:\AI\bockwork\graphic\desgin\implementation\README.md'
```

预期：均为 True

- [ ] **Step 4: 更新预览页面 index.html 课程引用**

确保 index.html 侧边栏和引用链接与修改后的文件结构一致。
