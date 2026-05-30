#include "app/EffectDetailScene.h"
#include "app/CoverFlowScene.h"
#include "app/Application.h"
#include "app/LanguageManager.h"
#include "shader/ShaderLoader.h"
#include "render/OpenGLBackend.h"
#include "input/VideoPlayer.h"

#include <imgui.h>
#include <cstdio>
#include <algorithm>

#include "stb_image.h"

EffectDetailScene::EffectDetailScene(const EffectCard& card, TextureHandle inputTex)
    : m_card(card)
    , m_inputTex(inputTex)
{
}

void EffectDetailScene::OnEnter()
{
    if (!m_backend) {
        fprintf(stderr, "[EffectDetailScene] OnEnter called without backend!\n");
        return;
    }

    // Load SPIR-V shaders
    auto vertSpirv = ShaderLoader::LoadSPIRV(m_card.vertSpirvPath);
    auto fragSpirv = ShaderLoader::LoadSPIRV(m_card.fragSpirvPath);

    if (vertSpirv.empty() || fragSpirv.empty()) {
        fprintf(stderr, "[EffectDetailScene] Failed to load SPIR-V shaders\n");
        return;
    }

    // Create shaders via backend
    // Try GLSL source first (more reliable for UBO in OpenGL), fallback to SPIR-V
    std::string vertGlslPath = m_card.vertSpirvPath;
    {
        size_t pos = vertGlslPath.rfind(".vert.spv");
        if (pos != std::string::npos) {
            vertGlslPath = vertGlslPath.substr(0, pos) + ".vert";
        } else {
            pos = vertGlslPath.rfind(".spv");
            if (pos != std::string::npos) {
                vertGlslPath = vertGlslPath.substr(0, pos) + ".vert";
            }
        }
    }
    
    // Build alternative path: source shaders are at project_root/shaders/, not build/shaders/
    // The SPIR-V path resolves to: <project>/shader-showcase/build/shaders/common/fullscreen.vert
    // Source GLSL is at:            <project>/shader-showcase/shaders/common/fullscreen.vert
    // Strategy: normalize the path, then remove "build" component
    std::string vertGlslPathAlt;
    {
        // Simple path normalization: replace / with \ and resolve ..
        std::string norm = vertGlslPath;
        for (auto& c : norm) { if (c == '/') c = '\\'; }
        // Remove "..\" by going up one directory
        while (true) {
            size_t dotdot = norm.find("..\\");
            if (dotdot == std::string::npos) break;
            // Find the directory before ..
            size_t prevSlash = norm.rfind('\\', dotdot - 2);
            if (prevSlash == std::string::npos) break;
            norm = norm.substr(0, prevSlash) + norm.substr(dotdot + 2);
        }
        // Now norm should be like: E:\...\shader-showcase\build\shaders\common\fullscreen.vert
        // Remove "build\" component
        size_t bp = norm.find("\\build\\");
        if (bp != std::string::npos) {
            vertGlslPathAlt = norm.substr(0, bp + 1) + norm.substr(bp + 7); // keep the leading \, skip "build\"
        } else {
            vertGlslPathAlt = norm;
        }
    }
    
    // Try loading vertex GLSL source
    std::string vertGlslSource;
    for (const auto& tryPath : {vertGlslPath, vertGlslPathAlt}) {
        std::string normPath = tryPath;
        for (auto& c : normPath) { if (c == '/') c = '\\'; }
        FILE* f = fopen(normPath.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            vertGlslSource.resize(size);
            fread(&vertGlslSource[0], 1, size, f);
            fclose(f);
            break;
        }
    }
    
    // Use GLSL for vertex shader (SPIR-V UBO reflection broken on NVIDIA)
    auto* glBackend = dynamic_cast<OpenGLBackend*>(m_backend);
    if (!vertGlslSource.empty() && glBackend) {
        m_vertShader = glBackend->CreateVertexShaderFromGLSL(vertGlslSource);
        printf("[EffectDetailScene] Using GLSL vertex shader\n");
    } else {
        m_vertShader = m_backend->CreateVertexShader(vertSpirv);
        printf("[EffectDetailScene] Using SPIR-V vertex shader\n");
    }
    
    // Try GLSL source first (more reliable for UBO in OpenGL), fallback to SPIR-V
    // GLSL source files are in the source tree, not in the build directory.
    // The SPIR-V path is like: build/bin/Release/../../shaders/effects/bloom/bloom.frag.spv
    // The GLSL source is at: shaders/effects/bloom/bloom.frag (relative to project root)
    // We need to go up one more level from the build directory.
    std::string fragGlslPath = m_card.fragSpirvPath;
    {
        size_t pos = fragGlslPath.rfind(".frag.spv");
        if (pos != std::string::npos) {
            fragGlslPath = fragGlslPath.substr(0, pos) + ".frag";
        } else {
            pos = fragGlslPath.rfind(".spv");
            if (pos != std::string::npos) {
                fragGlslPath = fragGlslPath.substr(0, pos) + ".frag";
            }
        }
    }
    
    // Also try source directory (one level up from build dir)
    std::string fragGlslPathAlt;
    {
        std::string norm = fragGlslPath;
        for (auto& c : norm) { if (c == '/') c = '\\'; }
        while (true) {
            size_t dotdot = norm.find("..\\");
            if (dotdot == std::string::npos) break;
            size_t prevSlash = norm.rfind('\\', dotdot - 2);
            if (prevSlash == std::string::npos) break;
            norm = norm.substr(0, prevSlash) + norm.substr(dotdot + 2);
        }
        size_t bp = norm.find("\\build\\");
        if (bp != std::string::npos) {
            fragGlslPathAlt = norm.substr(0, bp + 1) + norm.substr(bp + 7);
        } else {
            fragGlslPathAlt = norm;
        }
    }
    
    // Try loading GLSL source (try source dir first, then build dir)
    std::string fragGlslSource;
    for (const auto& tryPath : {fragGlslPathAlt, fragGlslPath}) {
        std::string normPath = tryPath;
        for (auto& c : normPath) { if (c == '/') c = '\\'; }
        printf("[EffectDetailScene] Trying GLSL path: %s\n", normPath.c_str());
        FILE* f = fopen(normPath.c_str(), "rb");
        if (f) {
            printf("[EffectDetailScene] Found GLSL: %s\n", normPath.c_str());
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            fragGlslSource.resize(size);
            fread(&fragGlslSource[0], 1, size, f);
            fclose(f);
            break;
        }
    }
    
    // Use GLSL source for fragment shader (SPIR-V UBO reflection broken on NVIDIA)
    if (!fragGlslSource.empty() && glBackend) {
        m_fragShader = glBackend->CreateFragmentShaderFromGLSL(fragGlslSource);
        printf("[EffectDetailScene] Using GLSL fragment shader\n");
    } else {
        m_fragShader = m_backend->CreateFragmentShader(fragSpirv);
        printf("[EffectDetailScene] GLSL not found, falling back to SPIR-V fragment shader\n");
    }

    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[EffectDetailScene] Failed to create shaders\n");
        return;
    }

    // Initialize debug panel with effect parameters
    m_debugPanel.SetParams(m_card.params);
    
    // Initialize uniform values from card params defaults
    // Always pad to 6 floats to match SPIR-V UBO layout
    m_uniformFloats.clear();
    m_uniformInts.clear();
    for (const auto& p : m_card.params) {
        switch (p.type) {
        case ParamType::Float:
            m_uniformFloats.push_back(p.defaultVal[0]);
            break;
        case ParamType::Int:
        case ParamType::Bool:
            m_uniformInts.push_back(static_cast<int32_t>(p.defaultVal[0]));
            break;
        case ParamType::Float2:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            break;
        case ParamType::Float3:
        case ParamType::Color:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            break;
        case ParamType::Float4:
            m_uniformFloats.push_back(p.defaultVal[0]);
            m_uniformFloats.push_back(p.defaultVal[1]);
            m_uniformFloats.push_back(p.defaultVal[2]);
            m_uniformFloats.push_back(p.defaultVal[3]);
            break;
        }
    }
    // Save expected float count (actual shader params, NOT padded)
    m_expectedFloatCount = m_uniformFloats.size();
}

