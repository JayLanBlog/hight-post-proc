# 18 Shader 效果清册

| # | ID | 名称 | 分类 (EN) | 分类 (CN) | 动态 | uTime 用法 |
|---|-----|------|------|------|:---:|------|
| 1 | simple_test | Grayscale Test | Color Adjustment | 色彩调整 | | 声明未用 |
| 2 | bloom | Bloom | Blur Effects | 模糊效果 | | 声明未用 |
| 3 | blur | Gaussian Blur | Blur Effects | 模糊效果 | | 声明未用 |
| 4 | sharpen | Sharpen | Image Processing | 图像处理 | | 声明未用 |
| 5 | edge_detect | Edge Detection | Edge Detection | 边缘检测 | | 声明未用 |
| 6 | emboss | Emboss | Edge Detection | 边缘检测 | | 声明未用 |
| 7 | pixelate | Pixelate | Pixelate Effects | 像素化效果 | | 声明未用 |
| 8 | vignette | Vignette | Vignette Effects | 暗角效果 | | 声明未用 |
| 9 | chromatic | Chromatic Aberration | Color Adjustment | 色彩调整 | | 声明未用 |
| 10 | color_grade | Color Grading | Color Adjustment | 色彩调整 | | 声明未用 |
| 11 | noise | Noise Generator | Glitch Effects | 故障效果 | ✓ | `uTime * speed` 驱动噪声偏移 |
| 12 | kaleidoscope | Kaleidoscope | Image Processing | 图像处理 | ✓ | `angle + uTime * speed` 旋转 |
| 13 | glitch | Glitch Art | Glitch Effects | 故障效果 | ✓ | `floor(uTime * speed)` 时序控制 |
| 14 | toon | Toon Shading | Color Adjustment | 色彩调整 | | 声明未用 |
| 15 | vhs | VHS Retro | Glitch Effects | 故障效果 | ✓ | 扫描线偏移+噪声+色彩漂移 |
| 16 | crt | CRT Monitor | Glitch Effects | 故障效果 | ✓ | `sin(uTime*60)` 闪烁计算 |
| 17 | water_ripple | Water Ripple | Image Processing | 图像处理 | ✓ | `sin(uTime*speed)` 驱动波动画 |
| 18 | lens_distort | Lens Distortion | Image Processing | 图像处理 | | 声明未用 |

## 分类与卡片数

| 分类 | 卡片 |
|------|------|
| Blur Effects | bloom, blur |
| Pixelate Effects | pixelate |
| Edge Detection | edge_detect, emboss |
| Glitch Effects | noise, glitch, vhs, crt |
| Color Adjustment | simple_test, chromatic, color_grade, toon |
| Vignette Effects | vignette |
| Image Processing | sharpen, kaleidoscope, water_ripple, lens_distort |
| Process Effects (全部) | 所有 18 张 |

## Dynamic/Static 划分

| | 效果 | 数量 |
|--|------|:---:|
| **动态** (6) | noise, kaleidoscope, glitch, vhs, crt, water_ripple | 6 |
| **静态** (12) | 其余全部 | 12 |
