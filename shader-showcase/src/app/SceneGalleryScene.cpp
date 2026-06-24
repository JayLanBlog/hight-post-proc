#include "app/SceneGalleryScene.h"
#include "app/SceneRegistry.h"
#include "app/Application.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>

void SceneGalleryScene::OnEnter() {
    printf("[SceneGallery] Entered, %zu scenes registered\n",
           SceneRegistry::Instance().All().size());
}

void SceneGalleryScene::OnUpdate(float /*dt*/) {}
void SceneGalleryScene::OnRender(IRenderBackend* /*backend*/) {}

void SceneGalleryScene::OnImGui() {
    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) return;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.10f, 1.0f));
    ImGui::Begin("##Gallery", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(ImGui::GetFont(), h * 0.045f,
                ImVec2(w * 0.04f, h * 0.04f),
                IM_COL32(200, 210, 255, 255), "Scene Gallery");

    DrawSidebar(w, h);
    DrawCardList(w, h);

    ImGui::End();
    ImGui::PopStyleColor();
}

void SceneGalleryScene::DrawSidebar(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float sx = 0;
    float sy = h * 0.11f;
    float sw = m_sidebarW;
    float sh = h - sy;

    dl->AddRectFilled(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh),
                      IM_COL32(16, 18, 26, 220));

    auto cats = SceneRegistry::Instance().Categories();

    std::vector<std::string> items;
    items.push_back("全部");
    for (auto& c : cats) items.push_back(c);

    float itemH = h * 0.045f;
    float y = sy + 12.0f;

    for (size_t i = 0; i < items.size(); i++) {
        bool active = (items[i] == m_activeCategory);
        float ix = sx + 8.0f;
        float iw = sw - 16.0f;

        if (active) {
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + iw, y + itemH),
                              IM_COL32(55, 60, 90, 200), 4.0f);
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + 3.0f, y + itemH),
                              IM_COL32(140, 150, 220, 255));
        } else if (m_hoverCard == -(int)(i + 1)) {
            dl->AddRectFilled(ImVec2(ix, y), ImVec2(ix + iw, y + itemH),
                              IM_COL32(26, 28, 40, 160), 4.0f);
        }

        char label[128];
        const auto& list = (items[i] == "全部")
            ? SceneRegistry::Instance().All()
            : SceneRegistry::Instance().ByCategory(items[i]);
        snprintf(label, sizeof(label), "%s (%zu)", items[i].c_str(), list.size());

        ImU32 tc = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 170, 200, 220);
        dl->AddText(ImGui::GetFont(), h * 0.019f,
                    ImVec2(ix + 14.0f, y + (itemH - h * 0.019f) * 0.5f),
                    tc, label);

        ImGui::SetCursorScreenPos(ImVec2(ix, y));
        ImGui::InvisibleButton(("##cat_" + std::to_string(i)).c_str(), ImVec2(iw, itemH));
        if (ImGui::IsItemHovered()) m_hoverCard = -(int)(i + 1);
        if (ImGui::IsItemClicked()) m_activeCategory = items[i];

        y += itemH + 4.0f;
    }
}

void SceneGalleryScene::DrawCardList(float w, float h) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float lx = m_sidebarW + 16.0f;
    float ly = h * 0.11f;
    float rw = w - lx - 16.0f;

    const auto& scenes = (m_activeCategory == "全部")
        ? SceneRegistry::Instance().All()
        : SceneRegistry::Instance().ByCategory(m_activeCategory);

    float cardH = h * 0.12f;
    float gap   = 10.0f;

    for (size_t i = 0; i < scenes.size(); i++) {
        const auto& e = scenes[i];
        float cy = ly + i * (cardH + gap);
        if (cy + cardH < ly || cy > h) continue;

        bool hover = (m_hoverCard == (int)i);
        ImU32 bg = hover ? IM_COL32(35, 40, 60, 240) : IM_COL32(22, 25, 38, 200);
        dl->AddRectFilled(ImVec2(lx, cy), ImVec2(lx + rw, cy + cardH), bg, 6.0f);

        float thumbW = cardH * 1.78f;
        float thumbH = cardH - 6.0f;
        dl->AddRectFilled(ImVec2(lx + 8.0f, cy + 3.0f),
                          ImVec2(lx + 8.0f + thumbW, cy + 3.0f + thumbH),
                          IM_COL32(35, 40, 55, 255), 4.0f);
        dl->AddText(ImGui::GetFont(), h * 0.02f,
                    ImVec2(lx + 8.0f + thumbW * 0.3f, cy + thumbH * 0.42f),
                    IM_COL32(120, 130, 160, 160), "Preview");

        float tx = lx + 8.0f + thumbW + 16.0f;
        dl->AddText(ImGui::GetFont(), h * 0.025f,
                    ImVec2(tx, cy + 10.0f),
                    IM_COL32(255, 255, 255, 255), e.name.c_str());

        dl->AddText(ImGui::GetFont(), h * 0.018f,
                    ImVec2(tx, cy + h * 0.05f),
                    IM_COL32(140, 150, 170, 200), e.description.c_str());

        float btnW = h * 0.08f;
        float btnH = h * 0.035f;
        float btnX = lx + rw - btnW - 16.0f;
        float btnY = cy + (cardH - btnH) * 0.5f;

        if (e.available) {
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                              IM_COL32(72, 78, 120, 255), 4.0f);
            dl->AddText(ImGui::GetFont(), h * 0.018f,
                        ImVec2(btnX + btnW * 0.22f, btnY + btnH * 0.18f),
                        IM_COL32(255, 255, 255, 255), "Enter");
        } else {
            dl->AddRectFilled(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH),
                              IM_COL32(35, 38, 50, 200), 4.0f);
            dl->AddText(ImGui::GetFont(), h * 0.015f,
                        ImVec2(btnX + btnW * 0.12f, btnY + btnH * 0.2f),
                        IM_COL32(100, 100, 120, 180), "Coming Soon");
        }

        ImGui::SetCursorScreenPos(ImVec2(lx, cy));
        ImGui::InvisibleButton(("##card_" + std::to_string(i)).c_str(),
                               ImVec2(rw, cardH));
        if (ImGui::IsItemHovered()) m_hoverCard = (int)i;
        if (ImGui::IsItemClicked() && e.available) {
            m_pendingEnter = e.id;
        }
    }
}

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
