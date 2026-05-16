#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/device.h>

class VulkanContext
{
public:
    VulkanContext();
    ~VulkanContext();

    void Init(const VkApplicationInfo& appInfo);
    void Destroy();

    Instance instance;
    Device device;
    VmaAllocator allocator = VK_NULL_HANDLE;

    bool IsValid() const { return m_valid; }

    static VulkanContext* Get();
    static VulkanContext* Create(const VkApplicationInfo& appInfo);
    static void DestroyInstance();

private:
    bool m_valid = false;
    static VulkanContext* s_instance;
};
