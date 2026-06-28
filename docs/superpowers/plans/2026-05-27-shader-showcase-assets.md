# Shader Showcase 素材优化计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 18 个 Shader 效果下载针对性的测试图片和视频，重新生成截图

**架构：** 使用 Pexels/Pixabay 免费图库，按效果类型匹配最适合的素材，下载后重新运行 ShaderShowcase 生成新截图

**技术栈：** Python + requests + PIL + ShaderShowcase 程序

---

## 文件结构

```
screenshots/
├── README.md                    # 更新索引
├── assets/                      # 测试素材
│   ├── images/                  # 测试图片
│   │   ├── textured_wall.jpg    # 模糊/锐化
│   │   ├── architecture.jpg     # 边缘检测
│   │   ├── colorful_flowers.jpg # 卡通着色
│   │   └── ... (18张)
│   └── videos/                  # 测试视频
│       ├── crowd_walking.mp4    # 动态效果
│       ├── city_night.mp4       # 光晕/噪点
│       └── ... (18个)
└── (重新生成的截图)
```

---

## 素材匹配表

| 序号 | 效果 | 图片特征 | 视频特征 |
|------|------|----------|----------|
| 00 | Grayscale Test | 高对比风景 | 风景延时 |
| 01 | Bloom | 夜景灯光 | 城市夜景 |
| 02 | Gaussian Blur | 纹理细节（砖墙/树叶）| 静态纹理 |
| 03 | Sharpen | 建筑/文字 | 建筑视频 |
| 04 | Edge Detection | 建筑轮廓 | 建筑轮廓 |
| 05 | Emboss | 金属/浮雕纹理 | 纹理视频 |
| 06 | Pixelate | 人物肖像 | 人物视频 |
| 07 | Vignette | 中心构图风景 | 风景视频 |
| 08 | Chromatic Aberration | 高对比边缘 | 运动视频 |
| 09 | Color Grading | 色彩丰富场景 | 色彩视频 |
| 10 | Noise Generator | 纯色/天空 | 纯色背景 |
| 11 | Kaleidoscope | 对称图案 | 灯光视频 |
| 12 | Glitch Art | 科技/数字画面 | 数字画面 |
| 13 | Toon Shading | 卡通风格/鲜艳色彩 | 动画视频 |
| 14 | VHS Retro | 复古/怀旧场景 | 复古视频 |
| 15 | CRT Monitor | 屏幕/像素画面 | 屏幕录制 |
| 16 | Water Ripple | 水面/倒影 | 水面视频 |
| 17 | Lens Distortion | 广角建筑 | 广角视频 |

---

## Task 1: 创建素材下载脚本

**Files:**
- Create: `scripts/download_assets.py`

- [ ] **Step 1: 创建下载脚本框架**

```python
#!/usr/bin/env python3
"""下载 Shader Showcase 测试素材"""

import os
import requests
from pathlib import Path

ASSETS_DIR = Path("screenshots/assets")
IMAGES_DIR = ASSETS_DIR / "images"
VIDEOS_DIR = ASSETS_DIR / "videos"

# 素材配置
ASSETS = {
    "images": {
        "textured_wall": "https://images.pexels.com/photos/207300/pexels-photo-207300.jpeg",
        "architecture": "https://images.pexels.com/photos/1838640/pexels-photo-1838640.jpeg",
        "colorful_flowers": "https://images.pexels.com/photos/1179229/pexels-photo-1179229.jpeg",
        # ... 更多图片 URL
    },
    "videos": {
        "city_night": "https://videos.pexels.com/video-files/3129671/3129671-uhd_2560_1440_30fps.mp4",
        "crowd_walking": "https://videos.pexels.com/video-files/855029/855029-hd_1920_1080_30fps.mp4",
        # ... 更多视频 URL
    }
}

def download_file(url: str, dest: Path) -> bool:
    """下载文件到指定路径"""
    try:
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
        }
        response = requests.get(url, headers=headers, timeout=30)
        response.raise_for_status()
        dest.write_bytes(response.content)
        print(f"✓ Downloaded: {dest.name}")
        return True
    except Exception as e:
        print(f"✗ Failed: {dest.name} - {e}")
        return False

def main():
    """主函数"""
    # 创建目录
    IMAGES_DIR.mkdir(parents=True, exist_ok=True)
    VIDEOS_DIR.mkdir(parents=True, exist_ok=True)
    
    # 下载图片
    print("Downloading images...")
    for name, url in ASSETS["images"].items():
        ext = url.split('.')[-1].split('?')[0] or 'jpg'
        dest = IMAGES_DIR / f"{name}.{ext}"
        if not dest.exists():
            download_file(url, dest)
    
    # 下载视频
    print("\nDownloading videos...")
    for name, url in ASSETS["videos"].items():
        ext = url.split('.')[-1].split('?')[0] or 'mp4'
        dest = VIDEOS_DIR / f"{name}.{ext}"
        if not dest.exists():
            download_file(url, dest)
    
    print("\nDone!")

if __name__ == "__main__":
    main()
```

- [ ] **Step 2: 运行脚本测试**

```bash
cd e:/AI/graph/hight-post-proc
python scripts/download_assets.py
```

