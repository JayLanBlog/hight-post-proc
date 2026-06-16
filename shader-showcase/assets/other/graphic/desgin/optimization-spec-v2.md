# 图形学设计文档 — 第二轮优化规格（场景层·剥离实现）

> 日期：2026-06-14
> 目标：主文档只保留场景描述+效果图占位+课程引用，代码/公式/推导全部剥离到独立实现专栏（后续设计）

---

## 一、文件架构

```
desgin/
├── graphics-programming-handbook.md   ← 总索引（微调引用路径 + 新结构描述）
├── part1-foundation.md               ← 重写：纯场景描述层
├── part2-advanced.md                 ← 重写：纯场景描述层
├── part3-commercial.md               ← 重写：纯场景描述层
├── part4-frontier.md                 ← 重写：纯场景描述层
└── implementation/                   ← 后续设计，本次不动
    ├── part1-implementation.md
    ├── part2-implementation.md
    ├── part3-implementation.md
    └── part4-implementation.md
```

## 二、主文档模块格式模板

每个模块统一使用以下模板：

```markdown
## 模块X: 标题 (难度标签)

### 场景与阶段
> 在渲染管线的哪个阶段、什么场景下需要这个技术（1-2句）

### 做了什么
- 核心功能点（2-3条，无公式无代码，纯中文描述）

### 效果展示
![效果描述](图片URL占位)
<!-- 图源: LearnOpenGL / Games101课件 / ShaderToy / UE5文档 / 后续替换 -->

### 📎 深入学习
- 理论：[Games101 Lec X] 对应章节链接
- 实践：[LearnOpenGL 对应教程] 对应教程链接

### 💡 实现原理
> 详细实现见 [Part X 实现专栏](implementation/partX-implementation.md)（后续补充）
```

