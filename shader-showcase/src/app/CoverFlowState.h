#pragma once

#include "render/IRenderBackend.h"

#include <vector>
#include <string>

class Application;

/// Saved state from CoverFlowScene, used to restore when returning from detail view.
struct CoverFlowState {
    std::vector<void*>       thumbIds;
    std::vector<TextureHandle> inputTexCache;  // per-effect cached input textures
    std::vector<std::string> imagePool;
    std::vector<std::string> videoPool;
    int                      currentImageIndex = 0;
    int                      selectedIndex    = 0;
    Application*             app              = nullptr;
    TextureHandle            inputTex         = {0};
    IRenderBackend*          backend          = nullptr;
    // Screen capture state
    bool                     captureActive    = false;
    // Auto-test state
    bool                     autoTest         = false;
    int                      autoTestHoldFrames = 0;
    int                      autoTestCardIndex  = 0;
    std::string              testImageBaseDir;
};
