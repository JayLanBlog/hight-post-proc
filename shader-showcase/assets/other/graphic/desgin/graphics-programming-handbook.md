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
