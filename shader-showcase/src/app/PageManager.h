#pragma once

#include "app/Scene.h"
#include <vector>
#include <memory>
#include <string>

class IRenderBackend;

/// PageManager — multi-page navigation container.
///
/// Acts as the root Scene in Application.  Each page is an independent
/// Scene subclass (CoverFlowScene, etc.).  The manager draws a bottom
/// navigation bar and delegates lifecycle calls to the active page.
///
/// Scene transitions WITHIN a page (CoverFlow ↔ EffectDetail) are
/// intercepted by PageManager::GetNextScene() so that Application
/// never sees the swap — PageManager swaps the sub-page in place.
class PageManager : public Scene {
public:
    PageManager() = default;

    /// Register a new page.  PageManager takes ownership of `scene`.
    void AddPage(const std::string& name, std::unique_ptr<Scene> page);

    /// Switch to page at `index`.  Calls OnExit() on old page,
    /// OnEnter() on new page.
    void SwitchTo(int index);
    int  CurrentIndex() const { return m_currentIndex; }
    int  PageCount()    const { return (int)m_pages.size(); }

    // ---- Scene interface ----
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    std::unique_ptr<Scene> GetNextScene() override;

private:
    struct Page {
        std::string            name;
        std::unique_ptr<Scene> scene;
    };

    void DrawNavBar(float w, float h);

    std::vector<Page> m_pages;
    int               m_currentIndex = 0;
    IRenderBackend*   m_backend      = nullptr;
};
