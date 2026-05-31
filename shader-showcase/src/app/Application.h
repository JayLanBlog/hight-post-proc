#pragma once
#include "render/IRenderBackend.h"
#include <memory>
#include <functional>
#include <string>

struct GLFWwindow;

class Scene;

// Global screenshot request (for auto-test)
struct ScreenshotRequest {
    static bool pending;
    static char path[256];
    static void Request(const char* p) { pending = true; snprintf(path, sizeof(path), "%s", p); }
    static bool Consume() { if (pending) { pending = false; return true; } return false; }
};

#include "ui/PerformancePanel.h"

class Application {
public:
    Application();
    ~Application();
    int Run(int argc, char* argv[]);
    void SwitchBackend(BackendType type);
    BackendType GetBackendType() const { return m_backendType; }
    IRenderBackend* GetBackend() const { return m_backend.get(); }
    GLFWwindow* GetWindow() const { return m_window; }

    // Scene management
    void SetScene(std::unique_ptr<Scene> scene);
    Scene* GetCurrentScene() const { return m_currentScene.get(); }

    // Frame callback (deprecated in favor of scene system;
    // kept for backward compatibility)
    using FrameCallback = std::function<void(float dt)>;
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

    // Drag-drop file loading
    /// Returns the path of the most recently dropped file, or empty string.
    /// The caller should clear the pending flag after consuming.
    std::string ConsumeDroppedFile();

private:
    void InitBackend(BackendType type);
    void MainLoop();
    void Shutdown();

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<IRenderBackend> m_backend;
    BackendType m_backendType = BackendType::Vulkan;
    BackendType m_pendingBackend = BackendType::Vulkan;
    bool m_pendingBackendSwitch = false;
    FrameCallback m_frameCallback;
    std::unique_ptr<Scene> m_currentScene;
    std::unique_ptr<Scene> m_pendingNextScene;  // Deferred scene transition
    bool m_running = false;

    // Drag-drop state
    std::string m_droppedFilePath;

    // UI panels
    PerformancePanel m_perfPanel;

    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height);
    static void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void DropCallback(GLFWwindow* w, int count, const char** paths);
};
