#include "PerformancePanel.h"
#include "app/Application.h"
#include "render/IRenderBackend.h"
#include <imgui.h>

void PerformancePanel::Render(Application* app, IRenderBackend* backend) {
    UpdateFPS();

    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float panelWidth = 100.0f;
    float panelHeight = 70.0f;
    ImVec2 pos(windowSize.x - panelWidth - 8.0f, 8.0f);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

    if (ImGui::Begin("##PerformancePanel", nullptr, flags)) {
        RenderBackendButtons(app);
        RenderFPSDisplay(app);
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void PerformancePanel::UpdateFPS() {
    float dt = ImGui::GetIO().DeltaTime;
    m_fpsElapsed += dt;
    m_fpsFrameCount++;

    if (m_fpsElapsed >= 1.0f) {
        m_lastFps = m_fpsFrameCount / m_fpsElapsed;
        m_fpsFrameCount = 0;
        m_fpsElapsed = 0.0f;
    }
}

void PerformancePanel::RenderBackendButtons(Application* app) {
    BackendType current = app->GetBackendType();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

    // OpenGL button
    bool isOpenGL = (current == BackendType::OpenGL);
    if (isOpenGL) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    }
    if (ImGui::Button("GL", ImVec2(32, 20))) {
        if (!isOpenGL) app->SwitchBackend(BackendType::OpenGL);
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Vulkan button
    bool isVulkan = (current == BackendType::Vulkan);
#ifdef USE_VULKAN_BACKEND
    if (isVulkan) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.27f, 0.27f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
    }
    if (ImGui::Button("VK", ImVec2(32, 20))) {
        if (!isVulkan) app->SwitchBackend(BackendType::Vulkan);
    }
    ImGui::PopStyleColor();
#else
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
    ImGui::Button("VK", ImVec2(32, 20));
    ImGui::PopStyleColor(2);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Vulkan not available");
    }
#endif

    ImGui::PopStyleVar();
}

void PerformancePanel::RenderFPSDisplay(Application* app) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.33f, 1.0f, 0.33f, 1.0f));
    ImGui::Text("%.1f FPS", m_lastFps);
    ImGui::PopStyleColor();

    const char* backendLabel = (app->GetBackendType() == BackendType::Vulkan) ? "Vulkan 1.2" : "OpenGL 4.6";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 0.53f, 0.53f, 1.0f));
    ImGui::TextUnformatted(backendLabel);
    ImGui::PopStyleColor();
}
