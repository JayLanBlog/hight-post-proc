#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "imgui.h"

#include "stb_image.h"

#include <cstdio>
#include <cmath>
#include <algorithm>

// ============================================================================
// Lifecycle
// ============================================================================

void SceneGalleryScene::OnEnter() {
    m_activeCategory = "全部";
    m_pendingEnter.clear();
    m_hoverCard = -1;
    m_heroIndex = 0;
    m_thumbsLoaded = false;
    printf("[SceneGallery] Entered, %zu scenes registered\n",
           SceneRegistry::Instance().All().size());
}

void SceneGalleryScene::OnExit() {
    ReleaseThumbnails();
}

void SceneGalleryScene::OnRender(IRenderBackend* backend) {
    m_backend = backend;
}

void SceneGalleryScene::OnUpdate(float dt) {
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    if (m_cardAnims.size() != scenes.size()) {
        if (!m_cardAnims.empty()) {
            for (auto& a : m_cardAnims) a.entryT = -1.0f;
        }
        m_cardAnims.resize(scenes.size());
    }

    float dtC = std::min(dt, 0.1f);
    for (size_t i = 0; i < m_cardAnims.size(); i++) {
        auto& a = m_cardAnims[i];

        float hoverTarget = (m_hoverCard == (int)i) ? 1.0f : 0.0f;
        a.hoverT += (hoverTarget - a.hoverT) * std::min(1.0f, dtC * 10.0f);

        if (a.entryT < 0.0f) a.entryT = 0.0f;
        if (a.entryT < 1.0f) {
            a.entryT += dtC / 0.4f;
            if (a.entryT > 1.0f) a.entryT = 1.0f;
        }

        a.clickVel += (1.0f - a.clickScale) * 20.0f * dtC;
        a.clickVel *= std::exp(-12.0f * dtC);
        a.clickScale += a.clickVel * dtC;
        if (std::abs(a.clickScale - 1.0f) < 0.001f && std::abs(a.clickVel) < 0.01f) {
            a.clickScale = 1.0f;
            a.clickVel = 0.0f;
        }
    }
}

// ============================================================================
// Thumbnail management
// ============================================================================

void SceneGalleryScene::LoadThumbnails() {
    if (m_thumbsLoaded || !m_backend) return;

    const auto& scenes = SceneRegistry::Instance().All();
    m_thumbTextures.resize(scenes.size());

    for (size_t i = 0; i < scenes.size(); i++) {
        const auto& e = scenes[i];
        if (e.thumbPath.empty()) continue;

        int tw, th, comp;
        stbi_uc* data = stbi_load(e.thumbPath.c_str(), &tw, &th, &comp, 4);
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
        if (tex.id != INVALID_TEXTURE.id) {
            m_backend->DestroyTexture(tex);
        }
    }
    m_thumbTextures.clear();
    m_thumbsLoaded = false;
}

