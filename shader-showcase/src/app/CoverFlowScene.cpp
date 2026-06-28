#include "app/CoverFlowScene.h"
#include "app/EffectDetailScene.h"
#include "app/Application.h"
#include "app/LanguageManager.h"
#include "shader/ShaderLoader.h"
#include "input/ScreenCapture.h"
#include "input/VideoPlayer.h"
#include "render/IRenderBackend.h"

#include <imgui.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "stb_image.h"

// Card screenshot state
static int g_cardScreenshotIndex = 0;
static int g_cardScreenshotFrame = 0;
static bool g_cardScreenshotNeedsShot = false;

CoverFlowScene::CoverFlowScene() { RegisterCards(); }
CoverFlowScene::~CoverFlowScene()
{
    if (m_backend) {
        if (m_sharedVertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_sharedVertShader);
            m_sharedVertShader = INVALID_SHADER;
        }
        for (auto& state : m_thumbnailStates) {
            if (state.fragShader.id != INVALID_SHADER.id) {
                m_backend->DestroyShader(state.fragShader);
                state.fragShader = INVALID_SHADER;
            }
            if (state.thumbTex.id != INVALID_TEXTURE.id) {
                m_backend->DestroyTexture(state.thumbTex);
                state.thumbTex = INVALID_TEXTURE;
            }
        }
        m_thumbnailStates.clear();
    }
}

#define CARD(id, name, cat, desc, frag) \
    add(id, name, cat, desc, frag)

