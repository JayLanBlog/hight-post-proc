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

    // Ensure anim states match scene count
    if (m_cardAnims.size() != scenes.size()) {
        // Reset animations when scene count changes (category switch)
        if (!m_cardAnims.empty()) {
            for (auto& a : m_cardAnims) a.entryT = -1.0f;
        }
        m_cardAnims.resize(scenes.size());
    }

    // Update animations
    float dtClamped = std::min(dt, 0.1f);

    for (size_t i = 0; i < m_cardAnims.size(); i++) {
        auto& a = m_cardAnims[i];

        // Hover transition
        float hoverTarget = (m_hoverCard == (int)i) ? 1.0f : 0.0f;
        a.hoverT += (hoverTarget - a.hoverT) * std::min(1.0f, dtClamped * 10.0f);

        // Entry animation (stagger)
        if (a.entryT < 0.0f) {
            a.entryT = 0.0f;  // start entry
        }
        if (a.entryT < 1.0f) {
            a.entryT += dtClamped / 0.4f;  // 400ms duration
            if (a.entryT > 1.0f) a.entryT = 1.0f;
        }

        // Click spring
        a.clickVel += (1.0f - a.clickScale) * 20.0f * dtClamped;  // stiffness
        a.clickVel *= std::exp(-12.0f * dtClamped);                 // damping
        a.clickScale += a.clickVel * dtClamped;
        if (std::abs(a.clickScale - 1.0f) < 0.001f && std::abs(a.clickVel) < 0.01f) {
            a.clickScale = 1.0f;
            a.clickVel = 0.0f;
        }
    }

    // Entry stagger scheduling — start cards in sequence
    auto now = ImGui::GetTime();
    static double entryBaseTime = 0.0;
    if (m_cardAnims.size() > 0 && m_cardAnims[0].entryT < 0.01f && entryBaseTime == 0.0) {
        entryBaseTime = now;
    }
    // If anims were just resized, entryT is -1 again → reset timer
    if (m_cardAnims.size() > 0 && m_cardAnims[0].entryT < -0.5f) {
        entryBaseTime = now;
        for (size_t i = 0; i < m_cardAnims.size(); i++) {
            if (m_cardAnims[i].entryT < 0.0f) m_cardAnims[i].entryT = 0.0f;
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

        int w, h, comp;
        stbi_uc* data = stbi_load(e.thumbPath.c_str(), &w, &h, &comp, 4);
        if (data) {
            m_thumbTextures[i] = m_backend->CreateTexture(w, h, TextureFormat::RGBA8, data);
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
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) return;

    // Lazy-load thumbnails on first frame
    if (!m_thumbsLoaded) LoadThumbnails();

    // ---- Full-screen base window ----
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 1.0f));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    DrawBackground(w, h);
    DrawTopBar(w, h);
    DrawHero(w, h);
    DrawCategoryTabs(w, h);
    DrawCardGrid(w, h);

    ImGui::End();
    ImGui::PopStyleColor();
}

// ============================================================================
// Draw Background — ambient light / depth layers
// ============================================================================

void SceneGalleryScene::DrawBackground(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Layer 0 — deep void
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(w, h), kBgDeep);

    // Ambient light 1 — Hero glow (upper-left)
    float heroR  = h * 0.4f;
    dl->AddRectFilled(ImVec2(w * 0.3f - heroR, 0), ImVec2(w * 0.3f + heroR, h * 0.5f),
                      IM_COL32(74, 91, 207, 8));
    dl->AddRectFilled(ImVec2(w * 0.3f - heroR * 0.7f, 0), ImVec2(w, h * 0.45f),
                      IM_COL32(74, 91, 207, 6));

    // Ambient light 2 — bottom-right fill
    dl->AddRectFilled(ImVec2(w * 0.5f, h * 0.6f), ImVec2(w, h),
                      IM_COL32(74, 91, 207, 4));

    // Subtle grid lines (tech aesthetic)
    for (float y = h * 0.12f; y < h; y += h * 0.15f) {
        dl->AddLine(ImVec2(0, y), ImVec2(w, y),
                    IM_COL32(74, 91, 207, 3), 0.5f);
    }
}

