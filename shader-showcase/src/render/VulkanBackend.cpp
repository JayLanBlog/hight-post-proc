#include "VulkanBackend.h"
#include <GLFW/glfw3.h>

// Native Win32 surface creation — works on GLFW_OPENGL_API windows
// where glfwCreateWindowSurface() would fail on some NVIDIA drivers.
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>

#include <cstdio>
#include <cstring>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <string>
#include <array>
#include <fstream>

// stb_image
#include "stb_image.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// ============================================================================
// Configuration
// ============================================================================
#ifdef _DEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = false;
#endif

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// ============================================================================
// Helper Macros
// ============================================================================
#define VK_CHECK(x) do { \
    VkResult err = (x); \
    if (err != VK_SUCCESS) { \
        fprintf(stderr, "[Vulkan] VK_CHECK failed at %s:%d: %d\n", __FILE__, __LINE__, err); \
    } \
} while(0)

// ============================================================================
// Constructor / Destructor
// ============================================================================
VulkanBackend::VulkanBackend() = default;

VulkanBackend::~VulkanBackend() {
    if (m_initialized) {
        Shutdown();
    }
}

// ============================================================================
// Initialization
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
    printf("[Vulkan] Initialized successfully (%dx%d)\n", m_width, m_height);
    return true;
}

void VulkanBackend::CreateInstance() {
    // Check validation layer support
    if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport()) {
        fprintf(stderr, "[Vulkan] Validation layers requested but not available\n");
        throw std::runtime_error("Validation layers requested but not available");
    }

    // Application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shader Showcase";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shader Showcase Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Get required extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Add debug utils extension if validation layers are enabled
    if (ENABLE_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    printf("[Vulkan] Instance created\n");
}

void VulkanBackend::CreateSurface() {
    // Use native Win32 surface creation instead of glfwCreateWindowSurface.
    // GLFW_OPENGL_API windows are rejected by glfwCreateWindowSurface on
    // some NVIDIA drivers, but vkCreateWin32SurfaceKHR works on any HWND.
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = glfwGetWin32Window(m_window);

    VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
    printf("[Vulkan] Surface created (Win32 native)\n");
}

void VulkanBackend::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan-capable GPU found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    // Score devices and pick the best one
    int bestScore = -1;
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

    for (const auto& device : devices) {
        if (!IsDeviceSuitable(device)) continue;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 100;
        }

        // Prefer devices with larger memory
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);
        VkDeviceSize totalMemory = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalMemory += memProps.memoryHeaps[i].size;
            }
        }
        score += static_cast<int>(totalMemory / (1024 * 1024 * 1024)); // GB

        if (score > bestScore) {
            bestScore = score;
            bestDevice = device;
        }
    }

    if (bestDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("No suitable GPU found");
    }

    m_physicalDevice = bestDevice;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    printf("[Vulkan] Physical device: %s\n", props.deviceName);
}

bool VulkanBackend::IsDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = FindQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VulkanBackend::QueueFamilyIndices VulkanBackend::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        // Check for graphics support
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // Check for present support
        VkBool32 presentSupport = false;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport));
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

bool VulkanBackend::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanBackend::CheckValidationLayerSupport() {
    uint32_t layerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

    std::vector<VkLayerProperties> availableLayers(layerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    for (const char* layerName : VALIDATION_LAYERS) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

void VulkanBackend::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    m_graphicsFamily = indices.graphicsFamily;
    m_presentFamily = indices.presentFamily;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

    vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

    printf("[Vulkan] Logical device created (graphics queue: %u, present queue: %u)\n", 
           indices.graphicsFamily, indices.presentFamily);
}

VulkanBackend::SwapChainSupportDetails VulkanBackend::QuerySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data()));
    }

    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR VulkanBackend::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && 
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanBackend::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // Prefer FIFO (VSync) for stability
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanBackend::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanBackend::CreateSwapchain() {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain));

    // Get swapchain images
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));
    m_swapchainImages.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data()));

    m_swapchainFormat = surfaceFormat.format;
    m_swapchainExtent = extent;

    // Update dimensions
    m_width = static_cast<int>(extent.width);
    m_height = static_cast<int>(extent.height);

    printf("[Vulkan] Swapchain created (%dx%d, %zu images)\n", extent.width, extent.height, m_swapchainImages.size());
}

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

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
    printf("[Vulkan] Render pass created\n");
}

void VulkanBackend::CreateFramebuffers() {
    m_swapchainImageViews.resize(m_swapchainImages.size());
    m_swapchainFramebuffers.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapchainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]));

        VkImageView attachments[] = { m_swapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]));
    }

    printf("[Vulkan] Framebuffers created (%zu)\n", m_swapchainFramebuffers.size());
}

void VulkanBackend::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    VK_CHECK(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
    printf("[Vulkan] Command pool created\n");
}

void VulkanBackend::CreateCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer));
    printf("[Vulkan] Command buffer allocated\n");
}

void VulkanBackend::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't block

    VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore));
    VK_CHECK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore));
    VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence));

    printf("[Vulkan] Sync objects created\n");
}

