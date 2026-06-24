#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "imgui.h"

#include "stb_image.h"

#include <cstdio>
#include <vector>
#include <string>

void SceneGalleryScene::OnEnter() {
    m_activeCategory = "全部";
    m_pendingEnter.clear();
    m_heroIndex = 0;
    m_thumbsLoaded = false;
}

void SceneGalleryScene::OnExit() {
    ReleaseThumbnails();
}

void SceneGalleryScene::OnRender(IRenderBackend* backend) {
    m_backend = backend;
}

void SceneGalleryScene::OnUpdate(float /*dt*/) {}

// ---- thumbnails ----

void SceneGalleryScene::LoadThumbnails() {
    if (m_thumbsLoaded || !m_backend) return;
    const auto& scenes = SceneRegistry::Instance().All();
    m_thumbTextures.resize(scenes.size());
    for (size_t i = 0; i < scenes.size(); i++) {
        if (scenes[i].thumbPath.empty()) continue;
        int tw, th, comp;
        stbi_uc* data = stbi_load(scenes[i].thumbPath.c_str(), &tw, &th, &comp, 4);
        if (data) {
            m_thumbTextures[i] = m_backend->CreateTexture(tw, th, TextureFormat::RGBA8, data);
            stbi_image_free(data);
        }
    }
    m_thumbsLoaded = true;
}

void SceneGalleryScene::ReleaseThumbnails() {
    if (!m_backend) { m_thumbTextures.clear(); m_thumbsLoaded = false; return; }
    for (auto& tex : m_thumbTextures) {
        if (tex.id != INVALID_TEXTURE.id) m_backend->DestroyTexture(tex);
    }
    m_thumbTextures.clear();
    m_thumbsLoaded = false;
}

