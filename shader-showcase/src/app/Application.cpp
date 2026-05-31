#include "app/Application.h"
#include "app/Scene.h"
#include "app/LanguageManager.h"
#include "ui/PerformancePanel.h"
#include <exception>

// GL 4.6 types must be available before GLFW and backend headers
#include "render/gl_core_46.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>

// ScreenshotRequest static members
bool ScreenshotRequest::pending = false;
char ScreenshotRequest::path[256] = {};

#ifdef USE_OPENGL_BACKEND
#include "render/OpenGLBackend.h"
#endif
#ifdef USE_VULKAN_BACKEND
#include "render/VulkanBackend.h"
#endif

// ============================================================================
// Constructor / Destructor
// ============================================================================

Application::Application() {}

Application::~Application()
{
    Shutdown();
}

// ============================================================================
// Run 鈥?application entry point
// ============================================================================

int Application::Run(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!glfwInit())
    {
        std::cerr << "[Application] FATAL: Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    InitBackend(m_backendType);

    if (!m_window || !m_backend)
    {
        std::cerr << "[Application] FATAL: Failed to initialize backend" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    m_running = true;
    MainLoop();
    Shutdown();
    glfwTerminate();

    return EXIT_SUCCESS;
}

// ============================================================================
// InitBackend 鈥?destroy old backend/window, create new
// ============================================================================

void Application::InitBackend(BackendType type)
{
    // ---- Destroy old resources ---------------------------------------------
    // Reset current scene before destroying backend
    if (m_currentScene)
    {
        m_currentScene->OnExit();
        m_currentScene.reset();
    }

    if (m_backend)
    {
        m_backend->ImGuiShutdown();
        m_backend->Shutdown();
        m_backend.reset();
    }
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    m_backendType = type;

    // ---- Set window hints based on backend type ----------------------------
    glfwDefaultWindowHints();

    switch (type)
    {
    case BackendType::OpenGL:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        break;

    case BackendType::Vulkan:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    }

    // ---- Create window -----------------------------------------------------
    constexpr int WINDOW_WIDTH  = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, LanguageManager::Instance().WindowTitle(), nullptr, nullptr);

    if (!m_window)
    {
        std::cerr << "[Application] ERROR: Failed to create window for "
                  << (type == BackendType::OpenGL ? "OpenGL" : "Vulkan") << std::endl;
        return;
    }

    // ---- Set user pointer & callbacks --------------------------------------
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetDropCallback(m_window, DropCallback);

    // ---- OpenGL: make context current --------------------------------------
    if (type == BackendType::OpenGL)
    {
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // VSync
    }

    // ---- Create backend ----------------------------------------------------
    switch (type)
    {
    case BackendType::OpenGL:
#ifdef USE_OPENGL_BACKEND
        m_backend = std::make_unique<OpenGLBackend>();
#endif
        break;

    case BackendType::Vulkan:
#ifdef USE_VULKAN_BACKEND
        m_backend = std::make_unique<VulkanBackend>();
#endif
        break;
    }

    if (!m_backend)
    {
        std::cerr << "[Application] ERROR: Backend not available (compile-time disabled)" << std::endl;
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        return;
    }

    // ---- Initialize backend ------------------------------------------------
    if (!m_backend->Init(m_window))
    {
        std::cerr << "[Application] ERROR: Backend initialization failed" << std::endl;
        m_backend.reset();
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        return;
    }

    // ---- Initialize ImGui --------------------------------------------------
    m_backend->ImGuiInit(m_window);

    // ---- Set window title --------------------------------------------------
    std::string title = std::string(LanguageManager::Instance().WindowTitle()) + " [" + m_backend->GetName() + "]";
    glfwSetWindowTitle(m_window, title.c_str());

    std::cout << "[Application] Backend initialized: " << m_backend->GetName() << std::endl;
}

// ============================================================================
// SetScene 鈥?set the current scene
// ============================================================================

void Application::SetScene(std::unique_ptr<Scene> scene)
{
    if (m_currentScene)
    {
        printf("[Application] SetScene: exiting old scene\n");
        fflush(stdout);
        m_currentScene->OnExit();
        m_currentScene.reset();
    }
    m_currentScene = std::move(scene);
    if (m_currentScene)
    {
        printf("[Application] SetScene: entering new scene\n");
        fflush(stdout);
        m_currentScene->OnEnter();
        printf("[Application] SetScene: new scene entered OK\n");
        fflush(stdout);
    }
}

// ============================================================================
// SwitchBackend 鈥?switch to a different backend type
// ============================================================================

void Application::SwitchBackend(BackendType type)
{
    if (type == m_backendType && m_backend)
    {
        return; // Already on the requested backend
    }
    // Defer the actual switch to the start of the next frame
    // to avoid ImGui/Vulkan state corruption during rendering
    m_pendingBackend = type;
    m_pendingBackendSwitch = true;
    printf("[Application] Backend switch to %s scheduled for next frame\n",
           type == BackendType::OpenGL ? "OpenGL" : "Vulkan");
}

// ============================================================================
// MainLoop 鈥?frame-driven game loop
// ============================================================================