void CoverFlowScene::RegisterCards()
{
    std::string shaderDir = ShaderLoader::FindShaderDir();
    // OpenGL uses VAO vertex input, Vulkan uses VertexIndex-generated triangle
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv";
    if (m_backend && m_backend->GetType() == BackendType::Vulkan) {
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv";
    }

    m_cards.clear();
    m_cards.reserve(91);

    auto add = [&](const char* id, const char* name, const char* category,
                   const char* desc, const char* fragRelPath) {
        EffectCard c;
        c.id = id; c.name = name; c.category = category; c.description = desc;
        c.vertSpirvPath = vertPath;
        c.fragSpirvPath = shaderDir + "/" + fragRelPath;
        c.passes = 1;
        m_cards.push_back(std::move(c));
    };

    CARD("simple_test",  "Grayscale Test",    "Color Adjustment",
         "Basic grayscale shader, validates render pipeline",
         "effects/simple_test/simple_test.frag.spv");

    CARD("bloom",        "Bloom",             "Blur Effects",
         "Extract bright areas with blur overlay for dreamy glow",
         "effects/bloom/bloom.frag.spv");

    CARD("blur",         "Gaussian Blur",     "Blur Effects",
         "Multi-pass separable Gaussian blur, large radius soft focus",
         "effects/blur/blur.frag.spv");

    CARD("sharpen",      "Sharpen",           "Image Processing",
         "Unsharp mask image sharpening, enhances edge detail",
         "effects/sharpen/sharpen.frag.spv");

    CARD("edge_detect",  "Edge Detection",    "Edge Detection",
         "Sobel edge detection with optional normal visualization",
         "effects/edge_detect/edge_detect.frag.spv");

    CARD("emboss",       "Emboss",            "Edge Detection",
         "Emboss filter for relief texture effect",
         "effects/emboss/emboss.frag.spv");

    CARD("pixelate",     "Pixelate",          "Pixelate Effects",
         "Adjustable mosaic pixelation block size",
         "effects/pixelate/pixelate.frag.spv");

    CARD("vignette",     "Vignette",          "Vignette Effects",
         "Darken edges to focus on center subject",
         "effects/vignette/vignette.frag.spv");

    CARD("chromatic",    "Chromatic Aberration", "Color Adjustment",
         "RGB channel offset simulating chromatic distortion",
         "effects/chromatic/chromatic.frag.spv");

    CARD("color_grade",  "Color Grading",     "Color Adjustment",
         "LUT-based cinematic color grading",
         "effects/color_grade/color_grade.frag.spv");

    CARD("noise",        "Noise Generator",   "Glitch Effects",
         "Perlin noise with adjustable frequency and amplitude",
         "effects/noise/noise.frag.spv");

    CARD("kaleidoscope", "Kaleidoscope",      "Image Processing",
         "Radial symmetry with adjustable sectors and rotation",
         "effects/kaleidoscope/kaleidoscope.frag.spv");

    CARD("glitch",       "Glitch Art",        "Glitch Effects",
         "Digital glitch with random block shift and color tearing",
         "effects/glitch/glitch.frag.spv");

    CARD("toon",         "Toon Shading",      "Color Adjustment",
         "Cartoon-style color quantization, cel shading",
         "effects/toon/toon.frag.spv");

    CARD("vhs",          "VHS Retro",         "Glitch Effects",
         "VHS tape scanlines, noise and color drift",
         "effects/vhs/vhs.frag.spv");

    CARD("crt",          "CRT Monitor",       "Glitch Effects",
         "CRT scanlines + phosphor RGB pattern + screen curvature",
         "effects/crt/crt.frag.spv");

    CARD("water_ripple", "Water Ripple",      "Image Processing",
         "Normal-map based water ripple displacement",
         "effects/water_ripple/water_ripple.frag.spv");

    CARD("lens_distort", "Lens Distortion",   "Image Processing",
         "Barrel/pincushion lens distortion correction and simulation",
         "effects/lens_distort/lens_distort.frag.spv");

    // === XPL Glitch Effects (17) ===
    CARD("xpl_glitch_screen_jump",      "屏幕跳跃",    "Glitch Effects",
         "Screen jump displacement with horizontal/vertical scrolling.",
         "effects/xpl_glitch_screen_jump/xpl_glitch_screen_jump.frag.spv");
    CARD("xpl_glitch_screen_shake",     "屏幕震动",   "Glitch Effects",
         "Random screen shake simulating handheld camera/earthquake.",
         "effects/xpl_glitch_screen_shake/xpl_glitch_screen_shake.frag.spv");
    CARD("xpl_glitch_scan_line_jitter", "扫描线抖动","Glitch Effects",
         "Horizontal scan line random jitter.",
         "effects/xpl_glitch_scan_line_jitter/xpl_glitch_scan_line_jitter.frag.spv");
    CARD("xpl_glitch_rgb_split_v4",     "RGB分离 V4",   "Glitch Effects",
         "Clean jittery RGB channel separation with time-quantized noise.",
         "effects/xpl_glitch_rgb_split_v4/xpl_glitch_rgb_split_v4.frag.spv");
    CARD("xpl_glitch_rgb_split_v2",     "RGB分离 V2",   "Glitch Effects",
         "Organic pulsing chromatic aberration from 4-sine product.",
         "effects/xpl_glitch_rgb_split_v2/xpl_glitch_rgb_split_v2.frag.spv");
    CARD("xpl_glitch_analog_noise",     "模拟信号噪声",   "Glitch Effects",
         "Analog static interference with luminance jitter and color noise.",
         "effects/xpl_glitch_analog_noise/xpl_glitch_analog_noise.frag.spv");
    CARD("xpl_glitch_image_block_v3",   "画面块错位 V3", "Glitch Effects",
         "Single-layer block displacement with RGB split.",
         "effects/xpl_glitch_image_block_v3/xpl_glitch_image_block_v3.frag.spv");
    CARD("xpl_glitch_image_block_v4",   "画面块错位 V4", "Glitch Effects",
         "Dual-axis block RGB split with per-block channel shifting.",
         "effects/xpl_glitch_image_block_v4/xpl_glitch_image_block_v4.frag.spv");
    CARD("xpl_glitch_tile_jitter",      "瓦片抖动",    "Glitch Effects",
         "Alternating tile band jitter with frequency gating.",
         "effects/xpl_glitch_tile_jitter/xpl_glitch_tile_jitter.frag.spv");
    CARD("xpl_glitch_rgb_split_v3",     "RGB分离 V3",   "Glitch Effects",
         "Multi-channel sine-wave RGB split with 3 direction modes.",
         "effects/xpl_glitch_rgb_split_v3/xpl_glitch_rgb_split_v3.frag.spv");
    CARD("xpl_glitch_digital_stripe",   "数字条纹", "Glitch Effects",
         "Noise-driven UV shifting with trash frame color overlay.",
         "effects/xpl_glitch_digital_stripe/xpl_glitch_digital_stripe.frag.spv");
    CARD("xpl_glitch_image_block_v2",   "画面块错位 V2", "Glitch Effects",
         "Single block layer displacement with fade control.",
         "effects/xpl_glitch_image_block_v2/xpl_glitch_image_block_v2.frag.spv");
    CARD("xpl_glitch_image_block_v1",   "画面块错位 V1", "Glitch Effects",
         "Dual block layer grid corruption with RGB split.",
         "effects/xpl_glitch_image_block_v1/xpl_glitch_image_block_v1.frag.spv");
    CARD("xpl_glitch_rgb_split_v1",     "RGB分离 V1",   "Glitch Effects",
         "Classic sine-pulsing RGB split with center-distance fading.",
         "effects/xpl_glitch_rgb_split_v1/xpl_glitch_rgb_split_v1.frag.spv");
    CARD("xpl_glitch_rgb_split_v5",     "RGB分离 V5",   "Glitch Effects",
         "Noise-driven cross-channel permutation RGB split.",
         "effects/xpl_glitch_rgb_split_v5/xpl_glitch_rgb_split_v5.frag.spv");
    CARD("xpl_glitch_line_block",       "行块错位",     "Glitch Effects",
         "Complex rhythmic line block with YUV chrominance distortion.",
         "effects/xpl_glitch_line_block/xpl_glitch_line_block.frag.spv");
    CARD("xpl_glitch_wave_jitter",      "波浪抖动",    "Glitch Effects",
         "Simplex-noise wave displacement with per-line RGB split.",
         "effects/xpl_glitch_wave_jitter/xpl_glitch_wave_jitter.frag.spv");

    CARD("xpl_blur_bokeh", "散景模糊", "Blur Effects",
         "散景模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_bokeh/xpl_blur_bokeh.frag.spv");
    CARD("xpl_blur_box", "方框模糊", "Blur Effects",
         "方框模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_box/xpl_blur_box.frag.spv");
    CARD("xpl_blur_directional", "定向模糊", "Blur Effects",
         "定向模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_directional/xpl_blur_directional.frag.spv");
    CARD("xpl_blur_dual_box", "双重方框模糊", "Blur Effects",
         "双重方框模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_dual_box/xpl_blur_dual_box.frag.spv");
    CARD("xpl_blur_dual_gaussian", "双重高斯模糊", "Blur Effects",
         "双重高斯模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_dual_gaussian/xpl_blur_dual_gaussian.frag.spv");
    CARD("xpl_blur_dual_kawase", "双重Kawase模糊", "Blur Effects",
         "双重Kawase模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_dual_kawase/xpl_blur_dual_kawase.frag.spv");
    CARD("xpl_blur_dual_tent", "双重帐篷模糊", "Blur Effects",
         "双重帐篷模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_dual_tent/xpl_blur_dual_tent.frag.spv");
    CARD("xpl_blur_gaussian", "高斯模糊", "Blur Effects",
         "高斯模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_gaussian/xpl_blur_gaussian.frag.spv");
    CARD("xpl_blur_grainy", "颗粒模糊", "Blur Effects",
         "颗粒模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_grainy/xpl_blur_grainy.frag.spv");
    CARD("xpl_blur_iris", "光圈模糊", "Blur Effects",
         "光圈模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_iris/xpl_blur_iris.frag.spv");
    CARD("xpl_blur_iris_v2", "光圈模糊 V2", "Blur Effects",
         "光圈模糊 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_iris_v2/xpl_blur_iris_v2.frag.spv");
    CARD("xpl_blur_kawase", "Kawase 模糊", "Blur Effects",
         "Kawase 模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_kawase/xpl_blur_kawase.frag.spv");
    CARD("xpl_blur_radial", "径向模糊", "Blur Effects",
         "径向模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_radial/xpl_blur_radial.frag.spv");
    CARD("xpl_blur_radial_v2", "径向模糊 V2", "Blur Effects",
         "径向模糊 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_radial_v2/xpl_blur_radial_v2.frag.spv");
    CARD("xpl_blur_tent", "帐篷模糊", "Blur Effects",
         "帐篷模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_tent/xpl_blur_tent.frag.spv");
    CARD("xpl_blur_tilt_shift", "移轴模糊", "Blur Effects",
         "移轴模糊效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_tilt_shift/xpl_blur_tilt_shift.frag.spv");
    CARD("xpl_blur_tilt_shift_v2", "移轴模糊 V2", "Blur Effects",
         "移轴模糊 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_blur_tilt_shift_v2/xpl_blur_tilt_shift_v2.frag.spv");
    CARD("xpl_color_bleach_bypass", "漂白旁路", "Color Adjustment",
         "漂白旁路效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_bleach_bypass/xpl_color_bleach_bypass.frag.spv");
    CARD("xpl_color_brightness", "亮度", "Color Adjustment",
         "亮度效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_brightness/xpl_color_brightness.frag.spv");
    CARD("xpl_color_contrast", "对比度", "Color Adjustment",
         "对比度效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_contrast/xpl_color_contrast.frag.spv");
    CARD("xpl_color_contrast_v2", "对比度 V2", "Color Adjustment",
         "对比度 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_contrast_v2/xpl_color_contrast_v2.frag.spv");
    CARD("xpl_color_contrast_v3", "对比度 V3", "Color Adjustment",
         "对比度 V3效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_contrast_v3/xpl_color_contrast_v3.frag.spv");
    CARD("xpl_color_hue", "色相", "Color Adjustment",
         "色相效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_hue/xpl_color_hue.frag.spv");
    CARD("xpl_color_lens_filter", "镜头滤镜", "Color Adjustment",
         "镜头滤镜效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_lens_filter/xpl_color_lens_filter.frag.spv");
    CARD("xpl_color_saturation", "饱和度", "Color Adjustment",
         "饱和度效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_saturation/xpl_color_saturation.frag.spv");
    CARD("xpl_color_technicolor", "Technicolor", "Color Adjustment",
         "Technicolor效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_technicolor/xpl_color_technicolor.frag.spv");
    CARD("xpl_color_tint", "色调", "Color Adjustment",
         "色调效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_tint/xpl_color_tint.frag.spv");
    CARD("xpl_color_white_balance", "白平衡", "Color Adjustment",
         "白平衡效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_white_balance/xpl_color_white_balance.frag.spv");
    CARD("xpl_color_replace", "颜色替换", "Color Adjustment",
         "颜色替换效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_replace/xpl_color_replace.frag.spv");
    CARD("xpl_color_replace_v2", "颜色替换 V2", "Color Adjustment",
         "颜色替换 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_color_replace_v2/xpl_color_replace_v2.frag.spv");
    CARD("xpl_edge_roberts", "Roberts 边缘", "Edge Detection",
         "Roberts 边缘效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_roberts/xpl_edge_roberts.frag.spv");
    CARD("xpl_edge_roberts_neon", "Roberts 霓虹", "Edge Detection",
         "Roberts 霓虹效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_roberts_neon/xpl_edge_roberts_neon.frag.spv");
    CARD("xpl_edge_roberts_neon_v2", "Roberts 霓虹 V2", "Edge Detection",
         "Roberts 霓虹 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_roberts_neon_v2/xpl_edge_roberts_neon_v2.frag.spv");
    CARD("xpl_edge_scharr", "Scharr 边缘", "Edge Detection",
         "Scharr 边缘效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_scharr/xpl_edge_scharr.frag.spv");
    CARD("xpl_edge_scharr_neon", "Scharr 霓虹", "Edge Detection",
         "Scharr 霓虹效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_scharr_neon/xpl_edge_scharr_neon.frag.spv");
    CARD("xpl_edge_scharr_neon_v2", "Scharr 霓虹 V2", "Edge Detection",
         "Scharr 霓虹 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_scharr_neon_v2/xpl_edge_scharr_neon_v2.frag.spv");
    CARD("xpl_edge_sobel", "Sobel 边缘", "Edge Detection",
         "Sobel 边缘效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_sobel/xpl_edge_sobel.frag.spv");
    CARD("xpl_edge_sobel_neon", "Sobel 霓虹", "Edge Detection",
         "Sobel 霓虹效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_sobel_neon/xpl_edge_sobel_neon.frag.spv");
    CARD("xpl_edge_sobel_neon_v2", "Sobel 霓虹 V2", "Edge Detection",
         "Sobel 霓虹 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_edge_sobel_neon_v2/xpl_edge_sobel_neon_v2.frag.spv");
    CARD("xpl_pixel_circle", "圆形像素化", "Pixelate Effects",
         "圆形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_circle/xpl_pixel_circle.frag.spv");
    CARD("xpl_pixel_diamond", "菱形像素化", "Pixelate Effects",
         "菱形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_diamond/xpl_pixel_diamond.frag.spv");
    CARD("xpl_pixel_hexagon", "六边形像素化", "Pixelate Effects",
         "六边形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_hexagon/xpl_pixel_hexagon.frag.spv");
    CARD("xpl_pixel_hexagon_grid", "六边形网格", "Pixelate Effects",
         "六边形网格效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_hexagon_grid/xpl_pixel_hexagon_grid.frag.spv");
    CARD("xpl_pixel_leaf", "叶片像素化", "Pixelate Effects",
         "叶片像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_leaf/xpl_pixel_leaf.frag.spv");
    CARD("xpl_pixel_led", "LED 像素化", "Pixelate Effects",
         "LED 像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_led/xpl_pixel_led.frag.spv");
    CARD("xpl_pixel_quad", "方形像素化", "Pixelate Effects",
         "方形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_quad/xpl_pixel_quad.frag.spv");
    CARD("xpl_pixel_sector", "扇形像素化", "Pixelate Effects",
         "扇形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_sector/xpl_pixel_sector.frag.spv");
    CARD("xpl_pixel_triangle", "三角形像素化", "Pixelate Effects",
         "三角形像素化效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_pixel_triangle/xpl_pixel_triangle.frag.spv");
    CARD("xpl_vignette_aurora", "极光暗角", "Vignette Effects",
         "极光暗角效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_vignette_aurora/xpl_vignette_aurora.frag.spv");
    CARD("xpl_vignette_old_tv", "老电视暗角", "Vignette Effects",
         "老电视暗角效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_vignette_old_tv/xpl_vignette_old_tv.frag.spv");
    CARD("xpl_vignette_old_tv_v2", "老电视暗角 V2", "Vignette Effects",
         "老电视暗角 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_vignette_old_tv_v2/xpl_vignette_old_tv_v2.frag.spv");
    CARD("xpl_vignette_rapid", "快速暗角", "Vignette Effects",
         "快速暗角效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_vignette_rapid/xpl_vignette_rapid.frag.spv");
    CARD("xpl_vignette_rapid_v2", "快速暗角 V2", "Vignette Effects",
         "快速暗角 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_vignette_rapid_v2/xpl_vignette_rapid_v2.frag.spv");
    CARD("xpl_sharpen_v1", "锐化 V1", "Image Processing",
         "锐化 V1效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_sharpen_v1/xpl_sharpen_v1.frag.spv");
    CARD("xpl_sharpen_v2", "锐化 V2", "Image Processing",
         "锐化 V2效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_sharpen_v2/xpl_sharpen_v2.frag.spv");
    CARD("xpl_sharpen_v3", "锐化 V3", "Image Processing",
         "锐化 V3效果 - 自动迁移自 X-PostProcessing-Library",
         "effects/xpl_sharpen_v3/xpl_sharpen_v3.frag.spv");

    printf("[CoverFlowScene] Registered %zu effect cards\n", m_cards.size());

    // Load effect.json params for each card
    int loadedCount = 0;
    for (auto& card : m_cards) {
        try {
            std::string jsonPath = shaderDir + "/effects/" + card.id + "/effect.json";
            EffectCard loaded = LoadEffectFromJson(jsonPath);
            if (!loaded.params.empty()) {
                card.params = std::move(loaded.params);
                loadedCount++;
            }
        } catch (...) {}
    }
    printf("[CoverFlowScene] Loaded effect.json params for %d/%zu cards\n", loadedCount, m_cards.size());

    // Build Dynamic / Static index lists (based on actual shader uTime usage)
    // Dynamic: effects that use uTime for animation (6 orig + 17 xpl_glitch)
    // Static:  all others
    for (int i = 0; i < (int)m_cards.size(); i++) {
        const auto& id = m_cards[i].id;
        if (id == "water_ripple" || id == "vhs" || id == "noise" ||
            id == "glitch" || id == "crt" || id == "kaleidoscope" ||
            id == "xpl_glitch_analog_noise" || id == "xpl_glitch_digital_stripe" ||
            id == "xpl_glitch_image_block_v1" || id == "xpl_glitch_image_block_v2" ||
            id == "xpl_glitch_image_block_v3" || id == "xpl_glitch_image_block_v4" ||
            id == "xpl_glitch_line_block" ||
            id == "xpl_glitch_rgb_split_v1" || id == "xpl_glitch_rgb_split_v2" ||
            id == "xpl_glitch_rgb_split_v3" || id == "xpl_glitch_rgb_split_v4" || id == "xpl_glitch_rgb_split_v5" ||
            id == "xpl_glitch_scan_line_jitter" || id == "xpl_glitch_screen_jump" ||
            id == "xpl_glitch_screen_shake" || id == "xpl_glitch_tile_jitter" || id == "xpl_glitch_wave_jitter"
           || id.substr(0,4) == "xpl_") {
            m_dynamicIndices.push_back(i);
        } else {
            m_staticIndices.push_back(i);
        }
    }
    printf("[CoverFlowScene] Dynamic: %zu, Static: %zu effects\n",
           m_dynamicIndices.size(), m_staticIndices.size());
}

