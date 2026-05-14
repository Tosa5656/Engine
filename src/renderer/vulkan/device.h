#pragma once

#include <optional>
#include <stdexcept>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>

#include <renderer/vulkan/instance.h>

class Surface;
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

class Device
{
public:
    Device();
    ~Device();

    void Create(Instance* instance, Surface* surface);

    void PickPhysicalDevice(Instance* instance, Surface* surface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, Surface* surface);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device, Surface* surface);

    VkDevice GetDevice();
    VkPhysicalDevice GetPhysicalDevice();
    VkQueue GetGraphicsQueue();
    uint32_t GetGraphicsQueueFamilyIndex(Surface* surface);
    VkQueue GetPresentQueue();
    uint32_t GetPresentQueueFamilyIndex(Surface* surface);

    void CreateTimestampQueryPool();
    VkQueryPool GetTimestampQueryPool();
    float GetTimestampPeriod();
    void DestroyTimestampQueryPool();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    bool IsMemoryHeapDeviceLocal(uint32_t heapIndex) const;
private:
    Instance* m_instance;

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkQueryPool m_timestampQueryPool = VK_NULL_HANDLE;
    float m_timestampPeriod = 0.0f;
    VkPhysicalDeviceMemoryProperties m_memProperties{};

    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };
};