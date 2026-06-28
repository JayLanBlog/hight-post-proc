# Awesome-Unity-Shader → ShaderShowcase 移植设计

> 创建: 2026-06-28 · 状态: 已确认

## 1. 目标

将 Awesome-Unity-Shader（项目11）的 16 卷 Shader 全部移植到 ShaderShowcase 桌面应用，升级为"后处理+3D物体"综合 Shader 展示器。

## 2. 源材料

| 项 | 值 |
|---|-----|
| 源项目 | `e:\AI\test\Awesome-Unity-Shader` |
| 目标项目 | `e:\AI\graph\hight-post-proc\shader-showcase` |
| 总卷数 | 16 (Volume 01 ~ Volume 16) |
| 源文件 | 67 个 .shader + 5 个 .cs + 18 张纹理 + 1 个 .cubemap |
| 移植后新增效果 | **62** (跳过的5个: 3个模板、Standard已内置可参考、像素化已有点差) |
| 最终效果总数 | 91(现有) + 62(新增) = **153** |

## 3. 架构：双管线共存

```
┌─────────────────────────────────────────────────────────┐
│                    ShaderShowcase                        │
├──────────────────────────────┬──────────────────────────┤
│   现有 91 效果 (全屏后处理)      │   新增 62 效果 (3D+后处理)      │
│   fullscreen.vert            │   mesh3d.vert  ←新       │
│   UBO=48字节 (固定)            │   UBO=160字节 (扩展)        │
│   DrawFullscreenQuad         │   DrawMesh ←新           │
│   输入: 1张纹理                │   输入: 1~3张纹理             │
└──────────────────────────────┴──────────────────────────┘
```

**原则:** 两种管线通过 CARD 宏的 `vert` 字段区分，运行时互不干扰。

## 4. 3D 基础舞台 (BaseScene)

### 4.1 几何体

CPU 侧预生成，GPU buffer 上传：
- **球体**: 半径 1.0, 经纬细分 64×32, ~2000 顶点, ~4000 三角形索引
- **立方体**: 边长 2.0, 6 面×2 三角, 36 顶点 (含法线)
- 顶点格式: `[Position:vec3, Normal:vec3, UV:vec2]` (28 字节)

### 4.2 灯光

UBO 中直传（对标 Unity `_WorldSpaceLightPos0` / `_LightColor0`）:
- `vec3 LightDir` = 方向光 (可旋转)
- `vec3 LightColor` = 白色 (1,1,1)
- ImGui 提供滑块调节

### 4.3 摄像机

CPU 侧计算矩阵，UBO 传 `mat4 MVP` + `mat4 ModelView`:
- 鼠标右键拖拽旋转、滚轮缩放
- `vec3 EyePos` 用于边缘光计算 (对标 Unity `_WorldSpaceCameraPos`)

### 4.4 UBO 布局 (mesh3d.vert, 160 字节)

```glsl
layout(std140, binding=1) uniform Params {
    float uParamFloat0..5;        // offset  0~23 → 兼容现有
    vec2 uResolution; float uTime; float uFrameCount;  // 24~39 → 兼容现有
    mat4 uMVP;                    // 40~103 → 新增
    mat4 uModelView;              // 104~167 → 新增
    vec3 uLightDir;               // 168~179 → 新增
    vec3 uLightColor;             // 180~191 → 新增
    vec3 uEyePos;                 // 192~203 → 新增 (padding 到 208)
};
```

## 5. 效果分类与移植策略

### 5.1 后处理屏幕特效 (5个，Phase 1)

| 卷 | Shader | 效果 | GLSL 实现要点 |
|----|--------|------|---------------|
| Vol.08 | MotionBlurEffects | 径向模糊 | 中心→边缘采样密度递增 |
| Vol.09 | ScreenWaterDropEffect | 水幕特效 | 法线贴图uv偏移 + 折射模拟 |
| Vol.10 | ScreenOilPaintEffect | 油画特效 | Kuwahara 滤波, 网格采样中值 |
| Vol.11 | PixelEffect | 像素化 | 已有对等效果 (pixelate/xpl_pixel_quad)，跳过 |
| Vol.15 | RapidBlurEffect | 高斯模糊 | 降采样 + 两 Pass 水平/垂直 → 单 Pass 实现 |