// ============================================================================
// Shutdown
// ============================================================================
void VulkanBackend::Shutdown() {
    if (!m_initialized) return;

    WaitIdle();

    printf("[Vulkan] Shutting down...\n");

    // Process any pending deferred destructions (now safe after WaitIdle)
    for (auto& dd : m_deferredDestroys) {
        if (dd.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, dd.pipeline, nullptr);
        if (dd.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, dd.layout, nullptr);
        if (dd.descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, dd.descSetLayout, nullptr);
        if (dd.descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, dd.descPool, nullptr);
        if (dd.uboBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, dd.uboBuffer, nullptr);
        if (dd.uboMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, dd.uboMemory, nullptr);
    }
    m_deferredDestroys.clear();

    for (auto& dd : m_deferredTexDestroys) {
        if (dd.sampler != VK_NULL_HANDLE)     vkDestroySampler(m_device, dd.sampler, nullptr);
        if (dd.view != VK_NULL_HANDLE)         vkDestroyImageView(m_device, dd.view, nullptr);
        if (dd.image != VK_NULL_HANDLE)        vkDestroyImage(m_device, dd.image, nullptr);
        if (dd.memory != VK_NULL_HANDLE)       vkFreeMemory(m_device, dd.memory, nullptr);
        if (dd.framebuffer != VK_NULL_HANDLE)  vkDestroyFramebuffer(m_device, dd.framebuffer, nullptr);
        if (dd.renderPass != VK_NULL_HANDLE)   vkDestroyRenderPass(m_device, dd.renderPass, nullptr);
    }
    m_deferredTexDestroys.clear();

    // Clean up pipelines
    for (auto& [id, pipeline] : m_pipelines) {
        if (pipeline->pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, pipeline->pipeline, nullptr);
        }
        if (pipeline->layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, pipeline->layout, nullptr);
        }
        if (pipeline->descSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device, pipeline->descSetLayout, nullptr);
        }
        if (pipeline->descPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device, pipeline->descPool, nullptr);
        }
    }
    m_pipelines.clear();

    // Clean up textures
    for (auto& [id, texture] : m_textures) {
        if (texture->sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device, texture->sampler, nullptr);
        }
        if (texture->view != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, texture->view, nullptr);
        }
        if (texture->image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device, texture->image, nullptr);
        }
        if (texture->memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, texture->memory, nullptr);
        }
        if (texture->framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, texture->framebuffer, nullptr);
        }
        if (texture->renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(m_device, texture->renderPass, nullptr);
        }
    }
    m_textures.clear();

    // Clean up shaders
    for (auto& [id, shader] : m_shaders) {
        if (shader.module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shader.module, nullptr);
        }
    }
    m_shaders.clear();

    // Destroy sync objects
    if (m_inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_inFlightFence, nullptr);
        m_inFlightFence = VK_NULL_HANDLE;
    }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    // Destroy command pool
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    // Clean up swapchain
    CleanupSwapchain();

    // Destroy render pass
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    // Destroy device
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    // Destroy surface
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    // Destroy instance
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }

    m_initialized = false;
    printf("[Vulkan] Shutdown complete\n");
}

void VulkanBackend::CleanupSwapchain() {
    for (auto framebuffer : m_swapchainFramebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
    }
    m_swapchainFramebuffers.clear();

    for (auto imageView : m_swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
    m_swapchainImageViews.clear();

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    m_swapchainImages.clear();
}

void VulkanBackend::RecreateSwapchain() {
    // Handle minimization: wait until window has valid dimensions
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    while (fbWidth == 0 || fbHeight == 0) {
        glfwWaitEvents();
        glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    }

    WaitIdle();

    printf("[Vulkan] Recreating swapchain...\n");

    CleanupSwapchain();
    CreateSwapchain();
    CreateFramebuffers();

    if (m_imguiInitialized) {
        ImGui_ImplVulkan_SetMinImageCount(2);
    }

    m_framebufferResized = false;
}

// ============================================================================
// Frame Management
// ============================================================================
void VulkanBackend::BeginFrame() {
    // Guard against 0-dimension swapchain (window minimized during switch)
    if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0) {
        RecreateSwapchain();
        if (m_swapchainExtent.width == 0 || m_swapchainExtent.height == 0)
            return;
    }

    // Wait for previous frame
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);

    // Clean up deferred destructions from previous frame (GPU has finished using them)
    for (auto& dd : m_deferredDestroys) {
        if (dd.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_device, dd.pipeline, nullptr);
        if (dd.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_device, dd.layout, nullptr);
        if (dd.descSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, dd.descSetLayout, nullptr);
        if (dd.descPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, dd.descPool, nullptr);
        if (dd.uboBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, dd.uboBuffer, nullptr);
        if (dd.uboMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, dd.uboMemory, nullptr);
    }
    m_deferredDestroys.clear();

    for (auto& dd : m_deferredTexDestroys) {
        if (dd.sampler != VK_NULL_HANDLE)     vkDestroySampler(m_device, dd.sampler, nullptr);
        if (dd.view != VK_NULL_HANDLE)         vkDestroyImageView(m_device, dd.view, nullptr);
        if (dd.image != VK_NULL_HANDLE)        vkDestroyImage(m_device, dd.image, nullptr);
        if (dd.memory != VK_NULL_HANDLE)       vkFreeMemory(m_device, dd.memory, nullptr);
        if (dd.framebuffer != VK_NULL_HANDLE)  vkDestroyFramebuffer(m_device, dd.framebuffer, nullptr);
        if (dd.renderPass != VK_NULL_HANDLE)   vkDestroyRenderPass(m_device, dd.renderPass, nullptr);
    }
    m_deferredTexDestroys.clear();

    // Acquire next image
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, 
                                            m_imageAvailableSemaphore, VK_NULL_HANDLE, 
                                            &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "[Vulkan] Failed to acquire swapchain image: %d\n", result);
        return;
    }

    // Reset and begin command buffer
    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
    m_isRecording = true;

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchainExtent;

    VkClearValue clearColor = {{{m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Track current render pass for pipeline creation
    m_currentRenderPass = m_renderPass;
    m_currentFramebuffer = m_swapchainFramebuffers[m_currentImageIndex];
}

void VulkanBackend::EndFrame() {
    if (!m_isRecording) return;

    // Render ImGui if pending (before ending render pass)
    if (m_imguiRenderPending) {
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->CmdListsCount > 0) {
            ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffer);
        }
        m_imguiRenderPending = false;
    }

    // End render pass
    vkCmdEndRenderPass(m_commandBuffer);

    // Clear current render pass tracking
    m_currentRenderPass = VK_NULL_HANDLE;
    m_currentFramebuffer = VK_NULL_HANDLE;

    // End command buffer
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
    m_isRecording = false;

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence));

    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_currentImageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] Failed to present swapchain image: %d\n", result);
    }
}

void VulkanBackend::WaitIdle() {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
}

// ============================================================================
// Viewport
// ============================================================================
void VulkanBackend::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    m_framebufferResized = true;
}

void VulkanBackend::GetFramebufferSize(int& width, int& height) {
    width = m_width;
    height = m_height;
}

// ============================================================================
// Shaders
// ============================================================================
ShaderHandle VulkanBackend::CreateVertexShader(const uint32_t* spirv, size_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size * sizeof(uint32_t);
    createInfo.pCode = spirv;

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule));

    uint32_t id = m_nextShaderId++;
    VulkanShader shader;
    shader.module = shaderModule;
    shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_shaders[id] = shader;

    printf("[Vulkan] Vertex shader created (id=%u)\n", id);
    return {id};
}