#undef CARD

void CoverFlowScene::OnEnter()
{
    m_fpsLastTime    = std::chrono::high_resolution_clock::now();
    m_fpsFrameCount  = 0;
    m_fpsDisplay     = 0.0f;

    // Default to Dynamic + Blur Effects for visible animation
    m_currentCategory = EffectCategory::Dynamic;
    m_activeCategory  = "Blur Effects";
    m_filteredIndices.clear();
    for (int idx : m_dynamicIndices)
        if (m_cards[idx].category == "Blur Effects")
            m_filteredIndices.push_back(idx);
    if (!m_filteredIndices.empty())
        m_selectedIndex = m_filteredIndices[0];
    else if (!m_dynamicIndices.empty())
        m_selectedIndex = m_dynamicIndices[0];

    printf("[CoverFlowScene] Entered with %zu cards, selected=%d\n",
           m_cards.size(), m_selectedIndex);

    // Initialize thumbnails for all backends (OpenGL and Vulkan)
    InitializeThumbnails();
}

void CoverFlowScene::OnExit()
{
    printf("[CoverFlowScene] Exiting\n");
}

void CoverFlowScene::OnUpdate(float dt)
{
    // Auto-test: if we just returned from a detail scene and need to test next card
    if (m_autoTest && m_autoTestLastOpenedCard >= 0) {
        m_autoTestCardIndex = m_autoTestLastOpenedCard + 1;
        m_autoTestLastOpenedCard = -1;
        if (m_autoTestCardIndex >= (int)m_cards.size()) {
            // All cards tested!
            printf("\n[AutoTest] ALL %zu CARDS TESTED SUCCESSFULLY!\n", m_cards.size());
            m_autoTest = false;
        } else {
            printf("[AutoTest] Card %d/%zu passed, now testing card %d...\n",
                   m_autoTestCardIndex, m_cards.size(), m_autoTestCardIndex);
            SelectCard(m_autoTestCardIndex);
            OpenSelectedEffect(); // Enter detail page
            m_autoTestFrameCounter = m_autoTestHoldFrames;
        }
    }

    // Smooth scroll animation
    float diff = m_targetOffset - m_scrollOffset;
    m_scrollOffset += diff * std::fmin(1.0f, dt * 8.0f);

    ImGuiIO& io = ImGui::GetIO();

    // ---- FPS counter ----
    UpdateFPSCounter();

    // ---- Drag-drop: check for dropped files ----
    if (m_app) {
        std::string dropped = m_app->ConsumeDroppedFile();
        if (!dropped.empty()) {
            // Check if it's a video file
            std::string ext = dropped;
            auto dot = ext.find_last_of('.');
            if (dot != std::string::npos) {
                ext = ext.substr(dot);
                // Convert to lowercase
                for (auto& c : ext) c = (c >= 'A' && c <= 'Z') ? c + 32 : c;
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
                    OpenVideoFile(dropped);
                } else {
                    StopVideo();
                    ReloadInputTexture(dropped);
                }
            }
        }
    }

    // ---- Video player: update frames ----
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend) {
        double now = ImGui::GetTime();
        // ffmpeg pipe outputs at fixed 30fps (see StartFFmpegProcess: -r 30)
        double frameInterval = 1.0 / 30.0;
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else if (m_videoPlayer->IsEnded()) {
                // Pipe actually broken — video truly ended
                printf("[CoverFlowScene] Video pipe ended, stopping playback\n");
                StopVideo();
            }
            // else: still accumulating, try again next frame
        }
    }

    // ---- Screen capture: update input texture if active ----
    if (m_captureActive && m_screenCapture && m_screenCapture->IsReady()) {
        bool newFrame = m_screenCapture->CaptureFrame();
        if (newFrame && m_backend) {
            m_backend->UpdateTexture(m_captureTex, 0, 0,
                m_captureWidth, m_captureHeight,
                m_screenCapture->GetPixels());
        }
        // Use capture texture as input (even if no new frame, texture has previous capture)
    }

    // ---- Auto-test timer ----
    if (m_autoTest && !m_wantsExit) {
        m_autoTestFrameCounter--;
        if (m_autoTestFrameCounter <= 0) {
            // Time's up: open current card's detail scene
            printf("[AutoTest] Opening card %d: %s\n", m_autoTestCardIndex,
                   m_cards[m_autoTestCardIndex].name.c_str());
            m_autoTestLastOpenedCard = m_autoTestCardIndex;
            OpenSelectedEffect();
        }
    }

    // ---- Keyboard navigation (within filtered pool) ----
    int curPos = -1;
    for (int p = 0; p < (int)m_filteredIndices.size(); p++)
        if (m_filteredIndices[p] == m_selectedIndex) { curPos = p; break; }
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) {
        curPos = curPos - 1;
        if (curPos < 0) curPos = (int)m_filteredIndices.size() - 1;
        if (!m_filteredIndices.empty()) SelectCard(m_filteredIndices[curPos]);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) {
        curPos = curPos + 1;
        if (curPos >= (int)m_filteredIndices.size()) curPos = 0;
        if (!m_filteredIndices.empty()) SelectCard(m_filteredIndices[curPos]);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        OpenSelectedEffect();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        printf("[CoverFlowScene] ESC detected, returning to gallery\n");
        m_wantsReturn = true;
    }

    // ---- F1-F8: Quick effect selection (within filtered pool) ----
    for (int k = 0; k < 8; k++) {
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + k))) {
            if (k < (int)m_filteredIndices.size()) {
                SelectCard(m_filteredIndices[k]);
            }
        }
    }

    // ---- Ctrl+S: Toggle screen capture ----
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        ToggleScreenCapture();
    }

    // ---- Ctrl+Left/Right: Cycle built-in images ----
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        CycleImage(-1);
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        CycleImage(+1);
    }

    // ---- Mouse wheel (within filtered pool) ----
    float wheel = io.MouseWheel;
    if (wheel != 0.0f && !m_filteredIndices.empty()) {
        int newPos = curPos - static_cast<int>(wheel);
        while (newPos < 0) newPos += (int)m_filteredIndices.size();
        while (newPos >= (int)m_filteredIndices.size()) newPos -= (int)m_filteredIndices.size();
        SelectCard(m_filteredIndices[newPos]);
    }

    // ---- Mouse drag to swipe between effects (within filtered pool) ----
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        float dx = io.MouseDelta.x;
        if (!m_dragging) {
            m_dragging = true;
            m_dragStartX = io.MousePos.x;
        }
        const float swipeThreshold = 100.0f;
        float totalDx = io.MousePos.x - m_dragStartX;
        if (std::abs(totalDx) > swipeThreshold && !m_filteredIndices.empty()) {
            int newPos = curPos - (totalDx > 0 ? 1 : -1);
            while (newPos < 0) newPos += (int)m_filteredIndices.size();
            while (newPos >= (int)m_filteredIndices.size()) newPos -= (int)m_filteredIndices.size();
            SelectCard(m_filteredIndices[newPos]);
            m_dragStartX = io.MousePos.x; // reset for next swipe
        }
    } else {
        m_dragging = false;
    }
}