**移植方式:** 读原 .shader → 提取 frag 核心 → 重写 GLSL 460 → effect.json 配参 → 编译

### 5.2 3D 物体着色器 (57个，Phase 2~6)

| Phase | 卷 | 效果数 | 说明 |
|-------|-----|--------|------|
| 2 | Vol.01-04 | 19 | 凹凸纹理/边缘光, 基础光照/纹理混合, 剔除/Alpha/雾效 |
| 3 | Vol.05-07 | 15 | Blend/玻璃, SurfaceShader/凹凸/细节/边缘, 自定义光照模型 |
| 4 | Vol.12-13 | 12 | 可编程管线(RGB Cube/Lambert), 透明/镜面高光 |
| 5 | Vol.14, Vol.16 | 3 | Rim Shader×2, MatCap车漆 |
| — | Vol.09 Standard | 1 | 跳过 (教学参考) |
| — | Vol.08 模板×3 | 3 | 跳过 (模板) |

每效果文件结构:
```
shaders/common/mesh3d.vert            ← 共享的3D顶点着色器 (所有3D效果引用此路径)
shaders/effects/aus_v01_first_shader/
├── aus_v01_first_shader.frag         ← 核心算法
├── aus_v01_first_shader.frag.spv
└── effect.json                       ← 元数据
```

### 5.3 纹理资源

直接从项目11复制（同机 `e:\AI\test\` → `e:\AI\graph\hight-post-proc\`）:

| 源路径 | 目标路径 | 用途 |
|--------|----------|------|
| Volume 16/MatCap Textrues/*.jpg | assets/textures/matcap/ | MatCap 车漆 |
| Volume 09/ScreenWaterDropEffect/Resources/*.png | assets/textures/ | 水幕法线 |
| 现有 assets/images/00~17.jpg | (复用) | 后处理特效输入 |

## 6. 自动化验证

沿用本次会话验证的成熟流程，扩展到 153 个效果：

### Step 1: 编译验证
```ps1
glslangValidator -V $effect.frag -o $effect.frag.spv
# 0 error → pass
```

### Step 2: 全卡截图
```ps1
$env:AUTO_TEST_CARDS = '1'
.\ShaderShowcase.exe  # 遍历0~152, 输出153张 PPM
```

### Step 3: 统计验证 (复用 deep_compare.py / full_reaudit.py)
- avg > 3 且 avg < 252 (非全黑/全白)
- unique > 5 (非单色)
- 3D 效果: 与 2D 参考图差异 > 10% (证明渲染了3D物体非原图)

## 7. 文件变更范围

| 文件 | 变更 | 说明 |
|------|------|------|
| `src/app/CoverFlowScene.cpp` | +62 CARD 宏 | 新增效果卡片 |
| `src/app/CoverFlowScene.h` | 新增成员 | m_meshVB/VB_mem/IB/IB_mem, m_modelViewProj |
| `src/app/EffectDetailScene.cpp` | +60行 | 3D 效果特殊处理 |
| `shaders/common/mesh3d.vert` | 新建 | 3D 顶点着色器 |
| `shaders/effects/aus_v01_*/` | 新建 62个目录 | 每个含 .vert .frag .frag.spv effect.json |
| `shaders/CMakeLists.txt` | +62行 | 新增编译规则 |
| `assets/textures/matcap/` | 新建目录 | 17张 MatCap 贴图 |
| `assets/textures/` | +1张 | 水幕法线贴图 |
| `src/render/VulkanBackend.cpp` | +40行 | DrawMesh 方法 |

## 8. 风险与应对

| 风险 | 应对 |
|------|------|
| 62个效果编译失败 | 逐个排查, SPIR-V 兼容性由 glslangValidator 把关 |
| 3D 效果在缩略图中不可见 | 缩略图渲染时额外提供 3D 场景基座 |
| UBO 布局溢出 | 自适应 UBO 大小计算 `std140` 对齐 |
| 影响现有效果 | fullscreen.vert 不被修改, 新增代码仅添加不修改 |