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

static std::vector<uint32_t> ReadSPIRV(const char* relPath) {
    const char* tries[] = {"shaders/", "build/shaders/"};
    for (int t = 0; t < 2; t++) {
        std::string path = tries[t] + std::string(relPath);
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f.is_open()) continue;
        size_t sz = f.tellg(); f.seekg(0);
        std::vector<uint32_t> data((sz+3)/4);
        f.read((char*)data.data(), sz);
        return data;
    }
    fprintf(stderr,"[AUS3D] ReadSPIRV failed: %s\n",relPath);
    return {};
}

static std::vector<AUS3DEffect> BuildEffects() {
    std::vector<AUS3DEffect> out;
    auto add = [&](const char* n, const char* d, const char* p,
                   std::vector<float> defs={}, std::vector<std::string> labs={},
                   std::vector<float> mi={}, std::vector<float> ma={}) {
        AUS3DEffect e; e.name=n; e.description=d; e.fragShaderPath=p;
        e.defaultValues=std::move(defs); e.paramLabels=std::move(labs);
        e.paramMin=std::move(mi); e.paramMax=std::move(ma);
        out.push_back(std::move(e));
    };
    // === Diagnostic shaders first (verify pipeline end-to-end) ===
    add("DIAG_UV梯度","UV坐标验证-四角着色","aus3d/diag_uv.frag.spv");
    add("DIAG_UBO相机","UBO验证-眼位置着色", "aus3d/diag_ubo.frag.spv");
    add("DIAG_Light发光","光照方向验证",     "aus3d/diag_light.frag.spv");
    add("TEST纯白",  "最小管线-无绑定", "aus3d/test_white.frag.spv");
    add("TEST后处理","灰度化后处理3D球", "aus3d/diag_post.frag.spv", {1.0f},{"亮度"});
    add("TEST单色",  "有UBO无纹理",     "aus3d/v02_solid.frag.spv", {0.8f,0.3f,0.1f},{"R","G","B"});
    add("凹凸边缘光","Vol.01 Rim+Bump",  "aus3d/v01_rim_bump.frag.spv", {3,0,1,1},{"强度","R","G","B"},{0.5f,0,0,0},{8,1,1,1});
    add("基础单色",  "Vol.02 Solid(绿)",  "aus3d/v02_solid.frag.spv", {0.2f,0.7f,0.3f},{"R","G","B"});
    add("漫反射纹理","Vol.02 DiffuseTex", "aus3d/v02_diffuse_tex.frag.spv", {1},{"亮度"});
    add("卡通渐变",  "Vol.07 Toon",       "aus3d/v07_toon.frag.spv", {4},{"级数"},{2},{10});
    add("半兰伯特",  "Vol.07 HalfLambert","aus3d/v07_halflambert.frag.spv", {1},{"亮度"});
    add("镜面高光",  "Vol.07 Specular",   "aus3d/v07_specular.frag.spv", {0.6f,32},{"强度","光泽"},{0,1},{1,128});
    add("边缘发光",  "Vol.14 Rim",        "aus3d/v14_rim.frag.spv", {3,0,1,1},{"强度","R","G","B"});
    add("车漆MatCap","Vol.16 CarPaint",   "aus3d/v16_carpaint.frag.spv", {0.8f},{"反射"});
    return out;
}

AUS3DScene::AUS3DScene() { m_effects=BuildEffects(); m_totalEffects=(int)m_effects.size(); }
AUS3DScene::~AUS3DScene()=default;

void AUS3DScene::OnEnter() {
    printf("[AUS3D] OnEnter - %d effects\n",m_totalEffects);
    m_fpsLastTime=std::chrono::high_resolution_clock::now();
    LoadShaders();
}
void AUS3DScene::OnExit() { printf("[AUS3D] OnExit\n"); }

void AUS3DScene::LoadShaders() {
    if(!m_backend)return;
    auto vd=ReadSPIRV("common/fullscreen_vk.vert.spv");
    if(!vd.empty()) m_sharedVert=m_backend->CreateVertexShader(vd.data(),vd.size());
    for(auto&fx:m_effects) {
        auto fd=ReadSPIRV(fx.fragShaderPath.c_str());
        if(!fd.empty()) fx.fragShader=m_backend->CreateFragmentShader(fd.data(),fd.size());
    }
}
void AUS3DScene::NavigateTo(int i){if(i>=0&&i<m_totalEffects)m_currentIndex=i;}