ShaderHandle VulkanBackend::CreateFragmentShader(const uint32_t* spirv, size_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size * sizeof(uint32_t);
    createInfo.pCode = spirv;

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule));

    uint32_t id = m_nextShaderId++;
    VulkanShader shader;
    shader.module = shaderModule;
    shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    m_shaders[id] = shader;

    printf("[Vulkan] Fragment shader created (id=%u)\n", id);
    return {id};
}

ShaderHandle VulkanBackend::CreateVertexShaderFromGLSL(const std::string& source) {
    (void)source;
    fprintf(stderr, "[Vulkan] CreateVertexShaderFromGLSL not implemented, use SPIRV directly\n");
    return INVALID_SHADER;
}

ShaderHandle VulkanBackend::CreateFragmentShaderFromGLSL(const std::string& source) {
    (void)source;
    fprintf(stderr, "[Vulkan] CreateFragmentShaderFromGLSL not implemented, use SPIRV directly\n");
    return INVALID_SHADER;
}

void VulkanBackend::DestroyShader(ShaderHandle handle) {
    auto it = m_shaders.find(handle.id);
    if (it != m_shaders.end()) {
        if (it->second.module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, it->second.module, nullptr);
        }
        m_shaders.erase(it);
        printf("[Vulkan] Shader destroyed (id=%u)\n", handle.id);
    }
}

// ============================================================================
// Textures
// ============================================================================
static VkFormat TextureFormatToVkFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:   return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::R8:      return VK_FORMAT_R8_UNORM;
        default:                     return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

static size_t GetTextureFormatSize(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:   return 4;
        case TextureFormat::RGBA32F: return 16;
        case TextureFormat::R8:      return 1;
        default:                     return 4;
    }
}

