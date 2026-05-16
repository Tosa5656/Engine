#include "device.h"
#include "surface.h"

Device::Device() : m_instance(nullptr), m_device(VK_NULL_HANDLE), m_physicalDevice(VK_NULL_HANDLE), m_graphicsQueue(VK_NULL_HANDLE), m_presentQueue(VK_NULL_HANDLE) {}
Device::~Device() {}

void Device::Create(Instance* instance, Surface* surface)
{
    m_instance = instance;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &dynamicRenderingFeatures;
    deviceFeatures2.features = deviceFeatures;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pNext = &deviceFeatures2;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Device::PickPhysicalDevice(Instance* instance, Surface* surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance->GetInstance(), &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance->GetInstance(), &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device, surface))
        {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");

    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memProperties);
}

QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device, Surface* surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface->GetSurface(), &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device, Surface* surface)
{
    QueueFamilyIndices indices = FindQueueFamilies(device, surface);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        auto swapChainSupport = surface->QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

VkDevice Device::GetDevice()
{
    return m_device;
}

VkPhysicalDevice Device::GetPhysicalDevice()
{
    return m_physicalDevice;
}

VkQueue Device::GetGraphicsQueue()
{
    return m_graphicsQueue;
}

uint32_t Device::GetGraphicsQueueFamilyIndex(Surface* surface)
{
    return FindQueueFamilies(m_physicalDevice, surface).graphicsFamily.value();
}

VkQueue Device::GetPresentQueue()
{
    return m_presentQueue;
}

uint32_t Device::GetPresentQueueFamilyIndex(Surface* surface)
{
    return FindQueueFamilies(m_physicalDevice, surface).presentFamily.value();
}

void Device::CreateTimestampQueryPool()
{
    if (m_timestampQueryPool != VK_NULL_HANDLE) return;

    VkQueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolInfo.queryCount = 2;

    if (vkCreateQueryPool(m_device, &queryPoolInfo, nullptr, &m_timestampQueryPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create timestamp query pool!");

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    m_timestampPeriod = properties.limits.timestampPeriod;
}

VkQueryPool Device::GetTimestampQueryPool()
{
    return m_timestampQueryPool;
}

float Device::GetTimestampPeriod()
{
    return m_timestampPeriod;
}

void Device::DestroyTimestampQueryPool()
{
    if (m_timestampQueryPool != VK_NULL_HANDLE)
    {
        vkDestroyQueryPool(m_device, m_timestampQueryPool, nullptr);
        m_timestampQueryPool = VK_NULL_HANDLE;
    }
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < m_memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (m_memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool Device::IsMemoryHeapDeviceLocal(uint32_t heapIndex) const
{
    if (heapIndex >= m_memProperties.memoryHeapCount)
        return false;
    return m_memProperties.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
}