#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "imgui.h"

#include "stb_image.h"

#include <cstdio>
#include <cmath>

// ===== life =====

void SceneGalleryScene::OnEnter() {
    m_activeCategory = "全部";
    m_pendingEnter.clear();
    m_hoverCard = -1;
    m_heroIdx   = 0;
    m_thumbsOk  = false;
}

void SceneGalleryScene::OnExit()  { FreeThumbs(); }
void SceneGalleryScene::OnRender(IRenderBackend* be) { m_be = be; }

// ===== anim update =====

void SceneGalleryScene::OnUpdate(float dt) {
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    // Resize anim array on category switch
    if (m_anim.size() != scenes.size()) {
        m_anim.assign(scenes.size(), CardAnim{});
    }

    float d = std::min(dt, 0.1f);
    for (size_t i = 0; i < m_anim.size(); i++) {
        auto& a = m_anim[i];

        // hover lerp (200ms)
        float ht = (m_hoverCard == (int)i) ? 1.0f : 0.0f;
        a.hoverT += (ht - a.hoverT) * std::min(1.0f, d * 12.0f);

        // entry stagger: start at i*80ms delay, 400ms duration
        if (a.entryT < 0.0f) a.entryT = 0.0f;
        if (a.entryT < 1.0f) {
            a.entryT += d / 0.4f;
            if (a.entryT > 1.0f) a.entryT = 1.0f;
        }

        // click spring
        a.clickVel += (1.0f - a.clickScale) * 25.0f * d;
        a.clickVel *= std::exp(-14.0f * d);
        a.clickScale += a.clickVel * d;
        if (std::fabs(a.clickScale - 1.0f) < 0.001f && std::fabs(a.clickVel) < 0.02f) {
            a.clickScale = 1.0f; a.clickVel = 0.0f;
        }
    }
}

// ===== thumbnails =====

void SceneGalleryScene::LoadThumbs() {
    if (m_thumbsOk || !m_be) return;
    const auto& all = SceneRegistry::Instance().All();
    m_thumbs.resize(all.size());
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].thumbPath.empty()) continue;
        int w, h, c;
        stbi_uc* d = stbi_load(all[i].thumbPath.c_str(), &w, &h, &c, 4);
        if (d) {
            m_thumbs[i] = m_be->CreateTexture(w, h, TextureFormat::RGBA8, d);
            stbi_image_free(d);
        }
    }
    m_thumbsOk = true;
}

void SceneGalleryScene::FreeThumbs() {
    if (!m_be) { m_thumbs.clear(); m_thumbsOk = false; return; }
    for (auto& t : m_thumbs) {
        if (t.id != INVALID_TEXTURE.id) m_be->DestroyTexture(t);
    }
    m_thumbs.clear();
    m_thumbsOk = false;
}