Expected: 成功下载测试图片到 screenshots/assets/

- [ ] **Step 3: 提交**

```bash
git add scripts/download_assets.py
git commit -m "feat: add asset download script"
```

---

## Task 2: 收集完整素材 URL

**Files:**
- Modify: `scripts/download_assets.py` (补充完整 URL 列表)

- [ ] **Step 1: 搜索并收集 18 张图片 URL**

访问以下网站，为每个效果找到最合适的图片：
- https://www.pexels.com
- https://unsplash.com
- https://pixabay.com

搜索关键词：
- textured wall, brick wall, leaf texture
- architecture silhouette, building outline
- colorful flowers, vibrant colors
- night city, city lights
- portrait, face
- water reflection, lake
- retro, vintage
- etc.

- [ ] **Step 2: 搜索并收集 18 个视频 URL**

访问：
- https://www.pexels.com/videos/
- https://pixabay.com/videos/

搜索关键词同上，选择 5-10 秒的短视频。

- [ ] **Step 3: 更新脚本中的 URL 列表**

将收集到的 URL 填入 `ASSETS` 字典。

- [ ] **Step 4: 运行完整下载**

```bash
python scripts/download_assets.py
```

Expected: 下载 18 张图片 + 18 个视频到 assets/ 目录

- [ ] **Step 5: 提交**

```bash
git add scripts/download_assets.py
git commit -m "feat: add complete asset URLs and download all assets"
```

---

## Task 3: 修改 ShaderShowcase 使用新素材

**Files:**
- Modify: `shader-showcase/src/main.cpp` (修改缩略图渲染逻辑)
- Modify: `shader-showcase/src/app/EffectDetailScene.cpp` (修改详情页输入)

- [ ] **Step 1: 修改 main.cpp 加载不同图片**

找到缩略图渲染循环，改为根据效果索引加载对应的测试图片：

```cpp
// 在 main.cpp 中，缩略图渲染部分
std::vector<std::string> testImages = {
    "assets/images/landscape.jpg",      // 00: Grayscale
    "assets/images/city_night.jpg",     // 01: Bloom
    "assets/images/textured_wall.jpg",  // 02: Gaussian Blur
    // ... 18 张图片
};

for (int i = 0; i < NUM_EFFECTS; i++) {
    // 加载对应的测试图片
    TextureHandle testTex = LoadTexture(testImages[i]);
    // ... 渲染缩略图
}
```

- [ ] **Step 2: 修改 EffectDetailScene 支持视频输入**

添加视频帧提取逻辑，在详情页播放视频效果：

```cpp
// 在 EffectDetailScene 中添加视频支持
class EffectDetailScene {
    // ...
    void LoadVideo(const std::string& videoPath);
    void UpdateVideoFrame();  // 每帧更新视频画面
    // ...
};
```

- [ ] **Step 3: 重新编译程序**

```bash
cd shader-showcase/build
cmake --build . --config Release
```

- [ ] **Step 4: 提交**

```bash
git add shader-showcase/src/main.cpp shader-showcase/src/app/EffectDetailScene.cpp
git commit -m "feat: use specific assets for each effect, add video support"
```

---

## Task 4: 重新生成所有截图

**Files:**
- 输出到: `screenshots/` 目录

- [ ] **Step 1: 运行卡片截图生成**

```bash
cd shader-showcase/build/bin/Release
set AUTO_TEST_CARDS=1
./ShaderShowcase.exe
```

Expected: 生成 18 张新的卡片截图

- [ ] **Step 2: 运行详情页截图生成**

```bash
set AUTO_TEST_DETAILS=1
./ShaderShowcase.exe
```

Expected: 生成 18 张新的详情页截图

- [ ] **Step 3: 转换为 JPEG 格式（手机兼容）**

```bash
python -c "
from PIL import Image
import glob
import os

for png in glob.glob('screenshots/*.png'):
    img = Image.open(png)
    jpg = png.replace('.png', '.jpg')
    img.convert('RGB').save(jpg, 'JPEG', quality=95)
    print(f'Converted: {os.path.basename(jpg)}')
"
```

- [ ] **Step 4: 更新 README.md**

更新产物清单，列出所有新截图和对应的素材来源。

- [ ] **Step 5: 提交**

```bash
git add screenshots/
git commit -m "feat: regenerate all screenshots with new assets"
```

---

## 验证清单

- [ ] 18 张测试图片已下载到 screenshots/assets/images/
- [ ] 18 个测试视频已下载到 screenshots/assets/videos/
- [ ] 新的卡片截图已生成（18 张）
- [ ] 新的详情页截图已生成（18 张）
- [ ] JPEG 版本已生成（手机兼容）
- [ ] README.md 已更新

---

## 注意事项

1. **版权问题**：只使用 Pexels/Pixabay/Unsplash 等明确标注免费可商用的素材
2. **文件大小**：视频文件可能较大，选择 720p 或更低分辨率以节省空间
3. **命名规范**：使用英文小写 + 下划线命名，如 `city_night.jpg`
4. **备份原图**：如需保留原截图，先备份到 `screenshots/backup/` 目录