TextureHandle VulkanBackend::CreateTexture(int width, int height, TextureFormat format, const void* data) {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    size_t pixelSize = GetTextureFormatSize(format);
    VkDeviceSize imageSize = width * height * pixelSize;

    auto texture = std::make_unique<VulkanTexture>();
    texture->width = width;
    texture->height = height;
    texture->format = vkFormat;
    texture->isFBO = (data == nullptr);  // No initial data means it's a render target

    // Create image
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture->isFBO) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    CreateImage(
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        vkFormat,
        VK_IMAGE_TILING_OPTIMAL,
        usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture->image,
        texture->memory
    );

    // Create image view
    texture->view = CreateImageView(texture->image, vkFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    // Create sampler
    texture->sampler = CreateSampler();

    // Upload data if provided
    if (data != nullptr) {
        // Create staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        // Copy data to staging buffer
        void* mappedData;
        vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingBufferMemory);

        // Transition layout and copy
        TransitionImageLayout(texture->image, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, texture->image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        TransitionImageLayout(texture->image, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Cleanup staging buffer
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    } else {
        // For render targets, transition to SHADER_READ_ONLY_OPTIMAL.
        // The FBO render pass expects initialLayout=SHADER_READ_ONLY_OPTIMAL.
        TransitionImageLayout(texture->image, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    uint32_t id = m_nextTextureId++;
    m_textures[id] = std::move(texture);

    printf("[Vulkan] Texture created (id=%u, %dx%d, format=%d)\n", id, width, height, static_cast<int>(format));
    return {id};
}

TextureHandle VulkanBackend::CreateTextureFromFile(const std::string& path) {
    int width, height;
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, NULL, 4); // force RGBA
    if (!pixels) {
        fprintf(stderr, "[Vulkan] Failed to load texture: %s\n", path.c_str());
        return INVALID_TEXTURE;
    }

    TextureHandle handle = CreateTexture(width, height, TextureFormat::RGBA8, pixels);
    stbi_image_free(pixels);

    if (handle.id != 0) {
        fprintf(stdout, "[Vulkan] Loaded texture from file: %s (%dx%d)\n", path.c_str(), width, height);
    }
    return handle;
}

TextureHandle VulkanBackend::CreateTextureFromData(int width, int height, const uint8_t* rgbaData) {
    return CreateTexture(width, height, TextureFormat::RGBA8, rgbaData);
}

void VulkanBackend::UpdateTexture(TextureHandle handle, int x, int y, int width, int height, const void* data) {
    auto it = m_textures.find(handle.id);
    if (it == m_textures.end() || data == nullptr) return;

    auto& tex = it->second;
    size_t pixelSize = 4;  // Assume RGBA8 for now
    VkDeviceSize imageSize = width * height * pixelSize;

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    // Copy data to staging buffer
    void* mappedData;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
    memcpy(mappedData, data, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Transition to transfer dst, copy, then transition back
    TransitionImageLayout(tex->image, tex->format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {x, y, 0};
    region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(commandBuffer);

    TransitionImageLayout(tex->image, tex->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Cleanup staging buffer
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanBackend::DestroyTexture(TextureHandle handle) {
    auto it = m_textures.find(handle.id);
    if (it == m_textures.end()) return;
    auto* tex = it->second.get();

    // Queue GPU resources for deferred destruction.
    // They may still be referenced by an in-flight command buffer / descriptor set.
    // We move the raw handles out of the texture record and delete them in
    // BeginFrame, after vkWaitForFences guarantees the GPU is done.
    DeferredTexDestroy dd;
    dd.image       = tex->image;
    dd.view        = tex->view;
    dd.sampler     = tex->sampler;
    dd.memory      = tex->memory;
    dd.framebuffer = tex->framebuffer;
    dd.renderPass  = tex->renderPass;
    m_deferredTexDestroys.push_back(dd);

    // Clear the record's handles so the unique_ptr destructor won't double-free
    tex->image       = VK_NULL_HANDLE;
    tex->view        = VK_NULL_HANDLE;
    tex->sampler     = VK_NULL_HANDLE;
    tex->memory      = VK_NULL_HANDLE;
    tex->framebuffer = VK_NULL_HANDLE;
    tex->renderPass  = VK_NULL_HANDLE;

    m_textures.erase(it);
    printf("[Vulkan] Texture queued for deferred destroy (id=%u)\n", handle.id);
}

void* VulkanBackend::GetImTextureID(TextureHandle handle) {
    auto it = m_textures.find(handle.id);
    if (it == m_textures.end()) return nullptr;

    auto* tex = it->second.get();

    // Create ImGui descriptor set using ImGui_ImplVulkan_AddTexture
    // This uses VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE (set 0) which matches ImGui's pipeline layout
    if (tex->imguiDescriptorSet == VK_NULL_HANDLE) {
        tex->imguiDescriptorSet = ImGui_ImplVulkan_AddTexture(tex->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    return reinterpret_cast<void*>(tex->imguiDescriptorSet);
}

// ============================================================================
// Pipeline Helpers
// ============================================================================
VkDescriptorSetLayout VulkanBackend::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding bindings[3] = {};
    // binding=0: combined image sampler
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // binding=1: uniform buffer
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    // binding=2: uAuxTex (auxiliary texture, e.g. droplet texture, ramp texture)
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;
    VkDescriptorSetLayout descriptorSetLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout));
    return descriptorSetLayout;
}

VkDescriptorPool VulkanBackend::CreateDescriptorPool(uint32_t maxSets) {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets * 2 },  // *2 for binding=0 + binding=2
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = maxSets;

    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

VkPipelineLayout VulkanBackend::CreatePipelineLayout(VkDescriptorSetLayout descSetLayout) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (descSetLayout != VK_NULL_HANDLE) ? 1 : 0;
    pipelineLayoutInfo.pSetLayouts = (descSetLayout != VK_NULL_HANDLE) ? &descSetLayout : nullptr;
    // No push constants — uniforms go through UBO (binding=1)

    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkRenderPass VulkanBackend::CreateRenderPassForFormat(VkFormat format) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependency: transition from shader read (external) to color attachment (subpass 0)
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Also need dependency for transition back to shader read after subpass
    VkSubpassDependency postDependency{};
    postDependency.srcSubpass = 0;
    postDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    postDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    postDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    postDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkSubpassDependency dependencies[2] = { dependency, postDependency };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies;

    VkRenderPass renderPass;
    VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

// ============================================================================
// Pipelines
// ============================================================================
PipelineHandle VulkanBackend::CreatePipeline(const PipelineDesc& desc) {
    // Get shaders
    auto vertIt = m_shaders.find(desc.vertShader.id);
    auto fragIt = m_shaders.find(desc.fragShader.id);
    if (vertIt == m_shaders.end() || fragIt == m_shaders.end()) {
        fprintf(stderr, "[Vulkan] Invalid shader handles for pipeline creation\n");
        return {0};
    }

    // Cache key: vert shader + frag shader + vertex input mode
    VkRenderPass renderPass = m_renderPass;  // always use swapchain render pass
    uint64_t cacheKey = (uint64_t(desc.vertShader.id) << 32) | uint64_t(desc.fragShader.id);
    cacheKey ^= (desc.useVertexInput ? (1ULL << 60) : 0);
    cacheKey ^= (desc.blendEnable ? (1ULL << 59) : 0);
    cacheKey ^= (uint64_t(renderPass) >> 3);  // pointer as hash (shift to drop alignment bits)

    // Check cache
    auto cacheIt = m_pipelineCache.find(cacheKey);
    if (cacheIt != m_pipelineCache.end()) {
        return cacheIt->second;
    }

    auto pipeline = std::make_unique<VulkanPipeline>();
    pipeline->vertShader = desc.vertShader;
    pipeline->fragShader = desc.fragShader;

    // Create descriptor set layout and pipeline layout
    pipeline->descSetLayout = CreateDescriptorSetLayout();
    pipeline->layout = CreatePipelineLayout(pipeline->descSetLayout);
    pipeline->descPool = CreateDescriptorPool(1000);

    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertIt->second.module;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragIt->second.module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input state
    VkVertexInputBindingDescription bindingDesc{};
    VkVertexInputAttributeDescription attrDesc[3]{};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    if (desc.useVertexInput) {
        // 3D mesh vertex layout: pos(3) + normal(3) + uv(2) = 8 floats * 4 = 32 bytes stride
        bindingDesc.binding = 0;
        bindingDesc.stride = 32;
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        // Position: location=0, format R32G32B32, offset 0
        attrDesc[0].location = 0;
        attrDesc[0].binding = 0;
        attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[0].offset = 0;

        // Normal: location=1, format R32G32B32, offset 12
        attrDesc[1].location = 1;
        attrDesc[1].binding = 0;
        attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[1].offset = 12;

        // UV: location=2, format R32G32, offset 24
        attrDesc[2].location = 2;
        attrDesc[2].binding = 0;
        attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrDesc[2].offset = 24;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.vertexAttributeDescriptionCount = 3;
        vertexInputInfo.pVertexAttributeDescriptions = attrDesc;
    } else {
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(desc.width);
    viewport.height = static_cast<float>(desc.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(desc.width), static_cast<uint32_t>(desc.height)};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (desc.blendEnable) {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    } else {
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic state
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipeline->layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkResult pipeResult = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->pipeline);
    if (pipeResult != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] vkCreateGraphicsPipelines failed: %d (vert=%d frag=%d vtxInput=%d)\n",
                pipeResult, desc.vertShader.id, desc.fragShader.id, desc.useVertexInput);
        // Cache failure to avoid repeated attempts
        m_pipelineCache[cacheKey] = PipelineHandle{0};
        return PipelineHandle{0};
    }

    uint32_t id = m_nextPipelineId++;
    m_pipelines[id] = std::move(pipeline);

    // Add to cache for reuse
    m_pipelineCache[cacheKey] = PipelineHandle{id};

    // --- Create per-pipeline UBO and descriptor set (reused every frame) ---
    auto& pp = m_pipelines[id];
    const size_t UBO_SIZE = 224;  // std140: 6f+vec2+2f+pad8+mat4+mat4+3x vec3(16 each)
    CreateBuffer(UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 pp->uboBuffer, pp->uboMemory);

    // Allocate one descriptor set for this pipeline
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pp->descPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &pp->descSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &pp->descSet));
    }

    printf("[Vulkan] Pipeline created (id=%u)\n", id);
    return {id};
}

void VulkanBackend::DestroyPipeline(PipelineHandle handle) {
    auto it = m_pipelines.find(handle.id);
    if (it != m_pipelines.end()) {
        auto& pipe = it->second;
        if (pipe->pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, pipe->pipeline, nullptr);
        }
        if (pipe->layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, pipe->layout, nullptr);
        }
        if (pipe->descSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device, pipe->descSetLayout, nullptr);
        }
        if (pipe->uboBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device, pipe->uboBuffer, nullptr);
            pipe->uboBuffer = VK_NULL_HANDLE;
        }
        if (pipe->uboMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, pipe->uboMemory, nullptr);
            pipe->uboMemory = VK_NULL_HANDLE;
        }
        if (pipe->descPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device, pipe->descPool, nullptr);
        }
        m_pipelines.erase(it);
        printf("[Vulkan] Pipeline destroyed (id=%u)\n", handle.id);
    }
}

