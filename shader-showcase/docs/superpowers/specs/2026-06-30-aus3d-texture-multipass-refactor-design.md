# AUS3D 纹理+多Pass重构设计文档

**日期**: 2026-06-30  
**状态**: 已批准  
**范围**: 14个AUS3D效果重构，Vulkan后端扩展，纹理系统引入

---

## 1. 背景与动机

当前 AUS3D 系统的所有效果基于 Vulkan GLSL 光线追踪（球体/立方体 ray-sphere intersection），纯程序化数学计算渲染，不使用纹理采样。参考项目 `aus_repo` 是 Unity ShaderLab 实现，依赖纹理贴图（Ramp纹理、Cubemap、水滴纹理）和多Pass渲染管线。两者技术路线完全不同，导致部分效果（尤其是水幕特效、卡通渐变、高斯模糊）与参考项目的视觉表现差异较大。

本次重构的目标是：在保持现有 Vulkan 光追架构的基础上，引入纹理系统和多Pass渲染能力，让14个关键效果更接近参考项目的实现方式。

---

## 2. 范围

### 2.1 重构效果（14个）

**Tier 1 — 必须重构（6个）：**
| 效果 | 当前Shader | 改造方式 |
|------|-----------|---------|
| 水幕特效 | `v09_water_drop.frag` | 多Pass: 水滴纹理采样 → UV偏移 → 合成 |
| 卡通渐变 | `v07_toon.frag` | 保留球体: Ramp纹理替代色阶 |
| 玻璃v2 | `v05_glass_v2.frag` | 保留球体: Cubemap反射替代程序化天空 |
| 玻璃v3 | `v05_glass_v3.frag` | 保留球体: Cubemap反射 + 纹理Alpha |
| 高斯模糊 | `v15_gaussian_blur.frag` | 多Pass: 降采样 → 垂直模糊 → 水平模糊 |
| 径向模糊 | `v08_radial_blur.frag` | 后处理: 10次迭代缩放UV采样 |

**Tier 2 — 建议重构（8个）：**
| 效果 | 当前Shader | 改造方式 |
|------|-----------|---------|
| 像素化 | `v11_pixelate.frag` | 后处理: UV量化（ceil对齐） |
| 油画特效 | `v10_oil_paint.frag` | 后处理: 邻域颜色统计 |
| MatCap车漆 | `v16_carpaint.frag` | 保留球体: MatCap纹理采样 |
| 凹凸边缘光 | `v01_bump_rim.frag` | 保留球体: 凹凸纹理法线扰动 |
| 细节纹理 | `v06_detail.frag` | 保留球体: 细节纹理叠加 |
| 边缘光+纹理 | `v06_rim_detail.frag` | 保留球体: 纹理+边缘光 |
| Alpha混合纹理 | `v03_alpha.frag` | 保留球体: 纹理Alpha通道 |
| 纹理混合 | `v03_mix.frag` | 保留球体: 多纹理混合 |

### 2.2 不变效果（33个）

基础单色、漫反射、镜面高光、半Lambert、诊断/测试类、Vol.12可编程Shader、Vol.13透明Shader等保持现有光追实现不变。

---

## 3. 架构设计

### 3.1 总体架构

```
┌─────────────────────────────────────────────────┐
│                  AUS3DScene                      │
│  ┌──────────────┐  ┌──────────────┐             │
│  │ AUS3DEffect  │  │ TextureMgr   │             │
│  │ +passes[]    │  │ +LoadTexture │             │
│  │ +auxTextures │  │ +GenRamp     │             │
│  │ +use3DGeom   │  │ +GenNoise    │             │
│  └──────┬───────┘  └──────┬───────┘             │
│         │                 │                      │
│  ┌──────▼─────────────────▼───────┐             │
│  │        渲染循环                  │             │
│  │  for (pass in effect.passes)   │             │
│  │    if pass.isOutput:           │             │
│  │      DrawToScreen()            │             │
│  │    else:                       │             │
│  │      DrawToTexture(RT)         │             │
│  └──────────────┬─────────────────┘             │
└─────────────────┼───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│              VulkanBackend                       │
│  + CreateRenderTarget(w, h) → TextureHandle     │
│  + DestroyRenderTarget(handle)                  │
│  + DrawToTexture(rt, vert, frag, params)        │
│  + DrawToScreen(vert, frag, params, inputTex)   │
│  + CreateTextureFromFile(path) → TextureHandle  │
│  + CreateTextureFromData(w, h, data)            │
└─────────────────────────────────────────────────┘
```

