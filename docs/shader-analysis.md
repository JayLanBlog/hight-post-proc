# Shader Showcase — 9个后处理特效着色器源码详解

> 本文档对 Shader Showcase 项目中的 9 个后处理特效着色器进行深入的技术分析，涵盖效果概述、参数说明、算法原理、逐段代码分析及完整注释源码。

---

## 目录

- [公共基础设施](#公共基础设施)
  - [全屏四边形顶点着色器 fullscreen.vert](#全屏四边形顶点着色器-fullscreenvert)
  - [UBO 统一参数块布局](#ubo-统一参数块布局)
- [1. 灰度测试 (simple_test)](#1-灰度测试-simple_test)
- [2. 泛光 (bloom)](#2-泛光-bloom)
- [3. 高斯模糊 (blur)](#3-高斯模糊-blur)
- [4. 锐化 (sharpen)](#4-锐化-sharpen)
- [5. 边缘检测 (edge_detect)](#5-边缘检测-edge_detect)
- [6. 浮雕 (emboss)](#6-浮雕-emboss)
- [7. 像素化 (pixelate)](#7-像素化-pixelate)
- [8. 暗角 (vignette)](#8-暗角-vignette)
- [9. 色差 (chromatic)](#9-色差-chromatic)

---

## 公共基础设施

### 全屏四边形顶点着色器 fullscreen.vert

所有后处理特效共享同一个顶点着色器。该着色器负责渲染一个覆盖整个屏幕的四边形，为片段着色器提供插值后的 UV 坐标。

**核心思路：**

- 输入顶点坐标 `aPos` 范围为 `[-1, 1]`（NDC 空间），直接作为 `gl_Position` 输出
- UV 坐标通过 `vUV = (aPos + 1.0) * 0.5` 将 NDC 坐标映射到 `[0, 1]` 的纹理空间

```glsl
#version 460

// 通过 VAO 提供顶点输入（OpenGL 路径）
layout(location=0) in vec2 aPos;
layout(location=0) out vec2 vUV;

void main() {
    // 将 NDC 坐标 [-1,1] 映射到纹理坐标 [0,1]
    vUV = (aPos + 1.0) * 0.5;
    // 直接使用 NDC 坐标作为裁剪空间位置，z=0, w=1
    gl_Position = vec4(aPos, 0, 1);
}
```

### UBO 统一参数块布局

所有片段着色器共享同一个 Uniform Buffer Object (UBO) 布局，使用 `std140` 标准布局规则以确保 CPU/GPU 之间的内存对齐一致。

```glsl
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 特效参数 0（各特效含义不同）
    float uParamFloat1;   // 特效参数 1
    float uParamFloat2;   // 特效参数 2
    float uParamFloat3;   // 特效参数 3
    float uParamFloat4;   // 特效参数 4
    float uParamFloat5;   // 特效参数 5
    vec2  uResolution;    // 屏幕分辨率（宽, 高）
    float uTime;          // 运行时间（秒）
    float uFrameCount;    // 帧计数
};
```

**std140 布局规则要点：**
- 每个 `float` 占用 4 字节，自然对齐
- `vec2` 占用 8 字节，按 8 字节对齐
- 整个 UBO 绑定到 `binding=1`，与 `binding=0` 的输入纹理分离

---

## 1. 灰度测试 (simple_test)

### 效果概述

灰度测试是最基础的后处理特效，用于验证 Shader 管线是否正常工作。它将彩色图像转换为灰度图像，属于 **基础颜色处理** 类别。该效果通过 ITU-R BT.601 标准的亮度加权公式计算灰度值，并提供强度参数控制灰度化程度。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 灰度强度 | Float | 0.0 | 1.0 | 0.5 | 滑块 |

- **灰度强度**：控制灰度化程度。0.0 时输出全黑，1.0 时输出标准灰度，0.5 时输出半亮度灰度。

### 算法原理

灰度转换的核心是 **ITU-R BT.601 亮度公式**。人眼对绿色最敏感，对蓝色最不敏感，因此三个通道的权重不同：

$$
L = 0.299 \cdot R + 0.587 \cdot G + 0.114 \cdot B
$$

在 GLSL 中，使用 `dot()` 点积运算高效计算：

$$
\text{gray} = \text{dot}(\text{color}, \vec{3}(0.299, 0.587, 0.114)) \times \text{uParamFloat0}
$$

最终将灰度值复制到 RGB 三个通道，Alpha 设为 1.0。

### 逐段代码分析

**片段着色器声明与资源绑定：**

声明输入 UV、输出颜色、输入纹理和 UBO 参数块。这是所有后处理着色器的标准模板。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;

layout(binding=0) uniform sampler2D uInputTex;

layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

**主函数 — 灰度计算：**

采样输入纹理，使用 BT.601 权重计算灰度值，乘以强度参数后输出。

```glsl
void main() {
    // 采样输入纹理获取原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 使用 BT.601 亮度权重计算灰度值，乘以强度参数
    float gray = dot(color, vec3(0.299, 0.587, 0.114)) * uParamFloat0;
    // 将灰度值写入 RGB 三通道，Alpha 设为 1.0
    outColor = vec4(vec3(gray), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：从顶点着色器插值得到的纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理绑定到 binding=0
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块，std140 标准布局，绑定到 binding=1
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 灰度强度
    float uParamFloat1;   // 未使用
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 从输入纹理采样当前像素的颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 使用 ITU-R BT.601 标准权重进行灰度转换
    // 人眼对绿色最敏感(0.587)，红色次之(0.299)，蓝色最不敏感(0.114)
    float gray = dot(color, vec3(0.299, 0.587, 0.114)) * uParamFloat0;
    // 输出灰度图像，RGB 三通道相同，Alpha 通道为 1.0
    outColor = vec4(vec3(gray), 1.0);
}
```

---

## 2. 泛光 (bloom)

### 效果概述

泛光（Bloom）是一种模拟真实相机镜头光晕的后期处理特效，属于 **光照** 类别。它通过提取画面中亮度超过阈值的区域，对这些高亮区域进行高斯模糊处理，最后将模糊后的光晕叠加回原始图像，产生一种柔和的发光效果。常用于增强画面中光源、高光反射等明亮区域的视觉冲击力。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 泛光强度 | Float | 0.0 | 3.0 | 0.8 | 滑块 |
| `uParamFloat1` | 阈值 | Float | 0.0 | 1.0 | 0.7 | 滑块 |
| `uParamFloat2` | 模糊大小 | Float | 1.0 | 10.0 | 4.0 | 滑块 |

- **泛光强度**：控制最终叠加的泛光亮度倍率
- **阈值**：只有亮度超过此值的像素才会参与泛光计算
- **模糊大小**：控制高斯模糊核的采样范围和扩散程度

### 算法原理

泛光效果采用 **单通道实现**，核心步骤如下：

**1. 亮度阈值提取：**

对每个采样点计算其 ITU-R BT.709 亮度值：

$$
L = 0.2126 \cdot R + 0.7152 \cdot G + 0.0722 \cdot B
$$

只有 $L > \text{Threshold}$ 的像素才参与泛光，超出部分按比例缩放：

$$
\text{brightness} = \frac{L - T}{1 - T + 0.001}
$$

**2. 二维高斯模糊：**

使用二维高斯核进行卷积，权重为：

$$
w(x, y) = e^{-\frac{x^2 + y^2}{2\sigma^2}}
$$

其中 $\sigma = \text{BlurSize}$，采样范围由 `range = ceil(BlurSize)` 决定。

**3. 加法混合：**

$$
\text{final} = \text{original} + \text{bloom} \times \text{BloomIntensity}
$$

### 逐段代码分析

**参数读取与提前退出：**

从 UBO 读取三个控制参数，当泛光强度为 0 时直接输出原图，避免不必要的计算。

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float BloomIntensity = uParamFloat0;
    float Threshold = uParamFloat1;
    float BlurSize = max(uParamFloat2, 1.0);

    // 提前退出：无泛光时直接输出原图
    if (BloomIntensity <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }
```

**高斯模糊核计算：**

计算纹素大小、模糊范围和 sigma 值，准备二维高斯卷积。

```glsl
    vec2 texelSize = 1.0 / uResolution;
    
    // BlurSize 同时控制核范围和采样偏移量，产生明显的视觉效果
    int range = int(ceil(BlurSize));
    float sigma = BlurSize;
    float sigma2 = 2.0 * sigma * sigma;
```

**二维高斯卷积循环：**

遍历以当前像素为中心的方形区域，对每个采样点计算高斯权重，并根据亮度阈值过滤。

```glsl
    vec3 bloom = vec3(0.0);
    float weightSum = 0.0;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            // 计算高斯权重
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            
            // 采样偏移量乘以 BlurSize，扩大模糊范围
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurSize;
            vec3 sampleColor = texture(uInputTex, vUV + offset).rgb;
            
            // 阈值过滤：只有亮度超过阈值的像素才参与泛光
            float sampleLuma = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));
            if (sampleLuma > Threshold) {
                // 超出阈值的亮度按比例贡献
                float brightness = (sampleLuma - Threshold) / (1.0 - Threshold + 0.001);
                bloom += sampleColor * brightness * w;
            }
            weightSum += w;
        }
    }
    bloom /= max(weightSum, 0.001);
```

**最终混合输出：**

将泛光结果按强度叠加到原始图像上。

```glsl
    outColor = vec4(color + bloom * BloomIntensity, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 泛光强度
    float uParamFloat1;   // 亮度阈值
    float uParamFloat2;   // 模糊大小
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 采样当前像素的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // 读取控制参数
    float BloomIntensity = uParamFloat0;  // 泛光强度
    float Threshold = uParamFloat1;       // 亮度阈值
    float BlurSize = max(uParamFloat2, 1.0); // 模糊大小，最小为 1.0

    // 提前退出：泛光强度为 0 时直接输出原图
    if (BloomIntensity <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // 计算单个纹素的大小（用于纹理坐标偏移）
    vec2 texelSize = 1.0 / uResolution;
    
    // 模糊范围 = ceil(BlurSize)，决定卷积核的半径
    int range = int(ceil(BlurSize));
    // sigma 直接等于 BlurSize
    float sigma = BlurSize;
    // 2 * sigma^2，用于高斯公式
    float sigma2 = 2.0 * sigma * sigma;
    
    // 泛光累积值和权重总和
    vec3 bloom = vec3(0.0);
    float weightSum = 0.0;

    // 二维高斯卷积：遍历 [-range, range] 的方形区域
    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            // 计算采样点到中心距离的平方
            float dist2 = float(x * x + y * y);
            // 高斯权重：w = exp(-dist^2 / (2*sigma^2))
            float w = exp(-dist2 / sigma2);
            
            // 采样偏移 = 整数偏移 * 纹素大小 * BlurSize
            // 乘以 BlurSize 进一步扩大模糊扩散范围
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurSize;
            vec3 sampleColor = texture(uInputTex, vUV + offset).rgb;
            
            // 使用 BT.709 标准计算采样点亮度
            float sampleLuma = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));
            // 阈值过滤：只有亮度超过阈值的像素才产生泛光
            if (sampleLuma > Threshold) {
                // 超出阈值部分按比例贡献亮度
                float brightness = (sampleLuma - Threshold) / (1.0 - Threshold + 0.001);
                bloom += sampleColor * brightness * w;
            }
            weightSum += w;
        }
    }
    // 归一化泛光结果
    bloom /= max(weightSum, 0.001);

    // 将泛光按强度叠加到原始图像
    outColor = vec4(color + bloom * BloomIntensity, 1.0);
}
```

---

## 3. 高斯模糊 (blur)

### 效果概述

高斯模糊是最经典的图像模糊算法，属于 **模糊** 类别。它通过对当前像素周围的邻域进行加权平均来实现平滑效果，权重由二维高斯函数决定。距离中心越远的像素权重越低，从而在模糊的同时保留一定的图像结构。本实现采用固定 9x9 采样核，通过参数控制模糊半径和混合强度。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 模糊半径 | Float | 1.0 | 20.0 | 5.0 | 滑块 |
| `uParamFloat1` | 模糊强度 | Float | 0.0 | 1.0 | 1.0 | 滑块 |

- **模糊半径**：控制高斯核的 sigma 值和采样偏移量，值越大模糊越强
- **模糊强度**：控制原图与模糊图之间的混合比例（0 = 原图，1 = 完全模糊）

### 算法原理

**二维高斯卷积：**

高斯核的权重函数为：

$$
w(x, y) = e^{-\frac{x^2 + y^2}{2\sigma^2}}
$$

其中 $\sigma = \text{BlurRadius} \times 0.5$。

采样偏移量为：

$$
\Delta_{uv} = (x, y) \times \text{texelSize} \times \text{BlurRadius} \times 0.3
$$

最终通过 `mix()` 函数在原图和模糊结果之间线性插值：

$$
\text{output} = \text{mix}(\text{original}, \text{blurred}, \text{BlurStrength})
$$

### 逐段代码分析

**参数读取与提前退出：**

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float BlurRadius = max(uParamFloat0, 0.5); // 防止除零
    float BlurStrength = uParamFloat1;

    // 提前退出：无模糊时直接输出原图
    if (BlurStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }
```

**高斯模糊计算：**

使用固定 9x9 核（范围 -4 到 4），sigma 和采样偏移均由 BlurRadius 控制。

```glsl
    // 高斯模糊：半径控制核大小和采样扩散
    vec2 texelSize = 1.0 / uResolution;
    vec3 result = vec3(0.0);
    float total = 0.0;
    
    // sigma = 半径 * 0.5
    float sigma = BlurRadius * 0.5;
    float sigma2 = 2.0 * sigma * sigma;

    // 固定 9x9 采样核
    for (int x = -4; x <= 4; x++) {
        for (int y = -4; y <= 4; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            // 偏移量 = 整数偏移 * 纹素大小 * 半径 * 0.3
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurRadius * 0.3;
            result += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    result /= max(total, 0.001);
```

**混合输出：**

```glsl
    // 强度控制原图与模糊结果的混合
    outColor = vec4(mix(color, result, BlurStrength), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 模糊半径
    float uParamFloat1;   // 模糊强度
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 采样当前像素的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // 读取模糊参数，半径最小值 0.5 防止除零
    float BlurRadius = max(uParamFloat0, 0.5);
    float BlurStrength = uParamFloat1;

    // 提前退出：模糊强度为 0 时直接输出原图
    if (BlurStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // 计算纹素大小
    vec2 texelSize = 1.0 / uResolution;
    vec3 result = vec3(0.0);
    float total = 0.0;
    
    // sigma 与模糊半径成正比
    float sigma = BlurRadius * 0.5;
    float sigma2 = 2.0 * sigma * sigma;

    // 固定 9x9 高斯核卷积
    for (int x = -4; x <= 4; x++) {
        for (int y = -4; y <= 4; y++) {
            // 采样点到中心的距离平方
            float dist2 = float(x * x + y * y);
            // 高斯权重
            float w = exp(-dist2 / sigma2);
            // 采样偏移量，乘以半径和 0.3 系数控制扩散程度
            vec2 offset = vec2(float(x), float(y)) * texelSize * BlurRadius * 0.3;
            result += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    // 权重归一化
    result /= max(total, 0.001);

    // 在原图和模糊结果之间按强度混合
    outColor = vec4(mix(color, result, BlurStrength), 1.0);
}
```

---

## 4. 锐化 (sharpen)

### 效果概述

锐化效果用于增强图像中的边缘和细节，属于 **图像增强** 类别。本实现采用经典的 **Unsharp Mask（USM）** 算法：先对图像进行高斯模糊得到低频分量，然后用原图减去模糊图得到高频细节，最后将高频细节按强度叠加回原图。这种方法能有效增强边缘而不会引入过多噪声。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 强度 | Float | 0.0 | 3.0 | 1.0 | 滑块 |
| `uParamFloat1` | 半径 | Float | 0.5 | 3.0 | 1.0 | 滑块 |

- **强度**：控制锐化程度，值越大边缘增强越明显
- **半径**：控制用于计算模糊的采样范围，影响锐化的"粗细"

### 算法原理

**Unsharp Mask 公式：**

$$
\text{sharpened} = \text{original} + \text{Amount} \times (\text{original} - \text{blur})
$$

其中 `blur` 是对原图进行高斯模糊的结果。`original - blur` 提取了图像的高频分量（边缘和细节），乘以 `Amount` 后叠加回原图即完成锐化。

**高斯模糊核：**

与泛光效果类似，使用动态范围的高斯核：

$$
w(x, y) = e^{-\frac{x^2 + y^2}{2\sigma^2}}, \quad \sigma = \text{Radius} \times 0.5
$$

采样范围为 `[-ceil(Radius), ceil(Radius)]`，偏移量乘以 `Radius` 以控制扩散。

最终结果通过 `clamp()` 限制在 `[0, 1]` 范围内，防止过曝。

### 逐段代码分析

**参数读取与提前退出：**

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float Amount = uParamFloat0;
    float Radius = uParamFloat1;

    // 提前退出：锐化强度为 0 时直接输出原图
    if (Amount <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }
```

**高斯模糊计算（用于提取低频分量）：**

使用动态范围的高斯核，半径由参数控制。

```glsl
    // 高斯加权模糊，用于 Unsharp Mask
    vec2 texelSize = 1.0 / uResolution;
    float sigma = Radius * 0.5;
    float sigma2 = 2.0 * sigma * sigma;
    float total = 0.0;
    vec3 blur = vec3(0.0);

    // 动态范围：由 Radius 参数决定
    int range = int(ceil(Radius));
    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            float dist2 = float(x * x + y * y);
            float w = exp(-dist2 / sigma2);
            vec2 offset = vec2(float(x), float(y)) * texelSize * Radius;
            blur += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    blur /= max(total, 0.001);
```

**Unsharp Mask 锐化与输出：**

```glsl
    // Unsharp Mask：原图 + 强度 * (原图 - 模糊)
    vec3 sharpened = color + (color - blur) * Amount;

    // 限制输出范围防止过曝
    outColor = vec4(clamp(sharpened, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 锐化强度
    float uParamFloat1;   // 锐化半径
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 采样当前像素的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // 读取锐化参数
    float Amount = uParamFloat0;   // 锐化强度
    float Radius = uParamFloat1;    // 锐化半径

    // 提前退出：锐化强度为 0 时直接输出原图
    if (Amount <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // 计算纹素大小
    vec2 texelSize = 1.0 / uResolution;
    // 高斯 sigma 与半径成正比
    float sigma = Radius * 0.5;
    float sigma2 = 2.0 * sigma * sigma;
    float total = 0.0;
    vec3 blur = vec3(0.0);

    // 动态范围的高斯模糊核
    int range = int(ceil(Radius));
    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            // 距离平方
            float dist2 = float(x * x + y * y);
            // 高斯权重
            float w = exp(-dist2 / sigma2);
            // 采样偏移量乘以半径
            vec2 offset = vec2(float(x), float(y)) * texelSize * Radius;
            blur += texture(uInputTex, vUV + offset).rgb * w;
            total += w;
        }
    }
    // 归一化模糊结果
    blur /= max(total, 0.001);

    // Unsharp Mask 锐化公式
    // sharpened = original + amount * (original - blur)
    // (original - blur) 提取高频细节
    vec3 sharpened = color + (color - blur) * Amount;

    // clamp 到 [0,1] 防止颜色溢出
    outColor = vec4(clamp(sharpened, 0.0, 1.0), 1.0);
}
```

---

## 5. 边缘检测 (edge_detect)

### 效果概述

边缘检测是图像处理中的经典操作，属于 **风格化** 类别。本实现采用 **Sobel 算子**，通过计算图像在水平和垂直方向上的亮度梯度来检测边缘。检测到的边缘可以以灰度或彩色两种模式显示。该效果常用于非真实感渲染（NPR）、卡通渲染的轮廓线提取等场景。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 边缘强度 | Float | 0.0 | 5.0 | 1.5 | 滑块 |
| `uParamFloat1` | 显示颜色 | Float | 0.0 | 1.0 | 0.0 | 滑块 |

- **边缘强度**：控制边缘检测的灵敏度/亮度倍率
- **显示颜色**：0.0 时输出灰度边缘，1.0 时输出彩色边缘（用原色乘以边缘值）

### 算法原理

**Sobel 算子**使用两个 3x3 卷积核分别检测水平和垂直方向的梯度：

**水平方向核 $G_x$：**

$$
G_x = \begin{bmatrix} -1 & 0 & 1 \\ -2 & 0 & 2 \\ -1 & 0 & 1 \end{bmatrix}
$$

**垂直方向核 $G_y$：**

$$
G_y = \begin{bmatrix} -1 & -2 & -1 \\ 0 & 0 & 0 \\ 1 & 2 & 1 \end{bmatrix}
$$

对 3x3 邻域的 8 个像素（除中心像素外）分别计算亮度值，然后按 Sobel 核权重求和：

$$
S_x = -1 \cdot tl + 1 \cdot tr - 2 \cdot l + 2 \cdot r - 1 \cdot bl + 1 \cdot br
$$

$$
S_y = -1 \cdot tl - 2 \cdot t - 1 \cdot tr + 1 \cdot bl + 2 \cdot b + 1 \cdot br
$$

最终梯度幅值为：

$$
\text{edge} = \sqrt{S_x^2 + S_y^2} \times \text{EdgeStrength}
$$

### 逐段代码分析

**参数读取与提前退出：**

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    float EdgeStrength = uParamFloat0;
    float ShowColor = uParamFloat1;

    // 提前退出：边缘强度为 0 时直接输出原图
    if (EdgeStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }
```

**3x3 邻域采样与亮度计算：**

对当前像素周围的 8 个邻域像素进行采样，使用 BT.709 亮度系数将 RGB 转换为灰度值。

```glsl
    vec2 texelSize = 1.0 / uResolution;
    vec3 lumaCoeff = vec3(0.2126, 0.7152, 0.0722);

    // 采样 3x3 邻域并计算亮度
    float tl = dot(texture(uInputTex, vUV + vec2(-1.0, -1.0) * texelSize).rgb, lumaCoeff);
    float t  = dot(texture(uInputTex, vUV + vec2( 0.0, -1.0) * texelSize).rgb, lumaCoeff);
    float tr = dot(texture(uInputTex, vUV + vec2( 1.0, -1.0) * texelSize).rgb, lumaCoeff);
    float l  = dot(texture(uInputTex, vUV + vec2(-1.0,  0.0) * texelSize).rgb, lumaCoeff);
    float r  = dot(texture(uInputTex, vUV + vec2( 1.0,  0.0) * texelSize).rgb, lumaCoeff);
    float bl = dot(texture(uInputTex, vUV + vec2(-1.0,  1.0) * texelSize).rgb, lumaCoeff);
    float b  = dot(texture(uInputTex, vUV + vec2( 0.0,  1.0) * texelSize).rgb, lumaCoeff);
    float br = dot(texture(uInputTex, vUV + vec2( 1.0,  1.0) * texelSize).rgb, lumaCoeff);
```

**Sobel 梯度计算与输出：**

```glsl
    // Sobel 水平和垂直梯度
    float sx = tl * -1.0 + tr * 1.0 + l * -2.0 + r * 2.0 + bl * -1.0 + br * 1.0;
    float sy = tl * -1.0 + t * -2.0 + tr * -1.0 + bl * 1.0 + b * 2.0 + br * 1.0;

    // 梯度幅值 = sqrt(sx^2 + sy^2)
    float edge = sqrt(sx * sx + sy * sy) * EdgeStrength;

    // 在灰度边缘和彩色边缘之间混合
    vec3 edgeColor = mix(vec3(edge), color * edge, ShowColor);

    outColor = vec4(edgeColor, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 边缘强度
    float uParamFloat1;   // 显示颜色模式
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 采样当前像素的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // 读取边缘检测参数
    float EdgeStrength = uParamFloat0;  // 边缘强度
    float ShowColor = uParamFloat1;       // 颜色显示模式

    // 提前退出：边缘强度为 0 时直接输出原图
    if (EdgeStrength <= 0.0) {
        outColor = vec4(color, 1.0);
        return;
    }

    // 计算纹素大小
    vec2 texelSize = 1.0 / uResolution;
    // BT.709 亮度系数，用于将 RGB 转为灰度
    vec3 lumaCoeff = vec3(0.2126, 0.7152, 0.0722);

    // 采样 3x3 邻域的 8 个像素并计算亮度值
    // tl = 左上, t = 上, tr = 右上
    float tl = dot(texture(uInputTex, vUV + vec2(-1.0, -1.0) * texelSize).rgb, lumaCoeff);
    float t  = dot(texture(uInputTex, vUV + vec2( 0.0, -1.0) * texelSize).rgb, lumaCoeff);
    float tr = dot(texture(uInputTex, vUV + vec2( 1.0, -1.0) * texelSize).rgb, lumaCoeff);
    // l = 左, r = 右
    float l  = dot(texture(uInputTex, vUV + vec2(-1.0,  0.0) * texelSize).rgb, lumaCoeff);
    float r  = dot(texture(uInputTex, vUV + vec2( 1.0,  0.0) * texelSize).rgb, lumaCoeff);
    // bl = 左下, b = 下, br = 右下
    float bl = dot(texture(uInputTex, vUV + vec2(-1.0,  1.0) * texelSize).rgb, lumaCoeff);
    float b  = dot(texture(uInputTex, vUV + vec2( 0.0,  1.0) * texelSize).rgb, lumaCoeff);
    float br = dot(texture(uInputTex, vUV + vec2( 1.0,  1.0) * texelSize).rgb, lumaCoeff);

    // Sobel 水平梯度 Gx
    // 核: [-1 0 1; -2 0 2; -1 0 1]
    float sx = tl * -1.0 + tr * 1.0 + l * -2.0 + r * 2.0 + bl * -1.0 + br * 1.0;
    // Sobel 垂直梯度 Gy
    // 核: [-1 -2 -1; 0 0 0; 1 2 1]
    float sy = tl * -1.0 + t * -2.0 + tr * -1.0 + bl * 1.0 + b * 2.0 + br * 1.0;

    // 梯度幅值 = sqrt(Gx^2 + Gy^2)，乘以强度
    float edge = sqrt(sx * sx + sy * sy) * EdgeStrength;

    // 混合模式：ShowColor=0 时为灰度边缘，ShowColor=1 时为彩色边缘
    vec3 edgeColor = mix(vec3(edge), color * edge, ShowColor);

    outColor = vec4(edgeColor, 1.0);
}
```

---

## 6. 浮雕 (emboss)

### 效果概述

浮雕效果模拟光线从特定方向照射时产生的凹凸纹理感，属于 **风格化** 类别。它通过计算当前像素与沿指定方向偏移的邻居像素之间的差值，并将结果偏移到中间灰度（0.5），从而产生类似浮雕的立体视觉效果。方向角度可调，使浮雕的光照方向可以任意旋转。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 强度 | Float | 0.0 | 10.0 | 4.0 | 滑块 |
| `uParamFloat1` | 角度 | Float | 0.0 | 360.0 | 135.0 | 滑块 |

- **强度**：控制浮雕效果的深浅程度，值越大凹凸感越强
- **角度**：控制浮雕光照方向（0-360 度），默认 135 度为左上方光源

### 算法原理

浮雕的核心是 **方向性差分**：

1. 根据角度参数计算方向向量：

$$
\vec{d} = (\cos(\theta), \sin(\theta)) \times \text{texelSize}
$$

2. 采样当前像素和沿方向偏移一个纹素的邻居像素，计算差值：

$$
\text{diff} = (\text{neighbor} - \text{center}) \times \text{Strength} + 0.5
$$

加 0.5 的目的是将差值从 `[-0.5, 0.5]` 偏移到 `[0, 1]`，使无变化的区域呈现中灰色（0.5），而边缘区域呈现亮色或暗色，形成浮雕效果。

### 逐段代码分析

**方向向量计算：**

将角度转换为弧度，计算方向单位向量并乘以纹素大小。

```glsl
void main() {
    vec2 texelSize = 1.0 / uResolution;
    // 将角度参数从度数转换为弧度
    float rad = radians(uParamFloat1);
    // 计算方向向量并缩放到纹素大小
    vec2 dir = vec2(cos(rad), sin(rad)) * texelSize;
```

**浮雕差分计算：**

采样当前像素和偏移邻居，计算差值并偏移到中间灰度。

```glsl
    vec3 color = texture(uInputTex, vUV).rgb;
    vec3 neighbor = texture(uInputTex, vUV + dir).rgb;
    // 差值乘以强度，加 0.5 偏移到中间灰度
    vec3 diff = (neighbor - color) * uParamFloat0 + 0.5;
    outColor = vec4(clamp(diff, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 浮雕强度
    float uParamFloat1;   // 光照角度
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 计算单个纹素的大小
    vec2 texelSize = 1.0 / uResolution;
    // 将角度从度数转换为弧度
    float rad = radians(uParamFloat1);
    // 计算浮雕方向向量（单位圆上的点），缩放到纹素大小
    vec2 dir = vec2(cos(rad), sin(rad)) * texelSize;

    // 采样当前像素颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 沿方向偏移一个纹素采样邻居颜色
    vec3 neighbor = texture(uInputTex, vUV + dir).rgb;
    // 计算差值：邻居 - 中心，乘以强度，加 0.5 偏移到中间灰度
    // 正差值 -> 亮色（凸起），负差值 -> 暗色（凹陷）
    vec3 diff = (neighbor - color) * uParamFloat0 + 0.5;
    // clamp 到 [0,1] 防止溢出
    outColor = vec4(clamp(diff, 0.0, 1.0), 1.0);
}
```

---

## 7. 像素化 (pixelate)

### 效果概述

像素化（马赛克）效果将图像分割为固定大小的方块，每个方块内的所有像素显示为同一颜色，属于 **风格化** 类别。这种效果模拟了低分辨率显示器的像素网格外观，常用于复古游戏风格、隐私遮挡、以及艺术化处理等场景。本实现通过将 UV 坐标量化到方块网格上来实现。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 像素大小 | Float | 1.0 | 50.0 | 8.0 | 滑块 |

- **像素大小**：每个马赛克方块的边长（以纹素为单位），值越大像素化越明显

### 算法原理

像素化的核心是 **UV 坐标量化**：

1. 计算像素化后的网格数量：

$$
\text{pixelCount} = \frac{\text{resolution}}{\max(\text{blockSize}, 1)}
$$

2. 将连续的 UV 坐标映射到离散的网格单元，取每个单元的中心点：

$$
\text{uv}_{\text{quantized}} = \frac{\lfloor \text{uv} \times \text{pixelCount} \rfloor + 0.5}{\text{pixelCount}}
$$

`floor()` 将 UV 坐标对齐到网格的左下角，`+ 0.5` 偏移到方块中心。使用方块中心采样而非角落，可以利用 GPU 的双线性滤波避免方块边缘的颜色渗透。

### 逐段代码分析

**UV 量化与方块中心采样：**

```glsl
void main() {
    // 计算像素化后的网格数量（每个维度有多少个方块）
    vec2 pixelCount = uResolution / max(uParamFloat0, 1.0);
    // 将 UV 坐标量化到方块中心
    // floor(uv * pixelCount) 对齐到网格左下角
    // + 0.5 偏移到方块中心（配合线性滤波避免边缘渗透）
    vec2 uv = (floor(vUV * pixelCount) + 0.5) / pixelCount;
    // 使用量化后的 UV 采样纹理
    vec3 color = texture(uInputTex, uv).rgb;
    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 像素大小
    float uParamFloat1;   // 未使用
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 计算像素化后的网格数量
    // 分辨率除以方块大小，得到每个维度的方块数
    vec2 pixelCount = uResolution / max(uParamFloat0, 1.0);
    // 将连续 UV 坐标量化到方块中心
    // 步骤1: vUV * pixelCount -> 将 UV 映射到网格坐标
    // 步骤2: floor() -> 向下取整对齐到网格左下角
    // 步骤3: + 0.5 -> 偏移到方块中心（利用线性滤波避免边缘颜色渗透）
    // 步骤4: / pixelCount -> 映射回 [0,1] 的 UV 空间
    vec2 uv = (floor(vUV * pixelCount) + 0.5) / pixelCount;
    // 使用量化后的 UV 采样纹理，同一方块内的所有像素获得相同颜色
    vec3 color = texture(uInputTex, uv).rgb;
    outColor = vec4(color, 1.0);
}
```

---

## 8. 暗角 (vignette)

### 效果概述

暗角效果模拟真实相机镜头的光学衰减现象，使图像边缘逐渐变暗，属于 **颜色调整** 类别。这种效果能自然地将观者注意力引导到画面中心，同时增添电影感和专业摄影质感。本实现支持调节暗角强度、柔和度和圆度三个参数，并自动进行宽高比校正。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 暗角强度 | Float | 0.0 | 2.0 | 0.5 | 滑块 |
| `uParamFloat1` | 柔和度 | Float | 0.0 | 1.0 | 0.5 | 滑块 |
| `uParamFloat2` | 圆度 | Float | 0.0 | 1.0 | 0.5 | 滑块 |

- **暗角强度**：控制暗角的衰减程度，值越大边缘越暗
- **柔和度**：控制暗角过渡的平滑程度，值越大过渡越柔和
- **圆度**：控制暗角形状的圆度，0 为椭圆形（不校正宽高比），1 为正圆形（校正宽高比）

### 算法原理

**暗角计算步骤：**

1. 将 UV 坐标以画面中心为原点，进行宽高比校正：

$$
\vec{uv}_{\text{centered}} = \vec{uv} - 0.5, \quad uv_x \leftarrow uv_x \times \text{mix}(1, \text{aspect}, \text{roundness})
$$

2. 计算到中心的距离：

$$
d = |\vec{uv}_{\text{centered}}|
$$

3. 使用 `smoothstep` 生成平滑衰减：

$$
\text{vignette} = \text{smoothstep}(0.5 - S \times 0.5,\; 0.5 + S \times 0.2,\; d \times (1 + I))
$$

其中 $I$ 为暗角强度，$S$ 为柔和度。

4. 最终颜色：

$$
\text{output} = \text{color} \times (1 - \text{vignette})
$$

`smoothstep` 函数在两个边界之间产生平滑的 0→1 过渡，使暗角效果自然柔和。

### 逐段代码分析

**坐标中心化与宽高比校正：**

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;
    // 将 UV 坐标以画面中心为原点
    vec2 uv = vUV - 0.5;
    // 宽高比校正：圆度参数控制校正程度
    float aspect = uResolution.x / uResolution.y;
    uv.x *= mix(1.0, aspect, uParamFloat2);
```

**暗角衰减计算：**

```glsl
    // 计算到中心的欧几里得距离
    float dist = length(uv);
    // smoothstep 生成平滑衰减曲线
    // 第一个参数：开始衰减的距离（受柔和度影响）
    // 第二个参数：完全衰减的距离（受柔和度影响）
    // 第三个参数：当前像素距离（受强度缩放）
    float vignette = smoothstep(0.5 - uParamFloat1 * 0.5, 0.5 + uParamFloat1 * 0.2, dist * (1.0 + uParamFloat0));
    // 将暗角衰减乘到颜色上
    color *= 1.0 - vignette;
    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 暗角强度
    float uParamFloat1;   // 柔和度
    float uParamFloat2;   // 圆度
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 采样当前像素的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 将 UV 坐标以画面中心 (0.5, 0.5) 为原点
    vec2 uv = vUV - 0.5;
    // 宽高比校正：圆度参数控制 x 方向的缩放
    // 圆度=0 时不校正（椭圆形暗角），圆度=1 时完全校正（正圆形暗角）
    float aspect = uResolution.x / uResolution.y;
    uv.x *= mix(1.0, aspect, uParamFloat2);
    // 计算当前像素到中心的欧几里得距离
    float dist = length(uv);
    // 使用 smoothstep 生成平滑的暗角衰减
    // 参数1 (edge0): 衰减起始距离 = 0.5 - 柔和度*0.5
    // 参数2 (edge1): 衰减结束距离 = 0.5 + 柔和度*0.2
    // 参数3 (x): 当前距离 * (1 + 强度)，强度越大有效距离越大，暗角越强
    float vignette = smoothstep(0.5 - uParamFloat1 * 0.5, 0.5 + uParamFloat1 * 0.2, dist * (1.0 + uParamFloat0));
    // 将暗角衰减应用到颜色上（1 - vignette 使边缘变暗）
    color *= 1.0 - vignette;
    outColor = vec4(color, 1.0);
}
```

---

## 9. 色差 (chromatic)

### 效果概述

色差（Chromatic Aberration）模拟真实光学镜头中因色散导致的 RGB 通道错位现象，属于 **扭曲** 类别。由于不同波长的光在透镜中折射率不同，红、绿、蓝三个通道在图像边缘会产生不同程度的偏移，形成彩色边缘。本实现支持径向和线性两种色差模式，径向模式下偏移量随距画面中心的距离增大而增大，更接近真实镜头效果。

### 参数说明

| 参数名 | UI 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 控件 |
|--------|---------|------|--------|--------|--------|---------|
| `uParamFloat0` | 色差强度 | Float | 0.0 | 20.0 | 3.0 | 滑块 |
| `uParamFloat1` | 径向程度 | Float | 0.0 | 1.0 | 1.0 | 滑块 |

- **色差强度**：控制 RGB 通道偏移的总体大小
- **径向程度**：0.0 时为纯线性色差（均匀偏移），1.0 时为纯径向色差（边缘偏移大，中心无偏移）

### 算法原理

**色差的核心是 RGB 通道分离采样：**

1. 计算当前像素到画面中心的方向和距离：

$$
\vec{c} = \text{vUV} - 0.5, \quad d = |\vec{c}| \times 2, \quad \hat{d} = \text{normalize}(\vec{c} + \epsilon)
$$

2. 计算总偏移量，混合径向和线性分量：

$$
\text{offset}_{\text{base}} = \text{Intensity} \times \text{texelSize}_x
$$

$$
\text{offset}_{\text{radial}} = \text{offset}_{\text{base}} \times d \times \text{RadialFactor}
$$

$$
\text{offset}_{\text{linear}} = \text{offset}_{\text{base}} \times (1 - \text{RadialFactor})
$$

$$
\text{offset}_{\text{total}} = \text{offset}_{\text{radial}} + \text{offset}_{\text{linear}}
$$

3. 对三个通道分别在不同位置采样：

$$
R = \text{sample}(\text{vUV} + \hat{d} \times \text{offset}_{\text{total}})  \\
G = \text{sample}(\text{vUV})  \\
B = \text{sample}(\text{vUV} - \hat{d} \times \text{offset}_{\text{total}})
$$

红色通道沿径向向外偏移，蓝色通道向内偏移，绿色通道保持不变。这种不对称偏移模拟了光学色散中长波长（红）和短波长（蓝）折射率差异。

### 逐段代码分析

**方向和距离计算：**

```glsl
void main() {
    vec2 texelSize = 1.0 / uResolution;
    // 计算到画面中心的向量
    vec2 center = vUV - 0.5;
    // 归一化距离 [0, 1]（对角线方向最大为 ~0.707，乘 2 扩展范围）
    float dist = length(center) * 2.0;
    // 计算径向方向单位向量，加微小值防止中心点除零
    vec2 dir = normalize(center + 0.0001);
```

**偏移量混合计算：**

```glsl
    // 基础偏移量 = 强度 * 水平纹素大小
    float offset = uParamFloat0 * texelSize.x;
    // 径向偏移：与到中心的距离成正比
    float radialOffset = offset * dist * uParamFloat1;
    // 线性偏移：与距离无关的均匀偏移
    float linearOffset = offset * (1.0 - uParamFloat1);
    // 总偏移 = 径向 + 线性
    float totalOffset = radialOffset + linearOffset;
```

**RGB 通道分离采样：**

```glsl
    // 红色通道沿径向向外偏移（模拟长波长折射率小）
    float r = texture(uInputTex, vUV + dir * totalOffset).r;
    // 绿色通道保持原位
    float g = texture(uInputTex, vUV).g;
    // 蓝色通道沿径向向内偏移（模拟短波长折射率大）
    float b = texture(uInputTex, vUV - dir * totalOffset).b;

    outColor = vec4(r, g, b, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 输入：插值纹理坐标
layout(location=0) in vec2 vUV;
// 输出：最终像素颜色
layout(location=0) out vec4 outColor;

// 输入纹理
layout(binding=0) uniform sampler2D uInputTex;

// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;   // 色差强度
    float uParamFloat1;   // 径向程度
    float uParamFloat2;   // 未使用
    float uParamFloat3;   // 未使用
    float uParamFloat4;   // 未使用
    float uParamFloat5;   // 未使用
    vec2  uResolution;    // 屏幕分辨率
    float uTime;          // 运行时间
    float uFrameCount;    // 帧计数
};

void main() {
    // 计算纹素大小
    vec2 texelSize = 1.0 / uResolution;
    // 计算当前像素到画面中心的向量
    vec2 center = vUV - 0.5;
    // 归一化距离，乘以 2 扩展到 [0, ~1.414] 范围
    float dist = length(center) * 2.0;
    // 计算径向方向单位向量，加 0.0001 防止中心点 normalize 除零
    vec2 dir = normalize(center + 0.0001);

    // 基础偏移量 = 色差强度 * 水平纹素大小
    float offset = uParamFloat0 * texelSize.x;
    // 径向偏移分量：偏移量与到中心距离成正比
    // 距离越大（越靠近边缘），偏移越大
    float radialOffset = offset * dist * uParamFloat1;
    // 线性偏移分量：均匀偏移，与距离无关
    float linearOffset = offset * (1.0 - uParamFloat1);
    // 总偏移量 = 径向偏移 + 线性偏移
    float totalOffset = radialOffset + linearOffset;

    // RGB 通道分离采样
    // 红色通道：沿径向向外偏移（模拟长波长光折射率较小）
    float r = texture(uInputTex, vUV + dir * totalOffset).r;
    // 绿色通道：保持原位不偏移
    float g = texture(uInputTex, vUV).g;
    // 蓝色通道：沿径向向内偏移（模拟短波长光折射率较大）
    float b = texture(uInputTex, vUV - dir * totalOffset).b;

    // 组合三个通道输出
    outColor = vec4(r, g, b, 1.0);
}
```

---

> **文档说明：** 本文档基于 Shader Showcase 项目中的 9 个后处理特效着色器编写，所有着色器使用 GLSL 460 版本，共享统一的 UBO 参数块布局和全屏四边形顶点着色器。每个特效均为单通道（single-pass）实现，通过 `effect.json` 配置文件定义参数范围和 UI 控件。


## 10. 调色 (color_grade)

### 效果概述

调色着色器（Color Grading）属于 **Color（色彩校正）** 类别，是电影级后期处理中最核心的效果之一。该着色器模拟了专业色彩分级工作流中的五个关键参数：曝光（Exposure）、对比度（Contrast）、饱和度（Saturation）、色温（Color Temperature）和色调偏移（Tint）。通过这些参数的组合调整，用户可以快速为画面赋予不同的视觉风格——从冷峻的蓝调电影感，到温暖的复古胶片感，再到高对比度的戏剧化效果。色温转换采用了 Tanner Helland 提出的简化算法，将开尔文温度值映射为 RGB 颜色，并以 6500K（D65 标准白光）作为中性参考点进行相对偏移计算。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 曝光 | Float | -3.0 | 3.0 | 0.0 | slider | 控制画面整体亮度，基于 2 的幂次方调整 |
| `uParamFloat1` | 对比度 | Float | -1.0 | 1.0 | 0.0 | slider | 以 0.5 为中心点的对比度拉伸/压缩 |
| `uParamFloat2` | 饱和度 | Float | -1.0 | 1.0 | 0.0 | slider | 颜色饱和度调整，-1 为完全灰度，+1 为过度饱和 |
| `uParamFloat3` | 色温 | Float | 1000.0 | 40000.0 | 6500.0 | slider | 开尔文色温值，6500K 为标准白光 |
| `uParamFloat4` | 色调 | Float | -1.0 | 1.0 | 0.0 | slider | 绿-品红轴色调偏移 |

### 算法原理

#### 1. 曝光调整（Exposure）

曝光调整基于摄影学中的曝光值（EV）概念，采用 2 的幂次方函数：

$$C_{exposure} = C_{original} \times 2^{EV}$$

其中 $EV$ 为曝光参数值。当 $EV = 0$ 时画面不变；$EV > 0$ 时画面变亮（每增加 1 档亮度翻倍）；$EV < 0$ 时画面变暗。这种对数线性映射与人眼的亮度感知更吻合。

#### 2. 对比度调整（Contrast）

对比度以 0.5（中灰）为中心点进行线性拉伸：

$$C_{contrast} = (C_{exposure} - 0.5) \times (1 + k) + 0.5$$

其中 $k$ 为对比度参数。当 $k > 0$ 时，亮部更亮、暗部更暗，画面反差增大；当 $k < 0$ 时，画面趋向灰蒙蒙的平淡效果。

#### 3. 饱和度调整（Saturation）

饱和度调整通过在原始颜色与灰度亮度之间进行线性插值实现：

$$L = 0.2126 \cdot R + 0.7152 \cdot G + 0.0722 \cdot B$$

$$C_{sat} = \text{mix}(L, C_{contrast}, 1 + s)$$

其中 $L$ 是基于 ITU-R BT.709 标准的亮度权重，$s$ 为饱和度参数。当 $s = -1$ 时，权重变为 0，输出完全灰度图像；当 $s = 0$ 时保持原色。

#### 4. 色温转换（Color Temperature）

色温转换采用 Tanner Helland 的近似算法，将开尔文温度 $T$ 映射为 RGB 三通道值。核心思路是分段定义各通道在不同温度区间的响应曲线：

- **红色通道**：$T \leq 6600K$ 时为 1.0（全红）；$T > 6600K$ 时按幂律衰减
- **绿色通道**：在 $T \leq 6600K$ 时为对数曲线，$T > 6600K$ 时为幂律曲线
- **蓝色通道**：$T \leq 1900K$ 时为 0（无蓝）；$1900K < T < 6600K$ 时为对数曲线；$T \geq 6600K$ 时为 1.0

最终以 6500K（D65 标准白光）为中性参考点，计算相对偏移：

$$C_{temp} = C_{sat} \times \frac{RGB(T)}{RGB(6500)}$$

#### 5. 色调偏移（Tint）

色调偏移沿绿-品红轴进行简单线性偏移：

$$C_{final} = C_{temp}, \quad C_{final}.g += t \times 0.1$$

其中 $t$ 为色调参数，正值偏绿，负值偏品红。

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

该段定义了 GLSL 460 版本的着色器输入输出和统一变量块。`vUV` 为顶点着色器传递的纹理坐标，`uInputTex` 为输入纹理采样器，`Params` 统一块包含了 6 个浮点参数（其中前 5 个用于调色控制）、屏幕分辨率、时间和帧计数。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：开尔文色温转 RGB 函数

`kelvinToRGB` 函数实现了 Tanner Helland 的色温近似算法。输入为开尔文温度值，输出为归一化的 RGB 颜色。函数内部按温度区间对 R、G、B 三通道分别计算，使用 `pow`（幂函数）和 `log`（对数函数）来模拟黑体辐射的光谱分布特征。所有结果通过 `clamp` 限制在 [0, 1] 范围内。

```glsl
vec3 kelvinToRGB(float temp) {
    float t = temp / 100.0;
    vec3 color;
    // Red
    if (t <= 66.0) color.r = 1.0;
    else color.r = clamp(1.292936 * pow(t - 60.0, -0.133204), 0.0, 1.0);
    // Green
    if (t <= 66.0) color.g = clamp(0.39008 * log(t) - 0.63184, 0.0, 1.0);
    else color.g = clamp(1.12989 * pow(t - 60.0, -0.07551), 0.0, 1.0);
    // Blue
    if (t >= 66.0) color.b = 1.0;
    else if (t <= 19.0) color.b = 0.0;
    else color.b = clamp(0.54320 * log(t - 10.0) - 0.27556, 0.0, 1.0);
    return color;
}
```

#### 段落三：主函数 — 曝光、对比度、饱和度处理

主函数首先采样原始纹理颜色，然后依次进行曝光（`exp2` 对数调整）、对比度（以 0.5 为中心的线性拉伸）、饱和度（基于 BT.709 亮度权重的 mix 插值）三项处理。曝光使用 `exp2` 而非简单的乘法，是因为摄影中的曝光值本身就是以 2 为底的对数单位。

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    // Exposure
    color *= exp2(uParamFloat0);

    // Contrast (around 0.18 midpoint)
    color = (color - 0.5) * (1.0 + uParamFloat1) + 0.5;

    // Saturation
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, 1.0 + uParamFloat2);
```

#### 段落四：主函数 — 色温偏移与色调调整

色温处理通过分别计算目标温度和 6500K 中性白光的 RGB 值，然后取比值得到偏移因子，乘以当前颜色实现色温偏移。色调偏移则直接在绿色通道上叠加一个线性偏移量，模拟绿-品红轴的色彩偏移。最终结果通过 `clamp` 限制到 [0, 1] 范围。

```glsl
    // Temperature
    vec3 kelvinColor = kelvinToRGB(uParamFloat3);
    vec3 kelvinNeutral = kelvinToRGB(6500.0);
    vec3 tempTint = kelvinColor / kelvinNeutral;
    color *= tempTint;

    // Tint (green-magenta shift)
    color.g += uParamFloat4 * 0.1;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块（std140 布局）
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 曝光
    float uParamFloat1;  // 对比度
    float uParamFloat2;  // 饱和度
    float uParamFloat3;  // 色温
    float uParamFloat4;  // 色调
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

// 简化的色温转RGB (Tanner Helland算法)
// 将开尔文温度值转换为归一化的RGB颜色
vec3 kelvinToRGB(float temp) {
    float t = temp / 100.0; // 将温度缩放到合理范围
    vec3 color;
    // 红色通道：低温时为1.0，高温时按幂律衰减
    if (t <= 66.0) color.r = 1.0;
    else color.r = clamp(1.292936 * pow(t - 60.0, -0.133204), 0.0, 1.0);
    // 绿色通道：低温时为对数曲线，高温时为幂律曲线
    if (t <= 66.0) color.g = clamp(0.39008 * log(t) - 0.63184, 0.0, 1.0);
    else color.g = clamp(1.12989 * pow(t - 60.0, -0.07551), 0.0, 1.0);
    // 蓝色通道：极低温时为0，高温时为1.0，中间为对数曲线
    if (t >= 66.0) color.b = 1.0;
    else if (t <= 19.0) color.b = 0.0;
    else color.b = clamp(0.54320 * log(t - 10.0) - 0.27556, 0.0, 1.0);
    return color;
}

void main() {
    // 采样输入纹理的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // 曝光调整：使用2的幂次方函数，模拟摄影曝光值(EV)
    color *= exp2(uParamFloat0);

    // 对比度调整：以0.5为中心点进行线性拉伸
    color = (color - 0.5) * (1.0 + uParamFloat1) + 0.5;

    // 饱和度调整：基于ITU-R BT.709亮度权重计算灰度值
    // 在灰度和原色之间插值，控制饱和度
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, 1.0 + uParamFloat2);

    // 色温调整：计算目标温度的RGB，以6500K(D65标准白光)为中性参考
    vec3 kelvinColor = kelvinToRGB(uParamFloat3);
    vec3 kelvinNeutral = kelvinToRGB(6500.0);
    vec3 tempTint = kelvinColor / kelvinNeutral; // 相对偏移因子
    color *= tempTint;

    // 色调偏移：沿绿-品红轴进行线性偏移
    color.g += uParamFloat4 * 0.1;

    // 输出最终颜色，限制到[0,1]范围
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

---

## 11. 噪声生成 (noise)

### 效果概述

噪声生成着色器属于 **Procedural（程序化生成）** 类别，实现了基于分形布朗运动（FBM, Fractal Brownian Motion）的 Perlin 噪声叠加效果。该着色器通过多层（5 层）不同频率和振幅的噪声叠加，生成具有自然有机感的纹理图案，可模拟烟雾、云层、水波、大理石纹理等自然现象。噪声图案会随时间动态演变，产生流动的视觉效果。用户可控制噪声强度、纹理缩放比例和动画速度三个参数。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 噪声强度 | Float | 0.0 | 1.0 | 0.3 | slider | 噪声叠加到原图上的强度 |
| `uParamFloat1` | 缩放 | Float | 1.0 | 100.0 | 10.0 | slider | 噪声纹理的频率缩放（值越大纹理越细密） |
| `uParamFloat2` | 动画速度 | Float | 0.0 | 5.0 | 1.0 | slider | 噪声随时间流动的速度 |

### 算法原理

#### 1. 哈希函数（Hash Function）

哈希函数将二维坐标映射为伪随机值，是噪声生成的基础。该实现使用了一种基于分形运算的哈希方法：

$$h(\mathbf{p}) = \text{fract}\left(\text{fract}(\mathbf{p} \times 0.1031) + \text{dot}(\mathbf{p}', \mathbf{p}'_{yzx} + 33.33)\right)$$

其中 $\mathbf{p}' = \text{fract}(\mathbf{p}_{xyx} \times 0.1031)$，最终取小数部分得到 [0, 1) 范围的伪随机值。

#### 2. Value Noise（值噪声）

值噪声通过在网格顶点上放置伪随机值，然后在网格内部进行双线性插值生成连续的噪声场：

$$n(\mathbf{p}) = \text{bilinear}\left(h(\lfloor \mathbf{p} \rfloor), h(\lfloor \mathbf{p} \rfloor + (1,0)), h(\lfloor \mathbf{p} \rfloor + (0,1)), h(\lfloor \mathbf{p} \rfloor + (1,1)), \text{fract}(\mathbf{p})\right)$$

插值权重使用 Hermite 平滑步进函数（smoothstep）：

$$f(t) = t^2(3 - 2t)$$

该函数在网格边界处的一阶导数为零，确保噪声场的连续性。

#### 3. 分形布朗运动（FBM）

FBM 通过叠加多个不同频率（octave）的噪声来生成具有多尺度细节的复杂纹理：

$$\text{FBM}(\mathbf{p}) = \sum_{i=0}^{N-1} a_i \cdot n(2^i \cdot \mathbf{p}), \quad a_i = 0.5^i$$

其中 $N = 5$ 为叠加层数，每层频率翻倍（$2^i$），振幅减半（$0.5^i$）。这种 1/f 噪声特性使得生成的图案在不同尺度上都具有自然的细节变化。

#### 4. 噪声叠加到图像

最终将 FBM 噪声值（范围约 [0, 1]）减去 0.5 映射到 [-0.5, 0.5]，再乘以强度参数叠加到原图颜色上：

$$C_{final} = C_{original} + (\text{FBM}(\mathbf{p}) - 0.5) \times \text{strength}$$

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

与调色着色器相同的标准声明结构。包含输入纹理坐标、输出颜色、纹理采样器和参数块。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：哈希函数与值噪声

`hash` 函数基于分形运算生成伪随机数，输入二维坐标，输出 [0, 1) 范围的随机值。`noise` 函数实现值噪声：先获取网格四个顶点的哈希值，然后使用 smoothstep 插值权重进行双线性插值，生成连续平滑的噪声场。

```glsl
// Hash functions for noise
float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise(vec2 p) {
    vec2 i = floor(p);       // 网格整数坐标
    vec2 f = fract(p);       // 网格内小数坐标
    f = f * f * (3.0 - 2.0 * f); // smoothstep 插值权重

    float a = hash(i);                // 左下角
    float b = hash(i + vec2(1.0, 0.0)); // 右下角
    float c = hash(i + vec2(0.0, 1.0)); // 左上角
    float d = hash(i + vec2(1.0, 1.0)); // 右上角

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y); // 双线性插值
}
```

#### 段落三：分形布朗运动（FBM）

FBM 函数通过 5 次循环叠加噪声，每次将坐标频率翻倍（`p *= 2.0`），振幅减半（`amplitude *= 0.5`）。这种多尺度叠加方式产生了从大尺度结构到小尺度细节的丰富纹理。

```glsl
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(p); // 叠加当前层噪声
        p *= 2.0;                      // 频率翻倍
        amplitude *= 0.5;              // 振幅减半
    }
    return value;
}
```

#### 段落四：主函数 — 噪声采样与叠加

主函数将纹理坐标乘以缩放参数并加上时间偏移（实现动画），然后计算 FBM 噪声值，将其映射到 [-0.5, 0.5] 后乘以强度叠加到原图颜色上。

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;
    vec2 uv = vUV * uParamFloat1 + uTime * uParamFloat2; // 缩放+时间偏移
    float n = fbm(uv);                                   // 计算FBM噪声
    color += (n - 0.5) * uParamFloat0;                    // 叠加噪声
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 噪声强度
    float uParamFloat1;  // 缩放
    float uParamFloat2;  // 动画速度
    float uParamFloat3;  // 保留参数
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

// 哈希函数：将二维坐标映射为伪随机值
// 使用分形运算产生良好的伪随机分布
float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031); // 坐标缩放并取小数部分
    p3 += dot(p3, p3.yzx + 33.33);          // 混合各分量增加随机性
    return fract((p3.x + p3.y) * p3.z);     // 最终取小数部分
}

// 值噪声函数：在网格顶点间进行平滑插值
float noise(vec2 p) {
    vec2 i = floor(p);       // 获取网格整数坐标（左下角顶点）
    vec2 f = fract(p);       // 获取网格内小数坐标（插值权重）
    f = f * f * (3.0 - 2.0 * f); // Hermite smoothstep 平滑插值

    // 获取网格四个顶点的伪随机值
    float a = hash(i);                // 左下角
    float b = hash(i + vec2(1.0, 0.0)); // 右下角
    float c = hash(i + vec2(0.0, 1.0)); // 左上角
    float d = hash(i + vec2(1.0, 1.0)); // 右上角

    // 双线性插值：先水平插值，再垂直插值
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// 分形布朗运动（FBM）：叠加多层不同频率的噪声
float fbm(vec2 p) {
    float value = 0.0;      // 累积噪声值
    float amplitude = 0.5;  // 初始振幅
    for (int i = 0; i < 5; i++) { // 5层叠加
        value += amplitude * noise(p); // 叠加当前层噪声
        p *= 2.0;                      // 频率翻倍（更细密的纹理）
        amplitude *= 0.5;              // 振幅减半（更小的影响）
    }
    return value;
}

void main() {
    // 采样输入纹理的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 将纹理坐标缩放并加上时间偏移，实现动态噪声
    vec2 uv = vUV * uParamFloat1 + uTime * uParamFloat2;
    // 计算FBM噪声值
    float n = fbm(uv);
    // 将噪声从[0,1]映射到[-0.5,0.5]，乘以强度后叠加到原图
    color += (n - 0.5) * uParamFloat0;
    // 输出最终颜色，限制到[0,1]范围
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

---

## 12. 万花筒 (kaleidoscope)

### 效果概述

万花筒着色器属于 **Distort（扭曲变形）** 类别，模拟了光学万花筒的径向对称效果。该着色器将画面从中心向外划分为若干等角扇形区域，每个扇形内的图像通过镜像翻转与其他扇形共享，从而产生类似万花筒的对称图案。同时支持自动旋转和缩放功能，可创建动态变化的几何对称效果。这种效果常用于音乐可视化、艺术风格化视频和创意后期处理中。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 对称段数 | Float | 2.0 | 12.0 | 6.0 | slider | 万花筒的对称扇形数量 |
| `uParamFloat1` | 旋转速度 | Float | 0.0 | 2.0 | 0.3 | slider | 自动旋转的角速度 |
| `uParamFloat2` | 缩放 | Float | 0.5 | 3.0 | 1.0 | slider | 画面缩放比例 |

### 算法原理

#### 1. 极坐标转换

首先将屏幕空间的纹理坐标从笛卡尔坐标转换为极坐标：

$$r = \|\mathbf{uv}\| = \sqrt{x^2 + y^2}$$

$$\theta = \text{atan2}(y, x)$$

其中 $\mathbf{uv}$ 是以画面中心为原点的归一化坐标。

#### 2. 角度折叠与镜像

将极角 $\theta$ 折叠到一个扇形区域内，实现对称效果：

$$\theta_{seg} = \frac{2\pi}{N}$$

$$\theta' = \text{mod}(\theta + \omega \cdot t, \theta_{seg})$$

$$\theta'' = \begin{cases} \theta' & \text{if } \theta' \leq \theta_{seg}/2 \\ \theta_{seg} - \theta' & \text{if } \theta' > \theta_{seg}/2 \end{cases}$$

其中 $N$ 为对称段数，$\omega$ 为旋转速度，$t$ 为时间。`mod` 操作将任意角度映射到单个扇形内，条件判断实现扇形内的镜像翻转，使得相邻扇形互为镜像。

#### 3. 坐标重建

将折叠后的极坐标转回笛卡尔坐标：

$$\mathbf{uv}_{new} = (\cos\theta'' \cdot r \cdot 0.5 + 0.5, \sin\theta'' \cdot r \cdot 0.5 + 0.5)$$

乘以 0.5 并偏移 0.5 是将 [-1, 1] 范围映射回 [0, 1] 的纹理坐标空间。

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

标准声明结构，定义了输入输出和参数块。万花筒效果不需要帧计数参数，但保留了统一的参数块结构。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：极坐标转换与角度折叠

将纹理坐标以画面中心为原点进行归一化（除以缩放参数），然后转换为极坐标。通过 `mod` 运算将角度折叠到单个扇形内，再通过条件判断实现镜像翻转，使得每个扇形内的图案互为镜像。

```glsl
#define PI 3.14159265359

void main() {
    vec2 uv = (vUV - 0.5) * 2.0 / uParamFloat2; // 以中心为原点归一化
    float angle = atan(uv.y, uv.x);                // 极角
    float radius = length(uv);                      // 极径

    // 将角度映射到 [0, 2PI/segments]
    float segAngle = PI * 2.0 / uParamFloat0;      // 单个扇形的角度范围
    angle = mod(angle + uTime * uParamFloat1, segAngle); // 折叠+旋转
    if (angle > segAngle * 0.5) angle = segAngle - angle;  // 镜像翻转
```

#### 段落三：坐标重建与纹理采样

将折叠后的极坐标转回笛卡尔坐标，映射回 [0, 1] 纹理空间，使用 `clamp` 防止越界采样，最后从输入纹理采样输出颜色。

```glsl
    // 重建UV (镜像)
    vec2 newUV = vec2(cos(angle), sin(angle)) * radius * 0.5 + 0.5;
    newUV = clamp(newUV, 0.0, 1.0);

    vec3 color = texture(uInputTex, newUV).rgb;
    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 对称段数
    float uParamFloat1;  // 旋转速度
    float uParamFloat2;  // 缩放
    float uParamFloat3;  // 保留参数
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

#define PI 3.14159265359 // 圆周率常量

void main() {
    // 将纹理坐标以画面中心为原点进行归一化，并应用缩放
    vec2 uv = (vUV - 0.5) * 2.0 / uParamFloat2;
    // 计算极坐标：角度和半径
    float angle = atan(uv.y, uv.x); // 极角（弧度）
    float radius = length(uv);       // 极径

    // 计算单个扇形的角度范围
    float segAngle = PI * 2.0 / uParamFloat0;
    // 将角度折叠到单个扇形内，并加上时间偏移实现旋转
    angle = mod(angle + uTime * uParamFloat1, segAngle);
    // 镜像翻转：超过扇形一半时取反，产生对称效果
    if (angle > segAngle * 0.5) angle = segAngle - angle;

    // 将折叠后的极坐标重建为笛卡尔坐标
    // 乘以0.5并偏移0.5，将[-1,1]映射回[0,1]纹理空间
    vec2 newUV = vec2(cos(angle), sin(angle)) * radius * 0.5 + 0.5;
    // 限制UV范围，防止越界采样
    newUV = clamp(newUV, 0.0, 1.0);

    // 使用重建的UV坐标采样输入纹理
    vec3 color = texture(uInputTex, newUV).rgb;
    outColor = vec4(color, 1.0);
}
```

---

## 13. 故障艺术 (glitch)

### 效果概述

故障艺术着色器属于 **Stylize（风格化）** 类别，模拟了数字信号故障产生的视觉失真效果。该着色器综合了三种经典的故障表现形态：扫描线撕裂（Scanline Tearing）、RGB 通道分离（Chromatic Aberration）和随机色块污染（Block Artifacts）。这三种效果以随机概率触发，故障强度参数控制各效果的发生概率和幅度。该效果广泛应用于赛博朋克风格、音乐视频、科幻电影和数字艺术作品中。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 故障强度 | Float | 0.0 | 1.0 | 0.5 | slider | 控制所有故障效果的发生概率和幅度 |
| `uParamFloat1` | 速度 | Float | 0.0 | 10.0 | 3.0 | slider | 故障效果的切换频率 |
| `uParamFloat2` | 块大小 | Float | 1.0 | 50.0 | 10.0 | slider | 扫描线撕裂和色块的基本单元大小（像素） |

### 算法原理

#### 1. 伪随机数生成

使用经典的 GLSL 伪随机数生成公式：

$$\text{random}(\mathbf{st}) = \text{fract}(\sin(\text{dot}(\mathbf{st}, (12.9898, 78.233))) \times 43758.5453)$$

该公式通过正弦函数的大数乘法和取小数运算，将二维坐标映射为伪随机值。

#### 2. 扫描线撕裂（Scanline Tearing）

以时间帧为单位，随机选择某些水平扫描线进行水平偏移：

$$P(\text{tear}) = \text{step}(0.95 - g \times 0.5, \text{random}(y_{line}, t))$$

$$\text{offset} = (\text{random}(t, y_{50}) - 0.5) \times g \times 0.3$$

其中 $g$ 为故障强度，$t$ 为离散化时间，$y_{line}$ 为像素所在的扫描线编号。当随机值超过阈值时，该扫描线上的所有像素在水平方向发生偏移。

#### 3. RGB 通道分离（Chromatic Aberration）

以较低概率触发 RGB 三通道的水平偏移，模拟色差效果：

$$P(\text{split}) = \text{step}(0.98 - g \times 0.3, \text{random}(t, 1.0))$$

$$\Delta x = P(\text{split}) \times g \times 0.05$$

红色通道向右偏移 $\Delta x$，蓝色通道向左偏移 $\Delta x$，绿色通道保持不变。

#### 4. 随机色块（Block Artifacts）

将画面划分为网格块，随机选择某些块替换为纯色噪声：

$$P(\text{block}) = \text{step}(0.99 - g \times 0.2, \text{random}(x_{block}, y_{block} + t))$$

被选中的块颜色为基于时间和垂直位置的随机值。

### 逐段代码分析

#### 段落一：着色器声明与伪随机函数

标准声明结构加上经典的 GLSL 伪随机数生成函数。该随机函数使用正弦函数的点积输入和大数乘法，产生视觉上足够随机的值。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

float random(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}
```

#### 段落二：主函数 — 扫描线撕裂

主函数首先对时间进行离散化（`floor`），使故障效果以帧为单位跳变而非连续变化。然后通过随机概率决定每条扫描线是否发生水平偏移，偏移量也由随机函数决定。

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;
    float t = floor(uTime * uParamFloat1); // 离散化时间

    // 随机行偏移 (扫描线撕裂)
    float lineNoise = step(0.95 - uParamFloat0 * 0.5, random(vec2(floor(vUV.y * uResolution.y / uParamFloat2), t)));
    float offset = (random(vec2(t, floor(vUV.y * 50.0))) - 0.5) * uParamFloat0 * 0.3;
    color = texture(uInputTex, vUV + vec2(offset, 0.0)).rgb;
```

#### 段落三：RGB 通道分离与随机色块

RGB 通道分离以较低概率触发，红蓝通道向相反方向偏移。随机色块将画面划分为网格，随机选择某些块替换为纯色噪声值。

```glsl
    // RGB 通道分离
    float channelShift = step(0.98 - uParamFloat0 * 0.3, random(vec2(t, 1.0))) * uParamFloat0 * 0.05;
    color.r = texture(uInputTex, vUV + vec2(channelShift, 0.0)).r;
    color.b = texture(uInputTex, vUV - vec2(channelShift, 0.0)).b;

    // 随机色块
    float blockNoise = step(0.99 - uParamFloat0 * 0.2, random(vec2(floor(vUV.x * uResolution.x / uParamFloat2), floor(vUV.y * uResolution.y / uParamFloat2) + t)));
    color = mix(color, vec3(random(vec2(t, vUV.y * 100.0))), blockNoise);

    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 故障强度
    float uParamFloat1;  // 速度
    float uParamFloat2;  // 块大小
    float uParamFloat3;  // 保留参数
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

// 经典GLSL伪随机数生成函数
// 通过正弦函数的大数乘法和取小数运算产生伪随机值
float random(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    // 采样原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;
    // 将时间离散化为帧索引，使故障效果以帧为单位跳变
    float t = floor(uTime * uParamFloat1);

    // === 扫描线撕裂 ===
    // 以块大小为单位划分扫描线，随机决定是否发生偏移
    float lineNoise = step(0.95 - uParamFloat0 * 0.5, random(vec2(floor(vUV.y * uResolution.y / uParamFloat2), t)));
    // 偏移量由随机函数决定，方向和大小都随机
    float offset = (random(vec2(t, floor(vUV.y * 50.0))) - 0.5) * uParamFloat0 * 0.3;
    // 使用偏移后的UV重新采样
    color = texture(uInputTex, vUV + vec2(offset, 0.0)).rgb;

    // === RGB通道分离 ===
    // 以较低概率触发色差效果
    float channelShift = step(0.98 - uParamFloat0 * 0.3, random(vec2(t, 1.0))) * uParamFloat0 * 0.05;
    // 红色通道向右偏移，蓝色通道向左偏移
    color.r = texture(uInputTex, vUV + vec2(channelShift, 0.0)).r;
    color.b = texture(uInputTex, vUV - vec2(channelShift, 0.0)).b;

    // === 随机色块 ===
    // 将画面划分为网格块，随机选择某些块替换为纯色噪声
    float blockNoise = step(0.99 - uParamFloat0 * 0.2, random(vec2(floor(vUV.x * uResolution.x / uParamFloat2), floor(vUV.y * uResolution.y / uParamFloat2) + t)));
    // 将选中块的颜色替换为随机纯色
    color = mix(color, vec3(random(vec2(t, vUV.y * 100.0))), blockNoise);

    outColor = vec4(color, 1.0);
}
```

---

## 14. 卡通着色 (toon)

### 效果概述

卡通着色着色器属于 **Stylize（风格化）** 类别，实现了经典的卡通渲染（Cel Shading / Toon Shading）后处理效果。该着色器通过两个核心步骤实现卡通风格化：颜色量化（Color Posterization）将连续的色彩梯度减少为有限的色阶数，产生色块分明的卡通效果；边缘检测（Edge Detection）在量化色块的边界处绘制黑色描边轮廓线。这种效果广泛应用于动画风格化、游戏渲染和非真实感渲染（NPR）领域。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 色阶数 | Float | 2.0 | 16.0 | 5.0 | slider | 颜色量化后的色阶数量（值越小越卡通） |
| `uParamFloat1` | 边缘阈值 | Float | 0.0 | 0.5 | 0.1 | slider | 描边的可见强度 |
| `uParamFloat2` | 描边宽度 | Float | 0.5 | 4.0 | 1.0 | slider | 边缘检测的采样距离（像素） |

### 算法原理

#### 1. 颜色量化（Posterization）

颜色量化将连续的颜色值映射到有限数量的离散色阶：

$$C_{quantized} = \left\lfloor \frac{C \times N}{N - 1} \right\rfloor \times \frac{1}{N - 1}$$

其中 $N$ 为色阶数。例如当 $N = 5$ 时，原来 [0, 1] 范围的连续值被映射为 5 个离散值：0, 0.25, 0.5, 0.75, 1.0。色阶数越少，色块效果越明显，卡通感越强。

#### 2. 边缘检测（Edge Detection）

边缘检测基于量化后颜色的邻域比较。对于当前像素，分别检查其右侧和下方邻居的量化颜色是否与当前像素不同：

$$\text{isEdge} = \begin{cases} 1 & \text{if } C_{quantized} \neq C_{right} \text{ or } C_{quantized} \neq C_{down} \\ 0 & \text{otherwise} \end{cases}$$

采样步长由描边宽度参数控制：

$$\Delta x = \frac{w}{\text{resolution}_x}, \quad \Delta y = \frac{w}{\text{resolution}_y}$$

其中 $w$ 为描边宽度参数。

#### 3. 描边渲染

检测到边缘的像素颜色被大幅压暗：

$$C_{final} = \text{mix}(C_{quantized}, C_{quantized} \times 0.15, \text{isEdge} \times \text{outline})$$

其中 `outline` 由边缘阈值参数控制（`clamp(uParamFloat1 * 4.0, 0.0, 1.0)`），0.15 的系数使边缘像素变为接近黑色的深色。

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

标准声明结构。卡通着色效果需要屏幕分辨率来计算边缘检测的采样步长。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：颜色量化

对原始颜色进行量化处理，将连续颜色值映射到有限色阶。使用 `floor` 函数实现向下取整，`max(levels - 1.0, 1.0)` 防止除零错误。

```glsl
void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    // 颜色量化
    float levels = max(uParamFloat0, 2.0); // 至少2个色阶
    vec3 quantized = floor(color * levels) / max(levels - 1.0, 1.0);
    quantized = clamp(quantized, 0.0, 1.0);
```

#### 段落三：边缘检测与描边渲染

计算采样步长，分别获取右侧和下方邻居的量化颜色，通过 `notEqual` 比较检测边缘。边缘像素颜色被压暗到原来的 15%，产生黑色描边效果。

```glsl
    // 边缘检测 — 量化色块边界
    vec2 ts = 1.0 / uResolution * uParamFloat2; // 采样步长
    vec3 nr = clamp(floor(texture(uInputTex, vUV + vec2( 1, 0) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    vec3 nd = clamp(floor(texture(uInputTex, vUV + vec2( 0, 1) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    float isEdge = any(notEqual(quantized, nr)) || any(notEqual(quantized, nd)) ? 1.0 : 0.0;

    // 边缘变暗
    float outline = clamp(uParamFloat1 * 4.0, 0.0, 1.0);
    color = mix(quantized, quantized * 0.15, isEdge * outline);

    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 色阶数
    float uParamFloat1;  // 边缘阈值
    float uParamFloat2;  // 描边宽度
    float uParamFloat3;  // 保留参数
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

void main() {
    // 采样输入纹理的原始颜色
    vec3 color = texture(uInputTex, vUV).rgb;

    // === 颜色量化（Posterization）===
    // 将连续颜色映射到有限色阶，产生色块分明的卡通效果
    float levels = max(uParamFloat0, 2.0); // 色阶数，至少为2
    vec3 quantized = floor(color * levels) / max(levels - 1.0, 1.0); // 量化
    quantized = clamp(quantized, 0.0, 1.0); // 限制范围

    // === 边缘检测 ===
    // 计算采样步长（基于描边宽度和屏幕分辨率）
    vec2 ts = 1.0 / uResolution * uParamFloat2;
    // 获取右侧邻居的量化颜色
    vec3 nr = clamp(floor(texture(uInputTex, vUV + vec2( 1, 0) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    // 获取下方邻居的量化颜色
    vec3 nd = clamp(floor(texture(uInputTex, vUV + vec2( 0, 1) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    // 比较当前像素与邻居的量化颜色，任一通道不同即为边缘
    float isEdge = any(notEqual(quantized, nr)) || any(notEqual(quantized, nd)) ? 1.0 : 0.0;

    // === 描边渲染 ===
    // 将边缘像素颜色压暗到15%，产生黑色描边效果
    float outline = clamp(uParamFloat1 * 4.0, 0.0, 1.0); // 描边强度
    color = mix(quantized, quantized * 0.15, isEdge * outline);

    outColor = vec4(color, 1.0);
}
```

---

## 15. VHS 复古 (vhs)

### 效果概述

VHS 复古着色器属于 **Retro（复古怀旧）** 类别，模拟了模拟录像带（VHS）的典型视觉特征。该着色器综合了四种 VHS 特征效果：水平跟踪失真（Tracking Distortion）模拟录像带播放时偶尔出现的水平跳动；色彩漂移（Color Drift）模拟磁头对齐不良导致的 Y/C 信号延迟；扫描线（Scanlines）模拟 CRT 电视的行扫描结构；随机噪声（Noise）模拟磁带信号劣化产生的雪花点。此外还叠加了轻微的色彩偏移（偏蓝/偏红），还原 VHS 录像的独特色彩质感。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 扫描线强度 | Float | 0.0 | 1.0 | 0.3 | slider | 扫描线暗化程度 |
| `uParamFloat1` | 噪声量 | Float | 0.0 | 0.5 | 0.1 | slider | 随机雪花噪声强度 |
| `uParamFloat2` | 色彩漂移 | Float | 0.0 | 0.02 | 0.005 | slider | RGB 通道水平偏移量 |
| `uParamFloat3` | 跟踪失真 | Float | 0.0 | 0.1 | 0.02 | slider | 水平跟踪失真的发生概率 |

### 算法原理

#### 1. 水平跟踪失真（Tracking Distortion）

VHS 录像带播放时，磁头跟踪不良会导致整帧画面水平跳动。该效果通过随机概率触发：

$$P(\text{tracking}) = \text{step}(0.98 - g, \text{random}(\lfloor t \times 10 \rfloor, 0))$$

$$\Delta x = P(\text{tracking}) \times (\text{random}(t, 1) - 0.5) \times 0.1$$

其中 $g$ 为跟踪失真参数，$t$ 为时间。当触发时，整帧画面在水平方向发生随机偏移。

#### 2. 色彩漂移（Color Drift / Chroma Delay）

VHS 信号的亮度（Y）和色度（C）分量通过不同时间处理，磁头对齐不良会导致色度信号延迟。该效果通过正弦波驱动的 RGB 通道水平偏移模拟：

$$\Delta x(y) = \sin(y \times \frac{H}{2} + t \times 2) \times d$$

其中 $H$ 为屏幕高度（像素），$d$ 为漂移参数。红色通道向右偏移 $\Delta x$，蓝色通道向左偏移 $\Delta x$，绿色通道保持不变。

#### 3. 扫描线（Scanlines）

CRT 电视的电子束逐行扫描，行间有微小间隙导致亮度变化：

$$\text{scanline}(y) = \sin(y \times H \times \pi) \times 0.5 + 0.5$$

$$C_{scan} = C \times (1 - \text{scanline} \times s)$$

其中 $s$ 为扫描线强度参数。

#### 4. 随机噪声（Noise）

模拟磁带信号劣化产生的雪花点：

$$C_{noise} = C_{scan} + (\text{random}(\text{uv} \times \text{res} + t \times 100) - 0.5) \times n$$

其中 $n$ 为噪声量参数。

#### 5. VHS 色彩偏移

VHS 录像通常带有轻微的色彩偏差，通过简单的通道乘法实现：

$$C_{final}.r = C_{noise}.r \times 1.05, \quad C_{final}.b = C_{noise}.b \times 0.95$$

### 逐段代码分析

#### 段落一：着色器声明与伪随机函数

标准声明结构加上伪随机函数。VHS 效果中多处需要随机值来驱动故障和噪声。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

float random(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}
```

#### 段落二：跟踪失真与色彩漂移

跟踪失真以低概率触发整帧水平偏移，模拟 VHS 磁头跟踪不良。色彩漂移通过正弦波驱动 RGB 通道的水平偏移，偏移量随垂直位置变化，模拟磁头对齐偏差。

```glsl
void main() {
    // 水平跟踪失真
    float trackingLine = step(0.98 - uParamFloat3, random(vec2(floor(uTime * 10.0), 0.0)));
    float trackingOffset = trackingLine * (random(vec2(uTime, 1.0)) - 0.5) * 0.1;
    vec2 uv = vUV + vec2(trackingOffset, 0.0);

    // 色彩漂移 (Y通道延迟)
    float drift = sin(uv.y * uResolution.y * 0.5 + uTime * 2.0) * uParamFloat2;
    float r = texture(uInputTex, uv + vec2(drift, 0.0)).r;
    float g = texture(uInputTex, uv).g;
    float b = texture(uInputTex, uv - vec2(drift, 0.0)).b;
    vec3 color = vec3(r, g, b);
```

#### 段落三：扫描线、噪声与色彩偏移

扫描线通过正弦函数产生周期性明暗变化。噪声通过随机函数叠加雪花点。最后进行轻微的色彩偏移（红色增强、蓝色减弱），模拟 VHS 录像的独特色彩特征。

```glsl
    // 扫描线
    float scanline = sin(uv.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    color *= 1.0 - scanline * uParamFloat0;

    // 噪声
    float noise = random(vec2(uv * uResolution + uTime * 100.0));
    color += (noise - 0.5) * uParamFloat1;

    // VHS 色彩偏移 (轻微偏蓝/偏红)
    color.r *= 1.05;
    color.b *= 0.95;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 扫描线强度
    float uParamFloat1;  // 噪声量
    float uParamFloat2;  // 色彩漂移
    float uParamFloat3;  // 跟踪失真
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

// 伪随机数生成函数
float random(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    // === 水平跟踪失真 ===
    // 以低概率触发整帧水平偏移，模拟VHS磁头跟踪不良
    float trackingLine = step(0.98 - uParamFloat3, random(vec2(floor(uTime * 10.0), 0.0)));
    float trackingOffset = trackingLine * (random(vec2(uTime, 1.0)) - 0.5) * 0.1;
    vec2 uv = vUV + vec2(trackingOffset, 0.0);

    // === 色彩漂移 (Y/C延迟) ===
    // 通过正弦波驱动RGB通道的水平偏移，模拟磁头对齐偏差
    float drift = sin(uv.y * uResolution.y * 0.5 + uTime * 2.0) * uParamFloat2;
    float r = texture(uInputTex, uv + vec2(drift, 0.0)).r;  // 红色通道右偏
    float g = texture(uInputTex, uv).g;                      // 绿色通道不变
    float b = texture(uInputTex, uv - vec2(drift, 0.0)).b;  // 蓝色通道左偏
    vec3 color = vec3(r, g, b);

    // === 扫描线 ===
    // 通过正弦函数产生周期性明暗变化，模拟CRT行扫描结构
    float scanline = sin(uv.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    color *= 1.0 - scanline * uParamFloat0;

    // === 随机噪声 ===
    // 叠加基于位置和时间的随机噪声，模拟磁带信号劣化
    float noise = random(vec2(uv * uResolution + uTime * 100.0));
    color += (noise - 0.5) * uParamFloat1;

    // === VHS色彩偏移 ===
    // 轻微增强红色、减弱蓝色，还原VHS录像的色彩特征
    color.r *= 1.05;
    color.b *= 0.95;

    // 输出最终颜色，限制到[0,1]范围
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

---

## 16. CRT 显示器 (crt)

### 效果概述

CRT 显示器着色器属于 **Retro（复古怀旧）** 类别，模拟了阴极射线管（CRT）显示器的典型视觉特征。该着色器综合了五种 CRT 特征效果：桶形畸变（Barrel Distortion）模拟 CRT 屏幕的曲面玻璃形状；扫描线（Scanlines）模拟电子束逐行扫描的明暗交替；RGB 磷光点遮罩（Phosphor Mask）模拟 CRT 荧光粉的子像素排列；闪烁（Flicker）模拟 CRT 的刷新频率不稳定；暗角（Vignette）模拟 CRT 屏幕边缘的亮度衰减。该效果还包含超出屏幕范围的柔和暗边处理和亮度补偿机制。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 扫描线强度 | Float | 0.0 | 1.0 | 0.15 | slider | 扫描线暗化程度 |
| `uParamFloat1` | 屏幕弯曲 | Float | 0.0 | 0.05 | 0.01 | slider | 桶形畸变强度 |
| `uParamFloat2` | RGB遮罩 | Float | 0.0 | 1.0 | 0.3 | slider | 磷光点遮罩的可见程度 |
| `uParamFloat3` | 亮度 | Float | 0.0 | 2.0 | 1.0 | slider | 整体亮度补偿 |
| `uParamFloat4` | 闪烁 | Float | 0.0 | 0.1 | 0.01 | slider | 闪烁强度 |

### 算法原理

#### 1. 桶形畸变（Barrel Distortion）

CRT 屏幕的曲面玻璃使图像产生向外凸起的桶形畸变。该效果通过径向距离的二次函数模拟：

$$\mathbf{uv}_{curved} = \mathbf{uv}_{centered} \times (1 + k \times r^2) + 0.5$$

其中 $k$ 为畸变参数，$r^2 = \|\mathbf{uv}_{centered}\|^2$ 为到屏幕中心的距离平方。正值产生桶形畸变（边缘向外膨胀），负值产生枕形畸变（边缘向内收缩）。

#### 2. 超出范围处理（Border Fade）

当桶形畸变导致 UV 坐标超出 [0, 1] 范围时，不是简单地截断，而是计算超出量并产生柔和的暗边过渡：

$$\text{excess} = |\mathbf{uv}_{curved} - 0.5| - 0.5$$

$$\text{borderFade} = 1 - \text{clamp}(\max(\text{excess}_x, \text{excess}_y) \times 3, 0.15, 0.4)$$

#### 3. 扫描线（Scanlines）

与 VHS 效果类似，但 CRT 的扫描线更微弱：

$$\text{scanline}(y) = \sin(y \times H \times \pi) \times 0.5 + 0.5$$

$$C_{scan} = C \times \max(1 - \text{scanline} \times s \times 0.25, 0.92)$$

注意这里设置了最低亮度 0.92，防止扫描线过于明显。

#### 4. RGB 磷光点遮罩（Phosphor Mask）

CRT 显示器的每个像素由红、绿、蓝三个荧光粉点组成，呈水平排列。遮罩通过像素位置的模 3 运算模拟：

$$m = \text{mod}(\text{pixel}_x, 3)$$

$$\text{mask} = \begin{cases} 1.0 & m < 1 \text{ (红)} \\ 0.93 & 1 \leq m < 2 \text{ (绿)} \\ 0.87 & 2 \leq m < 3 \text{ (蓝)} \end{cases}$$

#### 5. 闪烁与暗角

闪烁通过 60Hz 正弦波模拟 CRT 的刷新频率不稳定：

$$\text{flicker} = 1 - f \times \sin(t \times 60) \times 0.2$$

暗角通过径向距离的二次函数模拟屏幕边缘亮度衰减：

$$\text{vignette} = 1 - r^2 \times 0.35$$

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

标准声明结构。CRT 效果使用了全部 5 个浮点参数和屏幕分辨率。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：桶形畸变与超出范围处理

将 UV 坐标以画面中心为原点，计算到中心的距离平方。通过二次函数产生桶形畸变。超出 [0, 1] 范围的部分计算柔和暗边过渡，而非硬截断。

```glsl
void main() {
    vec2 uv = vUV - 0.5; // 以中心为原点
    float dist2 = dot(uv, uv); // 到中心的距离平方

    // 屏幕弯曲 (barrel distortion)
    vec2 curvedUV = uv * (1.0 + uParamFloat1 * dist2) + 0.5;

    // 超出范围: 柔和暗边
    vec2 clampedUV = clamp(curvedUV, 0.0, 1.0);
    float borderFade = 1.0;
    if (curvedUV.x < 0.0 || curvedUV.x > 1.0 ||
        curvedUV.y < 0.0 || curvedUV.y > 1.0) {
        vec2 excess = abs(curvedUV - 0.5) - 0.5;
        borderFade = 1.0 - clamp(max(excess.x, excess.y) * 3.0, 0.15, 0.4);
    }
```

#### 段落三：扫描线与磷光点遮罩

扫描线通过正弦函数产生微弱的明暗交替。磷光点遮罩通过像素水平位置的模 3 运算，对 RGB 三通道施加不同的衰减系数，模拟 CRT 荧光粉的子像素排列。

```glsl
    vec3 color = texture(uInputTex, clampedUV).rgb;

    // 扫描线 — 微弱可见
    float scanline = sin(curvedUV.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    float scanDarken = 1.0 - scanline * uParamFloat0 * 0.25;
    color *= max(scanDarken, 0.92);

    // RGB 磷光点遮罩
    vec2 pixelPos = curvedUV * uResolution;
    float modX = mod(pixelPos.x, 3.0);
    float mask = (modX < 1.0) ? 1.0 : ((modX < 2.0) ? 0.93 : 0.87);
    color *= mix(1.0, mask, uParamFloat2);
```

#### 段落四：闪烁、暗角与亮度补偿

闪烁通过 60Hz 正弦波模拟。暗角通过径向距离衰减边缘亮度。最后应用暗边淡出和亮度补偿（含暗部提亮），确保 CRT 效果不会使画面过暗。

```glsl
    // 闪烁
    float flicker = 1.0 - uParamFloat4 * sin(uTime * 60.0) * 0.2;
    color *= flicker;

    // 边缘暗角 — 轻柔
    float vignette = 1.0 - dist2 * 0.35;
    color *= max(vignette, 0.65);

    color *= borderFade;

    // 亮度补偿 + 暗部提亮
    float brightness = 1.0 + uParamFloat3 * 2.0;
    color = color * brightness + 0.05;  // 暗部也加一点

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 扫描线强度
    float uParamFloat1;  // 屏幕弯曲
    float uParamFloat2;  // RGB遮罩
    float uParamFloat3;  // 亮度
    float uParamFloat4;  // 闪烁
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

void main() {
    // 将UV坐标以画面中心为原点
    vec2 uv = vUV - 0.5;
    // 计算到屏幕中心的距离平方
    float dist2 = dot(uv, uv);

    // === 桶形畸变 (Barrel Distortion) ===
    // 通过径向距离的二次函数模拟CRT曲面玻璃的畸变效果
    vec2 curvedUV = uv * (1.0 + uParamFloat1 * dist2) + 0.5;

    // === 超出范围处理 ===
    // 畸变后UV超出[0,1]时，产生柔和暗边过渡而非硬截断
    vec2 clampedUV = clamp(curvedUV, 0.0, 1.0);
    float borderFade = 1.0;
    if (curvedUV.x < 0.0 || curvedUV.x > 1.0 ||
        curvedUV.y < 0.0 || curvedUV.y > 1.0) {
        vec2 excess = abs(curvedUV - 0.5) - 0.5; // 计算超出量
        borderFade = 1.0 - clamp(max(excess.x, excess.y) * 3.0, 0.15, 0.4);
    }

    // 使用畸变后的UV采样输入纹理
    vec3 color = texture(uInputTex, clampedUV).rgb;

    // === 扫描线 ===
    // 通过正弦函数产生微弱的明暗交替，模拟CRT电子束逐行扫描
    float scanline = sin(curvedUV.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    float scanDarken = 1.0 - scanline * uParamFloat0 * 0.25;
    color *= max(scanDarken, 0.92); // 最低亮度0.92，防止扫描线过强

    // === RGB磷光点遮罩 ===
    // 通过像素水平位置的模3运算，模拟CRT荧光粉子像素排列
    vec2 pixelPos = curvedUV * uResolution;
    float modX = mod(pixelPos.x, 3.0);
    float mask = (modX < 1.0) ? 1.0 : ((modX < 2.0) ? 0.93 : 0.87);
    color *= mix(1.0, mask, uParamFloat2); // 通过mix控制遮罩可见度

    // === 闪烁 ===
    // 通过60Hz正弦波模拟CRT刷新频率不稳定
    float flicker = 1.0 - uParamFloat4 * sin(uTime * 60.0) * 0.2;
    color *= flicker;

    // === 暗角 ===
    // 通过径向距离衰减边缘亮度，模拟CRT屏幕边缘变暗
    float vignette = 1.0 - dist2 * 0.35;
    color *= max(vignette, 0.65); // 最低亮度0.65，防止暗角过重

    // 应用暗边淡出
    color *= borderFade;

    // === 亮度补偿 + 暗部提亮 ===
    float brightness = 1.0 + uParamFloat3 * 2.0;
    color = color * brightness + 0.05; // 加0.05确保暗部也有一定亮度

    // 输出最终颜色，限制到[0,1]范围
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

---

## 17. 水波纹 (water_ripple)

### 效果概述

水波纹着色器属于 **Distort（扭曲变形）** 类别，模拟了水面波纹的光学折射效果。该着色器通过三层不同方向、频率和速度的正弦波叠加，生成复杂的水面波纹图案，并基于波纹的法线近似计算折射偏移，使背景图像产生类似透过水面观看的扭曲效果。此外还叠加了高光反射（模拟水面光泽）和深度明暗变化（波峰亮、波谷暗），增强水面的真实感。该效果可应用于水面反射模拟、玻璃折射、热空气扰动等场景。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 振幅 | Float | 0.0 | 0.05 | 0.01 | slider | 波纹的振幅高度 |
| `uParamFloat1` | 频率 | Float | 1.0 | 50.0 | 15.0 | slider | 波纹的空间频率 |
| `uParamFloat2` | 速度 | Float | 0.0 | 5.0 | 2.0 | slider | 波纹的传播速度 |
| `uParamFloat3` | 折射强度 | Float | 0.0 | 0.1 | 0.02 | slider | 折射偏移的强度 |

### 算法原理

#### 1. 多层正弦波叠加

水面波纹由三个不同方向的正弦波叠加而成：

$$h_1(x, t) = A \cdot \sin(f \cdot x + \omega \cdot t)$$

$$h_2(y, t) = 0.7A \cdot \sin(0.8f \cdot y + 1.3\omega \cdot t)$$

$$h_3(x, y, t) = 0.5A \cdot \sin(0.5f \cdot (x + y) + 0.7\omega \cdot t)$$

其中 $A$ 为振幅，$f$ 为频率，$\omega$ 为角速度。三个波分别沿水平、垂直和对角方向传播，频率和速度各不相同，叠加后产生复杂的干涉图案。

#### 2. 法线近似（Normal Approximation）

水面法线通过波高函数的偏导数近似计算：

$$n_x = \frac{\partial h}{\partial x} \approx \frac{\partial h_1}{\partial x} + \frac{\partial h_3}{\partial x}$$

$$n_y = \frac{\partial h}{\partial y} \approx \frac{\partial h_2}{\partial y} + \frac{\partial h_3}{\partial y}$$

正弦函数的导数为余弦函数，因此：

$$\frac{\partial h_1}{\partial x} = A \cdot f \cdot \cos(f \cdot x + \omega \cdot t)$$

#### 3. 折射偏移（Refraction）

基于法线近似值计算折射偏移：

$$\Delta \mathbf{uv} = (n_x, n_y) \times \text{refraction}$$

折射偏移量与法线分量成正比，法线越陡（波纹越剧烈），折射偏移越大。

#### 4. 高光反射（Specular Highlight）

使用 Phong 高光模型计算水面反射光斑：

$$\text{specular} = \left(\max(\text{dot}(\hat{\mathbf{n}}, \hat{\mathbf{v}}), 0)\right)^{32}$$

其中 $\hat{\mathbf{n}} = \text{normalize}(n_x, n_y, 1)$ 为近似法线，$\hat{\mathbf{v}} = (0, 0, 1)$ 为视线方向。指数 32 产生集中的高光光斑。

#### 5. 深度明暗（Depth Shading）

波峰（正值）变亮，波谷（负值）变暗，增强水面的立体感：

$$C_{depth} = (h_1 + h_2 + h_3) \times 10 \times 0.05$$

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

标准声明结构。水波纹效果使用 4 个浮点参数控制波纹的物理特性。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：多层正弦波叠加

三个正弦波分别沿不同方向传播，频率和速度各不相同。第一波沿水平方向，第二波沿垂直方向（频率和速度略有不同），第三波沿对角方向（频率最低、振幅最小）。

```glsl
void main() {
    // 多层正弦波叠加模拟水波纹
    vec2 uv = vUV;
    float wave1 = sin(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0;
    float wave2 = sin(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7;
    float wave3 = sin((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5;
```

#### 段落三：法线近似与折射偏移

通过对波高函数求偏导数（正弦的导数为余弦），得到水面法线的近似值。法线分量乘以折射强度参数得到 UV 偏移量，用于采样偏移后的纹理。

```glsl
    // 法线近似 (偏导数)
    float dx = cos(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0 * uParamFloat1
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;
    float dy = cos(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7 * uParamFloat1 * 0.8
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;

    // 折射偏移
    vec2 refractOffset = vec2(dx, dy) * uParamFloat3;
    vec3 color = texture(uInputTex, uv + refractOffset).rgb;
```

#### 段落四：高光反射与深度明暗

高光使用 Phong 模型计算法线与视线的点积的 32 次方，产生集中的光斑。深度明暗通过波高总和的缩放实现波峰亮、波谷暗的效果。

```glsl
    // 高光 (模拟水面反射)
    float specular = pow(max(dot(normalize(vec3(dx, dy, 1.0)), normalize(vec3(0.0, 0.0, 1.0))), 0.0), 32.0);
    color += specular * 0.15;

    // 深度感 (波峰亮波谷暗)
    float depth = (wave1 + wave2 + wave3) * 10.0;
    color += depth * 0.05;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 振幅
    float uParamFloat1;  // 频率
    float uParamFloat2;  // 速度
    float uParamFloat3;  // 折射强度
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

void main() {
    // === 多层正弦波叠加 ===
    // 三层不同方向、频率和速度的正弦波，模拟复杂的水面干涉图案
    vec2 uv = vUV;
    // 第一波：水平方向，基准频率和速度
    float wave1 = sin(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0;
    // 第二波：垂直方向，频率略低(0.8x)，速度略快(1.3x)，振幅0.7x
    float wave2 = sin(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7;
    // 第三波：对角方向，频率最低(0.5x)，速度最慢(0.7x)，振幅0.5x
    float wave3 = sin((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5;

    // === 法线近似（偏导数）===
    // 对波高函数求x和y方向的偏导数（正弦的导数为余弦）
    // dx: 第一波和第三波的x方向偏导数之和
    float dx = cos(uv.x * uParamFloat1 + uTime * uParamFloat2) * uParamFloat0 * uParamFloat1
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;
    // dy: 第二波和第三波的y方向偏导数之和
    float dy = cos(uv.y * uParamFloat1 * 0.8 + uTime * uParamFloat2 * 1.3) * uParamFloat0 * 0.7 * uParamFloat1 * 0.8
             + cos((uv.x + uv.y) * uParamFloat1 * 0.5 + uTime * uParamFloat2 * 0.7) * uParamFloat0 * 0.5 * uParamFloat1 * 0.5;

    // === 折射偏移 ===
    // 法线分量乘以折射强度，得到UV偏移量
    vec2 refractOffset = vec2(dx, dy) * uParamFloat3;
    // 使用偏移后的UV采样输入纹理
    vec3 color = texture(uInputTex, uv + refractOffset).rgb;

    // === 高光反射 ===
    // Phong高光模型：法线与视线方向的点积的32次方
    // 法线近似为(dx, dy, 1)，视线方向为(0, 0, 1)
    float specular = pow(max(dot(normalize(vec3(dx, dy, 1.0)), normalize(vec3(0.0, 0.0, 1.0))), 0.0), 32.0);
    color += specular * 0.15; // 高光强度0.15

    // === 深度明暗 ===
    // 波峰（正值）变亮，波谷（负值）变暗
    float depth = (wave1 + wave2 + wave3) * 10.0;
    color += depth * 0.05; // 深度影响系数0.05

    // 输出最终颜色，限制到[0,1]范围
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

---

## 18. 镜头畸变 (lens_distort)

### 效果概述

镜头畸变着色器属于 **Distort（扭曲变形）** 类别，模拟了真实相机镜头的光学畸变效果。该着色器基于 Brown-Conrady 畸变模型的简化版本，通过径向距离的二阶和四阶多项式来控制图像的桶形畸变（Barrel Distortion，正值参数）和枕形畸变（Pincushion Distortion，负值参数）。桶形畸变使图像边缘向外膨胀，中心区域相对收缩，类似鱼眼镜头效果；枕形畸变使图像边缘向内收缩，中心区域相对膨胀。该效果常用于模拟特定镜头特征、校正镜头畸变或创造特殊的视觉风格。

### 参数说明

| 参数名 | 标签 | 类型 | 最小值 | 最大值 | 默认值 | UI 类型 | 说明 |
|--------|------|------|--------|--------|--------|---------|------|
| `uParamFloat0` | 畸变强度 | Float | -0.5 | 0.5 | 0.1 | slider | 畸变系数，正值=桶形，负值=枕形 |
| `uParamFloat1` | 缩放 | Float | 0.5 | 2.0 | 1.0 | slider | 画面缩放比例 |

### 算法原理

#### 1. Brown-Conrady 畸变模型

Brown-Conrady 模型是摄影测量学中描述镜头畸变的标准模型。其简化径向畸变公式为：

$$r_{distorted} = r \times (1 + k_1 \cdot r^2 + k_2 \cdot r^4)$$

其中 $r$ 为到图像中心的归一化距离，$k_1$ 为二阶径向畸变系数，$k_2$ 为四阶径向畸变系数。

在本着色器中，$k_2$ 被简化为 $k_1$ 的一半：

$$d(r) = 1 + k \cdot r^2 + 0.5k \cdot r^4$$

$$\mathbf{uv}_{distorted} = \frac{\mathbf{uv} \times d(r)}{zoom} \times 0.5 + 0.5$$

#### 2. 坐标空间转换

- 原始 UV 坐标范围：[0, 1]
- 以中心为原点：$\mathbf{uv} = (\text{vUV} - 0.5) \times 2$，范围 [-1, 1]
- 径向距离：$r^2 = \|\mathbf{uv}\|^2 = u_x^2 + u_y^2$
- 畸变后坐标：$\mathbf{uv}_{distorted} = \mathbf{uv} \times d(r) / zoom$
- 映射回 UV 空间：$\mathbf{uv}_{final} = \mathbf{uv}_{distorted} \times 0.5 + 0.5$

#### 3. 边界处理

畸变后的坐标通过 `clamp` 限制在 [0, 1] 范围内，超出部分采样最近边缘像素。这与 CRT 着色器的柔和暗边处理不同，镜头畸变使用简单的边缘夹紧，因为真实镜头畸变不会产生暗边。

### 逐段代码分析

#### 段落一：着色器声明与 Uniform 变量

标准声明结构。镜头畸变效果仅使用 2 个浮点参数，是所有着色器中参数最少的之一。

```glsl
#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};
```

#### 段落二：坐标转换与畸变计算

将 UV 坐标以画面中心为原点归一化到 [-1, 1]，计算径向距离的平方和四次方，然后应用 Brown-Conrady 畸变公式。除以缩放参数实现画面缩放，最后映射回 [0, 1] 纹理空间。

```glsl
void main() {
    vec2 uv = (vUV - 0.5) * 2.0; // [-1, 1]

    // 径向畸变 (Brown-Conrady 模型简化版)
    float r2 = dot(uv, uv);     // 距离平方
    float r4 = r2 * r2;          // 距离四次方
    float distortion = 1.0 + uParamFloat0 * r2 + uParamFloat0 * 0.5 * r4;
    vec2 distortedUV = uv * distortion / uParamFloat1; // 应用畸变和缩放
    distortedUV = distortedUV * 0.5 + 0.5;            // 映射回[0,1]

    // 超出范围采样最近边缘
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    vec3 color = texture(uInputTex, distortedUV).rgb;
    outColor = vec4(color, 1.0);
}
```

### 完整源码

```glsl
#version 460
// 顶点着色器传入的纹理坐标
layout(location=0) in vec2 vUV;
// 输出颜色
layout(location=0) out vec4 outColor;
// 输入纹理采样器
layout(binding=0) uniform sampler2D uInputTex;
// 统一参数块
layout(std140, binding=1) uniform Params {
    float uParamFloat0;  // 畸变强度
    float uParamFloat1;  // 缩放
    float uParamFloat2;  // 保留参数
    float uParamFloat3;  // 保留参数
    float uParamFloat4;  // 保留参数
    float uParamFloat5;  // 保留参数
    vec2 uResolution;    // 屏幕分辨率
    float uTime;         // 时间
    float uFrameCount;   // 帧计数
};

void main() {
    // 将UV坐标以画面中心为原点归一化到[-1, 1]范围
    vec2 uv = (vUV - 0.5) * 2.0;

    // === 径向畸变 (Brown-Conrady模型简化版) ===
    // 计算到中心的距离平方和四次方
    float r2 = dot(uv, uv);  // r^2 = x^2 + y^2
    float r4 = r2 * r2;       // r^4 = (r^2)^2
    // 畸变因子：1 + k*r^2 + 0.5k*r^4
    // k > 0: 桶形畸变（边缘向外膨胀）
    // k < 0: 枕形畸变（边缘向内收缩）
    float distortion = 1.0 + uParamFloat0 * r2 + uParamFloat0 * 0.5 * r4;
    // 应用畸变并除以缩放参数
    vec2 distortedUV = uv * distortion / uParamFloat1;
    // 将坐标从[-1,1]映射回[0,1]纹理空间
    distortedUV = distortedUV * 0.5 + 0.5;

    // 超出范围时采样最近边缘像素（clamp夹紧）
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    // 使用畸变后的UV坐标采样输入纹理
    vec3 color = texture(uInputTex, distortedUV).rgb;
    outColor = vec4(color, 1.0);
}
```
