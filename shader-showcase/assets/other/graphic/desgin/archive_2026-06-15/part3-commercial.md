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
- **Cluster 集群化**：将最小处理单元从单个三角形提升到 Cluster（约 128 个三角形），附带包围盒和朝向锥。所有后续剔除和 LOD 选择都在 Cluster 级别执行
- **两级剔除**：Instance 级（粗粒度，用包围盒+HZB 遮挡查询过滤整个物体），Cluster 级（细粒度，逐个 Cluster 做视锥剔除、背面剔除、HZB 遮挡剔除）
- **混合光栅化**：屏幕投影面积大 → 硬件光栅化（效率高）；投影面积小 → 软件光栅化（Compute Shader 实现，避免 Quad Overdraw）
- **Visibility Buffer**：几何阶段只写深度和 MaterialID/TriangleID。材质属性在后续 Material Pass 中按需查询。比传统 G-Buffer（每像素 160+ 位）仅需 64-128 位，极大节省带宽
- **自动 LOD**：从最精细几何通过误差引导的集群简化自动生成连续细节层次。GPU 沿 DAG 树向下遍历，根据屏幕空间误差逐节点决定细化或停止
- **流式加载**：Cluster 数据按页面组织（64KB），仅加载当前帧可见且所需 LOD 级别的页面

#### 效果展示

![Nanite 高几何密度场景](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

#### Nanite 局限性
- 不支持骨骼动画/蒙皮网格（Cluster 数据为预处理的静态几何）
- 不支持透明物体（Visibility Buffer 方案难以处理半透明排序）
- 薄几何（树叶、草）的 Cluster 分组和 LOD 简化容易产生视觉瑕疵
- 需要 Shader Model 6.6+ 和较新 GPU 架构

---

### 2.2 Lumen — 全动态全局光照

#### 场景与阶段
> 全局光照方案决策阶段。传统方案要么静态烘焙（高品质但无法动态变化），要么屏幕空间（全动态但视野外无信息）。

#### 做了什么
- **Surface Cache 路径（漫反射主力）**：将场景体素化→提取表面卡片（Card）→在卡片间传播辐射度。Card 的间接光照跨帧异步更新，覆盖面广且开销稳定
- **Screen Probe Gather 路径（高光/镜面主力）**：在屏幕空间自适应布设探针→球谐编码入射辐亮度→SDF/屏幕空间/硬件 RT 追踪→滤波积分→逐像素插值着色
- **双路径混合**：根据材质粗糙度自动混合。粗糙表面 → Surface Cache，光滑表面 → Screen Probe Gather

#### 效果展示

![Lumen 室内全局光照](https://learnopengl.com/img/advanced-lighting/bloom.png)

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
- **路径追踪渲染器对比**：RenderMan（Pixar，RIS 架构+多层材质）、Arnold（Autodesk，无偏差渲染）、Cycles（Blender，开源+OptiX 硬件加速）
- **高级光传输**：BDPT（双向路径追踪，擅长焦散）、MLT（马尔可夫链蒙特卡洛）、VCM（统一 BDPT+光子映射）
- **降噪技术**：OptiX AI Denoiser（NVIDIA，CNN 降噪）、OpenImageDenoise（Intel，CPU AI 降噪）
- **材质标准统一**：MaterialX（跨工具材质交换）、OpenPBR（开放 PBR 标准）、Disney Principled BSDF（11 个参数统一材质描述）
- **色彩管理 ACES**：线性场景参考色彩空间→参考渲染变换→目标显示设备

### 效果展示

![离线路径追踪渲染（电影级画质）](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

### 📎 深入学习
- [Physically Based Rendering (PBR Book)](https://pbr-book.org/)
- [OpenPBR Specification](https://academysoftwarefoundation.github.io/OpenPBR/)
- [ACES Central](https://acescentral.com/)

---

## 4. 移动端与 Web 渲染 🔴

### 场景与阶段
> 移动端/Web 平台渲染适配阶段。在极其有限的功耗、带宽和 GPU 算力下，尽可能接近桌面端的视觉品质。

### 做了什么
- **TBDR 架构适配**：移动端 GPU（ARM Mali、Adreno、Apple GPU）采用 Tile-Based Deferred Rendering——先分块再着色，On-Chip Tile Memory 天然消除 Overdraw
- **Vulkan Mobile 最佳实践**：管线预编译、Render Pass Subpass 表达数据依赖、LoadOp/StoreOp 精确控制
- **ASTC 纹理压缩**：自适应块大小（4×4 到 12×12），支持 LDR/HDR/3D/Cube Map
- **移动端 PBR 降级策略**：高端（1080p+ Full PBR+IBL）、中端（720p Cook-Torrance 单主光）、低端（≤540p Lambert+Blinn）
- **WebGPU 迁移**：从 WebGL 2.0 全局状态机转向声明式 Pipeline，原生支持 Compute Shader

### 移动端 PBR 降级策略

| 预算级别 | 分辨率 | 动态光源 | PBR方案 | 阴影 | 后处理 | 代表案例 |
|---|---|---|---|---|---|---|
| 高端(旗舰) | 1080p+ | <8 | Full PBR+IBL | CSM 2级 | Bloom+AO | 原神 |
| 中端 | 720p | <4 | Cook-Torrance 单主光 | 静态烘焙 | ❌ | 王者荣耀 |
| 低端 | ≤540p | 1 方向光 | Lambert+Blinn | ❌ | ❌ | 轻量游戏 |

### 效果展示

![原神移动端渲染](https://learnopengl.com/img/getting-started/transformations.png)

### 📎 深入学习
- [LearnOpenGL 中文站](https://learnopengl-cn.github.io/)
- [ARM Mali GPU 最佳实践](https://developer.arm.com/documentation)
- [Vulkan Mobile 最佳实践 (Samsung)](https://developer.samsung.com/galaxy-gamedev)

### 💡 实现原理
> 见 [Part 3 实现专栏](implementation/part3-implementation.md) — Nanite 软件光栅化 HLSL 伪代码、Lumen Radiance Propagation 追踪流程、WebGL PBR Shader 代码、WebGPU Compute 粒子系统、移动端带宽优化策略矩阵

---

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 3 实现专栏](implementation/part3-implementation.md)  
> 📖 继续阅读 [Part 4: 前沿发展](part4-frontier.md)