void CoverFlowScene::OnRender(IRenderBackend* backend)
{
    if (!backend) return;

    // Always refresh visible thumbnails with updated time (enables animation)
    RenderVisibleThumbnails();

    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_thumbnailStates.size()) return;
    const auto& state = m_thumbnailStates[m_selectedIndex];
    if (state.fragShader.id == INVALID_SHADER.id) return;

    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    m_effectTime += dt;
    m_effectFrameCount++;

    int fbWidth = 0, fbHeight = 0;
    backend->GetFramebufferSize(fbWidth, fbHeight);
    if (fbWidth != m_immersiveTexW || fbHeight != m_immersiveTexH) {
        if (m_immersiveTex.id != INVALID_TEXTURE.id)
            backend->DestroyTexture(m_immersiveTex);
        m_immersiveTex = backend->CreateTexture(fbWidth, fbHeight, TextureFormat::RGBA8, nullptr);
        m_immersiveTexW = fbWidth;
        m_immersiveTexH = fbHeight;
        m_immersiveImTexID = nullptr;
    }

    TextureHandle input = m_inputTex;
    if (m_selectedIndex < (int)m_inputTexCache.size() &&
        m_inputTexCache[m_selectedIndex].id != INVALID_TEXTURE.id)
        input = m_inputTexCache[m_selectedIndex];

    const auto& cardParams = m_cards[m_selectedIndex].params;
    std::vector<float> uniformFloats;
    std::vector<int32_t> uniformInts;
    for (const auto& p : cardParams) {
        switch (p.type) {
        case ParamType::Float: uniformFloats.push_back(p.defaultVal[0]); break;
        case ParamType::Int:
        case ParamType::Bool: uniformInts.push_back(static_cast<int32_t>(p.defaultVal[0])); break;
        case ParamType::Float2: uniformFloats.push_back(p.defaultVal[0]); uniformFloats.push_back(p.defaultVal[1]); break;
        case ParamType::Float3:
        case ParamType::Color: uniformFloats.push_back(p.defaultVal[0]); uniformFloats.push_back(p.defaultVal[1]); uniformFloats.push_back(p.defaultVal[2]); break;
        case ParamType::Float4: uniformFloats.push_back(p.defaultVal[0]); uniformFloats.push_back(p.defaultVal[1]); uniformFloats.push_back(p.defaultVal[2]); uniformFloats.push_back(p.defaultVal[3]); break;
        }
    }

    ShaderParams params;
    params.inputTextures.push_back(input);
    params.uniformFloats  = uniformFloats;
    params.uniformInts    = uniformInts;
    params.viewportWidth  = fbWidth;
    params.viewportHeight = fbHeight;
    params.time           = m_effectTime;
    params.frameCount     = m_effectFrameCount;

    backend->BeginRenderToTexture(m_immersiveTex);
    backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
    backend->EndRenderToTexture();
    m_immersiveImTexID = backend->GetImTextureID(m_immersiveTex);
}

