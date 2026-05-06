#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/surface.h>

class SwapChain
{
public:
    SwapChain();
    ~SwapChain();

    void Create(Device* device, GLFWwindow* window, Surface* surface, VmaAllocator allocator);
    void Recreate(Device* device, GLFWwindow* window, Surface* surface, CommandBufferManager* cmdManager, VmaAllocator allocator);
    void Cleanup(Device* device);

    void CreateImageViews(Device* device);

    VkSwapchainKHR GetSwapChain();
    std::vector<VkImage> GetSwapChainImages();
    VkFormat GetSwapChainImageFormat();
    VkExtent2D GetSwapChainExtent();
    std::vector<VkImageView> GetSwapChainImageViews();
    VkImageView GetDepthImageView();
    VkFormat GetDepthFormat();
    VkImage GetDepthImage();

private:
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    Surface* m_surface;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VmaAllocation m_depthImageAllocation = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;

    void CreateDepthResources(Device* device, VmaAllocator allocator);
};