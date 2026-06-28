# XPL Full Migration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate 56 remaining X-PostProcessing-Library effects to ShaderShowcase via automated HLSL→GLSL conversion with full integration.

**Architecture:** Python migration script reads XPL `.shader`+`.cs` → generates `.frag` (GLSL 460 + std140 Params UBO) + `effect.json` → updates CMake, CoverFlowScene, LanguageManager. Multi-pass effects generate multi-frag files. Verify via compile+auto-screenshot.

**Tech Stack:** Python 3, GLSL 460, CMake, C++17, PowerShell

---

### Task 0: Prepare Migration Mapping Table

**Files:**
- Create: `c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_map.py`

- [ ] **Step 1: Create the full mapping table**

```python
"""Full XPL effect mapping table: XPL directory → ShaderShowcase ID, Chinese name, category."""
MIGRATIONS = [
    # ===== Blur (17) =====
    {"xpl_dir": "BokehBlur",             "id": "xpl_blur_bokeh",             "name_cn": "散景模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "BoxBlur",               "id": "xpl_blur_box",               "name_cn": "方框模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "DirectionalBlur",       "id": "xpl_blur_directional",       "name_cn": "定向模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "DualBoxBlur",           "id": "xpl_blur_dual_box",          "name_cn": "双重方框模糊",    "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},
    {"xpl_dir": "DualGaussianBlur",      "id": "xpl_blur_dual_gaussian",     "name_cn": "双重高斯模糊",    "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},
    {"xpl_dir": "DualKawaseBlur",        "id": "xpl_blur_dual_kawase",       "name_cn": "双重Kawase模糊",  "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},
    {"xpl_dir": "DualTentBlur",          "id": "xpl_blur_dual_tent",         "name_cn": "双重帐篷模糊",    "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},
    {"xpl_dir": "GaussianBlur",          "id": "xpl_blur_gaussian",          "name_cn": "高斯模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "GrainyBlur",            "id": "xpl_blur_grainy",            "name_cn": "颗粒模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "IrisBlur",              "id": "xpl_blur_iris",              "name_cn": "光圈模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "IrisBlurV2",            "id": "xpl_blur_iris_v2",           "name_cn": "光圈模糊 V2",     "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "KawaseBlur",            "id": "xpl_blur_kawase",            "name_cn": "Kawase 模糊",     "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "RadialBlur",            "id": "xpl_blur_radial",            "name_cn": "径向模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "RadialBlurV2",          "id": "xpl_blur_radial_v2",         "name_cn": "径向模糊 V2",     "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "TentBlur",              "id": "xpl_blur_tent",              "name_cn": "帐篷模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": False},
    {"xpl_dir": "TiltShiftBlur",         "id": "xpl_blur_tilt_shift",        "name_cn": "移轴模糊",        "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},
    {"xpl_dir": "TiltShiftBlurV2",       "id": "xpl_blur_tilt_shift_v2",     "name_cn": "移轴模糊 V2",     "cat": "Blur Effects",         "cat_cn": "模糊效果",    "multi": True},

    # ===== Color Adjustment (11) =====
    {"xpl_dir": "ColorAdjustmentBleachBypass",  "id": "xpl_color_bleach_bypass",   "name_cn": "漂白旁路",        "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentBrightness",    "id": "xpl_color_brightness",      "name_cn": "亮度",            "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentContrast",      "id": "xpl_color_contrast",        "name_cn": "对比度",          "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentContrastV2",    "id": "xpl_color_contrast_v2",     "name_cn": "对比度 V2",       "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentContrastV3",    "id": "xpl_color_contrast_v3",     "name_cn": "对比度 V3",       "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentHue",           "id": "xpl_color_hue",              "name_cn": "色相",            "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentLensFilter",    "id": "xpl_color_lens_filter",      "name_cn": "镜头滤镜",        "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentSaturation",    "id": "xpl_color_saturation",       "name_cn": "饱和度",          "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentTechnicolor",   "id": "xpl_color_technicolor",      "name_cn": "Technicolor",     "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentTint",          "id": "xpl_color_tint",             "name_cn": "色调",            "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorAdjustmentWhiteBalance",  "id": "xpl_color_white_balance",    "name_cn": "白平衡",          "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},

    # ===== Color Replace (2) =====
    {"xpl_dir": "ColorReplace",           "id": "xpl_color_replace",          "name_cn": "颜色替换",        "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},
    {"xpl_dir": "ColorReplaceV2",         "id": "xpl_color_replace_v2",       "name_cn": "颜色替换 V2",     "cat": "Color Adjustment",     "cat_cn": "色彩调整",    "multi": False},

    # ===== Edge Detection (9) =====
    {"xpl_dir": "EdgeDetectionRoberts",         "id": "xpl_edge_roberts",            "name_cn": "Roberts 边缘",     "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionRobertsNeon",     "id": "xpl_edge_roberts_neon",       "name_cn": "Roberts 霓虹",     "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionRobertsNeonV2",   "id": "xpl_edge_roberts_neon_v2",    "name_cn": "Roberts 霓虹 V2",  "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionScharr",          "id": "xpl_edge_scharr",              "name_cn": "Scharr 边缘",      "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionScharrNeon",      "id": "xpl_edge_scharr_neon",         "name_cn": "Scharr 霓虹",      "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionScharrNeonV2",    "id": "xpl_edge_scharr_neon_v2",      "name_cn": "Scharr 霓虹 V2",   "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionSobel",           "id": "xpl_edge_sobel",               "name_cn": "Sobel 边缘",       "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionSobelNeon",       "id": "xpl_edge_sobel_neon",           "name_cn": "Sobel 霓虹",       "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},
    {"xpl_dir": "EdgeDetectionSobelNeonV2",     "id": "xpl_edge_sobel_neon_v2",        "name_cn": "Sobel 霓虹 V2",    "cat": "Edge Detection",       "cat_cn": "边缘检测",    "multi": False},

    # ===== Pixelate (9) =====
    {"xpl_dir": "PixelizeCircle",         "id": "xpl_pixel_circle",            "name_cn": "圆形像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeDiamond",        "id": "xpl_pixel_diamond",           "name_cn": "菱形像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeHexagon",        "id": "xpl_pixel_hexagon",           "name_cn": "六边形像素化",    "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeHexagonGrid",    "id": "xpl_pixel_hexagon_grid",      "name_cn": "六边形网格",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeLeaf",           "id": "xpl_pixel_leaf",              "name_cn": "叶片像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeLed",            "id": "xpl_pixel_led",               "name_cn": "LED 像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeQuad",           "id": "xpl_pixel_quad",              "name_cn": "方形像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeSector",         "id": "xpl_pixel_sector",            "name_cn": "扇形像素化",      "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},
    {"xpl_dir": "PixelizeTriangle",       "id": "xpl_pixel_triangle",          "name_cn": "三角形像素化",    "cat": "Pixelate Effects",     "cat_cn": "像素化效果",  "multi": False},

    # ===== Vignette (5) =====
    {"xpl_dir": "AuroraVignette",         "id": "xpl_vignette_aurora",         "name_cn": "极光暗角",        "cat": "Vignette Effects",     "cat_cn": "暗角效果",    "multi": False},
    {"xpl_dir": "RapidOldTVVignette",     "id": "xpl_vignette_old_tv",         "name_cn": "老电视暗角",      "cat": "Vignette Effects",     "cat_cn": "暗角效果",    "multi": False},
    {"xpl_dir": "RapidOldTVVignetteV2",   "id": "xpl_vignette_old_tv_v2",      "name_cn": "老电视暗角 V2",   "cat": "Vignette Effects",     "cat_cn": "暗角效果",    "multi": False},
    {"xpl_dir": "RapidVignette",          "id": "xpl_vignette_rapid",           "name_cn": "快速暗角",        "cat": "Vignette Effects",     "cat_cn": "暗角效果",    "multi": False},
    {"xpl_dir": "RapidVignetteV2",        "id": "xpl_vignette_rapid_v2",        "name_cn": "快速暗角 V2",     "cat": "Vignette Effects",     "cat_cn": "暗角效果",    "multi": False},

    # ===== Sharpen (3) =====
    {"xpl_dir": "SharpenV1",              "id": "xpl_sharpen_v1",              "name_cn": "锐化 V1",         "cat": "Image Processing",     "cat_cn": "图像处理",    "multi": False},
    {"xpl_dir": "SharpenV2",              "id": "xpl_sharpen_v2",              "name_cn": "锐化 V2",         "cat": "Image Processing",     "cat_cn": "图像处理",    "multi": False},
    {"xpl_dir": "SharpenV3",              "id": "xpl_sharpen_v3",              "name_cn": "锐化 V3",         "cat": "Image Processing",     "cat_cn": "图像处理",    "multi": False},
]

if __name__ == "__main__":
    print(f"Total migrations: {len(MIGRATIONS)}")
    for i, m in enumerate(MIGRATIONS):
        print(f"  {i+1:2d}. {m['xpl_dir']:35s} -> {m['id']}")
```

