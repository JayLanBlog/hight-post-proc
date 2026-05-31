#pragma once
#include "render/IRenderBackend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

struct GLFWwindow;

// ============================================================================
// Vulkan Resource Wrappers
// ============================================================================
struct VulkanShader {
    VkShaderModule module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
};

struct VulkanTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    int width = 0, height = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    bool isFBO = false;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    // ImGui descriptor set (cached for GetImTextureID)
    VkDescriptorSet imguiDescriptorSet = VK_NULL_HANDLE;
};

struct VulkanPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;   // pre-allocated: binds texture(0)+UBO(1)
    ShaderHandle vertShader;
    ShaderHandle fragShader;
    // UBO for shader Params block (binding=1), reused every frame
    VkBuffer uboBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uboMemory = VK_NULL_HANDLE;
};

// ============================================================================
// Vulkan Backend
// ============================================================================
class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;

    // Lifecycle
    bool Init(GLFWwindow* window) override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // Viewport
    void Resize(int width, int height) override;
    void GetFramebufferSize(int& width, int& height) override;

    // Shaders (SPIR-V)
    ShaderHandle CreateVertexShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateFragmentShader(const uint32_t* spirv, size_t size) override;
    ShaderHandle CreateVertexShaderFromGLSL(const std::string& source) override;
    ShaderHandle CreateFragmentShaderFromGLSL(const std::string& source) override;
    void DestroyShader(ShaderHandle handle) override;

    // Textures
    TextureHandle CreateTexture(int width, int height, TextureFormat format, const void* data) override;
    void UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) override;
    void DestroyTexture(TextureHandle handle) override;
    void* GetImTextureID(TextureHandle handle) override;

    // Pipelines
    PipelineHandle CreatePipeline(const PipelineDesc& desc) override;
    void DestroyPipeline(PipelineHandle handle) override;
    void BindPipeline(PipelineHandle handle) override;

    // Fullscreen draw
    void DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) override;
    void DrawCards(const std::vector<IRenderBackend::CardDrawInfo>& cards, const float* viewMatrix, const float* projMatrix) override;
    void BlitToScreen(TextureHandle src) override;
    void BeginRenderToTexture(TextureHandle target) override;
    void EndRenderToTexture() override;

    // Utility
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;

    // ImGui
    void ImGuiInit(GLFWwindow* window) override;
    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void ImGuiShutdown() override;

    // Query
    BackendType GetType() const override { return BackendType::Vulkan; }
    const char* GetName() const override { return "Vulkan 1.2 (SPIR-V)"; }
    int GetMaxTextureSize() const override;

private:
    // Window reference
    GLFWwindow* m_window = nullptr;
    int m_width = 1280;
    int m_height = 720;
    bool m_initialized = false;
    bool m_framebufferResized = false;

    // ============================================================================
    // Core Vulkan Objects
    // ============================================================================
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsFamily = 0;
    uint32_t m_presentFamily = 0;

    // ============================================================================
    // Surface and Swapchain
    // ============================================================================
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D m_swapchainExtent = {};
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    // ============================================================================
    // Command Buffers
    // ============================================================================
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    // ============================================================================
    // Synchronization Objects
    // ============================================================================
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    // ============================================================================
    // Frame State
    // ============================================================================
    uint32_t m_currentImageIndex = 0;
    bool m_isRecording = false;

    // ============================================================================
    // Resource Management
    // ============================================================================
    uint32_t m_nextShaderId = 1;
    uint32_t m_nextTextureId = 1;
    uint32_t m_nextPipelineId = 1;
    std::unordered_map<uint32_t, VulkanShader> m_shaders;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanTexture>> m_textures;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanPipeline>> m_pipelines;

    // ============================================================================
    // Current State
    // ============================================================================
    PipelineHandle m_currentPipeline = {0};
    VkFramebuffer m_currentFramebuffer = VK_NULL_HANDLE;
    VkRenderPass m_currentRenderPass = VK_NULL_HANDLE;
    bool m_isRenderToTexture = false;

    // ============================================================================
    // ImGui Vulkan Resources
    // ============================================================================
    VkDescriptorPool m_imguiDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_imguiDescSetLayout = VK_NULL_HANDLE;
    bool m_imguiInitialized = false;
    bool m_imguiRenderPending = false;  // Flag to indicate ImGui render is pending

    // Deferred destruction queue (resources to destroy after frame submission)
    struct DeferredDestroy {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descPool = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
    };
    std::vector<DeferredDestroy> m_deferredDestroys;

    // Pipeline cache: reuse pipelines for same shader+renderpass combination
    std::unordered_map<uint64_t, PipelineHandle> m_pipelineCache;

    // ============================================================================
    // Initialization Helpers
    // ============================================================================
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

    // ============================================================================
    // Swapchain Management
    // ============================================================================
    void CleanupSwapchain();
    void RecreateSwapchain();

    // ============================================================================
    // Command Buffer Recording
    // ============================================================================
    void RecordCommandBuffer();
    void BeginRenderPass();
    void EndRenderPass();

    // ============================================================================
    // Utility Functions
    // ============================================================================
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkSampler CreateSampler();

    // ============================================================================
    // Pipeline Helpers
    // ============================================================================
    VkDescriptorSetLayout CreateDescriptorSetLayout();
    VkDescriptorPool CreateDescriptorPool(uint32_t maxSets);
    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
    VkRenderPass CreateRenderPassForFormat(VkFormat format);

    // ============================================================================
    // Helper Queries
    // ============================================================================
    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    // Queue family indices
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        bool isComplete() const { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX; }
    };
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    // Swapchain support details
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
};