// ============================================================================
// Top Bar — title + subtitle
// ============================================================================

void SceneGalleryScene::DrawTopBar(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float ty = h * 0.016f;

    // Title
    dl->AddText(ImGui::GetFont(), h * 0.035f,
                ImVec2(w * 0.05f, ty),
                kTextPrimary, "Scene Gallery");

    // Subtitle
    dl->AddText(ImGui::GetFont(), h * 0.016f,
                ImVec2(w * 0.05f, ty + h * 0.04f),
                kTextSecondary, "GPU Real-Time Effects Showcase");

    // Accent line under title
    dl->AddLine(ImVec2(w * 0.05f, ty + h * 0.06f + 2.0f),
                ImVec2(w * 0.05f + w * 0.06f, ty + h * 0.06f + 2.0f),
                kAccent, 2.0f);
}

// ============================================================================
// Hero — large featured scene card
// ============================================================================

void SceneGalleryScene::DrawHero(float /*w*/, float h) {
    const auto& scenes = SceneRegistry::Instance().All();
    if (scenes.empty()) return;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float hy  = h * 0.09f;
    float hh  = h * 0.25f;
    float hx  = h * 0.047f;   // ~5% margin (proportional to height for consistent look)
    float hw  = h * 0.85f;   // width proportional to height (maintain aspect across resolutions)

    // Clamp hero to screen width
    if (hx + hw > h * 1.5f) hw = h * 1.5f - hx;

    const auto& scene = scenes[m_heroIndex];

    // Hero card background
    dl->AddRectFilled(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                      IM_COL32(26, 28, 48, 230), 8.0f);

    // Border glow
    dl->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                IM_COL32(74, 91, 207, 40), 8.0f, 0, 1.0f);

    // Thumbnail area (left side, ~40% width)
    float thumbH = hh - 16.0f;
    float thumbW = thumbH * 1.78f;
    float thumbX = hx + 12.0f;
    float thumbY = hy + 8.0f;

    // Draw thumbnail or placeholder
    if (m_heroIndex < (int)m_thumbTextures.size() &&
        m_thumbTextures[m_heroIndex].id != INVALID_TEXTURE.id) {
        ImTextureID texId = (ImTextureID)(uintptr_t)m_thumbTextures[m_heroIndex].id;
        dl->AddImage(texId,
                     ImVec2(thumbX, thumbY),
                     ImVec2(thumbX + thumbW, thumbY + thumbH));
    } else {
        // Geometric placeholder
        dl->AddRectFilled(ImVec2(thumbX, thumbY),
                          ImVec2(thumbX + thumbW, thumbY + thumbH),
                          IM_COL32(20, 22, 38, 255), 4.0f);
        dl->AddRect(ImVec2(thumbX, thumbY),
                    ImVec2(thumbX + thumbW, thumbY + thumbH),
                    kAccent, 4.0f, 0, 0.5f);
        // Cross decoration
        float cx = thumbX + thumbW * 0.5f, cy = thumbY + thumbH * 0.5f;
        dl->AddLine(ImVec2(cx - 20, cy), ImVec2(cx + 20, cy), kTextTertiary, 1.0f);
        dl->AddLine(ImVec2(cx, cy - 12), ImVec2(cx, cy + 12), kTextTertiary, 1.0f);
    }

    // Text area
    float tx = thumbX + thumbW + 20.0f;

    // Scene name
    dl->AddText(ImGui::GetFont(), h * 0.028f,
                ImVec2(tx, hy + 14.0f),
                kTextPrimary, scene.name.c_str());

    // Category badge
    float badgeW = ImGui::CalcTextSize(scene.category.c_str()).x + 16.0f;
    dl->AddRectFilled(ImVec2(tx, hy + 14.0f + h * 0.035f),
                      ImVec2(tx + badgeW, hy + 14.0f + h * 0.035f + h * 0.024f),
                      IM_COL32(74, 91, 207, 80), 3.0f);
    dl->AddText(ImGui::GetFont(), h * 0.014f,
                ImVec2(tx + 8.0f, hy + 14.0f + h * 0.037f),
                kAccentHover, scene.category.c_str());

    // Description
    dl->AddText(ImGui::GetFont(), h * 0.017f,
                ImVec2(tx, hy + 14.0f + h * 0.07f),
                kTextSecondary, scene.description.c_str());

    // Status label
    float statusY = hy + hh - h * 0.04f - 8.0f;
    if (scene.available) {
        dl->AddRectFilled(ImVec2(tx, statusY),
                          ImVec2(tx + 6.0f, statusY + 6.0f),
                          IM_COL32(100, 220, 120, 255), 3.0f);
        dl->AddText(ImGui::GetFont(), h * 0.014f,
                    ImVec2(tx + 12.0f, statusY - 2.0f),
                    IM_COL32(100, 220, 120, 200), "Available");
    } else {
        dl->AddText(ImGui::GetFont(), h * 0.014f,
                    ImVec2(tx, statusY - 2.0f),
                    kTextTertiary, "Coming Soon");
    }

    // Enter button (right side)
    float btnW = h * 0.12f;
    float btnH = h * 0.05f;
    float btnX = hx + hw - btnW - 16.0f;
    float btnY = hy + hh - btnH - 12.0f;

    if (scene.available) {
        // Hover detection area
        ImGui::SetCursorScreenPos(ImVec2(btnX, btnY));
        ImGui::InvisibleButton("##hero_enter", ImVec2(btnW, btnH));
        bool btnHover = ImGui::IsItemHovered();

        ImU32 btnBg = btnHover ? kAccentHover : kAccent;
        dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                          btnBg, 4.0f);

        const char* enterText = "Enter";
        ImVec2 et = ImGui::CalcTextSize(enterText);
        dl->AddText(ImGui::GetFont(), h * 0.021f,
                    ImVec2(btnX + (btnW - et.x) * 0.5f,
                           btnY + (btnH - et.y) * 0.5f),
                    IM_COL32(255, 255, 255, 255), enterText);

        if (ImGui::IsItemClicked()) {
            m_pendingEnter = scene.id;

            // Click feedback on hero card
            for (size_t i = 0; i < m_cardAnims.size(); i++) {
                if (i == (size_t)m_heroIndex) {
                    m_cardAnims[i].clickScale = 0.97f;
                    m_cardAnims[i].clickVel = 0.0f;
                }
            }
        }
    } else {
        dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                          IM_COL32(40, 42, 55, 180), 4.0f);
        const char* csText = "Coming Soon";
        ImVec2 ct = ImGui::CalcTextSize(csText);
        dl->AddText(ImGui::GetFont(), h * 0.015f,
                    ImVec2(btnX + (btnW - ct.x) * 0.5f,
                           btnY + (btnH - ct.y) * 0.5f),
                    kTextTertiary, csText);
    }
}

