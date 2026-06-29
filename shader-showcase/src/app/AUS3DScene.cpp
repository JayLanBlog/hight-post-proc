#include "AUS3DScene.h"
#include "Application.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <vector>
#include <cstdint>

// ============================================================================
// Helper: read SPIR-V binary file
// ============================================================================
static std::vector<uint32_t> ReadSPIRV(const char* relativePath) {
    std::string full = "shaders/" + std::string(relativePath);
    std::ifstream file(full, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // Try build/ prefix
        std::string alt = "build/shaders/" + std::string(relativePath);
        file.open(alt, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            fprintf(stderr, "[AUS3D] Cannot open: %s (tried %s)\n", relativePath, full.c_str());
            return {};
        }
    }
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint32_t> data((size + 3) / 4);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

// ============================================================================
// Effect registry
// ============================================================================
static std::vector<AUS3DEffect> BuildEffectList() {
    std::vector<AUS3DEffect> out;
    // Helper to avoid MSVC aggregate-init push_back issues
    auto add = [&](const char* name, const char* desc, const char* path,
                   std::vector<float> defs, std::vector<std::string> labels,
                   std::vector<float> mins = {}, std::vector<float> maxs = {}) {
        AUS3DEffect fx;
        fx.name = name; fx.description = desc; fx.fragShaderPath = path;
        fx.defaultValues = std::move(defs); fx.paramLabels = std::move(labels);
        fx.paramMin = std::move(mins); fx.paramMax = std::move(maxs);
        out.push_back(std::move(fx));
    };

    add("凹凸+边缘光", "Vol.01 边缘发光强度 & 颜色",
        "aus3d/v01_rim_bump.frag.spv",
        {3.0f,0.0f,1.0f,1.0f}, {"强度","R","G","B"}, {0.5f,0,0,0}, {8,1,1,1});

    add("基础单色", "Vol.02 可调纯色着色",
        "aus3d/v02_solid.frag.spv",
        {0.8f,0.3f,0.1f}, {"R","G","B"});

    add("漫反射纹理", "Vol.02 漫反射光照+棋盘格纹理",
        "aus3d/v02_diffuse_tex.frag.spv",
        {1.0f}, {"亮度"});

    add("卡通渐变", "Vol.07 色调级数渐变",
        "aus3d/v07_toon.frag.spv",
        {4.0f}, {"级数"}, {2}, {10});

    add("半兰伯特", "Vol.07 半兰伯特光照",
        "aus3d/v07_halflambert.frag.spv",
        {1.0f}, {"亮度"});

    add("镜面高光", "Vol.07 Phong 高光",
        "aus3d/v07_specular.frag.spv",
        {0.6f,32.0f}, {"强度","光泽"}, {0,1}, {1,128});

    add("边缘发光", "Vol.14 纯边缘光",
        "aus3d/v14_rim.frag.spv",
        {3.0f,0.0f,1.0f,1.0f}, {"强度","R","G","B"});

    add("车漆MatCap", "Vol.16 MatCap 真实车漆",
        "aus3d/v16_carpaint.frag.spv",
        {0.8f}, {"反射"});

    return out;
}

// ============================================================================
// Construction
// ============================================================================
AUS3DScene::AUS3DScene() {
    m_effects = BuildEffectList();
    m_totalEffects = (int)m_effects.size();
    m_currentIndex = 0;
}

AUS3DScene::~AUS3DScene() = default;

void AUS3DScene::OnEnter() {
    printf("[AUS3D] OnEnter — %d effects\n", m_totalEffects);
    m_fpsLastTime = std::chrono::high_resolution_clock::now();
    LoadShaders();
}

void AUS3DScene::OnExit() {
    printf("[AUS3D] OnExit\n");
}

void AUS3DScene::LoadShaders() {
    if (!m_backend) return;

    // Shared vertex shader
    auto vertData = ReadSPIRV("aus3d/aus3d_shared.vert.spv");
    if (!vertData.empty())
        m_sharedVert = m_backend->CreateVertexShader(vertData.data(), vertData.size() * sizeof(uint32_t));

    for (auto& fx : m_effects) {
        auto fragData = ReadSPIRV(fx.fragShaderPath.c_str());
        if (!fragData.empty())
            fx.fragShader = m_backend->CreateFragmentShader(fragData.data(), fragData.size() * sizeof(uint32_t));
    }
}

void AUS3DScene::CompileEffect(int) {}
void AUS3DScene::NavigateTo(int i) {
    if (i >= 0 && i < m_totalEffects) m_currentIndex = i;
}

// ============================================================================
// Update
// ============================================================================
void AUS3DScene::OnUpdate(float) {
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
        NavigateTo((m_currentIndex + m_totalEffects - 1) % m_totalEffects);
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
        NavigateTo((m_currentIndex + 1) % m_totalEffects);
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        m_wantsReturn = true;

    if (!io.WantCaptureMouse) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            m_dragging = true; m_dragStartX = io.MousePos.x; m_dragStartY = io.MousePos.y;
            m_dragStartTheta = m_camTheta; m_dragStartPhi = m_camPhi;
        }
        if (m_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            m_camTheta = m_dragStartTheta - (io.MousePos.x - m_dragStartX) * 0.005f;
            m_camPhi   = m_dragStartPhi   - (io.MousePos.y - m_dragStartY) * 0.005f;
            m_camPhi = std::max(-1.5f, std::min(1.5f, m_camPhi));
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) m_dragging = false;
        m_camRadius -= io.MouseWheel * 0.5f;
        m_camRadius = std::max(1.5f, std::min(8.0f, m_camRadius));
    }

    m_fpsFrameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    float elap = std::chrono::duration<float>(now - m_fpsLastTime).count();
    if (elap >= 1.0f) { m_fpsDisplay = m_fpsFrameCount / elap; m_fpsFrameCount = 0; m_fpsLastTime = now; }
}

