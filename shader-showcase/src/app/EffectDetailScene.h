#pragma once

#include "app/Scene.h"
#include "app/CoverFlowState.h"
#include "shader/EffectMetadata.h"
#include "render/IRenderBackend.h"
#include "ui/DebugPanel.h"

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class Application;
class ScreenCapture;
class VideoPlayer;

class EffectDetailScene : public Scene {
public:
    EffectDetailScene(const EffectCard& card, TextureHandle inputTex);

    void SetBackend(IRenderBackend* backend) { m_backend = backend; }
    void SetApplication(Application* app) { m_app = app; }

    /// Save CoverFlowScene state so we can restore it on return.
    void SetCoverFlowState(const CoverFlowState& state) {
        m_savedState = state;
        // Auto-test: if coming from auto-test mode, set up auto-return timer
        if (state.autoTest) {
            m_autoTestHoldFrames = state.autoTestHoldFrames;
        }
    }

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsExit() const override { return m_wantsExit; }
    std::unique_ptr<Scene> GetNextScene() override;

private:
    EffectCard      m_card;
    TextureHandle   m_inputTex;
    IRenderBackend* m_backend = nullptr;
    Application*    m_app     = nullptr;

    ShaderHandle m_vertShader = INVALID_SHADER;
    ShaderHandle m_fragShader = INVALID_SHADER;

    float    m_time       = 0.0f;
    uint32_t m_frameCount = 0;
    int      m_viewportWidth  = 0;
    int      m_viewportHeight = 0;

    bool      m_showDebug = true;
    DebugPanel m_debugPanel;

    std::vector<float>   m_uniformFloats;
    std::vector<int32_t> m_uniformInts;
    size_t               m_expectedFloatCount = 0;  // actual shader param count from effect.json

    bool m_wantsExit         = false;
    bool m_returnToCoverFlow = false;

    // Saved CoverFlow state for restoration
    CoverFlowState m_savedState;

    // Auto-test
    int m_autoTestHoldFrames = 0; // >0 means auto-exit after this many frames

    // Compare mode (slider before/after)
    bool m_compareMode = false;       // before/after comparison (toggle with C key)
    float m_compareSplitPos = 0.5f;   // split position (0=left all original, 1=right all effect)
    bool m_compareDragging = false;  // is user dragging the split handle?
    TextureHandle m_effectTex = {0};  // FBO texture for effect output
    bool m_effectTexCreated = false;
    
    // Screenshot counter (per-scene instance)
    int m_screenshotCaptured = 0;

    // Video playback
    std::unique_ptr<VideoPlayer> m_videoPlayer;
    TextureHandle m_videoTex = {0};
    bool m_videoActive = false;
    double m_videoLastFrameTime = 0.0;

    void LoadImageFromFile(const std::string& path);
    void LoadVideoFromFile(const std::string& path);
    void StopVideo();
    void EnsureEffectTexture();

    // Compare view rendering
    void RenderCompareView(IRenderBackend* backend);
    void RenderFullscreenEffect(IRenderBackend* backend);
};
