#!/usr/bin/env python3
"""
素材下载脚本
从 Pexels/Pixabay 下载测试图片和视频

使用方法:
    python scripts/download_assets.py
"""

import os
import sys
import shutil
import requests
from pathlib import Path
from typing import Dict, List, Tuple, Optional

# 配置目录
ASSETS_DIR = Path("screenshots/assets")
IMAGES_DIR = ASSETS_DIR / "images"
VIDEOS_DIR = ASSETS_DIR / "videos"

# 本地备选视频路径
LOCAL_VIDEO_SOURCE = Path("shader-showcase/assets/test/test_video.mp4")

# 完整的 18 个效果的测试素材 - 使用 Pexels 免费素材
# 每个效果对应一张图片和一个视频
ASSETS: Dict[str, Dict[str, str]] = {
    "images": {
        # 00: Grayscale Test - 高对比风景
        "00_grayscale_landscape.jpg": "https://images.pexels.com/photos/1619317/pexels-photo-1619317.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 01: Bloom - 夜景灯光
        "01_bloom_citynight.jpg": "https://images.pexels.com/photos/1519088/pexels-photo-1519088.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 02: Gaussian Blur - 纹理细节
        "02_blur_brickwall.jpg": "https://images.pexels.com/photos/207300/pexels-photo-207300.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 03: Sharpen - 建筑细节
        "03_sharpen_architecture.jpg": "https://images.pexels.com/photos/1838640/pexels-photo-1838640.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 04: Edge Detection - 建筑轮廓
        "04_edge_building.jpg": "https://images.pexels.com/photos/302769/pexels-photo-302769.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 05: Emboss - 金属纹理
        "05_emboss_metal.jpg": "https://images.pexels.com/photos/952670/pexels-photo-952670.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 06: Pixelate - 人物肖像
        "06_pixelate_portrait.jpg": "https://images.pexels.com/photos/1239291/pexels-photo-1239291.jpeg?auto=compress&cs=tinysrgb&w=800&h=1200",
        # 07: Vignette - 中心构图风景
        "07_vignette_flower.jpg": "https://images.pexels.com/photos/1179229/pexels-photo-1179229.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 08: Chromatic Aberration - 高对比边缘
        "08_chromatic_leaves.jpg": "https://images.pexels.com/photos/1547813/pexels-photo-1547813.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 09: Color Grading - 色彩丰富场景
        "09_colorgrading_food.jpg": "https://images.pexels.com/photos/1099680/pexels-photo-1099680.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 10: Noise Generator - 纯色/天空
        "10_noise_sky.jpg": "https://images.pexels.com/photos/209807/pexels-photo-209807.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 11: Kaleidoscope - 对称图案
        "11_kaleidoscope_mandala.jpg": "https://images.pexels.com/photos/1191710/pexels-photo-1191710.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 12: Glitch Art - 科技/数字画面
        "12_glitch_tech.jpg": "https://images.pexels.com/photos/3862632/pexels-photo-3862632.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 13: Toon Shading - 卡通风格/鲜艳色彩
        "13_toon_cartoon.jpg": "https://images.pexels.com/photos/1049622/pexels-photo-1049622.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 14: VHS Retro - 复古/怀旧场景
        "14_vhs_retro.jpg": "https://images.pexels.com/photos/33109/fall-autumn-red-season.jpg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 15: CRT Monitor - 屏幕/像素画面
        "15_crt_screen.jpg": "https://images.pexels.com/photos/1779487/pexels-photo-1779487.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 16: Water Ripple - 水面/倒影
        "16_water_lake.jpg": "https://images.pexels.com/photos/132037/pexels-photo-132037.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
        # 17: Lens Distortion - 广角建筑
        "17_lens_wideangle.jpg": "https://images.pexels.com/photos/2255935/pexels-photo-2255935.jpeg?auto=compress&cs=tinysrgb&w=1920&h=1080",
    },
    "videos": {
        # 00: Grayscale Test
        "00_grayscale.mp4": "https://videos.pexels.com/video-files/857251/857251-hd_1920_1080_25fps.mp4",
        # 01: Bloom
        "01_bloom.mp4": "https://videos.pexels.com/video-files/3129671/3129671-hd_1920_1080_30fps.mp4",
        # 02: Gaussian Blur
        "02_blur.mp4": "https://videos.pexels.com/video-files/855029/855029-hd_1920_1080_30fps.mp4",
        # 03: Sharpen
        "03_sharpen.mp4": "https://videos.pexels.com/video-files/2022395/2022395-hd_1920_1080_30fps.mp4",
        # 04: Edge Detection
        "04_edge.mp4": "https://videos.pexels.com/video-files/3141207/3141207-hd_1920_1080_24fps.mp4",
        # 05: Emboss
        "05_emboss.mp4": "https://videos.pexels.com/video-files/2795748/2795748-hd_1920_1080_25fps.mp4",
        # 06: Pixelate
        "06_pixelate.mp4": "https://videos.pexels.com/video-files/3209828/3209828-hd_1920_1080_30fps.mp4",
        # 07: Vignette
        "07_vignette.mp4": "https://videos.pexels.com/video-files/1536322/1536322-hd_1920_1080_30fps.mp4",
        # 08: Chromatic Aberration
        "08_chromatic.mp4": "https://videos.pexels.com/video-files/3571264/3571264-hd_1920_1080_30fps.mp4",
        # 09: Color Grading
        "09_color.mp4": "https://videos.pexels.com/video-files/4092757/4092757-hd_1920_1080_30fps.mp4",
        # 10: Noise Generator
        "10_noise.mp4": "https://videos.pexels.com/video-files/2792370/2792370-hd_1920_1080_25fps.mp4",
        # 11: Kaleidoscope
        "11_kaleidoscope.mp4": "https://videos.pexels.com/video-files/3141207/3141207-hd_1920_1080_24fps.mp4",
        # 12: Glitch Art
        "12_glitch.mp4": "https://videos.pexels.com/video-files/3121459/3121459-hd_1920_1080_25fps.mp4",
        # 13: Toon Shading
        "13_toon.mp4": "https://videos.pexels.com/video-files/1644473/1644473-hd_1920_1080_30fps.mp4",
        # 14: VHS Retro
        "14_vhs.mp4": "https://videos.pexels.com/video-files/2792370/2792370-hd_1920_1080_25fps.mp4",
        # 15: CRT Monitor
        "15_crt.mp4": "https://videos.pexels.com/video-files/3121459/3121459-hd_1920_1080_25fps.mp4",
        # 16: Water Ripple
        "16_water.mp4": "https://videos.pexels.com/video-files/1536322/1536322-hd_1920_1080_30fps.mp4",
        # 17: Lens Distortion
        "17_lens.mp4": "https://videos.pexels.com/video-files/2022395/2022395-hd_1920_1080_30fps.mp4",
    }
}

