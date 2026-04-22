#include "swapchain.h"

SwapChain::SwapChain() {}
SwapChain::~SwapChain() {}

void SwapChain::CreateSurface(Instance* instance, GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance->GetInstance(), window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

// TODO: Connect commented functions after implement
void SwapChain::CreateSwapChain(Device* device, GLFWwindow* window)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
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
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    QueueFamilyIndices indices = device->FindQueueFamilies(device->GetPhysicalDevice(), m_surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->GetDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void SwapChain::RecreateSwapChain(Device* device, GLFWwindow* window, CommandBufferManager& commandBufferManager)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device->GetDevice());

    CleanupSwapChain(device);

    commandBufferManager.Recreate(device->GetGraphicsQueueFamilyIndex(m_surface));
    CreateSwapChain(device, window);
    CreateImageViews(device);
    //CreateGraphicsPipeline();
    //CreateUniformBuffers();
    //CreateDescriptorPool();
    //CreateDescriptorSets();
    //CreateCommandBuffers();
    //CreateRenderFinishedSemaphores();
}

void SwapChain::CreateImageViews(Device* device)
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device->GetDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

// TODO: Connect vars after implementation
void SwapChain::CleanupSwapChain(Device* device)
{
    // TEMP!!
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkBuffer> m_uniformBuffers;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    std::vector<VmaAllocation> m_uniformAllocations;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;

    vkDeviceWaitIdle(device->GetDevice());

    vkDestroyPipeline(device->GetDevice(), m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device->GetDevice(), m_pipelineLayout, nullptr);

    for (auto view : m_swapChainImageViews)
        vkDestroyImageView(device->GetDevice(), view, nullptr);

    vkDestroySwapchainKHR(device->GetDevice(), m_swapChain, nullptr);

    for (size_t i = 0; i < m_uniformBuffers.size(); i++)
    {
        if (m_uniformBuffers[i] != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(m_allocator, m_uniformBuffers[i], m_uniformAllocations[i]);
            m_uniformBuffers[i] = VK_NULL_HANDLE;
            m_uniformAllocations[i] = VK_NULL_HANDLE;
        }
    }

    vkDestroyDescriptorPool(device->GetDevice(), m_descriptorPool, nullptr);

    for (auto semaphore : m_renderFinishedSemaphores)
        if (semaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(device->GetDevice(), semaphore, nullptr);

    //m_renderFinishedSemaphores.clear();
    m_swapChainImageViews.clear();
    //m_uniformBuffers.clear();
    //m_descriptorSets.clear();
    //m_graphicsPipeline = VK_NULL_HANDLE;
    //m_pipelineLayout = VK_NULL_HANDLE;
    m_swapChain = VK_NULL_HANDLE;
}

VkSurfaceKHR SwapChain::GetSurface()
{
    return m_surface;
}

VkSwapchainKHR SwapChain::GetSwapChain()
{
    return m_swapChain;
}

std::vector<VkImage> SwapChain::GetSwapChainImages()
{
    return m_swapChainImages;
}

VkFormat SwapChain::GetSwapChainImageFormat()
{
    return m_swapChainImageFormat;
}

VkExtent2D SwapChain::GetSwapChainExtent()
{
    return m_swapChainExtent;
}

std::vector<VkImageView> SwapChain::GetSwapChainImageViews()
{
    return m_swapChainImageViews;
}

SwapChainSupportDetails SwapChain::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

