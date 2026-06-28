#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "imgui.h"

#include "stb_image.h"

#include <cstdio>
#include <cmath>

void SceneGalleryScene::OnEnter() {
    m_activeCategory = "全部";
    m_pendingEnter.clear();
    m_hoverCard = -1;
    m_heroIdx   = 0;
    m_thumbsOk  = false;
}
void SceneGalleryScene::OnExit()  { FreeThumbs(); }
void SceneGalleryScene::OnRender(IRenderBackend* be) { m_be = be; }

void SceneGalleryScene::OnUpdate(float dt) {
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);
    if (m_anim.size() != scenes.size()) m_anim.assign(scenes.size(), CardAnim{});
    float d = std::min(dt, 0.1f);
    for (size_t i = 0; i < m_anim.size(); i++) {
        auto& a = m_anim[i];
        float ht = (m_hoverCard == (int)i) ? 1.0f : 0.0f;
        a.hoverT += (ht - a.hoverT) * std::min(1.0f, d * 12.0f);
        if (a.entryT < 0.0f) a.entryT = 0.0f;
        if (a.entryT < 1.0f) { a.entryT += d / 0.4f; if (a.entryT > 1.0f) a.entryT = 1.0f; }
        a.clickVel += (1.0f - a.clickScale) * 25.0f * d;
        a.clickVel *= std::exp(-14.0f * d);
        a.clickScale += a.clickVel * d;
        if (std::fabs(a.clickScale - 1.0f) < 0.001f && std::fabs(a.clickVel) < 0.02f) {
            a.clickScale = 1.0f; a.clickVel = 0.0f;
        }
    }
}

void SceneGalleryScene::LoadThumbs() {
    if (m_thumbsOk || !m_be) return;
    const auto& all = SceneRegistry::Instance().All();
    m_thumbs.resize(all.size());
    m_thumbDims.resize(all.size(), {0,0});
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].thumbPath.empty()) continue;
        int w, h, c;
        stbi_uc* d = stbi_load(all[i].thumbPath.c_str(), &w, &h, &c, 4);
        if (d) {
            m_thumbs[i] = m_be->CreateTexture(w, h, TextureFormat::RGBA8, d);
            m_thumbDims[i] = {w, h};
            stbi_image_free(d);
        }
    }
    m_thumbsOk = true;
}