void CoverFlowScene::OnImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) return;

    auto& LM = LanguageManager::Instance();
    const float cx = w * 0.5f;
    ImFont* font = ImGui::GetFont();

    // Build filtered indices: Dynamic/Static -> category -> cards
    const auto& basePool = (m_currentCategory == EffectCategory::Dynamic)
        ? m_dynamicIndices : m_staticIndices;
    m_filteredIndices.clear();
    for (int idx : basePool) {
        if (m_activeCategory == "Process Effects" || m_cards[idx].category == m_activeCategory)
            m_filteredIndices.push_back(idx);
    }
    // Isolation guard: if filtering by category yields nothing in current pool,
    // silently revert to "Process Effects" (show all from current pool)
    if (m_filteredIndices.empty() && !basePool.empty()) {
        m_activeCategory = "Process Effects";
        for (int idx : basePool)
            m_filteredIndices.push_back(idx);
    }
    // AUTO_TEST: bypass Dynamic/Static isolation to screenshot all cards
    if (getenv("AUTO_TEST_CARDS")) {
        m_filteredIndices.clear();
        for (int i = 0; i < (int)m_cards.size(); i++)
            m_filteredIndices.push_back(i);
    }
    int filteredPos = 0;
    int filteredTotal = (int)m_filteredIndices.size();
    if (filteredTotal > 0) {
        for (int p = 0; p < filteredTotal; p++)
            if (m_filteredIndices[p] == m_selectedIndex) { filteredPos = p; break; }
        if (m_filteredIndices[filteredPos] != m_selectedIndex) {
            filteredPos = 0;
            m_selectedIndex = m_filteredIndices[0];
        }
    }

    if (m_immersiveImTexID) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        ImGui::Begin("##ImmersiveBg", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs);
        ImDrawList* bgdl = ImGui::GetWindowDrawList();
        bgdl->AddImage(m_immersiveImTexID, ImVec2(0, 0), ImVec2(w, h),
            ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 255, 255, 255));
        ImGui::End();
        ImGui::PopStyleColor(2);
    }

    ImDrawList* fg = ImGui::GetForegroundDrawList();

    auto fgShadowText = [&](float size, ImVec2 pos, ImU32 col, const char* text) {
        ImU32 sh = IM_COL32(0, 0, 0, 180);
        fg->AddText(font, size, ImVec2(pos.x - 2, pos.y - 2), sh, text);
        fg->AddText(font, size, ImVec2(pos.x + 2, pos.y - 2), sh, text);
        fg->AddText(font, size, ImVec2(pos.x - 2, pos.y + 2), sh, text);
        fg->AddText(font, size, ImVec2(pos.x + 2, pos.y + 2), sh, text);
        fg->AddText(font, size, ImVec2(pos.x, pos.y + 3), sh, text);
        fg->AddText(font, size, pos, col, text);
    };

    // ==== Category tabs — top-left, pinned ====
    struct CatEntry { const char* key; std::string name; };
    CatEntry cats[] = {
        {"Process Effects",     LM.CategoryProcess()},
        {"Blur Effects",        LM.CategoryBlur()},
        {"Pixelate Effects",    LM.CategoryPixelate()},
        {"Edge Detection",      LM.CategoryEdge()},
        {"Glitch Effects",      LM.CategoryGlitch()},
        {"Color Adjustment",    LM.CategoryColorAdj()},
        {"Vignette Effects",    LM.CategoryVignette()},
        {"Image Processing",    LM.CategoryImageProc()},
    };
    const int nCats = sizeof(cats) / sizeof(cats[0]);
    float tabH = h * 0.033f, tabY = h * 0.008f, tabFS = h * 0.014f;
    float tabX = w * 0.015f;

    struct TR { float x0, y0, x1, y1; const char* key; } tr[nCats];
    for (int i = 0; i < nCats; i++) {
        float tW = ImGui::CalcTextSize(cats[i].name.c_str()).x + 24.0f;
        bool sel = (cats[i].key == m_activeCategory);
        tr[i] = {tabX, tabY, tabX + tW, tabY + tabH, cats[i].key};
        ImU32 bg = sel ? IM_COL32(74, 91, 207, 85) : IM_COL32(28, 31, 48, 100);
        fg->AddRectFilled(ImVec2(tabX, tabY), ImVec2(tabX + tW, tabY + tabH), bg, 4.0f);
        if (sel) fg->AddRect(ImVec2(tabX, tabY), ImVec2(tabX + tW, tabY + tabH), IM_COL32(107,124,232,180), 4.0f);
        ImU32 tc = sel ? IM_COL32(220, 225, 255, 255) : IM_COL32(150, 160, 190, 200);
        ImVec2 ts = ImGui::CalcTextSize(cats[i].name.c_str());
        fgShadowText(tabFS, ImVec2(tabX + (tW - ts.x) * 0.5f, tabY + (tabH - ts.y) * 0.5f), tc, cats[i].name.c_str());
        tabX += tW + 6.0f;
    }

    // ==== Segmented Control (Dynamic/Static) — centered below category tabs ====
    {
        const float segW = 220.0f, segH = 38.0f, segR = 19.0f;
        float segX = cx - segW * 0.5f;
        float segY = h * 0.048f;
        const char* dynLabel = (LM.GetLanguage() == Language::Chinese) ? u8"动态" : "Dynamic";
        const char* staLabel = (LM.GetLanguage() == Language::Chinese) ? u8"静态" : "Static";

        fg->AddRectFilled(ImVec2(segX, segY), ImVec2(segX + segW, segY + segH),
            IM_COL32(30, 30, 40, 200), segR);

        float halfW = segW * 0.5f;
        bool isDyn = (m_currentCategory == EffectCategory::Dynamic);

        if (isDyn)
            fg->AddRectFilled(ImVec2(segX, segY), ImVec2(segX + halfW, segY + segH),
                IM_COL32(255, 255, 255, 230), segR, ImDrawFlags_RoundCornersLeft);
        else
            fg->AddRectFilled(ImVec2(segX + halfW, segY), ImVec2(segX + segW, segY + segH),
                IM_COL32(255, 255, 255, 230), segR, ImDrawFlags_RoundCornersRight);

        float labelSize = h * 0.018f;
        ImVec2 dynSz = ImGui::CalcTextSize(dynLabel);
        ImVec2 staSz = ImGui::CalcTextSize(staLabel);
        fg->AddText(font, labelSize,
            ImVec2(segX + halfW * 0.5f - dynSz.x * 0.5f, segY + segH * 0.5f - dynSz.y * 0.5f),
            isDyn ? IM_COL32(0, 0, 0, 230) : IM_COL32(255, 255, 255, 160), dynLabel);
        fg->AddText(font, labelSize,
            ImVec2(segX + halfW + halfW * 0.5f - staSz.x * 0.5f, segY + segH * 0.5f - staSz.y * 0.5f),
            !isDyn ? IM_COL32(0, 0, 0, 230) : IM_COL32(255, 255, 255, 160), staLabel);
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("##ImmersiveUI", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);

    ImGui::SetCursorScreenPos(ImVec2(w - 110.0f, h * 0.012f));
    if (ImGui::Button(LM.LanguageButton(), ImVec2(90, 28)))
        LM.ToggleLanguage();

    for (int i = 0; i < nCats; i++) {
        ImGui::SetCursorScreenPos(ImVec2(tr[i].x0, tr[i].y0));
        ImGui::InvisibleButton(("##cfcat" + std::to_string(i)).c_str(), ImVec2(tr[i].x1 - tr[i].x0, tr[i].y1 - tr[i].y0));
        if (ImGui::IsItemClicked()) {
            m_activeCategory = tr[i].key;
            const auto& pool = (m_currentCategory == EffectCategory::Dynamic)
                ? m_dynamicIndices : m_staticIndices;
            m_filteredIndices.clear();
            for (int idx : pool)
                if (m_activeCategory == "Process Effects" || m_cards[idx].category == m_activeCategory)
                    m_filteredIndices.push_back(idx);
            // Isolation: if category has no cards in current pool, revert to "Process Effects"
            if (m_filteredIndices.empty()) {
                m_activeCategory = "Process Effects";
                for (int idx : pool)
                    m_filteredIndices.push_back(idx);
            }
            m_selectedIndex = m_filteredIndices.empty() ? 0 : m_filteredIndices[0];
        }
    }

    // Segmented Control buttons (centered)
    {
        const float segW = 220.0f, segH = 38.0f;
        float segX = cx - segW * 0.5f;
        float segY = h * 0.048f;
        ImGui::SetCursorScreenPos(ImVec2(segX, segY));
        ImGui::InvisibleButton("##segDyn", ImVec2(segW * 0.5f, segH));
        if (ImGui::IsItemClicked()) {
            m_currentCategory = EffectCategory::Dynamic;
            m_activeCategory = "Process Effects";
            const auto& pool = m_dynamicIndices;
            m_selectedIndex = pool.empty() ? 0 : pool[0];
        }
        ImGui::SetCursorScreenPos(ImVec2(segX + segW * 0.5f, segY));
        ImGui::InvisibleButton("##segSta", ImVec2(segW * 0.5f, segH));
        if (ImGui::IsItemClicked()) {
            m_currentCategory = EffectCategory::Static;
            m_activeCategory = "Process Effects";
            const auto& pool = m_staticIndices;
            m_selectedIndex = pool.empty() ? 0 : pool[0];
        }
    }

    ImGui::SetCursorScreenPos(ImVec2(0, 0));
    ImGui::InvisibleButton("##clickOpen", ImVec2(w, h));
    if (m_effectFrameCount > 15 && ImGui::IsItemClicked()) {
        ImVec2 mp = io.MousePos;
        bool onSeg = (mp.x >= cx - 110 && mp.x <= cx + 110 && mp.y >= h * 0.048f && mp.y <= h * 0.048f + 38);
        bool onTab = false;
        for (int i = 0; i < nCats; i++)
            if (mp.x >= tr[i].x0 && mp.x <= tr[i].x1 && mp.y >= tr[i].y0 - 5 && mp.y <= tr[i].y1 + 5) { onTab = true; break; }
        bool onLang = (mp.x >= w - 120 && mp.x <= w - 10 && mp.y < h * 0.06f);
        if (!onSeg && !onTab && !onLang && !m_cards.empty()) OpenSelectedEffect();
    }

    ImGui::End();
    ImGui::PopStyleColor(2);

    // ==== Left-aligned vertical info bar ──
    const float LX = w * 0.04f;  // left margin

    // Effect name — large, prominent
    {
        const auto& card = m_cards[m_selectedIndex];
        const char* ns = LM.CardName(card.id);
        float nsSize = h * 0.06f;
        fgShadowText(nsSize, ImVec2(LX, h * 0.78f), IM_COL32(245, 250, 255, 245), ns);
    }
    // Description — single line, muted
    {
        const auto& card = m_cards[m_selectedIndex];
        const char* ds = LM.CardDesc(card.id);
        float dsSize = h * 0.022f;
        // truncate if too wide
        char line[384];
        snprintf(line, sizeof(line), "%s", ds);
        float maxW = w * 0.65f;
        ImVec2 sz = ImGui::CalcTextSize(line);
        int cut = (int)strlen(line);
        while (cut > 0 && sz.x > maxW) {
            cut--; sz = ImGui::CalcTextSize(line, line + cut, false);
        }
        if (cut < (int)strlen(line)) snprintf(line, sizeof(line), "%.*s...", cut, ds);
        fgShadowText(dsSize, ImVec2(LX, h * 0.78f + h * 0.06f * 1.3f),
                     IM_COL32(200, 205, 215, 160), line);
    }
    // Page dots — left aligned row
    {
        const float dr = 3.0f, gs = 10.0f;
        float dy = h * 0.89f;
        if (filteredTotal > 0) {
            for (int i = 0; i < filteredTotal; i++) {
                fg->AddCircleFilled(ImVec2(LX + i * gs, dy), dr,
                    i == filteredPos ? IM_COL32(74, 91, 207, 255) : IM_COL32(74, 91, 207, 55));
            }
        }
    }
    // Page number — right below dots
    {
        char buf[64]; snprintf(buf, sizeof(buf), "%d / %d", filteredPos + 1, filteredTotal);
        fgShadowText(h * 0.02f, ImVec2(LX, h * 0.91f), IM_COL32(220, 225, 240, 150), buf);
    }
    // Bottom help — near bottom edge
    {
        const char* hh = LM.BottomHelp();
        fgShadowText(h * 0.017f, ImVec2(LX, h * 0.955f), IM_COL32(140, 145, 165, 130), hh);
    }
    // Left / Right arrows — right edge, same vertical band as name
    if (filteredTotal > 1) {
        fgShadowText(h * 0.05f, ImVec2(w * 0.93f, h * 0.78f), IM_COL32(255, 255, 255, 60), "<");
        ImVec2 gsz = ImGui::CalcTextSize(">");
        fgShadowText(h * 0.05f, ImVec2(w * 0.97f - gsz.x, h * 0.78f), IM_COL32(255, 255, 255, 60), ">");
    }

    if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_DETAILS")) {
        g_cardScreenshotFrame++;
        if (g_cardScreenshotNeedsShot && g_cardScreenshotFrame >= 5) {
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/card_%02d.ppm", g_cardScreenshotIndex - 1);
            ScreenshotRequest::Request(path);
            g_cardScreenshotNeedsShot = false;
        }
        if (g_cardScreenshotFrame >= 30) {
            g_cardScreenshotFrame = 0;
            if (g_cardScreenshotIndex < (int)m_cards.size()) {
                SelectCard(g_cardScreenshotIndex);
                printf("[CardScreenshot] Selected card %d: %s\n", g_cardScreenshotIndex, m_cards[g_cardScreenshotIndex].name.c_str());
                g_cardScreenshotIndex++;
                g_cardScreenshotNeedsShot = true;
            } else {
                printf("[CardScreenshot] All %zu cards captured\n", m_cards.size());
                m_wantsExit = true;
            }
        }
    }
    if (getenv("AUTO_TEST_DETAILS") && !getenv("AUTO_TEST_UI")) {
        static int detailCardIndex = 0;
        static int frameWait = 0;
        frameWait++;
        if (frameWait >= 30) {
            frameWait = 0;
            if (detailCardIndex < (int)m_cards.size()) {
                SelectCard(detailCardIndex);
                printf("[DetailScreenshot] Opening card %d: %s\n", detailCardIndex, m_cards[detailCardIndex].name.c_str());
                detailCardIndex++;
                OpenSelectedEffect();
            } else {
                printf("[DetailScreenshot] All %zu detail pages captured\n", m_cards.size());
                m_wantsExit = true;
            }
        }
    }
}

