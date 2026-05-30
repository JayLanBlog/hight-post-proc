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
#include <cmath>

#include "stb_image.h"

EffectDetailScene::EffectDetailScene(const EffectCard& card, TextureHandle inputTex)
    : m_card(card)
    , m_inputTex(inputTex)
{
}

EffectDetailScene::~EffectDetailScene()
{
    // Release GL resources to prevent accumulation across scene switches
    if (m_backend) {
        if (m_vertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_vertShader);
            m_vertShader = INVALID_SHADER;
        }
        if (m_fragShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_fragShader);
            m_fragShader = INVALID_SHADER;
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_effectTex);
            m_effectTex = {0};
            m_effectTexCreated = false;
        }
    }
    StopVideo();
}

void EffectDetailScene::SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime) {
    m_videoPlayer = std::move(player);
    m_videoTex = videoTex;
    m_videoActive = active;
    m_videoLastFrameTime = lastFrameTime;
    if (active) {
        m_inputTex = m_videoTex;  // Use video texture as input for effect rendering
    }
}

void EffectDetailScene::OnEnter()
{
    if (!m_backend) {
        fprintf(stderr, "[EffectDetailScene] OnEnter called without backend!\n");
        return;
    }

    // Initialize debug panel with effect parameters (works on all backends)
    m_debugPanel.SetParams(m_card.params);
    
    // Initialize uniform values from card params defaults
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
    m_expectedFloatCount = m_uniformFloats.size();

    // Load SPIR-V shaders (works on all backends)
    auto vertSpirv = ShaderLoader::LoadSPIRV(m_card.vertSpirvPath);
    auto fragSpirv = ShaderLoader::LoadSPIRV(m_card.fragSpirvPath);

    if (vertSpirv.empty() || fragSpirv.empty()) {
        fprintf(stderr, "[EffectDetailScene] Failed to load SPIR-V shaders\n");
        return;
    }

    // Always use SPIR-V shaders (consistent with CoverFlowScene thumbnail path).
    // GLSL+UBO has unreliable reflection on NVIDIA when vertex/fragment shader
    // types are mixed (SPIR-V vs GLSL). Pure SPIR-V works correctly in all cases.
    m_vertShader = m_backend->CreateVertexShader(vertSpirv.data(), vertSpirv.size());
    m_fragShader = m_backend->CreateFragmentShader(fragSpirv.data(), fragSpirv.size());
    printf("[EffectDetailScene] Using SPIR-V shaders\n");

    if (m_vertShader.id == INVALID_SHADER.id || m_fragShader.id == INVALID_SHADER.id) {
        fprintf(stderr, "[EffectDetailScene] Failed to create shaders\n");
        return;
    }
}

void EffectDetailScene::OnExit()
{
    // Stop video playback
    StopVideo();

    // Release GL resources (destructor also does this as safety net)
    if (m_backend) {
        if (m_vertShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_vertShader);
            m_vertShader = INVALID_SHADER;
        }
        if (m_fragShader.id != INVALID_SHADER.id) {
            m_backend->DestroyShader(m_fragShader);
            m_fragShader = INVALID_SHADER;
        }
        if (m_effectTexCreated && m_effectTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_effectTex);
            m_effectTex = {0};
            m_effectTexCreated = false;
        }
    }

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
    if (m_videoActive && m_videoPlayer && m_videoPlayer->IsOpen() && m_backend
        && dynamic_cast<OpenGLBackend*>(m_backend)) {
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
    m_viewportWidth = width;
    m_viewportHeight = height;

    if (m_compareMode) {
        RenderCompareView(backend);
    } else {
        RenderFullscreenEffect(backend);
    }
}

void EffectDetailScene::RenderFullscreenEffect(IRenderBackend* backend)
{
    ShaderParams params;
    params.inputTextures.push_back(m_inputTex);
    params.uniformFloats = m_uniformFloats;
    params.uniformInts = m_uniformInts;
    params.time = m_time;
    params.frameCount = m_frameCount;
    params.viewportWidth = m_viewportWidth;
    params.viewportHeight = m_viewportHeight;

    backend->DrawFullscreenQuad(m_vertShader, m_fragShader, params);
}