// ============================================================================
// Main ImGui entry
// ============================================================================

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;

    if (!m_thumbsLoaded) LoadThumbnails();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 1.0f));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    // Get draw list AFTER Begin() so coordinates are window-relative
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // --- Layout constants (all proportional to H) ---
    const float margin  = W * 0.05f;
    const float topH    = H * 0.080f;  // top bar region
    const float heroH   = H * 0.250f;  // hero card region
    const float tabH    = H * 0.045f;  // category tab bar
    const float gap     = H * 0.015f;  // spacing between regions

    // ===== Draw Background =====
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), kBgDeep);

    // Ambient glows
    dl->AddRectFilled(ImVec2(W * 0.1f, 0), ImVec2(W * 0.6f, H * 0.35f),
                      IM_COL32(74, 91, 207, 6));
    dl->AddRectFilled(ImVec2(W * 0.4f, H * 0.5f), ImVec2(W, H),
                      IM_COL32(74, 91, 207, 3));

    // ===== 1. Top bar =====
    float sectionY = H * 0.012f;
    dl->AddText(ImGui::GetFont(), H * 0.033f,
                ImVec2(margin, sectionY), kTextPrimary, "Scene Gallery");
    dl->AddText(ImGui::GetFont(), H * 0.015f,
                ImVec2(margin, sectionY + H * 0.038f),
                kTextSecondary, "GPU Real-Time Effects Showcase");
    dl->AddLine(ImVec2(margin, sectionY + H * 0.058f),
                ImVec2(margin + W * 0.06f, sectionY + H * 0.058f),
                kAccent, 2.0f);

    // ===== 2. Hero =====
    const auto& allScenes = SceneRegistry::Instance().All();
    if (!allScenes.empty()) {
        const auto& scene = allScenes[m_heroIndex];
        float hx = margin + H * 0.02f;  // use a small height-based margin for alignment
        float hy = topH + gap;
        float hh = heroH;
        float hw = W - hx * 2.0f;

        // Hero card bg
        dl->AddRectFilled(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                          IM_COL32(26, 28, 48, 230), 8.0f);
        dl->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                    IM_COL32(74, 91, 207, 40), 8.0f, 0, 1.0f);

        // Thumbnail
        float thumbPad = 10.0f;
        float thumbH = hh - thumbPad * 2.0f;
        float thumbW = thumbH * 1.78f;
        float thumbX = hx + thumbPad;
        float thumbY = hy + thumbPad;

        if (m_heroIndex < (int)m_thumbTextures.size() &&
            m_thumbTextures[m_heroIndex].id != INVALID_TEXTURE.id) {
            dl->AddImage((ImTextureID)(uintptr_t)m_thumbTextures[m_heroIndex].id,
                         ImVec2(thumbX, thumbY),
                         ImVec2(thumbX + thumbW, thumbY + thumbH));
        } else {
            dl->AddRectFilled(ImVec2(thumbX, thumbY),
                              ImVec2(thumbX + thumbW, thumbY + thumbH),
                              IM_COL32(20, 22, 38, 255), 4.0f);
            float tcx = thumbX + thumbW * 0.5f, tcy = thumbY + thumbH * 0.5f;
            dl->AddLine(ImVec2(tcx - 20, tcy), ImVec2(tcx + 20, tcy), kTextTertiary);
            dl->AddLine(ImVec2(tcx, tcy - 12), ImVec2(tcx, tcy + 12), kTextTertiary);
        }

        // Text
        float tx = thumbX + thumbW + 20.0f;
        float tw = hx + hw - tx - 16.0f;

        dl->AddText(ImGui::GetFont(), H * 0.027f,
                    ImVec2(tx, hy + 14.0f), kTextPrimary, scene.name.c_str());

        // Category badge
        float badgeW = ImGui::CalcTextSize(scene.category.c_str()).x + 16.0f;
        dl->AddRectFilled(ImVec2(tx, hy + 14.0f + H * 0.034f),
                          ImVec2(tx + badgeW, hy + 14.0f + H * 0.034f + H * 0.022f),
                          IM_COL32(74, 91, 207, 80), 3.0f);
        dl->AddText(ImGui::GetFont(), H * 0.013f,
                    ImVec2(tx + 8.0f, hy + 14.0f + H * 0.036f),
                    kAccentHover, scene.category.c_str());

        // Desc
        dl->AddText(ImGui::GetFont(), H * 0.016f,
                    ImVec2(tx, hy + 14.0f + H * 0.066f),
                    kTextSecondary, scene.description.c_str());

        // Enter button
        float btnW = H * 0.12f, btnH = H * 0.045f;
        float btnX = hx + hw - btnW - 16.0f;
        float btnY = hy + hh - btnH - 12.0f;

        if (scene.available) {
            ImGui::SetCursorScreenPos(ImVec2(btnX, btnY));
            ImGui::InvisibleButton("##hero_enter", ImVec2(btnW, btnH));
            bool btnHover = ImGui::IsItemHovered();
            ImU32 btnBg = btnHover ? kAccentHover : kAccent;
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH), btnBg, 4.0f);

            const char* txt = "Enter";
            ImVec2 ts = ImGui::CalcTextSize(txt);
            dl->AddText(ImGui::GetFont(), H * 0.020f,
                        ImVec2(btnX + (btnW - ts.x) * 0.5f, btnY + (btnH - ts.y) * 0.5f),
                        IM_COL32(255,255,255,255), txt);

            if (ImGui::IsItemClicked()) {
                m_pendingEnter = scene.id;
                if (m_heroIndex < (int)m_cardAnims.size()) {
                    m_cardAnims[m_heroIndex].clickScale = 0.97f;
                    m_cardAnims[m_heroIndex].clickVel = 0.0f;
                }
            }
        } else {
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                              IM_COL32(40, 42, 55, 180), 4.0f);
            const char* txt = "Coming Soon";
            ImVec2 ts = ImGui::CalcTextSize(txt);
            dl->AddText(ImGui::GetFont(), H * 0.014f,
                        ImVec2(btnX + (btnW - ts.x) * 0.5f, btnY + (btnH - ts.y) * 0.5f),
                        kTextTertiary, txt);
        }
    }

    // ===== 3. Category tabs =====
    auto cats = SceneRegistry::Instance().Categories();
    std::vector<std::string> items;
    items.push_back("全部");
    for (auto& c : cats) items.push_back(c);

    float tabY = topH + gap + heroH + gap;
    float tabBarH = tabH;
    float tx = margin;

    for (size_t i = 0; i < items.size(); i++) {
        float bw = ImGui::CalcTextSize(items[i].c_str()).x + 24.0f;
        bool active = (items[i] == m_activeCategory);

        ImU32 bg = active ? IM_COL32(74, 91, 207, 60) : IM_COL32(30, 32, 50, 120);
        dl->AddRectFilled(ImVec2(tx, tabY), ImVec2(tx + bw, tabY + tabBarH), bg, 4.0f);

        if (active) {
            dl->AddRect(ImVec2(tx, tabY), ImVec2(tx + bw, tabY + tabBarH),
                        IM_COL32(74, 91, 207, 120), 4.0f, 0, 1.0f);
        }

        ImU32 tc = active ? kAccentHover : kTextSecondary;
        ImVec2 ts = ImGui::CalcTextSize(items[i].c_str());
        dl->AddText(ImGui::GetFont(), H * 0.016f,
                    ImVec2(tx + 12.0f, tabY + (tabBarH - ts.y) * 0.5f),
                    tc, items[i].c_str());

        ImGui::SetCursorScreenPos(ImVec2(tx, tabY));
        ImGui::InvisibleButton(("##tab_" + std::to_string(i)).c_str(), ImVec2(bw, tabBarH));
        if (ImGui::IsItemClicked() && items[i] != m_activeCategory) {
            m_activeCategory = items[i];
            for (auto& a : m_cardAnims) a.entryT = -1.0f;
        }
        tx += bw + 8.0f;
    }

    dl->AddLine(ImVec2(margin, tabY + tabBarH + 4.0f),
                ImVec2(W - margin, tabY + tabBarH + 4.0f),
                IM_COL32(74, 91, 207, 20), 1.0f);

    // ===== 4. Card grid =====
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    float gridY = tabY + tabBarH + 14.0f;
    float availH = H - gridY - 8.0f;

    if (scenes.empty()) {
        dl->AddText(ImGui::GetFont(), H * 0.025f,
                    ImVec2(margin, gridY + availH * 0.4f),
                    kTextTertiary, "No scenes in this category");
    } else {
        int cols = 3;
        float cgapX = 16.0f, cgapY = 14.0f;
        float cardW = (W - margin * 2.0f - cgapX * (cols - 1)) / (float)cols;
        float cardH = cardW * 0.75f;

        // Manual clipping: only draw visible rows
        int rows = ((int)scenes.size() + cols - 1) / cols;
        int firstRow = 0, lastRow = rows;
        for (int r = 0; r < rows; r++) {
            float rowTop = gridY + r * (cardH + cgapY);
            if (rowTop + cardH < gridY) { firstRow = r + 1; continue; }
            if (rowTop > H) { lastRow = r; break; }
        }

        for (size_t i = 0; i < scenes.size(); i++) {
            const auto& scene = scenes[i];
            int col = (int)i % cols, row = (int)i / cols;
            if (row < firstRow || row > lastRow) continue;

            float cx = margin + col * (cardW + cgapX);
            float cy = gridY + row * (cardH + cgapY);

            auto& anim = (i < m_cardAnims.size()) ? m_cardAnims[i] : CardAnimState{};
            float hoverT = (i < m_cardAnims.size()) ? anim.hoverT : 0.0f;
            float clkS   = (i < m_cardAnims.size()) ? anim.clickScale : 1.0f;

            float scale = (1.0f + hoverT * 0.02f) * clkS;
            float cCX = cx + cardW * 0.5f, cCY = cy + cardH * 0.5f;
            float dW  = cardW * scale, dH = cardH * scale;
            float dX  = cCX - dW * 0.5f, dY = cCY - dH * 0.5f;

            // Card bg
            ImU32 cardBg;
            if (hoverT > 0.01f) {
                cardBg = IM_COL32(22 + (int)(hoverT*13), 25 + (int)(hoverT*15),
                                  38 + (int)(hoverT*22), 200 + (int)(hoverT*55));
            } else {
                cardBg = IM_COL32(22, 25, 38, 200);
            }
            dl->AddRectFilled(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH), cardBg, 6.0f);

            // Border
            int ba = (int)(12.0f + hoverT * 50.0f);
            dl->AddRect(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH),
                        IM_COL32(74, 91, 207, ba), 6.0f, 0, 1.0f);

            // Hover glow
            if (hoverT > 0.05f) {
                int ga = (int)(hoverT * 15.0f);
                dl->AddRectFilled(ImVec2(dX - 2, dY - 2), ImVec2(dX + dW + 2, dY + dH + 2),
                                  IM_COL32(74, 91, 207, ga), 8.0f);
            }

            // Thumbnail
            float pad = 6.0f;
            float tH = dH * 0.55f, tW = dW - pad * 2.0f;
            float tX = dX + pad, tY = dY + pad;

            if (i < m_thumbTextures.size() &&
                m_thumbTextures[i].id != INVALID_TEXTURE.id) {
                dl->AddImage((ImTextureID)(uintptr_t)m_thumbTextures[i].id,
                             ImVec2(tX, tY), ImVec2(tX + tW, tY + tH));
            } else {
                dl->AddRectFilled(ImVec2(tX, tY), ImVec2(tX + tW, tY + tH),
                                  IM_COL32(18, 20, 32, 255), 3.0f);
                float pcx = tX + tW * 0.5f, pcy = tY + tH * 0.5f;
                dl->AddLine(ImVec2(pcx - 10, pcy), ImVec2(pcx + 10, pcy),
                            IM_COL32(74, 91, 207, 40));
                dl->AddLine(ImVec2(pcx, pcy - 6), ImVec2(pcx, pcy + 6),
                            IM_COL32(74, 91, 207, 40));
            }

            // Name
            float lx = dX + pad, ly = tY + tH + 4.0f;
            dl->AddText(ImGui::GetFont(), H * 0.017f,
                        ImVec2(lx, ly), kTextPrimary, scene.name.c_str());

            // Tag
            float tagW = ImGui::CalcTextSize(scene.category.c_str()).x + 8.0f;
            dl->AddText(ImGui::GetFont(), H * 0.012f,
                        ImVec2(lx, ly + H * 0.022f), kTextSecondary, scene.category.c_str());

            // Status dot
            if (scene.available) {
                dl->AddCircleFilled(ImVec2(lx + tagW + 12.0f, ly + H * 0.022f + 5.0f),
                                    3.0f, IM_COL32(100, 220, 120, 200));
            }

            // Click
            ImGui::SetCursorScreenPos(ImVec2(dX, dY));
            ImGui::InvisibleButton(("##gcard_" + std::to_string(i)).c_str(), ImVec2(dW, dH));
            if (ImGui::IsItemHovered()) { m_hoverCard = (int)i; m_heroIndex = (int)i; }
            if (ImGui::IsItemClicked() && scene.available) {
                m_pendingEnter = scene.id;
                if (i < m_cardAnims.size()) {
                    m_cardAnims[i].clickScale = 0.97f; m_cardAnims[i].clickVel = 0.0f;
                }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

// ============================================================================
// GetNextScene — factory dispatch
// ============================================================================

std::unique_ptr<Scene> SceneGalleryScene::GetNextScene() {
    if (m_pendingEnter.empty()) return nullptr;

    const auto& all = SceneRegistry::Instance().All();
    for (auto& e : all) {
        if (e.id == m_pendingEnter && e.factory) {
            printf("[SceneGallery] Entering scene: %s\n", e.name.c_str());
            return e.factory();
        }
    }
    return nullptr;
}
