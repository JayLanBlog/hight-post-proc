#include "LiquidGlassScene.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdint>

// 读取图片文件尺寸 (支持PNG和JPEG)
static bool GetImageSize(const std::string& path, int& w, int& h) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    uint8_t sig[8];
    f.read((char*)sig, 8);
    // PNG
    if (sig[0]==0x89 && sig[1]=='P' && sig[2]=='N' && sig[3]=='G') {
        f.seekg(16);
        uint8_t buf[8]; f.read((char*)buf, 8);
        w = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        h = (buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7];
        return true;
    }
    // JPEG
    if (sig[0]==0xFF && sig[1]==0xD8) {
        f.seekg(2);
        while (f) {
            uint8_t m[2]; f.read((char*)m, 2);
            if (m[0]!=0xFF) return false;
            while (m[1]==0xFF) f.read((char*)&m[1],1);
            if (m[1]>=0xC0 && m[1]<=0xC2) {
                f.seekg(3, std::ios::cur);
                uint8_t d[4]; f.read((char*)d, 4);
                h = (d[0]<<8)|d[1]; w = (d[2]<<8)|d[3];
                return true;
            }
            uint8_t l[2]; f.read((char*)l,2);
            f.seekg((l[0]<<8)|l[1]-2, std::ios::cur);
        }
    }
    return false;
}

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
            int tw=1, th=1;
            GetImageSize(path, tw, th);
            m_bgTexSizes.push_back({tw, th});
            printf("[LiquidGlass] Loaded: %s (id=%d, %dx%d)\n", path.c_str(), tex.id, tw, th);
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
    if (!be || !m_sharedVert.id || !m_bgShader.id || !m_blurShader.id || !m_glassShader.id) return;
    if (m_bgTextures.empty()) return;

    int fw = 1280, fh = 720;
    be->GetFramebufferSize(fw, fh);

    int bw = (int)(fw * m_blurDownscale);
    int bh = (int)(fh * m_blurDownscale);
    if (m_lastRTSizeW != fw || m_lastRTSizeH != fh) {
        CreateRTs(fw, fh);
    }

    TextureHandle bgTex = m_bgTextures[m_currentBgIndex];
    int texW = m_bgTexSizes[m_currentBgIndex].first;
    int texH = m_bgTexSizes[m_currentBgIndex].second;
    float bgHalfX = 0.375f;
    float bgHalfY = (5.0f * (float)texH / (float)texW) / 7.5f;

    // Step 1: bg → RT A (居中四边形 + 暗色边框)
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {bgTex};
        p.uniformFloats = {bgHalfX, bgHalfY, 0.0f, 0.0f, 0.0f, 0.0f};
        be->BeginRenderToTexture(m_rtA);
        be->DrawFullscreenQuad(m_sharedVert, m_bgShader, p);
        be->EndRenderToTexture();
    }

    // Step 2: 模糊 (水平+垂直)
    if (m_blurIters > 0) {
        for (int i = 0; i < m_blurIters; i++) {
            int resW = (i == 0) ? fw : bw;
            int resH = (i == 0) ? fh : bh;
            {
                ShaderParams p;
                p.viewportWidth = bw; p.viewportHeight = bh;
                p.inputTextures = {(i == 0) ? m_rtA : m_rtC};
                p.uniformFloats = {m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                p.modelView = {(float)resW,(float)resH,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
                be->BeginRenderToTexture(m_rtB);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
            {
                ShaderParams p;
                p.viewportWidth = bw; p.viewportHeight = bh;
                p.inputTextures = {m_rtB};
                p.uniformFloats = {0.0f, m_blurRadius, 0.0f, 0.0f, 0.0f, 0.0f};
                p.modelView = {(float)resW,(float)resH,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
                be->BeginRenderToTexture(m_rtC);
                be->DrawFullscreenQuad(m_sharedVert, m_blurShader, p);
                be->EndRenderToTexture();
            }
        }
    }

    // Step 3: Clear + glass shader 背景直通 (P2=2.0) → 屏幕
    be->Clear(0.1f, 0.1f, 0.1f, 1.0f);
    {
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {m_rtA};
        p.uniformFloats = {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f};
        be->DrawFullscreenQuad(m_sharedVert, m_glassShader, p);
    }

    // Step 4: squircle 玻璃 (P2=1.0) + 模糊
    {
        TextureHandle blurTex = (m_blurIters > 0) ? m_rtC : m_rtA;
        ShaderParams p;
        p.viewportWidth = fw; p.viewportHeight = fh;
        p.inputTextures = {blurTex};
        p.blendEnable = true;  // alpha blending: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
        p.uniformFloats = {m_powerFactor, m_fPower, 1.0f, m_noise, m_glowWeight, m_glowBias};
        p.mvp = {m_a,m_b,m_c,m_d, m_glowEdge0,m_glowEdge1,0,0, 0,0,0,0, 0,0,0,0};
        p.modelView = {m_glassScaleX,m_glassScaleY,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
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
        if (ImGui::Button("Reset Size")) { m_glassScaleX = 7.62f; m_glassScaleY = 4.29f; }
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