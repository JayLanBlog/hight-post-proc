#pragma once

#include <string>

/// Supported UI languages
enum class Language {
    English,
    Chinese
};

/// Singleton language manager — provides localized strings for all UI text.
/// Call LanguageManager::Instance().SetLanguage(lang) to switch at runtime.
class LanguageManager {
public:
    static LanguageManager& Instance() {
        static LanguageManager s;
        return s;
    }

    Language GetLanguage() const { return m_lang; }
    void SetLanguage(Language lang) { m_lang = lang; }
    void ToggleLanguage() { m_lang = (m_lang == Language::English) ? Language::Chinese : Language::English; }

    // ===== CoverFlowScene strings =====
    const char* Title() const { return m_lang == Language::Chinese ? u8"Shader 效果展示" : "Shader Effect Showcase"; }
    const char* ScreenCaptureLabel() const { return m_lang == Language::Chinese ? u8"屏幕捕获 (Ctrl+S 停止)" : "Screen Capture (Ctrl+S to stop)"; }
    const char* StaticImageLabel() const { return m_lang == Language::Chinese ? u8"静态图片" : "Static Image"; }
    const char* InputInfoFormat() const { return m_lang == Language::Chinese ? u8"输入: %s  |  FPS: %.0f  |  拖放文件加载  |  Ctrl+←→ 切换图片" : "Input: %s  |  FPS: %.0f  |  Drag File to Load  |  Ctrl+Arrow Cycle Images"; }
    const char* BottomHelp() const { return m_lang == Language::Chinese ? u8"F1-F8 快速选择  |  点击卡片打开  |  拖动浏览  |  Ctrl+S 截图  |  Esc 退出" : "F1-F8 Quick Select  |  Click Card to Open  |  Drag to Browse  |  Ctrl+S Screen Capture  |  Esc Exit"; }

    const char* CategorySimple() const { return m_lang == Language::Chinese ? u8"简单" : "Simple"; }
    const char* CategoryLighting() const { return m_lang == Language::Chinese ? u8"光照" : "Lighting"; }
    const char* CategoryFilter() const { return m_lang == Language::Chinese ? u8"滤镜" : "Filter"; }
    const char* CategoryStylize() const { return m_lang == Language::Chinese ? u8"风格化" : "Stylize"; }
    const char* CategoryColor() const { return m_lang == Language::Chinese ? u8"色彩" : "Color"; }
    const char* CategoryDistort() const { return m_lang == Language::Chinese ? u8"扭曲" : "Distort"; }
    const char* CategoryProcedural() const { return m_lang == Language::Chinese ? u8"程序化" : "Procedural"; }
    const char* CategoryRetro() const { return m_lang == Language::Chinese ? u8"复古" : "Retro"; }

    const char* LanguageButton() const { return m_lang == Language::Chinese ? u8"中文 / EN" : u8"EN / 中文"; }

    // ===== EffectDetailScene strings =====
    const char* EscReturn() const { return m_lang == Language::Chinese ? u8"|  ESC 返回  |  拖放图片切换输入" : "|  ESC to Return  |  Drag Image to Change Input"; }
    const char* EffectParams() const { return m_lang == Language::Chinese ? u8"效果参数" : "Effect Parameters"; }
    const char* CompareOriginal() const { return m_lang == Language::Chinese ? u8"原图" : "Original"; }
    const char* CompareEffect() const { return m_lang == Language::Chinese ? u8"效果" : "Effect"; }
    const char* AssetLibrary() const { return m_lang == Language::Chinese ? u8"素材库" : "Asset Library"; }
    const char* Images() const { return m_lang == Language::Chinese ? u8"图片" : "Images"; }
    const char* Videos() const { return m_lang == Language::Chinese ? u8"视频" : "Videos"; }

    // ===== Application strings =====
    const char* WindowTitle() const { return m_lang == Language::Chinese ? u8"Shader 效果展示" : "Shader Showcase"; }

    // ===== Card names (bilingual) =====
    const char* CardName(const std::string& id) const;

    // ===== Card descriptions (bilingual) =====
    const char* CardDesc(const std::string& id) const;

    // ===== Category translation =====
    const char* TranslateCategory(const std::string& cat) const;

private:
    LanguageManager() : m_lang(Language::Chinese) {}
    Language m_lang;
};
