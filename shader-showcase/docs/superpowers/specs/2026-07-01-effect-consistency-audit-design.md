# 效果一致性审计与修复流程设计

> 日期：2026-07-01 | 状态：已批准

## 1. 背景

ShaderShowcase 项目包含 45 个 AUS3D 效果（8 个诊断 shader + 37 个实际效果），参考实现为 Awesome-Unity-Shader 项目（16 个 Volume，66 个 `.shader` 文件）。历史 session 已修复大量问题（光照模型、SPV 同步、纹理绑定），但用户反馈当前效果与参考项目相比仍存在明显视觉差异。

## 2. 目标

建立系统性的"对比→记录→逐个修复→验证"流程，确保每个效果与参考项目在算法层面一致。

## 3. 范围

- **包含**：索引 8-44 的 37 个实际效果（排除诊断 shader 0-7）
- **不包含**：诊断 shader（仅验证管线，不参与效果对比）
- **参考标准**：`e:\AI\test\Awesome-Unity-Shader` 中的 `.shader` 源文件为唯一参考实现

## 4. 三阶段流程

### Phase 1 — 差异表建立

**产出**：`docs/effect-diff-table.md`

**方法**：
1. 逐个读取参考 `.shader` 源码，推导渲染画面特征（不运行 Unity）：
   - 光照模型类型（Lambert / Half-Lambert / Blinn-Phong / 顶点光照 / 无光照）
   - 颜色计算公式（diffuse + ambient + specular 的组合方式）
   - 纹理混合方式（multiply / add / double / blend / alpha）
   - 特殊效果（rim、alpha test、cubemap、fresnel 等）
   - 预期画面特征（球体中心亮度、边缘颜色、棋盘格/渐变分布等）
2. 读取当前 `.frag` 文件，提取相同算法特征
3. 逐项对比，记录差异到 markdown 表格

**差异等级定义**：
- **高**：光照模型完全错误，或公式结构错误
- **中**：光照模型正确但参数默认值/系数不同，或缺少纹理混合步骤
- **低**：算法正确但细微常量差异（如 ambient 系数 0.1 vs 0.15）
- **无**：算法完全一致

**表结构**：

| 列 | 说明 |
|---|---|
| 索引 | 效果编号 |
| 名称 | 效果名称 |
| 参考Shader | Awesome-Unity-Shader 源文件路径 |
| 当前Frag | ShaderShowcase .frag 文件路径 |
| 参考光照 | 参考源码光照类型 |
| 当前光照 | 当前 .frag 光照类型 |
| 参考公式 | 参考源码颜色计算方式 |
| 当前公式 | 当前 .frag 颜色计算方式 |
| 预期特征 | 从参考推导的预期画面特征 |
| 差异等级 | 高/中/低/无 |
| 修复优先级 | 1-5（1 最高） |

### Phase 2 — 逐个修复（含确认门禁）

**修复顺序**：按差异等级从高到低排列。

**每个效果的修复流程**：

```
1. 从差异表中读取该效果的差异记录
2. 向用户展示：
   - 效果名称和索引
   - 参考 shader vs 当前 frag 的具体差异点
   - 计划修改方案（具体到哪几行、改什么）
   - 预期修复后的画面特征变化
3. 等待用户确认
4. 用户确认后，修改 .frag 文件
5. 编译 cmake --build
6. 同步 SPV：build/shaders/ → shaders/
7. 运行该效果截图，验证中心像素是否符合预期特征
8. 展示修复前后像素对比
9. 进入下一个效果
```

**安全措施**：
- 每次只改一个 `.frag` 文件
- 修复前 git stash 当前状态，修复后如果验证失败可回滚
- 修复完成后运行全量回归测试，确认其他效果不受影响

### Phase 3 — 全量回归测试

全部修复完成后，运行完整 45 效果自动化截图测试，确认无回归。

## 5. 约束

- 所有效果必须与 Awesome-Unity-Shader 对应 Volume 完全一致（算法、参数名、默认值、范围）
- 修复前必须向用户展示差异点和修改方案，等待确认后才能执行
- 每次只修复一个效果，完成后立即验证
- 不可影响已正常工作的效果

## 6. 关键文件

| 角色 | 路径 |
|------|------|
| 差异表 | `docs/effect-diff-table.md`（Phase 1 产出） |
| 当前着色器 | `shaders/aus3d/*.frag` |
| 编译输出 | `build/shaders/aus3d/*.frag.spv` |
| 运行时SPV | `shaders/aus3d/*.frag.spv`（从 build 同步） |
| 效果定义 | `src/app/AUS3DScene.cpp` BuildEffects() |
| 参考源码 | `e:\AI\test\Awesome-Unity-Shader\Volume *\*.shader` |
| 测试输出 | `screenshots/card_aus3d_*.ppm` |
| 自动化测试 | `test/test_automated.py` |