- [ ] **Step 2: Run to verify count**

Run: `python c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_map.py`
Expected: `Total migrations: 56`

- [ ] **Step 3: Commit**

```bash
git add c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_map.py
git commit -m "feat: add XPL full migration mapping table (56 effects)"
```

---

### Task 1: Build the Migration Engine — HLSL → GLSL Converter

**Files:**
- Create: `c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_xpl.py`
- Read: `migrate_map.py` (mapping table)

- [ ] **Step 1: Write the conversion engine skeleton**

```python
#!/usr/bin/env python3
"""X-PostProcessing-Library → ShaderShowcase automated migration engine.
Reads XPL .shader + .cs, generates .frag (GLSL 460) + effect.json."""
import os, re, json, shutil, textwrap

XPL_SRC  = r'e:\AI\graph\X-PostProcessing-Library\Assets\X-PostProcessing\Effects'
DST_ROOT = r'e:\AI\graph\hight-post-proc\shader-showcase\shaders\effects'

# Import mapping table
from migrate_map import MIGRATIONS

# ── HLSL → GLSL Conversion Rules ──

def hlsl_to_glsl(hlsl_code):
    """Convert HLSL shader body to GLSL 460."""

    # Remove includes
    hlsl_code = re.sub(r'#include\s+"[^"]*"', '// include stripped', hlsl_code)
    # Remove #pragma
    hlsl_code = re.sub(r'#pragma\s+\w+.*\n', '', hlsl_code)
    # Remove #define macros from _Params member access
    hlsl_code = re.sub(r'#define\s+\w+\s+_Params\.\w+.*\n', '', hlsl_code)
    # Remove ShaderLab wrapper (everything before HLSLINCLUDE and after ENDHLSL)
    hlsl_code = re.sub(r'Shader\s+"[^"]*".*\n', '', hlsl_code)
    hlsl_code = re.sub(r'HLSLINCLUDE', '', hlsl_code)
    hlsl_code = re.sub(r'ENDHLSL', '', hlsl_code)
    hlsl_code = re.sub(r'SubShader\s*\{', '', hlsl_code)
    hlsl_code = re.sub(r'Pass\s*\{', '', hlsl_code)
    hlsl_code = re.sub(r'HLSLPROGRAM', '', hlsl_code)
    hlsl_code = re.sub(r'Cull\s+Off.*\n', '', hlsl_code)
    hlsl_code = re.sub(r'ZWrite\s+.*\n', '', hlsl_code)
    hlsl_code = re.sub(r'ZTest\s+.*\n', '', hlsl_code)
    hlsl_code = re.sub(r'SV_Target', '', hlsl_code)
    hlsl_code = re.sub(r'VaryingsDefault\s+i\s*\)', 'vec2 vUV)', hlsl_code)
    hlsl_code = re.sub(r'VaryingsDefault\s+i\)', 'vec2 vUV)', hlsl_code)
    hlsl_code = re.sub(r'\}\)', '}', hlsl_code)  # Clean closing braces

    # Remove comments
    hlsl_code = re.sub(r'//.*', '', hlsl_code)

    # Text replacements
    replacements = [
        # Texture sampling
        (r'SAMPLE_TEXTURE2D\s*\(\s*_MainTex\s*,\s*[^,]+,\s*([^)]+)\)', r'texture(uInputTex, \1)'),
        # Basic types
        ('half4', 'vec4'), ('half3', 'vec3'), ('half2', 'vec2'),
        ('float4', 'vec4'), ('float3', 'vec3'), ('float2', 'vec2'),
        ('half ', 'float '), ('float1', 'float'),
        ('half2x2', 'mat2'), ('half3x3', 'mat3'), ('half4x4', 'mat4'),
        # Functions
        ('saturate(', 'clamp((',  # Handled case-by-case
        ('lerp(', 'mix('),
        ('fmod(', 'mod('),
        ('frac(', 'fract('),
        ('ddx(', 'dFdx('),
        ('ddy(', 'dFdy('),
        # Vertex attributes
        ('i.texcoord', 'vUV'), ('i.texcoordStereo', 'vUV'),
        # Built-in uniforms
        ('_ScreenParams.xy', 'uResolution'),
        ('_ScreenParams.x', 'uResolution.x'),
        ('_ScreenParams.y', 'uResolution.y'),
        ('_Time.y', 'uTime'),
        ('_Time.x', 'uTime'),
        # UV fix
        ('UNITY_UV_STARTS_AT_TOP', '// OpenGL: UV origin is bottom-left'),
        # semicolons cleanup
        (';;', ';'),
    ]
    for old, new in replacements:
        hlsl_code = re.sub(old, new, hlsl_code)

    # Fix saturate special case (saturate → clamp with 0,1)
    def fix_saturate(m):
        return f'clamp({m.group(1)}, 0.0, 1.0)'
    hlsl_code = re.sub(r'saturate\(([^)]+)\)', fix_saturate, hlsl_code)

    return hlsl_code


def extract_frag_body(hlsl_code):
    """Extract the Frag() function body (everything between { and } after 'Frag')."""
    # Find all function bodies named Frag* (including helpers)
    frag_match = re.search(r'(?:half4|float4|vec4)\s+Frag\w*\s*\(VaryingsDefault\s+i\)\s*\{', hlsl_code)
    if not frag_match:
        # Try alternative signatures
        frag_match = re.search(r'(?:half4|float4|vec4)\s+Frag\w*\s*\([^)]+\)\s*\{', hlsl_code)
    if not frag_match:
        return None

    # Extract from Frag function start
    start = frag_match.start()
    # Find matching closing brace
    depth = 0
    for i, c in enumerate(hlsl_code[start:]):
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                body = hlsl_code[start:i+start+1]
                # Extract just the body (remove function signature)
                brace_start = body.index('{')
                return body[brace_start+1:]

    return None


def extract_helper_functions(hlsl_code):
    """Extract all non-Frag helper functions (intensity, sobel, custom vertex, etc)."""
    helpers = []
    # Match function definitions that aren't Frag* variants
    pattern = r'(?:(?:half|float|vec)[234]?\s+(?:\w+)\s+)(\w+)\s*\(([^)]*)\)\s*\{'
    for m in re.finditer(pattern, hlsl_code):
        name = m.group(1)
        if not name.startswith('Frag') and not name.startswith('Vert'):
            # Extract body
            start = m.end() - 1  # at first {
            depth = 0
            for i, c in enumerate(hlsl_code[start:]):
                if c == '{': depth += 1
                elif c == '}':
                    depth -= 1
                    if depth == 0:
                        helpers.append(hlsl_code[start:start+i+1])
                        break
    return '\n\n'.join(helpers)


def extract_uniforms(hlsl_code):
    """Extract uniform declarations from HLSL."""
    uniforms = {}
    for m in re.finditer(r'(?:half|float|int)([234]?)\s+(\w+)\s*;', hlsl_code):
        dim = m.group(1) or '1'
        name = m.group(2)
        uniforms[name] = dim
    return uniforms


def generate_glsl(hlsl_body, helpers, param_count, param_labels):
    """Generate the final GLSL 460 .frag file."""
    # Build Params UBO with padding
    params_lines = []
    for i in range(6):
        if i < param_count and i < len(param_labels):
            label = param_labels[i]
        else:
            label = "unused"
        name = f"uParamFloat{i}" if i < param_count else f"_pad{i - param_count}"
        comment = f"// {label}" if i < param_count else "// padding"
        params_lines.append(f"    float {name}; {comment}")

    params_block = '\n'.join(params_lines)

    glsl = f"""#version 460
// Auto-migrated from X-PostProcessing-Library
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {{
{params_block}
    vec2 uResolution;
    float uTime;
    float uFrameCount;
}};

{helpers}

void main() {{
    vec4 color = vec4(0.0);
    vec2 uv = vUV;
{textwrap.indent(hlsl_body, '    ')}
    outColor = color;
}}
"""
    return glsl


print("Migration engine loaded. Ready to process 56 effects.")
print(f"Source: {XPL_SRC}")
print(f"Target: {DST_ROOT}")
```