void VulkanBackend::BindPipeline(PipelineHandle handle) {
    auto it = m_pipelines.find(handle.id);
    if (it != m_pipelines.end()) {
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, it->second->pipeline);
        m_currentPipeline = handle;
    }
}

// ============================================================================
// Rendering
// ============================================================================
void VulkanBackend::DrawFullscreenQuad(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params) {
    if (!m_isRecording) return;

    PipelineDesc desc;
    desc.vertShader = vert;
    desc.fragShader = frag;
    desc.width = params.viewportWidth;
    desc.height = params.viewportHeight;
    desc.blendEnable = params.blendEnable;

    PipelineHandle pipeHandle = CreatePipeline(desc);
    if (pipeHandle.id == 0) return;

    BindPipeline(pipeHandle);

    // Set viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(params.viewportWidth);
    viewport.height = static_cast<float>(params.viewportHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    // Set scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(params.viewportWidth), static_cast<uint32_t>(params.viewportHeight)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    // Bind pre-allocated descriptor set (UBO is pipeline-owned, texture is per-frame)
    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end() && pipeIt->second->descSet != VK_NULL_HANDLE) {
        const size_t UBO_SIZE = 224;  // std140: 6f + vec2 + 2f + pad8 + mat4 + mat4 + 3× vec3(16)

        // --- Update UBO data via map/memcpy/unmap (HOST_COHERENT, no flush needed) ---
        {
            uint8_t uboData[UBO_SIZE] = {};
            // P0-P5: 6 floats at offset 0-23
            for (size_t i = 0; i < std::min(params.uniformFloats.size(), size_t(6)); i++) {
                float v = params.uniformFloats[i];
                memcpy(uboData + i * 4, &v, sizeof(float));
            }
            // uResolution: vec2 at offset 24
            { float r[2] = { static_cast<float>(params.viewportWidth), static_cast<float>(params.viewportHeight) }; memcpy(uboData + 24, r, 8); }
            // uTime, uFrameCount: at offset 32, 36
            { memcpy(uboData + 32, &params.time, 4); float fc = static_cast<float>(params.frameCount); memcpy(uboData + 36, &fc, 4); }
            // padding to align mat4 at offset 48 (8 bytes of pad)
            // uMVP: mat4 at offset 48 (64 bytes)
            if (params.mvp.size() >= 16)
                memcpy(uboData + 48, params.mvp.data(), 64);
            // uModelView: mat4 at offset 112 (64 bytes)
            if (params.modelView.size() >= 16)
                memcpy(uboData + 112, params.modelView.data(), 64);
            // uLightDir: vec3 at offset 176
            if (params.lightDir.size() >= 3)
                memcpy(uboData + 176, params.lightDir.data(), 12);
            // uLightColor: vec3 at offset 192
            if (params.lightColor.size() >= 3)
                memcpy(uboData + 192, params.lightColor.data(), 12);
            // uEyePos: vec3 at offset 208
            if (params.eyePos.size() >= 3)
                memcpy(uboData + 208, params.eyePos.data(), 12);

            void* mapped = nullptr;
            vkMapMemory(m_device, pipeIt->second->uboMemory, 0, UBO_SIZE, 0, &mapped);
            memcpy(mapped, uboData, UBO_SIZE);
            vkUnmapMemory(m_device, pipeIt->second->uboMemory);
        }

        // --- Update pre-allocated descriptor set with current texture + UBO ---
        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
        std::vector<VkWriteDescriptorSet> writes;

        if (!params.inputTextures.empty()) {
            auto texIt = m_textures.find(params.inputTextures[0].id);
            if (texIt != m_textures.end()) {
                imageInfo.sampler = texIt->second->sampler;
                imageInfo.imageView = texIt->second->view;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet texWrite{};
                texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                texWrite.dstSet = pipeIt->second->descSet;
                texWrite.dstBinding = 0;
                texWrite.dstArrayElement = 0;
                texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                texWrite.descriptorCount = 1;
                texWrite.pImageInfo = &imageInfo;
                writes.push_back(texWrite);
            }
        }

        bufferInfo.buffer = pipeIt->second->uboBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = UBO_SIZE;

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = pipeIt->second->descSet;
        uboWrite.dstBinding = 1;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        writes.push_back(uboWrite);

        // binding=2: auxiliary texture (uAuxTex)
        if (!params.auxTextures.empty()) {
            auto auxIt = m_textures.find(params.auxTextures[0].id);
            if (auxIt != m_textures.end()) {
                VkDescriptorImageInfo auxImageInfo{};
                auxImageInfo.sampler = auxIt->second->sampler;
                auxImageInfo.imageView = auxIt->second->view;
                auxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet auxWrite{};
                auxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                auxWrite.dstSet = pipeIt->second->descSet;
                auxWrite.dstBinding = 2;
                auxWrite.dstArrayElement = 0;
                auxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                auxWrite.descriptorCount = 1;
                auxWrite.pImageInfo = &auxImageInfo;
                writes.push_back(auxWrite);
            }
        }

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeIt->second->layout, 0, 1, &pipeIt->second->descSet, 0, nullptr);
    }

    // Draw fullscreen triangle (3 vertices, no vertex buffer)
    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    // NOTE: Do NOT destroy pipeline here. Vulkan commands are async and GPU still needs them.
    // Pipelines are cached in m_pipelines and will be cleaned up when the backend is destroyed.
    // The UBO buffer is also kept alive with the pipeline.
    m_currentPipeline = {0};
}

void VulkanBackend::DrawToScreen(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params, TextureHandle inputTex) {
    if (!m_isRecording) return;

    // Ensure we're rendering to swapchain (not FBO)
    if (m_isRenderToTexture) {
        EndRenderToTexture();
    }

    ShaderParams screenParams = params;
    screenParams.inputTextures = {inputTex};
    DrawFullscreenQuad(vert, frag, screenParams);
}