# HTTP 请求头
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "Accept-Language": "en-US,en;q=0.5",
    "Accept-Encoding": "gzip, deflate, br",
    "Connection": "keep-alive",
    "Upgrade-Insecure-Requests": "1",
}


def format_size(size_bytes: int) -> str:
    """将字节大小转换为可读格式"""
    if size_bytes < 1024:
        return f"{size_bytes} B"
    elif size_bytes < 1024 * 1024:
        return f"{size_bytes / 1024:.1f} KB"
    else:
        return f"{size_bytes / (1024 * 1024):.1f} MB"


def download_file(url: str, dest: Path, timeout: int = 120) -> bool:
    """
    下载文件到指定路径

    Args:
        url: 文件URL
        dest: 目标路径
        timeout: 下载超时时间(秒)

    Returns:
        下载成功返回True，否则返回False
    """
    try:
        # 检查文件是否已存在
        if dest.exists():
            print(f"  [跳过] 文件已存在: {dest.name}")
            return True

        print(f"  [下载] {dest.name} ...", end=" ", flush=True)

        # 发送请求
        response = requests.get(url, headers=HEADERS, timeout=timeout, stream=True)
        response.raise_for_status()

        # 获取文件大小
        total_size = int(response.headers.get('content-length', 0))
        downloaded = 0
        last_percent = -1

        # 确保目标目录存在
        dest.parent.mkdir(parents=True, exist_ok=True)

        # 下载文件并显示进度
        with open(dest, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)
                    downloaded += len(chunk)
                    if total_size > 0:
                        percent = int((downloaded / total_size) * 100)
                        if percent != last_percent and percent % 10 == 0:
                            print(f"{percent}%", end=" ", flush=True)
                            last_percent = percent

        file_size = dest.stat().st_size
        print(f"\r  [成功] {dest.name} ({format_size(file_size)})")
        return True

    except requests.exceptions.Timeout:
        print(f"\r  [失败] {dest.name} (超时)")
        if dest.exists():
            dest.unlink()
        return False
    except requests.exceptions.RequestException as e:
        print(f"\r  [失败] {dest.name} ({str(e)})")
        if dest.exists():
            dest.unlink()
        return False
    except Exception as e:
        print(f"\r  [失败] {dest.name} ({str(e)})")
        if dest.exists():
            dest.unlink()
        return False


