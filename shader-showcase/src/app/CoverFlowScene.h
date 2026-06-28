#pragma once

enum class EffectCategory { Dynamic, Static };

#include "app/Scene.h"
#include "app/CoverFlowState.h"
#include "shader/EffectMetadata.h"
#include "render/IRenderBackend.h"

#include <vector>
#include <memory>
#include <string>
#include <chrono>

class Application;
class ScreenCapture;
class VideoPlayer;

/// Per-card thumbnail real-time render state
struct CardThumbnailState {
    ShaderHandle fragShader;   // effect shader per card
    TextureHandle thumbTex;    // 256x144 thumbnail FBO texture
};

class CoverFlowScene : public Scene {
public:
    CoverFlowScene();
    ~CoverFlowScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(IRenderBackend* backend) override;
    void OnImGui() override;
    bool WantsExit() const override { return m_wantsExit; }
    bool WantsReturn() const override { return m_wantsReturn; }

    std::unique_ptr<Scene> GetNextScene() override;
    void SetInputTexture(TextureHandle tex) { m_inputTex = tex; }
    void SetInputTexCache(const std::vector<TextureHandle>& cache) { m_inputTexCache = cache; }
    void SetBackend(IRenderBackend* backend) { m_backend = backend; }
    void SetApplication(Application* app) { m_app = app; }

    /// Set pre-rendered thumbnail ImTextureIDs (one per card, same order as RegisterCards).
    void SetThumbnails(const std::vector<void*>& imTexIds) { m_thumbIds = imTexIds; }

    /// Restore selected card index and scroll offset (used when returning from detail scene).
    void SetSelectedIndex(int index) { m_selectedIndex = index; m_targetOffset = 0.0f; m_scrollOffset = 0.0f; }

    /// Set test image base directory (for thumbnail input textures)
    void SetTestImageBaseDir(const std::string& dir) { m_testImageBaseDir = dir; }

    /// Transfer video player back from detail scene.
    void SetVideoPlayer(std::unique_ptr<VideoPlayer> player, TextureHandle videoTex, bool active, double lastFrameTime);

    /// Enable auto-test mode: cycle through all cards, holdFrames per card.
    void EnableAutoTest(int holdFrames);

    /// Resume auto-test after returning from detail scene.
    void ResumeAutoTest(int holdFrames, int lastOpenedCard);

    /// Reload input texture from a new file path (drag-drop support).
    void ReloadInputTexture(const std::string& filePath);

    /// Add an image path to the built-in image pool (for Ctrl+Left/Right cycling).
    void AddImageToPool(const std::string& path);

    /// Add a video path to the built-in video pool.
    void AddVideoToPool(const std::string& path);

    /// Get current state for saving (used by EffectDetailScene to restore).
    CoverFlowState GetState() const;

private:
    std::vector<EffectCard> m_cards;
    std::string m_activeCategory = "Process Effects";
    std::vector<int> m_filteredIndices;
    std::vector<int> m_dynamicIndices;  // cards that are dynamic (time-varying)
    std::vector<int> m_staticIndices;   // cards that are static (no time)
    EffectCategory m_currentCategory = EffectCategory::Static;
    int m_selectedIndex   = 0;
    float m_scrollOffset  = 0.0f;

    // Full-screen immersive rendering
    TextureHandle m_immersiveTex    = INVALID_TEXTURE;
    int m_immersiveTexW            = 0;
    int m_immersiveTexH            = 0;
    void* m_immersiveImTexID       = nullptr;
    float m_effectTime             = 0.0f;
    uint32_t m_effectFrameCount    = 0;

    float m_targetOffset  = 0.0f;
    TextureHandle m_inputTex = {0};
    std::vector<TextureHandle> m_inputTexCache;  // per-effect cached input textures
    IRenderBackend* m_backend = nullptr;
    Application* m_app = nullptr;
    bool m_wantsExit = false;
    bool m_wantsReturn = false;
    std::unique_ptr<Scene> m_nextScene;

    // Auto-test mode
    bool m_autoTest = false;
    int  m_autoTestHoldFrames = 0;
    int  m_autoTestFrameCounter = 0;
    int  m_autoTestCardIndex = 0;
    int  m_autoTestLastOpenedCard = -1;

    // Thumbnails
    std::vector<void*> m_thumbIds;  // ImTextureID for each card

    // ---- Dynamic thumbnail rendering ----
    ShaderHandle m_sharedVertShader = INVALID_SHADER;
    std::vector<CardThumbnailState> m_thumbnailStates;
    int m_thumbWidth  = 256;
    int m_thumbHeight = 144;
    float m_lastWindowW = 0;   // track window resize for thumbnail rebuild
    float m_lastWindowH = 0;
    bool m_thumbInitialized = false;
    std::string m_testImageBaseDir;
    float m_thumbElapsedTime = 0.0f;
    uint32_t m_thumbFrameCount = 0;

    void InitializeThumbnails();
    void RenderVisibleThumbnails();

    // Mouse drag state
    bool  m_dragging    = false;
    float m_dragStartX  = 0.0f;
    float m_dragBaseOff = 0.0f;

    // FPS counter
    std::chrono::high_resolution_clock::time_point m_fpsLastTime;
    int    m_fpsFrameCount = 0;
    float  m_fpsDisplay    = 0.0f;

    // Screen capture
    std::unique_ptr<ScreenCapture> m_screenCapture;
    TextureHandle m_captureTex = {0};
    bool m_captureActive  = false;
    bool m_captureReady   = false;
    int  m_captureWidth   = 0;
    int  m_captureHeight  = 0;

    // Image cycling
    std::vector<std::string> m_imagePool;      // paths to built-in images
    int m_currentImageIndex = 0;

    // Video cycling
    std::vector<std::string> m_videoPool;      // paths to built-in videos
    int m_currentVideoIndex = 0;

    // Video player
    std::unique_ptr<VideoPlayer> m_videoPlayer;
    TextureHandle m_videoTex = {0};
    bool m_videoActive = false;
    double m_videoLastFrameTime = 0.0;

    void RegisterCards();
    void SelectCard(int index);
    void OpenSelectedEffect();
    void UpdateFPSCounter();
    void ToggleScreenCapture();
    void CycleImage(int direction);  // -1=prev, +1=next
    void LoadImageFromFile(const std::string& path);
    void OpenVideoFile(const std::string& path);
    void StopVideo();
};
