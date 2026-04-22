#pragma once

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <renderer/vulkan/instance.h>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Surface
{
public:
    Surface();
    ~Surface();

    void Create(Instance* instance, GLFWwindow* window);
    void Cleanup(Instance* instance);

    VkSurfaceKHR GetSurface();
    VkSurfaceFormatKHR GetSurfaceFormat();
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

private:
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_surfaceFormat{};
};