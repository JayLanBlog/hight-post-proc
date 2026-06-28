# XPL 全量效果迁移设计规格

> 日期: 2026-06-27 | 状态: 已批准

---

## 1. 目标

将 `X-PostProcessing-Library` 中剩余 56 个 HLSL 后处理效果全部迁移到 ShaderShowcase，
通过自动化脚本转换，统一为 GLSL 460 + std140 UBO Params 规范。

---

## 2. 源与目标

| 项 | 值 |
|---|-----|
| 源路径 | `e:\AI\graph\X-PostProcessing-Library\Assets\X-PostProcessing\Effects\` |
| 目标路径 | `e:\AI\graph\hight-post-proc\shader-showcase\shaders\effects\` |
| 源格式 | Unity `.shader`（HLSL + ShaderLab 包装） |
| 目标格式 | `.frag`（GLSL 460）+ `effect.json`（JSON 元数据） |

---

## 3. 效果清单（7 类 56 个）

### 3.1 Blur 模糊（17）

| XPL 目录 | ID | 中文名 | 多 Pass |
|---|---|---|---|
| BokehBlur | xpl_blur_bokeh | 散景模糊 | 否 |
| BoxBlur | xpl_blur_box | 方框模糊 | 否 |
| DirectionalBlur | xpl_blur_directional | 定向模糊 | 否 |
| DualBoxBlur | xpl_blur_dual_box | 双重方框模糊 | **是** |
| DualGaussianBlur | xpl_blur_dual_gaussian | 双重高斯模糊 | **是** |
| DualKawaseBlur | xpl_blur_dual_kawase | 双重 Kawase 模糊 | **是** |
| DualTentBlur | xpl_blur_dual_tent | 双重帐篷模糊 | **是** |
| GaussianBlur | xpl_blur_gaussian | 高斯模糊 | 否 |
| GrainyBlur | xpl_blur_grainy | 颗粒模糊 | 否 |
| IrisBlur | xpl_blur_iris | 光圈模糊 | 否 |
| IrisBlurV2 | xpl_blur_iris_v2 | 光圈模糊 V2 | 否 |
| KawaseBlur | xpl_blur_kawase | Kawase 模糊 | 否 |
| RadialBlur | xpl_blur_radial | 径向模糊 | 否 |
| RadialBlurV2 | xpl_blur_radial_v2 | 径向模糊 V2 | 否 |
| TentBlur | xpl_blur_tent | 帐篷模糊 | 否 |
| TiltShiftBlur | xpl_blur_tilt_shift | 移轴模糊 | **是** |
| TiltShiftBlurV2 | xpl_blur_tilt_shift_v2 | 移轴模糊 V2 | **是** |

### 3.2 Color Adjustment 色彩调整（11）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| ColorAdjustmentBleachBypass | xpl_color_bleach_bypass | 漂白旁路 |
| ColorAdjustmentBrightness | xpl_color_brightness | 亮度 |
| ColorAdjustmentContrast | xpl_color_contrast | 对比度 |
| ColorAdjustmentContrastV2 | xpl_color_contrast_v2 | 对比度 V2 |
| ColorAdjustmentContrastV3 | xpl_color_contrast_v3 | 对比度 V3 |
| ColorAdjustmentHue | xpl_color_hue | 色相 |
| ColorAdjustmentLensFilter | xpl_color_lens_filter | 镜头滤镜 |
| ColorAdjustmentSaturation | xpl_color_saturation | 饱和度 |
| ColorAdjustmentTechnicolor | xpl_color_technicolor | Technicolor |
| ColorAdjustmentTint | xpl_color_tint | 色调 |
| ColorAdjustmentWhiteBalance | xpl_color_white_balance | 白平衡 |

### 3.3 Color Replace 颜色替换（2）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| ColorReplace | xpl_color_replace | 颜色替换 |
| ColorReplaceV2 | xpl_color_replace_v2 | 颜色替换 V2 |

### 3.4 Edge Detection 边缘检测（9）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| EdgeDetectionRoberts | xpl_edge_roberts | Roberts 边缘 |
| EdgeDetectionRobertsNeon | xpl_edge_roberts_neon | Roberts 霓虹 |
| EdgeDetectionRobertsNeonV2 | xpl_edge_roberts_neon_v2 | Roberts 霓虹 V2 |
| EdgeDetectionScharr | xpl_edge_scharr | Scharr 边缘 |
| EdgeDetectionScharrNeon | xpl_edge_scharr_neon | Scharr 霓虹 |
| EdgeDetectionScharrNeonV2 | xpl_edge_scharr_neon_v2 | Scharr 霓虹 V2 |
| EdgeDetectionSobel | xpl_edge_sobel | Sobel 边缘 |
| EdgeDetectionSobelNeon | xpl_edge_sobel_neon | Sobel 霓虹 |
| EdgeDetectionSobelNeonV2 | xpl_edge_sobel_neon_v2 | Sobel 霓虹 V2 |

### 3.5 Pixelate 像素化（9）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| PixelizeCircle | xpl_pixel_circle | 圆形像素化 |
| PixelizeDiamond | xpl_pixel_diamond | 菱形像素化 |
| PixelizeHexagon | xpl_pixel_hexagon | 六边形像素化 |
| PixelizeHexagonGrid | xpl_pixel_hexagon_grid | 六边形网格 |
| PixelizeLeaf | xpl_pixel_leaf | 叶片像素化 |
| PixelizeLed | xpl_pixel_led | LED 像素化 |
| PixelizeQuad | xpl_pixel_quad | 方形像素化 |
| PixelizeSector | xpl_pixel_sector | 扇形像素化 |
| PixelizeTriangle | xpl_pixel_triangle | 三角形像素化 |

### 3.6 Vignette 暗角（5）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| AuroraVignette | xpl_vignette_aurora | 极光暗角 |
| RapidOldTVVignette | xpl_vignette_old_tv | 老电视暗角 |
| RapidOldTVVignetteV2 | xpl_vignette_old_tv_v2 | 老电视暗角 V2 |
| RapidVignette | xpl_vignette_rapid | 快速暗角 |
| RapidVignetteV2 | xpl_vignette_rapid_v2 | 快速暗角 V2 |

### 3.7 Sharpen 锐化（3）

| XPL 目录 | ID | 中文名 |
|---|---|---|
| SharpenV1 | xpl_sharpen_v1 | 锐化 V1 |
| SharpenV2 | xpl_sharpen_v2 | 锐化 V2 |
| SharpenV3 | xpl_sharpen_v3 | 锐化 V3 |

---

## 4. 自动化迁移脚本（migrate_xpl.py）

### 4.1 输入
- XPL `.shader` 文件（HLSL 源码）
- XPL `.cs` 文件（提取参数名/标签/范围/默认值）

### 4.2 处理步骤
1. **解析 .shader**：提取 `Frag()` 函数体 + uniform 声明
2. **解析 .cs**：提取 PostProcessEffectSettings 类参数（名称、类型、范围、默认值）
3. **生成 .frag**：HLSL→GLSL 转换 + 标准 Params UBO 模板 + 6-padding
4. **生成 effect.json**：参数映射 + 中英文名/分类
5. **写入目标目录**

### 4.3 HLSL → GLSL 转换规则

| HLSL | GLSL |
|------|------|
| `half4 Frag(VaryingsDefault i) : SV_Target` | `void main()` → `outColor` |
| `SAMPLE_TEXTURE2D(_MainTex, s, uv)` | `texture(uInputTex, uv)` |
| `uniform half _Param` | 映射到 `uParamFloatN` |
| `i.texcoord` | `vUV` |
| `_ScreenParams.xy` | `uResolution` |
| `_Time.y` | `uTime` |
| `lerp(a,b,t)` | `mix(a,b,t)` |
| `half4/float4` | `vec4` |
| `half3/float3` | `vec3` |
| `half/float` | `float` |
| `saturate(x)` | `clamp(x, 0.0, 1.0)` |
| `#pragma multi_compile _ X` | 省略（无 Unity 变体系统） |

### 4.4 生成模板

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // {label}
    // ... up to 6 params with _padN filling
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

void main() {
    vec2 uv = vUV;
    vec4 color = texture(uInputTex, uv);
    // {converted effect logic}
    outColor = color;
}
```

---

## 5. 集成步骤

1. 运行 `migrate_xpl.py` 生成 56 对 `.frag` + `effect.json`
2. 更新 `shaders/CMakeLists.txt` 添加 56 个编译条目
3. 更新 `CoverFlowScene.cpp` 添加 56 个 CARD 注册
4. 更新 `LanguageManager.cpp` CardName/CardDesc 表
5. 编译 + 自动截图验证

---

## 6. 测试

| 阶段 | 内容 | 通过条件 |
|------|------|---------|
| 编译 | cmake build Release | 0 error |
| 渲染 | AUTO_TEST_CARDS=1 截图 | 56 张 card_NN.ppm 非空 |
| 效果 | 自动像素分析（variance > 5.0）| 每个效果有可见输出 |
| 分类 | 大厅分类标签正确 | 7 个新分类出现在 Grid |
