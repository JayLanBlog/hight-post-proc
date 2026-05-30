#include "VulkanBackend.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstring>
#include <set>
#include <algorithm>
#include <stdexcept>

// ============================================================================
// Helper macros
// ============================================================================
#define VK_CHECK(x) do { VkResult err = (x); if (err) { fprintf(stderr, "[Vulkan] VK_CHECK failed at %s:%d: %d\n", __FILE__, __LINE__, err); throw std::runtime_error("Vulkan error"); } } while(0)

// ============================================================================
// Validation layers (debug builds only)
// ============================================================================
#ifdef _DEBUG
static const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
#else
static const std::vector<const char*> kValidationLayers = {};
#endif

static bool CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> available(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, available.data());

    for (const char* layerName : kValidationLayers) {
        bool found = false;
        for (const auto& props : available) {
            if (strcmp(layerName, props.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

static const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// ============================================================================
// Destructor
// ============================================================================
VulkanBackend::~VulkanBackend() {
    if (m_initialized) Shutdown();
}

// ============================================================================
// Init / Shutdown
// ============================================================================
bool VulkanBackend::Init(GLFWwindow* window) {
    m_window = window;
    glfwGetFramebufferSize(window, &m_width, &m_height);

    try {
        CreateInstance();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateRenderPass();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSyncObjects();
    } catch (const std::exception& e) {
        fprintf(stderr, "[Vulkan] Init failed: %s\n", e.what());
        return false;
    }

    m_initialized = true;
    printf("[Vulkan] Initialized\n");
    return true;
}

void VulkanBackend::Shutdown() {
    if (!m_initialized) return;
    WaitIdle();

    // Destroy shader modules
    for (auto& [id, mod] : m_shaderModules) {
        if (mod != VK_NULL_HANDLE)
            vkDestroyShaderModule(m_device, mod, nullptr);
    }
    m_shaderModules.clear();

    // Destroy textures
    for (auto& [id, view] : m_textureViews) {
        if (view != VK_NULL_HANDLE)
            vkDestroyImageView(m_device, view, nullptr);
    }
    for (auto& [id, img] : m_textureImages) {
        if (img != VK_NULL_HANDLE)
            vkDestroyImage(m_device, img, nullptr);
    }
    for (auto& [id, mem] : m_textureMemories) {
        if (mem != VK_NULL_HANDLE)
            vkFreeMemory(m_device, mem, nullptr);
    }
    m_textureViews.clear();
    m_textureImages.clear();
    m_textureMemories.clear();

    // Sync objects
    if (m_fence != VK_NULL_HANDLE)      vkDestroyFence(m_device, m_fence, nullptr);
    if (m_renderDone != VK_NULL_HANDLE) vkDestroySemaphore(m_device, m_renderDone, nullptr);
    if (m_imAvail != VK_NULL_HANDLE)    vkDestroySemaphore(m_device, m_imAvail, nullptr);

    // Command pool
    if (m_commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    // Swapchain
    CleanupSwapchain();

    // Render pass
    if (m_renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    // Device
    if (m_device != VK_NULL_HANDLE)
        vkDestroyDevice(m_device, nullptr);

    // Surface
    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    // Instance
    if (m_instance != VK_NULL_HANDLE)
        vkDestroyInstance(m_instance, nullptr);

    m_initialized = false;
    printf("[Vulkan] Shutdown\n");
}

// ============================================================================
// BeginFrame / EndFrame
// ============================================================================
void VulkanBackend::BeginFrame() {
    vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_fence);

    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
        m_imAvail, VK_NULL_HANDLE, &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }

    vkResetCommandBuffer(m_commandBuffer, 0);
    RecordCommandBuffer();
}

void VulkanBackend::EndFrame() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSems[]   = { m_imAvail };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores    = waitSems;
    submitInfo.pWaitDstStageMask  = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_commandBuffer;

    VkSemaphore signalSems[] = { m_renderDone };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSems;

    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSems;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapchain;
    presentInfo.pImageIndices      = &m_currentImageIndex;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else {
        VK_CHECK(result);
    }
}

void VulkanBackend::WaitIdle() {
    if (m_device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(m_device);
}

// ============================================================================
// Viewport
// ============================================================================
void VulkanBackend::Resize(int w, int h) {
    m_width = w;
    m_height = h;
}

void VulkanBackend::GetFramebufferSize(int& w, int& h) {
    w = m_width;
    h = m_height;
}

// ============================================================================
// CreateInstance
// ============================================================================
void VulkanBackend::CreateInstance() {
    // Validation layers
    if (!kValidationLayers.empty() && !CheckValidationLayerSupport()) {
        fprintf(stderr, "[Vulkan] Validation layers requested but not available\n");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shader Showcase";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shader Showcase";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // GLFW required extensions
    uint32_t glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    std::vector<const char*> extensions(glfwExts, glfwExts + glfwExtCount);

    if (!kValidationLayers.empty()) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
    createInfo.ppEnabledLayerNames = kValidationLayers.data();

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    printf("[Vulkan] Instance created\n");
}

// ============================================================================
// CreateSurface
// ============================================================================
void VulkanBackend::CreateSurface() {
    VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));
}

// ============================================================================
// PickPhysicalDevice
// ============================================================================
void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan-capable GPU found");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    // Score: discrete=10, integrated=5, other=1
    int bestScore = -1;
    for (auto& dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);

        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)       score = 10;
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score = 5;
        else                                                                  score = 1;

        if (score > bestScore) {
            bestScore = score;
            m_physicalDevice = dev;
        }
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    printf("[Vulkan] Physical device: %s\n", props.deviceName);
}

// ============================================================================
// CreateLogicalDevice
// ============================================================================
void VulkanBackend::CreateLogicalDevice() {
    // Queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    int graphicsIdx = -1, presentIdx = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsIdx = static_cast<int>(i);

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport) presentIdx = static_cast<int>(i);

        if (graphicsIdx >= 0 && presentIdx >= 0) break;
    }

    if (graphicsIdx < 0 || presentIdx < 0)
        throw std::runtime_error("Required queue families not found");

    m_graphicsFamily = static_cast<uint32_t>(graphicsIdx);
    m_presentFamily  = static_cast<uint32_t>(presentIdx);

    std::set<uint32_t> uniqueFamilies = { m_graphicsFamily, m_presentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t fam : uniqueFamilies) {
        VkDeviceQueueCreateInfo qi{};
        qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = fam;
        qi.queueCount = 1;
        qi.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(qi);
    }

    // Check device extension support
    uint32_t extCount;
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> availableExts(extCount);
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, availableExts.data());

    for (const char* required : kDeviceExtensions) {
        bool found = false;
        for (auto& av : availableExts) {
            if (strcmp(required, av.extensionName) == 0) { found = true; break; }
        }
        if (!found) throw std::runtime_error(std::string("Device extension not supported: ") + required);
    }

    VkPhysicalDeviceFeatures deviceFeatures{}; // no special features needed for skeleton

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK(vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device));

    vkGetDeviceQueue(m_device, m_graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentFamily, 0, &m_presentQueue);
    printf("[Vulkan] Logical device created\n");
}