// ---- main draw ----

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;

    if (!m_thumbsLoaded) LoadThumbnails();

    // ================ Full-screen dark background ================
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(10, 11, 16, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float pad = W * 0.04f;  // proportional padding

    // ---- Ambient glow ----
    dl->AddRectFilled(ImVec2(W * 0.15f, 0), ImVec2(W * 0.55f, H * 0.35f),
                      IM_COL32(74, 91, 207, 5));

    // =============================================
    // 1. TITLE
    // =============================================
    float curY = H * 0.015f;
    dl->AddText(ImGui::GetFont(), H * 0.033f,
                ImVec2(pad, curY), IM_COL32(240, 245, 255, 255),
                "Scene Gallery");
    curY += H * 0.042f;
    dl->AddText(ImGui::GetFont(), H * 0.014f,
                ImVec2(pad, curY), IM_COL32(160, 170, 200, 220),
                "GPU Real-Time Effects Showcase");
    curY += H * 0.024f;

    // =============================================
    // 2. HERO CARD
    // =============================================
    const auto& allScenes = SceneRegistry::Instance().All();
    if (!allScenes.empty()) {
        const auto& scene = allScenes[m_heroIndex % allScenes.size()];
        float hx = pad, hy = curY + H * 0.008f;
        float hh = H * 0.22f, hw = W - pad * 2.0f;

        dl->AddRectFilled(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                          IM_COL32(26, 28, 48, 230), 8.0f);
        dl->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                    IM_COL32(74, 91, 207, 35), 8.0f);

        // thumb
        float th = hh - 14.0f, tw = th * 1.78f;
        float tX = hx + 10.0f, tY = hy + 7.0f;
        if (m_heroIndex < (int)m_thumbTextures.size() &&
            m_thumbTextures[m_heroIndex].id != INVALID_TEXTURE.id) {
            dl->AddImage((ImTextureID)(uintptr_t)m_thumbTextures[m_heroIndex].id,
                         ImVec2(tX, tY), ImVec2(tX + tw, tY + th));
        } else {
            dl->AddRectFilled(ImVec2(tX, tY), ImVec2(tX + tw, tY + th),
                              IM_COL32(20, 22, 38, 255), 4.0f);
            dl->AddRect(ImVec2(tX, tY), ImVec2(tX + tw, tY + th),
                        IM_COL32(74, 91, 207, 35), 4.0f);
        }

        // name
        float lx = tX + tw + 18.0f;
        dl->AddText(ImGui::GetFont(), H * 0.026f,
                    ImVec2(lx, hy + 12.0f),
                    IM_COL32(240, 245, 255, 255), scene.name.c_str());

        // category badge
        float bw = ImGui::CalcTextSize(scene.category.c_str()).x + 18.0f;
        dl->AddRectFilled(ImVec2(lx, hy + 12.0f + H * 0.034f),
                          ImVec2(lx + bw, hy + 12.0f + H * 0.034f + H * 0.022f),
                          IM_COL32(74, 91, 207, 70), 3.0f);
        dl->AddText(ImGui::GetFont(), H * 0.013f,
                    ImVec2(lx + 9.0f, hy + 12.0f + H * 0.036f),
                    IM_COL32(160, 175, 240, 255), scene.category.c_str());

        // desc
        dl->AddText(ImGui::GetFont(), H * 0.015f,
                    ImVec2(lx, hy + 12.0f + H * 0.062f),
                    IM_COL32(140, 150, 175, 200), scene.description.c_str());

        // enter button
        float btnW = H * 0.10f, btnH = H * 0.040f;
        float bX = hx + hw - btnW - 14.0f, bY = hy + hh - btnH - 10.0f;
        ImGui::SetCursorScreenPos(ImVec2(bX, bY));
        bool entering = ImGui::Button("Enter", ImVec2(btnW, btnH));
        if (entering && scene.available) m_pendingEnter = scene.id;

        curY = hy + hh;
    }

    // =============================================
    // 3. CATEGORY TABS
    // =============================================
    curY += H * 0.016f;
    auto cats = SceneRegistry::Instance().Categories();
    std::vector<std::string> tabs; tabs.push_back("全部");
    for (auto& c : cats) tabs.push_back(c);

    float tabX = pad, tabH = H * 0.032f;
    for (size_t i = 0; i < tabs.size(); i++) {
        float tw = ImGui::CalcTextSize(tabs[i].c_str()).x + 28.0f;
        bool sel = (tabs[i] == m_activeCategory);

        ImU32 bg = sel ? IM_COL32(74, 91, 207, 70) : IM_COL32(30, 33, 50, 140);
        dl->AddRectFilled(ImVec2(tabX, curY), ImVec2(tabX + tw, curY + tabH), bg, 4.0f);
        if (sel) dl->AddRect(ImVec2(tabX, curY), ImVec2(tabX + tw, curY + tabH),
                             IM_COL32(74, 91, 207, 130), 4.0f);

        ImU32 tc = sel ? IM_COL32(200, 210, 255, 255) : IM_COL32(150, 160, 190, 200);
        ImVec2 ts = ImGui::CalcTextSize(tabs[i].c_str());
        dl->AddText(ImGui::GetFont(), H * 0.015f,
                    ImVec2(tabX + tw * 0.5f - ts.x * 0.5f, curY + (tabH - ts.y) * 0.5f),
                    tc, tabs[i].c_str());

        ImGui::SetCursorScreenPos(ImVec2(tabX, curY));
        ImGui::InvisibleButton(("##tab" + std::to_string(i)).c_str(), ImVec2(tw, tabH));
        if (ImGui::IsItemClicked()) m_activeCategory = tabs[i];

        tabX += tw + 8.0f;
    }
    curY += tabH + 10.0f;

    // separator
    dl->AddLine(ImVec2(pad, curY), ImVec2(W - pad, curY),
                IM_COL32(74, 91, 207, 18), 1.0f);
    curY += 8.0f;

    // =============================================
    // 4. CARD GRID
    // =============================================

    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    if (scenes.empty()) {
        dl->AddText(ImGui::GetFont(), H * 0.025f,
                    ImVec2(pad, curY + H * 0.1f),
                    IM_COL32(100, 110, 130, 180), "No scenes in this category");
    } else {
        int COLS = 3;
        float gapX = 14.0f, gapY = 14.0f;
        float cardW = (W - pad * 2.0f - gapX * (COLS - 1)) / (float)COLS;
        float cardH = cardW * 0.72f;

        for (size_t i = 0; i < scenes.size(); i++) {
            int col = (int)i % COLS, row = (int)i / COLS;
            float cX = pad + col * (cardW + gapX);
            float cY = curY + row * (cardH + gapY);

            // skip off-screen
            if (cY + cardH < 0 || cY > H) continue;

            bool hover = m_hoverCard == (int)i;
            ImU32 cardBg = hover ? IM_COL32(35, 40, 62, 240)
                                 : IM_COL32(22, 25, 38, 200);
            dl->AddRectFilled(ImVec2(cX, cY), ImVec2(cX + cardW, cY + cardH),
                              cardBg, 6.0f);
            if (hover) {
                dl->AddRect(ImVec2(cX, cY), ImVec2(cX + cardW, cY + cardH),
                            IM_COL32(74, 91, 207, 80), 6.0f);
            }

            // thumb
            float tp = 5.0f;
            float thH = cardH * 0.55f, thW = cardW - tp * 2.0f;
            float thX = cX + tp, thY = cY + tp;

            if (i < m_thumbTextures.size() &&
                m_thumbTextures[i].id != INVALID_TEXTURE.id) {
                dl->AddImage((ImTextureID)(uintptr_t)m_thumbTextures[i].id,
                             ImVec2(thX, thY), ImVec2(thX + thW, thY + thH));
            } else {
                dl->AddRectFilled(ImVec2(thX, thY), ImVec2(thX + thW, thY + thH),
                                  IM_COL32(18, 20, 32, 255), 3.0f);
                float cx = thX + thW * 0.5f, cy = thY + thH * 0.5f;
                dl->AddLine(ImVec2(cx - 8, cy), ImVec2(cx + 8, cy),
                            IM_COL32(74, 91, 207, 30));
                dl->AddLine(ImVec2(cx, cy - 4), ImVec2(cx, cy + 4),
                            IM_COL32(74, 91, 207, 30));
            }

            // name
            dl->AddText(ImGui::GetFont(), H * 0.016f,
                        ImVec2(cX + 6.0f, thY + thH + 3.0f),
                        IM_COL32(230, 238, 255, 255), scenes[i].name.c_str());

            // tag
            dl->AddText(ImGui::GetFont(), H * 0.012f,
                        ImVec2(cX + 6.0f, thY + thH + 3.0f + H * 0.021f),
                        IM_COL32(140, 150, 180, 180), scenes[i].category.c_str());

            // click
            ImGui::SetCursorScreenPos(ImVec2(cX, cY));
            ImGui::InvisibleButton(("##card" + std::to_string(i)).c_str(),
                                   ImVec2(cardW, cardH));
            if (ImGui::IsItemHovered()) { m_hoverCard = (int)i; m_heroIndex = (int)i; }
            if (ImGui::IsItemClicked() && scenes[i].available)
                m_pendingEnter = scenes[i].id;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ---- scene transition ----

std::unique_ptr<Scene> SceneGalleryScene::GetNextScene() {
    if (m_pendingEnter.empty()) return nullptr;
    const auto& all = SceneRegistry::Instance().All();
    for (auto& e : all) {
        if (e.id == m_pendingEnter && e.factory) return e.factory();
    }
    return nullptr;
}
