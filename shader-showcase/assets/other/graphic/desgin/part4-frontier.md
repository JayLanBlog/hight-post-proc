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
- **Instant-NGP 千倍加速**：多分辨率 Hash Encoding 将空间划分为多级网格，每个顶点存储可学习的特征向量，替代大型 MLP 的逐层计算

### 1.2 3DGS — 范式转换（核心锚定案例）💎

#### 场景与阶段
> 3DGS 用显式椭球体直接表示场景，通过可微分光栅化实现端到端优化——训练分钟级、渲染 >100FPS。

#### 做了什么
- **显式椭球表示**：场景 = 数百万个 3D 高斯椭球，每个椭球带有位置、协方差矩阵（形状/大小/朝向）、颜色（球谐函数编码视角依赖颜色）、不透明度
- **Splatting 渲染**：将 3D 高斯投影到 2D 屏幕（EWA Splatting），按深度排序后做 α 混合——相比 NeRF 每像素 256 次 MLP 推理，3DGS 每像素只需一次椭球遍历
- **自适应密度控制**：训练过程中动态管理高斯数量——欠重建区域分裂/克隆高斯，低 α 或过大高斯删除，最终收敛到紧凑高质量表示
- **Tile-based Rasterizer**：GPU 友好的渲染器，将屏幕分块并行排序，充分利用 Shared Memory

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
- **Mesh 提取**：SuGaR/2DGS 将高斯平贴在表面上，Poisson 重建提取标准 Mesh

#### 效果展示

![3DGS 渲染效果](https://learnopengl.com/img/advanced-lighting/bloom.png)

![NeRF vs 3DGS 对比](https://learnopengl.com/img/advanced-lighting/deferred_shading.png)

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
- **硬件 RT 核心演进**：NVIDIA RT Core（Turing→Ada→Blackwell）卸载光线-三角形求交和 BVH 遍历。SER（线程重排序）、OMM（透明微图）、DMM（微网格位移）
- **降噪魔法**：SVGF（时域累积+空间滤波）、NRD（NVIDIA 实时降噪库）、DLSS Ray Reconstruction（Transformer 神经网络从 1-2spp 重建干净画面）
- **商业落地里程碑**：Cyberpunk 2077 RT Overdrive（首个 AAA 全路径追踪）、Portal RTX（RTX Remix 翻新老游戏）、Alan Wake 2（混合 RT 渐进迁移）

### RT 应用策略总结

| 策略 | 代表游戏 | 说明 |
|---|---|---|
| 仅 RT 阴影 | Shadow of the Tomb Raider | 最低成本 |
| RT 反射 + 阴影 | Spider-Man, Battlefield V | 最明显的视觉提升 |
| RTGI（全局光照） | Metro Exodus Enhanced | 全场景动态 GI |
| 混合 RT（多效果） | Alan Wake 2, Black Myth Wukong | 保留光栅化基础 |
| 全路径追踪 | Cyberpunk 2077 PT, Portal RTX | 完全物理正确 |

### 效果展示

![路径追踪画面对比](https://learnopengl.com/img/advanced-lighting/ssao.png)

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
- **神经纹理压缩（NTC）**：用小型 MLP 替代传统 BCn/ASTC 固定块编码——压缩率 15-20× 超越 BC7，同时质量更高
- **Photo→Material AI 管线**：输入单张照片 → 分割材质区域 → AI 估计 Albedo/Roughness/Normal/Metalness → 输出完整 PBR 材质包
- **Text-to-3D 生成**：DreamFusion→DreamGaussian→Meshy/Luma Genie 的演进路线，从小时级到分钟级

### 效果展示

![DLSS 超分辨率对比](https://learnopengl.com/img/advanced-lighting/bloom.png)

![神经纹理压缩效果](https://learnopengl.com/img/getting-started/transformations.png)

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
- **Mesh Shading**：以 Meshlet（64 顶点+84 三角形）替代三角形为最小处理单元。Amplification Shader 阶段 GPU 可动态决定工作量——视锥体剔除、LOD 选择全部在 GPU 上完成，零 CPU 参与
- **Work Graphs (DirectX 12)**：GPU 自调度计算图。传统模式 CPU 逐个 dispatch GPU 工作并等待结果→Work Graphs 让 GPU 自行管理依赖链，零 CPU-GPU 往返
- **Neural Shader**：在渲染管线中嵌入小型神经网络。Cooperative Vectors 指令让 Shader 能高效执行矩阵-向量乘法（Tensor Core 加速），网络推理替代手工 BRDF 公式、纹理采样、阴影计算

### 2030 混合管线推测
> 传统 Mesh Shader（主几何体） + Neural Shader（程序化细节） + 3DGS（远景/背景） + Path Tracing（镜面反射/折射） + Neural Material（BRDF 评估） + DLSS RR（降噪+超分） + DLSS MFG（多帧生成）

### 效果展示

![Mesh Shader 管线](https://learnopengl.com/img/getting-started/transformations.png)

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
