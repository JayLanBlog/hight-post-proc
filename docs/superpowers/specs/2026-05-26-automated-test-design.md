# Shader Showcase 自动化测试方案

## 1. 概述

本项目为 Shader Showcase 应用编写自动化测试脚本，通过无头 GUI 自动化（PyAutoGUI）模拟用户操作，验证所有效果的渲染、交互和场景切换功能。

## 2. 测试环境

| 组件 | 版本要求 |
|------|---------|
| Python | 3.8+ |
| pyautogui | 最新版 |
| pywinauto | 最新版 |
| Pillow | 最新版 |
| imagehash | 最新版 |
| pytest | 最新版 |
| requests | 最新版 |
| ffmpeg | 在 PATH 中 |

## 3. 测试资源

### 3.1 测试图片（自动下载到 `assets/test/`）

| 文件名 | 来源 | 用途 |
|--------|------|------|
| portrait.jpg | picsum.photos/seed/portrait/1920/1280 | 人像图 |
| nature.jpg | picsum.photos/seed/landscape/1920/1280 | 风景图 |
| abstract.jpg | picsum.photos/seed/texture/1920/1280 | 抽象纹理 |
| grid.jpg | 程序生成（黑色背景+白色网格线） | 网格图（检测扭曲效果） |

### 3.2 测试视频（自动生成到 `assets/test/`）

| 文件名 | 参数 | 用途 |
|--------|------|------|
| test_video.mp4 | 640x480, 30fps, 5秒, 移动色块 | 视频播放测试 |

## 4. 测试用例

### 4.1 启动测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-01 | 程序启动 | 运行 ShaderShowcase.exe | 窗口出现，缩略图渲染完成，无崩溃 |

### 4.2 缩略图渲染测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-10 | 缩略图非空 | 启动后截图 | 所有18个缩略图非纯色/纯黑 |
| TC-11 | 缩略图可见 | 截图对比 | 每个缩略图宽度约256px |

### 4.3 卡片交互测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-20 | 鼠标滚轮翻页 | scroll(-3) | 卡片索引增加3 |
| TC-21 | 滚轮向上 | scroll(3) | 卡片索引减少3 |
| TC-22 | F1快捷键 | press('f1') | 选中第1张卡片 |
| TC-23 | F8快捷键 | press('f8') | 选中第8张卡片 |
| TC-24 | 左箭头键 | press('left') | 卡片索引-1 |
| TC-25 | 右箭头键 | press('right') | 卡片索引+1 |

### 4.4 详情页测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-30 | 点击卡片打开详情 | click center | 详情页打开，无崩溃 |
| TC-31 | ESC返回CoverFlow | press('esc') | 返回CoverFlow，缩略图完整 |
| TC-32 | Tab显示参数面板 | press('tab') | DebugPanel 显示 |
| TC-33 | 详情页动画 | 停留5秒 | shader动画持续（非静态） |
| TC-34 | 拖放图片切换输入 | 拖入 test.jpg | 输入图片切换 |
| TC-35 | 18个效果逐一测试 | 依次点击每个效果 | 每个效果详情页正常打开/返回 |

### 4.5 图片池测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-40 | Ctrl+右切换图片 | Ctrl+right | 图片切换到下一张 |
| TC-41 | Ctrl+左切换图片 | Ctrl+left | 图片切换到上一张 |

### 4.6 视频播放测试

| ID | 用例名称 | 操作 | 预期结果 |
|----|---------|------|---------|
| TC-50 | 拖放视频播放 | 拖入 test_video.mp4 | 视频帧持续更新 |

## 5. 判定标准

- 所有 TC-** 通过 → 测试通过（PASS）
- 任何 TC-** 失败 → 记录日志 → 尝试自动修复 → 如修复失败则报告 FAIL

## 6. 自动化修复策略

如果测试发现崩溃或错误，脚本按以下顺序尝试修复：

1. **崩溃在详情页打开时**：检查 `EffectDetailScene::GetNextScene()` 是否正确恢复状态
2. **缩略图为空**：检查 `SetThumbnails()` 调用和 `m_thumbIds` 是否为空
3. **Shader 编译错误**：检查 SPIR-V 文件是否存在
4. **拖放不工作**：检查 `DropCallback` 是否被调用

## 7. 输出

- 测试报告：`test_report.html`
- 截图证据：`test_screenshots/` 目录
- 资源目录：`assets/test/`

## 8. 文件结构

```
shader-showcase/
├── test_automated.py          # 主测试脚本
├── test_screenshots/          # 测试截图
└── assets/test/               # 测试资源
    ├── portrait.jpg
    ├── nature.jpg
    ├── abstract.jpg
    ├── grid.jpg
    └── test_video.mp4
```
