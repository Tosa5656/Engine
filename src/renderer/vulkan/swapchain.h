#pragma once

#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
public:
    SwapChain();
    ~SwapChain();

    void CreateSurface(Instance* instance, GLFWwindow* window);
    void CreateSwapChain(Device* device, GLFWwindow* window);
    void RecreateSwapChain(Device* device, GLFWwindow* window, CommandBufferManager& commandBufferManager);
    void CreateImageViews(Device* device);

    void CleanupSwapChain(Device* device);

    VkSurfaceKHR GetSurface();
    VkSwapchainKHR GetSwapChain();
    std::vector<VkImage> GetSwapChainImages();
    VkFormat GetSwapChainImageFormat();
    VkExtent2D GetSwapChainExtent();
    std::vector<VkImageView> GetSwapChainImageViews();

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
private:
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;
};