void CoverFlowScene::SelectCard(int index) {
    if (index < 0) index = (int)m_cards.size() - 1;
    if (index >= (int)m_cards.size()) index = 0;
    if (index != m_selectedIndex) {
        m_selectedIndex = index;
        m_targetOffset = 0.0f;
        printf("[CoverFlowScene] Selected card %d: %s\n", index, m_cards[index].name.c_str());
    }
}

void CoverFlowScene::EnableAutoTest(int holdFrames) {
    m_autoTest = true;
    m_autoTestHoldFrames = holdFrames;
    m_autoTestFrameCounter = holdFrames;
    m_autoTestCardIndex = 0;
    m_autoTestLastOpenedCard = -1;
    // Only enter detail page if not just taking UI screenshot
    if (!getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_CARDS")) {
        SelectCard(0);
        OpenSelectedEffect(); // Enter first card's detail page
    }
    printf("[CoverFlowScene] Auto-test enabled: %zu cards, %d frames each\n",
           m_cards.size(), holdFrames);
}

void CoverFlowScene::ResumeAutoTest(int holdFrames, int lastOpenedCard) {
    m_autoTest = true;
    m_autoTestHoldFrames = holdFrames;
    m_autoTestFrameCounter = 0; // will be set by the OnUpdate logic
    m_autoTestCardIndex = lastOpenedCard;
    m_autoTestLastOpenedCard = lastOpenedCard; // triggers "next card" logic in OnUpdate
    printf("[CoverFlowScene] Auto-test resumed: card %d done, moving to next\n", lastOpenedCard);
}