void AUS3DScene::OnUpdate(float) {
    ImGuiIO&io=ImGui::GetIO();
    if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) NavigateTo((m_currentIndex+m_totalEffects-1)%m_totalEffects);
    if(ImGui::IsKeyPressed(ImGuiKey_RightArrow)) NavigateTo((m_currentIndex+1)%m_totalEffects);
    if(ImGui::IsKeyPressed(ImGuiKey_Escape)) m_wantsReturn=true;
    if(!io.WantCaptureMouse){
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){m_dragging=true;m_dragStartX=io.MousePos.x;m_dragStartY=io.MousePos.y;m_dragStartTheta=m_camTheta;m_dragStartPhi=m_camPhi;}
        if(m_dragging&&ImGui::IsMouseDown(ImGuiMouseButton_Right)){m_camTheta=m_dragStartTheta-(io.MousePos.x-m_dragStartX)*0.005f;m_camPhi=m_dragStartPhi-(io.MousePos.y-m_dragStartY)*0.005f;m_camPhi=std::max(-1.5f,std::min(1.5f,m_camPhi));}
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Right))m_dragging=false;
        m_camRadius-=io.MouseWheel*0.5f;m_camRadius=std::max(1.5f,std::min(8.f,m_camRadius));
    }
    m_fpsFrameCount++;
    auto now=std::chrono::high_resolution_clock::now();
    float e=std::chrono::duration<float>(now-m_fpsLastTime).count();
    if(e>=1.f){m_fpsDisplay=m_fpsFrameCount/e;m_fpsFrameCount=0;m_fpsLastTime=now;}
}

