#pragma once

#include "app/Scene.h"
#include <string>
#include <vector>

class IRenderBackend;
class Application;

class SceneGalleryScene : public Scene {
public:
    void SetApplication(Application* app) { m_app = app; }

    void OnEnter() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    std::unique_ptr<Scene> GetNextScene() override;

private:
    void DrawSidebar(float w, float h);
    void DrawCardList(float w, float h);

    std::string m_activeCategory = "全部";
    std::string m_pendingEnter;
    int         m_hoverCard  = -1;
    Application* m_app        = nullptr;
    float m_sidebarW = 220.0f;
};