// ===== main draw =====

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;

    if (!m_thumbsOk) LoadThumbs();

    // Full-screen window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float M = W * 0.04f;

    // Ambient
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), kDeep);
    dl->AddRectFilled(ImVec2(W * 0.1f, 0), ImVec2(W * 0.55f, H * 0.33f),
                      IM_COL32(74, 91, 207, 5));

    float y = H * 0.012f;

    // ---- 1. TOP BAR ----
    dl->AddText(ImGui::GetFont(), H * 0.033f, ImVec2(M, y), kText1, "Scene Gallery");
    y += H * 0.042f;
    dl->AddText(ImGui::GetFont(), H * 0.014f, ImVec2(M, y), kText2, "GPU Real-Time Effects Showcase");
    y += H * 0.026f;

    // ---- 2. HERO ----
    const auto& all = SceneRegistry::Instance().All();
    if (!all.empty()) {
        int hi = m_heroIdx % (int)all.size();
        const auto& s = all[hi];
        float hh = H * 0.20f, hw = W - M * 2.0f;
        float hx = M, hy = y;

        dl->AddRectFilled(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh), kHeroBg, 8.0f);
        dl->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh), kAccentDim, 8.0f);

        // thumb (use GetImTextureID properly)
        float th = hh - 12.0f, tw = th * 1.78f;
        float tx = hx + 8.0f, ty = hy + 6.0f;
        if (hi < (int)m_thumbs.size() && m_thumbs[hi].id != INVALID_TEXTURE.id && m_be) {
            void* texId = m_be->GetImTextureID(m_thumbs[hi]);
            if (texId) {
                dl->AddImage(texId, ImVec2(tx, ty), ImVec2(tx + tw, ty + th));
            } else {
                dl->AddRectFilled(ImVec2(tx, ty), ImVec2(tx + tw, ty + th),
                                  IM_COL32(20, 22, 38, 255), 4.0f);
            }
        } else {
            dl->AddRectFilled(ImVec2(tx, ty), ImVec2(tx + tw, ty + th),
                              IM_COL32(20, 22, 38, 255), 4.0f);
            dl->AddRect(ImVec2(tx, ty), ImVec2(tx + tw, ty + th), kAccentDim, 4.0f);
        }

        // name + badge + desc
        float lx = tx + tw + 18.0f;
        dl->AddText(ImGui::GetFont(), H * 0.026f, ImVec2(lx, hy + 10.0f), kText1, s.name.c_str());
        float bw = ImGui::CalcTextSize(s.category.c_str()).x + 18.0f;
        dl->AddRectFilled(ImVec2(lx, hy + 10 + H * 0.034f),
                          ImVec2(lx + bw, hy + 10 + H * 0.034f + H * 0.022f),
                          IM_COL32(74, 91, 207, 70), 3.0f);
        dl->AddText(ImGui::GetFont(), H * 0.013f,
                    ImVec2(lx + 9.0f, hy + 10 + H * 0.036f),
                    IM_COL32(160, 175, 240, 255), s.category.c_str());
        dl->AddText(ImGui::GetFont(), H * 0.015f,
                    ImVec2(lx, hy + 10 + H * 0.062f), kText2, s.description.c_str());

        // Enter button
        float bW = H * 0.10f, bH = H * 0.038f;
        float bX = hx + hw - bW - 12.0f, bY = hy + hh - bH - 8.0f;
        ImGui::SetCursorScreenPos(ImVec2(bX, bY));
        if (ImGui::Button(s.available ? "Enter" : "...", ImVec2(bW, bH))) {
            if (s.available) m_pendingEnter = s.id;
        }
        y = hy + hh;
    }

    // ---- 3. CATEGORY TABS ----
    y += H * 0.016f;
    auto cats = SceneRegistry::Instance().Categories();
    std::vector<std::string> tabs = {"全部"};
    for (auto& c : cats) tabs.push_back(c);

    float tX = M, tH = H * 0.032f;
    for (size_t i = 0; i < tabs.size(); i++) {
        float tW = ImGui::CalcTextSize(tabs[i].c_str()).x + 28.0f;
        bool sel = (tabs[i] == m_activeCategory);
        ImU32 bg = sel ? IM_COL32(74, 91, 207, 70) : IM_COL32(30, 33, 50, 140);
        dl->AddRectFilled(ImVec2(tX, y), ImVec2(tX + tW, y + tH), bg, 4.0f);
        if (sel) dl->AddRect(ImVec2(tX, y), ImVec2(tX + tW, y + tH),
                             IM_COL32(107,124,232,140), 4.0f);

        ImVec2 ts = ImGui::CalcTextSize(tabs[i].c_str());
        ImU32 tc = sel ? IM_COL32(200,210,255,255) : IM_COL32(150,160,190,200);
        dl->AddText(ImGui::GetFont(), H * 0.015f,
                    ImVec2(tX + (tW - ts.x) * 0.5f, y + (tH - ts.y) * 0.5f),
                    tc, tabs[i].c_str());
        ImGui::SetCursorScreenPos(ImVec2(tX, y));
        ImGui::InvisibleButton(("##tab" + std::to_string(i)).c_str(), ImVec2(tW, tH));
        if (ImGui::IsItemClicked()) m_activeCategory = tabs[i];
        tX += tW + 8.0f;
    }
    y += tH + 10.0f;
    dl->AddLine(ImVec2(M, y), ImVec2(W - M, y), IM_COL32(74, 91, 207, 18), 1.0f);
    y += 8.0f;

    // ---- 4. CARD GRID with ANIMATIONS ----
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    if (!scenes.empty()) {
        const int COLS = 3;
        float gX = 14.0f, gY = 14.0f;
        float cW = (W - M * 2.0f - gX * (COLS - 1)) / (float)COLS;
        float cH = cW * 0.72f;

        for (size_t i = 0; i < scenes.size(); i++) {
            int col = (int)i % COLS, row = (int)i / COLS;
            float cx = M + col * (cW + gX);
            float cy = y + row * (cH + gY);
            if (cy + cH < 0 || cy > H) continue;

            // Animation state
            auto& a = (i < m_anim.size()) ? m_anim[i] : CardAnim{};
            float entryT = (i < m_anim.size()) ? a.entryT : 1.0f;
            float hoverT = (i < m_anim.size()) ? a.hoverT : 0.0f;
            float clkS   = (i < m_anim.size()) ? a.clickScale : 1.0f;
            if (entryT < 0.0f) continue;

            // entry: slide up + fade
            float sY = (1.0f - std::min(entryT, 1.0f)) * 24.0f;
            float alpha = std::min(entryT, 1.0f);

            // scale: hover 1.02x + click spring
            float sc = (1.0f + hoverT * 0.02f) * clkS;
            float ccX = cx + cW * 0.5f, ccY = cy + cH * 0.5f;
            float dW = cW * sc, dH = cH * sc;
            float dX = ccX - dW * 0.5f, dY = ccY - dH * 0.5f + sY;

            // card bg
            int br = 22 + (int)(hoverT * 15);
            int bg = 25 + (int)(hoverT * 17);
            int bb = 38 + (int)(hoverT * 24);
            int ba = (int)(200 * alpha + hoverT * 55);
            dl->AddRectFilled(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH),
                              IM_COL32(br, bg, bb, ba), 6.0f);

            // hover border
            if (hoverT > 0.02f) {
                int bo = (int)(hoverT * 65.0f);
                dl->AddRect(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH),
                            IM_COL32(74, 91, 207, bo), 6.0f);
                // glow
                dl->AddRectFilled(ImVec2(dX - 2, dY - 2), ImVec2(dX + dW + 2, dY + dH + 2),
                                  IM_COL32(74, 91, 207, (int)(hoverT * 12)), 8.0f);
            }

            // thumb
            float tp = 5.0f;
            float thH = dH * 0.55f, thW = dW - tp * 2.0f;
            float thX = dX + tp, thY = dY + tp;
            if (i < m_thumbs.size() && m_thumbs[i].id != INVALID_TEXTURE.id && m_be) {
                void* tid = m_be->GetImTextureID(m_thumbs[i]);
                if (tid) dl->AddImage(tid, ImVec2(thX, thY), ImVec2(thX + thW, thY + thH));
                else dl->AddRectFilled(ImVec2(thX, thY), ImVec2(thX + thW, thY + thH),
                                       IM_COL32(18, 20, 32, 255), 3.0f);
            } else {
                // Deformed placeholder
                dl->AddRectFilled(ImVec2(thX, thY), ImVec2(thX + thW, thY + thH),
                                  IM_COL32(18, 20, 32, 255), 3.0f);
                float pcx = thX + thW * 0.5f, pcy = thY + thH * 0.5f;
                dl->AddLine(ImVec2(pcx - 9, pcy), ImVec2(pcx + 9, pcy), kAccentDim);
                dl->AddLine(ImVec2(pcx, pcy - 5), ImVec2(pcx, pcy + 5), kAccentDim);
            }

            // text
            float lx2 = dX + tp, ly2 = thY + thH + 3.0f;
            int ta = (int)(alpha * 255.0f);
            dl->AddText(ImGui::GetFont(), H * 0.016f, ImVec2(lx2, ly2),
                        IM_COL32(230,238,255, ta), scenes[i].name.c_str());
            float tagW = ImGui::CalcTextSize(scenes[i].category.c_str()).x + 8.0f;
            dl->AddText(ImGui::GetFont(), H * 0.012f, ImVec2(lx2, ly2 + H * 0.021f),
                        IM_COL32(140,150,180, ta), scenes[i].category.c_str());
            if (scenes[i].available) {
                dl->AddCircleFilled(ImVec2(lx2 + tagW + 12.0f, ly2 + H * 0.021f + 5.0f),
                                    3.0f, IM_COL32(100,220,120, ta));
            }

            // click / hover interactive
            ImGui::SetCursorScreenPos(ImVec2(dX, dY));
            ImGui::InvisibleButton(("##gc" + std::to_string(i)).c_str(), ImVec2(dW, dH));
            if (ImGui::IsItemHovered()) { m_hoverCard = (int)i; m_heroIdx = (int)i; }
            if (ImGui::IsItemClicked() && scenes[i].available) {
                m_pendingEnter = scenes[i].id;
                if (i < m_anim.size()) { a.clickScale = 0.95f; a.clickVel = 0.0f; }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ===== transition =====

std::unique_ptr<Scene> SceneGalleryScene::GetNextScene() {
    if (m_pendingEnter.empty()) return nullptr;
    const auto& a = SceneRegistry::Instance().All();
    for (auto& e : a) if (e.id == m_pendingEnter && e.factory) return e.factory();
    return nullptr;
}