- [ ] **Step 2: Run to verify no syntax errors**

Run: `python c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_xpl.py`
Expected: prints "Migration engine loaded..."

- [ ] **Step 3: Commit**

```bash
git add c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_xpl.py
git commit -m "feat: add HLSL→GLSL migration engine skeleton"
```

---

### Task 2: Add C# Parser for Parameter Extraction

**Files:**
- Modify: `migrate_xpl.py` (append C# parser)

- [ ] **Step 1: Add CS parser to migration script**

```python
# ── C# Parameter Extraction ──

def parse_cs_params(cs_path):
    """Parse XPL .cs file to extract parameter definitions.
    Returns list of {name, label, type, default, min, max}."""
    with open(cs_path, 'r', encoding='utf-8') as f:
        cs = f.read()

    params = []

    # Pattern: [Range(min, max)]\n public FloatParameter Name = new FloatParameter { value = default };
    range_pattern = r'\[Range\(([0-9.]+)f?\s*,\s*([0-9.]+)f?\)\]\s*\n\s*public\s+(\w+)Parameter\s+(\w+)\s*=\s*new\s+\w+Parameter\s*\{\s*value\s*=\s*([^}]+)\}'
    for m in re.finditer(range_pattern, cs):
        ptype = m.group(3)  # Float, Int, Bool, Color
        name = m.group(4)
        default = m.group(5).strip().rstrip('f').rstrip(';').strip()
        params.append({
            'name': name,
            'type': ptype.lower(),
            'default': default,
            'min': m.group(1),
            'max': m.group(2),
        })

    # Pattern for ColorParameter:
    color_pattern = r'\[ColorUsageAttribute[^\]]*\]\s*\n\s*public\s+ColorParameter\s+(\w+)\s*=\s*new\s+ColorParameter\s*\{\s*value\s*=\s*new\s+Color\(([^)]+)\)'
    for m in re.finditer(color_pattern, cs):
        name = m.group(1)
        rgba = m.group(2).strip().rstrip('f')
        params.append({
            'name': name,
            'type': 'color',
            'default': rgba,
            'min': '0', 'max': '1',
        })

    return params


def csharp_value_to_json(val, ptype):
    """Convert C# parameter value to JSON-compatible format."""
    val = val.strip().rstrip('f')
    if val.lower() == 'true': return True
    if val.lower() == 'false': return False
    if '.' in val or ptype in ('float', 'color'):
        try: return float(val)
        except: return 0.5
    try: return int(val)
    except: return 0


def generate_effect_json(effect_id, name_cn, cat, cat_cn, params):
    """Generate effect.json content."""
    json_params = []
    for i, p in enumerate(params):
        pname = p.get('name', f'unknown_{i}')
        ptype = p.get('type', 'float')
        minv = csharp_value_to_json(p.get('min', '0'), ptype)
        maxv = csharp_value_to_json(p.get('max', '1'), ptype)
        defv = csharp_value_to_json(p.get('default', '0.5'), ptype)

        uiname = f"uParamFloat{i}"

        ui_type = 'slider'
        if ptype == 'color':
            ui_type = 'color'
            defv = [float(x.strip()) for x in defv.split(',')] if isinstance(defv, str) else defv
        elif ptype in ('int', 'bool'):
            ui_type = 'drag'

        json_params.append({
            "name": uiname,
            "label": p.get('name', uiname),
            "label_cn": p.get('name', uiname),
            "type": ptype.capitalize() if ptype != 'color' else 'Color',
            "min": minv,
            "max": maxv,
            "default": defv,
            "ui_type": ui_type,
        })

    return {
        "name": name_cn,
        "name_cn": name_cn,
        "category": cat,
        "category_cn": cat_cn,
        "description": f"{name_cn} - auto-migrated from X-PostProcessing-Library/{effect_id}",
        "description_cn": f"{name_cn}效果，自动迁移自 X-PostProcessing-Library",
        "params": json_params,
    }
```

- [ ] **Step 2: Test C# parser on BokehBlur**

Run: `python -c "import migrate_xpl as m; params = m.parse_cs_params(r'e:\AI\graph\X-PostProcessing-Library\Assets\X-PostProcessing\Effects\BokehBlur\BokehBlur.cs'); print(params)"`
Expected: 3 parameters (BlurRadius, Iteration, RTDownScaling)

- [ ] **Step 3: Commit**

```bash
git commit -a -m "feat: add C# parameter parser for XPL migration"
```

---

### Task 3: Batch Generate All .frag + effect.json Files

**Files:**
- Modify: `migrate_xpl.py` (add main processing loop)

- [ ] **Step 1: Add main processing loop**

```python
def process_single_pass(effect_id, xpl_dir, name_cn, cat, cat_cn, shader_path, cs_path):
    """Process a single-pass XPL effect → generate .frag + effect.json."""
    with open(shader_path, 'r', encoding='utf-8') as f:
        shader_code = f.read()

    # Extract HLSL code (between HLSLINCLUDE and ENDHLSL)
    hlsl_match = re.search(r'HLSLINCLUDE(.*?)ENDHLSL', shader_code, re.DOTALL)
    if not hlsl_match:
        # Some shaders have HLSLPROGRAM directly in Pass
        hlsl_match = re.search(r'HLSLPROGRAM(.*?)ENDHLSL', shader_code, re.DOTALL)
    if not hlsl_match:
        print(f"  WARN: No HLSL block found in {shader_path}, trying whole file")
        hlsl_code = shader_code
    else:
        hlsl_code = hlsl_match.group(1)

    # Convert
    glsl = hlsl_to_glsl(hlsl_code)
    body = extract_frag_body(glsl)
    helpers = extract_helper_functions(glsl)

    if body is None:
        print(f"  ERROR: No Frag function found in {shader_path}")
        return False

    # Map _Params.x → uParamFloat0 etc
    # Count distinct _Params member accesses
    param_refs = set()
    for m in re.finditer(r'_Params\.(\w+)', body):
        param_refs.add(m.group(1))
    n_params = len(param_refs) if param_refs else 1

    # Also check standalone uniform params (not _Params bundled)
    uniforms = extract_uniforms(hlsl_code)
    standalone_params = {k: v for k, v in uniforms.items() if not k.startswith('_')}

    # Replace _Params.xxx with uParamFloatN
    param_labels = sorted(param_refs) if param_refs else ["intensity"]
    for i, ref in enumerate(param_labels):
        body = body.replace(f'_Params.{ref}', f'uParamFloat{i}')

    # Replace standalone uniforms with uParamFloatN
    idx_offset = len(param_labels)
    for j, (uname, udim) in enumerate(standalone_params.items()):
        if idx_offset + j >= 6:
            break
        body = body.replace(uname, f'uParamFloat{idx_offset + j}')

    n_total_params = min(len(param_labels) + len(standalone_params), 6)

    # Generate .frag
    frag_code = generate_glsl(body, helpers, n_total_params, param_labels + list(standalone_params.keys()))

    # Parse .cs for effect.json
    params = parse_cs_params(cs_path) if cs_path and os.path.exists(cs_path) else []

    # Write files
    out_dir = os.path.join(DST_ROOT, effect_id)
    os.makedirs(out_dir, exist_ok=True)

    frag_path = os.path.join(out_dir, f'{effect_id}.frag')
    with open(frag_path, 'w', encoding='utf-8') as f:
        f.write(frag_code)

    json_path = os.path.join(out_dir, 'effect.json')
    effect_json = generate_effect_json(effect_id, name_cn, cat, cat_cn, params)
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(effect_json, f, ensure_ascii=False, indent=2)

    return True


def find_shader_file(xpl_dir):
    """Find the .shader file in XPL effect directory."""
    base = os.path.join(XPL_SRC, xpl_dir)
    # Check Shader/ subdirectory first
    for sub in ['Shader', '']:
        search = os.path.join(base, sub) if sub else base
        if os.path.isdir(search):
            for f in os.listdir(search):
                if f.endswith('.shader'):
                    return os.path.join(search, f)
    return None


def find_cs_file(xpl_dir):
    """Find the .cs file in XPL effect directory."""
    base = os.path.join(XPL_SRC, xpl_dir)
    for f in os.listdir(base):
        if f.endswith('.cs'):
            return os.path.join(base, f)
    return None


def main():
    import sys
    success = 0
    failed = []

    for m in MIGRATIONS:
        xpl_dir = m['xpl_dir']
        effect_id = m['id']
        name_cn = m['name_cn']
        cat = m['cat']
        cat_cn = m['cat_cn']

        shader_path = find_shader_file(xpl_dir)
        cs_path = find_cs_file(xpl_dir)

        if not shader_path:
            print(f"  SKIP {effect_id}: no .shader found")
            failed.append(effect_id)
            continue

        print(f"[{success+1}/{len(MIGRATIONS)}] {effect_id} ({name_cn})")

        if m.get('multi'):
            print(f"  → multi-pass, requires manual review")
            # Generate first pass only, mark for manual review
            result = process_single_pass(effect_id, xpl_dir, name_cn, cat, cat_cn, shader_path, cs_path)
            print(f"  → Pass 0 generated; Pass 1 requires manual conversion")
            if result:
                success += 1
        else:
            if process_single_pass(effect_id, xpl_dir, name_cn, cat, cat_cn, shader_path, cs_path):
                success += 1
            else:
                failed.append(effect_id)

    print(f"\n{'='*60}")
    print(f"Migration complete: {success}/{len(MIGRATIONS)} generated")
    if failed:
        print(f"Failed ({len(failed)}): {', '.join(failed)}")
    print(f"Multi-pass effects need manual Pass 1 review.")
    return 0 if not failed else 1

if __name__ == '__main__':
    sys.exit(main())
```

- [ ] **Step 2: Run migration on all 56 effects**

Run: `python c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\migrate_xpl.py`
Expected: 56 effects processed, ~50 single-pass generated, ~6 multi-pass marked

- [ ] **Step 3: Verify generated files**

Run: `python -c "import os; d=r'e:\AI\graph\hight-post-proc\shader-showcase\shaders\effects'; print(len([x for x in os.listdir(d) if x.startswith('xpl_')]))"`
Expected: `73` (17 existing glitch + 56 new)

- [ ] **Step 4: Commit**

```bash
git add shaders/effects/xpl_blur_* shaders/effects/xpl_color_* shaders/effects/xpl_edge_* shaders/effects/xpl_pixel_* shaders/effects/xpl_vignette_* shaders/effects/xpl_sharpen_*
git commit -m "feat: auto-generate .frag + effect.json for 56 XPL effects"
```

---

### Task 4: Update CMakeLists.txt with Compile Entries

**Files:**
- Modify: `e:\AI\graph\hight-post-proc\shader-showcase\shaders\CMakeLists.txt`

- [ ] **Step 1: Add compile_frag entries for all 56 effects**

Read existing CMakeLists.txt to find the pattern, then append:

```cmake
# Blur (17)
compile_frag(xpl_blur_bokeh)
compile_frag(xpl_blur_box)
compile_frag(xpl_blur_directional)
compile_frag(xpl_blur_dual_box)
compile_frag(xpl_blur_dual_gaussian)
compile_frag(xpl_blur_dual_kawase)
compile_frag(xpl_blur_dual_tent)
compile_frag(xpl_blur_gaussian)
compile_frag(xpl_blur_grainy)
compile_frag(xpl_blur_iris)
compile_frag(xpl_blur_iris_v2)
compile_frag(xpl_blur_kawase)
compile_frag(xpl_blur_radial)
compile_frag(xpl_blur_radial_v2)
compile_frag(xpl_blur_tent)
compile_frag(xpl_blur_tilt_shift)
compile_frag(xpl_blur_tilt_shift_v2)

# Color Adjustment (11)
compile_frag(xpl_color_bleach_bypass)
compile_frag(xpl_color_brightness)
compile_frag(xpl_color_contrast)
compile_frag(xpl_color_contrast_v2)
compile_frag(xpl_color_contrast_v3)
compile_frag(xpl_color_hue)
compile_frag(xpl_color_lens_filter)
compile_frag(xpl_color_saturation)
compile_frag(xpl_color_technicolor)
compile_frag(xpl_color_tint)
compile_frag(xpl_color_white_balance)

# Color Replace (2)
compile_frag(xpl_color_replace)
compile_frag(xpl_color_replace_v2)

# Edge Detection (9)
compile_frag(xpl_edge_roberts)
compile_frag(xpl_edge_roberts_neon)
compile_frag(xpl_edge_roberts_neon_v2)
compile_frag(xpl_edge_scharr)
compile_frag(xpl_edge_scharr_neon)
compile_frag(xpl_edge_scharr_neon_v2)
compile_frag(xpl_edge_sobel)
compile_frag(xpl_edge_sobel_neon)
compile_frag(xpl_edge_sobel_neon_v2)

# Pixelate (9)
compile_frag(xpl_pixel_circle)
compile_frag(xpl_pixel_diamond)
compile_frag(xpl_pixel_hexagon)
compile_frag(xpl_pixel_hexagon_grid)
compile_frag(xpl_pixel_leaf)
compile_frag(xpl_pixel_led)
compile_frag(xpl_pixel_quad)
compile_frag(xpl_pixel_sector)
compile_frag(xpl_pixel_triangle)

# Vignette (5)
compile_frag(xpl_vignette_aurora)
compile_frag(xpl_vignette_old_tv)
compile_frag(xpl_vignette_old_tv_v2)
compile_frag(xpl_vignette_rapid)
compile_frag(xpl_vignette_rapid_v2)

# Sharpen (3)
compile_frag(xpl_sharpen_v1)
compile_frag(xpl_sharpen_v2)
compile_frag(xpl_sharpen_v3)
```

- [ ] **Step 2: Commit**

```bash
git add shaders/CMakeLists.txt
git commit -m "feat: register 56 new XPL effects in CMakeLists.txt"
```

---

### Task 5: Register Cards in CoverFlowScene.cpp

**Files:**
- Modify: `e:\AI\graph\hight-post-proc\shader-showcase\src\app\CoverFlowScene.cpp`

- [ ] **Step 1: Update reserve size**

```cpp
m_cards.reserve(91);  // was 35, now 35 + 56 = 91
```

- [ ] **Step 2: Add 56 CARD macros after existing xpl_glitch entries**

```cpp
    // ==== XPL Blur Effects (17) ====
    CARD("xpl_blur_bokeh",          "散景模糊",        "Blur Effects",
         "黄金角度旋转散景模糊，模拟镜头大光圈虚化效果",
         "effects/xpl_blur_bokeh/xpl_blur_bokeh.frag.spv");
    CARD("xpl_blur_box",            "方框模糊",        "Blur Effects",
         "均匀盒式采样模糊，简单高效的平滑滤波",
         "effects/xpl_blur_box/xpl_blur_box.frag.spv");
    // ... (all 17 Blur entries with Chinese names)
```

Write Python to auto-generate all 56 CARD entries from `MIGRATIONS`:

```python
def generate_card_macros(m):
    """Generate CARD macro string for CoverFlowScene.cpp"""
    return f'    CARD("{m["id"]}", "{m["name_cn"]}", "{m["cat"]}",\n         "{m["name_cn"]}效果 — 自动迁移自 X-PostProcessing-Library",\n         "effects/{m["id"]}/{m["id"]}.frag.spv");'
```

- [ ] **Step 3: Add Dynamic pool registration**

All 56 new effects are dynamic (use uTime when present in shader). Add to dynamic check:

```cpp
// After existing xpl_glitch checks, add:
if (id.find("xpl_") == 0) {
    return true;  // All xpl_ effects are dynamic
}
```

- [ ] **Step 4: Commit**

```bash
git add src/app/CoverFlowScene.cpp
git commit -m "feat: register 56 new XPL effects in CoverFlowScene (91 total)"
```

---

### Task 6: Update LanguageManager

**Files:**
- Modify: `e:\AI\graph\hight-post-proc\shader-showcase\src\app\LanguageManager.cpp`

- [ ] **Step 1: Generate LanguageManager entries**

Python script to auto-generate CardName and CardDesc table entries:

```python
from migrate_map import MIGRATIONS

for m in MIGRATIONS:
    desc = f"{m['name_cn']}效果，自动迁移自 X-PostProcessing-Library"
    print(f'        {{"{m["id"]}", "{m["name_cn"]}", u8"{m["name_cn"]}"}},')
    print(f'        {{"{m["id"]}", "{m["name_cn"]}", u8"{desc}"}},')
```

- [ ] **Step 2: Add to CardName table**

```cpp
// After lens_distort entry:
{"xpl_blur_bokeh", "散景模糊", u8"散景模糊"},
{"xpl_blur_box", "方框模糊", u8"方框模糊"},
// ... (all 56)
```

- [ ] **Step 3: Add to CardDesc table**

```cpp
{"xpl_blur_bokeh", "散景模糊", u8"黄金角度旋转散景模糊效果"},
// ... (all 56)
```

- [ ] **Step 4: Commit**

```bash
git add src/app/LanguageManager.cpp
git commit -m "feat: add 56 XPL effect entries to LanguageManager"
```

---

### Task 7: Compile and Verify

**Files:** None new

- [ ] **Step 1: Kill old processes and build**

```powershell
taskkill /F /IM ShaderShowcase.exe 2>&1 | Out-Null
Start-Sleep 1
cmake --build e:\AI\graph\hight-post-proc\shader-showcase\build --config Release 2>&1 | Select-Object -Last 10
```

Expected: `ShaderShowcase.vcxproj -> ...\ShaderShowcase.exe` with 0 errors.

If any shaders fail GLSL compilation, fix the generated .frag and re-build.

- [ ] **Step 2: Copy SPV files from build to source tree**

```bash
python -c "import shutil,os; ...copy all new SPV files..."
```

- [ ] **Step 3: Commit**

```bash
git add shaders/effects/xpl_*/*.spv
git commit -m "feat: committed compiled SPIR-V binaries for 56 XPL effects"
```

---

### Task 8: Auto-Screenshot Verification

**Files:**
- Create: `c:\Users\86178\.trae-cn\work\6a143cac3f0e7808aed56f96\verify_xpl.py`

- [ ] **Step 1: Run auto-test screenshots**

```powershell
python -c "import glob,os; [os.remove(f) for f in glob.glob(r'e:\AI\graph\hight-post-proc\screenshots\*.ppm')]"
$env:AUTO_TEST_CARDS='1'
Start-Process 'e:\AI\graph\hight-post-proc\shader-showcase\build\bin\Release\ShaderShowcase.exe' -PassThru -RedirectStandardOutput 'e:\AI\graph\hight-post-proc\screenshots\out.txt'
Start-Sleep 300
```

Expected: card_00.ppm ~ card_90.ppm generated (91 total).

- [ ] **Step 2: Analyze screenshots for rendering issues**

```python
"""Analyze all new XPL screenshots for non-black, non-uniform output."""
# Check card_35.ppm through card_90.ppm
# Each must have variance > 5.0
```

- [ ] **Step 3: Report failures, fix problem shaders**

Report any effects that show BLACK or UNIFORM. Fix the .frag generation logic in `migrate_xpl.py`, regenerate, rebuild, retest.

- [ ] **Step 4: Final commit**

```bash
git add -A
git commit -m "chore: final verification — all 56 XPL effects render correctly"
```

---

### Task 9: Update AGENTS.md

**Files:**
- Modify: `e:\AI\graph\hight-post-proc\shader-showcase\AGENTS.md`

- [ ] **Step 1: Update effect count (35 → 91) and add new category tables**

```markdown
## 4. 91 个 Shader 效果

### 4.1 原有经典效果 (18 个)
### 4.2 XPL Glitch 移植效果 (17 个)
### 4.3 XPL Blur 效果 (17 个) — new
### 4.4 XPL Color Adjustment 效果 (13 个) — new
### 4.5 XPL Edge Detection 效果 (9 个) — new
### 4.6 XPL Pixelate 效果 (9 个) — new
### 4.7 XPL Vignette 效果 (5 个) — new
### 4.8 XPL Sharpen 效果 (3 个) — new
```

- [ ] **Step 2: Commit**

```bash
git add AGENTS.md
git commit -m "docs: update AGENTS.md for 91 effects (35 + 56 XPL)"
```