void EffectDetailScene::OnExit()
{
    // Stop video playback
    StopVideo();

    // Don't destroy shaders here — let the backend handle cleanup.
    // Destroying shaders during scene transition can cause GL state issues.
    // The backend will clean up all resources on shutdown.
    m_vertShader = INVALID_SHADER;
    m_fragShader = INVALID_SHADER;

    printf("[EffectDetailScene] Exited: %s\n", m_card.name.c_str());
}

void EffectDetailScene::OnUpdate(float dt)
{
    m_time += dt;
    m_frameCount++;

    // Auto-test: auto-return after holdFrames
    if (m_autoTestHoldFrames > 0) {
        m_autoTestHoldFrames--;
        if (m_autoTestHoldFrames <= 0) {
            m_wantsExit = true;
            m_returnToCoverFlow = true;
            printf("[EffectDetailScene] Auto-test timer expired, returning to CoverFlow\n");
        }
    }

    // ESC returns to CoverFlow
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_wantsExit         = true;
        m_returnToCoverFlow = true;
        printf("[EffectDetailScene] ESC pressed, returning to CoverFlow\n");
    }

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
                    LoadVideoFromFile(dropped);
                } else {
                    StopVideo();
                    LoadImageFromFile(dropped);
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
                m_backend->UpdateTexture(m_videoTex,
                    m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
                    m_videoPlayer->GetPixels());
                m_inputTex = m_videoTex;
                m_videoLastFrameTime = now;
            } else {
                // Video ended — loop
                printf("[EffectDetailScene] Video ended, stopping playback\n");
                StopVideo();
            }
        }
    }
}

