# VFX Fire Book - Technical Design Specification

## 项目概述

完整复刻 Unity VFX Graph Magic Fire Book 效果到 ShaderShowcase 项目。包含：
- 3D 翻开书本模型渲染（10个子网格，8种材质）
- 书页燃烧溶解Shader（后处理Pass，噪声纹理+AlphaClip）
- GPU粒子系统（纸屑+烟雾，Compute Shader驱动）
- 火焰燃烧音频（集成miniaudio轻量库）

**设计决策汇总：**

| 项目 | 选择 | 理由 |
|------|------|------|
| 3D书模 | 全部8种材质 | 完整复刻 |
| 溶解Shader | 后处理Pass | B方案 - 灵活可扩展，便于添加辉光合成 |
| 粒子系统 | GPU Compute Shader | A方案 - 5000烟雾粒子性能更好 |
| 音频 | 需要集成 | A方案 - 完整复刻所有元素 |
| 架构 | 模块化拆分 | B方案 - 职责分离，可复用粒子系统 |

---

## 1. 3D书本模型数据

### 1.1 源文件信息

- **FBX源文件**: `e:\AI\test\VFX-SHADER-GRAPH-Magic-Fire-Book\Assets\Models\Book\BOOK FBX.fbx` (148KB)
- **提取输出**: `assets/models/book/`
- **文件结构**:
  ```
  assets/models/book/
  ├── book_combined.bin      # 所有10个子网格合并 (100KB)
  ├── book_mesh_info.json     # 网格信息JSON
  ├── book_mesh0.bin ...     # 独立子网格文件
  └── textures/
      ├── Book_1_Diffuse.png
      ├── Book_1_Normal.png
      ├── Book_2_Diffuse.png
      ├── Book_2_Normal.png
      ├── Front_Diffuse.png
      ├── Front_Normal.png
      ├── Side_Diffuse.png
      └── Side_Normal.png
  ```

### 1.2 网格结构统计

| 节点名称 | 顶点数 | 索引数 | 三角形数 | 材质 |
|----------|--------|--------|----------|------|
| page_top_left_2 | 224 | 753 | 251 | Book_Base |
| page_top_left | 224 | 753 | 251 | Book_Base |
| page_top_right_2 | 224 | 753 | 251 | Book_Base |
| page_top_right | 224 | 753 | 251 | Book_Base |
| lower_right | 60 | 228 | 76 | 多材质 |
| side_right | 4 | 4 | 2 | Book_Base |
| lower_left | 60 | 228 | 76 | 多材质 |
| Side_left | 4 | 4 | 2 | Book_Base |
| Background | 4 | 4 | 2 | Book_Base |
| Book_Cover_base | 1110 | 4416 | 1472 | Book_Base+Book_Page_Left |
| **总计** | **2138** | **7896** | **2634** | **8种材质** |

### 1.3 纹理资源

所有纹理已复制到 `assets/models/book/textures/`:

| 纹理名称 | 用途 |
|----------|------|
| Book_1_Diffuse.png | 左页纸张漫反射 |
| Book_1_Normal.png | 左页纸张法线 |
| Book_2_Diffuse.png | 右页纸张漫反射 |
| Book_2_Normal.png | 右页纸张法线 |
| Front_Diffuse.png | 封面漫反射 |
| Front_Normal.png | 封面法线 |
| Side_Diffuse.png | 书侧边缘漫反射 |
| Side_Normal.png | 书侧边缘法线 |

### 1.4 渲染光照设置

参考场景中有一个平行光 + 三个点光源：

| 光源 | 位置 | 强度 | 范围 | 颜色 |
|------|------|------|------|------|
| Directional | (0, 3, 0) | 2.0 | 10 | (1, 1, 1) |
| PointLight1 | (-165.45, 1.697, 12.49) | - | - | - |

*注：平行光为主要光源，强度2.0足够照亮整个场景*

---

## 2. 燃烧书页溶解Shader

### 2.1 原始 ShaderGraph 结构

源文件: `Assets/Shaders/BurningPaper.shadergraph`

**7个暴露参数**:

| 参数名 | 默认值 | 范围 | 说明 |
|--------|--------|------|------|
| `BaseMap` | - | - | 基础纹理采样 |
| `NormalMap` | - | - | 法线贴图 |
| `DissolveAmount` | 0.5 | 0-1 | 溶解进度 |
| `EdgeWidth` | 0.2 | 0-1 | 燃烧边缘宽度 |
| `EdgeColor` | (6.5, 0.89, 0.0, 1.0) | - | 边缘发光颜色（橙色） |
| `NoiseScale` | (4.0, 8.0) | - | 噪声缩放（XY） |
| `NoiseOffset` | (0.0, 1.0) | - | 噪声偏移（随时间动画） |

**算法节点流程**:

```
DissolveAmount (0-1)
  ↓
Remap (in 0-1 → out 0.5-1.3)
  ↓
Clamp (0-1)
  ↓
Step (threshold=0.29) → 二值化溶解区域
  ↓
OneMinus → 翻转（溶解区域=1，非溶解=0）
  ↓
Sample Noise Texture with NoiseScale+NoiseOffset
  ↓
Multiply with OneMinus result
  ↓
Alpha Clip (discard if value < 0.5)
  ↓
Edge Detection:
  - Remap 结果 ± EdgeWidth
  - Smoothstep 生成边缘mask
  - 乘以 EdgeColor → 添加到 Emission
```

**关键参数提取**:
- `Remap In Min/Max`: 0 → 1
- `Remap Out Min/Max`: 0.5 → 1.3
- `Step Threshold`: 0.29
- `AlphaClip Threshold`: 0.5
- `EdgeWidth`: 0.2
- `EdgeColor`: RGB (6.5, 0.894, 0.0) - 高亮度橙色发光

### 2.2 后处理Pass方案

根据选择B方案，溶解在独立后处理Pass进行：

**管线流程**:
```
1. 正常渲染书本到帧缓冲 (RT)
2. 全屏四边形绘制，溶解Shader采样RT
3. 溶解Shader执行:
   - 噪声采样 + 溶解计算
   - discard 掉已溶解像素
   - 边缘发光输出
4. 粒子系统合成到最终画面
```

**Shader 输入**:
- `binding=0`: `uInputTex` - 渲染好的书本RT
- `binding=1`: `uNoiseTex` - 噪声纹理
- `binding=2`: UBO - `uniformFloats[0]` = DissolveAmount (0-1)

**输出**: 直接到屏幕

---

## 3. GPU粒子系统

### 3.1 BurningPagesVFX（燃烧纸屑）

参数提取自 `BurningPagesVFX2.vfx`:

| 参数 | 值 | 说明 |
|------|-----|------|
| **容量** | 32粒子 | 最大同时存在 |
| **Spawn Rate** | 16 粒子/秒 | 在溶解边缘区域产生 |
| **Spawn Volume** | AABB (-0.0087, 1.4014, -0.0185) 尺寸 (3.0, 4.0, 3.1) | 产生区域 |
| **Velocity** | 初始向量 (0.8, 0.4, -0.1) + 随机范围 (0.1, 1.2, 1.5) | 速度范围 |
| **Lifetime** | 随机 1-3 秒 | 生命周期 |
| **Size** | 随时间渐变：起点 (1.0) → 中点 (0.88) → 终点 (0.0) | 大小曲线 |
| **Color** | 渐变：起点 (1, 1, 1, 1) → 中 (12, 4.8, 0) → 终点 (0, 0, 0, 0) | 颜色渐变 |
| **Angular Velocity** | 范围 (0.2, 0.1, -0.1) → (0.3, 0.4, 2.0) | 角速度范围 |
| **Scale** | (0.2, 1.0, 1.0) | 整体缩放 |

### 3.2 SmokeVFX（烟雾粒子）

参数提取自 `SmokeVFX.vfx`:

| 参数 | 值 | 说明 |
|------|-----|------|
| **容量** | 5000 粒子 | 最大同时存在 |
| **Spawn Rate** | 160 粒子/秒 | 持续产生 |
| **Spawn Volume** | AABB (0.0164, -0.1317, -0.0315) 尺寸 (0.50, 0.046, 0.70) | 小范围集中在书上方 |
| **Velocity** | 初始 (-0.2, 0.1, -0.2) + 随机 (0.2, 0.3, 0.2) | 向上慢飘 |
| **Lifetime** | 随机 1-3 秒 |  |
| **Size** | 范围 0.5-0.7 |  |
| **SmokeColor** | 黑色 (0, 0, 0, 0) |  |