void VulkanBackend::DrawCards(const std::vector<IRenderBackend::CardDrawInfo>& cards, const float* viewMatrix, const float* projMatrix) {
    (void)cards; (void)viewMatrix; (void)projMatrix;
    // TODO: Implement card rendering
    // This would require vertex buffers, index buffers, and more complex setup
}

// ============================================================================
// 3D Mesh Drawing
// ============================================================================
void VulkanBackend::DrawMesh(ShaderHandle vert, ShaderHandle frag, const ShaderParams& params,
                              const float* vertexData, size_t vertexCount, size_t vertexStride,
                              const uint32_t* indexData, size_t indexCount)
{
    if (!m_isRecording) return;

    PipelineDesc desc;
    desc.vertShader = vert;
    desc.fragShader = frag;
    desc.width = params.viewportWidth;
    desc.height = params.viewportHeight;
    desc.blendEnable = false;
    desc.useVertexInput = true;

    PipelineHandle pipeHandle = CreatePipeline(desc);
    if (pipeHandle.id == 0) return;

    BindPipeline(pipeHandle);

    // Set viewport
    VkViewport vp{};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width = static_cast<float>(params.viewportWidth);
    vp.height = static_cast<float>(params.viewportHeight);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &vp);

    // Set scissor
    VkRect2D sc{};
    sc.offset = {0, 0};
    sc.extent = {static_cast<uint32_t>(params.viewportWidth), static_cast<uint32_t>(params.viewportHeight)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &sc);

    // --- Create temporary vertex buffer ---
    VkDeviceSize vbSize = vertexCount * vertexStride;
    VkBuffer vb = VK_NULL_HANDLE;
    VkDeviceMemory vbMem = VK_NULL_HANDLE;
    {
        VkBufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.size = vbSize;
        ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(m_device, &ci, nullptr, &vb));

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(m_device, vb, &memReq);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = memReq.size;
        ai.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(m_device, &ai, nullptr, &vbMem));
        VK_CHECK(vkBindBufferMemory(m_device, vb, vbMem, 0));

        void* mapped;
        VK_CHECK(vkMapMemory(m_device, vbMem, 0, vbSize, 0, &mapped));
        memcpy(mapped, vertexData, static_cast<size_t>(vbSize));
        vkUnmapMemory(m_device, vbMem);
    }

    // --- Create temporary index buffer ---
    VkDeviceSize ibSize = indexCount * sizeof(uint32_t);
    VkBuffer ib = VK_NULL_HANDLE;
    VkDeviceMemory ibMem = VK_NULL_HANDLE;
    {
        VkBufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.size = ibSize;
        ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(m_device, &ci, nullptr, &ib));

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(m_device, ib, &memReq);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = memReq.size;
        ai.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(m_device, &ai, nullptr, &ibMem));
        VK_CHECK(vkBindBufferMemory(m_device, ib, ibMem, 0));

        void* mapped;
        VK_CHECK(vkMapMemory(m_device, ibMem, 0, ibSize, 0, &mapped));
        memcpy(mapped, indexData, static_cast<size_t>(ibSize));
        vkUnmapMemory(m_device, ibMem);
    }

    // --- Bind vertex and index buffers ---
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &vb, offsets);
    vkCmdBindIndexBuffer(m_commandBuffer, ib, 0, VK_INDEX_TYPE_UINT32);

    // --- Update UBO (224 bytes) and bind descriptor set ---
    auto pipeIt = m_pipelines.find(pipeHandle.id);
    if (pipeIt != m_pipelines.end() && pipeIt->second->descSet != VK_NULL_HANDLE) {
        const size_t UBO_SIZE = 224;

        // Update UBO data via map/memcpy/unmap
        {
            uint8_t uboData[224] = {};
            // floats 0-5 (offset 0-23)
            for (size_t i = 0; i < std::min(params.uniformFloats.size(), size_t(6)); i++) {
                float v = params.uniformFloats[i];
                memcpy(uboData + i * 4, &v, sizeof(float));
            }
            // uResolution + uTime + uFrameCount (offset 24-39)
            { float r[2] = {static_cast<float>(params.viewportWidth), static_cast<float>(params.viewportHeight)}; memcpy(uboData + 24, r, 8); }
            { memcpy(uboData + 32, &params.time, 4); float fc = static_cast<float>(params.frameCount); memcpy(uboData + 36, &fc, 4); }

            // std140 layout (224 bytes):
            // float[6]@0-23, vec2@24-31, float@32-35, float@36-39
            // [pad 8]@40-47 (mat4 needs align 16)
            // mat4 MVP@48-111, mat4 ModelView@112-175
            // vec3 LightDir@176-191 (align 16), vec3 LightColor@192-207 (align 16), vec3 EyePos@208-223 (align 16)
            if (!params.mvp.empty()) memcpy(uboData + 48, params.mvp.data(), 64);
            if (!params.modelView.empty()) memcpy(uboData + 112, params.modelView.data(), 64);
            if (!params.lightDir.empty()) memcpy(uboData + 176, params.lightDir.data(), 12);
            if (!params.lightColor.empty()) memcpy(uboData + 192, params.lightColor.data(), 12);
            if (!params.eyePos.empty()) memcpy(uboData + 208, params.eyePos.data(), 12);

            void* mapped = nullptr;
            VK_CHECK(vkMapMemory(m_device, pipeIt->second->uboMemory, 0, UBO_SIZE, 0, &mapped));
            memcpy(mapped, uboData, UBO_SIZE);
            vkUnmapMemory(m_device, pipeIt->second->uboMemory);
        }

        // --- Update descriptor set with current texture + UBO ---
        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
        std::vector<VkWriteDescriptorSet> writes;

        if (!params.inputTextures.empty()) {
            auto texIt = m_textures.find(params.inputTextures[0].id);
            if (texIt != m_textures.end()) {
                imageInfo.sampler = texIt->second->sampler;
                imageInfo.imageView = texIt->second->view;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet texWrite{};
                texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                texWrite.dstSet = pipeIt->second->descSet;
                texWrite.dstBinding = 0;
                texWrite.dstArrayElement = 0;
                texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                texWrite.descriptorCount = 1;
                texWrite.pImageInfo = &imageInfo;
                writes.push_back(texWrite);
            }
        }

        bufferInfo.buffer = pipeIt->second->uboBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = UBO_SIZE;

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = pipeIt->second->descSet;
        uboWrite.dstBinding = 1;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        writes.push_back(uboWrite);

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeIt->second->layout, 0, 1, &pipeIt->second->descSet, 0, nullptr);
    }

    // Draw indexed
    vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);

    // NOTE: Destroying vertex/index buffers immediately is unsafe in Vulkan
    // (GPU commands are asynchronous). For production, use a ring buffer or
    // defer-destroy scheme. The leak is acceptable for demo/single-frame use.
    // Note: temp VBO/IBO not destroyed immediately — GPU may still use them.
    // They leak ~70KB per DrawMesh call, acceptable for auto-test mode.

    m_currentPipeline = {0};
}

