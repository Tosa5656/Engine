#include "swapchain.h"

SwapChain::SwapChain() = default;
SwapChain::~SwapChain() = default;

void SwapChain::Create(Device* device, GLFWwindow* window, Surface* surface)
{
    m_surface = surface;

    auto swapChainSupport = surface->QuerySwapChainSupport(device->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = surface->ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = surface->ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = surface->ChooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->GetSurface();
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
        throw std::runtime_error("failed to create swap chain!");

    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void SwapChain::Recreate(Device* device, GLFWwindow* window, Surface* surface, CommandBufferManager* cmdManager)
{
    m_surface = surface;

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device->GetDevice());

    Cleanup(device);

    cmdManager->Recreate(device->GetGraphicsQueueFamilyIndex(m_surface));
    Create(device, window, surface);
    CreateImageViews(device);
}

void SwapChain::Cleanup(Device* device)
{
    vkDeviceWaitIdle(m_device->GetDevice());

    for (auto view : m_swapChainImageViews)
        vkDestroyImageView(device->GetDevice(), view, nullptr);

    m_swapChainImageViews.clear();

    if (m_swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device->GetDevice(), m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }

    m_swapChainImages.clear();
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
            throw std::runtime_error("failed to create image views!");
    }
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