### 3.2 数据流

**单Pass效果（保持现有）：**
```
Clear → DrawFullscreenQuad(vert, frag, params) → 屏幕
```

**多Pass效果（新增）：**
```
Clear → DrawToTexture(RT1, pass0)
      → DrawToTexture(RT2, pass1, inputTex=RT1)
      → DrawToScreen(pass2, inputTex=RT2) → 屏幕
```

**纹理效果（新增）：**
```
加载PNG → CreateTexture → Bind(binding=2) → Shader: tex2D(uAuxTex, uv)
```

---

## 4. VulkanBackend 扩展

### 4.1 新增接口

| 方法 | 用途 | 关键实现 |
|------|------|---------|
| `CreateRenderTarget(w, h)` | 创建离屏渲染目标 | `VkImage` + `VkImageView` + `VkFramebuffer` + `VkSampler` |
| `DestroyRenderTarget(rt)` | 销毁离屏RT | 逆序释放上述资源 |
| `DrawToTexture(rt, vert, frag, params, [inputTex])` | 渲染到RT | 复用 `DrawFullscreenQuad` 逻辑，替换 framebuffer |
| `DrawToScreen(vert, frag, params, inputTex)` | 渲染到屏幕（带纹理输入） | 增强版 `DrawFullscreenQuad`，指定 inputTex |
| `CreateTextureFromFile(path)` | 从PNG/JPG加载纹理 | stb_image → VkImage → VkImageView → VkSampler |
| `CreateTextureFromData(w, h, data)` | 从内存数据创建纹理 | 原始像素数据 → VkImage |

### 4.2 描述符布局扩展

当前布局：
- binding=0: `uInputTex` (sampler2D)
- binding=1: UBO (std140)

新布局：
- binding=0: `uInputTex` (sampler2D) — 主输入纹理 / Pass间中间结果
- binding=1: UBO (std140) — 不变
- binding=2: `uAuxTex` (sampler2D) — 新增，辅助纹理（水滴图、Ramp等）

管线缓存键需包含新 binding 信息。

### 4.3 RT池管理

为减少每帧 VkImage 创建销毁开销，AUS3DScene 维护简单 RT 池：

```cpp
struct RTPool {
    map<pair<int,int>, vector<TextureHandle>> free;
    TextureHandle Acquire(int w, int h);
    void Release(TextureHandle rt);
};
```

---

## 5. AUS3DScene 扩展

### 5.1 数据结构

```cpp
struct AUS3DPass {
    string fragShader;    // SPIR-V路径
    int targetWidth;      // 0=全屏, N=降采样到N
    int targetHeight;
    bool isOutput;        // 最后一个Pass输出到屏幕
};

struct AUS3DEffect {
    // ... 现有字段 ...
    vector<AUS3DPass> passes;     // Pass序列
    vector<string> auxTextures;   // 辅助纹理路径
    bool use3DGeometry;           // 是否保留光追球体
};
```

### 5.2 纹理管理器

```cpp
class TextureManager {
    IRenderBackend* m_backend;
    map<string, TextureHandle> m_cache;
    
    TextureHandle LoadTexture(const string& path);
    TextureHandle GenerateRampTexture(int bands);      // 程序化色阶
    TextureHandle GenerateNoiseTexture(int size);       // 程序化噪声
    TextureHandle GetOrCreate(const string& key);
};
```

纹理文件路径: `assets/textures/aus3d/`
- `water_drop.png` — 从参考项目导出
- `matcap_*.png` — 从参考项目复制
- Ramp/Noise — 程序化生成，无需文件

### 5.3 渲染循环改造

```cpp
void OnRender() {
    if (fx.passes.empty()) {
        // 旧路径：单Pass光追球体
        DrawFullscreenQuad(sharedVert, fx.fragShader, params);
    } else {
        // 新路径：多Pass序列
        TextureHandle prevRT = nullptr;
        for (int i = 0; i < fx.passes.size(); i++) {
            auto& pass = fx.passes[i];
            if (pass.isOutput) {
                backend->DrawToScreen(sharedVert, pass.fragShader, params, prevRT);
            } else {
                auto rt = rtPool.Acquire(pass.targetWidth, pass.targetHeight);
                backend->DrawToTexture(rt, sharedVert, pass.fragShader, params, prevRT);
                rtPool.Release(prevRT);
                prevRT = rt;
            }
        }
    }
}
```

