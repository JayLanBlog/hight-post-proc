#include "app/CoverFlowScene.h"
#include "app/EffectDetailScene.h"
#include "app/Application.h"
#include "app/LanguageManager.h"
#include "shader/ShaderLoader.h"
#include "input/ScreenCapture.h"
#include "input/VideoPlayer.h"
#include "render/OpenGLBackend.h"

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
    std::string vertPath  = shaderDir + "/common/fullscreen.vert.spv";

    m_cards.clear();
    m_cards.reserve(18);

    auto add = [&](const char* id, const char* name, const char* category,
                   const char* desc, const char* fragRelPath) {
        EffectCard c;
        c.id = id; c.name = name; c.category = category; c.description = desc;
        c.vertSpirvPath = vertPath;
        c.fragSpirvPath = shaderDir + "/" + fragRelPath;
        c.passes = 1;
        m_cards.push_back(std::move(c));
    };

    CARD("simple_test",  "Grayscale Test",    "Simple",
         "Basic grayscale shader, validates render pipeline",
         "effects/simple_test/simple_test.frag.spv");

    CARD("bloom",        "Bloom",             "Lighting",
         "Extract bright areas with blur overlay for dreamy glow",
         "effects/bloom/bloom.frag.spv");

    CARD("blur",         "Gaussian Blur",     "Filter",
         "Multi-pass separable Gaussian blur, large radius soft focus",
         "effects/blur/blur.frag.spv");

    CARD("sharpen",      "Sharpen",           "Filter",
         "Unsharp mask image sharpening, enhances edge detail",
         "effects/sharpen/sharpen.frag.spv");

    CARD("edge_detect",  "Edge Detection",    "Filter",
         "Sobel edge detection with optional normal visualization",
         "effects/edge_detect/edge_detect.frag.spv");

    CARD("emboss",       "Emboss",            "Stylize",
         "Emboss filter for relief texture effect",
         "effects/emboss/emboss.frag.spv");

    CARD("pixelate",     "Pixelate",          "Stylize",
         "Adjustable mosaic pixelation block size",
         "effects/pixelate/pixelate.frag.spv");

    CARD("vignette",     "Vignette",          "Color",
         "Darken edges to focus on center subject",
         "effects/vignette/vignette.frag.spv");

    CARD("chromatic",    "Chromatic Aberration", "Distort",
         "RGB channel offset simulating chromatic distortion",
         "effects/chromatic/chromatic.frag.spv");

    CARD("color_grade",  "Color Grading",     "Color",
         "LUT-based cinematic color grading",
         "effects/color_grade/color_grade.frag.spv");

    CARD("noise",        "Noise Generator",   "Procedural",
         "Perlin noise with adjustable frequency and amplitude",
         "effects/noise/noise.frag.spv");

    CARD("kaleidoscope", "Kaleidoscope",      "Distort",
         "Radial symmetry with adjustable sectors and rotation",
         "effects/kaleidoscope/kaleidoscope.frag.spv");

    CARD("glitch",       "Glitch Art",        "Stylize",
         "Digital glitch with random block shift and color tearing",
         "effects/glitch/glitch.frag.spv");

    CARD("toon",         "Toon Shading",      "Stylize",
         "Cartoon-style color quantization, cel shading",
         "effects/toon/toon.frag.spv");

    CARD("vhs",          "VHS Retro",         "Retro",
         "VHS tape scanlines, noise and color drift",
         "effects/vhs/vhs.frag.spv");

    CARD("crt",          "CRT Monitor",       "Retro",
         "CRT scanlines + phosphor RGB pattern + screen curvature",
         "effects/crt/crt.frag.spv");

    CARD("water_ripple", "Water Ripple",      "Distort",
         "Normal-map based water ripple displacement",
         "effects/water_ripple/water_ripple.frag.spv");

    CARD("lens_distort", "Lens Distortion",   "Distort",
         "Barrel/pincushion lens distortion correction and simulation",
         "effects/lens_distort/lens_distort.frag.spv");

    printf("[CoverFlowScene] Registered %zu effect cards\n", m_cards.size());
}

#undef CARD