// ============================================================================
// Category tabs — horizontal tab bar
// ============================================================================

void SceneGalleryScene::DrawCategoryTabs(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    auto cats = SceneRegistry::Instance().Categories();

    std::vector<std::string> items;
    items.push_back("全部");
    for (auto& c : cats) items.push_back(c);

    float ty = h * 0.36f;
    float tabH = h * 0.035f;
    float tabGap = 8.0f;
    float startX = w * 0.047f;

    for (size_t i = 0; i < items.size(); i++) {
        float tw = ImGui::CalcTextSize(items[i].c_str()).x + 24.0f;
        float tx = startX;
        for (size_t j = 0; j < i; j++) {
            tx += ImGui::CalcTextSize(items[j].c_str()).x + 24.0f + tabGap;
        }

        bool active = (items[i] == m_activeCategory);

        ImU32 bg = active ? IM_COL32(74, 91, 207, 60) : IM_COL32(30, 32, 50, 120);
        dl->AddRectFilled(ImVec2(tx, ty), ImVec2(tx + tw, ty + tabH), bg, 4.0f);

        if (active) {
            dl->AddRect(ImVec2(tx, ty), ImVec2(tx + tw, ty + tabH),
                        IM_COL32(74, 91, 207, 120), 4.0f, 0, 1.0f);
        }

        ImU32 tc = active ? kAccentHover : kTextSecondary;
        ImVec2 ts = ImGui::CalcTextSize(items[i].c_str());
        dl->AddText(ImGui::GetFont(), h * 0.017f,
                    ImVec2(tx + 12.0f, ty + (tabH - ts.y) * 0.5f),
                    tc, items[i].c_str());

        // Click
        ImGui::SetCursorScreenPos(ImVec2(tx, ty));
        ImGui::InvisibleButton(("##tab_" + std::to_string(i)).c_str(), ImVec2(tw, tabH));
        if (ImGui::IsItemClicked() && items[i] != m_activeCategory) {
            m_activeCategory = items[i];
            // Reset entry animations
            for (auto& a : m_cardAnims) a.entryT = -1.0f;
        }
    }

    // Separator line below tabs
    dl->AddLine(ImVec2(startX, ty + tabH + 4.0f),
                ImVec2(w - startX, ty + tabH + 4.0f),
                IM_COL32(74, 91, 207, 20), 1.0f);
}