def create_directories() -> bool:
    """创建必要的目录结构"""
    try:
        IMAGES_DIR.mkdir(parents=True, exist_ok=True)
        VIDEOS_DIR.mkdir(parents=True, exist_ok=True)
        return True
    except Exception as e:
        print(f"[错误] 创建目录失败: {e}")
        return False


def copy_local_video(dest: Path) -> bool:
    """
    从本地复制视频作为备选

    Args:
        dest: 目标路径

    Returns:
        复制成功返回True，否则返回False
    """
    try:
        if not LOCAL_VIDEO_SOURCE.exists():
            return False

        if dest.exists():
            print(f"  [跳过] 文件已存在: {dest.name}")
            return True

        print(f"  [本地复制] {dest.name} ...", end=" ", flush=True)
        shutil.copy2(LOCAL_VIDEO_SOURCE, dest)
        file_size = dest.stat().st_size
        print(f"\r  [成功] {dest.name} ({format_size(file_size)}) [本地]")
        return True
    except Exception as e:
        print(f"\r  [失败] {dest.name} (本地复制: {str(e)})")
        return False


def download_assets(asset_type: str, assets_dict: Dict[str, str], dest_dir: Path, use_local_fallback: bool = False) -> Tuple[int, int]:
    """
    下载指定类型的素材

    Args:
        asset_type: 素材类型名称
        assets_dict: 素材URL字典
        dest_dir: 目标目录
        use_local_fallback: 下载失败时是否使用本地备选

    Returns:
        (成功数量, 总数)
    """
    success_count = 0
    total_count = len(assets_dict)

    if total_count == 0:
        return 0, 0

    print(f"\n{'='*60}")
    print(f"下载 {asset_type} ({total_count} 个文件)")
    print(f"{'='*60}")

    for filename, url in assets_dict.items():
        dest_path = dest_dir / filename
        if download_file(url, dest_path):
            success_count += 1
        elif use_local_fallback:
            # 下载失败，尝试本地复制
            if copy_local_video(dest_path):
                success_count += 1

    return success_count, total_count


def print_summary(image_success: int, image_total: int, video_success: int, video_total: int):
    """打印下载摘要"""
    print(f"\n{'='*60}")
    print("下载完成摘要")
    print(f"{'='*60}")
    print(f"图片: {image_success}/{image_total} 成功")
    print(f"视频: {video_success}/{video_total} 成功")
    print(f"总计: {image_success + video_success}/{image_total + video_total} 成功")

    # 显示下载的文件
    print(f"\n下载目录:")
    print(f"  图片: {IMAGES_DIR.absolute()}")
    print(f"  视频: {VIDEOS_DIR.absolute()}")


def main():
    """主函数"""
    print("="*60)
    print("Shader Showcase 素材下载工具")
    print("="*60)
    print(f"目标目录: {ASSETS_DIR.absolute()}")
    print(f"图片数量: {len(ASSETS['images'])}")
    print(f"视频数量: {len(ASSETS['videos'])}")

    # 创建目录
    if not create_directories():
        sys.exit(1)

    # 下载图片
    image_success, image_total = download_assets("图片", ASSETS["images"], IMAGES_DIR)

    # 下载视频 (使用本地备选)
    video_success, video_total = download_assets("视频", ASSETS["videos"], VIDEOS_DIR, use_local_fallback=True)

    # 打印摘要
    print_summary(image_success, image_total, video_success, video_total)

    # 返回退出码
    total_success = image_success + video_success
    total_count = image_total + video_total

    if total_success == total_count:
        print("\n[完成] 所有素材下载成功!")
        sys.exit(0)
    elif total_success > 0:
        print("\n[警告] 部分素材下载失败，但可以使用已下载的素材")
        sys.exit(0)  # 部分成功也返回0，因为可以继续
    else:
        print("\n[错误] 所有素材下载失败")
        sys.exit(2)


if __name__ == "__main__":
    main()