void Application::MainLoop()
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && m_window && !glfwWindowShouldClose(m_window))
    {
        try {
        glfwPollEvents();

        // Process pending backend switch (deferred to avoid corruption during ImGui rendering)
        if (m_pendingBackendSwitch) {
            m_pendingBackendSwitch = false;
            printf("[Application] Processing deferred backend switch...\n");
            InitBackend(m_pendingBackend);
        }

        // Compute delta time
        auto now      = std::chrono::high_resolution_clock::now();
        float dt      = std::chrono::duration<float>(now - lastTime).count();
        lastTime      = now;

        if (!m_backend)
        {
            continue;
        }

        m_backend->BeginFrame();

        // --- Deferred scene transition ---
        // Must happen AFTER BeginFrame's vkWaitForFences so GPU has finished
        // with old scene's resources before they're destroyed.
        if (m_pendingNextScene) {
            printf("[Application] Processing deferred scene transition\n");
            if (m_currentScene) {
                m_currentScene->OnExit();
                m_currentScene.reset();
            }
            m_currentScene = std::move(m_pendingNextScene);
            if (m_currentScene) {
                m_currentScene->OnEnter();
                printf("[Application] New scene entered OK\n");
            }
        }

        m_backend->ImGuiNewFrame();

        // --- Scene-driven update ---
        if (m_currentScene)
        {
            m_currentScene->OnUpdate(dt);
            m_currentScene->OnRender(m_backend.get());

            m_currentScene->OnImGui();

            // Check for scene transition - DEFER to next frame's BeginFrame
            // This ensures GPU has finished with old scene's resources before they're destroyed
            if (m_currentScene->WantsExit())
            {
                auto nextScene = m_currentScene->GetNextScene();
                if (nextScene)
                {
                    printf("[Application] Deferring scene transition to next frame\n");
                    m_pendingNextScene = std::move(nextScene);
                }
                else
                {
                    // No next scene — exit application
                    printf("[Application] Scene requested exit with no replacement; shutting down\n");
                    m_running = false;
                }
            }
        }
        else if (m_frameCallback)
        {
            // Fallback: legacy frame callback
            m_frameCallback(dt);
        }

        // Always render performance panel (visible even without a scene)
        m_perfPanel.Render(this, m_backend.get());

        m_backend->ImGuiRender();
        
        // Check for screenshot request (from auto-test)
        if (ScreenshotRequest::Consume()) {
            printf("[Application] Processing screenshot request: %s\n", ScreenshotRequest::path);
            if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                gl->SaveScreenshot(ScreenshotRequest::path);
                printf("[Application] Screenshot saved: %s\n", ScreenshotRequest::path);
            } else {
                printf("[Application] Failed to get OpenGL backend for screenshot\n");
            }
        }
        
        // Auto-screenshot for UI demo
        {
            static int screenshotFrame = -1;
            if (screenshotFrame == -1 && getenv("AUTO_TEST_UI")) {
                screenshotFrame = 0;
            }
            if (screenshotFrame >= 0 && screenshotFrame < 5) {
                screenshotFrame++;
                if (screenshotFrame == 3) {
                    if (auto* gl = dynamic_cast<OpenGLBackend*>(m_backend.get())) {
                        gl->SaveScreenshot("e:/AI/graph/hight-post-proc/ui_screenshot.ppm");
                        printf("[Application] UI screenshot saved at frame %d\n", screenshotFrame);
                    }
                }
            }
        }
        
        m_backend->EndFrame();
        } catch (const std::exception& e) {
            fprintf(stderr, "[Application] EXCEPTION in main loop: %s\n", e.what());
            m_running = false;
        } catch (...) {
            fprintf(stderr, "[Application] UNKNOWN EXCEPTION in main loop\n");
            m_running = false;
        }
    }

    printf("[Application] Main loop exited: m_running=%d, m_window=%p, shouldClose=%d\n",
           (int)m_running, (void*)m_window,
           m_window ? glfwWindowShouldClose(m_window) : -1);

    m_running = false;
}

// ============================================================================
// Shutdown 鈥?release all resources
// ============================================================================

void Application::Shutdown()
{
    m_running = false;

    // Exit and destroy current scene
    if (m_currentScene)
    {
        m_currentScene->OnExit();
        m_currentScene.reset();
    }

    if (m_backend)
    {
        m_backend->ImGuiShutdown();
        m_backend->Shutdown();
        m_backend.reset();
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

// ============================================================================
// Static callbacks
// ============================================================================

void Application::FramebufferSizeCallback(GLFWwindow* w, int width, int height)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (app && app->m_backend)
    {
        app->m_backend->Resize(width, height);
    }
}

void Application::KeyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS)
    {
        return;
    }

    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (!app)
    {
        return;
    }

    // ESC is now handled by the scene system (CoverFlowScene exits app,
    // EffectDetailScene returns to CoverFlow). The KeyCallback ignores ESC.
    // Global override: Ctrl+Q always exits
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(w, GLFW_TRUE);
        return;
    }

    // Ctrl+1 鈥?switch to OpenGL
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_1)
    {
        app->SwitchBackend(BackendType::OpenGL);
        return;
    }

    // Ctrl+2 — switch to Vulkan
    if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_2)
    {
        app->SwitchBackend(BackendType::Vulkan);
        return;
    }
}

void Application::DropCallback(GLFWwindow* w, int count, const char** paths)
{
    if (count < 1) return;
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    // Accept only the first dropped file
    app->m_droppedFilePath = paths[0];
    printf("[Application] File dropped: %s\n", paths[0]);
}

std::string Application::ConsumeDroppedFile()
{
    std::string path;
    std::swap(path, m_droppedFilePath);
    return path;
}