void VulkanBackend::BlitToScreen(TextureHandle src) {
    auto it = m_textures.find(src.id);
    if (it == m_textures.end()) return;

    // For now, just transition the image layout if needed
    // Full implementation would require a blit shader or vkCmdBlitImage
    // This is a placeholder
}

void VulkanBackend::BeginRenderToTexture(TextureHandle target) {
    if (!m_isRecording) return;

    auto it = m_textures.find(target.id);
    if (it == m_textures.end()) {
        return;
    }

    auto& tex = it->second;

    // End current render pass if active
    if (m_currentRenderPass != VK_NULL_HANDLE) {
        vkCmdEndRenderPass(m_commandBuffer);
    }

    // Create framebuffer for this texture if not exists
    if (tex->framebuffer == VK_NULL_HANDLE) {
        // Create render pass for this format
        if (tex->renderPass == VK_NULL_HANDLE) {
            tex->renderPass = CreateRenderPassForFormat(tex->format);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = tex->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &tex->view;
        framebufferInfo.width = static_cast<uint32_t>(tex->width);
        framebufferInfo.height = static_cast<uint32_t>(tex->height);
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &tex->framebuffer));
    }

    // Render pass handles layout transition automatically via initialLayout=UNDEFINED/finalLayout=SHADER_READ_ONLY.
    // No manual barrier needed.

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = tex->renderPass;
    renderPassInfo.framebuffer = tex->framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {static_cast<uint32_t>(tex->width), static_cast<uint32_t>(tex->height)};

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_currentFramebuffer = tex->framebuffer;
    m_currentRenderPass = tex->renderPass;
    m_isRenderToTexture = true;
}

void VulkanBackend::EndRenderToTexture() {
    if (!m_isRecording || !m_isRenderToTexture) return;

    // End FBO render pass - finalLayout transitions to SHADER_READ_ONLY_OPTIMAL
    vkCmdEndRenderPass(m_commandBuffer);

    // Resume swapchain render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchainExtent;

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.15f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_currentFramebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    m_currentRenderPass = m_renderPass;
    m_isRenderToTexture = false;
}

// ============================================================================
// Utility
// ============================================================================
void VulkanBackend::SetViewport(int x, int y, int width, int height) {
    if (!m_isRecording) return;

    VkViewport viewport{};
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {x, y};
    scissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void VulkanBackend::Clear(float r, float g, float b, float a) {
    m_clearColor[0] = r; m_clearColor[1] = g;
    m_clearColor[2] = b; m_clearColor[3] = a;
}

// ============================================================================
// ImGui Vulkan Implementation
// ============================================================================
void VulkanBackend::ImGuiInit(GLFWwindow* window) {
    if (m_imguiInitialized) return;

    printf("[Vulkan] Initializing ImGui Vulkan backend...\n");

    IMGUI_CHECKVERSION();
    if (ImGui::GetCurrentContext() == nullptr) {
        ImGui::CreateContext();
    }

    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Load fonts at large base size for crisp rendering when scaled up.
    // CoverFlowScene renders text at up to ~35 px — loading at 16 px causes
    // severe aliasing when glyphs are enlarged.  32 px base + oversampling
    // gives sharp results at both small and large sizes.
    const float BASE_FONT_SIZE = 32.0f;
    ImGuiIO& io = ImGui::GetIO();

    const char* chineseFontPath = nullptr;

    // Check for Microsoft YaHei (msyh.ttc) - best CJK coverage on Windows
    {
        std::ifstream testFile("C:\\Windows\\Fonts\\msyh.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    }
    // Fallback: SimSun
    if (!chineseFontPath) {
        std::ifstream testFile("C:\\Windows\\Fonts\\simsun.ttc", std::ios::binary);
        if (testFile.good()) chineseFontPath = "C:\\Windows\\Fonts\\simsun.ttc";
    }

    if (chineseFontPath) {
        // Build full CJK + Latin atlas in one pass (avoids tiny AddFontDefault glyphs)
        ImFontConfig cfg;
        cfg.OversampleH = 3;   // horizontal sub-pixel anti-aliasing
        cfg.OversampleV = 1;
        cfg.RasterizerMultiply = 1.2f;  // slightly bolder for readability
        io.Fonts->AddFontFromFileTTF(chineseFontPath, BASE_FONT_SIZE, &cfg,
            io.Fonts->GetGlyphRangesChineseFull());
    }

    // Create descriptor pool
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_imguiDescriptorPool));

    // Create a descriptor set layout for ImGui texture display
    // (single combined image sampler at binding 0)
    VkDescriptorSetLayoutBinding imguiSamplerBinding{};
    imguiSamplerBinding.binding = 0;
    imguiSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imguiSamplerBinding.descriptorCount = 1;
    imguiSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo imguiLayoutInfo{};
    imguiLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    imguiLayoutInfo.bindingCount = 1;
    imguiLayoutInfo.pBindings = &imguiSamplerBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &imguiLayoutInfo, nullptr, &m_imguiDescSetLayout));

    // Initialize ImGui Vulkan backend
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.QueueFamily = m_graphicsFamily;
    initInfo.Queue = m_graphicsQueue;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_imguiDescriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(m_swapchainImages.size());
    initInfo.PipelineInfoMain.RenderPass = m_renderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    // ImGui_ImplVulkan_Init sets RendererHasTextures flag. Font textures are
    // uploaded automatically by ImGui_ImplVulkan_RenderDrawData() on first frame.
    m_imguiInitialized = true;
    printf("[Vulkan] ImGui Vulkan backend initialized successfully\n");
}

