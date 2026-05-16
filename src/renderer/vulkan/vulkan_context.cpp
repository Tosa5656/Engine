#include "vulkan_context.h"

VulkanContext* VulkanContext::s_instance = nullptr;

VulkanContext::VulkanContext() = default;
VulkanContext::~VulkanContext() = default;

void VulkanContext::Init(const VkApplicationInfo& appInfo)
{
    instance.Create(appInfo);
    instance.SetupDebugMessenger();
    m_valid = true;
}

void VulkanContext::Destroy()
{
    if (!m_valid) return;
    m_valid = false;

    VkDevice vkDevice = device.GetDevice();
    VkInstance vkInstance = instance.GetInstance();

    if (vkDevice != VK_NULL_HANDLE)
    {
        if (allocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }
        vkDestroyDevice(vkDevice, nullptr);
    }

    if (vkInstance != VK_NULL_HANDLE)
    {
        if (instance.IsExtensionValidationEnabled())
            instance.DestroyDebugUtilsMessengerEXT(vkInstance, instance.GetDebugMessenger(), nullptr);

        vkDestroyInstance(vkInstance, nullptr);
    }
}

VulkanContext* VulkanContext::Get()
{
    return s_instance;
}

VulkanContext* VulkanContext::Create(const VkApplicationInfo& appInfo)
{
    if (s_instance)
        return s_instance;

    s_instance = new VulkanContext();
    s_instance->Init(appInfo);
    return s_instance;
}

void VulkanContext::DestroyInstance()
{
    if (!s_instance) return;
    s_instance->Destroy();
    delete s_instance;
    s_instance = nullptr;
}