### 3.3 GPU Compute Shader 架构

使用 Vulkan Compute Pipeline 实现：

**三个阶段**:

1. **Init Pass** (`cs_particles_init.comp`)
   - 初始化新产生粒子的位置/速度/生命周期
   - 死亡粒子复用空间

2. **Update Pass** (`cs_particles_update.comp`)
   - 每帧更新所有存活粒子
   - 重力/风力模拟
   - 生命周期递减
   - 更新位置根据速度

3. **Render Pass** (`frag_particles_render.frag`)
   - 点精灵(Point Sprite)渲染
   -  Billboard 朝向相机
   - 混合模式: 透明累加

**存储结构**:
- 存储缓冲区 (SSBO) - 粒子结构体数组
- `binding=0`: 粒子输入 (位置+颜色+大小+生命周期)
- `binding=1`: 粒子输出 (更新后)
- `binding=2`: 统一参数 (时间+溶解进度+相机矩阵)

---

## 4. 音频播放

### 4.1 原始代码逻辑

`PlaySfxSound.cs`:
```csharp
- Awake(): 加载 AudioClip
- Start(): 播放燃烧音效，循环模式
- OnDestroy(): 停止
```

### 4.2 ShaderShowcase 集成方案

选择集成 **miniaudio** (公共领域，零编译依赖，单头文件):

- 头文件: `thirdparty/miniaudio.h` (已存在无需新增)
- 播放: `ma_device` 设备 → `ma_wav` 解码
- 音效文件: 从参考项目复制 `Assets/FireSound/*` 到 `assets/audio/`

---

## 5. 整体架构模块化设计

根据选择方案B（模块化），代码结构如下：

```
src/
├── app/
│   ├── VFXFireBookScene.h       # 主场景类
│   └── VFXFireBookScene.cpp     # 调度入口
├── render/
│   ├── BookMeshRenderer.h       # 3D书本渲染器
│   ├── BookMeshRenderer.cpp      # 加载网格+材质+纹理+Phong光照
│   ├── GPUParticleSystem.h      # GPU粒子系统基类
│   ├── GPUParticleSystem.cpp     # Compute管线分配+描述符
│   ├── BurningPagesEffect.h      # 纸屑粒子子类
│   ├── BurningPagesEffect.cpp
│   ├── SmokeEffect.h            # 烟雾粒子子类
│   ├── SmokeEffect.cpp
│   ├── DissolvePostProcess.h    # 溶解后处理
│   ├── DissolvePostProcess.cpp
│   └── AudioPlayer.h/cpp        # miniaudio 音频播放器
```

### 5.1 类职责划分

**VFXFireBookScene**:
- 初始化所有子组件
- 每帧更新时序控制
- 处理相机交互
- 最终合成输出到屏幕

**BookMeshRenderer**:
- 加载二进制网格数据
- 创建顶点缓冲区+索引缓冲区
- 加载8种材质对应的纹理
- 每一帧根据溶解进度渲染网格
- 输出到渲染目标纹理

**GPUParticleSystem<T>**:
- 通用基类，管理 Vulkan Compute 管线
- 处理描述符集分配
- 内存管理：粒子存活/死亡链表
- 子类实现具体参数

**DissolvePostProcess**:
- 接收BookMeshRenderer输出RT
- 执行溶解噪声后处理
- 输出最终渲染结果

**AudioPlayer**:
- 初始化miniaudio设备
- 加载WAV/MP3文件
- 开始循环播放
- 场景销毁时停止

---

## 6. 渲染管线时序

```
每帧:

1. Update DissolveAmount = (time * speed) mod 1.0
   → 0 = 未溶解，1 = 完全溶解

2. GPU Compute:
   a. BurningPages: Init new particles → Update particles
   b. Smoke: Init new particles → Update particles

3. CPU->GPU: 推送溶解参数到UBO

4. 渲染Pass:
   a. BookMeshRenderer 渲染3D书本到 RT_A
   b. DissolvePostProcess 处理 RT_A → RT_B (溶解裁切+边缘发光)
   c. 渲染粒子从GPU缓冲区到 RT_B
   d. 合成 RT_B 到屏幕

5. 音频继续循环播放（不受渲染影响）
```

