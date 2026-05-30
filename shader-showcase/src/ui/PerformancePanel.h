#pragma once
#include "render/BackendType.h"
#include <imgui.h>

class Application;
class IRenderBackend;

class PerformancePanel {
public:
    void Render(Application* app, IRenderBackend* backend);

private:
    float m_lastFps = 0.0f;
    int m_fpsFrameCount = 0;
    float m_fpsElapsed = 0.0f;

    void UpdateFPS();
    void RenderBackendButtons(Application* app);
    void RenderFPSDisplay();
};