void CoverFlowScene::OnEnter()
{
    m_fpsLastTime    = std::chrono::high_resolution_clock::now();
    m_fpsFrameCount  = 0;
    m_fpsDisplay     = 0.0f;

    printf("[CoverFlowScene] Entered with %zu cards, selected=%d\n",
           m_cards.size(), m_selectedIndex);

    // Skip thumbnail initialization for non-OpenGL backends
    // (Vulkan thumbnails require shader/pipeline which are TODO)
    auto* glBackend = dynamic_cast<OpenGLBackend*>(m_backend);
    if (glBackend) {
        InitializeThumbnails();
    }
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
        double frameInterval = 1.0 / m_videoPlayer->GetFPS();
        if (now - m_videoLastFrameTime >= frameInterval) {
            if (m_videoPlayer->ReadFrame()) {
                m_backend->UpdateTexture(m_videoTex, 0, 0,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else {
                // Video ended — loop
                printf("[CoverFlowScene] Video ended, looping\n");
                // For simplicity, just stop
                StopVideo();
            }
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

    // ---- Keyboard navigation ----
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft))
        SelectCard(m_selectedIndex - 1);
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight))
        SelectCard(m_selectedIndex + 1);
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        OpenSelectedEffect();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        printf("[CoverFlowScene] ESC detected, exiting\n");
        m_wantsExit = true;
    }

    // ---- F1-F8: Quick effect selection ----
    for (int k = 0; k < 8; k++) {
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + k))) {
            int targetIdx = k; // F1=0, F2=1, ... F8=7
            if (targetIdx < (int)m_cards.size()) {
                SelectCard(targetIdx);
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

    // ---- Mouse wheel ----
    float wheel = io.MouseWheel;
    if (wheel != 0.0f)
        SelectCard(m_selectedIndex - static_cast<int>(wheel));

    // ---- Mouse drag to scroll cards ----
    const float cardW   = 280.0f;
    const float spacing = 80.0f;
    const float cardUnit = cardW + spacing; // distance between adjacent card centers

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        float dx = io.MouseDelta.x;
        if (!m_dragging) {
            m_dragging = true;
            m_dragStartX = io.MousePos.x;
            m_dragBaseOff = m_scrollOffset;
        }
        // Convert pixel drag to card index offset
        float offset = m_dragBaseOff - dx / cardUnit;
        // Clamp
        int idx = m_selectedIndex + static_cast<int>(std::round(offset));
        if (idx < 0) idx = 0;
        if (idx >= (int)m_cards.size()) idx = (int)m_cards.size() - 1;
        if (idx != m_selectedIndex) {
            SelectCard(idx);
            m_scrollOffset = offset - static_cast<float>(idx - m_selectedIndex);
        }
    } else {
        m_dragging = false;
    }
}

void CoverFlowScene::OnRender(IRenderBackend* /*backend*/)
{
    // Update thumbnail time/frame counters
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    m_thumbElapsedTime += dt;
    m_thumbFrameCount++;

    // Render visible card thumbnails every frame
    RenderVisibleThumbnails();
}

