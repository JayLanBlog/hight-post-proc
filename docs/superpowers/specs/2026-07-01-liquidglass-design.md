# LiquidGlass 启动页场景设计

## 概述

在 ShaderShowcase 中新增 **LiquidGlass 场景**，一比一复刻 `e:\AI\test\LiquidGlass` 项目的玻璃效果。作为 Gallery 中的独立卡片场景，通过 ImGui 参数面板实时调节所有参数。

**参考项目**: `e:\AI\test\LiquidGlass` (C++ OpenGL, OverEngine 自研引擎)

## 目标

- 超椭圆 SDF 裁剪形状（squircle, n=4）
- 指数折射函数驱动的背景扭曲
- 13-tap 可分离高斯模糊
- 白噪声 + 正弦角度发光
- 全部 14 个参数可通过 ImGui 实时调节
- 11 张背景纹理可切换
- 仅 Vulkan 后端（GLSL 460 → SPIR-V）

## 架构

### 场景类

新建 `LiquidGlassScene`，继承 `Scene` 基类：

```
LiquidGlassScene : public Scene
  ├── OnEnter()    — 加载 Shader/纹理，创建 RT
  ├── OnExit()     — 清理资源
  ├── OnUpdate()   — 时间推进
  ├── OnRender()   — 3 Stage 渲染管线
  ├── OnImGui()    — 参数面板 + 背景切换
  └── WantsReturn() — ESC 返回 Gallery
```

### 渲染管线（3 Stage）

```
Stage 1: 背景渲染
  bgTexture → RT_A (1920×1080)
  Shader: lg_bg.frag (纹理直通)

Stage 2: 高斯模糊
  RT_A → RT_B (水平) → RT_C (垂直), 可配置迭代次数
  Shader: lg_blur.frag (13-tap 可分离)
  每迭代: 水平pass(RT_A→RT_B) + 垂直pass(RT_B→RT_C), 下一轮 RT_C 作为输入

Stage 3: 合成
  Pass 3a: DrawFullscreenQuad(RT_A → 屏幕) v_LiquidGlass=2
  Pass 3b: DrawFullscreenQuad(RT_C → 屏幕) v_LiquidGlass=1 (squircle)
  Shader: lg_glass.frag (核心)
```

### 数据流

```
OnEnter:
  ├── 加载 11 张背景纹理 (TextureManager)
  ├── 创建 RT_A (1920×1080), RT_B, RT_C (960×540, downsample=0.5)
  ├── 加载 3 个 SPIR-V Shader
  └── 初始化 14 个默认参数

OnRender:
  ├── Stage 1: BeginRenderToTexture(RT_A) → DrawFullscreenQuad(lg_bg) → EndRenderToTexture
  ├── Stage 2: for i in 0..blurIters: 水平pass(RT_A/RT_C→RT_B) + 垂直pass(RT_B→RT_C)
  ├── Stage 3a: DrawFullscreenQuad(lg_glass, mode=2, RT_A) → 背景全屏
  └── Stage 3b: DrawFullscreenQuad(lg_glass, mode=1, RT_C) → squircle玻璃
```

## Shader 设计

### Shader 1: `lg_bg.frag` (纹理直通)

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;

void main() {
    outColor = texture(uInputTex, vUV);
}
```

用途：将背景纹理渲染到 RT_A，以及将 RT_A 全屏输出到屏幕。

### Shader 2: `lg_blur.frag` (13-tap 高斯模糊)

与 `Blur.glsl` 完全一致。13-tap 可分离高斯卷积核（σ≈1.0）：

| 偏移 | 权重 |
|------|------|
| 0 | 0.19648255 |
| ±1.4118 | 0.29690696 |
| ±3.2941 | 0.09447040 |
| ±5.1765 | 0.01038136 |

```glsl
#version 460
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140,binding=1) uniform Params {
    float P0,P1; vec2 uRes; float uTime,uFC;
};

vec4 blur13(sampler2D img, vec2 uv, vec2 res, vec2 dir) {
    vec4 c = vec4(0.0);
    vec2 o1=vec2(1.4117647)*dir, o2=vec2(3.2941176)*dir, o3=vec2(5.1764706)*dir;
    c += texture(img, uv) * 0.19648255;
    c += texture(img, uv+(o1/res)) * 0.29690696;
    c += texture(img, uv-(o1/res)) * 0.29690696;
    c += texture(img, uv+(o2/res)) * 0.09447040;
    c += texture(img, uv-(o2/res)) * 0.09447040;
    c += texture(img, uv+(o3/res)) * 0.01038136;
    c += texture(img, uv-(o3/res)) * 0.01038136;
    return c;
}
void main() {
    outColor = blur13(uInputTex, vUV, uRes, vec2(P0, P1));
}
```

- P0 = 1.0 时水平模糊，P1 = 0.0
- P0 = 0.0 时垂直模糊，P1 = 1.0
- 方向向量乘以 `u_blurRadius` 作为 uniform float 传入

### Shader 3: `lg_glass.frag` (核心)

与 `BatchRenderer2D.glsl` 的 `LiquidGlass()` 函数完全一致。关键算法：

**超椭圆 SDF**:
```
sdSuperellipse(p, n, r) = (|x|^n + |y|^n - r^n) / (n * sqrt(|x|^(2n-2) + |y|^(2n-2)) + ε)
```

**指数折射函数**:
```
f(x) = 1 - b * (c * e)^(-d * x - a)
```
- x=0（边缘）→ f→0（折射最大）
- x→∞（中心）→ f→1（无折射）

**折射偏移**:
```
sampleP = p * pow(f(dist), u_fPower)
```

**坐标变换**:
```
targetNDC = sampleP * v_QuadNDC2ScreenNDCScale + v_MidPoint.xy
coord = targetNDC * 0.5 + 0.5
```

**噪声**:
```
rand(gl_FragCoord.xy * 1e-3)
```

**发光**:
```
Glow() = sin(atan(vUV.y*2-1, vUV.x*2-1) - 0.5)
mul = Glow() * u_glowWeight * smoothstep(u_glowEdge0, u_glowEdge1, dist) + 1 + u_glowBias
```

操作模式通过 uniform P2 控制：
- P2 = 2.0：纹理直通（背景全屏）
- P2 = 1.0：LiquidGlass squircle

## 文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/app/LiquidGlassScene.h` | 新增 | 场景类声明 |
| `src/app/LiquidGlassScene.cpp` | 新增 | 场景实现 (~300行) |
| `shaders/liquid_glass/lg_bg.frag` | 新增 | 纹理直通 |
| `shaders/liquid_glass/lg_blur.frag` | 新增 | 13-tap 高斯模糊 |
| `shaders/liquid_glass/lg_glass.frag` | 新增 | LiquidGlass 核心 |
| `assets/textures/liquid_glass/` | 新增 | 11 张背景纹理 |
| `CMakeLists.txt` | 修改 | 添加源文件+Shader编译目标 |
| `src/main.cpp` | 修改 | 注册 LiquidGlass 场景卡片 |