---

## 7. 参数控制（UI滑动条）

按照项目惯例，统一使用 `uniformFloats[0..5]`:

| 索引 | 参数 | 范围 | 默认 |
|------|------|------|------|
| P0 | DissolveAmount | 0.0 - 1.0 | 0.5 |
| P1 | DissolveSpeed | 0.0 - 0.5 | 0.05 |
| P2 | EdgeWidth | 0.0 - 0.5 | 0.2 |
| P3 | ParticleScale | 0.1 - 2.0 | 1.0 |
| P4 | SmokeDensity | 0.0 - 1.0 | 1.0 |
| P5 | RotationSpeed | 0.0 - 2.0 | 0.1 |

*P1控制自动动画速度，0 = 手动通过P0控制*

---

## 8. 依赖项

| 依赖 | 状态 | 说明 |
|------|------|------|
| 书本二进制模型 | ✅ 已提取 | `assets/models/book/` |
| 书本纹理 | ✅ 已复制 | 8张PNG |
| miniaudio | ✅ 项目已有 | `thirdparty/miniaudio.h` |
| 噪声纹理 | 需要 | 需要从参考项目获取或程序化生成 |
| 音频文件 | 需要 | 需要从参考项目获取 |

---

## 9. 验收标准

1. ✅ 3D书本模型形状正确，所有子网格可见
2. ✅ 纹理对应正确，法线贴图生效
3. ✅ 溶解动画随P0变化正常，边缘发光正确
4. ✅ 纸屑粒子在溶解边缘产生，参数范围正确
5. ✅ 烟雾粒子持续产生，缓慢上升，混合透明
6. ✅ GPU计算，CPU占用低
7. ✅ 音频循环播放火焰燃烧音效
8. ✅ 所有参数滑动条生效
9. ✅ 参考项目视觉效果一致

---

## 10. 文件清单（新建/修改）

| 路径 | 操作 | 行数估算 |
|------|------|----------|
| `assets/models/book/` | 新建 | 已完成 |
| `assets/models/book/book_combined.bin` | 新建 | 100KB |
| `assets/models/book/book_mesh_info.json` | 新建 | 完成 |
| `assets/models/book/textures/` | 新建 | 8文件 完成 |
| `src/app/VFXFireBookScene.h` | 新建 | ~100 |
| `src/app/VFXFireBookScene.cpp` | 新建 | ~300 |
| `src/render/BookMeshRenderer.h` | 新建 | ~80 |
| `src/render/BookMeshRenderer.cpp` | 新建 | ~250 |
| `src/render/GPUParticleSystem.h` | 新建 | ~120 |
| `src/render/GPUParticleSystem.cpp` | 新建 | ~200 |
| `src/render/BurningPagesEffect.h` | 新建 | ~60 |
| `src/render/BurningPagesEffect.cpp` | 新建 | ~120 |
| `src/render/SmokeEffect.h` | 新建 | ~60 |
| `src/render/SmokeEffect.cpp` | 新建 | ~120 |
| `src/render/DissolvePostProcess.h` | 新建 | ~60 |
| `src/render/DissolvePostProcess.cpp` | 新建 | ~150 |
| `src/render/AudioPlayer.h` | 新建 | ~50 |
| `src/render/AudioPlayer.cpp` | 新建 | ~100 |
| `shaders/vfx_fire/book.vert` | 新建 | ~50 |
| `shaders/vfx_fire/book.frag` | 新建 | ~150 |
| `shaders/vfx_fire/dissolve.frag` | 新建 | ~100 |
| `shaders/vfx_fire/particles_init.comp` | 新建 | ~60 |
| `shaders/vfx_fire/particles_update.comp` | 新建 | ~80 |
| `shaders/vfx_fire/particles_render.frag` | 新建 | ~80 |
| `src/main.cpp` | 修改 | +5行注册场景 |
| `CMakeLists.txt` | 修改 | +新文件 |

**总计新建**: ~24文件，约2600行

---

更新: 2026-07-02