**硬约束：**
- ❌ 主文档中不得出现任何代码块（\`\`\`）
- ❌ 主文档中不得出现任何数学公式推导
- ❌ 不展开技术原理（留给实现专栏）
- ✅ 可保留内联代码名（如 \`glm::lookAt\`、\`fragCoord\`），不超过5字

---

## 三、Part 1 重写结构（基础入门）

### 模块1: 空间变换 — 从坐标到屏幕 🟢

**场景与阶段**：顶点着色器阶段，物体从本地坐标系开始，经过 Model→View→Projection 三步变换，最终抵达屏幕。

**做了什么：**
- **MVP 变换**：Model 变换将物体从本地空间搬到世界空间；View 变换将整个世界"装进"相机取景框；Projection 变换将 3D 场景压扁到 2D 平面，同时保留深度用于遮挡判定
- **坐标系切换**：全过程涉及右手系（OpenGL）和左手系（DirectX）的转换，核心区别在于 Z 轴正方向朝向
- **旋转无死角**：用四元数替代欧拉角做旋转，避免"万向节锁"——使用 \`glm::quat\` 直接封装，一次绕轴旋转解决问题

**效果展示**：
| 阶段 | 图片 | 来源 |
|---|---|---|
| MVP流程示意 | `[占位]` 参考 Games101 Lec 4 课件第15页 | games101 |
| 旋转立方体 3D → 2D 投影 | `[占位]` 参考 LearnOpenGL 入门-坐标系 最终截图 | learnopengl |

**📎 深入学习**：理论 [Games101 Lec 2-4] / 实践 [LearnOpenGL 入门-变换-坐标系-摄像机]

**💡 实现原理**：见 Part 1 实现专栏（GLM 矩阵运算、VAO/VBO 配置、Shader 编译链接流程）


### 模块2: 光照着色 — 物体为什么有颜色 🟢

**场景与阶段**：片元着色器阶段，已知表面位置和法线，需要计算该点最终呈现的颜色。

**做了什么：**
- **Blinn-Phong 光照模型**：将光照分解为环境光（全局底色）、漫反射（粗糙表面均匀散射）和镜面反射（光滑表面高光）三层叠加。Blinn 用半角向量 \`halfVec\` 替代 Phong 的反射向量，计算更快更自然
- **阴影映射（Shadow Map）**：先以光源视角渲染一张深度图，正常渲染时比较片元深度——比深度图"远"则判定被遮挡
- **软阴影（PCF）**：采样周围像素做平均，消除硬边缘锯齿，模拟自然阴影的半影过渡

**效果展示**：
| 效果 | 图片 | 来源 |
|---|---|---|
| Blinn-Phong 球体（环境+漫反射+镜面反射三层叠加） | `[占位]` 参考 LearnOpenGL 光照-Basic Lighting 最终截图 | learnopengl |
| Shadow Map 硬阴影 vs PCF 软阴影对比 | `[占位]` 参考 LearnOpenGL Shadow Mapping 最终截图 | learnopengl |

**📎 深入学习**：理论 [Games101 Lec 7-8] / 实践 [LearnOpenGL 光照-阴影系列]

**💡 实现原理**：见 Part 1 实现专栏（Blinn-Phong GLSL、Shadow Map 两步法、PCF 采样核）


### 模块3: 渲染架构 — 组织画面而非画单个物体 🟢

**场景与阶段**：引擎渲染架构设计阶段——决定几何数据如何组织、光照何时计算、后处理何时叠加。

**做了什么：**
- **离屏渲染（FBO）**：先把场景渲染到纹理而非屏幕，再将纹理当素材做后处理——反相、灰度、边缘检测、模糊等全屏效果实质是"把渲染结果当纹理逐像素处理"
- **延迟渲染**：先过一遍几何（填 G-Buffer，记录位置/法线/颜色/材质），再过一遍光照（全屏累加光源）。几何 O(N) + 光照 O(M) 替代前向渲染的 O(N×M)，让 100+ 动态光源成为可能
- **后处理管线**：边缘检测（Sobel 卷积）、模糊（高斯核）、Bloom（亮部提取→模糊→叠加），全部通过全屏着色器实现

**效果展示**：
| 效果 | 图片 | 来源 |
|---|---|---|
| 延迟渲染 100 动态点光源场景 | `[占位]` 参考 LearnOpenGL Deferred Shading 最终截图 | learnopengl |
| 后处理效果切换（灰度/反相/边缘检测/模糊） | `[占位]` 参考 LearnOpenGL Framebuffers 后处理截图 | learnopengl |

**📎 深入学习**：理论 [Games101 Lec 8] / 实践 [LearnOpenGL 高级OpenGL-帧缓冲-延迟着色]

**💡 实现原理**：见 Part 1 实现专栏（FBO 创建/绑定、G-Buffer 布局、光照累积 Pass、后处理着色器）


### 模块4: Shader思维 ★核心差异化★ 🟡

> Games101 和 LearnOpenGL 均未覆盖，是本体系独特价值。

**场景与阶段**：完全超越传统管线——不需要三角形、不需要 VAO/VBO，纯着色器计算定义场景和光照。

**做了什么：**
- **SDF有向距离场**：用数学函数定义几何体——球体=到球心距离减半径、立方体=轴对称距离。用 CSG 操作（并/交/差/平滑混合）像搭积木一样组合复杂形状
- **Ray Marching 光线步进**：从屏幕每个像素发射射线，沿射线方向一步步"安全前进"（每步距离=当前到最近表面的距离），撞到表面后计算法线和光照——全过程无三角形
- **噪声与程序化纹理**：Value/Perlin/Simplex/Voronoi 四类噪声 + fBm（分形叠加），用纯数学生成云、地形、水渍等自然纹理

**效果展示**：
| 效果 | 图片 | 来源 |
|---|---|---|
| SDF 图元（球/立方/环面）+ CSG 组合渲染 | `[占位]` ShaderToy: 搜索 "SDF primitives" 任意作品截图 | shadertoy |
| Ray Marching 完整场景（金属球+玻璃球+软阴影+地面反射） | `[占位]` ShaderToy: 搜索 "ray marching spheres" 任意作品截图 | shadertoy |
| 噪声程序化纹理（fBm 多层叠加效果） | `[占位]` ShaderToy: 搜索 "fbm noise" 任意作品截图 或 The Book of Shaders Noise章节 | shadertoy |

**📎 深入学习**：本体系独有内容，无直接对应课程。参考 [The Book of Shaders] / [ShaderToy] / [Inigo Quilez 博客]

**💡 实现原理**：见 Part 1 实现专栏（SDF 图元 GLSL、CSG 操作、Ray Marching 主循环、噪声哈希函数、fBm 叠加逻辑、完整场景代码）

---

## 四、Part 2-4 重写要点

### Part 2（高级渲染）5 Case 格式统一

| Case | 场景与阶段 | 核心做了什么 | 效果图来源占位 |
|---|---|---|---|
| Case 1 PBR管线 | 材质系统阶段，替代传统 Phong | 微表面 BRDF（Cook-Torrance GGX）统一描述金属/非金属材质；IBL 三件套将环境光预计算为可查表纹理 | LearnOpenGL PBR Lighting 金属球+粗糙度对比截图 |
| Case 2 延迟渲染+海量光源 | 管线架构阶段，需支持 100+ 动态光源 | Tiled Light Culling 将屏幕分块、每块只处理影响该块的光源 | LearnOpenGL Deferred Shading 截图 |
| Case 3 屏幕空间效果 | 后处理阶段，给画面加"滤镜" | SSAO（遮挡暗化）、SSR（屏幕空间反射）、Bloom（亮部光晕）、DoF（景深模糊） | LearnOpenGL SSAO/Bloom 截图 |
| Case 4 GPU Driven | 渲染提交阶段，CPU 忙不过来时 | 把剔除/遮挡/LOD 判断全搬到 GPU Compute Shader | UE5 Nanite 文档截图 或 GPU Open 森林场景 |
| Case 5 体积渲染 | 氛围渲染阶段，让"空气可见" | God Rays 体积光、GPU 粒子系统、Worley 噪声体积云、Froxel 体积雾 | UE5 Lumen/Fog 截图 或 ShaderToy 体积云作品 |

### Part 3（商业级别）聚焦"决策"

- 保留当前表格式引擎对比和 Nanite/Lumen 分析框架，**但剥离所有代码块**
- Nanite 深度分析只保留"做了什么"级描述（Cluster 两级剔除、Visibility Buffer、混合光栅化决策），删 HLSL 伪代码
- 每个技术方案后面加 "效果展示" 占位：UE5 官方截图 / GDC 演示截图

### Part 4（前沿发展）聚焦"趋势+判断"

- 保留当前 🧭 本体系判断 结构，删代码块
- Neural Rendering 保留 3DGS 核心描述，删 NeRF 公式
- 效果图用论文关键图 / 社区作品截图占位

---

## 五、总索引微调

- 更新 Part 1 描述为「4 模块·场景层」+ 标注「实现原理见实现专栏」
- 学习路线选择表保持不变
- 参考资源中增加 LearnOpenGL / Games101 作为首要引用
- 增加实现专栏文件索引（标注为"后续补充"）

---

## 六、篇幅预估

| 文件 | 当前行数 | 预期缩减 | 主要改动 |
|---|---|---|---|
| part1-foundation.md | 624 | → ~200 | 全删代码块，保留场景描述+效果图占位+课程引用 |
| part2-advanced.md | 1548 | → ~350 | 5 Case 全删代码块+推导，统一模板 |
| part3-commercial.md | 981 | → ~400 | 删代码块，Nanite/Lumen 只保留架构描述 |
| part4-frontier.md | 1307 | → ~500 | 删代码块，保留趋势判断 |
| handbook.md | 110 | → ~120 | 微调引用路径 |

全体系从当前 3423 行缩减到 **~1570 行**（-54%），同时新增 implementation/ 目录留空待后续填充。