// ============================================================================
// CreateSwapchain
// ============================================================================
void VulkanBackend::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &caps);

    // Extent
    VkExtent2D extent;
    if (caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        extent.width  = std::clamp(static_cast<uint32_t>(m_width), caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(m_height), caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    m_width  = static_cast<int>(extent.width);
    m_height = static_cast<int>(extent.height);

    uint32_t imageCount = std::max(2u, caps.minImageCount);
    if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

    // Format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            m_swapchainFormat = f.format;
            colorSpace = f.colorSpace;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapInfo{};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = m_surface;
    swapInfo.minImageCount = imageCount;
    swapInfo.imageFormat = m_swapchainFormat;
    swapInfo.imageColorSpace = colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (m_graphicsFamily != m_presentFamily) {
        uint32_t families[] = { m_graphicsFamily, m_presentFamily };
        swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapInfo.queueFamilyIndexCount = 2;
        swapInfo.pQueueFamilyIndices = families;
    } else {
        swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapInfo.preTransform = caps.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &swapInfo, nullptr, &m_swapchain));

    // Get images
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    // Create image views
    m_swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchainImageViews[i]));
    }

    printf("[Vulkan] Swapchain created (%dx%d, %zu images)\n", extent.width, extent.height, m_swapchainImages.size());
}

// ============================================================================
// CreateRenderPass
// ============================================================================
void VulkanBackend::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &colorAttachment;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_device, &rpInfo, nullptr, &m_renderPass));
}

// ============================================================================
// CreateFramebuffers
// ============================================================================
void VulkanBackend::CreateFramebuffers() {
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        VkImageView attachments[] = { m_swapchainImageViews[i] };

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = m_renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = attachments;
        fbInfo.width = static_cast<uint32_t>(m_width);
        fbInfo.height = static_cast<uint32_t>(m_height);
        fbInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_device, &fbInfo, nullptr, &m_swapchainFramebuffers[i]));
    }
}

// ============================================================================
// CreateCommandPool
// ============================================================================
void VulkanBackend::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsFamily;

    VK_CHECK(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

// ============================================================================
// CreateCommandBuffers
// ============================================================================
void VulkanBackend::CreateCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer));
}

// ============================================================================
// CreateSyncObjects
// ============================================================================
void VulkanBackend::CreateSyncObjects() {
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled so first frame doesn't block

    VK_CHECK(vkCreateSemaphore(m_device, &semInfo, nullptr, &m_imAvail));
    VK_CHECK(vkCreateSemaphore(m_device, &semInfo, nullptr, &m_renderDone));
    VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_fence));
}

