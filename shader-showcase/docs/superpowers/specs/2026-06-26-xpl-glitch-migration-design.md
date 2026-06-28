# XPL Glitch Effects — 迁移到 ShaderShowcase 设计规格

> 日期: 2026-06-26 | 状态: 待审批

---

## 1. 目标

将 X-PostProcessing-Library 的 **17 个 Glitch 效果**从 HLSL/Unity 百分百复刻为 GLSL，集成到 ShaderShowcase 中，不动现有 18 个效果的任何代码，命名不冲突。

---

## 2. 范围

| 项 | 详情 |
|---|------|
| **新增效果** | 17 个（见第 3 节完整清单） |
| **新增文件** | 51 个：17 × `.frag` + 17 × `.frag.spv` + 17 × `effect.json` |
| **修改文件** | `CoverFlowScene.cpp`（追加 17 个 RegisterCards 条目）、`CMakeLists.txt`（追加 SPIR-V 编译目标） |
| **不改文件** | 现有 18 个效果目录、`EffectDetailScene`、`LanguageManager`、所有 `.h` 头文件 |
| **测试图片** | 17 张高质量测试图（从 Unsplash/Pexels 下载） |

---

## 3. 效果清单

| # | 新 ID | XPL 原名 | 中文名 | HLSL复杂度 | 特殊依赖 |
|---|-------|----------|--------|:---:|------|
| 1 | `xpl_glitch_analog_noise` | GlitchAnalogNoise | 模拟噪声 | 低 | — |
| 2 | `xpl_glitch_digital_stripe` | GlitchDigitalStripe | 数字条纹 | 中 | 需 GPU 噪声纹理 |
| 3 | `xpl_glitch_image_block_v1` | GlitchImageBlock | 画面块 V1 | 中 | — |
| 4 | `xpl_glitch_image_block_v2` | GlitchImageBlockV2 | 画面块 V2 | 低 | — |
| 5 | `xpl_glitch_image_block_v3` | GlitchImageBlockV3 | 画面块 V3 | 低 | — |
| 6 | `xpl_glitch_image_block_v4` | GlitchImageBlockV4 | 画面块 V4 | 低 | — |
| 7 | `xpl_glitch_line_block` | GlitchLineBlock | 行块错位 | **高** | YUV 转换 |
| 8 | `xpl_glitch_rgb_split_v1` | GlitchRGBSplit | RGB分离 V1 | 低中 | — |
| 9 | `xpl_glitch_rgb_split_v2` | GlitchRGBSplitV2 | RGB分离 V2 | 低 | — |
| 10 | `xpl_glitch_rgb_split_v3` | GlitchRGBSplitV3 | RGB分离 V3 | 低中 | — |
| 11 | `xpl_glitch_rgb_split_v4` | GlitchRGBSplitV4 | RGB分离 V4 | 低 | — |
| 12 | `xpl_glitch_rgb_split_v5` | GlitchRGBSplitV5 | RGB分离 V5 | 中 | 需噪声纹理 |
| 13 | `xpl_glitch_scan_line_jitter` | GlitchScanLineJitter | 扫描线抖动 | 低 | — |
| 14 | `xpl_glitch_screen_jump` | GlitchScreenJump | 屏幕跳跃 | **最低** | — |
| 15 | `xpl_glitch_screen_shake` | GlitchScreenShake | 屏幕震动 | **最低** | — |
| 16 | `xpl_glitch_tile_jitter` | GlitchTileJitter | 瓦片抖动 | 低中 | — |
| 17 | `xpl_glitch_wave_jitter` | GlitchWaveJitter | 波浪抖动 | **高** | snoise |

---

## 4. HLSL → GLSL 转换规则

### 4.1 全局替换

| HLSL | GLSL |
|------|------|
| `TEXTURE2D_SAMPLER2D(_MainTex, sampler_MainTex)` | `uniform sampler2D uInputTex;` |
| `SAMPLE_TEXTURE2D(_MainTex, sampler_MainTex, uv)` | `texture(uInputTex, uv)` |
| `half` / `half2` / `half3` / `half4` | `float` / `vec2` / `vec3` / `vec4` |
| `lerp(a, b, t)` | `mix(a, b, t)` |
| `frac(x)` | `fract(x)` |
| `saturate(x)` | `clamp(x, 0.0, 1.0)` |
| `fmod(a, b)` | `mod(a, b)` |
| `step(edge, x)` | `step(edge, x)` (同) |
| `_Time.y` | `uTime` (引擎注入) |
| `_ScreenParams.xy` | `iResolution.xy` (引擎注入) |

### 4.2 特殊处理

**多 Pass 效果**：XPL 的 13 个多 pass 效果（ImageBlock/LineBlock/RGBSplit 等），**每个 pass 编译为一个独立的 SPIR-V `.frag.spv` 文件**，在 effect.json 中通过 `"passes"` 字段指定数量。

**需要噪声纹理的效果** (DigitalStripe / RGBSplitV5 / WaveJitter)：
- DigitalStripe：CPU 生成的 `_NoiseTex` → GLSL 中用 `randomNoise()` 替代（无需纹理）
- RGBSplitV5：`Resources.Load("X-Noise256")` → GLSL 中用 `randomNoise()` 替代
- WaveJitter：`XNoiseLibrary.hlsl` → 内联 2D simplex noise `snoise()` 函数