void EffectDetailScene::EnsureEffectTexture()
{
    if (m_effectTexCreated || !m_backend) return;
    int w = 0, h = 0;
    m_backend->GetFramebufferSize(w, h);
    if (w <= 0 || h <= 0) return;
    m_effectTex = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    m_effectTexCreated = true;
}

void EffectDetailScene::OnRender(IRenderBackend* backend)
{
    if (!backend) return;
    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) return;

    // Sync uniform values from debug panel
    m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
    
    // Restore correct UBO float count (DebugPanel may shrink the array)
    if (m_uniformFloats.size() != m_expectedFloatCount) {
        m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
    }

    // Get framebuffer size
    int width = 0, height = 0;
    backend->GetFramebufferSize(width, height);
    if (width <= 0 || height <= 0) return;

    // Ensure effect texture exists and matches size
    EnsureEffectTexture();

    // Fill ShaderParams
    ShaderParams params;
    params.inputTextures.push_back(m_inputTex);
    params.uniformFloats  = m_uniformFloats;
    params.uniformInts    = m_uniformInts;
    params.viewportWidth  = width;
    params.viewportHeight = height;
    params.time           = m_time;
    params.frameCount     = m_frameCount;

    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id) {
        // Render effect to FBO texture
        backend->BeginRenderToTexture(m_effectTex);
        backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
        backend->EndRenderToTexture();
    } else {
        // Render effect directly to screen
        backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
    }
}