---

## 6. 着色器改造策略

### 6.1 类型A：完全后处理（5个效果）

移除光追球体，改为全屏采样 `uInputTex`。

| 效果 | Pass数 | 各Pass说明 |
|------|--------|-----------|
| 水幕特效 | 3 | Pass0: 水滴纹理 → UV偏移; Pass1: 偏移UV采样球体; Pass2: 合成输出 |
| 高斯模糊 | 3 | Pass0: 降采样1/4; Pass1: 7tap垂直模糊; Pass2: 7tap水平模糊 |
| 径向模糊 | 1 | 10次迭代缩放UV，以中心为原点 |
| 像素化 | 1 | UV坐标ceil取整到像素块网格 |
| 油画特效 | 1 | 邻域像素颜色统计取众数 |

### 6.2 类型B：保留球体 + 纹理增强（9个效果）

保留光追球体，用纹理采样替代部分数学计算。

| 效果 | 纹理类型 | 采样方式 |
|------|---------|---------|
| 卡通渐变 | Ramp纹理 | `tex2D(Ramp, NdotL)` 替代色阶if/else |
| 玻璃v2/v3 | Cubemap | `texCube(Cubemap, R)` 替代程序化天空 |
| MatCap车漆 | MatCap纹理 | `tex2D(MatCap, N·V)` 替代公式 |
| 凹凸纹理 | 噪声纹理 | `N += tex2D(Bump, uv) * strength` |
| 细节纹理 | 细节纹理 | `baseColor * tex2D(Detail, uv*scale)` |
| Alpha混合 | 带Alpha纹理 | `alpha = tex2D(Tex, uv).a` |

---

## 7. 测试与验证

### 7.1 自动化截图测试

- 环境变量 `AUTO_TEST_AUS3D=1` 驱动全47效果截图
- 33个未变更效果 → 截图应与重构前完全一致
- 14个变更效果 → 与参考项目截图做特征级匹配（非像素级精确匹配）

### 7.2 分阶段实施

| 阶段 | 内容 | 验证方式 |
|------|------|---------|
| 阶段1 | Vulkan后端扩展（RT/CreateTexture） | 诊断效果验证 |
| 阶段2 | 后处理型效果逐个迁移（5个） | 每迁移1个截图对比 |
| 阶段3 | 球体+纹理型效果逐个迁移（9个） | 每迁移1个截图对比 |

### 7.3 回退安全

- `use3DGeometry` 标志控制走新/旧路径
- 33个未变更效果 `passes.empty()` 走旧路径，零影响
- 任何阶段出错可立即回退到旧实现

### 7.4 性能约束

- 多Pass效果每帧 RT 创建/销毁 ≤ 3个
- RT池复用避免频繁 VkImage 分配
- 纹理加载仅在场景初始化时执行一次

---

## 8. 附录

### 8.1 参考项目纹理清单

| 纹理 | 来源 | 用途 |
|------|------|------|
| `ScreenWaterDrop.png` | `aus_repo/Volume 09/.../Resources/` | 水幕特效水滴纹理 |
| `CarPaint-MatCap.png` | `aus_repo/Volume 16/` | MatCap车漆纹理 |
| `MatCap Textrues/*.jpg` | `aus_repo/Volume 16/MatCap Textrues/` | MatCap参考纹理 |
| Ramp纹理 | 程序化生成 | 卡通渐变色阶 |
| Cubemap | 程序化生成 | 玻璃反射 |
| 噪声纹理 | 程序化生成 | 凹凸纹理 |

### 8.2 关键决策记录

- **多Pass方案**: 选择中层方案B（扩展Vulkan后端支持RenderTarget），而非完整DAG管线框架
- **纹理方案**: 选择混合方案B（外部文件 + 程序化兜底），关键纹理从文件加载，Ramp/Cubemap/噪声程序化生成
- **范围**: 选择Tier 1+2（14个效果），Tier 3（33个效果）保持不变