**CPU 驱动时间**：XPL 中 7 个效果用 CPU 侧 `_TimeX`（每帧累加 `Time.deltaTime`）→ GLSL 中直接用引擎注入的 `uTime`。

**#pragma shader_feature** → `#define` 预处理器宏，在编译时由 CMake 传入（或用 `const bool` + if 分支，编译器自动优化）。

### 4.3 多 Pass 对照表

| 效果 | Pass 数 | Pass 名称 |
|------|:---:|------|
| GlitchImageBlock V1/V2 | 2 | Frag (main) + Frag_Debug（合为 1 pass，debug 用 if 分支） |
| GlitchLineBlock | 2 | Frag_Horizontal + Frag_Vertical |
| GlitchRGBSplit V1/V2/V3/V4 | 3 | Frag_Horizontal + Frag_Vertical + Frag_Horizontal_Vertical |
| GlitchScanLineJitter | 2 | Frag_Horizontal + Frag_Vertical |
| GlitchScreenJump | 2 | Frag_Horizontal + Frag_Vertical |
| GlitchScreenShake | 2 | Frag_Horizontal + Frag_Vertical |
| GlitchTileJitter | 2 | Frag_Horizontal + Frag_Vertical |
| GlitchWaveJitter | 2 | Frag_Horizontal + Frag_Vertical |
| 其余 6 个 | 1 | Frag |

方向 pass（Horizontal/Vertical）通过一个 `uPassDirection` uniform (0.0=H, 1.0=V) 合并为一个 shader 文件（用 `if (uPassDirection > 0.5)` 分支），减少 SPIR-V 文件数量。

---

## 5. effect.json 规范

每个效果一个 `effect.json`，格式对齐现有 18 个：

```json
{
  "name": "Glitch Analog Noise",
  "name_cn": "模拟噪声",
  "category": "Glitch Effects",
  "category_cn": "故障效果",
  "description": "Analog static noise with luminance jitter, fading intensity control.",
  "description_cn": "模拟噪声信号干扰，亮度抖动，强度渐变控制。",
  "params": [
    {
      "name": "uNoiseSpeed",
      "label": "Noise Speed",
      "label_cn": "噪声速度",
      "type": "float",
      "min": 0.0, "max": 1.0, "default": 0.5,
      "ui": "slider"
    }
  ]
}
```

每个效果从 XPL 的 C# Settings 类直接提取参数映射到 `params[]`。

---

## 6. 文件结构

```
shaders/effects/
  xpl_analog_noise/
    xpl_analog_noise.frag
    xpl_analog_noise.frag.spv (编译产物)
    effect.json
  xpl_digital_stripe/
    ...
  ... (共 17 个目录)

shader-showcase/
  assets/images/
    xpl_glitch_01.jpg  ~  xpl_glitch_17.jpg  (测试图片)
```

---

## 7. CMake 集成

在 `shaders/CMakeLists.txt` 末尾追加 17 个编译目标，使用现有 `glslangValidator` 规则。不修改主 `CMakeLists.txt`（.frag 文件通过 `add_subdirectory(shaders)` 自动发现）。

---

## 8. CoverFlowScene.cpp 集成

在 `RegisterCards()` 末尾追加 17 个 `CARD()` 宏调用，例如：

```cpp
CARD("xpl_glitch_analog_noise", "Analog Noise", "Glitch Effects",
     "Analog static noise with luminance jitter.",
     "effects/xpl_analog_noise/xpl_analog_noise.frag.spv");
```

---

## 9. LanguageManager 处理

由于不动现有代码，XPL 效果的中/英文名称和描述直接在 `effect.json` 的 `name_cn` / `description_cn` 字段中提供，由现有的 `LoadEffectFromJson` 自动加载。无需修改 `LanguageManager`。

---

## 10. 测试图片

每张匹配效果主题：
- analog_noise → 都市夜景
- digital_stripe / scan_line → CRT 特写
- image_block v1~v4 → 科技/电路板
- rgb_split v1~v5 → 彩色几何
- screen_jump / screen_shake → 建筑/街景
- tile_jitter / wave_jitter → 纹理/图案
- line_block → 黑白高对比

来源：Unsplash/Pexels 免费商用图片，下载到 `assets/images/`，命名 `xpl_glitch_NN.jpg`。

---

## 11. 不修改的文件（完整清单）

- 现有 18 个效果目录（`shaders/effects/glitch/` 等）
- `CoverFlowScene.h`
- `EffectDetailScene.cpp` / `.h`
- `LanguageManager.cpp` / `.h`
- `SceneGalleryScene.cpp` / `.h`
- `PageManager.cpp` / `.h`
- `Application.cpp` / `.h`
- `DebugPanel.cpp` / `.h`
- `EffectMetadata.h`（结构体通用，无需改）

---

## 12. 实施顺序

1. 创建 17 个效果目录
2. 写 17 个 `.frag` GLSL 源码（HLSL→GLSL，按复杂度低→高）
3. 写 17 个 `effect.json`
4. 编译 SPIR-V（验证无语法错误）
5. `CoverFlowScene.cpp` 追加 17 个 `CARD()` + 动态索引
6. 下载 17 张测试图片
7. 编译 √ → 运行验证