// ============================================================================
// Card grid — 3-column layout with animations
// ============================================================================

void SceneGalleryScene::DrawCardGrid(float w, float h) {
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    if (scenes.empty()) {
        float cy = h * 0.42f;
        ImDrawList* wdl = ImGui::GetWindowDrawList();
        wdl->AddText(ImGui::GetFont(), h * 0.025f,
                    ImVec2(w * 0.4f, cy), kTextTertiary, "No scenes in this category");
        return;
    }

    int cols = 3;
    float margin = w * 0.047f;
    float gapX = 16.0f, gapY = 14.0f;
    float cardW = (w - margin * 2 - gapX * (cols - 1)) / (float)cols;
    float cardH = cardW * 0.7f;
    float gridY = h * 0.40f;
    float gridH = h - gridY - 8.0f;

    // Scrollable area
    ImGui::SetCursorScreenPos(ImVec2(margin, gridY));
    ImGui::BeginChild("##CardScroll", ImVec2(w - margin * 2, gridH), false,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

    ImGui::SetCursorScreenPos(ImVec2(margin, gridY));
    ImDrawList* cdl = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < scenes.size(); i++) {
        const auto& scene = scenes[i];
        int col = (int)i % cols;
        int row = (int)i / cols;

        float cx = margin + col * (cardW + gapX);
        float cy = gridY + row * (cardH + gapY);

        // Animation state
        auto& anim = (i < m_cardAnims.size()) ? m_cardAnims[i] : CardAnimState{};
        float entryT = (i < m_cardAnims.size()) ? anim.entryT : 1.0f;
        float hoverT = (i < m_cardAnims.size()) ? anim.hoverT : 0.0f;
        float clkS   = (i < m_cardAnims.size()) ? anim.clickScale : 1.0f;

        // Entry animation: translateY + opacity
        float entryY = (1.0f - std::min(entryT, 1.0f)) * 20.0f;
        float alpha  = std::min(entryT, 1.0f);
        if (entryT < 0.0f) continue; // not started

        // Hover scale
        float scale = 1.0f + hoverT * 0.02f;
        scale *= clkS;

        // Apply transforms: center-based scale + offset
        float cardCX = cx + cardW * 0.5f;
        float cardCY = cy + cardH * 0.5f;
        float drawW  = cardW * scale;
        float drawH  = cardH * scale;
        float drawX  = cardCX - drawW * 0.5f;
        float drawY  = cardCY - drawH * 0.5f + entryY;

        // Card background
        ImU32 cardBg;
        if (hoverT > 0.01f) {
            int r = (int)(22  + hoverT * 13);
            int g = (int)(25  + hoverT * 15);
            int b = (int)(38  + hoverT * 22);
            int a = (int)(200 + hoverT * 55);
            cardBg = IM_COL32(r,g,b,a);
        } else {
            cardBg = IM_COL32(22, 25, 38, 200);
        }
        cdl->AddRectFilled(ImVec2(drawX, drawY), ImVec2(drawX + drawW, drawY + drawH),
                          cardBg, 6.0f);

        // Border
        int borderAlpha = (int)(12.0f + hoverT * 50.0f);
        cdl->AddRect(ImVec2(drawX, drawY), ImVec2(drawX + drawW, drawY + drawH),
                    IM_COL32(74, 91, 207, borderAlpha), 6.0f, 0, 1.0f);

        // Glow shadow on hover
        if (hoverT > 0.05f) {
            int glowAlpha = (int)(hoverT * 15.0f);
            cdl->AddRectFilled(ImVec2(drawX - 2, drawY - 2),
                              ImVec2(drawX + drawW + 2, drawY + drawH + 2),
                              IM_COL32(74, 91, 207, glowAlpha), 8.0f);
        }

        // Thumbnail
        float pad = 6.0f;
        float thumbH = drawH * 0.55f;
        float thumbW = drawW - pad * 2;
        float thumbX = drawX + pad;
        float thumbY = drawY + pad;

        if (i < m_thumbTextures.size() &&
            m_thumbTextures[i].id != INVALID_TEXTURE.id) {
            ImTextureID texId = (ImTextureID)(uintptr_t)m_thumbTextures[i].id;
            cdl->AddImage(texId,
                          ImVec2(thumbX, thumbY),
                          ImVec2(thumbX + thumbW, thumbY + thumbH));
        } else {
            cdl->AddRectFilled(ImVec2(thumbX, thumbY),
                              ImVec2(thumbX + thumbW, thumbY + thumbH),
                              IM_COL32(18, 20, 32, 255), 3.0f);
            float tcx = thumbX + thumbW * 0.5f;
            float tcy = thumbY + thumbH * 0.5f;
            cdl->AddLine(ImVec2(tcx - 10, tcy), ImVec2(tcx + 10, tcy),
                        IM_COL32(74, 91, 207, 40), 1.0f);
            cdl->AddLine(ImVec2(tcx, tcy - 6), ImVec2(tcx, tcy + 6),
                        IM_COL32(74, 91, 207, 40), 1.0f);
        }

        // Scene name
        float tx = drawX + pad;
        float ty = thumbY + thumbH + 4.0f;
        int txtAlpha = (int)(alpha * 255.0f);
        cdl->AddText(ImGui::GetFont(), h * 0.018f,
                    ImVec2(tx, ty),
                    IM_COL32(240, 245, 255, txtAlpha),
                    scene.name.c_str());

        // Category tag
        float tagW = ImGui::CalcTextSize(scene.category.c_str()).x + 12.0f;
        float tagY = ty + h * 0.024f;
        int tagAlpha = (int)(alpha * 180.0f);
        cdl->AddText(ImGui::GetFont(), h * 0.012f,
                    ImVec2(tx, tagY),
                    IM_COL32(160, 170, 200, tagAlpha),
                    scene.category.c_str());

        // Status dot
        float dotY = tagY;
        float dotX = tx + tagW + 8.0f;
        if (scene.available) {
            int dotAlpha = (int)(alpha * 200.0f);
            cdl->AddCircleFilled(ImVec2(dotX + 3, dotY + 4), 3.0f,
                                IM_COL32(100, 220, 120, dotAlpha));
        }

        // Click interaction
        ImGui::SetCursorScreenPos(ImVec2(drawX, drawY));
        ImGui::InvisibleButton(("##gcard_" + std::to_string(i)).c_str(),
                               ImVec2(drawW, drawH));

        if (ImGui::IsItemHovered()) {
            m_hoverCard = (int)i;
            m_heroIndex = (int)i;  // update hero on hover
        }
        if (ImGui::IsItemClicked() && scene.available) {
            m_pendingEnter = scene.id;
            if (i < m_cardAnims.size()) {
                m_cardAnims[i].clickScale = 0.97f;
                m_cardAnims[i].clickVel = 0.0f;
            }
        }
    }

    ImGui::EndChild();
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