void EffectDetailScene::OnImGui()
{
    // TAB to toggle debug panel visibility
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_showDebug = !m_showDebug;
    }
    // C to toggle compare mode
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_compareMode = !m_compareMode;
    }

    // Get framebuffer size for positioning
    int width = 0, height = 0;
    if (m_backend) {
        m_backend->GetFramebufferSize(width, height);
    }

    // --- Compare mode: before/after split view ---
    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id && m_backend) {
        void* origImTex = nullptr;
        void* effectImTex = nullptr;
        if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend)) {
            origImTex = gl->GetImTextureID(m_inputTex);
            effectImTex = gl->GetImTextureID(m_effectTex);
        }

        if (origImTex && effectImTex) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
            ImGui::Begin("##CompareView", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            ImVec2 avail = ImGui::GetContentRegionAvail();

            // Draw original (left side)
            ImGui::Image(origImTex, ImVec2(avail.x * m_compareSplitPos, avail.y), ImVec2(0,0), ImVec2(1,1));
            ImGui::SameLine(0, 0);

            // Draw effect (right side)
            ImGui::Image(effectImTex, ImVec2(avail.x * (1.0f - m_compareSplitPos), avail.y), ImVec2(0,0), ImVec2(1,1));

            // Draggable split line
            ImGui::SetCursorScreenPos(ImVec2(avail.x * m_compareSplitPos - 2.0f, 0));
            ImGui::InvisibleButton("##split", ImVec2(4, (float)height));
            if (ImGui::IsItemActive()) {
                float delta = ImGui::GetIO().MouseDelta.x / avail.x;
                m_compareSplitPos = std::clamp(m_compareSplitPos + delta, 0.05f, 0.95f);
            }
            // Draw split line indicator
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float lineX = avail.x * m_compareSplitPos;
            dl->AddLine(ImVec2(lineX, 0), ImVec2(lineX, (float)height),
                        IM_COL32(255, 255, 255, 200), 2.0f);

            // Labels
            dl->AddText(ImVec2(10, 10), IM_COL32(255,255,255,180), LanguageManager::Instance().CompareOriginal());
            dl->AddText(ImVec2(lineX + 10, 10), IM_COL32(255,255,255,180), LanguageManager::Instance().CompareEffect());

            ImGui::End();
        }
    }

    // --- InfoBar at bottom of screen ---
    {
        const float barHeight = 60.0f;
        ImGui::SetNextWindowPos(ImVec2(0, (float)height - barHeight));
        ImGui::SetNextWindowSize(ImVec2((float)width, barHeight));
        ImGui::Begin("##InfoBar", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.9f), "%s", LanguageManager::Instance().CardName(m_card.id));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 0.7f), "%s", LanguageManager::Instance().EscReturn());
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", LanguageManager::Instance().CardDesc(m_card.id));
        ImGui::End();
    }

    // --- Debug Panel ---
    if (m_showDebug) {
        ImGui::Begin(LanguageManager::Instance().EffectParams(), &m_showDebug);
        m_debugPanel.Render(&m_showDebug);
        ImGui::End();
        // Update uniform values after UI interaction
        m_debugPanel.SetUniformValues(m_uniformFloats, m_uniformInts);
        if (m_uniformFloats.size() != m_expectedFloatCount) {
            m_uniformFloats.resize(m_expectedFloatCount, 0.0f);
        }
    }

    // --- Asset Library Panel ---
    if (m_showDebug && !m_savedState.imagePool.empty()) {
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(LanguageManager::Instance().AssetLibrary(), &m_showDebug)) {
            // Images section
            if (ImGui::CollapsingHeader(LanguageManager::Instance().Images())) {
                for (size_t i = 0; i < m_savedState.imagePool.size(); i++) {
                    const std::string& path = m_savedState.imagePool[i];
                    // Extract filename for display
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadImageFromFile(path);
                    }
                }
            }
            // Videos section
            if (!m_savedState.videoPool.empty() && ImGui::CollapsingHeader(LanguageManager::Instance().Videos())) {
                for (size_t i = 0; i < m_savedState.videoPool.size(); i++) {
                    const std::string& path = m_savedState.videoPool[i];
                    std::string fname = path;
                    size_t pos = path.find_last_of("/\\");
                    if (pos != std::string::npos) fname = path.substr(pos + 1);
                    if (ImGui::SmallButton(fname.c_str())) {
                        LoadVideoFromFile(path);
                    }
                }
            }
        }
        ImGui::End();
    }
    
    // ---- Detail page screenshot mode ----
    if (getenv("AUTO_TEST_DETAILS")) {
        static int detailScreenshotIndex = 0;
        static int frameCounter = 0;
        static bool needsScreenshot = true;  // Start with true to capture first frame
        
        frameCounter++;
        
        // Take screenshot after UI renders
        if (needsScreenshot && frameCounter >= 5) {
            char path[256];
            snprintf(path, sizeof(path), "e:/AI/graph/hight-post-proc/screenshots/detail_%02d.ppm", detailScreenshotIndex);
            ScreenshotRequest::Request(path);
            printf("[DetailScreenshot] Requested screenshot: %s\n", path);
            needsScreenshot = false;
        }
        
        // Exit after showing for a while
        if (frameCounter >= 30) {
            frameCounter = 0;
            detailScreenshotIndex++;
            if (detailScreenshotIndex < 18) {
                needsScreenshot = true;
            }
            // Return to coverflow
            m_returnToCoverFlow = true;
            m_wantsExit = true;
        }
    }
}

