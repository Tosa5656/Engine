#pragma once

#include <optional>
#include <stdexcept>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>

#include <renderer/vulkan/instance.h>

class SwapChain;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// TODO: Create surface class and connect to function
class Device
{
public:
    Device();
    ~Device();

    void Create(Instance* instance, VkSurfaceKHR surface);

    void PickPhysicalDevice(Instance* instance, SwapChain* swapChain, VkSurfaceKHR surface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device, SwapChain* swapChain, VkSurfaceKHR surface);

    VkDevice GetDevice();
    VkPhysicalDevice GetPhysicalDevice();
    VkQueue GetGraphicsQueue();
    uint32_t GetGraphicsQueueFamilyIndex(VkSurfaceKHR surface);
    VkQueue GetPresentQueue();
    uint32_t GetPresentQueueFamilyIndex(VkSurfaceKHR surface);
private:
    Instance* m_instance;

    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };
};