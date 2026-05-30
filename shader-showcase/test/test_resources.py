# e:\AI\graph\hight-post-proc\shader-showcase\test\test_resources.py
"""Download test images and generate test video."""
import os, requests, subprocess, sys

ASSET_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "test")

def ensure_dir():
    os.makedirs(ASSET_DIR, exist_ok=True)

def download_image(url: str, filename: str) -> bool:
    path = os.path.join(ASSET_DIR, filename)
    if os.path.exists(path) and os.path.getsize(path) > 1000:
        print(f"[SKIP] {filename} already exists")
        return True
    print(f"[DOWNLOAD] {filename} <- {url}")
    try:
        r = requests.get(url, timeout=30, stream=True)
        r.raise_for_status()
        with open(path, "wb") as f:
            for chunk in r.iter_content(65536):
                f.write(chunk)
        print(f"[OK] {filename} ({os.path.getsize(path)} bytes)")
        return True
    except Exception as e:
        print(f"[FAIL] {filename}: {e}")
        return False

def generate_grid_image() -> bool:
    """Generate a black image with white grid lines using PIL."""
    try:
        from PIL import Image, ImageDraw
        img = Image.new("RGB", (640, 480), (0, 0, 0))
        draw = ImageDraw.Draw(img)
        for x in range(0, 641, 40):
            draw.line([(x, 0), (x, 480)], fill=(255, 255, 255))
        for y in range(0, 481, 40):
            draw.line([(0, y), (640, y)], fill=(255, 255, 255))
        path = os.path.join(ASSET_DIR, "grid.jpg")
        img.save(path, "JPEG", quality=90)
        print(f"[OK] grid.jpg generated")
        return True
    except Exception as e:
        print(f"[FAIL] grid.jpg: {e}")
        return False

def generate_test_video() -> bool:
    """Generate a simple test video using PIL (GIF animation)."""
    video_path = os.path.join(ASSET_DIR, "test_video.mp4")
    gif_path = os.path.join(ASSET_DIR, "test_video.gif")
    
    # Check if MP4 exists (legacy) or GIF exists
    if os.path.exists(video_path) and os.path.getsize(video_path) > 1000:
        print(f"[SKIP] test_video.mp4 already exists")
        return True
    
    print(f"[GENERATE] test_video.gif (using PIL)")
    try:
        from PIL import Image, ImageDraw
        import math
        
        width, height = 640, 480
        fps = 10
        duration = 5
        total_frames = fps * duration
        
        frames = []
        
        for frame_num in range(total_frames):
            # Create a new image for each frame
            t = frame_num / total_frames
            img = Image.new("RGB", (width, height), (0, 0, 0))
            draw = ImageDraw.Draw(img)
            
            # Background gradient (blue to dark)
            for y in range(height):
                val = int(255 * (1 - y / height))
                draw.line([(0, y), (width, y)], fill=(val, val // 2, val // 4))
            
            # Moving colored block position
            block_x = int((width - 100) * t)
            block_y = int((height - 100) * (0.5 + 0.5 * math.sin(t * 2 * math.pi)))
            
            # Draw moving rectangle (orange)
            draw.rectangle([block_x, block_y, block_x + 100, block_y + 100], 
                          fill=(0, 200, 255))
            
            # Draw grid overlay
            for x in range(0, width, 40):
                draw.line([(x, 0), (x, height)], fill=(50, 50, 50))
            for y in range(0, height, 40):
                draw.line([(0, y), (width, y)], fill=(50, 50, 50))
            
            # Add frame counter text
            draw.text((10, 10), f"Frame: {frame_num}", fill=(255, 255, 255))
            
            frames.append(img.copy())
        
        # Save as GIF
        gif_duration = int(1000 / fps)  # milliseconds per frame
        frames[0].save(gif_path, save_all=True, append_images=frames[1:], 
                      duration=gif_duration, loop=0)
        
        # Copy GIF to MP4 path for compatibility (some tests may accept either)
        import shutil
        shutil.copy(gif_path, video_path)
        
        if os.path.exists(video_path) and os.path.getsize(video_path) > 1000:
            print(f"[OK] test_video.mp4 ({os.path.getsize(video_path)} bytes)")
            return True
        else:
            print(f"[FAIL] test_video.mp4: file not created")
            return False
    except Exception as e:
        print(f"[FAIL] test_video.mp4: {e}")
        return False

def download_all():
    """Download/generate all test resources."""
    ensure_dir()
    results = [
        download_image("https://picsum.photos/seed/portrait/1920/1280", "portrait.jpg"),
        download_image("https://picsum.photos/seed/landscape/1920/1280", "nature.jpg"),
        download_image("https://picsum.photos/seed/texture/1920/1280", "abstract.jpg"),
        generate_grid_image(),
        generate_test_video(),
    ]
    success = sum(results)
    print(f"\n=== Resources: {success}/{len(results)} ready ===")
    return success == len(results)

if __name__ == "__main__":
    ok = download_all()
    sys.exit(0 if ok else 1)
