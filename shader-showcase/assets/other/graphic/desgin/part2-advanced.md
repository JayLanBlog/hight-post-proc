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

![PBR 金属球系列](https://learnopengl.com/img/lighting/basic_lighting_phong.png)

![Phong vs PBR 对比](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

### 📎 深入学习
- 理论：[Games101 Lec 17 — 材质与外观](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_17.pdf)
- 实践：[LearnOpenGL PBR 系列](https://learnopengl-cn.github.io/07%20PBR/)

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

![延迟渲染多光源场景](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

### 📎 深入学习
- 实践：[LearnOpenGL 延迟着色](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/08%20Deferred%20Shading/)

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

![SSAO 环境光遮蔽](https://learnopengl.com/img/advanced-lighting/ssao.png)

![Bloom 泛光效果](https://learnopengl.com/img/advanced-lighting/bloom.png)

### 📎 深入学习
- 实践：[LearnOpenGL SSAO](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/09%20SSAO/) | [LearnOpenGL Bloom](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/07%20Bloom/)

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

![GPU Driven 森林场景](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

### 📎 深入学习
- 延伸：UE5 Nanite (见 Part 3) / Mesh Shading (见 Part 4)
- 实践：[DirectX 12 Mesh Shader](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html)

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

![体积光效果](https://learnopengl.com/img/advanced-lighting/bloom.png)

![体积云](https://learnopengl.com/img/advanced-lighting/ssao.png)

### 📎 深入学习
- 实践：[LearnOpenGL 阴影映射](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/)

### 💡 实现原理
> 见 [Part 2 实现专栏](implementation/part2-implementation.md) — 体积光屏幕空间 Ray Marching、粒子物理 Compute Shader、Bill boarding 顶点着色器、Froxel 体素化与散射计算、Worley 噪声密度函数、Beer-Lambert 光线步进、最终合成管线

---

> 📎 理论对照 [Games101 课程](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)  
> 💻 实践对照 [LearnOpenGL 教程](https://learnopengl-cn.github.io/)  
> 🔧 实现原理见 [Part 2 实现专栏](implementation/part2-implementation.md)  
> 📖 继续阅读 [Part 3: 商业级别](part3-commercial.md)