// ============================================================================
// Render
// ============================================================================
void AUS3DScene::OnRender(IRenderBackend* backend) {
    if (!backend || m_currentIndex >= m_totalEffects) return;
    const auto& fx = m_effects[m_currentIndex];
    if (!m_sharedVert.id || !fx.fragShader.id) return;

    if (!m_defaultTex.id) {
        const int TW = 64, TH = 64;
        std::vector<uint8_t> px(TW*TH*4);
        for (int y=0;y<TH;y++) for (int x=0;x<TW;x++) {
            int o = (y*TW+x)*4; bool ck = ((x/8+y/8)&1)==0;
            px[o]=ck?220:70; px[o+1]=ck?200:100; px[o+2]=ck?240:60; px[o+3]=255;
        }
        m_defaultTex = backend->CreateTexture(TW,TH,TextureFormat::RGBA8,px.data());
    }

    float cx = m_camRadius*cosf(m_camPhi)*sinf(m_camTheta);
    float cy = m_camRadius*sinf(m_camPhi);
    float cz = m_camRadius*cosf(m_camPhi)*cosf(m_camTheta);
    float id[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    ShaderParams p;
    p.inputTextures.push_back(m_defaultTex);
    p.uniformFloats = fx.defaultValues;
    p.eyePos = {cx,cy,cz};
    p.lightDir = {0.5f,0.8f,0.3f};
    p.lightColor = {1,1,1};
    p.mvp.assign(id,id+16);
    p.modelView.assign(id,id+16);

    backend->Clear(0.05f,0.05f,0.08f,1);
    backend->DrawFullscreenQuad(m_sharedVert, fx.fragShader, p);
}

// ============================================================================
// ImGui
// ============================================================================
void AUS3DScene::OnImGui() {
    if (m_currentIndex >= m_totalEffects) return;
    ImGuiIO& io = ImGui::GetIO();
    float W=io.DisplaySize.x, H=io.DisplaySize.y;
    ImFont* font=ImGui::GetFont();
    auto st = [&](float sz,ImVec2 pos,ImU32 col,const char* txt){
        ImDrawList* dl=ImGui::GetForegroundDrawList();
        ImU32 sh=IM_COL32(0,0,0,180);
        dl->AddText(font,sz,ImVec2(pos.x-2,pos.y-2),sh,txt);
        dl->AddText(font,sz,ImVec2(pos.x+2,pos.y-2),sh,txt);
        dl->AddText(font,sz,ImVec2(pos.x-2,pos.y+2),sh,txt);
        dl->AddText(font,sz,ImVec2(pos.x+2,pos.y+2),sh,txt);
        dl->AddText(font,sz,pos,col,txt);
    };

    // Title
    char buf[128];
    auto& fx = m_effects[m_currentIndex];
    snprintf(buf,sizeof(buf),"%d/%d  %s",m_currentIndex+1,m_totalEffects,fx.name.c_str());
    float tw=ImGui::CalcTextSize(buf).x; st(24.f,ImVec2((W-tw)*0.5f,16),IM_COL32(255,255,255,240),buf);

    // Description
    tw=ImGui::CalcTextSize(fx.description.c_str()).x; st(18.f,ImVec2((W-tw)*0.5f,H-50),IM_COL32(200,200,220,200),fx.description.c_str());

    // Arrows
    st(20.f,ImVec2(20,H*0.5f-10),IM_COL32(255,255,255,100),"<");
    st(20.f,ImVec2(W-30,H*0.5f-10),IM_COL32(255,255,255,100),">");

    // Back button
    ImGui::SetNextWindowPos(ImVec2(10,10)); ImGui::SetNextWindowSize(ImVec2(80,36));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,IM_COL32(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Border,IM_COL32(0,0,0,0));
    ImGui::Begin("##AUSBack",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
    ImGui::SetCursorPos(ImVec2(8,6));
    if(ImGui::Button("<- Back",ImVec2(64,24))) m_wantsReturn=true;
    ImGui::End(); ImGui::PopStyleColor(2);

    // FPS
    snprintf(buf,sizeof(buf),"%.0f FPS",m_fpsDisplay); st(14.f,ImVec2(W-80,16),IM_COL32(180,180,200,180),buf);

    // Parameter sliders (mutable copy)
    if (!fx.paramLabels.empty()) {
        ImGui::SetNextWindowPos(ImVec2(W-240,80)); ImGui::SetNextWindowSize(ImVec2(220,30+40*(int)fx.paramLabels.size()));
        ImGui::PushStyleColor(ImGuiCol_WindowBg,IM_COL32(10,10,20,200));
        ImGui::PushStyleColor(ImGuiCol_Border,IM_COL32(60,60,100,150));
        char title[64]; snprintf(title,sizeof(title),"Params##%d",m_currentIndex);
        ImGui::Begin(title,nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(0.7f,0.7f,0.9f,1),"%s",fx.name.c_str()); ImGui::Separator();
        for (size_t p=0; p<fx.paramLabels.size() && p<fx.defaultValues.size(); p++) {
            float v=fx.defaultValues[p];
            float mn=p<fx.paramMin.size()?fx.paramMin[p]:0;
            float mx=p<fx.paramMax.size()?fx.paramMax[p]:1;
            char lbl[32]; snprintf(lbl,sizeof(lbl),"##p%d_%d",(int)p,m_currentIndex);
            if (ImGui::SliderFloat(fx.paramLabels[p].c_str(),&v,mn,mx,"%.2f"))
                fx.defaultValues[p]=v;
        }
        ImGui::End(); ImGui::PopStyleColor(2);
    }
}