void CoverFlowScene::OpenSelectedEffect()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_cards.size()) return;
    if (!m_backend) {
        fprintf(stderr, "[CoverFlowScene] Cannot open effect: no backend set\n");
        return;
    }

    const auto& card = m_cards[m_selectedIndex];
    EffectCard resolvedCard = card;

    // Try to load effect.json for richer metadata (with try-catch safety)
    try {
        std::string shaderDir = ShaderLoader::FindShaderDir();
        std::string jsonPath = shaderDir + "/effects/" + card.id + "/effect.json";
        EffectCard fj = LoadEffectFromJson(jsonPath);
        if (!fj.name.empty()) {
            fj.vertSpirvPath = card.vertSpirvPath;
            fj.fragSpirvPath = card.fragSpirvPath;
            fj.id = card.id;
            resolvedCard = std::move(fj);
        }
    } catch (...) {
        fprintf(stderr, "[CoverFlowScene] effect.json parse error for %s, using defaults\n", card.id.c_str());
    }

    // Pick the per-effect cached input texture if available
    TextureHandle detailInputTex = m_inputTex;  // default fallback
    if (m_selectedIndex < (int)m_inputTexCache.size() &&
        m_inputTexCache[m_selectedIndex].id != INVALID_TEXTURE.id) {
        detailInputTex = m_inputTexCache[m_selectedIndex];
        printf("[CoverFlowScene] Using cached input texture for card %d\n", m_selectedIndex);
    }

    auto ds = std::make_unique<EffectDetailScene>(resolvedCard, detailInputTex);
    ds->SetBackend(m_backend);
    ds->SetApplication(m_app);
    ds->SetCoverFlowState(GetState());

    // Transfer video player to detail scene (for dynamic playback in compare view)
    if (m_videoActive && m_videoPlayer) {
        ds->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
        m_videoActive = false;
        m_videoTex = {0};
        printf("[CoverFlowScene] Transferred video player to detail scene\n");
    }

    printf("[CoverFlowScene] Opening effect: %s (%s)\n",
           resolvedCard.name.c_str(), resolvedCard.id.c_str());

    m_nextScene = std::move(ds);
    m_wantsExit = true;
}

std::unique_ptr<Scene> CoverFlowScene::GetNextScene()
{
    if (m_nextScene) {
        printf("[CoverFlowScene] Transitioning to next scene\n");
        return std::move(m_nextScene);
    }
    return nullptr;
}

// ============================================================================
// FPS Counter
// ============================================================================

void CoverFlowScene::UpdateFPSCounter()
{
    auto now = std::chrono::high_resolution_clock::now();
    m_fpsFrameCount++;

    float elapsed = std::chrono::duration<float>(now - m_fpsLastTime).count();
    if (elapsed >= 1.0f) {
        m_fpsDisplay = m_fpsFrameCount / elapsed;
        m_fpsFrameCount = 0;
        m_fpsLastTime = now;
    }
}

// ============================================================================
// Screen Capture Toggle
// ============================================================================

void CoverFlowScene::ToggleScreenCapture()
{
    if (m_captureActive) {
        // Disable screen capture
        m_captureActive = false;
        if (m_screenCapture) {
            m_screenCapture->Shutdown();
            m_screenCapture.reset();
        }
        if (m_captureTex.id != INVALID_TEXTURE.id && m_backend) {
            m_backend->DestroyTexture(m_captureTex);
            m_captureTex = INVALID_TEXTURE;
        }
        m_captureReady = false;
        printf("[CoverFlowScene] Screen capture stopped\n");
    } else {
        // Enable screen capture
        if (!m_backend) return;

        m_screenCapture = std::make_unique<ScreenCapture>();
        if (!m_screenCapture->Init()) {
            fprintf(stderr, "[CoverFlowScene] Screen capture init failed\n");
            m_screenCapture.reset();
            return;
        }

        // Capture first frame to get dimensions
        if (!m_screenCapture->CaptureFrame()) {
            fprintf(stderr, "[CoverFlowScene] Screen capture first frame failed\n");
            m_screenCapture->Shutdown();
            m_screenCapture.reset();
            return;
        }

        m_captureWidth  = m_screenCapture->GetWidth();
        m_captureHeight = m_screenCapture->GetHeight();

        // Create OpenGL texture for screen capture
        m_captureTex = m_backend->CreateTexture(m_captureWidth, m_captureHeight,
                                                 TextureFormat::RGBA8,
                                                 m_screenCapture->GetPixels());
        if (m_captureTex.id == INVALID_TEXTURE.id) {
            fprintf(stderr, "[CoverFlowScene] Failed to create capture texture\n");
            m_screenCapture->Shutdown();
            m_screenCapture.reset();
            return;
        }

        // Switch input texture to capture
        m_inputTex = m_captureTex;
        m_captureActive = true;
        m_captureReady  = true;
        printf("[CoverFlowScene] Screen capture started (%d x %d)\n",
               m_captureWidth, m_captureHeight);
    }
}

// ============================================================================
// Image Cycling
// ============================================================================

void CoverFlowScene::CycleImage(int direction)
{
    if (m_imagePool.empty()) return;
    if (m_captureActive) return; // Don't cycle while capturing

    m_currentImageIndex += direction;
    if (m_currentImageIndex < 0)
        m_currentImageIndex = (int)m_imagePool.size() - 1;
    if (m_currentImageIndex >= (int)m_imagePool.size())
        m_currentImageIndex = 0;

    LoadImageFromFile(m_imagePool[m_currentImageIndex]);
}

void CoverFlowScene::LoadImageFromFile(const std::string& path)
{
    if (!m_backend) return;

    int iw, ih, comp;
    stbi_set_flip_vertically_on_load(true); // Flip for OpenGL bottom-left origin
    stbi_uc* data = stbi_load(path.c_str(), &iw, &ih, &comp, 4);
    stbi_set_flip_vertically_on_load(false);
    if (!data) {
        fprintf(stderr, "[CoverFlowScene] Cannot load: %s\n", path.c_str());
        return;
    }

    printf("[CoverFlowScene] Loading image: %s (%d x %d)\n", path.c_str(), iw, ih);

    // Destroy old input texture if valid
    if (m_inputTex.id != INVALID_TEXTURE.id) {
        m_backend->DestroyTexture(m_inputTex);
        m_inputTex = INVALID_TEXTURE;
    }

    TextureHandle newTex = m_backend->CreateTexture(iw, ih, TextureFormat::RGBA8, data);
    stbi_image_free(data);

    if (newTex.id == INVALID_TEXTURE.id) return;

    m_inputTex = newTex;
}

// ============================================================================
// Drag-drop reload
// ============================================================================

void CoverFlowScene::AddImageToPool(const std::string& path)
{
    // Avoid duplicates
    for (const auto& p : m_imagePool) {
        if (p == path) return;
    }
    m_imagePool.push_back(path);
    printf("[CoverFlowScene] Added image to pool: %s (total %zu)\n",
           path.c_str(), m_imagePool.size());
}

