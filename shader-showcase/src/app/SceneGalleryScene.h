#pragma once

#include "app/Scene.h"
#include "render/IRenderBackend.h"
#include "imgui.h"
#include <string>
#include <vector>

class Application;

struct CardAnim {
    float hoverT     = 0.0f;   // lerp to hover state 0→1
    float entryT     = -1.0f;  // entry anim -1=pending, 0→1=active
    float clickScale = 1.0f;   // spring scale
    float clickVel   = 0.0f;
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
    void LoadThumbs();
    void FreeThumbs();

    // Colors
    static constexpr ImU32 kDeep      = IM_COL32(10, 11, 16, 255);
    static constexpr ImU32 kCardBg    = IM_COL32(22, 25, 38, 200);
    static constexpr ImU32 kHeroBg    = IM_COL32(26, 28, 48, 230);
    static constexpr ImU32 kAccent    = IM_COL32(74,  91, 207, 255);
    static constexpr ImU32 kAccentDim = IM_COL32(74,  91, 207, 35);
    static constexpr ImU32 kText1     = IM_COL32(240,245,255, 255);
    static constexpr ImU32 kText2     = IM_COL32(160,170,200, 220);
    static constexpr ImU32 kText3     = IM_COL32(100,110,130, 180);

    std::string m_activeCategory = "全部";
    std::string m_pendingEnter;
    int m_hoverCard = -1;
    int m_heroIdx   = 0;

    Application*    m_app;
    IRenderBackend* m_be = nullptr;

    std::vector<CardAnim> m_anim;
    std::vector<TextureHandle> m_thumbs;
    bool m_thumbsOk = false;
};
