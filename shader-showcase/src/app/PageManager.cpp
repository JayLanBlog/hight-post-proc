#include "app/PageManager.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <cstdio>

// ============================================================================
// Page registration
// ============================================================================

void PageManager::AddPage(const std::string& name, std::unique_ptr<Scene> page) {
    m_pages.push_back({name, std::move(page)});
}

void PageManager::SwitchTo(int index) {
    if (index < 0 || index >= (int)m_pages.size()) return;
    if (index == m_currentIndex) return;

    if (m_pages[m_currentIndex].scene)
        m_pages[m_currentIndex].scene->OnExit();

    m_currentIndex = index;

    if (m_pages[m_currentIndex].scene)
        m_pages[m_currentIndex].scene->OnEnter();

    printf("[PageManager] Switched to page %d: %s\n",
           m_currentIndex, m_pages[m_currentIndex].name.c_str());
}

// ============================================================================
// Scene lifecycle — delegate to active sub-page
// ============================================================================

void PageManager::OnEnter() {
    m_currentIndex = 0;
    if (!m_pages.empty() && m_pages[0].scene)
        m_pages[0].scene->OnEnter();
}

void PageManager::OnExit() {
    for (auto& p : m_pages) {
        if (p.scene) p.scene->OnExit();
    }
}

void PageManager::OnUpdate(float dt) {
    if (m_currentIndex >= 0 && m_currentIndex < (int)m_pages.size()) {
        auto* s = m_pages[m_currentIndex].scene.get();
        if (s) s->OnUpdate(dt);
    }
}

void PageManager::OnRender(IRenderBackend* backend) {
    m_backend = backend;
    if (m_currentIndex >= 0 && m_currentIndex < (int)m_pages.size()) {
        auto* s = m_pages[m_currentIndex].scene.get();
        if (s) s->OnRender(backend);
    }
}

void PageManager::OnImGui() {
    if (m_pages.empty()) return;

    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) return;

    const float navH = h * 0.065f;

    // ---- Delegate to active sub-page (uses full screen, nav bar sits on top) ----
    if (m_currentIndex >= 0 && m_currentIndex < (int)m_pages.size()) {
        auto* s = m_pages[m_currentIndex].scene.get();
        if (s) s->OnImGui();
    }

    // ---- Bottom navigation bar (ImGui window, renders after sub-page) ----
    DrawNavBar(w, h);
}

// ============================================================================
// GetNextScene — intercept sub-page scene transitions
// ============================================================================

std::unique_ptr<Scene> PageManager::GetNextScene() {
    if (m_currentIndex < 0 || m_currentIndex >= (int)m_pages.size())
        return nullptr;

    auto& page = m_pages[m_currentIndex];
    if (!page.scene) return nullptr;

    auto next = page.scene->GetNextScene();
    if (!next) return nullptr;

    // Sub-page wants to transition (CoverFlow ↔ EffectDetail).
    // Swap in-place — Application always sees PageManager as root.
    printf("[PageManager] Sub-page transition in page %d (%s)\n",
           m_currentIndex, page.name.c_str());

    page.scene->OnExit();
    page.scene = std::move(next);
    page.scene->OnEnter();

    return nullptr;
}

// ============================================================================
// Bottom Navigation Bar
// ============================================================================

void PageManager::DrawNavBar(float w, float h) {
    const float navH = h * 0.065f;

    ImGui::SetNextWindowPos(ImVec2(0, h - navH));
    ImGui::SetNextWindowSize(ImVec2(w, navH));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.08f, 0.11f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Border,  ImVec4(0.24f, 0.25f, 0.31f, 0.7f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("##PageNav", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    int n = (int)m_pages.size();

    if (n > 0) {
        const float btnW = ImGui::CalcTextSize("后处理").x + 40.0f;
        const float gap  = 12.0f;
        float totalW = n * btnW + (n - 1) * gap;
        float startX = (w - totalW) * 0.5f;
        float btnH   = navH * 0.6f;
        float btnY   = (navH - btnH) * 0.5f;

        for (int i = 0; i < n; i++) {
            float bx = startX + i * (btnW + gap);
            bool active = (i == m_currentIndex);

            ImU32 bg = active ? IM_COL32(72, 78, 120, 255)
                              : IM_COL32(35, 38, 55, 180);
            dl->AddRectFilled(ImVec2(bx, btnY), ImVec2(bx + btnW, btnY + btnH),
                              bg, 6.0f);

            if (active) {
                dl->AddRect(ImVec2(bx, btnY), ImVec2(bx + btnW, btnY + btnH),
                            IM_COL32(140, 150, 220, 200), 6.0f, 0, 1.5f);
            }

            const char* label = m_pages[i].name.c_str();
            ImVec2 ts = ImGui::CalcTextSize(label);
            ImU32 tc  = active ? IM_COL32(255, 255, 255, 255)
                               : IM_COL32(160, 160, 180, 200);
            dl->AddText(ImGui::GetFont(), 15.0f,
                        ImVec2(bx + (btnW - ts.x) * 0.5f,
                               btnY + (btnH - ts.y) * 0.5f),
                        tc, label);

            // Click
            ImGui::SetCursorScreenPos(ImVec2(bx, btnY));
            ImGui::InvisibleButton(("##nav_" + std::to_string(i)).c_str(),
                                   ImVec2(btnW, btnH));
            if (ImGui::IsItemClicked()) {
                SwitchTo(i);
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}