void SceneGalleryScene::FreeThumbs() {
    if (!m_be) { m_thumbs.clear(); m_thumbDims.clear(); m_thumbsOk = false; return; }
    for (auto& t : m_thumbs) if (t.id != INVALID_TEXTURE.id) m_be->DestroyTexture(t);
    m_thumbs.clear(); m_thumbDims.clear(); m_thumbsOk = false;
}

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float W = io.DisplaySize.x, H = io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;
    if (!m_thumbsOk) LoadThumbs();

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

    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), kDeep);
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H * 0.55f), IM_COL32(40, 35, 80, 6));
    dl->AddRectFilled(ImVec2(W * 0.2f, H * 0.05f), ImVec2(W * 0.8f, H * 0.50f),
                      IM_COL32(74, 91, 207, 7));
    dl->AddRectFilled(ImVec2(W * 0.3f, H * 0.10f), ImVec2(W * 0.7f, H * 0.45f),
                      IM_COL32(60, 70, 180, 5));

    const auto& all = SceneRegistry::Instance().All();
    float y = H * 0.006f;

    dl->AddText(ImGui::GetFont(), H * 0.028f, ImVec2(M, y), kText1, "Scene Gallery");
    y += H * 0.025f;

    // ==== HERO ====
    if (!all.empty()) {
        int hi = m_heroIdx % (int)all.size();
        const auto& s = all[hi];
        float hh = H * 0.46f, hw = W - M * 3.0f;
        float hx = (W - hw) * 0.5f, hy = y;

        dl->AddRectFilled(ImVec2(hx - hw * 0.35f, hy - hh * 0.10f),
                          ImVec2(hx + hw * 1.35f, hy + hh * 1.10f),
                          IM_COL32(74, 91, 207, 10), 120.0f);
        dl->AddRectFilled(ImVec2(hx - hw * 0.15f, hy - hh * 0.05f),
                          ImVec2(hx + hw * 1.15f, hy + hh * 1.05f),
                          IM_COL32(40, 48, 140, 14), 80.0f);

        dl->AddRectFilled(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh), kHeroBg, 12.0f);
        dl->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh), IM_COL32(90, 105, 220, 45), 12.0f);

        // thumbnail — aspect-correct fit
        float th = hh * 0.62f, tw = th * 1.78f;
        float tcx = hx + hw * 0.5f;
        float tY = hy + hh * 0.06f;
        float tX = tcx - tw * 0.5f;

        // Compute fitted rect preserving image aspect ratio
        float imgW = (float)tw, imgH = (float)th;
        if (hi < (int)m_thumbDims.size() && m_thumbDims[hi].first > 0) {
            float iar = (float)m_thumbDims[hi].first / (float)m_thumbDims[hi].second;
            float bar = tw / th;
            if (iar > bar) { imgH = tw / iar; imgW = tw; }
            else          { imgW = th * iar; imgH = th; }
            tX = tcx - imgW * 0.5f;
            tY = tY + (th - imgH) * 0.5f;
        }

        if (hi < (int)m_thumbs.size() && m_thumbs[hi].id != INVALID_TEXTURE.id && m_be) {
            void* tid = m_be->GetImTextureID(m_thumbs[hi]);
            if (tid) dl->AddImage(tid, ImVec2(tX, tY), ImVec2(tX + imgW, tY + imgH));
            else dl->AddRectFilled(ImVec2(tX, tY), ImVec2(tX + tw, tY + th),
                                    IM_COL32(20, 22, 38, 255), 6.0f);
        } else {
            dl->AddRectFilled(ImVec2(tX, tY), ImVec2(tX + tw, tY + th),
                              IM_COL32(20, 22, 38, 255), 8.0f);
            dl->AddRect(ImVec2(tX, tY), ImVec2(tX + tw, tY + th), IM_COL32(74, 91, 207, 30), 8.0f);
        }

        // name + description (both centered)
        float tty = tY + th + hh * 0.03f;
        ImVec2 ns = ImGui::CalcTextSize(s.name.c_str());
        dl->AddText(ImGui::GetFont(), H * 0.030f,
                    ImVec2(tcx - ns.x * 0.5f, tty), kText1, s.name.c_str());
        tty += H * 0.038f;
        ImVec2 ds = ImGui::CalcTextSize(s.description.c_str());
        dl->AddText(ImGui::GetFont(), H * 0.015f,
                    ImVec2(tcx - ds.x * 0.5f, tty),
                    IM_COL32(120, 130, 160, 180), s.description.c_str());

        // click
        ImGui::SetCursorScreenPos(ImVec2(hx, hy));
        ImGui::InvisibleButton("##heroClick", ImVec2(hw, hh));
        if (ImGui::IsItemClicked() && s.available) m_pendingEnter = s.id;

        y = hy + hh + H * 0.010f;
    }

    // ==== CATEGORY TABS ====
    auto cats = SceneRegistry::Instance().Categories();
    std::vector<std::string> tabs = {"全部"};
    for (auto& c : cats) tabs.push_back(c);

    float tx = M, tH = H * 0.030f;
    for (size_t i = 0; i < tabs.size(); i++) {
        float tW = ImGui::CalcTextSize(tabs[i].c_str()).x + 26.0f;
        bool sel = (tabs[i] == m_activeCategory);
        ImU32 bg = sel ? IM_COL32(74, 91, 207, 75) : IM_COL32(28, 31, 48, 130);
        dl->AddRectFilled(ImVec2(tx, y), ImVec2(tx + tW, y + tH), bg, 4.0f);
        if (sel) dl->AddRect(ImVec2(tx, y), ImVec2(tx + tW, y + tH), IM_COL32(107,124,232,150), 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(tabs[i].c_str());
        ImU32 tc = sel ? IM_COL32(200,210,255,255) : IM_COL32(140,150,180,200);
        dl->AddText(ImGui::GetFont(), H * 0.014f,
                    ImVec2(tx + (tW - ts.x) * 0.5f, y + (tH - ts.y) * 0.5f), tc, tabs[i].c_str());
        ImGui::SetCursorScreenPos(ImVec2(tx, y));
        ImGui::InvisibleButton(("##tab" + std::to_string(i)).c_str(), ImVec2(tW, tH));
        if (ImGui::IsItemClicked()) m_activeCategory = tabs[i];
        tx += tW + 7.0f;
    }
    y += tH + 8.0f;
    dl->AddLine(ImVec2(M, y), ImVec2(W - M, y), IM_COL32(74, 91, 207, 15), 1.0f);
    y += 6.0f;

    // ==== CARD GRID ====
    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    if (!scenes.empty()) {
        const int COLS = 3;
        float gX = W * 0.015f, gY = 10.0f;
        float cW = (W - M * 2.0f - gX * (COLS - 1)) / (float)COLS;
        float cH = cW * 0.45f;

        for (size_t i = 0; i < scenes.size(); i++) {
            int col = (int)i % COLS, row = (int)i / COLS;
            float cx = M + col * (cW + gX);
            float cy = y + row * (cH + gY);
            if (cy + cH < 0 || cy > H) continue;

            auto& a = (i < m_anim.size()) ? m_anim[i] : CardAnim{};
            float entryT = (i < m_anim.size()) ? a.entryT : 1.0f;
            float hoverT = (i < m_anim.size()) ? a.hoverT : 0.0f;
            float clkS   = (i < m_anim.size()) ? a.clickScale : 1.0f;
            if (entryT < 0.0f) continue;

            float sY = (1.0f - std::min(entryT, 1.0f)) * 20.0f;
            float alpha = std::min(entryT, 1.0f);
            float sc = (1.0f + hoverT * 0.015f) * clkS;
            float mcX = cx + cW * 0.5f, mcY = cy + cH * 0.5f;
            float dW = cW * sc, dH = cH * sc;
            float dX = mcX - dW * 0.5f, dY = mcY - dH * 0.5f + sY;

            int br = 22 + (int)(hoverT * 16), bgv = 25 + (int)(hoverT * 18);
            int bb = 38 + (int)(hoverT * 26), ba = (int)(alpha * 200 + hoverT * 55);
            dl->AddRectFilled(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH),
                              IM_COL32(br, bgv, bb, ba), 6.0f);

            if (hoverT > 0.02f) {
                int bo = (int)(hoverT * 70.0f);
                dl->AddRect(ImVec2(dX, dY), ImVec2(dX + dW, dY + dH),
                            IM_COL32(74, 91, 207, bo), 6.0f);
                dl->AddRectFilled(ImVec2(dX - 2, dY - 2), ImVec2(dX + dW + 2, dY + dH + 2),
                                  IM_COL32(74, 91, 207, (int)(hoverT * 14)), 8.0f);
            }

            float tp = 3.0f;
            float thAreaW = dW - tp * 2.0f;
            float thAreaH = dH * 0.72f;
            float thX = dX + tp, thY = dY + tp;

            // Fit image within card area, preserving aspect ratio
            float imgW = thAreaW, imgH = thAreaH;
            if (i < (int)m_thumbDims.size() && m_thumbDims[i].first > 0) {
                float iar = (float)m_thumbDims[i].first / (float)m_thumbDims[i].second;
                float bar = thAreaW / thAreaH;
                if (iar > bar) { imgH = thAreaW / iar; imgW = thAreaW; }
                else           { imgW = thAreaH * iar; imgH = thAreaH; }
                thX = dX + tp + (thAreaW - imgW) * 0.5f;
                thY = dY + tp + (thAreaH - imgH) * 0.5f;
            }

            if (i < (int)m_thumbs.size() && m_thumbs[i].id != INVALID_TEXTURE.id && m_be) {
                void* tid = m_be->GetImTextureID(m_thumbs[i]);
                if (tid) dl->AddImage(tid, ImVec2(thX, thY), ImVec2(thX + imgW, thY + imgH));
                else dl->AddRectFilledMultiColor(ImVec2(dX + tp, dY + tp), ImVec2(dX + tp + thAreaW, dY + tp + thAreaH),
                    IM_COL32(30, 35, 60, 255), IM_COL32(40, 50, 80, 255),
                    IM_COL32(40, 50, 80, 255), IM_COL32(30, 35, 60, 255));
            } else {
                dl->AddRectFilledMultiColor(ImVec2(dX + tp, dY + tp), ImVec2(dX + tp + thAreaW, dY + tp + thAreaH),
                    IM_COL32(25, 30, 55, 255), IM_COL32(35, 45, 75, 255),
                    IM_COL32(35, 45, 75, 255), IM_COL32(25, 30, 55, 255));
            }

            float lx = dX + tp, ly = dY + tp + thAreaH + 2.0f;
            int ta = (int)(alpha * 255.0f);
            dl->AddText(ImGui::GetFont(), H * 0.014f, ImVec2(lx, ly),
                        IM_COL32(210, 220, 240, ta), scenes[i].name.c_str());

            ImGui::SetCursorScreenPos(ImVec2(dX, dY));
            ImGui::InvisibleButton(("##gc" + std::to_string(i)).c_str(), ImVec2(dW, dH));
            if (ImGui::IsItemHovered()) { m_hoverCard = (int)i; m_heroIdx = (int)i; }
            if (ImGui::IsItemClicked() && scenes[i].available) {
                m_pendingEnter = scenes[i].id;
                if (i < m_anim.size()) { a.clickScale = 0.95f; a.clickVel = 0.0f; }
            }
        }
    }

    // ---- Keyboard: ENTER to enter first available scene ----
    if (m_pendingEnter.empty() && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        const auto& sc = (m_activeCategory == "全部")
            ? SceneRegistry::Instance().All()
            : SceneRegistry::Instance().ByCategory(m_activeCategory);
        if (!sc.empty() && sc[0].available) {
            m_pendingEnter = sc[0].id;
            printf("[SceneGalleryScene] ENTER: entering scene '%s'\n", sc[0].id.c_str());
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

std::unique_ptr<Scene> SceneGalleryScene::GetNextScene() {
    if (m_pendingEnter.empty()) return nullptr;
    const auto& a = SceneRegistry::Instance().All();
    for (auto& e : a) {
        if (e.id == m_pendingEnter) {
            if (e.factory) {
                printf("[SceneGalleryScene] GetNextScene: invoking factory for '%s'\n", m_pendingEnter.c_str());
                fflush(stdout);
                try {
                    auto scene = e.factory();
                    printf("[SceneGalleryScene] GetNextScene: factory returned %s\n",
                           scene ? "valid scene" : "NULL");
                    fflush(stdout);
                    return scene;
                } catch (const std::exception& ex) {
                    fprintf(stderr, "[SceneGalleryScene] FACTORY EXCEPTION: %s\n", ex.what());
                    fflush(stderr);
                } catch (...) {
                    fprintf(stderr, "[SceneGalleryScene] FACTORY UNKNOWN EXCEPTION\n");
                    fflush(stderr);
                }
            } else {
                fprintf(stderr, "[SceneGalleryScene] GetNextScene: factory is NULL for '%s'\n",
                        m_pendingEnter.c_str());
                fflush(stderr);
            }
        }
    }
    fprintf(stderr, "[SceneGalleryScene] GetNextScene: entry '%s' not found\n", m_pendingEnter.c_str());
    fflush(stderr);
    return nullptr;
}
