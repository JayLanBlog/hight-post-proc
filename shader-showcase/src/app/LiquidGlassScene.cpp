#include "LiquidGlassScene.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdint>

static std::vector<uint32_t> ReadSPIRV(const char* relPath) {
    const char* tries[] = {"shaders/", "build/shaders/"};
    for (int t = 0; t < 2; t++) {
        std::string path = tries[t] + std::string(relPath);
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f.is_open()) continue;
        size_t sz = f.tellg(); f.seekg(0);
        std::vector<uint32_t> data((sz+3)/4);
        f.read((char*)data.data(), sz);
        fprintf(stderr,"[LG] ReadSPIRV OK: %s (%zu bytes)\n", path.c_str(), sz);
        return data;
    }
    fprintf(stderr,"[LG] ReadSPIRV failed: %s\n", relPath);
    return {};
}

LiquidGlassScene::LiquidGlassScene() {
    m_fpsLastTime = std::chrono::high_resolution_clock::now();
}

LiquidGlassScene::~LiquidGlassScene() = default;

void LiquidGlassScene::CreateRTs(int w, int h) {
    DestroyRTs();
    int bw = (int)(w * m_blurDownscale);
    int bh = (int)(h * m_blurDownscale);
    m_rtA = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, nullptr);
    m_rtB = m_backend->CreateTexture(bw, bh, TextureFormat::RGBA8, nullptr);
    m_rtC = m_backend->CreateTexture(bw, bh, TextureFormat::RGBA8, nullptr);
    m_lastRTSizeW = w; m_lastRTSizeH = h;
    fprintf(stderr, "[LG] RTs created: A=%dx%d, B/C=%dx%d\n", w, h, bw, bh);
}

void LiquidGlassScene::DestroyRTs() {
    if (m_rtA.id) { m_backend->DestroyTexture(m_rtA); m_rtA = {0}; }
    if (m_rtB.id) { m_backend->DestroyTexture(m_rtB); m_rtB = {0}; }
    if (m_rtC.id) { m_backend->DestroyTexture(m_rtC); m_rtC = {0}; }
}

void LiquidGlassScene::OnEnter() {
    printf("[LiquidGlass] OnEnter\n");

    // 加载顶点 Shader
    auto vd = ReadSPIRV("common/fullscreen_vk.vert.spv");
    if (!vd.empty()) m_sharedVert = m_backend->CreateVertexShader(vd.data(), vd.size());

    // 加载 Fragment Shaders
    auto bgd = ReadSPIRV("liquid_glass/lg_bg.frag.spv");
    if (!bgd.empty()) m_bgShader = m_backend->CreateFragmentShader(bgd.data(), bgd.size());

    auto bld = ReadSPIRV("liquid_glass/lg_blur.frag.spv");
    if (!bld.empty()) m_blurShader = m_backend->CreateFragmentShader(bld.data(), bld.size());

    auto gld = ReadSPIRV("liquid_glass/lg_glass.frag.spv");
    if (!gld.empty()) m_glassShader = m_backend->CreateFragmentShader(gld.data(), gld.size());

    // 加载背景纹理
    const char* bgFiles[] = {
        "background-cubes.jpg", "background-spring.png", "background-summer.png",
        "background-autumn.png", "background-winter.png",
        "Seasonal Landscape 1.png", "Seasonal Landscape 2.png",
        "Newspaper.png", "Cartoon Cottage.png", "anime.png",
        "background-progress-bar.jpg"
    };
    const char* bgNames[] = {
        "Cubes", "Spring", "Summer", "Autumn", "Winter",
        "Landscape 1", "Landscape 2", "Newspaper", "Cartoon Cottage", "Anime",
        "Progress Bar"
    };
    for (int i = 0; i < 11; i++) {
        std::string path = std::string("assets/textures/liquid_glass/") + bgFiles[i];
        TextureHandle tex = m_backend->CreateTextureFromFile(path);
        if (tex.id != 0) {
            m_bgTextures.push_back(tex);
            m_bgNames.push_back(bgNames[i]);
            printf("[LiquidGlass] Loaded: %s (id=%d)\n", path.c_str(), tex.id);
        } else {
            printf("[LiquidGlass] FAILED: %s\n", path.c_str());
        }
    }
    if (m_bgTextures.empty()) {
        printf("[LiquidGlass] WARNING: No background textures loaded!\n");
    }
}

void LiquidGlassScene::OnExit() {
    printf("[LiquidGlass] OnExit\n");
    DestroyRTs();
    if (m_sharedVert.id) { m_backend->DestroyShader(m_sharedVert); m_sharedVert = {0}; }
    if (m_bgShader.id) { m_backend->DestroyShader(m_bgShader); m_bgShader = {0}; }
    if (m_blurShader.id) { m_backend->DestroyShader(m_blurShader); m_blurShader = {0}; }
    if (m_glassShader.id) { m_backend->DestroyShader(m_glassShader); m_glassShader = {0}; }
    for (auto& t : m_bgTextures) { if (t.id) m_backend->DestroyTexture(t); }
    m_bgTextures.clear();
}

void LiquidGlassScene::OnUpdate(float dt) {
    m_elapsedTime += dt;
    m_frameCount++;
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) m_wantsReturn = true;
    // FPS 计算
    m_fpsFrameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    float e = std::chrono::duration<float>(now - m_fpsLastTime).count();
    if (e >= 1.0f) { m_fpsDisplay = m_fpsFrameCount / e; m_fpsFrameCount = 0; m_fpsLastTime = now; }
}