void EffectDetailScene::RenderCompareView(IRenderBackend* backend)
{
    // 1. Ensure effect texture FBO exists and matches size
    EnsureEffectTexture();

    if (m_effectTex.id == INVALID_TEXTURE.id) return;

    // 2. Render effect to FBO texture
    backend->BeginRenderToTexture(m_effectTex);
    RenderFullscreenEffect(backend);
    backend->EndRenderToTexture();

    // 3. ImGui will handle the compare view display in OnImGui
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

    // --- Compare mode: slider before/after overlay view ---
    if (m_compareMode && m_effectTex.id != INVALID_TEXTURE.id && m_backend) {
        void* origImTex = m_backend->GetImTextureID(m_inputTex);
        void* effectImTex = m_backend->GetImTextureID(m_effectTex);

        if (origImTex && effectImTex) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
            ImGui::Begin("##CompareView", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            // Calculate display region
            ImVec2 winPos   = ImGui::GetWindowPos();
            ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
            ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
            ImVec2 displayMin = ImVec2(winPos.x + regionMin.x, winPos.y + regionMin.y);
            ImVec2 displayMax = ImVec2(winPos.x + regionMax.x, winPos.y + regionMax.y);
            ImVec2 displaySize(displayMax.x - displayMin.x, displayMax.y - displayMin.y);

            ImDrawList* dl = ImGui::GetWindowDrawList();

            // Split position in screen coordinates
            float splitX = displayMin.x + displaySize.x * m_compareSplitPos;

            // Draw original image as the base layer (full display area)
            // GL textures have bottom-left origin, ImGui expects top-left → flip Y
            dl->AddImage(origImTex, displayMin, displayMax, ImVec2(0, 1), ImVec2(1, 0));

            // Draw effect image on the right side of the split (clipped)
            // UV mapping: the left edge of the effect image maps to splitX
            float uvMinX = m_compareSplitPos;
            dl->AddImage(effectImTex,
                ImVec2(splitX, displayMin.y), displayMax,
                ImVec2(uvMinX, 1), ImVec2(1, 0));

            // Draw split line (blue, 3px)
            dl->AddLine(ImVec2(splitX, displayMin.y), ImVec2(splitX, displayMax.y),
                        IM_COL32(68, 175, 255, 255), 3.0f);

            // Draw drag handle (circle at center of display height)
            float handleY = (displayMin.y + displayMax.y) * 0.5f;
            dl->AddCircleFilled(ImVec2(splitX, handleY), 14.0f, IM_COL32(68, 175, 255, 255));
            dl->AddCircleFilled(ImVec2(splitX, handleY), 12.0f, IM_COL32(255, 255, 255, 255));

            // Draw handle icon (left/right arrows)
            dl->AddTriangleFilled(
                ImVec2(splitX - 6, handleY - 3), ImVec2(splitX - 2, handleY), ImVec2(splitX - 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));
            dl->AddTriangleFilled(
                ImVec2(splitX + 6, handleY - 3), ImVec2(splitX + 2, handleY), ImVec2(splitX + 6, handleY + 3),
                IM_COL32(68, 175, 255, 255));

            // Labels: "Before" on left, "After" on right
            dl->AddText(ImVec2(displayMin.x + 10, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "Before");
            dl->AddText(ImVec2(displayMax.x - 60, displayMin.y + 10),
                        IM_COL32(255, 255, 255, 200), "After");

            // Handle mouse interaction for dragging the split
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            bool mouseInWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

            // Check if mouse is near the split line
            bool nearSplit = mouseInWindow &&
                std::abs(mousePos.x - splitX) < 20.0f &&
                mousePos.y >= displayMin.y && mousePos.y <= displayMax.y;

            // Set cursor to resize east-west when near split
            if (nearSplit || m_compareDragging) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            // Handle drag start (on mouse down)
            if (nearSplit && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_compareDragging = true;
            }

            // Handle drag update
            if (m_compareDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                float newSplit = (mousePos.x - displayMin.x) / displaySize.x;
                m_compareSplitPos = std::clamp(newSplit, 0.1f, 0.9f);
            }

            // Handle drag end
            if (m_compareDragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_compareDragging = false;
            }

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

        // Transfer video player back to CoverFlowScene
        if (m_videoActive && m_videoPlayer) {
            coverFlow->SetVideoPlayer(std::move(m_videoPlayer), m_videoTex, true, m_videoLastFrameTime);
            m_videoActive = false;
            m_videoTex = {0};
            printf("[EffectDetailScene] Transferred video player back to CoverFlow\n");
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
        // Destroy old input texture to prevent leak
        if (m_inputTex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(m_inputTex);
        }
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