void VulkanBackend::ImGuiNewFrame() {
    if (!m_imguiInitialized) return;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VulkanBackend::ImGuiRender() {
    if (!m_imguiInitialized) return;
    ImGui::Render();
    m_imguiRenderPending = true;
}

void VulkanBackend::ImGuiShutdown() {
    if (!m_imguiInitialized) return;
    printf("[Vulkan] Shutting down ImGui Vulkan backend...\n");
    WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    if (m_imguiDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
        m_imguiDescriptorPool = VK_NULL_HANDLE;
    }
    if (m_imguiDescSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device, m_imguiDescSetLayout, nullptr);
        m_imguiDescSetLayout = VK_NULL_HANDLE;
    }
    m_imguiInitialized = false;
    printf("[Vulkan] ImGui Vulkan backend shutdown complete\n");
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
// Utility Functions
// ============================================================================
bool VulkanBackend::SaveScreenshot(const char* path) {
    // For Vulkan, read back swapchain image via staging buffer
    if (!m_device || !path || !m_isRecording) return false;
    
    // Capture current image BEFORE presentation (after present, app loses ownership)
    VkImage srcImage = m_swapchainImages[m_currentImageIndex];
    uint32_t captureIndex = m_currentImageIndex;
    
    // 1. End render pass and command buffer
    if (m_imguiRenderPending) {
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->CmdListsCount > 0)
            ImGui_ImplVulkan_RenderDrawData(drawData, m_commandBuffer);
        m_imguiRenderPending = false;
    }
    vkCmdEndRenderPass(m_commandBuffer);
    m_currentRenderPass = VK_NULL_HANDLE;
    m_currentFramebuffer = VK_NULL_HANDLE;
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
    m_isRecording = false;
    
    // 2. Submit rendering
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence));
    
    // 3. Wait for GPU to finish rendering (DON'T reset fence — leave signaled for next BeginFrame)
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    VkFormat fmt = m_swapchainFormat;
    uint32_t w = m_swapchainExtent.width, h = m_swapchainExtent.height;
    if (w == 0 || h == 0) return false;
    
    // Create staging buffer
    VkDeviceSize bufSize = w * h * 4; // RGBA
    VkBuffer stageBuf; VkDeviceMemory stageMem;
    VkBufferCreateInfo bci = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = bufSize; bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkCreateBuffer(m_device, &bci, nullptr, &stageBuf);
    
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_device, stageBuf, &memReq);
    VkMemoryAllocateInfo ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = memReq.size;
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if (memReq.memoryTypeBits & (1<<i) && (mp.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
            { ai.memoryTypeIndex = i; break; }
    vkAllocateMemory(m_device, &ai, nullptr, &stageMem);
    vkBindBufferMemory(m_device, stageBuf, stageMem, 0);
    
    // Transition swapchain image for transfer, copy, transition back
    // Use a one-time command buffer for this
    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cbai.commandPool = m_commandPool; cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbai.commandBufferCount = 1;
    VkCommandBuffer cb;
    vkAllocateCommandBuffers(m_device, &cbai, &cb);
    
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cb, &cbbi);
    
    // Transition image layout
    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.image = srcImage;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    VkBufferImageCopy region = {};
    region.bufferRowLength = w; region.bufferImageHeight = h;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {w, h, 1};
    vkCmdCopyImageToBuffer(cb, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stageBuf, 1, &region);
    
    // Transition back
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    vkEndCommandBuffer(cb);
    
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1; si.pCommandBuffers = &cb;
    vkQueueSubmit(m_graphicsQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);
    
    // Read back and save
    void* mapped;
    vkMapMemory(m_device, stageMem, 0, bufSize, 0, &mapped);
    uint8_t* src = (uint8_t*)mapped;
    // DEBUG: dump first 4 pixels + center pixel raw (BGRA)
    printf("[Vulkan] Raw pixels[0..3]: ");
    for (int i = 0; i < 4; i++) {
        printf("(%d,%d,%d,%d) ", src[i*4], src[i*4+1], src[i*4+2], src[i*4+3]);
    }
    { size_t cx = (h/2)*w + w/2; printf(" center=(%d,%d,%d,%d)", src[cx*4], src[cx*4+1], src[cx*4+2], src[cx*4+3]); }
    printf("\n");
    std::vector<uint8_t> pixels(w * h * 3);
    for (uint32_t y = 0; y < h; y++)
        for (uint32_t x = 0; x < w; x++) {
            size_t off_src = (y * w + x) * 4;
            size_t off_dst = ((h-1-y) * w + x) * 3; // flip + BGR->RGB
            pixels[off_dst] = src[off_src+2];
            pixels[off_dst+1] = src[off_src+1];
            pixels[off_dst+2] = src[off_src];
        }
    vkUnmapMemory(m_device, stageMem);
    
    FILE* fp = fopen(path, "wb");
    if (fp) {
        fprintf(fp, "P6\n%u %u\n255\n", w, h);
        fwrite(pixels.data(), 1, w*h*3, fp);
        fclose(fp);
        printf("[Vulkan] Screenshot saved to %s\n", path);
    }
    
    vkFreeCommandBuffers(m_device, m_commandPool, 1, &cb);
    vkDestroyBuffer(m_device, stageBuf, nullptr);
    vkFreeMemory(m_device, stageMem, nullptr);
    
    // 4. Present the image (now that we've captured it)
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    VkSemaphore presentWaitSem[] = {m_renderFinishedSemaphore};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = presentWaitSem;
    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &captureIndex;
    VkResult presult = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (presult == VK_ERROR_OUT_OF_DATE_KHR || presult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        RecreateSwapchain();
    
    // NOTE: Don't call BeginFrame() here — MainLoop will call it next iteration
    // m_inFlightFence is left signaled so BeginFrame won't block
    
    return fp != nullptr;
}
uint32_t VulkanBackend::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

VkCommandBuffer VulkanBackend::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanBackend::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void VulkanBackend::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    (void)format;
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == newLayout) {
        // No-op transition: texture already in desired layout
        return;
    } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        fprintf(stderr, "[Vulkan] Unsupported layout transition: %d -> %d\n", oldLayout, newLayout);
        return;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void VulkanBackend::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(commandBuffer);
}

void VulkanBackend::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void VulkanBackend::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VK_CHECK(vkCreateImage(m_device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory));

    vkBindImageMemory(m_device, image, imageMemory, 0);
}

VkImageView VulkanBackend::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView));
    return imageView;
}

VkSampler VulkanBackend::CreateSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler sampler;
    VK_CHECK(vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler));
    return sampler;
}
