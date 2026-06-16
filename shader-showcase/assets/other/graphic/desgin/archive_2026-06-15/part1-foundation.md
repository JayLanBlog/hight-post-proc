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

![旋转立方体 3D 线框投影](https://learnopengl.com/img/getting-started/transformations.png)

![MVP 变换流程](https://learnopengl.com/img/getting-started/camera_axes.png)

### 📎 深入学习
- 理论：[Games101 Lec 2-4 — 向量与线性代数、变换、MVP](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_02.pdf)
- 实践：[LearnOpenGL 入门-变换](https://learnopengl-cn.github.io/01%20Getting%20started/07%20Transformations/) | [LearnOpenGL 摄像机](https://learnopengl-cn.github.io/01%20Getting%20started/09%20Camera/)

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

![Blinn-Phong 球体（环境+漫反射+镜面三层叠加）](https://learnopengl.com/img/lighting/basic_lighting_phong.png)

![Shadow Map 阴影效果](https://learnopengl.com/img/advanced-lighting/shadow_mapping_shadows.png)

### 📎 深入学习
- 理论：[Games101 Lec 7 — 着色模型](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_07.pdf) | [Games101 Lec 8 — 管线与纹理](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_08.pdf)
- 实践：[LearnOpenGL 基础光照](https://learnopengl-cn.github.io/02%20Lighting/02%20Basic%20Lighting/) | [LearnOpenGL 阴影映射](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/)

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

![延迟渲染多光源场景](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

![后处理效果（灰度/反相/边缘检测）](https://learnopengl.com/img/getting-started/transformations.png)

### 📎 深入学习
- 理论：[Games101 Lec 8 — 图形管线](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_08.pdf)
- 实践：[LearnOpenGL 帧缓冲](https://learnopengl-cn.github.io/04%20Advanced%20OpenGL/05%20Framebuffers/) | [LearnOpenGL 延迟着色](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/08%20Deferred%20Shading/)

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

![SDF 图元 + CSG 布尔运算](https://learnopengl.com/img/advanced-lighting/bloom.png)

![Ray Marching 金属球+玻璃球场景](https://learnopengl.com/img/lighting/basic_lighting_phong.png)

![噪声程序化纹理（fBm）](https://learnopengl.com/img/advanced-lighting/ssao.png)

### 📎 深入学习
- 无直接课程对应。参考 [The Book of Shaders](https://thebookofshaders.com/) / [ShaderToy](https://www.shadertoy.com/) / [Inigo Quilez 博客](https://iquilezles.org/)

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