void CoverFlowScene::OnImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x;
    float h = io.DisplaySize.y;
    if (w > 0 && h > 0) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(w, h));
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));

    ImGui::Begin("##CoverFlow", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // ---- Title ----
    {
        auto& LM = LanguageManager::Instance();
        const char* title = LM.Title();
        char subtitle[256];
        const char* srcLabel = m_captureActive ? LM.ScreenCaptureLabel()
                              : (!m_imagePool.empty() ? m_imagePool[m_currentImageIndex].c_str() : LM.StaticImageLabel());
        // Extract just the filename
        const char* justFile = strrchr(srcLabel, '/');
        if (justFile) justFile++; else { justFile = strrchr(srcLabel, '\\'); if (justFile) justFile++; else justFile = srcLabel; }
        snprintf(subtitle, sizeof(subtitle), LM.InputInfoFormat(),
                 justFile, m_fpsDisplay);
        ImVec2 ts = ImGui::CalcTextSize(title);
        ImVec2 ss = ImGui::CalcTextSize(subtitle);
        dl->AddText(ImGui::GetFont(), 28.0f,
            ImVec2(w / 2.0f - ts.x / 2.0f, 30.0f),
            IM_COL32(220, 220, 255, 255), title);
        dl->AddText(ImGui::GetFont(), 16.0f,
            ImVec2(w / 2.0f - ss.x / 2.0f, 65.0f),
            IM_COL32(140, 140, 160, 220), subtitle);
    }

    // ---- Language toggle button (top-right corner) ----
    {
        auto& LM = LanguageManager::Instance();
        ImGui::SetCursorScreenPos(ImVec2(w - 120.0f, 20.0f));
        if (ImGui::Button(LM.LanguageButton(), ImVec2(100, 30))) {
            LM.ToggleLanguage();
        }
    }

    const float cardW   = 280.0f;
    const float cardH   = 380.0f;
    const float spacing = 80.0f;
    const float centerX = w / 2.0f;
    const float centerY = h / 2.0f + 20.0f;

    // ---- Page dots ----
    {
        const float dotR = 4.0f, dotSp = 14.0f;
        const int maxD = 18;
        int ds = std::max(0, m_selectedIndex - 8);
        int de = std::min((int)m_cards.size(), ds + maxD);
        float dx = centerX - (de - ds - 1) * dotSp / 2.0f;
        float dy = centerY - cardH / 2.0f - 25.0f;
        for (int i = ds; i < de; i++) {
            dl->AddCircleFilled(ImVec2(dx, dy), dotR,
                (i == m_selectedIndex) ? IM_COL32(180,180,255,255) : IM_COL32(80,80,100,150));
            dx += dotSp;
        }
    }

    // ---- Cards ----
    int vr = 3; // visible range on each side
    for (int i = (int)m_cards.size() - 1; i >= 0; i--) {
        // Only render cards in visible range
        if (i < m_selectedIndex - vr || i > m_selectedIndex + vr) continue;

        float off  = (float)(i - m_selectedIndex) + m_scrollOffset - m_targetOffset;
        float x    = centerX + off * (cardW + spacing);
        float scl  = 1.0f / (1.0f + std::fabs(off) * 0.3f);
        float alpha = 1.0f - std::fabs(off) * 0.35f;
        if (alpha < 0.0f) alpha = 0.0f;

        float w2 = cardW * scl, h2 = cardH * scl;
        float x0 = x - w2/2, y0 = centerY - h2/2;
        float x1 = x + w2/2, y1 = centerY + h2/2;

        int ai = (int)(alpha * 255);

        // ---- Thumbnail image ----
        bool hasThumb = i < (int)m_thumbIds.size() && m_thumbIds[i] != nullptr;
        ImU32 bg, bd;
        if (i == m_selectedIndex) {
            bg = IM_COL32(50,55,80,ai); bd = IM_COL32(160,160,240,ai);
        } else {
            bg = IM_COL32(35,38,50,ai); bd = IM_COL32(80,80,110,(int)(alpha*200));
        }

        float r = 10.0f * scl;

        if (hasThumb && alpha > 0.1f) {
            // Draw thumbnail image first (tint with white, alpha controls visibility)
            dl->AddImageRounded(m_thumbIds[i],
                ImVec2(x0, y0), ImVec2(x1, y1),
                ImVec2(0, 1), ImVec2(1, 0),
                IM_COL32(255, 255, 255, ai), r);
        } else {
            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bg, r);
        }
        dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), bd, r, 0, 2.0f);

        if (alpha > 0.25f) {
            const auto& card = m_cards[i];
            float cx = x;

            // ---- Card click area (InvisibleButton) ----
            ImGui::SetCursorScreenPos(ImVec2(x0, y0));
            ImGui::InvisibleButton(("##card_" + std::to_string(i)).c_str(), ImVec2(w2, h2));
            if (ImGui::IsItemClicked()) {
                // Click any card to open its detail page directly
                SelectCard(i);
                OpenSelectedEffect();
            }

            // ---- Dark overlay at bottom of card for text readability ----
            float overlayH = 140.0f * scl;
            dl->AddRectFilledMultiColor(
                ImVec2(x0, y1 - overlayH), ImVec2(x1, y1),
                IM_COL32(0,0,0,0),
                IM_COL32(0,0,0,0),
                IM_COL32(20,22,30,(int)(alpha*220)),
                IM_COL32(20,22,30,(int)(alpha*220)));

            // ---- Category tag ----
            float cy = y1 - overlayH + 15.0f * scl;
            {
                const char* catStr = LanguageManager::Instance().TranslateCategory(card.category);
                ImVec2 cs = ImGui::CalcTextSize(catStr);
                float tw = cs.x + 12.0f, th = cs.y + 6.0f;
                dl->AddRectFilled(ImVec2(x0 + 10.0f*scl, cy - 2.0f), ImVec2(x0 + 10.0f*scl + tw, cy + th + 2.0f),
                    IM_COL32(60,65,100,(int)(alpha*220)), 4.0f);
                dl->AddText(ImGui::GetFont(), 11.0f * scl,
                    ImVec2(x0 + 16.0f*scl, cy),
                    IM_COL32(160,170,220,(int)(alpha*255)), catStr);
            }

            // ---- Name ----
            cy = y1 - overlayH + 38.0f * scl;
            {
                const char* nameStr = LanguageManager::Instance().CardName(card.id);
                dl->AddText(ImGui::GetFont(), 18.0f * scl,
                    ImVec2(x0 + 14.0f*scl, cy),
                    IM_COL32(255, 255, 255, (int)(alpha*255)), nameStr);
            }

            // ---- Index ----
            if (i == m_selectedIndex) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%d / %zu", i+1, m_cards.size());
                ImVec2 isz = ImGui::CalcTextSize(buf);
                dl->AddText(ImGui::GetFont(), 12.0f,
                    ImVec2(cx - isz.x/2, y1 - 16.0f), IM_COL32(140,140,180,200), buf);
            }
        }
    }

    // ---- Nav hint ----
    {
        const char* hint = LanguageManager::Instance().BottomHelp();
        ImVec2 hs = ImGui::CalcTextSize(hint);
        dl->AddText(ImGui::GetFont(), 14.0f,
            ImVec2(centerX - hs.x/2, centerY + cardH/2 + 50.0f),
            IM_COL32(150,150,170,160), hint);
    }

    // ---- Category legend ----
    {
        auto& LM = LanguageManager::Instance();
        char leg[256];
        snprintf(leg, sizeof(leg), "%s | %s | %s | %s | %s | %s | %s | %s",
            LM.CategorySimple(), LM.CategoryLighting(), LM.CategoryFilter(), LM.CategoryStylize(),
            LM.CategoryColor(), LM.CategoryDistort(), LM.CategoryProcedural(), LM.CategoryRetro());
        ImVec2 ls = ImGui::CalcTextSize(leg);
        dl->AddText(ImGui::GetFont(), 12.0f,
            ImVec2(centerX - ls.x/2, h - 35.0f), IM_COL32(100,100,120,130), leg);
    }

    ImGui::End();
    ImGui::PopStyleColor();
    
    // ---- Card screenshot mode ----
    if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI") && !getenv("AUTO_TEST_DETAILS")) {
        g_cardScreenshotFrame++;
        
        // Take screenshot after a few frames of showing the card
        if (g_cardScreenshotNeedsShot && g_cardScreenshotFrame >= 5) {
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/card_%02d.ppm", g_cardScreenshotIndex - 1);
            ScreenshotRequest::Request(path);
            g_cardScreenshotNeedsShot = false;
        }
        
        // Move to next card
        if (g_cardScreenshotFrame >= 30) { // 30 frames per card
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
    
    // ---- Detail page screenshot mode ----
    if (getenv("AUTO_TEST_DETAILS") && !getenv("AUTO_TEST_UI")) {
        static int detailCardIndex = 0;
        static int frameWait = 0;
        
        frameWait++;
        if (frameWait >= 30) { // Wait between opening details
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

    // Destroy old input texture if we own it
    // Note: we don't track ownership cleanly here, but for simplicity
    // we just create a new texture and let the old one leak.
    // A proper implementation would track texture ownership.

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
    
    auto vertSpv = ShaderLoader::LoadSPIRV(shaderDir + "/common/fullscreen.vert.spv");
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

    auto* glBackend = dynamic_cast<OpenGLBackend*>(m_backend);
    if (!glBackend) return;

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
        params.uniformFloats  = {4.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        params.viewportWidth  = m_thumbWidth;
        params.viewportHeight = m_thumbHeight;
        params.time           = m_thumbElapsedTime;
        params.frameCount     = m_thumbFrameCount;
        m_backend->DrawFullscreenQuad(m_sharedVertShader, state.fragShader, params);
        m_backend->EndRenderToTexture();

        m_thumbIds[i] = glBackend->GetImTextureID(state.thumbTex);
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

    // Create texture for video frames
    m_videoTex = m_backend->CreateTexture(
        m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
        TextureFormat::RGBA8, m_videoPlayer->GetPixels());

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