void LiquidGlassScene::OnRender(IRenderBackend* be) {
    if (!be || !m_sharedVert.id || !m_bgShader.id || !m_blurShader.id || !m_glassShader.id)
        return;
    if (m_bgTextures.empty()) return;

    int fw = 1280, fh = 720;
    be->GetFramebufferSize(fw, fh);

    // 按需重建 RT
    int bw = (int)(fw * m_blurDownscale);
    int bh = (int)(fh * m_blurDownscale);
    if (m_lastRTSizeW != fw || m_lastRTSizeH != fh) {
        CreateRTs(fw, fh);
    }

    TextureHandle bgTex = m_bgTextures[m_currentBgIndex];

    // === Stage 1: 背景渲染 ===
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {bgTex};
        be->BeginRenderToTexture(m_rtA);
        be->DrawFullscreenQuad(m_sharedVert, m_bgShader, p);
        be->EndRenderToTexture();
    }

    // === Stage 2: 高斯模糊 ===
    // 参考: BlurPass::Run 首次迭代使用输入FBO全分辨率作为u_resolution
    if (m_blurIters > 0) {
        for (int i = 0; i < m_blurIters; i++) {
            // 首次迭代: uRes=全分辨率(fw,fh) 匹配参考 fb(1600x900)
            // 后续迭代: uRes=降采样(bw,bh) 匹配参考 blurFinal(800x450)
            int resW = (i == 0) ? fw : bw;
            int resH = (i == 0) ? fh : bh;
            // 水平 pass
            {
                ShaderParams p;
                p.viewportWidth = resW; p.viewportHeight = resH;
                p.inputTextures = {(i == 0) ? m_rtA : m_rtC};
                p.uniformFloats = {m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                be->BeginRenderToTexture(m_rtB);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
            // 垂直 pass
            {
                ShaderParams p;
                p.viewportWidth = resW; p.viewportHeight = resH;
                p.inputTextures = {m_rtB};
                p.uniformFloats = {0.0f, m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f};
                be->BeginRenderToTexture(m_rtC);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
        }
    }

    be->Clear(0.1f, 0.1f, 0.1f, 1.0f);

    // === Stage 3: 合成 ===
    // Pass 3a: 背景全屏 (P2=2.0 → 纹理直通)
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {m_rtA};
        p.uniformFloats = {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f};
        be->DrawFullscreenQuad(m_sharedVert, m_glassShader, p);
    }
    // Pass 3b: squircle 玻璃 (P2=1.0 → LiquidGlass)
    {
        TextureHandle blurTex = (m_blurIters > 0) ? m_rtC : m_rtA;
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {blurTex};
        p.uniformFloats = {m_powerFactor, m_fPower, 1.0f, m_noise, m_glowWeight, m_glowBias};
        p.mvp = {
            m_a, m_b, m_c, m_d,
            m_glowEdge0, m_glowEdge1, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        };
        // m1[0].xy = (u_scaleX, u_scaleY) — 玻璃四边形尺寸缩放
        p.modelView = {
            m_glassScaleX, m_glassScaleY, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        };
        be->DrawFullscreenQuad(m_sharedVert, m_glassShader, p);
    }
}

void LiquidGlassScene::OnImGui() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("LiquidGlass", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("FPS: %.1f", m_fpsDisplay);

    // 背景选择
    if (!m_bgNames.empty()) {
        if (ImGui::BeginCombo("Background", m_bgNames[m_currentBgIndex].c_str())) {
            for (int i = 0; i < (int)m_bgNames.size(); i++) {
                bool sel = (i == m_currentBgIndex);
                if (ImGui::Selectable(m_bgNames[i].c_str(), sel))
                    m_currentBgIndex = i;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    if (ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Power", &m_powerFactor, 1.001f, 6.0f);
        ImGui::SliderFloat("Scale X", &m_glassScaleX, 1.0f, 20.0f);
        ImGui::SliderFloat("Scale Y", &m_glassScaleY, 1.0f, 20.0f);
        if (ImGui::Button("Reset Size")) { m_glassScaleX = 4.286f; m_glassScaleY = 2.411f; }
    }

    if (ImGui::CollapsingHeader("Blur & Noise")) {
        ImGui::SliderInt("Blur Iters", &m_blurIters, 0, 10);
        ImGui::SliderFloat("Blur Radius", &m_blurRadius, 0.0f, 10.0f);
        if (ImGui::SliderFloat("Blur Downscale", &m_blurDownscale, 0.1f, 1.0f)) {
            m_lastRTSizeW = 0;
        }
        ImGui::SliderFloat("Noise", &m_noise, 0.0f, 0.3f);
    }

    if (ImGui::CollapsingHeader("Refraction")) {
        ImGui::Text("f(x) = 1 - b*(c*e)^(-d*x - a)");
        ImGui::SliderFloat("f(x) Power", &m_fPower, -1.5f, 6.0f);
        ImGui::SliderFloat("a", &m_a, 0.0f, 5.0f);
        ImGui::SliderFloat("b", &m_b, 0.0f, 6.0f);
        ImGui::SliderFloat("c", &m_c, 0.0f, 6.0f);
        ImGui::SliderFloat("d", &m_d, 0.0f, 10.0f);
    }

    if (ImGui::CollapsingHeader("Glow")) {
        ImGui::SliderFloat("Weight", &m_glowWeight, -1.0f, 1.0f);
        ImGui::SliderFloat("Bias", &m_glowBias, -1.0f, 1.0f);
        ImGui::SliderFloat("Edge0", &m_glowEdge0, -1.0f, 1.0f);
        ImGui::SliderFloat("Edge1", &m_glowEdge1, -1.0f, 1.0f);
    }

    ImGui::End();
}