## 参数默认值（与参考项目完全一致）

| 参数 | 默认值 | 范围 | 说明 |
|------|--------|------|------|
| u_powerFactor | 3.0 | 1.001~6.0 | 超椭圆指数 |
| u_a | 0.7 | 0.0~5.0 | 折射函数 a |
| u_b | 2.3 | 0.0~6.0 | 折射函数 b |
| u_c | 5.2 | 0.0~6.0 | 折射函数 c |
| u_d | 6.9 | 0.0~10.0 | 折射函数 d |
| u_fPower | 1.0 | -1.5~6.0 | 折射强度曲线 |
| u_noise | 0.06 | 0~0.3 | 噪声强度 |
| u_blurRadius | 2.0 | 0~10.0 | 高斯模糊半径 |
| blurIters | 1 | 0~10 | 模糊迭代次数 |
| blurDownscale | 0.5 | 0.1~1.0 | 模糊降采样因子 |
| u_glowWeight | 0.25 | -1.0~1.0 | 发光权重 |
| u_glowBias | 0.0 | -1.0~1.0 | 发光偏移 |
| u_glowEdge0 | 0.5 | -1.0~1.0 | 发光边缘阈值0 |
| u_glowEdge1 | -0.5 | -1.0~1.0 | 发光边缘阈值1 |

## 背景纹理清单

从原项目 `assets/textures/` 拷贝，共 11 张：

1. background-cubes.jpg
2. background-spring.png
3. background-summer.png
4. background-autumn.png
5. background-winter.png
6. Seasonal Landscape 1.png
7. Seasonal Landscape 2.png
8. Newspaper.png
9. Cartoon Cottage.png
10. anime.png
11. background-progress-bar.jpg

## ImGui 面板设计

```
Settings
├── Background: [下拉菜单 11选1]
├── Shape (折叠)
│   ├── Power: slider 1.001~6.0
│   ├── Width: slider 0~10
│   └── Height: slider 0~10
├── Blur & Noise (折叠)
│   ├── Blur Iters: slider 0~10
│   ├── Blur Radius: slider 0~10
│   ├── Blur Downscale: slider 0.1~1.0
│   └── Noise: slider 0~0.3
├── Refraction (折叠)
│   ├── f(x) = 1 - b(ce)^(-dx-a)
│   ├── f(x) Power: slider -1.5~6.0
│   ├── a: slider 0~5
│   ├── b: slider 0~6
│   ├── c: slider 0~6
│   └── d: slider 0~10
├── Glow (折叠)
│   ├── Glow Weight: slider -1~1
│   ├── Glow Bias: slider -1~1
│   ├── Glow Edge0: slider -1~1
│   └── Glow Edge1: slider -1~1
└── FPS 显示
```

## 与参考项目的差异

| 维度 | LiquidGlass (参考) | ShaderShowcase |
|------|-------------------|----------------|
| 引擎 | OverEngine (自研) | IRenderBackend (Vulkan) |
| Shader 语言 | GLSL 450 | GLSL 460 (SPIR-V) |
| 纹理绑定 | 32-slot 数组 | binding=0 (uInputTex) |
| 顶点输入 | 9 属性 (MidPoint/Scale) | 全屏四边形 (vUV) |
| UBO | 独立 uniform | 统一 Params UBO |
| 坐标传递 | 顶点属性 flat | 直接计算 (center=0.5, scale=1.0) |
| 交互 | WASD+鼠标+滚轮 | ImGui 参数面板 |

**关键简化**: 原项目通过顶点属性传递 `v_MidPoint` 和 `v_QuadNDC2ScreenNDCScale` 用于 squircle 定位和缩放。在 ShaderShowcase 中 squircle 固定在屏幕中央（center=0.5），缩放通过 Width/Height 参数控制，无需复杂顶点属性传递。

## 验收标准

1. 从 Gallery 点击 LiquidGlass 卡片进入场景
2. 超椭圆 squircle 正确渲染，边缘有折射效果
3. 背景模糊与参考项目一致（13-tap 高斯）
4. 噪声和发光效果叠加正确
5. 所有 14 个参数可通过 ImGui 实时调节并即时生效
6. 11 张背景可切换
7. ESC 返回 Gallery
8. 自动化截图测试通过