// ============================================================================
// RecordCommandBuffer
// ============================================================================
void VulkanBackend::RecordCommandBuffer() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.15f, 1.0f}}};
    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = m_renderPass;
    rpInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    rpInfo.renderArea.extent = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_commandBuffer);

    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
}

// ============================================================================
// CleanupSwapchain / RecreateSwapchain
// ============================================================================
void VulkanBackend::CleanupSwapchain() {
    for (auto fb : m_swapchainFramebuffers) {
        if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(m_device, fb, nullptr);
    }
    m_swapchainFramebuffers.clear();

    for (auto iv : m_swapchainImageViews) {
        if (iv != VK_NULL_HANDLE) vkDestroyImageView(m_device, iv, nullptr);
    }
    m_swapchainImageViews.clear();

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    m_swapchainImages.clear();
}

void VulkanBackend::RecreateSwapchain() {
    WaitIdle();
    CleanupSwapchain();
    CreateSwapchain();
    // Recreate framebuffers for new swapchain images
    CreateFramebuffers();
}

// ============================================================================
// Shaders (SPIR-V)
// ============================================================================
ShaderHandle VulkanBackend::CreateVertexShader(const std::vector<uint32_t>& spirv) {
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = spirv.size() * sizeof(uint32_t);
    info.pCode = spirv.data();

    VkShaderModule mod;
    VK_CHECK(vkCreateShaderModule(m_device, &info, nullptr, &mod));

    uint32_t id = m_nextShaderId++;
    m_shaderModules[id] = mod;
    return { id };
}

ShaderHandle VulkanBackend::CreateFragmentShader(const std::vector<uint32_t>& spirv) {
    // Same path as vertex shader for SPIR-V loading
    return CreateVertexShader(spirv);
}

void VulkanBackend::DestroyShader(ShaderHandle handle) {
    auto it = m_shaderModules.find(handle.id);
    if (it != m_shaderModules.end()) {
        vkDestroyShaderModule(m_device, it->second, nullptr);
        m_shaderModules.erase(it);
    }
}

// ============================================================================
// Textures (STUBS)
// ============================================================================
TextureHandle VulkanBackend::CreateTexture(int w, int h, TextureFormat fmt, const void* data) {
    (void)w; (void)h; (void)fmt; (void)data;
    return INVALID_TEXTURE;
}

void VulkanBackend::DestroyTexture(TextureHandle handle) {
    (void)handle;
}

void VulkanBackend::UpdateTexture(TextureHandle handle, int w, int h, const void* data) {
    (void)handle; (void)w; (void)h; (void)data;
}

// ============================================================================
// Fullscreen draw / Cards / Blit (STUBS)
// ============================================================================
void VulkanBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    (void)vert; (void)frag; (void)params;
}

void VulkanBackend::DrawCards(const std::vector<CardDrawInfo>& cards, const float* viewMatrix, const float* projMatrix) {
    (void)cards; (void)viewMatrix; (void)projMatrix;
}

void VulkanBackend::BlitToScreen(TextureHandle texture) {
    (void)texture;
}

void VulkanBackend::BeginRenderToTexture(TextureHandle target) {
    (void)target;
}

void VulkanBackend::EndRenderToTexture() {
}

// ============================================================================
// ImGui (STUBS)
// ============================================================================
void VulkanBackend::ImGuiInit(GLFWwindow* window) {
    (void)window;
}

void VulkanBackend::ImGuiNewFrame() {
}

void VulkanBackend::ImGuiRender() {
}

void VulkanBackend::ImGuiShutdown() {
}

// ============================================================================
// Query
// ============================================================================
int VulkanBackend::GetMaxTextureSize() const {
    if (m_physicalDevice == VK_NULL_HANDLE) return 4096;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    return static_cast<int>(props.limits.maxImageDimension2D);
}

// ============================================================================
// Utility (STUBS for now - will be needed by texture / buffer ops later)
// ============================================================================
uint32_t VulkanBackend::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    throw std::runtime_error("No suitable memory type found");
}

VkCommandBuffer VulkanBackend::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    return cmd;
}

void VulkanBackend::EndSingleTimeCommands(VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &cmd);
}

void VulkanBackend::TransitionImageLayout(VkImage img, VkFormat fmt, VkImageLayout oldL, VkImageLayout newL) {
    (void)img; (void)fmt; (void)oldL; (void)newL;
    // Stub - will be implemented when textures are added
}

void VulkanBackend::CopyBufferToImage(VkBuffer buf, VkImage img, uint32_t w, uint32_t h) {
    (void)buf; (void)img; (void)w; (void)h;
    // Stub - will be implemented when textures are added
}

void VulkanBackend::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buf, VkDeviceMemory& mem) {
    (void)size; (void)usage; (void)props; (void)buf; (void)mem;
    // Stub - will be implemented when buffers are needed
}
