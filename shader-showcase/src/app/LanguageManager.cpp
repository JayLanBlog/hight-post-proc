#include "app/LanguageManager.h"
#include <cstring>

const char* LanguageManager::CardName(const std::string& id) const {
    struct Entry { const char* id; const char* en; const char* zh; };
    static const Entry table[] = {
        {"simple_test",    "Grayscale Test",       u8"灰度测试"},
        {"bloom",          "Bloom",                u8"泛光"},
        {"blur",           "Gaussian Blur",        u8"高斯模糊"},
        {"sharpen",        "Sharpen",              u8"锐化"},
        {"edge_detect",    "Edge Detection",       u8"边缘检测"},
        {"emboss",         "Emboss",               u8"浮雕"},
        {"pixelate",       "Pixelate",             u8"像素化"},
        {"vignette",       "Vignette",             u8"暗角"},
        {"chromatic",      "Chromatic Aberration", u8"色差"},
        {"color_grade",    "Color Grading",        u8"调色"},
        {"noise",          "Noise Generator",      u8"噪声生成"},
        {"kaleidoscope",   "Kaleidoscope",         u8"万花筒"},
        {"glitch",         "Glitch Art",           u8"故障艺术"},
        {"toon",           "Toon Shading",         u8"卡通着色"},
        {"vhs",            "VHS Retro",            u8"VHS 复古"},
        {"crt",            "CRT Monitor",          u8"CRT 显示器"},
        {"water_ripple",   "Water Ripple",         u8"水波纹"},
        {"lens_distort",   "Lens Distortion",      u8"镜头畸变"},
    };
    for (const auto& e : table) {
        if (id == e.id) return m_lang == Language::Chinese ? e.zh : e.en;
    }
    return id.c_str(); // fallback: return the id itself
}

const char* LanguageManager::CardDesc(const std::string& id) const {
    struct Entry { const char* id; const char* en; const char* zh; };
    static const Entry table[] = {
        {"simple_test",    "Basic grayscale shader, validates render pipeline",           u8"基础灰度着色器，验证渲染管线"},
        {"bloom",          "Extract bright areas with blur overlay for dreamy glow",     u8"提取高亮区域叠加模糊，产生梦幻光晕"},
        {"blur",           "Multi-pass separable Gaussian blur, large radius soft focus", u8"多通道可分离高斯模糊，大半径柔焦"},
        {"sharpen",        "Unsharp mask image sharpening, enhances edge detail",        u8"反锐化掩模图像锐化，增强边缘细节"},
        {"edge_detect",    "Sobel edge detection with optional normal visualization",     u8"Sobel 边缘检测，可选法线可视化"},
        {"emboss",         "Emboss filter for relief texture effect",                   u8"浮雕滤镜，产生浮雕纹理效果"},
        {"pixelate",       "Adjustable mosaic pixelation block size",                   u8"可调马赛克像素化块大小"},
        {"vignette",       "Darken edges to focus on center subject",                   u8"压暗边缘以突出中心主体"},
        {"chromatic",      "RGB channel offset simulating chromatic distortion",         u8"RGB 通道偏移模拟色差畸变"},
        {"color_grade",    "LUT-based cinematic color grading",                         u8"基于 LUT 的电影级调色"},
        {"noise",          "Perlin noise with adjustable frequency and amplitude",       u8"可调频率和振幅的 Perlin 噪声"},
        {"kaleidoscope",   "Radial symmetry with adjustable sectors and rotation",       u8"可调扇区和旋转的径向对称"},
        {"glitch",         "Digital glitch with random block shift and color tearing",    u8"随机块位移和色彩撕裂的数字故障"},
        {"toon",           "Cartoon-style color quantization, cel shading",              u8"卡通风格色彩量化，赛璐珞着色"},
        {"vhs",            "VHS tape scanlines, noise and color drift",                  u8"VHS 磁带扫描线、噪声和色彩漂移"},
        {"crt",            "CRT scanlines + phosphor RGB pattern + screen curvature",   u8"CRT 扫描线 + 磷光 RGB 图案 + 屏幕弯曲"},
        {"water_ripple",   "Normal-map based water ripple displacement",                 u8"基于法线贴图的水波纹位移"},
        {"lens_distort",   "Barrel/pincushion lens distortion correction",              u8"桶形/枕形镜头畸变校正与模拟"},
    };
    for (const auto& e : table) {
        if (id == e.id) return m_lang == Language::Chinese ? e.zh : e.en;
    }
    return "";
}

const char* LanguageManager::TranslateCategory(const std::string& cat) const {
    if (cat == "Simple")     return CategorySimple();
    if (cat == "Lighting")   return CategoryLighting();
    if (cat == "Filter")     return CategoryFilter();
    if (cat == "Stylize")    return CategoryStylize();
    if (cat == "Color")      return CategoryColor();
    if (cat == "Distort")    return CategoryDistort();
    if (cat == "Procedural") return CategoryProcedural();
    if (cat == "Retro")      return CategoryRetro();
    return cat.c_str();
}
