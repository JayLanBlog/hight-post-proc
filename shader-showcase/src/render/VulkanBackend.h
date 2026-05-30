#pragma once
#include "render/IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend() = default;
    ~VulkanBackend() override;

    // lifecycle
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // viewport
    void Resize(int w, int h) override;
    void GetFramebufferSize(int& w, int& h) override;

    // shader (SPIR-V)
    ShaderHandle CreateVertexShader(const std::vector<uint32_t>& spirv) override;
    ShaderHandle CreateFragmentShader(const std::vector<uint32_t>& spirv) override;
    void DestroyShader(ShaderHandle handle) override;

    // texture
    TextureHandle CreateTexture(int w, int h, TextureFormat fmt, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void UpdateTexture(TextureHandle handle, int w, int h, const void* data) override;

    // fullscreen draw
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMatrix, const float* projMatrix) override;
    void BlitToScreen(TextureHandle texture) override;
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // imgui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // query
    BackendType GetType() const override { return BackendType::Vulkan; }
    const char* GetName() const override { return "Vulkan 1.2 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 1280, m_height = 720;
    bool m_initialized = false;

    // Core
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsFamily = 0;
    uint32_t m_presentFamily = 0;

    // Swapchain
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    uint32_t m_currentImageIndex = 0;

    // Command
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    // Sync
    VkSemaphore m_imAvail = VK_NULL_HANDLE;
    VkSemaphore m_renderDone = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;

    // Resources
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    std::unordered_map<uint32_t, VkShaderModule> m_shaderModules;
    std::unordered_map<uint32_t, VkImageView> m_textureViews;
    std::unordered_map<uint32_t, VkImage> m_textureImages;
    std::unordered_map<uint32_t, VkDeviceMemory> m_textureMemories;

    // Init helpers
    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CleanupSwapchain();
    void RecreateSwapchain();
    void RecordCommandBuffer();

    // Utility
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer cmd);
    void TransitionImageLayout(VkImage img, VkFormat fmt, VkImageLayout oldL, VkImageLayout newL);
    void CopyBufferToImage(VkBuffer buf, VkImage img, uint32_t w, uint32_t h);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buf, VkDeviceMemory& mem);
};