void CoverFlowScene::AddVideoToPool(const std::string& path)
{
    // Avoid duplicates
    for (const auto& p : m_videoPool) {
        if (p == path) return;
    }
    m_videoPool.push_back(path);
    printf("[CoverFlowScene] Added video to pool: %s (total %zu)\n",
           path.c_str(), m_videoPool.size());
}

void CoverFlowScene::ReloadInputTexture(const std::string& filePath)
{
    LoadImageFromFile(filePath);
    AddImageToPool(filePath);
    // Find index
    for (int i = 0; i < (int)m_imagePool.size(); i++) {
        if (m_imagePool[i] == filePath) {
            m_currentImageIndex = i;
            break;
        }
    }
}

// ============================================================================
// Dynamic Thumbnail Rendering
// ============================================================================

void CoverFlowScene::InitializeThumbnails()
{
    if (m_thumbInitialized || !m_backend) return;
    if (m_cards.empty()) return;

    std::string shaderDir = ShaderLoader::FindShaderDir();
    
    // OpenGL uses VAO vertex input, Vulkan uses VertexIndex-generated triangle
    std::string vertPath = shaderDir + "/common/fullscreen.vert.spv";
    if (m_backend->GetType() == BackendType::Vulkan) {
        vertPath = shaderDir + "/common/fullscreen_vk.vert.spv";
    }
    auto vertSpv = ShaderLoader::LoadSPIRV(vertPath);
    if (vertSpv.empty()) {
        fprintf(stderr, "[CoverFlowScene] Cannot load vertex shader for thumbnails\n");
        return;
    }
    m_sharedVertShader = m_backend->CreateVertexShader(vertSpv.data(), vertSpv.size());
    if (m_sharedVertShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[CoverFlowScene] Cannot create vertex shader for thumbnails\n");
        return;
    }

    m_thumbIds.resize(m_cards.size(), nullptr);

    for (int i = 0; i < (int)m_cards.size(); i++) {
        CardThumbnailState state;
        state.fragShader = INVALID_SHADER;
        state.thumbTex = INVALID_TEXTURE;

        auto fragSpv = ShaderLoader::LoadSPIRV(m_cards[i].fragSpirvPath);
        if (!fragSpv.empty()) {
            state.fragShader = m_backend->CreateFragmentShader(fragSpv.data(), fragSpv.size());
        }

        state.thumbTex = m_backend->CreateTexture(m_thumbWidth, m_thumbHeight, TextureFormat::RGBA8, nullptr);

        m_thumbnailStates.push_back(state);
    }

    m_thumbInitialized = true;
    printf("[CoverFlowScene] Thumbnails initialized: %zu cards, %dx%d\n",
           m_thumbnailStates.size(), m_thumbWidth, m_thumbHeight);
}

void CoverFlowScene::RenderVisibleThumbnails()
{
    if (!m_thumbInitialized || !m_backend) return;

    // Update thumbnail time every frame
    static auto thumbLastTime = std::chrono::high_resolution_clock::now();
    auto thumbNow = std::chrono::high_resolution_clock::now();
    m_thumbElapsedTime += std::chrono::duration<float>(thumbNow - thumbLastTime).count();
    m_thumbFrameCount++;
    thumbLastTime = thumbNow;

    const int vr = 3;

    for (int i = m_selectedIndex - vr; i <= m_selectedIndex + vr; i++) {
        if (i < 0 || i >= (int)m_thumbnailStates.size()) continue;
        const auto& state = m_thumbnailStates[i];
        if (state.fragShader.id == INVALID_SHADER.id) continue;
        if (state.thumbTex.id == INVALID_TEXTURE.id) continue;

        TextureHandle input = m_inputTex;
        if (i < (int)m_inputTexCache.size() && m_inputTexCache[i].id != INVALID_TEXTURE.id) {
            input = m_inputTexCache[i];
        }

        m_backend->BeginRenderToTexture(state.thumbTex);
        ShaderParams params;
        params.inputTextures.push_back(input);
        // Build per-effect uniform values from effect.json params
        std::vector<float> uf;
        std::vector<int32_t> ui;
        for (const auto& p : m_cards[i].params) {
            switch (p.type) {
            case ParamType::Float: uf.push_back(p.defaultVal[0]); break;
            case ParamType::Int:
            case ParamType::Bool: ui.push_back(static_cast<int32_t>(p.defaultVal[0])); break;
            case ParamType::Float2: uf.push_back(p.defaultVal[0]); uf.push_back(p.defaultVal[1]); break;
            case ParamType::Float3:
            case ParamType::Color: uf.push_back(p.defaultVal[0]); uf.push_back(p.defaultVal[1]); uf.push_back(p.defaultVal[2]); break;
            case ParamType::Float4: uf.push_back(p.defaultVal[0]); uf.push_back(p.defaultVal[1]); uf.push_back(p.defaultVal[2]); uf.push_back(p.defaultVal[3]); break;
            }
        }
        params.uniformFloats  = uf;
        params.uniformInts    = ui;
        params.viewportWidth  = m_thumbWidth;
        params.viewportHeight = m_thumbHeight;
        params.time           = m_thumbElapsedTime;
        params.frameCount     = m_thumbFrameCount;
        m_backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
        m_backend->EndRenderToTexture();

        m_thumbIds[i] = m_backend->GetImTextureID(state.thumbTex);
    }
}

// ============================================================================
// GetState — save current state for EffectDetailScene restoration
// ============================================================================

CoverFlowState CoverFlowScene::GetState() const
{
    CoverFlowState s;
    s.thumbIds         = m_thumbIds;
    s.inputTexCache    = m_inputTexCache;
    s.imagePool        = m_imagePool;
    s.videoPool        = m_videoPool;
    s.currentImageIndex = m_currentImageIndex;
    s.selectedIndex    = m_selectedIndex;
    s.app              = m_app;
    s.inputTex         = m_inputTex;
    s.backend          = m_backend;
    s.captureActive    = m_captureActive;
    s.autoTest         = m_autoTest;
    s.autoTestHoldFrames = m_autoTestHoldFrames;
    s.autoTestCardIndex  = m_autoTestCardIndex;
    s.testImageBaseDir = m_testImageBaseDir;
    return s;
}

// ============================================================================
// SetVideoPlayer — transfer video player ownership from detail scene
// ============================================================================

void CoverFlowScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);
    m_videoTex = videoTex;
    m_videoActive = active;
    m_videoLastFrameTime = lastFrameTime;
    if (active) {
        m_inputTex = m_videoTex;
    }
}

// ============================================================================
// Video Player
// ============================================================================

void CoverFlowScene::OpenVideoFile(const std::string& path)
{
    if (!m_backend) return;

    // Stop any existing video
    StopVideo();

    m_videoPlayer = std::make_unique<VideoPlayer>();
    if (!m_videoPlayer->Open(path)) {
        fprintf(stderr, "[CoverFlowScene] Cannot open video: %s\n", path.c_str());
        m_videoPlayer.reset();
        return;
    }

    // Create texture for video frames (nullptr = empty; first ReadFrame fills via UpdateTexture)
    m_videoTex = m_backend->CreateTexture(
        m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
        TextureFormat::RGBA8, nullptr);

    if (m_videoTex.id == INVALID_TEXTURE.id) {
        fprintf(stderr, "[CoverFlowScene] Cannot create video texture\n");
        m_videoPlayer->Close();
        m_videoPlayer.reset();
        return;
    }

    m_inputTex = m_videoTex;
    m_videoActive = true;
    m_videoLastFrameTime = ImGui::GetTime();
    printf("[CoverFlowScene] Video playing: %s\n", path.c_str());
}

void CoverFlowScene::StopVideo()
{
    if (m_videoPlayer) {
        m_videoPlayer->Close();
        m_videoPlayer.reset();
    }
    m_videoActive = false;
    // Note: we don't destroy m_videoTex here because it might still be referenced
    // by EffectDetailScene. A proper implementation would use reference counting.
}
