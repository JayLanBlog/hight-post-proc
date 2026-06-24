#pragma once

#include "app/Scene.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <unordered_map>

class Application;

struct CardAnimState {
    float hoverT      = 0.0f;   // hover transition 0→1 (ease 200ms)
    float entryT      = -1.0f;  // entry anim: -1=not started, 0→1=active
    float clickScale  = 1.0f;   // click press scale (1.0→0.97 spring)
    float clickVel    = 0.0f;   // click spring velocity
};

class SceneGalleryScene : public Scene {
public:
    void SetApplication(Application* app) { m_app = app; }

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    std::unique_ptr<Scene> GetNextScene() override;

private:
    void LoadThumbnails();
    void ReleaseThumbnails();

    void DrawBackground(float w, float h);
    void DrawTopBar(float w, float h);
    void DrawHero(float w, float h);
    void DrawCategoryTabs(float w, float h);
    void DrawCardGrid(float w, float h);

    // Color constants
    static constexpr ImU32 kBgDeep      = IM_COL32(10, 11, 16, 255);
    static constexpr ImU32 kBgSurface   = IM_COL32(20, 21, 32, 255);
    static constexpr ImU32 kBgElevated  = IM_COL32(26, 28, 48, 255);
    static constexpr ImU32 kAccent      = IM_COL32(74,  91, 207, 255);
    static constexpr ImU32 kAccentHover = IM_COL32(107,124,232, 255);
    static constexpr ImU32 kTextPrimary   = IM_COL32(240,245,255, 255);
    static constexpr ImU32 kTextSecondary = IM_COL32(160,170,200, 220);
    static constexpr ImU32 kTextTertiary  = IM_COL32(100,110,130, 180);

    std::string m_activeCategory = "全部";
    std::string m_pendingEnter;
    int         m_hoverCard    = -1;
    int         m_heroIndex    = 0;       // which scene is featured in Hero
    Application* m_app         = nullptr;
    IRenderBackend* m_backend  = nullptr;

    std::vector<CardAnimState> m_cardAnims;
    std::vector<TextureHandle> m_thumbTextures;
    bool m_thumbsLoaded = false;
};