std::unique_ptr<Scene> EffectDetailScene::GetNextScene()
{
    if (m_returnToCoverFlow) {
        printf("[EffectDetailScene] Restoring CoverFlowScene with full state\n");
        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetInputTexture(m_savedState.inputTex);
        coverFlow->SetInputTexCache(m_savedState.inputTexCache);
        coverFlow->SetBackend(m_savedState.backend);
        coverFlow->SetApplication(m_savedState.app);
        // Thumbnails are now initialized internally by CoverFlowScene::OnEnter
        coverFlow->SetTestImageBaseDir(m_savedState.testImageBaseDir);

        // Restore image pool
        for (const auto& img : m_savedState.imagePool) {
            coverFlow->AddImageToPool(img);
        }

        // Restore auto-test state
        if (m_savedState.autoTest) {
            coverFlow->ResumeAutoTest(m_savedState.autoTestHoldFrames, m_savedState.autoTestCardIndex);
        }

        printf("[EffectDetailScene] CoverFlowScene restored (thumbs=%zu, pool=%zu)\n",
               m_savedState.thumbIds.size(), m_savedState.imagePool.size());
        return coverFlow;
    }
    return nullptr;
}

// ============================================================================
// Load image from file (drag-drop support)
// ============================================================================

void EffectDetailScene::LoadImageFromFile(const std::string& path)
{
    if (!m_backend) return;

    int iw, ih, comp;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data = stbi_load(path.c_str(), &iw, &ih, &comp, 4);
    stbi_set_flip_vertically_on_load(false);
    if (!data) {
        fprintf(stderr, "[EffectDetailScene] Cannot load: %s\n", path.c_str());
        return;
    }

    printf("[EffectDetailScene] Loading image: %s (%d x %d)\n", path.c_str(), iw, ih);

    TextureHandle newTex = m_backend->CreateTexture(iw, ih, TextureFormat::RGBA8, data);
    stbi_image_free(data);

    if (newTex.id != INVALID_TEXTURE.id) {
        m_inputTex = newTex;
        // Also update saved state so CoverFlow gets the new texture
        m_savedState.inputTex = newTex;
    }
}

// ============================================================================
// Load video from file (drag-drop support)
// ============================================================================

void EffectDetailScene::LoadVideoFromFile(const std::string& path)
{
    if (!m_backend) return;

    // Stop any existing video
    StopVideo();

    m_videoPlayer = std::make_unique<VideoPlayer>();
    if (!m_videoPlayer->Open(path)) {
        fprintf(stderr, "[EffectDetailScene] Cannot open video: %s\n", path.c_str());
        m_videoPlayer.reset();
        return;
    }

    // Create texture for video frames
    m_videoTex = m_backend->CreateTexture(
        m_videoPlayer->GetWidth(), m_videoPlayer->GetHeight(),
        TextureFormat::RGBA8, m_videoPlayer->GetPixels());

    if (m_videoTex.id == INVALID_TEXTURE.id) {
        fprintf(stderr, "[EffectDetailScene] Cannot create video texture\n");
        m_videoPlayer->Close();
        m_videoPlayer.reset();
        return;
    }

    m_inputTex = m_videoTex;
    m_videoActive = true;
    m_videoLastFrameTime = ImGui::GetTime();
    printf("[EffectDetailScene] Playing video: %s\n", path.c_str());
}

void EffectDetailScene::StopVideo()
{
    if (m_videoPlayer) {
        m_videoPlayer->Close();
        m_videoPlayer.reset();
    }
    m_videoActive = false;
    // Note: we don't destroy m_videoTex here because it might still be referenced
    // A proper implementation would use reference counting.
}