void AUS3DScene::OnRender(IRenderBackend* be) {
    if(!be||m_currentIndex>=m_totalEffects)return;
    auto&fx=m_effects[m_currentIndex];
    if(!m_sharedVert.id||!fx.fragShader.id)return;
    if(!m_defaultTex.id){
        const int TW=64,TH=64;
        std::vector<uint8_t> px(TW*TH*4);
        for(int y=0;y<TH;y++)for(int x=0;x<TW;x++){int o=(y*TW+x)*4;bool ck=((x/8+y/8)&1)==0;px[o]=ck?220:70;px[o+1]=ck?200:100;px[o+2]=ck?240:60;px[o+3]=255;}
        m_defaultTex=be->CreateTexture(TW,TH,TextureFormat::RGBA8,px.data());
    }
    float cx=m_camRadius*cosf(m_camPhi)*sinf(m_camTheta);
    float cy=m_camRadius*sinf(m_camPhi);
    float cz=m_camRadius*cosf(m_camPhi)*cosf(m_camTheta);
    float id[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    ShaderParams p;
    // Use actual framebuffer size, not default 1280x720
    { int fw=1280,fh=720; be->GetFramebufferSize(fw,fh); p.viewportWidth=fw; p.viewportHeight=fh; }
    p.inputTextures.push_back(m_defaultTex);
    p.uniformFloats=fx.defaultValues;
    p.eyePos={cx,cy,cz}; p.lightDir={0.3f,1.0f,0.5f}; p.lightColor={1,1,1};
    p.mvp.assign(id,id+16); p.modelView.assign(id,id+16);
    be->Clear(0.05f,0.05f,0.08f,1);
    be->DrawFullscreenQuad(m_sharedVert,fx.fragShader,p);

    // AUTO-TEST: screenshot each effect once pipeline has settled
    if (getenv("AUTO_TEST_AUS3D")) {
        static int s_screenshotFrame = 0;
        static std::string s_screenshotBase = getenv("AUS3D_SCREENSHOT_DIR") 
            ? std::string(getenv("AUS3D_SCREENSHOT_DIR")) 
            : "e:/AI/graph/hight-post-proc/screenshots";
        s_screenshotFrame++;
        if (s_screenshotFrame == 15) {  // wait 15 frames for pipeline to settle
            char path[256];
            snprintf(path, sizeof(path), "%s/card_aus3d_%02d.ppm", s_screenshotBase.c_str(), m_currentIndex);
            ScreenshotRequest::Request(path);
        }
        if (s_screenshotFrame >= 25) {  // move to next effect
            s_screenshotFrame = 0;
            if (m_currentIndex + 1 < m_totalEffects) {
                NavigateTo(m_currentIndex + 1);
            } else {
                printf("[AUS3D] All effects screenshotted, exiting\n");
                // Unset env to exit auto mode
                putenv("AUTO_TEST_AUS3D=");
                m_wantsReturn = true;
            }
        }
    }
}

void AUS3DScene::OnImGui() {
    if(m_currentIndex>=m_totalEffects)return;
    ImGuiIO&io=ImGui::GetIO();float W=io.DisplaySize.x,H=io.DisplaySize.y;ImFont*f=ImGui::GetFont();
    auto st=[&](float sz,ImVec2 p,ImU32 c,const char*t){auto*dl=ImGui::GetForegroundDrawList();ImU32 s=IM_COL32(0,0,0,180);dl->AddText(f,sz,ImVec2(p.x-2,p.y-2),s,t);dl->AddText(f,sz,ImVec2(p.x+2,p.y-2),s,t);dl->AddText(f,sz,ImVec2(p.x-2,p.y+2),s,t);dl->AddText(f,sz,ImVec2(p.x+2,p.y+2),s,t);dl->AddText(f,sz,p,c,t);};
    auto&fx=m_effects[m_currentIndex];
    char b[128];
    snprintf(b,sizeof(b),"%d/%d %s",m_currentIndex+1,m_totalEffects,fx.name.c_str());
    float tw=ImGui::CalcTextSize(b).x;st(24,ImVec2((W-tw)*.5f,16),IM_COL32(255,255,255,240),b);
    tw=ImGui::CalcTextSize(fx.description.c_str()).x;st(18,ImVec2((W-tw)*.5f,H-50),IM_COL32(200,200,220,200),fx.description.c_str());
    st(20,ImVec2(20,H*.5f-10),IM_COL32(255,255,255,100),"<");
    st(20,ImVec2(W-30,H*.5f-10),IM_COL32(255,255,255,100),">");
    snprintf(b,sizeof(b),"%.0f FPS",m_fpsDisplay);st(14,ImVec2(W-80,16),IM_COL32(180,180,200,180),b);
    ImGui::SetNextWindowPos(ImVec2(10,10));ImGui::SetNextWindowSize(ImVec2(80,36));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,IM_COL32(0,0,0,0));ImGui::PushStyleColor(ImGuiCol_Border,IM_COL32(0,0,0,0));
    ImGui::Begin("##AUSBack",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
    ImGui::SetCursorPos(ImVec2(8,6)); if(ImGui::Button("<- Back",ImVec2(64,24))) m_wantsReturn=true;
    ImGui::End();ImGui::PopStyleColor(2);
    if(!fx.paramLabels.empty()){
        ImGui::SetNextWindowPos(ImVec2(W-240,80));ImGui::SetNextWindowSize(ImVec2(220,30+40*(int)fx.paramLabels.size()));
        ImGui::PushStyleColor(ImGuiCol_WindowBg,IM_COL32(10,10,20,200));ImGui::PushStyleColor(ImGuiCol_Border,IM_COL32(60,60,100,150));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,6);
        char t[64];snprintf(t,sizeof(t),"Params##%d",m_currentIndex);
        ImGui::Begin(t,nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(0.7f,0.7f,0.9f,1),"%s",fx.name.c_str());ImGui::Separator();
        for(size_t p=0;p<fx.paramLabels.size()&&p<fx.defaultValues.size();p++){
            float&v=fx.defaultValues[p];
            float mn=p<fx.paramMin.size()?fx.paramMin[p]:0;
            float mx=p<fx.paramMax.size()?fx.paramMax[p]:1;
            char l[32];snprintf(l,sizeof(l),"##p%d_%d",(int)p,m_currentIndex);
            ImGui::SliderFloat(fx.paramLabels[p].c_str(),&v,mn,mx,"%.2f");
        }
        ImGui::End();ImGui::PopStyleColor(2);ImGui::PopStyleVar();
    }
}
